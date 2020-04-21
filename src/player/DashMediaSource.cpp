/*
 * Copyright (c) 2019, Intel Corporation
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

//!
//! \file     DashMediaSource.cpp
//! \brief    Implement class for DashMediaSource.
//!
#ifndef _ENABLE_WEBRTC_SOURCE_
#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include "RenderType.h"
#include "DashMediaSource.h"
#include "OmafDashAccessApi.h"
#include "../trace/MtHQ_tp.h"

#define MAX_LIST_NUMBER 30
#define MIN_LIST_REMAIN 2

VCD_NS_BEGIN

DashMediaSource::DashMediaSource()
{
    pthread_mutex_init(&m_frameMutex, NULL);
    m_status = STATUS_UNKNOWN;
    InitializeDashSourceData();
    m_handler = NULL;

}

DashMediaSource::~DashMediaSource()
{
    m_status = STATUS_STOPPED;
    this->Join();
    int32_t res = pthread_mutex_destroy(&m_frameMutex);
    if (res != 0) {return;}
    if (!m_frameBuffer.empty())
    {
        for (auto &fb : m_frameBuffer)
        {
            delete fb;
            fb = NULL;
        }
    }
    if (!m_rwpkList.empty())
    {
        for (auto rwpk : m_rwpkList)
        {
            if (rwpk.rectRegionPacking != NULL)
            {
                delete rwpk.rectRegionPacking;
                rwpk.rectRegionPacking = NULL;
            }
        }
    }
    ClearDashSourceData();
    OmafAccess_CloseMedia(m_handler);
    OmafAccess_Close(m_handler);
}

RenderStatus DashMediaSource::GetPacket(AVPacket *pkt, RegionWisePacking *rwpk)
{
    if (NULL == m_handler)
    {
        return RENDER_ERROR;
    }
    uint32_t streamID = 0;
    //1. get one packet from DashStreaming lib.
    DashPacket dashPkt[5];
    memset(dashPkt, 0, 5 * sizeof(DashPacket));
    int dashPktNum = 0;
    static bool needHeaders = true;
    if (ERROR_NONE != OmafAccess_GetPacket(m_handler, streamID, &(dashPkt[0]), &dashPktNum, (uint64_t *)&(pkt->pts), needHeaders, false))//lack of rwpk
    {
        return RENDER_ERROR;
    }
    LOG(INFO)<<"Get packet has done! and segment id is "<<dashPkt[0].segID<<std::endl;
    //trace
    tracepoint(mthq_tp_provider, T7_get_packet, dashPkt[0].segID);
    if (NULL != dashPkt[0].buf && dashPkt[0].size && dashPktNum != 0)
    {
        int size = dashPkt[0].size;
        if (av_new_packet(pkt, size) < 0)
        {
            return RENDER_ERROR;
        }
        memcpy(pkt->data, dashPkt[0].buf, size);
        pkt->size = size;
        *rwpk = *(dashPkt[0].rwpk);
        free(dashPkt[0].buf);
        dashPkt[0].buf = NULL;
        delete dashPkt[0].rwpk;
        dashPkt[0].rwpk = NULL;
        if (needHeaders)
        {
            needHeaders = false;
        }
        m_mediaSourceInfo.currentFrameNum++;
        LOG(INFO)<<"-=-=-Get packet number-=-=-"<<m_mediaSourceInfo.currentFrameNum<<std::endl;
    }
    //get rwpk and region information. dash lib has filled it.
    return RENDER_STATUS_OK;
}

DecoderStatus DashMediaSource::GetOneFrame(uint8_t **buffer)
{
    //1. decode one packet to a av_frame.
    int32_t ret;
    ret = avcodec_send_packet(m_dashSourceData.codec_ctx, m_dashSourceData.packet);
    // av_free_packet(m_dashSourceData.packet);
    av_packet_unref(m_dashSourceData.packet);
    if (ret < 0)
    {
        return PACKET_ERROR;
    }
    ret = avcodec_receive_frame(m_dashSourceData.codec_ctx, m_dashSourceData.av_frame);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
    {
        av_frame_free(&m_dashSourceData.av_frame);
        return FRAME_ERROR;
    }
    //2. transfer av_frame to buffer.
    if (ret >= 0 && m_dashSourceData.av_frame->linesize[0])
    {
        uint32_t bufferNumber = 0;
        switch (m_mediaSourceInfo.pixFormat)
        {
        case PixelFormat::PIX_FMT_RGB24:
            bufferNumber = 1;
            break;
        case PixelFormat::PIX_FMT_YUV420P:
            bufferNumber = 3;
            break;
        }
        for (uint32_t i = 0; i < bufferNumber; i++)
        {
            if (m_dashSourceData.av_frame->linesize[i] > 0)
            {
                int factor = i==0? 1:4;
                buffer[i] = new uint8_t[(m_dashSourceData.codec_ctx->width * m_dashSourceData.codec_ctx->height)/factor];
            }
        }
        m_mediaSourceInfo.stride = m_dashSourceData.av_frame->linesize[0];
        isAllValid = true;
        switch (m_mediaSourceInfo.pixFormat)
        {
        case PixelFormat::PIX_FMT_RGB24:
            memcpy(buffer[0], m_dashSourceData.av_frame->data[0], m_dashSourceData.av_frame->linesize[0] * m_dashSourceData.codec_ctx->height * 3);
            break;
        case PixelFormat::PIX_FMT_YUV420P:
            for (uint32_t j=0;j<bufferNumber;j++)
            {
                uint32_t factor = j==0 ? 1:2;
                for (uint32_t i=0;i<(m_dashSourceData.codec_ctx->height)/factor;i++)
                {
                    memcpy(buffer[j] + i * (m_dashSourceData.codec_ctx->width)/factor, m_dashSourceData.av_frame->data[j] + i * m_dashSourceData.av_frame->linesize[j], (m_dashSourceData.codec_ctx->width)/factor);
                }
            }
            break;
        default:
            break;
        }
        return DECODER_OK;
    }
    else
    {
        return FRAME_ERROR;
    }
}

DecoderStatus DashMediaSource::FlushFrames(uint8_t **buffer)
{
    int32_t ret = 0;
    ret = avcodec_receive_frame(m_dashSourceData.codec_ctx, m_dashSourceData.av_frame);
    if (ret < 0)
    {
        return FRAME_ERROR;
    }
        //2. transfer av_frame to buffer.
    if (ret >= 0 && m_dashSourceData.av_frame->linesize[0])
    {
        uint32_t bufferNumber = 0;
        switch (m_mediaSourceInfo.pixFormat)
        {
        case PixelFormat::PIX_FMT_RGB24:
            bufferNumber = 1;
            break;
        case PixelFormat::PIX_FMT_YUV420P:
            bufferNumber = 3;
            break;
        }
        for (uint32_t i = 0; i < bufferNumber; i++)
        {
            if (m_dashSourceData.av_frame->linesize[i] > 0)
            {
                uint32_t factor = i==0? 1:4;
                buffer[i] = new uint8_t[(m_dashSourceData.codec_ctx->width * m_dashSourceData.codec_ctx->height)/factor];
            }
        }
        m_mediaSourceInfo.stride = m_dashSourceData.av_frame->linesize[0];
        isAllValid = true;
        switch (m_mediaSourceInfo.pixFormat)
        {
        case PixelFormat::PIX_FMT_RGB24:
            memcpy(buffer[0], m_dashSourceData.av_frame->data[0], m_dashSourceData.av_frame->linesize[0] * m_dashSourceData.codec_ctx->height * 3);
            break;
        case PixelFormat::PIX_FMT_YUV420P:
            for (uint32_t j=0;j<bufferNumber;j++)
            {
                uint32_t factor = j==0 ? 1:2;
                for (uint32_t i=0;i<(m_dashSourceData.codec_ctx->height)/factor;i++)
                {
                    memcpy(buffer[j] + i * (m_dashSourceData.codec_ctx->width)/factor, m_dashSourceData.av_frame->data[j] + i * m_dashSourceData.av_frame->linesize[j], (m_dashSourceData.codec_ctx->width)/factor);
                }
            }
            break;
        default:
            break;
        }
        return DECODER_OK;
    }
    else
    {
        return FRAME_ERROR;
    }
    return DECODER_OK;
}

RenderStatus DashMediaSource::SetRegionInfo(struct RegionInfo *regionInfo)
{
    if (NULL == regionInfo)
    {
        return RENDER_ERROR;
    }
    regionInfo->sourceNumber = m_mediaSourceInfo.sourceNumber;
    regionInfo->sourceInfo = (struct SourceInfo *)malloc(sizeof(struct SourceInfo) * regionInfo->sourceNumber);
    if (NULL == regionInfo->sourceInfo)
    {
        return RENDER_ERROR;
    }
    regionInfo->sourceInfo[0].sourceWidth = regionInfo->regionWisePacking->projPicWidth;
    regionInfo->sourceInfo[0].sourceHeight = regionInfo->regionWisePacking->projPicHeight;
    regionInfo->sourceInfo[0].tileColumnNumber = regionInfo->sourceInfo[0].sourceWidth / regionInfo->regionWisePacking->rectRegionPacking[0].projRegWidth;
    regionInfo->sourceInfo[0].tileRowNumber = regionInfo->sourceInfo[0].sourceHeight / regionInfo->regionWisePacking->rectRegionPacking[0].projRegHeight;

    regionInfo->sourceInfo[1].sourceWidth = m_mediaSourceInfo.sourceWH->width[1];
    regionInfo->sourceInfo[1].sourceHeight = m_mediaSourceInfo.sourceWH->height[1];
    regionInfo->sourceInfo[1].tileColumnNumber = regionInfo->sourceInfo[1].sourceWidth / regionInfo->regionWisePacking->rectRegionPacking[regionInfo->regionWisePacking->numRegions - 1].projRegWidth;
    regionInfo->sourceInfo[1].tileRowNumber = regionInfo->sourceInfo[1].sourceHeight / regionInfo->regionWisePacking->rectRegionPacking[regionInfo->regionWisePacking->numRegions - 1].projRegHeight;

    return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::ClearDashSourceData()
{
    if (m_dashSourceData.codec_ctx)
        avcodec_close(m_dashSourceData.codec_ctx);
    if (m_dashSourceData.av_frame)
        av_free(m_dashSourceData.av_frame);
    if (m_dashSourceData.gl_frame)
        av_free(m_dashSourceData.gl_frame);
    InitializeDashSourceData();
    return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::InitializeDashSourceData()
{
    m_dashSourceData.codec_ctx    = NULL;
    m_dashSourceData.decoder      = NULL;
    m_dashSourceData.av_frame     = NULL;
    m_dashSourceData.gl_frame     = NULL;
    m_dashSourceData.conv_ctx     = NULL;
    m_dashSourceData.fmt_ctx      = NULL;
    m_dashSourceData.packet       = NULL;
    m_dashSourceData.stream_idx   = -1;
    m_dashSourceData.video_stream = NULL;
    return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::Initialize(struct RenderConfig renderConfig)
{
    if (NULL == renderConfig.url)
    {
        return RENDER_ERROR;
    }
    //1.initial DashStreaming
    DashStreamingClient *pCtxDashStreaming = (DashStreamingClient *)malloc(sizeof(DashStreamingClient));
    if (NULL == pCtxDashStreaming)
    {
        return RENDER_ERROR;
    }
    pCtxDashStreaming->media_url = renderConfig.url;
    pCtxDashStreaming->cache_path = renderConfig.cachePath;
    pCtxDashStreaming->source_type = MultiResSource;
    m_handler = OmafAccess_Init(pCtxDashStreaming);
    if (NULL == m_handler)
    {
        LOG(ERROR)<<"handler init failed!"<<std::endl;
        free(pCtxDashStreaming);
        pCtxDashStreaming = NULL;
        return RENDER_ERROR;
    }
    //2. initial viewport.
    HeadSetInfo clientInfo;
    clientInfo.input_geoType = E_SVIDEO_EQUIRECT;
    clientInfo.output_geoType = E_SVIDEO_VIEWPORT;
    clientInfo.pose = (HeadPose*)malloc(sizeof(HeadPose));
    if (NULL == clientInfo.pose)
    {
        LOG(ERROR)<<"client info malloc failed!"<<std::endl;
        free(pCtxDashStreaming);
        pCtxDashStreaming = NULL;
        return RENDER_ERROR;
    }
    clientInfo.pose->yaw = 0;
    clientInfo.pose->pitch = 0;
    clientInfo.viewPort_hFOV = renderConfig.viewportHFOV;
    clientInfo.viewPort_vFOV = renderConfig.viewportVFOV;
    clientInfo.viewPort_Width = renderConfig.viewportWidth;
    clientInfo.viewPort_Height = renderConfig.viewportHeight;
    OmafAccess_SetupHeadSetInfo(m_handler, &clientInfo);
    //3.load media source
    if (ERROR_NONE != OmafAccess_OpenMedia(m_handler, pCtxDashStreaming, false))
    {
        LOG(ERROR)<<"Open media failed!"<<std::endl;
        free(pCtxDashStreaming);
        pCtxDashStreaming = NULL;
        free(clientInfo.pose);
        clientInfo.pose = NULL;
        return RENDER_ERROR;
    }
    //4. add extra information in mediaInfo for render.
    DashMediaInfo mediaInfo;
    OmafAccess_GetMediaInfo(m_handler, &mediaInfo);
    if(renderConfig.decoderType == VAAPI_DECODER) // HW decoder
    {
        m_mediaSourceInfo.pixFormat = PixelFormat::AV_PIX_FMT_NV12;
    }
    else if (renderConfig.decoderType == SW_DECODER) //SW decoder
    {
        m_mediaSourceInfo.pixFormat = PixelFormat::PIX_FMT_YUV420P;
    }
    SetMediaSourceInfo(&mediaInfo);
    //5. initial decoder
    // av_register_all();
    // avcodec_register_all();
    m_dashSourceData.packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    m_dashSourceData.decoder = avcodec_find_decoder(AV_CODEC_ID_HEVC);
    if (NULL == m_dashSourceData.decoder)
    {
        LOG(ERROR)<<"decoder find error!"<<std::endl;
        free(pCtxDashStreaming);
        pCtxDashStreaming = NULL;
        free(clientInfo.pose);
        clientInfo.pose = NULL;
        return RENDER_ERROR;
    }
    m_dashSourceData.codec_ctx = avcodec_alloc_context3(m_dashSourceData.decoder);
    if (NULL == m_dashSourceData.codec_ctx)
    {
        LOG(ERROR)<<"avcodec alloc context failed!"<<std::endl;
        free(pCtxDashStreaming);
        pCtxDashStreaming = NULL;
        free(clientInfo.pose);
        clientInfo.pose = NULL;
        return RENDER_ERROR;
    }
    m_dashSourceData.av_frame = av_frame_alloc();
    // may not available in bitstream.
    // m_dashSourceData.codec_ctx->width = mediaInfo->stream_info[0].width;
    // m_dashSourceData.codec_ctx->height = mediaInfo->stream_info[0].height;
    if (avcodec_open2(m_dashSourceData.codec_ctx, m_dashSourceData.decoder, NULL) < 0)
    {
        LOG(ERROR)<<"avcodec open failed!"<<std::endl;
        free(pCtxDashStreaming);
        pCtxDashStreaming = NULL;
        free(clientInfo.pose);
        clientInfo.pose = NULL;
        return RENDER_ERROR;
    }
    //6. set source type
    m_sourceType = (MediaSourceType::Enum)mediaInfo.streaming_type;
    //7. start thread
    StartThread();
    m_status = STATUS_CREATED;
    free(pCtxDashStreaming);
    pCtxDashStreaming = NULL;
    free(clientInfo.pose);
    clientInfo.pose = NULL;
    return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::SetMediaSourceInfo(void *mediaInfo)
{
    DashMediaInfo *dashMediaInfo = (DashMediaInfo *)mediaInfo;
    m_mediaSourceInfo.duration = dashMediaInfo->duration;
    m_mediaSourceInfo.frameRate = dashMediaInfo->stream_info[0].framerate_num / dashMediaInfo->stream_info[0].framerate_den;
    m_mediaSourceInfo.frameNum = round(float(m_mediaSourceInfo.duration) / 1000 * m_mediaSourceInfo.frameRate);
    m_mediaSourceInfo.hasAudio = dashMediaInfo->streaming_type == MediaType_Audio;
    m_mediaSourceInfo.width = dashMediaInfo->stream_info[0].width;//packed
    m_mediaSourceInfo.height = dashMediaInfo->stream_info[0].height;
    m_mediaSourceInfo.numberOfStreams = dashMediaInfo->stream_count;
    // should get from DashAccess lib.
    m_mediaSourceInfo.sourceWH = new SourceWH;
    int32_t sourceNumber = dashMediaInfo->stream_info[0].source_number;
    m_mediaSourceInfo.sourceWH->width  = new uint32_t[sourceNumber];
    m_mediaSourceInfo.sourceWH->height = new uint32_t[sourceNumber];
    memset(m_mediaSourceInfo.sourceWH->width, 0, sizeof(uint32_t) * sourceNumber);
    memset(m_mediaSourceInfo.sourceWH->height, 0, sizeof(uint32_t) * sourceNumber);
    for (int i=0;i<sourceNumber;i++)
    {
        int32_t qualityRanking = dashMediaInfo->stream_info[0].source_resolution[i].qualityRanking;
        m_mediaSourceInfo.sourceWH->width[qualityRanking - 1]  = dashMediaInfo->stream_info[0].source_resolution[i].width;
        m_mediaSourceInfo.sourceWH->height[qualityRanking - 1] = dashMediaInfo->stream_info[0].source_resolution[i].height;
    }
    m_mediaSourceInfo.sourceNumber = sourceNumber;
    m_mediaSourceInfo.stride = dashMediaInfo->stream_info[0].width;
    m_mediaSourceInfo.projFormat = dashMediaInfo->stream_info[0].mProjFormat;
    LOG(INFO)<<"------------------------------------------"<<std::endl;
    LOG(INFO)<<"Player [config]: fps               "<<m_mediaSourceInfo.frameRate<<std::endl;
    LOG(INFO)<<"Player [config]: render resolution "<<m_mediaSourceInfo.sourceWH->width[0]<<"x"<<m_mediaSourceInfo.sourceWH->height[0]<<std::endl;
    LOG(INFO)<<"Player [config]: packed resolution "<<m_mediaSourceInfo.width<<"x"<<m_mediaSourceInfo.height<<std::endl;
    LOG(INFO)<<"------------------------------------------"<<std::endl;
    std::cout<<"--Please press 's' key to start--"<<std::endl;
    //trace
    if (dashMediaInfo->streaming_type != 1 && dashMediaInfo->streaming_type != 2)
    {
        LOG(ERROR)<<"dash mode is invalid!"<<std::endl;
    }
    const char * dash_mode = (dashMediaInfo->streaming_type == 1) ? "static" : "dynamic";
    tracepoint(mthq_tp_provider, stream_information, dash_mode, dashMediaInfo->stream_info[0].segmentDuration, dashMediaInfo->duration, \
                m_mediaSourceInfo.frameRate, m_mediaSourceInfo.frameNum, m_mediaSourceInfo.sourceWH->width[0], m_mediaSourceInfo.sourceWH->height[0]);
    return RENDER_STATUS_OK;
}

struct MediaSourceInfo DashMediaSource::GetMediaSourceInfo()
{
    return m_mediaSourceInfo;
}

void *DashMediaSource::GetSourceMetaData()
{
    return &m_dashSourceData;
}

bool DashMediaSource::IsEOS()
{
    if (m_sourceType == 1)//vod
    {
        int32_t res = pthread_mutex_lock(&m_frameMutex);
        if (res != 0){return false;}
        if (m_mediaSourceInfo.currentFrameNum == m_mediaSourceInfo.frameNum && m_frameBuffer.size() == 0)
        {
            cout<<"Totally "<<m_mediaSourceInfo.frameNum<<" frames! "<<"End the player!"<<endl;
            res = pthread_mutex_unlock(&m_frameMutex);
            if (res != 0) {return false;}
            return true;
        }
        res = pthread_mutex_unlock(&m_frameMutex);
        if (res != 0) {return false;}
    }
    return false;//vod return false or live always false
}

RenderStatus DashMediaSource::ChangeViewport(float yaw, float pitch)
{
    HeadPose pose;
    pose.yaw = yaw;
    pose.pitch = pitch;
    OmafAccess_ChangeViewport(m_handler, &pose);
    return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::GetFrame(uint8_t **buffer, struct RegionInfo *regionInfo)
{
    static uint32_t cnt = 0;
    int32_t res = pthread_mutex_lock(&m_frameMutex);
    if (res != 0)
    {
        return RENDER_ERROR;
    }
    if (m_frameBuffer.size() == 0)
    {
        int32_t res = pthread_mutex_unlock(&m_frameMutex);
        if (res != 0) return RENDER_ERROR;
        return RENDER_ERROR;
    }
    struct FrameInfo *p = m_frameBuffer.front();
    m_frameBuffer.pop_front();
    LOG(INFO)<<"======player fifo remaining number:=============:"<<m_frameBuffer.size()<<std::endl;
    LOG(INFO)<<"====Get Frame number-"<<++cnt<<"===="<<std::endl;
    pthread_mutex_unlock(&m_frameMutex);
    for (int i=0;i<4;i++)
    {
        buffer[i] = p->mBuffer[i];
    }
    *regionInfo = *p->mRegionInfo;
    delete p;
    p = NULL;
    return RENDER_STATUS_OK;
}

void DashMediaSource::Run()
{
    m_status = STATUS_RUNNING;
    while (m_status != STATUS_STOPPED)
    {
        //vod && full
        pthread_mutex_lock(&m_frameMutex);
        if (m_sourceType == 1 && m_frameBuffer.size() >= MAX_LIST_NUMBER)
        {
            pthread_mutex_unlock(&m_frameMutex);
            continue;
        }
        pthread_mutex_unlock(&m_frameMutex);
        //1.get packet and decode frame from m_mediaSource
        if ((m_sourceType == 1 && m_mediaSourceInfo.frameNum != m_mediaSourceInfo.currentFrameNum) || m_sourceType == 2)// vod and getpacket not over || live
        {
            RegionWisePacking rwpk;
            if (RENDER_STATUS_OK != GetPacket(m_dashSourceData.packet, &rwpk))
            {
                // av_free_packet(m_dashSourceData.packet);
                continue;
            }
            if (rwpk.rectRegionPacking != NULL)//just for DASH Source.
            {
                m_rwpkList.push_back(rwpk);// keep the correct order of rwpk.
            }
        }
        
        uint8_t **buffer = new uint8_t*[4];
        struct RegionInfo *regionInfo = new struct RegionInfo;

        DecoderStatus decoderStatus = GetOneFrame(&buffer[0]);
        if (FRAME_ERROR == decoderStatus)
        {
            // av_free_packet(m_dashSourceData.packet);
            av_packet_unref(m_dashSourceData.packet);
            delete []buffer;
            buffer = NULL;
            delete regionInfo;
            regionInfo = NULL;
            continue;
        }
        else if (PACKET_ERROR == decoderStatus)
        {
            if (FRAME_ERROR == FlushFrames(buffer)) // flush end
            {
                if (buffer != NULL)
                {
                    delete []buffer;
                    buffer = NULL;
                }
                if (regionInfo != NULL)
                {
                    delete regionInfo;
                    regionInfo = NULL;
                }
                m_status = STATUS_STOPPED;
                break;
            }
        }
        // av_free_packet(m_dashSourceData.packet);
        av_packet_unref(m_dashSourceData.packet);
        LOG(INFO)<<"rwpkList.size: "<<m_rwpkList.size()<<std::endl;
        static int trace_flag = 1;
        if (trace_flag)
        {
            //trace first occurs
            tracepoint(mthq_tp_provider, decode_fifo, m_rwpkList.size());
            trace_flag = 0;
        }
        if (0 != m_rwpkList.size())//just for DASH Source.
        {
            regionInfo->regionWisePacking = new RegionWisePacking;
            *regionInfo->regionWisePacking = m_rwpkList.front();
            m_rwpkList.erase(m_rwpkList.begin());
        }
        SetRegionInfo(regionInfo);

        struct FrameInfo *frameInfo = new struct FrameInfo;
        if (NULL == frameInfo)
        {
            DeleteBuffer(buffer);
            if (regionInfo != NULL)
            {
                ClearRWPK(regionInfo->regionWisePacking);
                delete regionInfo;
                regionInfo = NULL;
            }
            return;
        }
        frameInfo->mBuffer = buffer;
        frameInfo->mRegionInfo = regionInfo;
        //vod && not full
        int32_t res = pthread_mutex_lock(&m_frameMutex);
        if (res != 0)
        {
            DeleteBuffer(buffer);
            if (regionInfo != NULL)
            {
                ClearRWPK(regionInfo->regionWisePacking);
                delete regionInfo;
                regionInfo = NULL;
            }
            delete frameInfo;
            frameInfo = NULL;
            return;
        }
        if (m_sourceType == 1)
        {
            m_frameBuffer.push_back(frameInfo);
        }
        //live
        else if (m_sourceType == 2)
        {
            //live && not full
            if (m_frameBuffer.size() < MAX_LIST_NUMBER)
            {
                m_frameBuffer.push_back(frameInfo);
                LOG(INFO)<<"=========not full========="<<std::endl;
            }
            //live && full
            else
            {
                struct FrameInfo *p = m_frameBuffer.front();
                m_frameBuffer.pop_front();
                ClearPackedData(p);
                m_frameBuffer.push_back(frameInfo);
                LOG(INFO)<<"!!!!!=========full===========!!!!!"<<std::endl;
            }
        }
        //trace
        tracepoint(mthq_tp_provider, T8_decode_finish, m_frameBuffer.size());
        LOG(INFO)<<"======push_back frameBuffer size:=============:"<<m_frameBuffer.size()<<std::endl;
        pthread_mutex_unlock(&m_frameMutex);
    }
}

void DashMediaSource::DeleteBuffer(uint8_t **buffer)
{
    uint32_t bufferNumber = 0;
    switch (m_mediaSourceInfo.pixFormat)
    {
    case PixelFormat::PIX_FMT_RGB24:
        bufferNumber = 1;
        break;
    case PixelFormat::PIX_FMT_YUV420P:
        bufferNumber = 3;
        break;
    }
    if (buffer != NULL)
    {
        for (uint32_t i=0;i<bufferNumber;i++)
        {
            if (buffer[i] != NULL)
            {
                delete buffer[i];
                buffer[i] = NULL;
            }
        }
    }
}

void DashMediaSource::ClearRWPK(RegionWisePacking *rwpk)
{
    if (rwpk != NULL)
    {
        if (rwpk->rectRegionPacking != NULL)
        {
            delete rwpk->rectRegionPacking;
            rwpk->rectRegionPacking = NULL;
        }
        delete rwpk;
        rwpk = NULL;
    }
}

RenderStatus DashMediaSource::ClearPackedData(struct FrameInfo *frameInfo)
{
    DeleteBuffer(frameInfo->mBuffer);
    if (frameInfo->mRegionInfo != NULL)
    {
        if (frameInfo->mRegionInfo->regionWisePacking != NULL)
        {
            if (frameInfo->mRegionInfo->regionWisePacking->rectRegionPacking != NULL)
            {
                delete [] frameInfo->mRegionInfo->regionWisePacking->rectRegionPacking;
                frameInfo->mRegionInfo->regionWisePacking->rectRegionPacking = NULL;
            }
            delete frameInfo->mRegionInfo->regionWisePacking;
            frameInfo->mRegionInfo->regionWisePacking = NULL;
        }
        delete frameInfo->mRegionInfo;
        frameInfo->mRegionInfo = NULL;
    }
    return RENDER_STATUS_OK;
}

VCD_NS_END
#endif//LOW_LATENCY_USAGE
