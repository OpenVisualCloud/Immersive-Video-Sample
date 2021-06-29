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
#include <condition_variable>
#ifdef _USE_TRACE_
#include "../../../trace/MtHQ_tp.h"
#endif

#define DECODE_THREAD_COUNT 16
#define MIN_REMAIN_SIZE_IN_FRAME 0
#define IS_DUMPED 0
#define INITIAL_DECODE_WIDTH 960
#define INITIAL_DECODE_HEIGHT 960

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
    mWidth      = INITIAL_DECODE_WIDTH;
    mHeight     = INITIAL_DECODE_HEIGHT;
    mCnt        = 0;
    mDump_YuvFile = NULL;
    mNextInputPts   = 0;
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
    AMediaFormat_setInt32(mDecCtx->mMediaFormat, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, mWidth * mHeight * 10);
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
    // mDump_YuvFile = fopen("sdcard/Android/data/tmp/1.yuv","wb");
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

    if (packet->bEOS && !packet->bCatchup) // eos
    {
        PacketInfo* endPkt = new PacketInfo;
        endPkt->pkt = nullptr;
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
        mPkt->pts = packet->pts;
        memcpy_s(mPkt->buf, packet->size, packet->buf, packet->size);

        *mRwpk = *(packet->rwpk);

        FrameData* data = new FrameData;
        mPktInfo->pkt = mPkt;
        mPktInfo->bEOS = packet->bEOS;
        mPktInfo->pts = packet->pts;
        mPktInfo->video_id = packet->videoID;
        mPktInfo->bCatchup = packet->bCatchup;
        mDecCtx->push_packet(mPktInfo);
        ANDROID_LOGD("Push packet at pts %ld, video id %d", packet->pts, mPktInfo->video_id);
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
        data->bCatchup = mPktInfo->bCatchup;
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
    // 1. decode one frame
    //send packet
    ssize_t buf_idx = AMediaCodec_dequeueInputBuffer(mDecCtx->mMediaCodec, 10000);
    if (buf_idx >= 0)
    {
        ANDROID_LOGD("CHANGE: PTS: %ld, Packet has width %d, height %d, video id %d", pkt->pts, pkt->width, pkt->height, mVideoId);
        ANDROID_LOGD("Ready to get input buffer pts is %ld !, video id %d", pkt->pts, mVideoId);
        size_t out_size = 0;
        uint8_t *input_buf = AMediaCodec_getInputBuffer(mDecCtx->mMediaCodec, buf_idx, &out_size);
        if (input_buf != nullptr && pkt->size <= out_size)
        {
            if (pkt->buf == nullptr)
            {
                ANDROID_LOGD("pkt->buf is empty!");
            }
            memcpy(input_buf, pkt->buf, pkt->size);

            int ptsFactor = (int) floor((1 * 1000 * 1000 / (float(mDecodeInfo.frameRate_num) / mDecodeInfo.frameRate_den)));
            media_status_t status = AMediaCodec_queueInputBuffer(mDecCtx->mMediaCodec, buf_idx, 0, pkt->size, pkt->pts * ptsFactor, 0);
            if (status != AMEDIA_OK)
            {
                ANDROID_LOGD("queue input buffer failed at pts %d", pkt->pts);
            }
        }
        else {
            ANDROID_LOGD("pkt->size %d is greater than out_size %d", pkt->size, out_size);
        }
    }
    else{
        ret = RENDER_NULL_PACKET;
        ANDROID_LOGD("send packet failed!, video id %d", mVideoId);
    }
    // usleep(20000);
    //receive frame
    AMediaCodecBufferInfo buf_info;
    ssize_t out_buf_idx = AMediaCodec_dequeueOutputBuffer(mDecCtx->mMediaCodec, &buf_info, 0);
    if (out_buf_idx >= 0) // buffer has no data
    {
        // 1. pop framedata
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
        ANDROID_LOGD("data->pts %ld, video id %d", data->pts, mVideoId);
        frame->bFmtChange = data->bCodecChange;
        frame->numQuality = data->numQuality;
        frame->qtyResolution = data->qtyResolution;
        frame->video_id = video_id;
        frame->bEOS = false;
        frame->bCatchup = data->bCatchup;
        while (frame->bCatchup && frame->pts > mNextInputPts) {
            ANDROID_LOGD("check frame pts %ld is greater than input pts %d, wait!", frame->pts, mNextInputPts);
            usleep(5);
        }
        ANDROID_LOGD("PTS: %d, frame rwpk num: %d, one rrwpk w: %d, h: %d, l: %d, t: %d", data->pts, data->rwpk->numRegions, data->rwpk->rectRegionPacking[0].projRegWidth,
        data->rwpk->rectRegionPacking[0].projRegHeight, data->rwpk->rectRegionPacking[0].projRegLeft, data->rwpk->rectRegionPacking[0].projRegTop);
        SAFE_DELETE(data);
        // 2. release output buffer
        if (out_buf_idx > 0) ANDROID_LOGD("frame info size is greater than zero!");
        if (IS_DUMPED != 1){
            bool render = (buf_info.size != 0) && (frame->pts == mNextInputPts || mNextInputPts == 0);
            ANDROID_LOGD("is render %d, pts %ld, video id %d", render, frame->pts, mVideoId);
            AMediaCodec_releaseOutputBuffer(mDecCtx->mMediaCodec, out_buf_idx, render);
        }
        else
        {
            uint8_t *outputBuf = AMediaCodec_getOutputBuffer(mDecCtx->mMediaCodec, out_buf_idx, NULL);
            size_t dataSize = buf_info.size;
            if (outputBuf != nullptr && dataSize != 0 && mVideoId == 0)
            {
                ANDROID_LOGD("CHANGE: dataSize is %d at pts %d", dataSize, pkt->pts);
                // fwrite(outputBuf, 1, dataSize, mDump_YuvFile);
            }
            AMediaCodec_releaseOutputBuffer(mDecCtx->mMediaCodec, out_buf_idx, false);
        }
        //successfully decode one frame
        if (frame->bCatchup)
            ANDROID_LOGD("CHANGE: successfully decode one catch up frame at pts %ld", frame->pts);
        else
            ANDROID_LOGD("CHANGE: successfully decode one frame at pts %ld video id %d", frame->pts, mVideoId);

        ANDROID_LOGD("mNextInputPts %d", mNextInputPts);
        mDecCtx->push_frame(frame);
        //swift deocde when needing drop frame, and for wait and normal situation, do as follows.
        if (frame->pts >= mNextInputPts) {
            ANDROID_LOGD("Input pts %lld, frame pts %lld video id %d", mNextInputPts, frame->pts, mVideoId);
            std::mutex mtx;
            std::unique_lock<std::mutex> lck(mtx);
            m_cv.wait(lck);
        }
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
            int32_t capacity_wh = pkt->width * pkt->height;
            AMediaFormat_getInt32(mDecCtx->mMediaFormat, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, &(capacity_wh));
            // AMediaFormat_getInt32(format, "stride", &(pkt->width));
            break;
        }
        default:
        {
            ANDROID_LOGD("output buffer index error occurs!, error id is %d", out_buf_idx);
            break;
        }
        }
    }
    return ret;
}

RenderStatus VideoDecoder_hw::FlushDecoder(uint32_t video_id)
{
    RenderStatus ret = RENDER_STATUS_OK;
    if (nullptr == mDecCtx->mMediaCodec)
    {
        LOG(ERROR) << "MediaCodec is not initialized successfully!" << endl;
        return RENDER_NULL_HANDLE;
    }
    // 1. decode one frame
    //send EOS flag to decoder
    ssize_t buf_idx = AMediaCodec_dequeueInputBuffer(mDecCtx->mMediaCodec, 10000);
    if (buf_idx >= 0)
    {
        ANDROID_LOGD("send EOS frame to decoder! video id %d", mVideoId);
        media_status_t status = AMediaCodec_queueInputBuffer(mDecCtx->mMediaCodec, buf_idx, 0, 0, 0, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
    }
    else{
        ret = RENDER_NULL_PACKET;
        ANDROID_LOGD("send packet failed!, error code is %d, video id %d", buf_idx, mVideoId);
        return ret;
    }
    // 2. receive frame until meeting the EOS flag
    AMediaCodecBufferInfo buf_info;
    do {
    ssize_t out_buf_idx = AMediaCodec_dequeueOutputBuffer(mDecCtx->mMediaCodec, &buf_info, 0);
    if (buf_info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
        ANDROID_LOGD("Found eos frame with flags! video id %d", mVideoId);
        break;
    }
    if (out_buf_idx >= 0) // buffer has no data
    {
        // 1. pop framedata
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
        frame->bCatchup = data->bCatchup;
        while (frame->bCatchup && frame->pts > mNextInputPts) {
            ANDROID_LOGD("check frame pts %ld is greater than input pts %d, wait!", frame->pts, mNextInputPts);
            usleep(5);
        }
        ANDROID_LOGD("PTS: %d, frame rwpk num: %d, one rrwpk w: %d, h: %d, l: %d, t: %d", data->pts, data->rwpk->numRegions, data->rwpk->rectRegionPacking[0].projRegWidth,
        data->rwpk->rectRegionPacking[0].projRegHeight, data->rwpk->rectRegionPacking[0].projRegLeft, data->rwpk->rectRegionPacking[0].projRegTop);
        // 2. release output buffer
        if (out_buf_idx > 0) ANDROID_LOGD("frame info size is greater than zero!");
        if (IS_DUMPED != 1){
            bool render = (buf_info.size != 0) && (frame->pts == mNextInputPts || mNextInputPts == 0);
            ANDROID_LOGD("is render %d, pts %ld, video id %d", render, frame->pts, mVideoId);
            AMediaCodec_releaseOutputBuffer(mDecCtx->mMediaCodec, out_buf_idx, render);
        }
        else
        {
            uint8_t *outputBuf = AMediaCodec_getOutputBuffer(mDecCtx->mMediaCodec, out_buf_idx, NULL);
            size_t dataSize = buf_info.size;
            if (outputBuf != nullptr && dataSize != 0 && mVideoId == 0)
            {
                // ANDROID_LOGD("CHANGE: dataSize is %d at pts %d", dataSize, cnt_pts-1);
                // fwrite(outputBuf, 1, dataSize, mDump_YuvFile);
            }
            AMediaCodec_releaseOutputBuffer(mDecCtx->mMediaCodec, out_buf_idx, false);
        }
        SAFE_DELETE(data);
        //successfully decode one frame
        if (frame->bCatchup)
            ANDROID_LOGD("CHANGE: successfully decode one catch up frame at pts %ld", frame->pts);
        else
            ANDROID_LOGD("CHANGE: successfully decode one frame at pts %ld video id %d", frame->pts, mVideoId);

        ANDROID_LOGD("mNextInputPts %d, video id %d", mNextInputPts, mVideoId);
        mDecCtx->push_frame(frame);
        //swift deocde when needing drop frame, and for wait and normal situation, do as follows.
        if (frame->pts >= mNextInputPts) {
            std::mutex mtx;
            std::unique_lock<std::mutex> lck(mtx);
            m_cv.wait(lck);
            ANDROID_LOGD("Input pts %lld, frame pts %lld", mNextInputPts, frame->pts);
        }
    }
    } while (true);
    return ret;
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
                ANDROID_LOGD("Now will flush the decoder %d", mVideoId);
                ret = FlushDecoder(mVideoId);
                if (RENDER_STATUS_OK != ret)
                {
                    LOG(INFO)<<"Video "<< mVideoId <<": failed to flush decoder when status is pending!"<<std::endl;
                }
                mIsFlushed = true;
                AMediaCodec_flush(mDecCtx->mMediaCodec);
                ANDROID_LOGD("After flushing, resume the decoder id %d", mVideoId);
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
        LOG(INFO)<<"Now packet pts is "<<pkt_info->pts<<"video id is " << mVideoId<<endl;
        ANDROID_LOGD("Now packet pts is %ld, video id %d", pkt_info->pts, mVideoId);
        ANDROID_LOGD("Packet size is %d", mDecCtx->get_size_of_packet());
	    // check eos status and do flush operation.
        if(pkt_info->bEOS)
        {
            bool isCatchup = pkt_info->bCatchup;
            if (pkt_info->pkt) {//catch up eos last frame
                LOG(INFO) << "Decoded frame is pts " << pkt_info->pts << endl;
                do {
                ret = DecodeFrame(pkt_info->pkt, pkt_info->video_id);
                } while (ret == RENDER_NULL_PACKET);// ensure that send packet is right
                if(RENDER_STATUS_OK != ret){
                    LOG(INFO)<<"Video "<< mVideoId <<": failed to decoder one frame"<<std::endl;
                }

                SAFE_DELETE(pkt_info->pkt);

                SAFE_DELETE(pkt_info);
            }
            ANDROID_LOGD("Finish to decode last eos frame, video id %d", mVideoId);
            do {
            ret = FlushDecoder(mVideoId);
            } while (ret == RENDER_NULL_PACKET);// ensure that send packet is right
            if(RENDER_STATUS_OK != ret){
                LOG(INFO)<<"Video "<< mVideoId <<": failed to flush decoder when EOS"<<std::endl;
            }
            ANDROID_LOGD("Flush the docoder video id %d", mVideoId);
            if (isCatchup) {
                AMediaCodec_flush(mDecCtx->mMediaCodec);
                ANDROID_LOGD("After flushing, resume the decoder id %d", mVideoId);
            }
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
    DecodedFrame* frame = nullptr;
    RenderStatus ret = GetFrame(pts, frame);
    mNextInputPts = pts + 1;
    ANDROID_LOGD("Update next input pts is %d, video id %d", mNextInputPts, mVideoId);
    if(NULL==frame)
    {
        ANDROID_LOGD("VideoDecoder_hw::Frame is empty!");
        return ret;
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

    ANDROID_LOGD("Create output surface decoder manager : set surface at i : %d surface is %p", mVideoId, m_nativeSurface);
    if (IS_DUMPED != 1)
        mDecCtx->mOutputSurface->native_window = ANativeWindow_fromSurface(JNIContext::GetJNIEnv(), (jobject)m_nativeSurface);
    else
        mDecCtx->mOutputSurface->native_window = ANativeWindow_fromSurface(JNIContext::GetJNIEnv(), jsurface);
    return RENDER_STATUS_OK;
}

VCD_NS_END
#endif //_ANDROID_OS_