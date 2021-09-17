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
//! \file:   DashSegmenter.cpp
//! \brief:  DashSegmenter class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <algorithm>
#include <sstream>

#include "DashSegmenter.h"

VCD_NS_BEGIN

DashSegmenter::DashSegmenter(GeneralSegConfig *dashConfig, bool createWriter)
    : m_config(*dashConfig)
{
    m_createWriter = createWriter;
}

void DashSegmenter::SetSegmentWriter(VCD::MP4::SegmentWriterBase *segWriter)
{
    m_segWriter = segWriter;

    if (m_createWriter)
    {
        //m_segmentWriter.reset(VCD::MP4::Writer::create());
        if (m_config.useSeparatedSidx)
        {
            m_segWriter->SetWriteSegmentHeader(false);
        }
    }

    for (auto trackIdMeta : m_config.tracks)
    {
        m_segWriter->AddTrack(trackIdMeta.first, trackIdMeta.second);
    }
}

DashSegmenter::~DashSegmenter()
{
    m_segWriter = NULL;
}

int32_t DashSegmenter::SegmentData(TrackSegmentCtx *trackSegCtx)
{
    if (!trackSegCtx->codedMeta.isEOS)
    {
        if (trackSegCtx->isAudio)
        {
            return SegmentOneTrack(&(trackSegCtx->audioNalu), trackSegCtx->codedMeta, trackSegCtx->dashCfg.trackSegBaseName);
        }
        else
        {
            if (trackSegCtx->isExtractorTrack)
            {

                if (!(trackSegCtx->extractors))
                    return OMAF_ERROR_NULL_PTR;

                PackExtractors(trackSegCtx->extractors, trackSegCtx->refTrackIdxs, &(trackSegCtx->extractorTrackNalu));
                return SegmentOneTrack(&(trackSegCtx->extractorTrackNalu), trackSegCtx->codedMeta, trackSegCtx->dashCfg.trackSegBaseName);
            }
            else
            {
                return SegmentOneTrack(trackSegCtx->tileInfo->tileNalu, trackSegCtx->codedMeta, trackSegCtx->dashCfg.trackSegBaseName);
            }
        }
    }
    else
    {
        return SegmentOneTrack(NULL, trackSegCtx->codedMeta, trackSegCtx->dashCfg.trackSegBaseName);
    }
}

int32_t DashSegmenter::SegmentOneTrack(Nalu *dataNalu, VCD::MP4::CodedMeta codedMeta, char *outBaseName)
{
    VCD::MP4::TrackId trackId = 1;
    trackId = codedMeta.trackId;

    TrackInfo& trackInfo = m_trackInfo[trackId];
    if (codedMeta.isEOS)
    {
        if (!trackInfo.endOfStream)
        {
            if (!trackInfo.isFirstFrame)
            {
                m_segWriter->FeedEOS(trackId);
            }
            trackInfo.endOfStream = true;
        }
    }
    else if (codedMeta.inCodingOrder)
    {
        VCD::MP4::CodedMeta frameMeta = codedMeta;
        VCD::MP4::FrameCts frameCts;
        if (frameMeta.presTime == VCD::MP4::FrameTime(0, 1))
        {
            if (trackInfo.isFirstFrame)
            {
                trackInfo.nextCodingTime = VCD::MP4::FrameTime(frameMeta.codingIndex, 1) * frameMeta.duration.cast<VCD::MP4::FrameTime>();
            }
            frameCts = { trackInfo.nextCodingTime };
            trackInfo.nextCodingTime += frameMeta.duration.cast<VCD::MP4::FrameTime>();
        }
        else
        {
            frameCts = { frameMeta.presTime };
        }

        trackInfo.lastPresIndex = frameMeta.presIndex;
        trackInfo.isFirstFrame = false;

        m_segWriter->Feed(trackId, codedMeta, dataNalu->data, dataNalu->dataSize, frameCts);

    }
    else
    {
         return OMAF_ERROR_UNDEFINED_OPERATION;
    }

    int32_t ret = WriteSegment(outBaseName);
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t DashSegmenter::WriteSegment(char *baseName)
{
    if (!m_config.cmafEnabled)
    {
        std::ostringstream frameStream;

        m_segWriter->WriteSegments(frameStream, &(m_segNum), m_segName, baseName, NULL);

        std::string frameString(frameStream.str());

        if (frameString.size())
        {
            m_file = fopen(m_segName, "wb+");
            if (!m_file)
                return OMAF_ERROR_NULL_PTR;

            m_segSize = frameString.size();
            fwrite(frameString.c_str(), 1, frameString.size(), m_file);

            fclose(m_file);
            m_file = NULL;
        }
    }
    else
    {
        uint64_t subSegNumInSeg = (uint64_t)(m_config.subsgtDuration.get().m_den / m_config.subsgtDuration.get().m_num);

        m_segWriter->WriteSegments(m_frameStream, &(m_segNum), m_segName, baseName, &m_segSize);

        if (m_segSize > m_prevSegSize)
        {
            m_subSegNum++;
            m_prevSegSize = m_segSize;
        }

        if (m_subSegNum == subSegNumInSeg)
        {
            m_frameStream.str("");
            m_subSegNum = 0;
            m_prevSegSize = 0;
            m_segSize = 0;
        }
    }

    return ERROR_NONE;
}

int32_t DashSegmenter::PackExtractors(
    std::map<uint8_t, VCD::MP4::Extractor*>* extractorsMap,
    std::list<VCD::MP4::TrackId> refTrackIdxs,
    Nalu *extractorsNalu)
{
    if (!extractorsMap)
        return OMAF_ERROR_NULL_PTR;

    std::vector<uint8_t> extractorNALUs;

    std::map<uint8_t, VCD::MP4::Extractor*>::iterator it;
    std::list<VCD::MP4::TrackId>::iterator itRefTrack = refTrackIdxs.begin();
    if (itRefTrack == refTrackIdxs.end())
        return OMAF_ERROR_INVALID_REF_TRACK;
    for (it = extractorsMap->begin(); it != extractorsMap->end(); it++, itRefTrack++)
    {
        VCD::MP4::Extractor *extractor = it->second;
        if (!extractor)
            return OMAF_ERROR_NULL_PTR;

        m_segWriter->GenPackedExtractors(*itRefTrack, extractorNALUs, extractor);
    }

    if (extractorsNalu->data == NULL)
    {
        if (extractorsNalu->dataSize != 0)
            return OMAF_ERROR_INVALID_DATA;

        int32_t extractorByteSize = extractorNALUs.size();

        extractorsNalu->dataSize = extractorByteSize;
        extractorsNalu->data = (uint8_t*)malloc(extractorByteSize * sizeof(uint8_t));

        if (!(extractorsNalu->data))
            return OMAF_ERROR_NULL_PTR;

         std::vector<uint8_t>::iterator it1;
         int32_t idx = 0;
         for (it1 = extractorNALUs.begin(); it1 != extractorNALUs.end(); )
         {
             extractorsNalu->data[idx] = *it1;
             idx++;
             it1++;
         }
     }
     else
     {
         if (extractorsNalu->dataSize == 0)
         {
             return OMAF_ERROR_INVALID_DATA;
         }

         int32_t extractorByteSize = extractorNALUs.size();
         int32_t origDataSize = extractorsNalu->dataSize;
         extractorsNalu->dataSize += extractorByteSize;

         extractorsNalu->data = (uint8_t*)realloc((void*)(extractorsNalu->data), extractorsNalu->dataSize * sizeof(uint8_t));
         if (!(extractorsNalu->data))
             return OMAF_ERROR_NULL_PTR;

         std::vector<uint8_t>::iterator it1;
         int32_t idx = origDataSize;
         for (it1 = extractorNALUs.begin(); it1 != extractorNALUs.end(); )
         {
             extractorsNalu->data[idx] = *it1;
             idx++;
             it1++;
         }
     }

     extractorNALUs.clear();

     return ERROR_NONE;
}

VCD_NS_END
