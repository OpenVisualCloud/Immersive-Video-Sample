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

//!
//! \file     DecoderManager.cpp
//! \brief    Implement class for DecoderManager.
//!

#include <stdio.h>
#include "DecoderManager.h"
#include "VideoDecoder.h"
#include "VideoDecoder_hw.h"
#include "AudioDecoder.h"
#ifndef _ANDROID_OS_
#ifdef _USE_TRACE_
#include "../../../trace/MtHQ_tp.h"
#endif
#endif
VCD_NS_BEGIN

#define MAX_DECODER_NUM 10

DecoderManager::DecoderManager()
{
    m_handlerFactory = NULL;
    this->m_mapAudioDecoder.clear();
    this->m_mapVideoDecoder.clear();
    this->m_mapCatchupVideoDecoder.clear();
    m_surfaces.resize(MAX_DECODER_NUM);
    m_textures.resize(MAX_DECODER_NUM);
    memset_s(&m_decodeInfo, sizeof(m_decodeInfo), 0);
}

DecoderManager::~DecoderManager()
{
    for (auto it=m_mapVideoDecoder.begin();it!=m_mapVideoDecoder.end();it++)
    {
        SAFE_DELETE(it->second);
    }
    for (auto it=m_mapAudioDecoder.begin();it!=m_mapAudioDecoder.end();it++)
    {
        SAFE_DELETE(it->second);
    }
    for (auto it=m_mapCatchupVideoDecoder.begin();it!=m_mapCatchupVideoDecoder.end();it++)
    {
        SAFE_DELETE(it->second);
    }
}

RenderStatus DecoderManager::Initialize(FrameHandlerFactory* factory)
{
    if (NULL == factory)
    {
        return RENDER_ERROR;
    }
    this->m_handlerFactory = factory;
    return RENDER_STATUS_OK;
}

///Video-relative operations
RenderStatus DecoderManager::CreateVideoDecoder(uint32_t video_id, Codec_Type video_codec, uint64_t startPts)
{
#ifdef _LINUX_OS_
    VideoDecoder* pDecoder = new VideoDecoder();
#endif
#ifdef _ANDROID_OS_
    VideoDecoder_hw* pDecoder = new VideoDecoder_hw();
#endif
    pDecoder->SetSurface(m_surfaces[video_id]);
    pDecoder->SetDecodeInfo(m_decodeInfo);
    RenderStatus ret = pDecoder->Initialize(video_id, video_codec, m_handlerFactory->CreateHandler(video_id, m_textures[video_id]), startPts);
    if( RENDER_STATUS_OK != ret ){
        SAFE_DELETE(pDecoder);
        return ret;
    }
    if (video_id < OFFSET_VIDEO_ID_FOR_CATCHUP) {
        m_mapVideoDecoder[video_id] = pDecoder;
    }
    else {
        m_mapCatchupVideoDecoder[video_id] = pDecoder;
    }

#ifdef _ANDROID_OS_
    ANDROID_LOGD("Create decoder decoder manager : set surface at i : %d surface is %p", m_textures[video_id], m_surfaces[video_id]);
#endif
    return RENDER_STATUS_OK;
}

RenderStatus DecoderManager::CheckVideoDecoders(vector<DashPacket*> packets, std::map<uint32_t, MediaDecoder*> decoderMap, uint32_t cnt, bool isCatchup)
{
    RenderStatus ret = RENDER_STATUS_OK;
    // condition 1: pending decoders
    if (!isCatchup) {
        uint32_t currentDecoderSize = decoderMap.size();
        if (cnt < currentDecoderSize && currentDecoderSize != 0)
        {
            LOG(INFO)<<currentDecoderSize-cnt<<" decoders are destroyed"<<endl;
            vector<int32_t> lossID;
            for(auto it=decoderMap.begin(); it!=decoderMap.end();it++){
                bool isFound = false;
                for (uint32_t i=0;i<cnt;i++)
                {
                    if (packets[i]->videoID == it->first)
                    {
                        isFound = true;
                        break;
                    }
                }
                if (!isFound)
                    lossID.push_back(it->first);
            }
            if (!lossID.empty())
            {
                for (int32_t id : lossID)
                {
                    auto it = decoderMap[id];
                    if (it->GetDecoderStatus() != STATUS_IDLE && it->GetDecoderStatus() != STATUS_PENDING)
                    {
                        it->Pending();
                        LOG(INFO)<<" Decoder "<< id << " status is set to pending!" << endl;
                    }
                }
            }
        }
    }

    // condtion 2: create decoders
    for(int i=0; i<cnt; i++){
        /// no video decoder relative to the packet; create one
        if(decoderMap.find(packets[i]->videoID)==decoderMap.end()){
            ret = CreateVideoDecoder(packets[i]->videoID, packets[i]->video_codec, packets[i]->pts);
            if(RENDER_STATUS_OK!=ret){
                LOG(ERROR)<<"Video "<< packets[i]->videoID <<" : Failed to create a decoder for it"<<std::endl;
                break;
            }
        } // idle status -> running status
        else if (decoderMap[packets[i]->videoID]->GetDecoderStatus() == STATUS_IDLE || decoderMap[packets[i]->videoID]->GetDecoderStatus() == STATUS_PENDING)
        {
            LOG(INFO) << "Reset " <<decoderMap[packets[i]->videoID]->GetDecoderStatus() <<" status to RUNNING!" << endl;
            decoderMap[packets[i]->videoID]->Reset(packets[i]->videoID, packets[i]->video_codec, packets[i]->pts);
        }
    }
    // condition 3: check EOS
    if (!isCatchup && !packets.empty() && packets[0] != nullptr && packets[0]->bEOS)
    {
        for (int i=0; i<cnt; i++)
        {
            packets[i]->bEOS = true;
            LOG(INFO) << "Set packet eos is true! pts " << packets[i]->pts << " video id " << packets[i]->videoID << endl;
        }
    }
    return ret;
}

RenderStatus DecoderManager::SendVideoPackets( DashPacket* packets, uint32_t cnt )
{
    RenderStatus ret = RENDER_STATUS_OK;

    //1. separate normal packets and catch-up packets
    vector<DashPacket*> normalPackets, catchupPackets;
    for (uint32_t i = 0; i < cnt; i++) {
        if (packets[i].bCatchup) {
            catchupPackets.push_back(&packets[i]);
        }
        else {
            normalPackets.push_back(&packets[i]);
        }
    }

    //2. check if video decoders status is changed
    if (!normalPackets.empty()) {
        ScopeLock lock(m_mapDecoderLock);
        ret = CheckVideoDecoders(normalPackets, m_mapVideoDecoder, normalPackets.size(), false);
        if(RENDER_STATUS_OK!=ret) return ret;
    }
    if (!catchupPackets.empty()) {
        ScopeLock lock(m_mapCatchupDecoderLock);
        ret = CheckVideoDecoders(catchupPackets, m_mapCatchupVideoDecoder, catchupPackets.size(), true);
        if(RENDER_STATUS_OK!=ret) return ret;
    }

    //3. send packets
    for(int i=0; i<normalPackets.size(); i++){
        ScopeLock lock(m_mapDecoderLock);
        m_mapVideoDecoder[normalPackets[i]->videoID]->SendPacket(normalPackets[i]);
        LOG(INFO)<<"send packet to video "<<normalPackets[i]->videoID<<" and pts is : "<<normalPackets[i]->pts<<endl;
    }
    for(int i=0; i<catchupPackets.size(); i++){
        ScopeLock lock(m_mapCatchupDecoderLock);
        m_mapCatchupVideoDecoder[catchupPackets[i]->videoID]->SendPacket(catchupPackets[i]);
        LOG(INFO)<<"send catch up packet to video "<<catchupPackets[i]->videoID<<" and pts is : "<<catchupPackets[i]->pts<<endl;
    }
    return RENDER_STATUS_OK;
}

RenderStatus DecoderManager::UpdateVideoFrame( uint32_t video_id, uint64_t pts, int64_t *corr_pts )
{
    RenderStatus ret = RENDER_STATUS_OK;
    if (video_id < OFFSET_VIDEO_ID_FOR_CATCHUP) {
        if(m_mapVideoDecoder.find(video_id)!=m_mapVideoDecoder.end()){
            ret = m_mapVideoDecoder[video_id]->UpdateFrame(pts, corr_pts);
#ifndef _ANDROID_OS_
            if((STATUS_IDLE == m_mapVideoDecoder[video_id]->GetDecoderStatus())
            &&(ret==RENDER_NO_FRAME)){// to remove rs handler
                LOG(INFO)<<" Now will destroy decoder and handler! video id is " << video_id<< endl;
                m_mapVideoDecoder[video_id]->Destroy();
                SAFE_DELETE(m_mapVideoDecoder[video_id]);
                if (NULL != this->m_handlerFactory)
                {
                    this->m_handlerFactory->RemoveHandler(video_id);
                }
                ret = RENDER_STATUS_OK; // time to destroy the decoder
            }
#endif
        }else{
            ret = RENDER_NO_MATCHED_DECODER;
        }
    }
    else {
        if(m_mapCatchupVideoDecoder.find(video_id)!=m_mapCatchupVideoDecoder.end()){
            ret = m_mapCatchupVideoDecoder[video_id]->UpdateFrame(pts, nullptr);
#ifndef _ANDROID_OS_
            if((STATUS_IDLE == m_mapCatchupVideoDecoder[video_id]->GetDecoderStatus())
            &&(ret==RENDER_NO_FRAME)){// to remove rs handler
                LOG(INFO)<<" Now will destroy decoder and handler! video id is " << video_id<< endl;
                m_mapCatchupVideoDecoder[video_id]->Destroy();
                SAFE_DELETE(m_mapCatchupVideoDecoder[video_id]);
                if (NULL != this->m_handlerFactory)
                {
                    this->m_handlerFactory->RemoveHandler(video_id);
                }
                ret = RENDER_STATUS_OK; // time to destroy the decoder
            }
#endif
        }else{
            ret = RENDER_NO_MATCHED_DECODER;
        }
    }

    return ret;
}

RenderStatus DecoderManager::UpdateVideoFrames( uint64_t pts, int64_t *corr_pts )
{
    RenderStatus ret = RENDER_STATUS_OK;
    uint32_t errorCnt = 0;
    ScopeLock lock(m_mapDecoderLock);
    if (m_mapVideoDecoder.size() == 0 || !IsReady(pts))
    {
        ret = RENDER_NO_FRAME;
        LOG(INFO)<<"There is no valid decoder for now!"<<endl;
        return ret;
    }
    //1. update normal video decoders
    int64_t max_corr_pts = 0;
    for(auto it=m_mapVideoDecoder.begin(); it!=m_mapVideoDecoder.end(); it++){
        int64_t single_corr_pts = 0;
        ret = UpdateVideoFrame(it->first, pts, &single_corr_pts);
        if( ret == RENDER_NO_FRAME ){
            LOG(INFO)<<"Video "<< it->first <<" : haven't found a matched Video Frame relative to pts: " << pts <<std::endl;
            errorCnt++;
        }
        else if (ret == RENDER_WAIT) {
            LOG(INFO) << "Video " << it->first << " : need to wait, input pts is less than frame pts!" << std::endl;
        }
        if(ret == RENDER_EOS)
            LOG(INFO)<<"Video "<< it->first <<" : Reach End Of Stream " << pts <<std::endl;
        if (single_corr_pts > max_corr_pts && single_corr_pts > 0) {
            max_corr_pts = single_corr_pts;
        }
    }
    if (corr_pts != nullptr) {
        if (max_corr_pts == 0) *corr_pts = 0;
        else *corr_pts = max_corr_pts;
    }

    if (errorCnt == m_mapVideoDecoder.size())
    {
        ret = RENDER_NO_FRAME;
    }
    else
    {
        ret = RENDER_STATUS_OK;
    }
    // delete IDLE decoder.
    for(auto it=m_mapVideoDecoder.begin(); it!=m_mapVideoDecoder.end(); ){
        if (it->second == NULL){
            LOG(INFO) << "delete idle decoder!" << endl;
            m_mapVideoDecoder.erase(it++);
        }
        else{
            it++;
        }
    }

    LOG(INFO)<<"[FrameSequences][Update]: Update one frame at:"<<pts<<endl;

    //2. update catch-up video decoders
    RenderStatus st = RENDER_STATUS_OK;
    for(auto it=m_mapCatchupVideoDecoder.begin(); it!=m_mapCatchupVideoDecoder.end(); it++){
        st = UpdateVideoFrame(it->first, pts, nullptr);
        if( st == RENDER_NO_FRAME ){
            LOG(INFO)<<"Catch up Video "<< it->first <<" : haven't found a matched Video Frame relative to pts: " << pts <<std::endl;
        }
        if(st == RENDER_EOS)
            LOG(INFO)<<"Catch up Video "<< it->first <<" : Reach End Of Stream " << pts <<std::endl;
        if (st != RENDER_STATUS_OK)
        {
            errorCnt++;
        }
    }
    if (errorCnt == m_mapCatchupVideoDecoder.size()) // all decoder are error!
    {
        st = RENDER_NO_FRAME;
    }
    else
    {
        st = RENDER_STATUS_OK;
    }
    // delete IDLE decoder.
    for(auto it=m_mapCatchupVideoDecoder.begin(); it!=m_mapCatchupVideoDecoder.end(); ){
        if (it->second == NULL){
            LOG(INFO) << "delete catchup idle decoder!" << endl;
            m_mapCatchupVideoDecoder.erase(it++);
        }
        else{
            it++;
        }
    }
    LOG(INFO)<<"Update one catch-up frame at:"<<pts<<endl;
#ifndef _ANDROID_OS_
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T11_update_time, pts);
#endif
#endif
    return ret;
}

RenderStatus DecoderManager::ResetDecoders()
{
    RenderStatus ret =  RENDER_STATUS_OK;
    return ret;
}

VCD_NS_END