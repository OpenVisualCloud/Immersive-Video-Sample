/*
 * Copyright (c) 2020, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 *
 */
#ifdef _ANDROID_OS_

#include "VideoDecoder_hw.h"
#include "../Common/RegionData.h"
#include <android/native_window_jni.h>
#include <chrono>
#include <math.h>
#ifdef _USE_TRACE_
#include "../../../trace/MtHQ_tp.h"
#endif

#define DECODE_THREAD_COUNT 16
#define MIN_REMAIN_SIZE_IN_FRAME 0
#define IS_DUMPED 0

std::condition_variable     m_cv; // condition variable for decoder and render target

VCD_NS_BEGIN

VideoDecoder_hw::VideoDecoder_hw()
{
    mDecCtx     = new DecoderContext();
    mHandler    = NULL;
    m_status    = STATUS_UNKNOWN;
    mVideoId    = -1;
    mPkt        = NULL;
    mPktInfo    = NULL;
    mRwpk       = NULL;
    mIsFlushed  = false;
    mDecCtx->mOutputSurface = new OutputSurface();
    mWidth      = 3840;
    mHeight     = 1920;
    mCnt        = 0;
    mDump_YuvFile = NULL;
}

VideoDecoder_hw::~VideoDecoder_hw()
{
    m_status = STATUS_STOPPED;
    CloseDecoder();
    SAFE_DELETE(mDecCtx);

    SAFE_DELETE(mPkt);

    mPktInfo = NULL;
    mRwpk = NULL;
    mIsFlushed = false;
}

RenderStatus VideoDecoder_hw::Initialize(int32_t id, Codec_Type codec, FrameHandler* handler, uint64_t startPts)
{
    this->mVideoId = id;
    switch(codec){
        case VideoCodec_HEVC:
            mDecCtx->mMediaCodec = AMediaCodec_createDecoderByType("video/hevc");
            // mDecCtx->mMediaCodec = AMediaCodec_createCodecByName("OMX.hisi.video.decoder.hevc");
            break;
        case VideoCodec_AVC:
            mDecCtx->mMediaCodec = AMediaCodec_createDecoderByType("video/avc");
            break;
        default:
            LOG(WARNING) << "unsupported type! default width video/hevc!" << endl;
            mDecCtx->mMediaCodec = AMediaCodec_createDecoderByType("video/hevc");
            break;
    }
    if (mDecCtx->mMediaCodec == nullptr)
    {
        LOG(ERROR) << "Failed to create decoder!" << endl;
    }
    mHandler = handler;
    SetStartPts(startPts);
    LOG(INFO) << "Start pts is " << startPts << " video id is " << id << endl;
    return Initialize(codec);
}

RenderStatus VideoDecoder_hw::Initialize(Codec_Type codec)
{
    // 1. new a media format context
    if (!mDecCtx->mMediaFormat)
    {
        mDecCtx->mMediaFormat = AMediaFormat_new();
    }
    // 2. set init params
    if (codec == VideoCodec_AVC)
    {
        ANDROID_LOGD("video codec is avc");
        AMediaFormat_setString(mDecCtx->mMediaFormat, "mime", "video/avc");
    }
    else
    {
        ANDROID_LOGD("video codec is hevc");
        AMediaFormat_setString(mDecCtx->mMediaFormat, "mime", "video/hevc");
    }

    AMediaFormat_setInt32(mDecCtx->mMediaFormat, AMEDIAFORMAT_KEY_WIDTH, mWidth);
    AMediaFormat_setInt32(mDecCtx->mMediaFormat, AMEDIAFORMAT_KEY_HEIGHT, mHeight);
    AMediaFormat_setInt32(mDecCtx->mMediaFormat, "allow-frame-drop", 0);
    AMediaFormat_setInt32(mDecCtx->mMediaFormat, AMEDIAFORMAT_KEY_FRAME_RATE, uint32_t(float(mDecodeInfo.frameRate_num) / mDecodeInfo.frameRate_den));
    // 3. get native window from surface
    if (m_status == STATUS_UNKNOWN) {
        CreateOutputSurface();
    }
    // 4. set native window to media codec configuration
    media_status_t status;
    if (IS_DUMPED != 1){
        status = AMediaCodec_configure(mDecCtx->mMediaCodec, mDecCtx->mMediaFormat, mDecCtx->mOutputSurface->native_window, NULL, 0);
    }else
    {
        status = AMediaCodec_configure(mDecCtx->mMediaCodec, mDecCtx->mMediaFormat, NULL, NULL, 0);
    }
    if (status != AMEDIA_OK)
    {
        AMediaCodec_delete(mDecCtx->mMediaCodec);
        mDecCtx->mMediaCodec = nullptr;
        return RENDER_CREATE_ERROR;
    }
    // 5. start the codec
    status = AMediaCodec_start(mDecCtx->mMediaCodec);
    if (status != AMEDIA_OK)
    {
        AMediaCodec_delete(mDecCtx->mMediaCodec);
        mDecCtx->mMediaCodec = nullptr;
        return RENDER_CREATE_ERROR;
    }
    if (m_status == STATUS_UNKNOWN) {
        StartThread();
    }
    mIsFlushed = false;
    LOG(INFO) << "A new video decoder is created!" << std::endl;
    mDump_YuvFile = fopen("sdcard/Android/data/tmp/1.yuv","wb");
    return RENDER_STATUS_OK;
}

RenderStatus VideoDecoder_hw::Destroy()
{
    CloseDecoder();
    return RENDER_STATUS_OK;
}

void VideoDecoder_hw::CloseDecoder()
{
    if( (m_status == STATUS_STOPPED) || (m_status == STATUS_IDLE) || m_status == STATUS_PENDING){
        m_status = STATUS_STOPPED;
        LOG(INFO)<<" decoder is closed! video id is " << mVideoId<<endl;
        this->Join();
    }
    if (mDecCtx != nullptr)
    {
        AMediaCodec_stop(mDecCtx->mMediaCodec);
        AMediaCodec_delete(mDecCtx->mMediaCodec);
        mDecCtx->mMediaCodec = nullptr;
    }
}

RenderStatus VideoDecoder_hw::Reset(int32_t id, Codec_Type codec, uint64_t startPts)
{
    CloseDecoder();

    return Initialize(id, codec, mHandler, startPts);
}

RenderStatus VideoDecoder_hw::SendPacket(DashPacket* packet)
{
    if(NULL == packet) return RENDER_NULL_PACKET;

    if (packet->bEOS) // eos
    {
        PacketInfo* endPkt = new PacketInfo;
        endPkt->bEOS = true;
        mDecCtx->push_packet(endPkt);
        mDecCtx->bPacketEOS = true;
        return RENDER_STATUS_OK;
    }
    RenderStatus ret = RENDER_STATUS_OK;
    SAFE_DELETE(mRwpk);
    mPktInfo = new PacketInfo;
    mRwpk = new RegionWisePacking;
    mPkt = new DashPacket;
    if (mPktInfo == NULL || mRwpk == NULL || mPkt == NULL)
    {
        LOG(ERROR)<<" alloc memory failed in send packet! " << endl;
        SAFE_DELETE(mPktInfo);
        SAFE_DELETE(mRwpk);
        SAFE_DELETE(mPkt);
        return RENDER_ERROR;
    }
    // mPktInfo->bCodecChange = false;//MediaInfoChange(packet, 0);

    // mDecCtx->width = packet->width;
    // mDecCtx->height = packet->height;

    //send a packet to AVPACKET list

    if (NULL != packet->buf && packet->size)
    {
        // use mPkt to store buf and size
        mPkt->size = packet->size;
        mPkt->buf = new char[mPkt->size];
        mPkt->width = packet->width;
        mPkt->height = packet->height;
        memcpy_s(mPkt->buf, packet->size, packet->buf, packet->size);

        *mRwpk = *(packet->rwpk);

        FrameData* data = new FrameData;
        mPktInfo->pkt = mPkt;
        mPktInfo->bEOS = packet->bEOS;
        mPktInfo->pts = packet->pts;
        mPktInfo->video_id = packet->videoID;
        mDecCtx->push_packet(mPktInfo);
        ANDROID_LOGD("Push packet at pts %ld", packet->pts);
        if (mPktInfo->pkt->buf == nullptr)
        {
            ANDROID_LOGD("Push empty buf at pts %ld ", packet->pts);
        }
        data->rwpk = new RegionWisePacking;
        *(data->rwpk) = *mRwpk;
        data->rwpk->rectRegionPacking = new RectangularRegionWisePacking[mRwpk->numRegions];
        memcpy_s(data->rwpk->rectRegionPacking, mRwpk->numRegions * sizeof(RectangularRegionWisePacking),
           mRwpk->rectRegionPacking, mRwpk->numRegions * sizeof(RectangularRegionWisePacking));

        // data->pts = mPkt->pts;
        data->pts = mPktInfo->pts;
        data->numQuality = packet->numQuality;
        data->qtyResolution = new SourceResolution[packet->numQuality];
        data->bCodecChange = mPktInfo->bCodecChange;
        for(int i =0; i<data->numQuality; i++){
            data->qtyResolution[i].height = packet->qtyResolution[i].height;
            data->qtyResolution[i].width = packet->qtyResolution[i].width;
            data->qtyResolution[i].left = packet->qtyResolution[i].left;
            data->qtyResolution[i].top = packet->qtyResolution[i].top;
            data->qtyResolution[i].qualityRanking = packet->qtyResolution[i].qualityRanking;
        }
        mDecCtx->push_framedata(data);
        // for (uint32_t i = 0; i < data->rwpk->numRegions; i++){
        // ANDROID_LOGD("PTS: %d, data->rwpk rwpk idx: %d, one rrwpk w: %d, h: %d, l: %d, t: %d", data->pts, i, data->rwpk->rectRegionPacking[i].projRegWidth,
        // data->rwpk->rectRegionPacking[i].projRegHeight, data->rwpk->rectRegionPacking[i].projRegLeft, data->rwpk->rectRegionPacking[i].projRegTop);
        // }
        LOG(INFO)<<"frame data fifo size is: "<<mDecCtx->get_size_of_framedata() <<"pts is " << data->pts <<" VIDEO ID : " << mPktInfo->video_id << endl;
    }

    return ret;
}

RenderStatus VideoDecoder_hw::DecodeFrame(DashPacket *pkt, uint32_t video_id)
{
    RenderStatus ret = RENDER_STATUS_OK;
    if (pkt == nullptr)
    {
        return RENDER_NULL_HANDLE;
    }
    if (nullptr == mDecCtx->mMediaCodec)
    {
        LOG(ERROR) << "MediaCodec is not initialized successfully!" << endl;
        return RENDER_NULL_HANDLE;
    }
    // if changing the resolution, then reconfigure the decoder
    static int cnt_pts = 0;
    ANDROID_LOGD("CHANGE: PTS: %ld, Packet has width %d, height %d", cnt_pts, pkt->width, pkt->height);
    cnt_pts++;
    // 1. decode one frame
    //send packet
    ssize_t buf_idx = AMediaCodec_dequeueInputBuffer(mDecCtx->mMediaCodec, 10000);
    if (buf_idx >= 0)
    {
        ANDROID_LOGD("Ready to get input buffer!");
        size_t out_size = 0;
        uint8_t *input_buf = AMediaCodec_getInputBuffer(mDecCtx->mMediaCodec, buf_idx, &out_size);
        if (input_buf != nullptr && pkt->size <= out_size)
        {
            if (pkt->buf == nullptr)
            {
                ANDROID_LOGD("pkt->buf is empty!");
            }
            memcpy(input_buf, pkt->buf, pkt->size);

            ANDROID_LOGD("Input buffer pts %ld", cnt_pts-1);
            int ptsFactor = (int) floor((1 * 1000 * 1000 / (float(mDecodeInfo.frameRate_num) / mDecodeInfo.frameRate_den)));
            media_status_t status = AMediaCodec_queueInputBuffer(mDecCtx->mMediaCodec, buf_idx, 0, pkt->size, mCnt++ * ptsFactor, 0);
            if (status != AMEDIA_OK)
            {
                ANDROID_LOGD("queue input buffer failed at pts %d", cnt_pts-1);
            }
        }
        else {
            ANDROID_LOGD("pkt->size %d is greater than out_size %d", pkt->size, out_size);
        }
    }
    else{
        ret = RENDER_NULL_PACKET;
        ANDROID_LOGD("send packet failed!");
    }
    // usleep(20000);
    //receive frame
    AMediaCodecBufferInfo buf_info;
    ssize_t out_buf_idx = AMediaCodec_dequeueOutputBuffer(mDecCtx->mMediaCodec, &buf_info, 0);
    if (out_buf_idx >= 0) // buffer has no data
    {
        if (out_buf_idx > 0) ANDROID_LOGD("frame info size is greater than zero!");
        if (IS_DUMPED != 1){
            static int cnt = 0;
            cnt++;
            // ANDROID_LOGD("buf_info size is %d", buf_info.size);
            AMediaCodec_releaseOutputBuffer(mDecCtx->mMediaCodec, out_buf_idx, buf_info.size != 0);
        }
        else
        {
            uint8_t *outputBuf = AMediaCodec_getOutputBuffer(mDecCtx->mMediaCodec, out_buf_idx, NULL);
            size_t dataSize = buf_info.size;
            if (outputBuf != nullptr && dataSize != 0 && mVideoId == 0)
            {
                ANDROID_LOGD("CHANGE: dataSize is %d at pts %d", dataSize, cnt_pts-1);
                fwrite(outputBuf, 1, dataSize, mDump_YuvFile);
            }
            AMediaCodec_releaseOutputBuffer(mDecCtx->mMediaCodec, out_buf_idx, false);
        }
        // 2. pop framedata
        ANDROID_LOGD("frame data size is %d", mDecCtx->get_size_of_framedata());
        FrameData* data = mDecCtx->pop_framedata();
        if (data == NULL)
        {
            LOG(ERROR) << "Frame data is empty!" << endl;
            return RENDER_NO_FRAME;
        }
        DecodedFrame *frame = new DecodedFrame;
        frame->output_surface = mDecCtx->mOutputSurface;
        frame->rwpk = data->rwpk;
        frame->pts = data->pts;
        frame->bFmtChange = data->bCodecChange;
        frame->numQuality = data->numQuality;
        frame->qtyResolution = data->qtyResolution;
        frame->video_id = video_id;
        frame->bEOS = false;
        mDecCtx->push_frame(frame);
        ANDROID_LOGD("PTS: %d, frame rwpk num: %d, one rrwpk w: %d, h: %d, l: %d, t: %d", data->pts, data->rwpk->numRegions, data->rwpk->rectRegionPacking[0].projRegWidth,
        data->rwpk->rectRegionPacking[0].projRegHeight, data->rwpk->rectRegionPacking[0].projRegLeft, data->rwpk->rectRegionPacking[0].projRegTop);
        SAFE_DELETE(data);
        //successfully decode one frame
        ANDROID_LOGD("CHANGE: successfully decode one frame at pts %ld", frame->pts);
        std::mutex mtx;
        std::unique_lock<std::mutex> lck(mtx);
        m_cv.wait(lck);
    }
    else
    {
        switch (out_buf_idx)
        {
        case AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED://output format has changed
        {
            ANDROID_LOGD("media codec info output format is changed!");
            AMediaFormat *format = AMediaCodec_getOutputFormat(mDecCtx->mMediaCodec);
            AMediaFormat_getInt32(format, "width", &(pkt->width));
            AMediaFormat_getInt32(format, "height", &(pkt->height));
            // AMediaFormat_getInt32(format, "stride", &(pkt->width));
            break;
        }
        default:
        {
            LOG(ERROR) << "outout buffer index error occurs!" << endl;
            break;
        }
        }
    }
    return ret;
}

RenderStatus VideoDecoder_hw::FlushDecoder(uint32_t video_id)
{
    if (mDecCtx->mMediaCodec)
    {
        AMediaCodec_flush(mDecCtx->mMediaCodec);
    }
    return RENDER_STATUS_OK;
}

bool VideoDecoder_hw::MediaInfoChange(DashPacket* packet, uint64_t pts)
{
    bool bChange = false;
    LOG(INFO)<<"packet has width "<<packet->width << " and height "<<packet->height<<endl;
    if(packet->height != mDecCtx->height){ // not the first time.
        ANDROID_LOGD("CHANGE: PTS: %ld, height has changed from %d to %d", pts, mDecCtx->height, packet->height);
        bChange = true;
        mDecCtx->height = packet->height;
    }
    if(packet->width != mDecCtx->width){
        ANDROID_LOGD("CHANGE: PTS: %ld, width has changed from %d to %d", pts, mDecCtx->width, packet->width);
        bChange = true;
        mDecCtx->width = packet->width;
    }
    if(packet->numQuality != mDecCtx->numQuality && mDecCtx->numQuality != 0){
        bChange = true;
        // mDecCtx->numQuality = packet->numQuality;

        // for(int i=0; i++; i<mDecCtx->numQuality){
        //     mDecCtx->qtyResolution[i].qualityRanking = packet->qtyResolution[i].qualityRanking;
        //     mDecCtx->qtyResolution[i].height = packet->qtyResolution[i].height;
        //     mDecCtx->qtyResolution[i].width = packet->qtyResolution[i].width;
        // }
    }
    // unused parameters
    if(packet->tileColNum != mDecCtx->tileColNum){
        //bChange = true;
        mDecCtx->tileColNum = packet->tileColNum;
    }
    if(packet->tileRowNum != mDecCtx->tileRowNum){
        //bChange = true;
        mDecCtx->tileRowNum = packet->tileRowNum;
    }

    return bChange;
}

void VideoDecoder_hw::Run()
{
    RenderStatus ret = RENDER_STATUS_OK;
    m_status = STATUS_RUNNING;

    while (m_status != STATUS_STOPPED && m_status != STATUS_IDLE)
    {
        // when the status is set to pending
        if (m_status == STATUS_PENDING)
        {
            //flush decoder until all packets are popped.
            if (mDecCtx->get_size_of_packet() == 0 && !mIsFlushed)
            {
                LOG(INFO)<<"Now will flush the decoder "<< mVideoId << endl;
                ret = FlushDecoder(mVideoId);
                if (RENDER_STATUS_OK != ret)
                {
                    LOG(INFO)<<"Video "<< mVideoId <<": failed to flush decoder when status is pending!"<<std::endl;
                }
                mIsFlushed = true;
                continue;
            }
        }
        if(0 == mDecCtx->get_size_of_packet()){
            usleep(1000);
            continue;
        }

        PacketInfo* pkt_info = mDecCtx->pop_packet();

        if(NULL == pkt_info){
            LOG(INFO)<<"possible error since null packet has been pushed to queue"<<std::endl;
            continue;
        }
        if (pkt_info->pkt->buf == nullptr)
        {
            ANDROID_LOGD("pop packet buf is empty!");
        }
        LOG(INFO)<<"Now packet pts is "<<pkt_info->pts<<"video id is " << mVideoId<<endl;
        ANDROID_LOGD("Packet size is %d", mDecCtx->get_size_of_packet());
	    // check eos status and do flush operation.
        if(pkt_info->bEOS)
        {
            ret = FlushDecoder(mVideoId);
            if(RENDER_STATUS_OK != ret){
                LOG(INFO)<<"Video "<< mVideoId <<": failed to flush decoder when EOS"<<std::endl;
            }
            m_status = STATUS_IDLE;
	        continue;
        }
        ///need not to reset decoder if w/h changed
        // if(pkt_info->bCodecChange){
        //     LOG(INFO)<<"Video "<< mVideoId <<":Input stream parameters Changed, reset decoder"<<std::endl;
        //     // this->Reset();
        //     mbFmtChange = true;
        // }
        ret = RENDER_STATUS_OK;
        do {
        ret = DecodeFrame(pkt_info->pkt, pkt_info->video_id);
        } while (ret == RENDER_NULL_PACKET);// ensure that send packet is right
        if(RENDER_STATUS_OK != ret){
             LOG(INFO)<<"Video "<< mVideoId <<": failed to decoder one frame"<<std::endl;
        }
    }
}

OutputSurface* VideoDecoder_hw::GetOutputSurface(uint64_t pts)
{
    return mDecCtx->mOutputSurface;
}

RenderStatus VideoDecoder_hw::GetFrame(uint64_t pts, DecodedFrame *&frame)
{
    bool waitFlag = false;
    while(mDecCtx->get_size_of_frame() > 0){
        frame = mDecCtx->get_front_of_frame();
        ANDROID_LOGD("frame size is: %d  and frame pts is: %d, and input pts is: %d, video id is: %d ", mDecCtx->get_size_of_frame(), frame->pts, pts, mVideoId);
        if(frame->pts == pts)
        {
            frame = mDecCtx->pop_frame();
            ANDROID_LOGD("Pop one frame at: %d video id is %d", pts, mVideoId);
            break;
        }
        else if (frame->pts > pts) // wait
        {
            ANDROID_LOGD("Need to wait frame to match current pts! frame->pts %d, pts is %d, video id is %d ", frame->pts, pts, mVideoId);
            frame = NULL;
            waitFlag = true;
            return RENDER_WAIT;
        }
        // drop over time frame.
        frame = mDecCtx->pop_frame();
        ANDROID_LOGD("Now will drop one frame since pts is over time! input pts is: %d, frame pts is %d, video id is %d", pts, frame->pts, mVideoId);
        if (frame->rwpk)
            SAFE_DELETE_ARRAY(frame->rwpk->rectRegionPacking);
        SAFE_DELETE(frame->rwpk);
        SAFE_DELETE_ARRAY(frame->qtyResolution);
        SAFE_DELETE(frame);
    }

    if( !waitFlag && (mDecCtx->get_size_of_frame() == 0) && (m_status==STATUS_PENDING) ){
        ANDROID_LOGD("frame fifo is empty now! video id is : %d", mVideoId);
        m_status = STATUS_IDLE;
        ANDROID_LOGD("decoder status is set to idle!");
    }

    if (frame == nullptr) return RENDER_NO_FRAME;
    else return RENDER_STATUS_OK;
}

void VideoDecoder_hw::Pending()
{
    mDecCtx->bPacketEOS = true;
    LOG(INFO) << "Set decoder status to PENDING!" << endl;
    m_status = STATUS_PENDING;
}

RenderStatus VideoDecoder_hw::UpdateFrame(uint64_t pts, int64_t *corr_pts)
{
    DecodedFrame* frame = GetFrame(pts);
    if(NULL==frame)
    {
        ANDROID_LOGD("VideoDecoder_hw::Frame is empty!");
        return RENDER_NO_FRAME;
    }
    if (frame->bEOS)
    {
        this->SetEOS(true);
    }
    // ANDROID_LOGD("frame numQ is %d, rwpk is %p, rwpk->rect is %p, source reso is %p", frame->numQuality, frame->rwpk, frame->rwpk->rectRegionPacking, frame->qtyResolution);
    BufferInfo* buf_info = new BufferInfo;
    buf_info->regionInfo = new RegionData(frame->rwpk, frame->numQuality, frame->qtyResolution);
    // ANDROID_LOGD("frame->numQuality: %d", frame->numQuality);
    if(NULL != this->mHandler){
        mHandler->process(buf_info);// pass region data
        // ANDROID_LOGD("VideoDecoder_hw::Do process");
    }
    buf_info->regionInfo = nullptr;
    SAFE_DELETE(buf_info);
    // SAFE_DELETE_ARRAY(frame->rwpk->rectRegionPacking);
    SAFE_DELETE(frame->rwpk);
    SAFE_DELETE_ARRAY(frame->qtyResolution);
    SAFE_DELETE(frame);
    return RENDER_STATUS_OK;
}

RenderStatus VideoDecoder_hw::SetRegionInfo(struct RegionInfo *regionInfo, int32_t nQuality, SourceResolution *qtyRes)
{
    return RENDER_STATUS_OK;
}

bool VideoDecoder_hw::IsReady(uint64_t pts)
{
    ANDROID_LOGD("At first, frame size is %d and packet eos is %d and video id is %d, pts is %ld", mDecCtx->get_size_of_frame(), mDecCtx->bPacketEOS, mVideoId, pts);
    if (mDecCtx->get_size_of_frame() > MIN_REMAIN_SIZE_IN_FRAME || mDecCtx->bPacketEOS || pts < GetStartPts()){
        return true;
    }else{
        return false;
    }
}

RenderStatus VideoDecoder_hw::CreateOutputSurface()
{
    if (mDecCtx->mOutputSurface->out_surface != nullptr)
    {
        return RENDER_ERROR;
    }
    mDecCtx->mOutputSurface->out_surface = new Surface();
    // init OpenglES context
    mDecCtx->mOutputSurface->out_surface->CreateSurface();

    jobject jsurface = mDecCtx->mOutputSurface->out_surface->GetJobject();

    ANDROID_LOGD("decoder manager : set surface at i : %d surface is %p", mVideoId, m_nativeSurface);
    if (IS_DUMPED != 1)
        mDecCtx->mOutputSurface->native_window = ANativeWindow_fromSurface(JNIContext::GetJNIEnv(), (jobject)m_nativeSurface);
    else
        mDecCtx->mOutputSurface->native_window = ANativeWindow_fromSurface(JNIContext::GetJNIEnv(), jsurface);
    return RENDER_STATUS_OK;
}

VCD_NS_END
#endif //_ANDROID_OS_