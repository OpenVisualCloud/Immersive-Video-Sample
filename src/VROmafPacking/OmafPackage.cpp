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
 */

//!
//! \file:   OmafPackage.cpp
//! \brief:  OmafPackage class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <dlfcn.h>

#include "OmafPackage.h"
#include "VideoStreamPluginAPI.h"
#include "AudioStreamPluginAPI.h"
#include "DefaultSegmentation.h"

VCD_NS_BEGIN

OmafPackage::OmafPackage()
{
    m_initInfo = NULL;
    m_segmentation = NULL;
    m_extractorTrackMan = NULL;
    m_isSegmentationStarted = false;
    m_videoThreadId = 0;
    m_hasAudio = false;
    m_audioThreadId = 0;
}

OmafPackage::OmafPackage(const OmafPackage& src)
{
    m_initInfo = std::move(src.m_initInfo);
    m_segmentation = std::move(src.m_segmentation);
    m_extractorTrackMan = std::move(src.m_extractorTrackMan);
    m_isSegmentationStarted = src.m_isSegmentationStarted;
    m_videoThreadId = src.m_videoThreadId;
    m_hasAudio      = src.m_hasAudio;
    m_audioThreadId = src.m_audioThreadId;
}

OmafPackage& OmafPackage::operator=(OmafPackage&& other)
{
    m_initInfo = std::move(other.m_initInfo);
    m_segmentation = std::move(other.m_segmentation);
    m_extractorTrackMan = std::move(other.m_extractorTrackMan);
    m_isSegmentationStarted = other.m_isSegmentationStarted;
    m_videoThreadId = other.m_videoThreadId;
    m_hasAudio      = other.m_hasAudio;
    m_audioThreadId = other.m_audioThreadId;

    return *this;
}

OmafPackage::~OmafPackage()
{
    if(m_videoThreadId != 0)
    {
        pthread_join(m_videoThreadId, NULL);
    }

    if(m_audioThreadId != 0)
    {
        pthread_join(m_audioThreadId, NULL);
    }

    DELETE_MEMORY(m_segmentation);
    DELETE_MEMORY(m_extractorTrackMan);

    std::map<uint8_t, MediaStream*>::iterator it;
    for (it = m_streams.begin(); it != m_streams.end();)
    {
        MediaStream *stream = it->second;
        if (stream)
        {
            CodecId codec = stream->GetCodecId();
            std::map<CodecId, void*>::iterator itHdl;
            itHdl = m_streamPlugins.find(codec);
            if (itHdl == m_streamPlugins.end())
            {
                OMAF_LOG(LOG_ERROR, "Can't find corresponding stream plugin for codec %d\n", codec);
                return;
            }
            void *pluginHdl = itHdl->second;
            if (!pluginHdl)
            {
                OMAF_LOG(LOG_ERROR, "The stream process plugin handle is NULL !\n");
                return;
            }
            if (stream->GetMediaType() == VIDEOTYPE)
            {
                DestroyVideoStream* destroyVS = NULL;
                destroyVS = (DestroyVideoStream*)dlsym(pluginHdl, "Destroy");
                const char *dlsymErr = dlerror();
                if (dlsymErr)
                {
                    OMAF_LOG(LOG_ERROR, "Failed to load symbol Destroy for codec %d\n", codec);
                    return;
                }
                if (!destroyVS)
                {
                    OMAF_LOG(LOG_ERROR, "NULL video stream destroyer !\n");
                    return;
                }
                destroyVS((VideoStream*)(stream));
            }
            else if (stream->GetMediaType() == AUDIOTYPE)
            {
                DestroyAudioStream* destroyAS = NULL;
                destroyAS = (DestroyAudioStream*)dlsym(pluginHdl, "Destroy");
                const char *dlsymErr = dlerror();
                if (dlsymErr)
                {
                    OMAF_LOG(LOG_ERROR, "Failed to load symbol Destroy for codec %d\n", codec);
                    return;
                }
                if (!destroyAS)
                {
                    OMAF_LOG(LOG_ERROR, "NULL audio stream destroyer !\n");
                    return;
                }
                destroyAS((AudioStream*)(stream));
            }
        }

        m_streams.erase(it++);
    }
    m_streams.clear();

    std::map<CodecId, void*>::iterator itPlug;
    for (itPlug = m_streamPlugins.begin(); itPlug != m_streamPlugins.end(); )
    {
        void *plugHdl = itPlug->second;
        if (plugHdl)
        {
            dlclose(plugHdl);
            plugHdl = NULL;
        }
        m_streamPlugins.erase(itPlug++);
    }
    m_streamPlugins.clear();
}

int32_t OmafPackage::AddMediaStream(uint8_t streamIdx, BSBuffer *bs)
{
    if (!bs || !(bs->data))
        return OMAF_ERROR_NULL_PTR;

    if (!(bs->dataSize))
        return OMAF_ERROR_DATA_SIZE;

    if (bs->mediaType == VIDEOTYPE)
    {
        if (bs->codecId == CODEC_ID_H265)
        {
            void *pluginHdl = NULL;
            std::map<CodecId, void*>::iterator it;
            it = m_streamPlugins.find(CODEC_ID_H265);
            if (it == m_streamPlugins.end())
            {
                char hevcPluginName[1024] = { 0 };//"/usr/local/lib/libHevcVideoStreamProcess.so";
                uint32_t videoPluginPathLen = strlen(m_initInfo->videoProcessPluginPath);
                if (m_initInfo->videoProcessPluginPath[videoPluginPathLen - 1] == '/')
                {
                    snprintf(hevcPluginName, 1024, "%slib%s.so", m_initInfo->videoProcessPluginPath, m_initInfo->videoProcessPluginName);
                }
                else
                {
                    snprintf(hevcPluginName, 1024, "%s/lib%s.so", m_initInfo->videoProcessPluginPath, m_initInfo->videoProcessPluginName);
                }
                OMAF_LOG(LOG_INFO, "Used video stream process plugin is %s\n", hevcPluginName);

                pluginHdl = dlopen(hevcPluginName, RTLD_LAZY);
                const char *dlsymErr = dlerror();
                if (!pluginHdl)
                {
                    OMAF_LOG(LOG_ERROR, "Failed to open HEVC video stream plugin %s\n", hevcPluginName);
                    if (dlsymErr)
                    {
                        OMAF_LOG(LOG_ERROR, "Get error msg %s\n", dlsymErr);
                    }
                    return OMAF_ERROR_DLOPEN;
                }
                m_streamPlugins.insert(std::make_pair(CODEC_ID_H265, pluginHdl));
            }
            else
            {
                pluginHdl = it->second;
                if (!pluginHdl)
                {
                    OMAF_LOG(LOG_ERROR, "NULL HEVC video stream plugin !\n");
                    return OMAF_ERROR_NULL_PTR;
                }
            }

            CreateVideoStream* createVS = NULL;
            createVS = (CreateVideoStream*)dlsym(pluginHdl, "Create");
            const char* dlsymErr1 = dlerror();
            if (dlsymErr1)
            {
                OMAF_LOG(LOG_ERROR, "Failed to load symbol Create: %s\n", dlsymErr1);
                return OMAF_ERROR_DLSYM;
            }

            if (!createVS)
            {
                OMAF_LOG(LOG_ERROR, "NULL video stream creator !\n");
                return OMAF_ERROR_NULL_PTR;
            }

            VideoStream *vs = createVS();
            if (!vs)
            {
                OMAF_LOG(LOG_ERROR, "Failed to create HEVC video stream !\n");
                return OMAF_ERROR_NULL_PTR;
            }

            ((MediaStream*)vs)->SetMediaType(VIDEOTYPE);
            ((MediaStream*)vs)->SetCodecId(CODEC_ID_H265);

            m_streams.insert(std::make_pair(streamIdx, (MediaStream*)vs));
            int32_t ret = vs->Initialize(streamIdx, bs, m_initInfo);
            if (ret)
            {
                OMAF_LOG(LOG_ERROR, "Failed to initialize HEVC video stream !\n");
                return ret;
            }

            vs = NULL;
        }
        else
        {
            OMAF_LOG(LOG_ERROR, "Not supported video codec %d\n", bs->codecId);
            return OMAF_ERROR_INVALID_CODEC;
        }
    } else if (bs->mediaType == AUDIOTYPE) {
        m_hasAudio = true;
        if (bs->codecId == CODEC_ID_AAC)
        {
            void *pluginHdl = NULL;
            std::map<CodecId, void*>::iterator it;
            it = m_streamPlugins.find(CODEC_ID_AAC);
            if (it == m_streamPlugins.end())
            {
                char aacPluginName[1024] = { 0 };//"/usr/local/lib/libAACAudioStreamProcess.so";
                uint32_t audioPluginPathLen = strlen(m_initInfo->audioProcessPluginPath);
                if (m_initInfo->audioProcessPluginPath[audioPluginPathLen - 1] == '/')
                {
                    snprintf(aacPluginName, 1024, "%slib%s.so", m_initInfo->audioProcessPluginPath, m_initInfo->audioProcessPluginName);
                }
                else
                {
                    snprintf(aacPluginName, 1024, "%s/lib%s.so", m_initInfo->audioProcessPluginPath, m_initInfo->audioProcessPluginName);
                }
                OMAF_LOG(LOG_INFO, "Used audio stream process plugin is %s\n", aacPluginName);

                pluginHdl = dlopen(aacPluginName, RTLD_LAZY);
                const char *dlsymErr = dlerror();
                if (!pluginHdl)
                {
                    OMAF_LOG(LOG_ERROR, "Failed to open AAC audio stream plugin %s\n", aacPluginName);
                    if (dlsymErr)
                    {
                        OMAF_LOG(LOG_ERROR, "Get error msg %s\n", dlsymErr);
                    }
                    return OMAF_ERROR_DLOPEN;
                }
                m_streamPlugins.insert(std::make_pair(CODEC_ID_AAC, pluginHdl));
            }
            else
            {
                pluginHdl = it->second;
                if (!pluginHdl)
                {
                    OMAF_LOG(LOG_ERROR, "NULL AAC audio stream plugin !\n");
                    return OMAF_ERROR_NULL_PTR;
                }
            }

            CreateAudioStream* createAS = NULL;
            createAS = (CreateAudioStream*)dlsym(pluginHdl, "Create");
            const char* dlsymErr1 = dlerror();
            if (dlsymErr1)
            {
                OMAF_LOG(LOG_ERROR, "Failed to load symbol Create: %s\n", dlsymErr1);
                return OMAF_ERROR_DLSYM;
            }

            if (!createAS)
            {
                OMAF_LOG(LOG_ERROR, "NULL audio stream creator !\n");
                return OMAF_ERROR_NULL_PTR;
            }

            AudioStream *as = createAS();
            if (!as)
            {
                OMAF_LOG(LOG_ERROR, "Failed to create AAC audio stream !\n");
                return OMAF_ERROR_NULL_PTR;
            }

            ((MediaStream*)as)->SetMediaType(AUDIOTYPE);
            ((MediaStream*)as)->SetCodecId(CODEC_ID_AAC);

            m_streams.insert(std::make_pair(streamIdx, (MediaStream*)as));
            int32_t ret = as->Initialize(streamIdx, bs, m_initInfo);
            if (ret)
            {
                OMAF_LOG(LOG_ERROR, "Failed to initialize AAC audio stream !\n");
                return ret;
            }
            OMAF_LOG(LOG_INFO, "Successfully add one audio stream !\n");
            as = NULL;
        }
        else
        {
            OMAF_LOG(LOG_ERROR, "Not supported audio codec %d\n", bs->codecId);
            return OMAF_ERROR_INVALID_CODEC;
        }
    }

    return ERROR_NONE;
}

int32_t OmafPackage::CreateExtractorTrackManager()
{
    m_extractorTrackMan = new ExtractorTrackManager(m_initInfo);
    if (!m_extractorTrackMan)
        return OMAF_ERROR_NULL_PTR;

    int32_t ret = m_extractorTrackMan->Initialize(&m_streams);
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t OmafPackage::CreateSegmentation()
{
    m_segmentation = new DefaultSegmentation(&m_streams, m_extractorTrackMan, m_initInfo);
    if (!m_segmentation)
        return OMAF_ERROR_NULL_PTR;

    return ERROR_NONE;
}

int32_t OmafPackage::InitOmafPackage(InitialInfo *initInfo)
{
    if (!initInfo)
        return OMAF_ERROR_NULL_PTR;

    if (!initInfo->bsBuffers)
        return OMAF_ERROR_NULL_PTR;

    if (!initInfo->videoProcessPluginPath || !initInfo->videoProcessPluginName)
        return OMAF_ERROR_NO_PLUGIN_SET;

    if (initInfo->bsNumAudio && (!initInfo->audioProcessPluginPath || !initInfo->audioProcessPluginName))
        return OMAF_ERROR_NO_PLUGIN_SET;

    m_initInfo = initInfo;
    if (initInfo->logFunction)
        logCallBack = (LogFunction)(initInfo->logFunction);
    else
        logCallBack = GlogFunction; //default log callback function

    uint8_t streamsNumTotal = initInfo->bsNumVideo + initInfo->bsNumAudio;
    uint8_t streamIdx = 0;
    for( ; streamIdx < streamsNumTotal; streamIdx++)
    {
        BSBuffer bsBuffer = initInfo->bsBuffers[streamIdx];
        int32_t ret = AddMediaStream(streamIdx, &bsBuffer);
        if (ret)
            return OMAF_ERROR_ADD_MEDIASTREAMS;

    }

    int32_t ret = CreateExtractorTrackManager();
    if (ret)
        return OMAF_ERROR_CREATE_EXTRACTORTRACK_MANAGER;

    ret = CreateSegmentation();
    if (ret)
        return OMAF_ERROR_CREATE_SEGMENTATION;

    return ERROR_NONE;
}

int32_t OmafPackage::SetLogCallBack(LogFunction logFunction)
{
    if (!logFunction)
        return OMAF_ERROR_NULL_PTR;

    logCallBack = logFunction;
    return ERROR_NONE;
}

int32_t OmafPackage::SetFrameInfo(uint8_t streamIdx, FrameBSInfo *frameInfo)
{
    MediaStream *stream = m_streams[streamIdx];
    if (!stream)
        return OMAF_ERROR_NULL_PTR;

    if ((stream->GetMediaType() != VIDEOTYPE) && (stream->GetMediaType() != AUDIOTYPE))
        return OMAF_ERROR_MEDIA_TYPE;

    int32_t ret = ERROR_NONE;
    if (stream->GetMediaType() == VIDEOTYPE)
    {
        ret = ((VideoStream*)stream)->AddFrameInfo(frameInfo);
    }
    else if (stream->GetMediaType() == AUDIOTYPE)
    {
        //OMAF_LOG(LOG_INFO, "To add one audio frame with pts %d\n", frameInfo->pts);
        ret = ((AudioStream*)stream)->AddFrameInfo(frameInfo);
    }

    if (ret)
        return OMAF_ERROR_ADD_FRAMEINFO;

    return ERROR_NONE;
}

void* OmafPackage::VideoSegmentationThread(void* pThis)
{
    OmafPackage *omafPackage = (OmafPackage*)pThis;

    omafPackage->SegmentAllVideoStreams();

    return NULL;
}

void OmafPackage::SegmentAllVideoStreams()
{
    m_segmentation->VideoSegmentation();
}

void* OmafPackage::AudioSegmentationThread(void* pThis)
{
    OmafPackage *omafPackage = (OmafPackage*)pThis;

    omafPackage->SegmentAllAudioStreams();

    return NULL;
}

void OmafPackage::SegmentAllAudioStreams()
{
    m_segmentation->AudioSegmentation();
}

int32_t OmafPackage::OmafPacketStream(uint8_t streamIdx, FrameBSInfo *frameInfo)
{
    int32_t ret = SetFrameInfo(streamIdx, frameInfo);
    if (ret)
        return ret;

    if (!m_isSegmentationStarted)
    {
        uint32_t vsNum = 0;
        std::map<uint8_t, MediaStream*>::iterator itMS;
        for (itMS = m_streams.begin(); itMS != m_streams.end(); itMS++)
        {
            MediaStream *stream = itMS->second;
            if (stream && (stream->GetMediaType() == VIDEOTYPE))
            {
                VideoStream *vs = (VideoStream*)stream;
                if (vs->GetBufferedFrameNum() >= (uint32_t)(m_initInfo->segmentationInfo->needBufedFrames))
                {
                    vsNum++;
                }
            }
        }

        uint32_t asNum = 0;
        for (itMS = m_streams.begin(); itMS != m_streams.end(); itMS++)
        {
            MediaStream *stream = itMS->second;
            if (stream && (stream->GetMediaType() == AUDIOTYPE))
            {
                AudioStream *as = (AudioStream*)stream;
                if (as->GetBufferedFrameNum() >= (uint32_t)(m_initInfo->segmentationInfo->needBufedFrames))
                {
                    asNum++;
                }
            }
        }

        if ((vsNum == m_initInfo->bsNumVideo) && (asNum == m_initInfo->bsNumAudio))
        {
            ret = pthread_create(&m_videoThreadId, NULL, VideoSegmentationThread, this);
            if (ret)
                return OMAF_ERROR_CREATE_THREAD;

            if (m_hasAudio)
            {
                ret = pthread_create(&m_audioThreadId, NULL, AudioSegmentationThread, this);
                if (ret)
                    return OMAF_ERROR_CREATE_THREAD;
            }
            m_isSegmentationStarted = true;
        }
    }

    return ERROR_NONE;
}

int32_t OmafPackage::OmafEndStreams()
{
    if (m_segmentation)
    {
        int32_t ret = m_segmentation->VideoEndSegmentation();
        if (ret)
            return ret;

        if (m_hasAudio)
        {
            ret = m_segmentation->AudioEndSegmentation();
            if (ret)
                return ret;
        }
    }

    return ERROR_NONE;
}

VCD_NS_END
