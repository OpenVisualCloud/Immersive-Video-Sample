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
//! \file:   DashInitSegmenter.cpp
//! \brief:  DashInitSegmenter class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <iterator>
#include <sstream>
#include <algorithm>

#include "DashSegmenter.h"
#include "DashWriterPluginAPI.h"

using namespace std;

VCD_NS_BEGIN


DashInitSegmenter::DashInitSegmenter(InitSegConfig *aConfig)
    : m_config(*aConfig)
{
    for (auto& trackIdAndConfig : m_config.tracks)
    {
        m_firstFrameRemaining.insert(trackIdAndConfig.first);
    }
}

DashInitSegmenter::~DashInitSegmenter()
{
}

int32_t DashInitSegmenter::GenerateInitSegment(
    TrackSegmentCtx *trackSegCtx,
    map<VCD::MP4::TrackId, TrackSegmentCtx*> tileTrackSegCtxs)
{
    VCD::MP4::TrackId trackId = trackSegCtx->trackIdx;
    OMAF_LOG(LOG_INFO, "Generate initial segment for track %d!\n", trackId.GetIndex());
    bool hadFirstFramesRemaining = m_firstFrameRemaining.size();
    bool endOfStream = trackSegCtx->codedMeta.isEOS;
    VCD::MP4::DataItem<VCD::MP4::CodedMeta> codedMeta;
    bool isOMAF = (m_config.mode == OperatingMode::OMAF) ? true : false;
    bool isPackedSubPic = m_config.packedSubPictures;
    VCD::MP4::VideoFramePackingType packingType = VCD::MP4::VideoFramePackingType::OMNI_MONOSCOPIC;
    if (m_config.tracks.at(trackId).pipelineOutput == DataInputFormat::VideoTopBottom)
    {
        packingType = VCD::MP4::VideoFramePackingType::OMNI_TOPBOTTOM;
    }
    else if (m_config.tracks.at(trackId).pipelineOutput == DataInputFormat::VideoSideBySide)
    {
        packingType = VCD::MP4::VideoFramePackingType::OMNI_SIDEBYSIDE;
    }

    //OMAF_LOG(LOG_INFO, "Is audio %d\n", trackSegCtx->isAudio);
    if (!(trackSegCtx->isExtractorTrack))
    {
        if (!endOfStream && m_firstFrameRemaining.count(trackId))
        {

            codedMeta = trackSegCtx->codedMeta;

            switch (codedMeta->format)
            {
            case VCD::MP4::CodedFormat::H264:
                m_segWriter->AddH264VideoTrack(trackId, m_config.tracks.at(trackId).meta, *codedMeta);
                break;
            case VCD::MP4::CodedFormat::H265:
                m_segWriter->AddH265VideoTrack(trackId, isOMAF, isPackedSubPic, packingType, m_config.tracks.at(trackId).meta, *codedMeta);
                break;
            case VCD::MP4::CodedFormat::AAC:
                //OMAF_LOG(LOG_INFO, "To add AAC track !\n");
                m_segWriter->AddAACTrack(trackId, m_config.tracks.at(trackId).meta, isOMAF, *codedMeta);
                break;
            case VCD::MP4::CodedFormat::TimedMetadata:
                return OMAF_ERROR_UNDEFINED_OPERATION;
            case VCD::MP4::CodedFormat::H265Extractor:
            {
                using TrackId = VCD::MP4::TrackId;

                map<TrackId, TrackConfig>::const_iterator iter = m_config.tracks.find(trackId);
                if (iter == m_config.tracks.end())
                {
                    OMAF_LOG(LOG_ERROR, "Can't find specified track index !\n");
                    return OMAF_ERROR_INVALID_REF_TRACK;
                }
                TrackConfig trackCfg = iter->second;

                m_segWriter->AddH265ExtractorTrack(trackId, isOMAF, isPackedSubPic, packingType, m_config.tracks.at(trackId).meta, trackCfg.trackReferences, *codedMeta);
                break;
            }
            case VCD::MP4::CodedFormat::NoneFormat:
                return OMAF_ERROR_UNDEFINED_OPERATION;
            }
        }
    }
    else
    {
        for (auto& normalTrack : m_config.tracks)
        {
            map<VCD::MP4::TrackId, TrackSegmentCtx*>::iterator itTrack;
            itTrack = tileTrackSegCtxs.find(normalTrack.first);
            if (itTrack != tileTrackSegCtxs.end())
            {
                TrackSegmentCtx *tileTrackSegCtx = itTrack->second;
                if (tileTrackSegCtx->isExtractorTrack)
                    return OMAF_ERROR_INVALID_TRACKSEG_CTX;

                if (!endOfStream && m_firstFrameRemaining.count(normalTrack.first))
                {
                    codedMeta = tileTrackSegCtx->codedMeta;

                    switch (codedMeta->format)
                    {
                    case VCD::MP4::CodedFormat::H264:
                        m_segWriter->AddH264VideoTrack(normalTrack.first, m_config.tracks.at(normalTrack.first).meta, *codedMeta);
                        break;
                    case VCD::MP4::CodedFormat::H265:
                        m_segWriter->AddH265VideoTrack(normalTrack.first, isOMAF, isPackedSubPic, packingType, m_config.tracks.at(normalTrack.first).meta, *codedMeta);
                        break;
                    case VCD::MP4::CodedFormat::AAC:
                        m_segWriter->AddAACTrack(normalTrack.first, m_config.tracks.at(normalTrack.first).meta, isOMAF, *codedMeta);
                        break;
                    case VCD::MP4::CodedFormat::TimedMetadata:
                        return OMAF_ERROR_UNDEFINED_OPERATION;
                    case VCD::MP4::CodedFormat::H265Extractor:
                    {
                        using TrackId = VCD::MP4::TrackId;

                        map<TrackId, TrackConfig>::const_iterator iter = m_config.tracks.find(normalTrack.first);
                        if (iter == m_config.tracks.end())
                        {
                            OMAF_LOG(LOG_ERROR, "Can't find specified track index !\n");
                            return OMAF_ERROR_INVALID_REF_TRACK;
                        }
                        TrackConfig trackCfg = iter->second;

                        m_segWriter->AddH265ExtractorTrack(normalTrack.first, isOMAF, isPackedSubPic, packingType, m_config.tracks.at(trackId).meta, trackCfg.trackReferences, *codedMeta);
                        break;
                    }
                    case VCD::MP4::CodedFormat::NoneFormat:
                        return OMAF_ERROR_UNDEFINED_OPERATION;
                    }
                }
                m_firstFrameRemaining.erase(normalTrack.first);
            }
            else
            {
                if (trackId.GetIndex() != normalTrack.first.GetIndex())
                    return OMAF_ERROR_INVALID_TRACKSEG_CTX;

                if (!endOfStream && m_firstFrameRemaining.count(trackId))
                {

                    codedMeta = trackSegCtx->codedMeta;

                    switch (codedMeta->format)
                    {
                    case VCD::MP4::CodedFormat::H264:
                        m_segWriter->AddH264VideoTrack(trackId, m_config.tracks.at(trackId).meta, *codedMeta);
                        break;
                    case VCD::MP4::CodedFormat::H265:
                        m_segWriter->AddH265VideoTrack(trackId, isOMAF, isPackedSubPic, packingType, m_config.tracks.at(trackId).meta, *codedMeta);
                        break;
                    case VCD::MP4::CodedFormat::AAC:
                        m_segWriter->AddAACTrack(trackId, m_config.tracks.at(trackId).meta, isOMAF, *codedMeta);
                        break;
                    case VCD::MP4::CodedFormat::TimedMetadata:
                        return OMAF_ERROR_UNDEFINED_OPERATION;
                    case VCD::MP4::CodedFormat::H265Extractor:
                    {
                        using TrackId = VCD::MP4::TrackId;
                        map<TrackId, TrackConfig>::const_iterator iter = m_config.tracks.find(trackId);
                        if (iter == m_config.tracks.end())
                        {
                            OMAF_LOG(LOG_ERROR, "Can't find specified track index !\n");
                            return OMAF_ERROR_INVALID_REF_TRACK;
                        }
                        TrackConfig trackCfg = iter->second;

                        m_segWriter->AddH265ExtractorTrack(trackId, isOMAF, isPackedSubPic, packingType, m_config.tracks.at(trackId).meta, trackCfg.trackReferences, *codedMeta);
                        break;
                    }
                    case VCD::MP4::CodedFormat::NoneFormat:
                        return OMAF_ERROR_UNDEFINED_OPERATION;
                    }
                }
            }
        }
    }

    m_firstFrameRemaining.erase(trackId);

    bool hasFirstFramesRemaining = m_firstFrameRemaining.size();
    if (hadFirstFramesRemaining && !hasFirstFramesRemaining)
    {
        if (m_config.writeToBitstream)
        {
            if (!endOfStream)
            {
                ostringstream frameStream;
                m_segWriter->WriteInitSegment(frameStream, m_config.fragmented);

                string frameString(frameStream.str());
                FILE *fp = fopen(trackSegCtx->dashInitCfg.initSegName, "wb+");
                if (!fp)
                {
                    OMAF_LOG(LOG_ERROR, "Failed to open %s\n", trackSegCtx->dashInitCfg.initSegName);
                    return OMAF_ERROR_NULL_PTR;
                }

                m_initSegSize = frameString.size();
                fwrite(frameString.c_str(), 1, frameString.size(), fp);
                fclose(fp);
                fp = NULL;
            }
        }
    }

    return ERROR_NONE;
}

VCD_NS_END
