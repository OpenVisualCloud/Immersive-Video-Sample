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
#include "../RenderType.h"
#include "DashMediaSource.h"
#include "OmafDashAccessApi.h"
#ifdef _USE_TRACE_
#include "../../trace/MtHQ_tp.h"
#endif
#define MAX_LIST_NUMBER 30
#define MIN_LIST_REMAIN 2
#define DECODE_THREAD_COUNT 16
#define MAX_PACKETS 16

VCD_NS_BEGIN

DashMediaSource::DashMediaSource()
{
    pthread_mutex_init(&m_frameMutex, NULL);
    m_status = STATUS_UNKNOWN;
    m_handler = NULL;
    m_bEOS = false;
    m_DecoderManager = NULL;
}

DashMediaSource::~DashMediaSource()
{
    m_status = STATUS_STOPPED;
    SAFE_DELETE(m_DecoderManager);
    OmafAccess_CloseMedia(m_handler);
    OmafAccess_Close(m_handler);
}

RenderStatus DashMediaSource::Initialize(struct RenderConfig renderConfig, RenderSourceFactory *rsFactory)
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
    pCtxDashStreaming->enable_extractor = renderConfig.enableExtractor;
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
    if (ERROR_NONE != OmafAccess_OpenMedia(m_handler, pCtxDashStreaming, renderConfig.enablePredictor, (char *)renderConfig.predictPluginName.c_str(), (char *)renderConfig.libPath.c_str()))
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

    m_bEOS = false;

    m_rsFactory = rsFactory;

    SetMediaInfo(&mediaInfo);

    SAFE_DELETE(m_DecoderManager);
    m_DecoderManager = new DecoderManager();

    RenderStatus ret = m_DecoderManager->Initialize(m_rsFactory);
    if(RENDER_STATUS_OK!=ret){
        LOG(INFO)<<"m_DecoderManager::Initialize failed"<<std::endl;
    }
    m_sourceType = (MediaSourceType::Enum)mediaInfo.streaming_type;
    StartThread();
    m_status = STATUS_CREATED;
    free(pCtxDashStreaming);
    pCtxDashStreaming = NULL;
    free(clientInfo.pose);
    clientInfo.pose = NULL;
    return RENDER_STATUS_OK;
}

void DashMediaSource::SetActiveStream( int32_t video_id, int32_t audio_id )
{
     mMediaInfo.SetActiveAudio(audio_id);
     mMediaInfo.SetActiveVideo(video_id);
}

RenderStatus DashMediaSource::SetMediaInfo(void *mediaInfo)
{
    DashMediaInfo *dashMediaInfo = (DashMediaInfo *)mediaInfo;

    mMediaInfo.mDuration = dashMediaInfo->duration;
    mMediaInfo.mStreamingType = dashMediaInfo->streaming_type;
    VideoInfo vi;
    AudioInfo ai;
    int32_t vidx = 0;
    int32_t aidx = 0;
    for(int i=0; i<dashMediaInfo->stream_count; i++){
        switch(dashMediaInfo->stream_info[i].stream_type ){
        case MediaType_Video:
            vi.streamID        = i;
            vi.bit_rate        = dashMediaInfo->stream_info[i].bit_rate;
            vi.codec           = dashMediaInfo->stream_info[i].codec;
            vi.codec_type      = dashMediaInfo->stream_info[i].codec_type;
            vi.framerate_den   = dashMediaInfo->stream_info[i].framerate_den;
            vi.framerate_num   = dashMediaInfo->stream_info[i].framerate_num;
            vi.height          = dashMediaInfo->stream_info[i].height;
            vi.mFpt            = dashMediaInfo->stream_info[i].mFpt;
            vi.mime_type       = dashMediaInfo->stream_info[i].mime_type;
            vi.mProjFormat     = dashMediaInfo->stream_info[i].mProjFormat;
            vi.width           = dashMediaInfo->stream_info[i].width;
            vi.sourceHighTileRow = dashMediaInfo->stream_info[i].tileRowNum;
            vi.sourceHighTileCol = dashMediaInfo->stream_info[i].tileColNum;
            vi.mPixFmt         = PixelFormat::PIX_FMT_YUV420P;
            mMediaInfo.AddVideoInfo(vidx, vi);
            vidx++;
            break;
        case MediaType_Audio:
            ai.streamID       = i;
            ai.bit_rate       = dashMediaInfo->stream_info[i].bit_rate;
            ai.channel_bytes  = dashMediaInfo->stream_info[i].channel_bytes;
            ai.channels       = dashMediaInfo->stream_info[i].channels;
            ai.codec          = dashMediaInfo->stream_info[i].codec;
            ai.codec_type     = dashMediaInfo->stream_info[i].codec_type;
            ai.mime_type      = dashMediaInfo->stream_info[i].mime_type;
            ai.sample_rate    = dashMediaInfo->stream_info[i].sample_rate;
            mMediaInfo.AddAudioInfo(aidx, ai);
            aidx++;
            break;
        default:
            break;
        }
    }
    mMediaInfo.GetActiveAudioInfo(ai);
    mMediaInfo.GetActiveVideoInfo(vi);

    if(NULL!=this->m_rsFactory){
        m_rsFactory->SetVideoSize(vi.width, vi.height);
        m_rsFactory->SetHighTileRow(vi.sourceHighTileRow);
        m_rsFactory->SetHighTileCol(vi.sourceHighTileCol);
    }
    int32_t frameNum = round(float(mMediaInfo.mDuration) / 1000 * (vi.framerate_num/vi.framerate_den));
    LOG(INFO)<<"------------------------------------------"<<std::endl;
    LOG(INFO)<<"Player [config]: fps               "<<vi.framerate_num/vi.framerate_den<<std::endl;
    //LOG(INFO)<<"Player [config]: render resolution "<<m_mediaSourceInfo.sourceWH->width[0]<<"x"<<m_mediaSourceInfo.sourceWH->height[0]<<std::endl;
    LOG(INFO)<<"Player [config]: packed resolution "<<vi.width<<"x"<<vi.height<<std::endl;
    LOG(INFO)<<"------------------------------------------"<<std::endl;
    //trace
    if (dashMediaInfo->streaming_type != 1 && dashMediaInfo->streaming_type != 2)
    {
        LOG(ERROR)<<"dash mode is invalid!"<<std::endl;
    }
#ifdef _USE_TRACE_
    const char * dash_mode = (dashMediaInfo->streaming_type == 1) ? "static" : "dynamic";
    tracepoint(mthq_tp_provider, stream_information, (char *)dash_mode, dashMediaInfo->stream_info[0].segmentDuration, dashMediaInfo->duration, \
                vi.framerate_num/vi.framerate_den, frameNum, vi.width, vi.height);
#endif
    return RENDER_STATUS_OK;
}

bool DashMediaSource::IsEOS()
{
    if (m_sourceType == 1)//vod
    {
        // return m_bEOS;
        return m_DecoderManager->GetEOS();
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

void DashMediaSource::ProcessVideoPacket()
{
    static int cnt =0;
    VideoInfo vi;
    mMediaInfo.GetActiveVideoInfo(vi);
    //1. get one packet from DashStreaming lib.
    DashPacket dashPkt[MAX_PACKETS];
    memset(dashPkt, 0, MAX_PACKETS * sizeof(DashPacket));
    int dashPktNum = 0;
    static bool needHeaders = true;
    uint64_t pts = 0;
    int ret = OmafAccess_GetPacket(m_handler, vi.streamID, &(dashPkt[0]), &dashPktNum, (uint64_t *)&pts, needHeaders, false);
    if(ERROR_NONE != ret){
        LOG(INFO)<<"Get packet failed: stream_id:" << vi.streamID<<std::endl;
        return;
    }
    if (dashPkt[0].bEOS)
    {
        m_status = STATUS_STOPPED;
    }
    LOG(INFO)<<"Get packet has done! and segment id is "<<dashPkt[0].segID<<std::endl;
#ifdef _USE_TRACE_
    //trace
    tracepoint(mthq_tp_provider, T7_get_packet, dashPkt[0].segID);
#endif

    if(NULL != m_DecoderManager){
        RenderStatus ret = m_DecoderManager->SendVideoPackets(&(dashPkt[0]), dashPktNum);
        // needHeaders = false;
        if(RENDER_STATUS_OK != ret){
            LOG(INFO)<<"m_DecoderManager::SendVideoPackets: stream_id:" << vi.streamID <<" segment id"<<dashPkt[0].segID<<std::endl;
        }
    }

    for(int i=0; i<dashPktNum; i++){
        SAFE_FREE(dashPkt[i].buf);
        if (dashPkt[i].rwpk)
            SAFE_DELETE_ARRAY(dashPkt[i].rwpk->rectRegionPacking);
        SAFE_DELETE(dashPkt[i].rwpk);
        SAFE_DELETE_ARRAY(dashPkt[i].qtyResolution);
    }
}

void DashMediaSource::ProcessAudioPacket()
{
    AudioInfo ai;
    mMediaInfo.GetActiveAudioInfo(ai);
}

void DashMediaSource::Run()
{
    if (NULL == m_handler)
    {
        return ;
    }

    m_status = STATUS_PLAYING;
    while (m_status != STATUS_STOPPED)
    {
        ScopeLock lock(m_Lock);
        ProcessVideoPacket();
        usleep(1000);
    }
}

RenderStatus DashMediaSource::UpdateFrames(uint64_t pts)
{
    if(NULL == m_DecoderManager) return RENDER_NO_MATCHED_DECODER;

    RenderStatus ret = m_DecoderManager->UpdateVideoFrames(pts);

    if(RENDER_STATUS_OK!=ret){
         LOG(INFO)<<"DashMediaSource::UpdateFrames failed with code:" << ret <<std::endl;
    }

    if(RENDER_EOS==ret){
        m_bEOS = true;
    }
    return ret;
}

RenderStatus DashMediaSource::GetFrame(uint8_t **buffer, struct RegionInfo *regionInfo)
{
    *buffer = NULL;
    return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::SeekTo(uint64_t pts)
{
    if(NULL == m_handler) return RENDER_NULL_HANDLE;
    if(NULL == m_DecoderManager) return RENDER_NULL_HANDLE;

    RenderStatus ret = RENDER_STATUS_OK;
    if(mMediaInfo.mStreamingType == 2) //if live mode, pause isn't supported
    {
        ScopeLock lock(m_Lock);
        m_status = STATUS_PAUSED;
        ret = m_DecoderManager->ResetDecoders();
        if(RENDER_STATUS_OK!=ret) return RENDER_SEEK_FAILURE;

        int res = OmafAccess_SeekMedia( m_handler, pts );
        if( ERROR_NONE!=res){
            LOG(INFO)<<"DashMediaSource::SeekTo failed with code:" << res <<std::endl;
            return RENDER_SEEK_FAILURE;
        }
        m_status = STATUS_PLAYING;
    }
    return ret;
}

RenderStatus DashMediaSource::Pause()
{
    if(mMediaInfo.mStreamingType == 2) //if live mode, pause isn't supported
    {
        ScopeLock lock(m_Lock);
        m_status = STATUS_PAUSED;
    }
    return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::Play()
{
    ScopeLock lock(m_Lock);
    m_status = STATUS_PLAYING;
    return RENDER_STATUS_OK;
}

VCD_NS_END
#endif//LOW_LATENCY_USAGE
