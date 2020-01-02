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
#include "streamsegmenter/segmenterapi.hpp"

VCD_NS_BEGIN

AcquireVideoFrameData::AcquireVideoFrameData(uint8_t *data, uint64_t size)
{
    m_data = data;
    m_dataSize = size;
}

AcquireVideoFrameData::~AcquireVideoFrameData()
{

}

StreamSegmenter::FrameData AcquireVideoFrameData::get() const
{
    StreamSegmenter::FrameData frameData(
        static_cast<const std::uint8_t*>(m_data),
        static_cast<const std::uint8_t*>(m_data) + m_dataSize);

    return frameData;
}

size_t AcquireVideoFrameData::getSize() const
{
    return (size_t)(m_dataSize);
}

AcquireVideoFrameData* AcquireVideoFrameData::clone() const
{
    return new AcquireVideoFrameData(m_data, m_dataSize);
}

StreamSegmenter::AutoSegmenterConfig MakeAutoSegmenterConfig(
    GeneralSegConfig *dashConfig)
{
    StreamSegmenter::AutoSegmenterConfig config {};
    config.segmentDuration = dashConfig->sgtDuration;
    config.subsegmentDuration = dashConfig->subsgtDuration;
    config.checkIDR = dashConfig->needCheckIDR;
    return config;
}

DashSegmenter::DashSegmenter(GeneralSegConfig *dashConfig, bool createWriter)
    : m_config(*dashConfig)
    , m_autoSegmenter(MakeAutoSegmenterConfig(dashConfig))
{
    if (createWriter)
    {
        m_segmentWriter.reset(StreamSegmenter::Writer::create());
        if (m_config.useSeparatedSidx)
        {
            m_segmentWriter->setWriteSegmentHeader(false);
        }
    }


    for (auto trackIdMeta : m_config.tracks)
    {
        m_autoSegmenter.addTrack(trackIdMeta.first, trackIdMeta.second);
    }
}

DashSegmenter::~DashSegmenter()
{
}

bool DashSegmenter::DetectNonRefFrame(uint8_t *frameData)
{
    bool flagNonRef = false;
    flagNonRef = ((frameData[AVC_STARTCODES_LEN] >> AVC_NALUTYPE_LEN) == 0);
    return flagNonRef;
}

void DashSegmenter::Feed(
    TrackId trackId,
    CodedMeta codedFrameMeta,
    Nalu *dataNalu,
    StreamSegmenter::FrameCts compositionTime)
{
    CodedMeta frameMeta = codedFrameMeta;
    std::unique_ptr<StreamSegmenter::AcquireFrameData> dataFrameAcquire(
        new AcquireVideoFrameData(dataNalu->data, dataNalu->dataSize));
    StreamSegmenter::FrameInfo infoPerFrame;
    infoPerFrame.cts = compositionTime;
    infoPerFrame.duration = frameMeta.duration;
    infoPerFrame.isIDR = frameMeta.isIDR();
    //LOG(INFO)<<"Feed one sample for track "<<trackId.get()<<" ,IDR status "<<infoPerFrame.isIDR<<" and frame duration "<<infoPerFrame.duration.num<<" and  "<<infoPerFrame.duration.den<<std::endl;
    bool flagNonRefframe = (frameMeta.format == CodedFormat::H264) ? DetectNonRefFrame(dataNalu->data) : false;

    infoPerFrame.sampleFlags.flags.reserved = 0;
    infoPerFrame.sampleFlags.flags.is_leading = (infoPerFrame.isIDR ? 3 : 0);
    infoPerFrame.sampleFlags.flags.sample_depends_on = (infoPerFrame.isIDR ? 2 : 1);
    infoPerFrame.sampleFlags.flags.sample_is_depended_on = (flagNonRefframe ? 2 : 1);
    infoPerFrame.sampleFlags.flags.sample_has_redundancy = 0;
    infoPerFrame.sampleFlags.flags.sample_padding_value = 0;
    infoPerFrame.sampleFlags.flags.sample_is_non_sync_sample = !infoPerFrame.isIDR;
    infoPerFrame.sampleFlags.flags.sample_degradation_priority = 0;

    StreamSegmenter::FrameProxy ssFrame(std::move(dataFrameAcquire), infoPerFrame);
    m_autoSegmenter.feed(trackId, ssFrame);
}

int32_t DashSegmenter::SegmentData(TrackSegmentCtx *trackSegCtx)
{
    if (!trackSegCtx->isEOS && trackSegCtx->isExtractorTrack)
    {

        if (!(trackSegCtx->extractors))
            return OMAF_ERROR_NULL_PTR;

        PackExtractors(trackSegCtx->extractors, trackSegCtx->refTrackIdxs, &(trackSegCtx->extractorTrackNalu));
        return SegmentOneTrack(&(trackSegCtx->extractorTrackNalu), trackSegCtx->codedMeta, trackSegCtx->dashCfg.tileSegBaseName);
    }
    else
    {
        return SegmentOneTrack(trackSegCtx->tileInfo->tileNalu, trackSegCtx->codedMeta, trackSegCtx->dashCfg.tileSegBaseName);
    }
}

int32_t DashSegmenter::SegmentOneTrack(Nalu *dataNalu, CodedMeta codedMeta, char *outBaseName)
{
    TrackId trackId = 1;
    trackId = codedMeta.trackId;

    TrackInfo& trackInfo = m_trackInfo[trackId];

    if (codedMeta.isEOS)
    {
        if (!trackInfo.endOfStream)
        {
            if (!trackInfo.isFirstFrame)
            {
                m_autoSegmenter.feedEnd(trackId);
            }
            trackInfo.endOfStream = true;
        }
    }
    else if (codedMeta.inCodingOrder)
    {
        CodedMeta frameMeta = codedMeta;
        StreamSegmenter::FrameCts frameCts;
        if (frameMeta.presTime == FrameTime(0, 1))
        {
            if (trackInfo.isFirstFrame)
            {
                trackInfo.nextCodingTime = FrameTime(frameMeta.codingIndex, 1) * frameMeta.duration.cast<FrameTime>();
            }
            frameCts = { trackInfo.nextCodingTime };
            trackInfo.nextCodingTime += frameMeta.duration.cast<FrameTime>();
        }
        else
        {
            frameCts = { frameMeta.presTime };
        }

        trackInfo.lastPresIndex = frameMeta.presIndex;
        trackInfo.isFirstFrame = false;

        Feed(trackId, codedMeta, dataNalu, frameCts);

    }
    else
    {
         return OMAF_ERROR_UNDEFINED_OPERATION;
    }

    std::list<StreamSegmenter::Segmenter::Segments> segments = m_autoSegmenter.extractSegmentsWithSubsegments();

    if (segments.size())
    {
        for (auto& segment : segments)
        {
            m_segNum++;
            snprintf(m_segName, 1024, "%s.%ld.mp4", outBaseName, m_segNum);

            WriteSegment(segment);
        }
    }

    return ERROR_NONE;
}

int32_t DashSegmenter::WriteSegment(StreamSegmenter::Segmenter::Segments& aSegment)
{
    std::ostringstream frameStream;
    std::unique_ptr<std::ostringstream> sidxStream;

    if (m_config.useSeparatedSidx)
    {
        sidxStream.reset(new std::ostringstream);
        if (!m_sidxWriter)
        {
            m_sidxWriter = m_segmentWriter->newSidxWriter(1000);
        }
        if (m_sidxWriter)
        {
            m_sidxWriter->setOutput(sidxStream.get());
        }
    }
    else
    {
        //mSidxWriter = mSegmentWriter.newSidxWriter();
    }

    m_segmentWriter->writeSubsegments(frameStream, aSegment);

    std::string frameString(frameStream.str());

    m_file = fopen(m_segName, "wb+");
    if (!m_file)
        return OMAF_ERROR_NULL_PTR;

    fwrite(frameString.c_str(), 1, frameString.size(), m_file);

    fclose(m_file);
    m_file = NULL;

    return ERROR_NONE;
}

int32_t DashSegmenter::PackExtractors(
    std::map<uint8_t, Extractor*>* extractorsMap,
    std::list<TrackId> refTrackIdxs,
    Nalu *extractorsNalu)
{
    if (!extractorsMap)
        return OMAF_ERROR_NULL_PTR;

    std::vector<uint8_t> extractorNALUs;

    std::map<uint8_t, Extractor*>::iterator it;
    std::list<TrackId>::iterator itRefTrack = refTrackIdxs.begin();
    if (itRefTrack == refTrackIdxs.end())
        return OMAF_ERROR_INVALID_REF_TRACK;
    for (it = extractorsMap->begin(); it != extractorsMap->end(); it++, itRefTrack++)
    {
        Extractor *extractor = it->second;
        if (!extractor)
            return OMAF_ERROR_NULL_PTR;


        StreamSegmenter::Segmenter::HevcExtractorTrackFrameData hevcExFrame;
        hevcExFrame.nuhTemporalIdPlus1 = DEFAULT_HEVC_TEMPORALIDPLUS1;//extractor.nuhTemporalIdPlus1;
        std::list<SampleConstructor*>::iterator createSam;
        std::list<InlineConstructor*>::iterator createLine;

        StreamSegmenter::Segmenter::HevcExtractor hevcExOutput;
        for (createSam = extractor->sampleConstructor.begin(),
            createLine = extractor->inlineConstructor.begin();
            createSam != extractor->sampleConstructor.end() ||
            createLine != extractor->inlineConstructor.end();)
        {
            if (createLine != extractor->inlineConstructor.end())
            {
                hevcExOutput.inlineConstructor = StreamSegmenter::Segmenter::HevcExtractorInlineConstructor{};
                std::vector<uint8_t> necessaryInput(
                    static_cast<const uint8_t*>((*createLine)->inlineData),
                    static_cast<const uint8_t*>((*createLine)->inlineData + (*createLine)->length));
                hevcExOutput.inlineConstructor->inlineData = std::move(necessaryInput);
                createLine++;
            }
            else if (createSam != extractor->sampleConstructor.end())
            {
                hevcExOutput.sampleConstructor = StreamSegmenter::Segmenter::HevcExtractorSampleConstructor{};
                hevcExOutput.sampleConstructor->sampleOffset = 0;
                hevcExOutput.sampleConstructor->dataOffset = (*createSam)->dataOffset;
                hevcExOutput.sampleConstructor->dataLength = (*createSam)->dataLength;
                hevcExOutput.sampleConstructor->trackId = (*itRefTrack).get();//(*createSam)->trackRefIndex;  // Note: this refers to the index in the track references. It works if trackIds are 1-based and contiguous, as the spec expects the index is 1-based.
                createSam++;
                // now we have a full extractor: either just a sample constructor, or inline+sample constructor pair
                hevcExFrame.samples.push_back(hevcExOutput);
                const StreamSegmenter::FrameData& nal = hevcExFrame.toFrameData();
                extractorNALUs.insert(extractorNALUs.end(), make_move_iterator(nal.begin()), make_move_iterator(nal.end()));
            }
        }
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
