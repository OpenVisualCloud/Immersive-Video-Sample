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

#include "OmafPackage.h"
#include "VideoStream.h"
#include "AudioStream.h"
#include "DefaultSegmentation.h"

VCD_NS_BEGIN

OmafPackage::OmafPackage()
{
    m_initInfo = NULL;
    m_segmentation = NULL;
    m_extractorTrackMan = NULL;
    m_isSegmentationStarted = false;
    m_threadId = 0;
}

OmafPackage::~OmafPackage()
{
    if(m_threadId != 0)
    {
        pthread_join(m_threadId, NULL);
    }

    DELETE_MEMORY(m_segmentation);
    DELETE_MEMORY(m_extractorTrackMan);

    std::map<uint8_t, MediaStream*>::iterator it;
    for (it = m_streams.begin(); it != m_streams.end();)
    {
        DELETE_MEMORY(it->second);

        m_streams.erase(it++);
    }
    m_streams.clear();
}

int32_t OmafPackage::AddMediaStream(uint8_t streamIdx, BSBuffer *bs)
{
    if (!bs || !(bs->data))
        return OMAF_ERROR_NULL_PTR;

    if (!(bs->dataSize))
        return OMAF_ERROR_DATA_SIZE;

    if (bs->mediaType == VIDEOTYPE)
    {
        VideoStream *vs = new VideoStream();
        if (!vs)
            return OMAF_ERROR_NULL_PTR;

        ((MediaStream*)vs)->SetMediaType(VIDEOTYPE);

        vs->Initialize(streamIdx, bs, m_initInfo);

        m_streams.insert(std::make_pair(streamIdx, (MediaStream*)vs));
    } else if (bs->mediaType == AUDIOTYPE) {
        AudioStream *as = new AudioStream();
        if (!as)
            return OMAF_ERROR_NULL_PTR;

        ((MediaStream*)as)->SetMediaType(AUDIOTYPE);

        m_streams.insert(std::make_pair(streamIdx, (MediaStream*)as));
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

    m_initInfo = initInfo;

    uint8_t videoStreamsNum = initInfo->bsNumVideo;
    if (!videoStreamsNum)
        return OMAF_ERROR_VIDEO_NUM;

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

int32_t OmafPackage::SetFrameInfo(uint8_t streamIdx, FrameBSInfo *frameInfo)
{
    MediaStream *stream = m_streams[streamIdx];
    if (!stream)
        return OMAF_ERROR_NULL_PTR;

    if (stream->GetMediaType() != VIDEOTYPE)
        return OMAF_ERROR_MEDIA_TYPE;

    int32_t ret = ((VideoStream*)stream)->AddFrameInfo(frameInfo);
    if (ret)
        return OMAF_ERROR_ADD_FRAMEINFO;

    return ERROR_NONE;
}

void* OmafPackage::SegmentationThread(void* pThis)
{
    OmafPackage *omafPackage = (OmafPackage*)pThis;

    omafPackage->SegmentAllStreams();

    return NULL;
}

void OmafPackage::SegmentAllStreams()
{
    m_segmentation->VideoSegmentation();
}

int32_t OmafPackage::OmafPacketStream(uint8_t streamIdx, FrameBSInfo *frameInfo)
{
    int32_t ret = SetFrameInfo(streamIdx, frameInfo);
    if (ret)
        return ret;
    //printf("m_initInfo->segmentationInfo->needBufedFrames %d \n", m_initInfo->segmentationInfo->needBufedFrames);
    if (!m_isSegmentationStarted)
    {
        uint32_t vsNum = 0;
        std::map<uint8_t, MediaStream*>::iterator itMS;
        for (itMS = m_streams.begin(); itMS != m_streams.end(); itMS++)
        {
            MediaStream *stream = itMS->second;
            if (stream->GetMediaType() == VIDEOTYPE)
            {
                VideoStream *vs = (VideoStream*)stream;
                if (vs->GetBufferedFrameNum() >= (uint32_t)(m_initInfo->segmentationInfo->needBufedFrames))
                {
                    vsNum++;
                }
            }
        }
        if (vsNum == m_initInfo->bsNumVideo)
        {
            ret = pthread_create(&m_threadId, NULL, SegmentationThread, this);
            if (ret)
                return OMAF_ERROR_CREATE_THREAD;

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
    }
    //pthread_join(m_threadId, NULL);

    return ERROR_NONE;
}

VCD_NS_END
