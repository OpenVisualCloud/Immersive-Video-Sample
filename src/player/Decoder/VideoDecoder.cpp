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

#include "VideoDecoder.h"
#include "../RegionData.h"
#include <chrono>
#ifdef _USE_TRACE_
#include "../../trace/MtHQ_tp.h"
#endif

#define DECODE_THREAD_COUNT 16
#define MIN_REMAIN_SIZE_IN_FRAME 2

VCD_NS_BEGIN

VideoDecoder::VideoDecoder()
{
    mDecCtx     = new DecoderContext();
    mHandler    = NULL;
    m_status    = STATUS_UNKNOWN;
    mVideoId    = -1;
    mPkt        = NULL;
    mPktInfo    = NULL;
    mRwpk       = NULL;
    mIsFlushed  = false;
}

VideoDecoder::~VideoDecoder()
{
    m_status = STATUS_STOPPED;
    CloseDecoder();
    SAFE_DELETE(mDecCtx);
    if (mPkt)
    {
        av_packet_free(&mPkt);
        mPkt = NULL;
    }
    mPktInfo = NULL;
    mRwpk = NULL;
    mIsFlushed = false;
}

RenderStatus VideoDecoder::Initialize(int32_t id, Codec_Type codec, FrameHandler* handler, uint64_t startPts)
{
    this->mVideoId = id;
    switch(codec){
        case VideoCodec_HEVC:
            mDecCtx->codec_id = AV_CODEC_ID_HEVC;
            break;
        case VideoCodec_AVC:
            mDecCtx->codec_id = AV_CODEC_ID_H264;
            break;
        case VideoCodec_AV1:
            LOG(ERROR)<<"unsupported codec!"<<std::endl;
            return RENDER_UNSUPPORT_DECODER;
        default:
            mDecCtx->codec_id = AV_CODEC_ID_HEVC;
            break;
    }

    mHandler = handler;
    SetStartPts(startPts);
    LOG(INFO) << "Start pts is " << startPts << " video id is " << id << endl;
    return Initialize();
}

RenderStatus VideoDecoder::Initialize()
{
    //5. initial decoder
    // av_register_all();
    // avcodec_register_all();
    mDecCtx->decoder = avcodec_find_decoder(mDecCtx->codec_id);
    if (NULL == mDecCtx->decoder)
    {
        LOG(ERROR)<<"decoder find error!"<<std::endl;
        return RENDER_ERROR;
    }
    mDecCtx->codec_ctx = avcodec_alloc_context3(mDecCtx->decoder);
    if (NULL == mDecCtx->codec_ctx)
    {
        LOG(ERROR)<<"avcodec alloc context failed!"<<std::endl;
        return RENDER_ERROR;
    }
    mDecCtx->codec_ctx->thread_count = DECODE_THREAD_COUNT;
    // mDecCtx->codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    // mDecCtx->codec_ctx->delay = 2;

    if (avcodec_open2(mDecCtx->codec_ctx, mDecCtx->decoder, NULL) < 0)
    {
        LOG(ERROR)<<"avcodec open failed!"<<std::endl;
        return RENDER_ERROR;
    }

    StartThread();
    mIsFlushed = false;
    LOG(INFO) << "A new video decoder is created!" << std::endl;
    return RENDER_STATUS_OK;
}

RenderStatus VideoDecoder::Destroy()
{
    CloseDecoder();
    return RENDER_STATUS_OK;
}

void VideoDecoder::CloseDecoder()
{
    if( (m_status == STATUS_STOPPED) || (m_status == STATUS_IDLE) || m_status == STATUS_PENDING){
        m_status = STATUS_STOPPED;
        LOG(INFO)<<" decoder is closed! video id is " << mVideoId<<endl;
        this->Join();
    }

    if(NULL != mDecCtx->codec_ctx){
        avcodec_close(mDecCtx->codec_ctx);
        mDecCtx->decoder      = NULL;
        mDecCtx->codec_ctx    = NULL;
    }
}

RenderStatus VideoDecoder::Reset(int32_t id, Codec_Type codec, uint64_t startPts)
{
    CloseDecoder();

    return Initialize(id, codec, mHandler, startPts);
}

RenderStatus VideoDecoder::SendPacket(DashPacket* packet)
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

    mPktInfo = new PacketInfo;
    mRwpk = new RegionWisePacking;
    mPkt = av_packet_alloc();
    if (mPktInfo == NULL || mRwpk == NULL || mPkt == NULL)
    {
        LOG(ERROR)<<" alloc memory failed in send packet! " << endl;
        SAFE_DELETE(mPktInfo);
        SAFE_DELETE(mRwpk);
        av_packet_free(&mPkt);
        return RENDER_ERROR;
    }
    mPktInfo->bCodecChange = MediaInfoChange(packet);

    mDecCtx->width = packet->width;
    mDecCtx->height = packet->height;

    //send a packet to AVPACKET list

    if (NULL != packet->buf && packet->size)
    {
        int size = packet->size;
        if (av_new_packet(mPkt, size) < 0)
        {
            SAFE_DELETE(mPktInfo);
            av_packet_free(&mPkt);
            SAFE_DELETE(mRwpk);
            return RENDER_ERROR;
        }
        memcpy_s(mPkt->data, size, packet->buf, size);
        mPkt->size = size;
        *mRwpk = *(packet->rwpk);

        SAFE_FREE(packet->buf);
        SAFE_DELETE(packet->rwpk);

        FrameData* data = new FrameData;
        mPktInfo->pkt = mPkt;
        mPktInfo->bEOS = packet->bEOS;
        mPktInfo->pts = packet->pts;
        mPktInfo->video_id = packet->videoID;
        mDecCtx->push_packet(mPktInfo);
        data->rwpk = mRwpk;
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
        LOG(INFO)<<"frame data fifo size is: "<<mDecCtx->get_size_of_framedata() <<"pts is " << data->pts <<" VIDEO ID : " << mPktInfo->video_id <<endl;
        // SAFE_DELETE_ARRAY(packet->qtyResolution);
    }

    return ret;
}

RenderStatus VideoDecoder::DecodeFrame(AVPacket *pkt, uint32_t video_id)
{
    std::chrono::high_resolution_clock clock;
    uint64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    int32_t ret = 0;
    ret = avcodec_send_packet(mDecCtx->codec_ctx, pkt);
    av_packet_unref(pkt);
    if (ret < 0)
    {
        LOG(ERROR)<<"send packet failed!"<<endl;
        return RENDER_DECODE_FAIL;
    }

    while (ret >= 0){
    AVFrame* av_frame = av_frame_alloc();
    if (NULL == av_frame)
    {
        LOG(ERROR)<<" alloc av frame failed in decode one frame! " << endl;
        return RENDER_DECODE_FAIL;
    }
    ret = avcodec_receive_frame(mDecCtx->codec_ctx, av_frame);
    if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        LOG(WARNING) << "avcodec_receive_frame FAILED video id is " << mVideoId << endl;
        av_frame_free(&av_frame);
        return RENDER_DECODE_FAIL;
    }
    if (av_frame->linesize[0] == 0)
    {
        LOG(INFO)<<"av_frame is null! video_id is "<<video_id<<endl;
        av_frame_free(&av_frame);
        return RENDER_DECODER_INVALID_FRAME;
    }
    FrameData* data = mDecCtx->pop_framedata();
    if (data == NULL)
    {
        LOG(ERROR) << "Frame data is empty!" << endl;
        av_frame_free(&av_frame);
        return RENDER_NO_FRAME;
    }
    DecodedFrame* frame = new DecodedFrame;
    frame->av_frame = av_frame;
    frame->rwpk  = data->rwpk;
    frame->pts = data->pts;
    frame->bFmtChange = data->bCodecChange;
    frame->numQuality = data->numQuality;
    frame->qtyResolution = data->qtyResolution;
    frame->video_id = video_id;
    frame->bEOS = false;
    LOG(INFO)<<"Push one frame at:"<<data->pts<<" video id is:"<<video_id<<endl;
    mDecCtx->push_frame(frame);
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T9_push_frame_to_fifo, data->pts, video_id);
#endif
    //SAFE_DELETE(data->rwpk);
    SAFE_DELETE(data);
    uint64_t end = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<" video id is "<< video_id <<" decode one frame cost time "<<(end-start)<<" ms reso is " << mDecCtx->codec_ctx->width <<" x " <<mDecCtx->codec_ctx->height<<endl;
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T10_decode_time_cost, video_id, end-start, mDecCtx->codec_ctx->width, mDecCtx->codec_ctx->height);
#endif
    }
    return RENDER_STATUS_OK;
}

RenderStatus VideoDecoder::FlushDecoder(uint32_t video_id)
{
    int32_t ret = 0;
    ret = avcodec_send_packet(mDecCtx->codec_ctx, NULL);
    if (ret < 0)
    {
        LOG(ERROR)<<"Send packet failed!"<<endl;
        return RENDER_DECODE_FAIL;
    }
    while(ret >= 0)
    {
        AVFrame* av_frame = av_frame_alloc();
        if (NULL == av_frame)
        {
            LOG(ERROR)<<"alloc av frame failed in flushing frame!" << endl;
            return RENDER_DECODE_FAIL;
        }
        ret = avcodec_receive_frame(mDecCtx->codec_ctx, av_frame);
        if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            LOG(INFO)<<"Receive frame failed!"<<std::endl;
            av_frame_free(&av_frame);
            return RENDER_DECODE_FAIL;
        }
        if (av_frame->linesize[0] == 0)
        {
            LOG(INFO)<<"av_frame is null in flush!"<<endl;
            av_frame_free(&av_frame);
            continue;
        }
        FrameData* data = mDecCtx->pop_framedata();
        if (data == NULL)
        {
            LOG(INFO)<<"Now will end decoder flush!"<<endl;
            return RENDER_STATUS_OK;
        }
        DecodedFrame* frame = new DecodedFrame;
        frame->av_frame = av_frame;
        frame->rwpk  = data->rwpk;
        frame->pts = data->pts;
        frame->bFmtChange = data->bCodecChange;
        frame->numQuality = data->numQuality;
        frame->video_id = video_id;
        frame->qtyResolution = data->qtyResolution;
        if (NULL == mDecCtx->get_front_of_framedata()) // set last frame eos to true
        {
            frame->bEOS = true;
        }else
        {
            frame->bEOS = false;
        }
        LOG(INFO)<<"Push one frame at:"<<data->pts<<" video id is:"<<video_id<<endl;
        mDecCtx->push_frame(frame);
        //SAFE_DELETE(data->rwpk);
        SAFE_DELETE(data);
    }
    return RENDER_STATUS_OK;
}

bool VideoDecoder::MediaInfoChange(DashPacket* packet)
{
    bool bChange = false;
    LOG(INFO)<<"packet has width "<<packet->width << " and height "<<packet->height<<endl;
    if(packet->height != mDecCtx->height && mDecCtx->height != 0){ // not the first time.
        LOG(INFO)<<"height has changed from "<<mDecCtx->height << " to "<<packet->height << endl;
        bChange = true;
        mDecCtx->height = packet->height;
    }
    if(packet->width != mDecCtx->width && mDecCtx->width != 0){
        LOG(INFO)<<"width has changed from "<<mDecCtx->width << " to "<<packet->width << endl;
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

void VideoDecoder::Run()
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

        LOG(INFO)<<"Now packet pts is "<<pkt_info->pts<<"video id is " << mVideoId<<endl;

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


        ret = DecodeFrame(pkt_info->pkt, pkt_info->video_id);
        if(RENDER_STATUS_OK != ret){
             LOG(INFO)<<"Video "<< mVideoId <<": failed to decoder one frame"<<std::endl;
        }

        av_packet_unref(pkt_info->pkt);

        SAFE_DELETE(pkt_info);

    }
}

DecodedFrame* VideoDecoder::GetFrame(uint64_t pts)
{
    DecodedFrame* frame = NULL;
    bool waitFlag = false;
    while(mDecCtx->get_size_of_frame() > 0){
        frame = mDecCtx->get_front_of_frame();
        LOG(INFO)<<"frame size is: " << mDecCtx->get_size_of_frame() << " and frame pts is: "<< frame->pts<<" and input pts is: "<<pts<<" video id is: "<<mVideoId<<endl;
        if(frame->pts == pts)
        {
            frame = mDecCtx->pop_frame();
            LOG(INFO)<<"Pop one frame at:"<<pts<<" video id is:"<<mVideoId<<endl;
            break;
        }
        else if (frame->pts > pts) // wait
        {
            LOG(INFO)<<"Need to wait frame to match current pts! frame->pts " << frame->pts << " pts is " <<pts << " video id is :" <<mVideoId<<endl;
            frame = NULL;
            waitFlag = true;
            break;
        }
        // drop over time frame.
        frame = mDecCtx->pop_frame();
        LOG(INFO)<<"Now will drop one frame since pts is over time! input pts is:" << pts <<" frame pts is:" << frame->pts<<"video id is:" << mVideoId<<endl;
        av_frame_free(&frame->av_frame);
        if (frame->rwpk)
            SAFE_DELETE_ARRAY(frame->rwpk->rectRegionPacking);
        SAFE_DELETE(frame->rwpk);
        SAFE_DELETE_ARRAY(frame->qtyResolution);
        SAFE_DELETE(frame);
    }

    if( !waitFlag && (mDecCtx->get_size_of_frame() == 0) && (m_status==STATUS_PENDING) ){
        LOG(INFO)<<"frame fifo is empty now! video id is : " << mVideoId<<endl;
        m_status = STATUS_IDLE;
        LOG(INFO)<<"decoder status is set to idle!"<<endl;
    }
    return frame;
}

void VideoDecoder::Pending()
{
    mDecCtx->bPacketEOS = true;
    LOG(INFO) << "Set decoder status to PENDING!" << endl;
    m_status = STATUS_PENDING;
}

RenderStatus VideoDecoder::UpdateFrame(uint64_t pts)
{
    std::chrono::high_resolution_clock clock;
    uint64_t start1 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    DecodedFrame* frame = GetFrame(pts);
    uint64_t end1 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<"GetFrame time is:"<<(end1 - start1)<<endl;
    if(NULL==frame)
    {
        LOG(INFO)<<"Frame is empty!"<<endl;
        return RENDER_NO_FRAME;
    }
    if (frame->bEOS)
    {
        this->SetEOS(true);
    }
    if( 0 >= frame->av_frame->linesize[0]){
        av_free(frame->av_frame);
        SAFE_DELETE(frame);
        return RENDER_DECODER_INVALID_FRAME;
    }

    uint64_t start2 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    BufferInfo* buf_info = new BufferInfo;
    uint32_t bufferNumber = 0;

    switch ( mDecCtx->codec_ctx->pix_fmt ){
        case AV_PIX_FMT_YUV420P:
            bufferNumber = 3;
            buf_info->pixelFormat = PixelFormat::PIX_FMT_YUV420P;
            break;
        default:
            bufferNumber = 1;
            buf_info->pixelFormat = PixelFormat::PIX_FMT_RGB24;
            break;
    }

    for (uint32_t idx=0;idx<bufferNumber;idx++)
        buf_info->buffer[idx] = frame->av_frame->data[idx];
    buf_info->bFormatChange = frame->bFmtChange;


    buf_info->width = frame->av_frame->width;
    buf_info->height = frame->av_frame->height;
    for (uint32_t i=0;i<bufferNumber;i++)
    {
        buf_info->stride[i] = frame->av_frame->linesize[i];
    }

    buf_info->regionInfo = new RegionData(frame->rwpk, frame->numQuality, frame->qtyResolution);

    uint64_t end2 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<"Transfer frame time is:"<<(end2 - start2)<<endl;
    uint64_t start3 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    if(NULL != this->mHandler){
        mHandler->process(buf_info);
    }
    uint64_t end3 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<"process time is:"<<(end3 - start3)<<endl;
    uint64_t start4 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();

    // for (uint32_t i = 0; i < bufferNumber; i++){
    //     SAFE_DELETE_ARRAY(buf_info->buffer[i]);
    // }

    SAFE_DELETE(buf_info->regionInfo);
    SAFE_DELETE(buf_info);

    av_frame_free(&frame->av_frame);
    // av_frame_unref(frame->av_frame);
    SAFE_DELETE_ARRAY(frame->rwpk->rectRegionPacking);
    SAFE_DELETE(frame->rwpk);
    SAFE_DELETE_ARRAY(frame->qtyResolution);
    SAFE_DELETE(frame);
    uint64_t end4 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<"delete frame time is:"<<(end4 - start4)<<endl;
    return RENDER_STATUS_OK;
}

RenderStatus VideoDecoder::SetRegionInfo(struct RegionInfo *regionInfo, int32_t nQuality, SourceResolution *qtyRes)
{
    return RENDER_STATUS_OK;
}

bool VideoDecoder::IsReady(uint64_t pts)
{
    LOG(INFO)<<"At first, frame size is"<<mDecCtx->get_size_of_frame()<<" and packet eos is "<< mDecCtx->bPacketEOS << "video id " << mVideoId << " pts : " << pts << endl;
    if (mDecCtx->get_size_of_frame() > MIN_REMAIN_SIZE_IN_FRAME || mDecCtx->bPacketEOS || pts < GetStartPts()){
        return true;
    }else{
        return false;
    }
}

VCD_NS_END