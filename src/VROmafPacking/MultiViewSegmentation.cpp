/*
 * Copyright (c) 2021, Intel Corporation
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
//! \file:   MultiViewSegmentation.cpp
//! \brief:  Segmentation class implementation for multiple views
//!
//! Created on Nov. 24, 2021, 6:04 AM
//!

#include "VideoStreamPluginAPI.h"
#include "AudioStreamPluginAPI.h"
#include "MultiViewSegmentation.h"
#include "Fraction.h"

#include <unistd.h>
#include <chrono>
#include <cstdint>
#include <sys/time.h>
#include <dlfcn.h>

#ifdef _USE_TRACE_
#include "../trace/E2E_latency_tp.h"
#endif

VCD_NS_BEGIN

MultiViewSegmentation::~MultiViewSegmentation()
{
    std::map<MediaStream*, TrackSegmentCtx*>::iterator itTrackCtx;
    for (itTrackCtx = m_streamSegCtx.begin();
        itTrackCtx != m_streamSegCtx.end();
        itTrackCtx++)
    {
        TrackSegmentCtx *trackSegCtxs = itTrackCtx->second;
        if (trackSegCtxs)
        {
            DELETE_MEMORY(trackSegCtxs->initSegmenter);
            DELETE_MEMORY(trackSegCtxs->dashSegmenter);
        }
        DELETE_MEMORY(trackSegCtxs);
    }
    m_streamSegCtx.clear();

    std::map<MediaStream*, VCD::MP4::MPDAdaptationSetCtx*>::iterator itASCtx;
    for (itASCtx = m_streamASCtx.begin();
        itASCtx != m_streamASCtx.end(); itASCtx++)
    {
        VCD::MP4::MPDAdaptationSetCtx *asCtxs = itASCtx->second;
        if (asCtxs)
        {
            DELETE_MEMORY(asCtxs->videoInfo);
            DELETE_MEMORY(asCtxs);
        }
    }
    m_streamASCtx.clear();
    /*
    if (m_segWriterPluginHdl)
    {
        dlclose(m_segWriterPluginHdl);
        m_segWriterPluginHdl = NULL;
    }
    */
}

int32_t MultiViewSegmentation::CreateDashMPDWriter()
{
    if (!m_mpdWriterPluginHdl)
    {
        OMAF_LOG(LOG_ERROR, "NULL MPD writer plugin handle !\n");
        return OMAF_ERROR_NULL_PTR;
    }

    CreateMPDWriter* createMPDWriter = NULL;
    createMPDWriter = (CreateMPDWriter*)dlsym(m_mpdWriterPluginHdl, "Create");
    const char *dlsymErr2 = NULL;
    dlsymErr2 = dlerror();
    if (dlsymErr2)
    {
        OMAF_LOG(LOG_ERROR, "Failed to load symbol Create: %s\n", dlsymErr2);
        return OMAF_ERROR_DLSYM;
    }

    if (!createMPDWriter)
    {
        OMAF_LOG(LOG_ERROR, "NULL MPD writer creator !\n");
        return OMAF_ERROR_NULL_PTR;
    }

    m_mpdWriter = createMPDWriter(
                        &m_streamASCtx,
                        NULL,//&m_extractorASCtx,
                        m_segInfo,
                        m_projType,
                        m_frameRate,
                        m_videosNum,
                        m_isCMAFEnabled);

    if (!m_mpdWriter)
    {
        OMAF_LOG(LOG_ERROR, "Failed to create MPD writer !\n");
        return OMAF_ERROR_NULL_PTR;
    }

    return ERROR_NONE;
}

int32_t MultiViewSegmentation::ConstructVideoTrackSegCtx()
{
    std::map<uint8_t, MediaStream*>::iterator it;
    for (it = m_streamMap->begin(); it != m_streamMap->end(); it++)
    {
        MediaStream *stream = it->second;
        if (stream && (stream->GetMediaType() == VIDEOTYPE))
        {
            m_videosNum++;
        }
    }

    uint32_t i = 0;
    for (it = m_streamMap->begin(); it != m_streamMap->end(); it++)
    {
        MediaStream *stream = it->second;
        if (stream && (stream->GetMediaType() == VIDEOTYPE))
        {
            VideoStream *vs = (VideoStream*)stream;
            uint16_t srcWidth = vs->GetSrcWidth();
            uint16_t srcHeight = vs->GetSrcHeight();
            NovelViewSEI *viewInfo = vs->GetNovelViewSEIInfo();
            Rational frameRate = vs->GetFrameRate();
            uint64_t bitRate = vs->GetBitRate();
            uint8_t qualityLevel = 1;
            m_projType = (VCD::OMAF::ProjectionFormat)vs->GetProjType();
            OMAF_LOG(LOG_INFO, "Get video source projection type %d\n", m_projType);
            m_videoSegInfo = vs->GetVideoSegInfo();
            Nalu *vpsNalu = vs->GetVPSNalu();
            if (!vpsNalu || !(vpsNalu->data) || !(vpsNalu->dataSize))
                return OMAF_ERROR_INVALID_HEADER;

            Nalu *spsNalu = vs->GetSPSNalu();
            if (!spsNalu || !(spsNalu->data) || !(spsNalu->dataSize))
                return OMAF_ERROR_INVALID_SPS;

            Nalu *ppsNalu = vs->GetPPSNalu();
            if (!ppsNalu || !(ppsNalu->data) || !(ppsNalu->dataSize))
                return OMAF_ERROR_INVALID_PPS;

            std::vector<uint8_t> vpsData(
                static_cast<const uint8_t*>(vpsNalu->data),
                static_cast<const uint8_t*>(vpsNalu->data) + vpsNalu->dataSize);
            std::vector<uint8_t> spsData(
                static_cast<const uint8_t*>(spsNalu->data),
                static_cast<const uint8_t*>(spsNalu->data) + spsNalu->dataSize);
            std::vector<uint8_t> ppsData(
                static_cast<const uint8_t*>(ppsNalu->data),
                static_cast<const uint8_t*>(ppsNalu->data) + ppsNalu->dataSize);

            VCD::MP4::MPDAdaptationSetCtx *mpdASCtx = new VCD::MP4::MPDAdaptationSetCtx;
            if (!mpdASCtx)
                return OMAF_ERROR_NULL_PTR;

            mpdASCtx->videoInfo = new VCD::MP4::VideoAdaptationSetInfo;
            if (!(mpdASCtx->videoInfo))
            {
                DELETE_MEMORY(mpdASCtx);
                return OMAF_ERROR_NULL_PTR;
            }
            memset_s(mpdASCtx->videoInfo, sizeof(VCD::MP4::VideoAdaptationSetInfo), 0);

            TrackSegmentCtx *trackSegCtx = new TrackSegmentCtx;
            if (!trackSegCtx)
            {
                DELETE_MEMORY(mpdASCtx->videoInfo);
                DELETE_MEMORY(mpdASCtx);
                return OMAF_ERROR_NULL_PTR;
            }

            trackSegCtx->isAudio = false;
            trackSegCtx->isExtractorTrack = false;
            trackSegCtx->tileInfo = NULL;
            trackSegCtx->tileIdx = 0;
            trackSegCtx->trackIdx = m_trackIdStarter + i;

            //set InitSegConfig
            TrackConfig trackConfig{};
            trackConfig.meta.trackId = m_trackIdStarter + i;
            trackConfig.meta.timescale = VCD::MP4::FractU64(frameRate.den, frameRate.num * 1000); //?
            trackConfig.meta.type = VCD::MP4::TypeOfMedia::Video;
            trackConfig.pipelineOutput = DataInputFormat::VideoMono;
            (trackSegCtx->dashInitCfg).tracks.insert(std::make_pair(trackSegCtx->trackIdx, trackConfig));
            trackSegCtx->dashInitCfg.fragmented = true;
            trackSegCtx->dashInitCfg.writeToBitstream = true;
            trackSegCtx->dashInitCfg.packedSubPictures = true;
            trackSegCtx->dashInitCfg.mode = OperatingMode::OMAF;
            trackSegCtx->dashInitCfg.streamIds.push_back(trackConfig.meta.trackId.GetIndex());
            snprintf(trackSegCtx->dashInitCfg.initSegName, 1024, "%s%s_track%ld.init.mp4", m_segInfo->dirName, m_segInfo->outName, m_trackIdStarter + i);

            //set GeneralSegConfig
            trackSegCtx->dashCfg.sgtDuration = VCD::MP4::FractU64(m_videoSegInfo->segDur, 1); //?
            if (!m_isCMAFEnabled)
            {
                trackSegCtx->dashCfg.subsgtDuration = trackSegCtx->dashCfg.sgtDuration / VCD::MP4::FrameDuration{ 1, 1}; //?
            }
            else
            {
                trackSegCtx->dashCfg.subsgtDuration = VCD::MP4::FractU64(m_segInfo->chunkDuration, m_segInfo->segDuration * 1000) / VCD::MP4::FrameDuration{ 1, 1};
                if (i == 0)
                {
                    uint64_t subSegNumInSeg = (uint64_t)(trackSegCtx->dashCfg.subsgtDuration.get().m_den / trackSegCtx->dashCfg.subsgtDuration.get().m_num);
                    OMAF_LOG(LOG_INFO, "One CMAF segment contains %ld chunks ! \n", subSegNumInSeg);
                }
            }
            trackSegCtx->dashCfg.needCheckIDR = true;

            VCD::MP4::TrackMeta trackMeta{};
            trackMeta.trackId = trackSegCtx->trackIdx;
            trackMeta.timescale = VCD::MP4::FractU64(frameRate.den, frameRate.num * 1000); //?
            trackMeta.type = VCD::MP4::TypeOfMedia::Video;
            trackSegCtx->dashCfg.tracks.insert(std::make_pair(trackSegCtx->trackIdx, trackMeta));

            trackSegCtx->dashCfg.useSeparatedSidx = false;
            trackSegCtx->dashCfg.streamsIdx.push_back(it->first);
            snprintf(trackSegCtx->dashCfg.trackSegBaseName, 1024, "%s%s_track%ld", m_segInfo->dirName, m_segInfo->outName, m_trackIdStarter + i);
            trackSegCtx->dashCfg.cmafEnabled = m_isCMAFEnabled;

            //setup VCD::MP4::SegmentWriterBase
            int32_t ret = ERROR_NONE;
            trackSegCtx->segWriterPluginHdl = m_segWriterPluginHdl;
            ret = trackSegCtx->CreateDashSegmentWriter();
            if (ret)
            {
                DELETE_MEMORY(mpdASCtx->videoInfo);
                DELETE_MEMORY(mpdASCtx);
                DELETE_MEMORY(trackSegCtx);

                return ret;
            }

            //setup DashInitSegmenter
            trackSegCtx->initSegmenter = new DashInitSegmenter(&(trackSegCtx->dashInitCfg));
            if (!(trackSegCtx->initSegmenter))
            {
                DELETE_MEMORY(mpdASCtx->videoInfo);
                DELETE_MEMORY(mpdASCtx);
                DELETE_MEMORY(trackSegCtx);
                return OMAF_ERROR_NULL_PTR;
            }
            (trackSegCtx->initSegmenter)->SetSegmentWriter(trackSegCtx->segWriter);

            //setup DashSegmenter
            trackSegCtx->dashSegmenter = new DashSegmenter(&(trackSegCtx->dashCfg), true);
            if (!(trackSegCtx->dashSegmenter))
            {
                DELETE_MEMORY(mpdASCtx->videoInfo);
                DELETE_MEMORY(mpdASCtx);
                DELETE_MEMORY(trackSegCtx->initSegmenter);
                DELETE_MEMORY(trackSegCtx);
                return OMAF_ERROR_NULL_PTR;
            }
            (trackSegCtx->dashSegmenter)->SetSegmentWriter(trackSegCtx->segWriter);

            trackSegCtx->qualityRanking = qualityLevel;

            //setup CodedMeta
            trackSegCtx->codedMeta.presIndex = 0;
            trackSegCtx->codedMeta.codingIndex = 0;
            trackSegCtx->codedMeta.codingTime = VCD::MP4::FrameTime{ 0, 1 };
            trackSegCtx->codedMeta.presTime = VCD::MP4::FrameTime{(int64_t) (frameRate.den * 1000), (int64_t)(frameRate.num * 1000) };
            trackSegCtx->codedMeta.duration = VCD::MP4::FrameDuration{ frameRate.den * 1000, frameRate.num * 1000};
            trackSegCtx->codedMeta.trackId = trackSegCtx->trackIdx;
            trackSegCtx->codedMeta.inCodingOrder = true;
            trackSegCtx->codedMeta.format = VCD::MP4::CodedFormat::H265;
            trackSegCtx->codedMeta.decoderConfig.insert(std::make_pair(VCD::MP4::ConfigType::VPS, vpsData));
            trackSegCtx->codedMeta.decoderConfig.insert(std::make_pair(VCD::MP4::ConfigType::SPS, spsData));
            trackSegCtx->codedMeta.decoderConfig.insert(std::make_pair(VCD::MP4::ConfigType::PPS, ppsData));
            trackSegCtx->codedMeta.width = srcWidth;
            trackSegCtx->codedMeta.height = srcHeight;
            trackSegCtx->codedMeta.bitrate.avgBitrate = bitRate;
            trackSegCtx->codedMeta.bitrate.maxBitrate = 0;
            trackSegCtx->codedMeta.type = VCD::MP4::FrameType::IDR;
            trackSegCtx->codedMeta.segmenterMeta.segmentDuration = VCD::MP4::FrameDuration{ 0, 1 }; //?
            trackSegCtx->codedMeta.projection = VCD::MP4::OmafProjectionType::PLANAR;
            trackSegCtx->codedMeta.isEOS = false;

            m_trackSegCtx.insert(std::make_pair(trackSegCtx->trackIdx, trackSegCtx));

            mpdASCtx->videoInfo->fullWidth = srcWidth;
            mpdASCtx->videoInfo->fullHeight = srcHeight;
            mpdASCtx->videoInfo->viewHIdx = viewInfo->cameraID_x;
            mpdASCtx->videoInfo->viewVIdx = viewInfo->cameraID_y;
            mpdASCtx->trackIdx = trackSegCtx->trackIdx;
            mpdASCtx->codedMeta = trackSegCtx->codedMeta;
            mpdASCtx->qualityRanking = DEFAULT_QUALITY_RANK;
            i++;
            m_streamSegCtx.insert(std::make_pair(stream, trackSegCtx));
            m_streamASCtx.insert(std::make_pair(stream, mpdASCtx));
            m_framesIsKey.insert(std::make_pair(stream, true));
            m_streamsIsEOS.insert(std::make_pair(stream, false));
        }
    }

    m_trackIdStarter += m_videosNum;

    return ERROR_NONE;
}

int32_t MultiViewSegmentation::ConstructAudioTrackSegCtx()
{
    uint8_t audioId = 0;
    std::map<uint8_t, MediaStream*>::iterator it;
    for (it = m_streamMap->begin(); it != m_streamMap->end(); it++)
    {
        uint8_t strId = it->first;
        MediaStream *stream = it->second;
        if (stream && (stream->GetMediaType() == AUDIOTYPE))
        {
            //OMAF_LOG(LOG_INFO, "Begin to construct audio track segmentation context !\n");
            AudioStream *as = (AudioStream*)stream;
            uint32_t frequency = as->GetSampleRate();
            uint8_t chlConfig  = as->GetChannelNum();
            uint16_t bitRate   = as->GetBitRate();
            std::vector<uint8_t> packedAudioSpecCfg = as->GetPackedSpecCfg();
            OMAF_LOG(LOG_INFO, "Audio sample rate %d\n", frequency);
            OMAF_LOG(LOG_INFO, "Audio channel number %d\n", chlConfig);
            OMAF_LOG(LOG_INFO, "Audio bit rate %d\n", bitRate);
            OMAF_LOG(LOG_INFO, "Audio specific configuration packed size %lld\n", packedAudioSpecCfg.size());

            VCD::MP4::MPDAdaptationSetCtx *mpdASCtx = new VCD::MP4::MPDAdaptationSetCtx;
            if (!mpdASCtx)
                return OMAF_ERROR_NULL_PTR;

            mpdASCtx->videoInfo = NULL;

            TrackSegmentCtx *trackSegCtx = new TrackSegmentCtx;
            if (!trackSegCtx)
            {
                DELETE_MEMORY(mpdASCtx);
                return OMAF_ERROR_NULL_PTR;
            }

            trackSegCtx->isAudio          = true;
            trackSegCtx->isExtractorTrack = false;
            trackSegCtx->tileInfo         = NULL;
            trackSegCtx->tileIdx          = 0;
            trackSegCtx->extractorTrackIdx = 0;
            trackSegCtx->extractors        = NULL;
            trackSegCtx->trackIdx          = DEFAULT_AUDIOTRACK_TRACKIDBASE + (uint64_t)audioId;

            TrackConfig trackConfig{};
            trackConfig.meta.trackId = trackSegCtx->trackIdx;
            trackConfig.meta.timescale = VCD::MP4::FractU64(1, frequency);
            trackConfig.meta.type = VCD::MP4::TypeOfMedia::Audio;
            trackConfig.pipelineOutput = DataInputFormat::Audio;
            trackSegCtx->dashInitCfg.tracks.insert(std::make_pair(trackSegCtx->trackIdx, trackConfig));
            trackSegCtx->dashInitCfg.fragmented = true;
            trackSegCtx->dashInitCfg.writeToBitstream = true;
            trackSegCtx->dashInitCfg.packedSubPictures = true;
            trackSegCtx->dashInitCfg.mode = OperatingMode::OMAF;
            trackSegCtx->dashInitCfg.streamIds.push_back(trackConfig.meta.trackId.GetIndex());
            snprintf(trackSegCtx->dashInitCfg.initSegName, 1024, "%s%s_track%ld.init.mp4", m_segInfo->dirName, m_segInfo->outName, (DEFAULT_AUDIOTRACK_TRACKIDBASE + (uint64_t)audioId));

            //set GeneralSegConfig
            trackSegCtx->dashCfg.sgtDuration = VCD::MP4::FractU64(m_segInfo->segDuration, 1); //?
            if (!m_isCMAFEnabled)
            {
                trackSegCtx->dashCfg.subsgtDuration = trackSegCtx->dashCfg.sgtDuration / VCD::MP4::FrameDuration{ 1, 1}; //?
            }
            else
            {
                trackSegCtx->dashCfg.subsgtDuration = VCD::MP4::FractU64(m_segInfo->chunkDuration, m_segInfo->segDuration * 1000) / VCD::MP4::FrameDuration{ 1, 1};
            }
            trackSegCtx->dashCfg.needCheckIDR = true;

            VCD::MP4::TrackMeta trackMeta{};
            trackMeta.trackId = trackSegCtx->trackIdx;
            trackMeta.timescale = VCD::MP4::FractU64(1, frequency);
            trackMeta.type = VCD::MP4::TypeOfMedia::Audio;
            trackSegCtx->dashCfg.tracks.insert(std::make_pair(trackSegCtx->trackIdx, trackMeta));

            trackSegCtx->dashCfg.useSeparatedSidx = false;
            trackSegCtx->dashCfg.streamsIdx.push_back(strId);
            snprintf(trackSegCtx->dashCfg.trackSegBaseName, 1024, "%s%s_track%ld", m_segInfo->dirName, m_segInfo->outName, (DEFAULT_AUDIOTRACK_TRACKIDBASE + (uint64_t)audioId));
            trackSegCtx->dashCfg.cmafEnabled = m_isCMAFEnabled;

            //setup VCD::MP4::SegmentWriterBase
            int32_t ret = ERROR_NONE;
            trackSegCtx->segWriterPluginHdl = m_segWriterPluginHdl;
            ret = trackSegCtx->CreateDashSegmentWriter();
            if (ret)
            {
                DELETE_MEMORY(mpdASCtx);
                DELETE_MEMORY(trackSegCtx);
                return ret;
            }

            //setup DashInitSegmenter
            trackSegCtx->initSegmenter = new DashInitSegmenter(&(trackSegCtx->dashInitCfg));
            if (!(trackSegCtx->initSegmenter))
            {
                DELETE_MEMORY(mpdASCtx);
                DELETE_MEMORY(trackSegCtx);
                return OMAF_ERROR_NULL_PTR;
            }
            (trackSegCtx->initSegmenter)->SetSegmentWriter(trackSegCtx->segWriter);

            //setup DashSegmenter
            trackSegCtx->dashSegmenter = new DashSegmenter(&(trackSegCtx->dashCfg), true);
            if (!(trackSegCtx->dashSegmenter))
            {
                DELETE_MEMORY(mpdASCtx);
                DELETE_MEMORY(trackSegCtx->initSegmenter);
                DELETE_MEMORY(trackSegCtx);
                return OMAF_ERROR_NULL_PTR;
            }
            (trackSegCtx->dashSegmenter)->SetSegmentWriter(trackSegCtx->segWriter);

            trackSegCtx->qualityRanking = DEFAULT_QUALITY_RANK;

            //setup CodedMeta
            trackSegCtx->codedMeta.presIndex = 0;
            trackSegCtx->codedMeta.codingIndex = 0;
            trackSegCtx->codedMeta.codingTime = VCD::MP4::FrameTime{ 0, 1 };
            trackSegCtx->codedMeta.presTime = VCD::MP4::FrameTime{ 0, 1000 };
            trackSegCtx->codedMeta.trackId = trackSegCtx->trackIdx;
            trackSegCtx->codedMeta.inCodingOrder = true;
            trackSegCtx->codedMeta.format = VCD::MP4::CodedFormat::AAC;
            trackSegCtx->codedMeta.duration = VCD::MP4::FrameDuration{SAMPLES_NUM_IN_FRAME, frequency};
            trackSegCtx->codedMeta.channelCfg = chlConfig;
            trackSegCtx->codedMeta.samplingFreq = frequency;
            trackSegCtx->codedMeta.type = VCD::MP4::FrameType::IDR;
            trackSegCtx->codedMeta.bitrate.avgBitrate = bitRate;
            trackSegCtx->codedMeta.bitrate.maxBitrate = 0;//?
            trackSegCtx->codedMeta.decoderConfig.insert(std::make_pair(VCD::MP4::ConfigType::AudioSpecificConfig, packedAudioSpecCfg));
            trackSegCtx->codedMeta.isEOS = false;

            trackSegCtx->isEOS = false;

            mpdASCtx->trackIdx = trackSegCtx->trackIdx;
            mpdASCtx->codedMeta = trackSegCtx->codedMeta;

            m_streamSegCtx.insert(std::make_pair(stream, trackSegCtx));
            m_streamASCtx.insert(std::make_pair(stream, mpdASCtx));
            m_trackSegCtx.insert(std::make_pair(trackSegCtx->trackIdx, trackSegCtx));
        }
    }

    m_audioSegCtxsConsted = true;
    //OMAF_LOG(LOG_INFO, "Complete audio segmentation context construction !\n");
    return ERROR_NONE;
}

int32_t MultiViewSegmentation::VideoEndSegmentation()
{
    if (m_streamMap->size())
    {
        std::map<uint8_t, MediaStream*>::iterator it = m_streamMap->begin();
        for ( ; it != m_streamMap->end(); it++)
        {
            MediaStream *stream = it->second;
            if (stream)
            {
                if (stream->GetMediaType() == VIDEOTYPE)
                {
                    int32_t ret = EndEachVideo(stream);
                    if (ret)
                        return ret;
                }
            }
        }
    }

    return ERROR_NONE;
}

int32_t MultiViewSegmentation::AudioEndSegmentation()
{
    if (m_streamMap->size())
    {
        std::map<uint8_t, MediaStream*>::iterator it = m_streamMap->begin();
        for ( ; it != m_streamMap->end(); it++)
        {
            MediaStream *stream = it->second;
            if (stream)
            {
                if (stream->GetMediaType() == AUDIOTYPE)
                {
                    int32_t ret = EndEachAudio(stream);
                    if (ret)
                        return ret;
                }
            }
        }
    }

    return ERROR_NONE;
}

int32_t MultiViewSegmentation::WriteSegmentForEachVideo(MediaStream *stream, FrameBSInfo *frameData, bool isKeyFrame, bool isEOS)
{
    if (!stream)
        return OMAF_ERROR_NULL_PTR;

    std::map<MediaStream*, TrackSegmentCtx*>::iterator itStreamTrack;
    itStreamTrack = m_streamSegCtx.find(stream);
    if (itStreamTrack == m_streamSegCtx.end())
        return OMAF_ERROR_STREAM_NOT_FOUND;

    TrackSegmentCtx *trackSegCtx = itStreamTrack->second;

    DashSegmenter *dashSegmenter = trackSegCtx->dashSegmenter;
    if (!dashSegmenter)
        return OMAF_ERROR_NULL_PTR;

    if (isKeyFrame)
        trackSegCtx->codedMeta.type = VCD::MP4::FrameType::IDR;
    else
        trackSegCtx->codedMeta.type = VCD::MP4::FrameType::NONIDR;

    trackSegCtx->codedMeta.isEOS = isEOS;

    if (frameData && frameData->data && frameData->dataSize)
    {
        trackSegCtx->videoNalu.data = frameData->data;
        trackSegCtx->videoNalu.dataSize = frameData->dataSize;

        uint64_t actualSize = trackSegCtx->videoNalu.dataSize - HEVC_STARTCODES_LEN;
        trackSegCtx->videoNalu.data[0] = (uint8_t)((0xff000000 & actualSize) >> 24);
        trackSegCtx->videoNalu.data[1] = (uint8_t)((0x00ff0000 & actualSize) >> 16);
        trackSegCtx->videoNalu.data[2] = (uint8_t)((0x0000ff00 & actualSize) >> 8);
        trackSegCtx->videoNalu.data[3] = (uint8_t)((uint8_t)actualSize);
    }
    else
    {
        if (!isEOS)
        {
            OMAF_LOG(LOG_ERROR, "NULL video data !\n");
            return OMAF_ERROR_INVALID_DATA;
        }
        else
        {
            trackSegCtx->videoNalu.data = NULL;
            trackSegCtx->videoNalu.dataSize = 0;
        }
    }

    int32_t ret = dashSegmenter->SegmentData(trackSegCtx);
    if (ret)
        return ret;

    trackSegCtx->codedMeta.presIndex++;
    trackSegCtx->codedMeta.codingIndex++;
    trackSegCtx->codedMeta.presTime.m_num += 1000;

    m_segNum = dashSegmenter->GetSegmentsNum();

    return ERROR_NONE;
}

int32_t MultiViewSegmentation::WriteSegmentForEachAudio(MediaStream *stream, FrameBSInfo *frameData, bool isKeyFrame, bool isEOS)
{
    if (!stream)
        return OMAF_ERROR_NULL_PTR;

    AudioStream *as = (AudioStream*)stream;

    uint8_t hdrSize = as->GetHeaderDataSize();

    std::map<MediaStream*, TrackSegmentCtx*>::iterator itStreamTrack;
    itStreamTrack = m_streamSegCtx.find(stream);
    if (itStreamTrack == m_streamSegCtx.end())
        return OMAF_ERROR_STREAM_NOT_FOUND;

    TrackSegmentCtx *trackSegCtx = itStreamTrack->second;
    if (!trackSegCtx)
        return OMAF_ERROR_NULL_PTR;

    if (isKeyFrame)
        trackSegCtx->codedMeta.type = VCD::MP4::FrameType::IDR;
    else
        trackSegCtx->codedMeta.type = VCD::MP4::FrameType::NONIDR;

    trackSegCtx->codedMeta.isEOS = isEOS;
    trackSegCtx->isEOS = isEOS;

    if (frameData && frameData->data && frameData->dataSize)
    {
        trackSegCtx->audioNalu.data = frameData->data + hdrSize;
        trackSegCtx->audioNalu.dataSize = frameData->dataSize - hdrSize;
    }
    else
    {
        trackSegCtx->audioNalu.data = NULL;
        trackSegCtx->audioNalu.dataSize = 0;
    }

    DashSegmenter *dashSegmenter = trackSegCtx->dashSegmenter;
    if (!dashSegmenter)
        return OMAF_ERROR_NULL_PTR;

    //OMAF_LOG(LOG_INFO, "Write audio track segment !\n");
    int32_t ret = dashSegmenter->SegmentData(trackSegCtx);
    if (ret)
        return ret;

    trackSegCtx->codedMeta.presIndex++;
    trackSegCtx->codedMeta.codingIndex++;
    trackSegCtx->codedMeta.presTime.m_num += 1000 / (m_frameRate.num / m_frameRate.den);
    trackSegCtx->codedMeta.presTime.m_den = 1000;

    //OMAF_LOG(LOG_INFO, "EOS %d\n", trackSegCtx->isEOS);
    m_audioSegNum = dashSegmenter->GetSegmentsNum();

    //OMAF_LOG(LOG_INFO, "AUDIO seg num %ld\n", m_audioSegNum);
    return ERROR_NONE;
}

bool MultiViewSegmentation::HasAudio()
{
    std::map<uint8_t, MediaStream*>::iterator it;
    for (it = m_streamMap->begin(); it != m_streamMap->end(); it++)
    {
        MediaStream *stream = it->second;
        if (stream && (stream->GetMediaType() == AUDIOTYPE))
        {
            return true;
        }
    }

    return false;
}

bool MultiViewSegmentation::OnlyAudio()
{
    std::map<uint8_t, MediaStream*>::iterator it;
    for (it = m_streamMap->begin(); it != m_streamMap->end(); it++)
    {
        MediaStream *stream = it->second;
        if (stream && (stream->GetMediaType() == VIDEOTYPE))
        {
            return false;
        }
    }

    return true;
}

int32_t MultiViewSegmentation::VideoSegmentation()
{
    uint64_t currentT = 0;
    int32_t ret = ConstructVideoTrackSegCtx();
    if (ret)
        return ret;

    bool hasAudio = HasAudio();
    if (hasAudio)
    {
        uint32_t waitTimes = 10000;
        uint32_t currWaitTime = 0;
        while (currWaitTime < waitTimes)
        {
            {
                std::lock_guard<std::mutex> lock(m_audioMutex);
                if (m_audioSegCtxsConsted)
                {
                    break;
                }
            }
            usleep(50);
            currWaitTime++;
        }
        if (currWaitTime >= waitTimes)
        {
            OMAF_LOG(LOG_ERROR, "Constructing segmentation context for audio stream takes too long time !\n");
            return OMAF_ERROR_TIMED_OUT;
        }

        {
            std::lock_guard<std::mutex> lock(m_audioMutex);
            /*
            m_mpdGen = new MpdGenerator(
                        &m_streamSegCtx,
                        NULL,//&m_extractorSegCtx,
                        m_segInfo,
                        m_projType,
                        m_frameRate,
                        m_videosNum,
                        m_isCMAFEnabled);
            if (!m_mpdGen)
                return OMAF_ERROR_NULL_PTR;
            */
            ret = CreateDashMPDWriter();
            if (ret)
                return ret;

            ret = m_mpdWriter->Initialize();

            if (ret)
                return ret;

            m_isMpdGenInit = true;
        }
    }
    else
    {
        /*
        m_mpdGen = new MpdGenerator(
                        &m_streamSegCtx,
                        NULL,//&m_extractorSegCtx,
                        m_segInfo,
                        m_projType,
                        m_frameRate,
                        m_videosNum,
                        m_isCMAFEnabled);
        if (!m_mpdGen)
            return OMAF_ERROR_NULL_PTR;
        */

        ret = CreateDashMPDWriter();
        if (ret)
            return ret;

        ret = m_mpdWriter->Initialize();

        if (ret)
            return ret;

        m_isMpdGenInit = true;
    }

    std::map<MediaStream*, TrackSegmentCtx*>::iterator itStreamTrack;
    for (itStreamTrack = m_streamSegCtx.begin(); itStreamTrack != m_streamSegCtx.end(); itStreamTrack++)
    {
        MediaStream *stream = itStreamTrack->first;
        TrackSegmentCtx* trackSegCtx = itStreamTrack->second;

        if (stream && (stream->GetMediaType() == VIDEOTYPE))
        {
            DashInitSegmenter *initSegmenter = trackSegCtx->initSegmenter;
            if (!initSegmenter)
                return OMAF_ERROR_NULL_PTR;

            ret = initSegmenter->GenerateInitSegment(trackSegCtx, m_trackSegCtx);
            if (ret)
                return ret;
        }
    }

    m_prevSegNum = m_segNum;

#ifdef _USE_TRACE_
    int64_t trackIdxTag = 0;
#endif

    while (1)
    {
        if (m_segNum == 1)
        {
            if (hasAudio)
            {
                uint32_t waitTimes = 50000;
                uint32_t currWaitTime = 0;
                while (currWaitTime < waitTimes)
                {
                    {
                        std::lock_guard<std::mutex> lock(m_audioMutex);
                        if (m_audioSegNum >= 1)
                        {
                            break;
                        }
                    }
                    usleep(50);
                    currWaitTime++;
                }
                if (currWaitTime >= waitTimes)
                {
                    OMAF_LOG(LOG_ERROR, "It takes too much time to generate the first audio segment !\n");
                    return OMAF_ERROR_TIMED_OUT;
                }
            }

            if (m_segInfo->isLive)
            {
                m_mpdWriter->UpdateMpd(m_segNum, m_framesNum);
            }
        }

        std::map<uint8_t, MediaStream*>::iterator itStream = m_streamMap->begin();
        for ( ; itStream != m_streamMap->end(); itStream++)
        {
            MediaStream *stream = itStream->second;
            if (stream && (stream->GetMediaType() == VIDEOTYPE))
            {
                VideoStream *vs = (VideoStream*)stream;
                vs->SetCurrFrameInfo();
                FrameBSInfo *currFrame = vs->GetCurrFrameInfo();

                while (!currFrame)
                {
                    usleep(50);
                    vs->SetCurrFrameInfo();
                    currFrame = vs->GetCurrFrameInfo();
                    if (!currFrame && (vs->GetEOS()))
                        break;
                }

                if (currFrame)
                {
                    m_framesIsKey[vs] = currFrame->isKeyFrame;
                    m_streamsIsEOS[vs] = false;

                    WriteSegmentForEachVideo(vs, currFrame, currFrame->isKeyFrame, false);
                }
                else
                {
                    m_framesIsKey[vs] = false;
                    m_streamsIsEOS[vs] = true;

                    WriteSegmentForEachVideo(vs, NULL, false, true);
                }
            }
        }

        std::map<MediaStream*, bool>::iterator itKeyFrame = m_framesIsKey.begin();
        if (itKeyFrame == m_framesIsKey.end())
            return OMAF_ERROR_INVALID_DATA;

        bool frameIsKey = itKeyFrame->second;
        itKeyFrame++;
        for ( ; itKeyFrame != m_framesIsKey.end(); itKeyFrame++)
        {
            bool keyFrame = itKeyFrame->second;
            if (frameIsKey != keyFrame)
                return OMAF_ERROR_INVALID_DATA;
        }
        m_nowKeyFrame = frameIsKey;

        std::map<MediaStream*, bool>::iterator itEOS = m_streamsIsEOS.begin();
        if (itEOS == m_streamsIsEOS.end())
            return OMAF_ERROR_INVALID_DATA;

        bool nowEOS = itEOS->second;
        itEOS++;
        for ( ; itEOS != m_streamsIsEOS.end(); itEOS++)
        {
            bool eos = itEOS->second;
            if (nowEOS != eos)
                return OMAF_ERROR_INVALID_DATA;
        }
        m_isEOS = nowEOS;

        for (itStream = m_streamMap->begin(); itStream != m_streamMap->end(); itStream++)
        {
            MediaStream *stream = itStream->second;
            if (stream && (stream->GetMediaType() == VIDEOTYPE))
            {
                VideoStream *vs = (VideoStream*)stream;

                if (m_segNum == (m_prevSegNum + 1))
                {
                    vs->DestroyCurrSegmentFrames();
                }

                vs->AddFrameToSegment();
            }
        }

        if (m_segNum == (m_prevSegNum + 1))
        {
            m_prevSegNum++;

            std::chrono::high_resolution_clock clock;
            uint64_t before = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
            OMAF_LOG(LOG_INFO, "Complete one seg for video in %lld ms\n", (before - currentT));
            currentT = before;
            if (m_isCMAFEnabled && m_segInfo->isLive)
            {
                m_mpdWriter->UpdateMpd(m_segNum, m_framesNum);
            }

        }

        if (m_segInfo->isLive)
        {
            if (m_segInfo->windowSize && m_segInfo->extraWindowSize)
            {
                int32_t removeCnt = m_segNum - m_segInfo->windowSize - m_segInfo->extraWindowSize;
                if (removeCnt > 0)
                {
                    std::map<VCD::MP4::TrackId, TrackSegmentCtx*>::iterator itOneTrack;

#ifdef _USE_TRACE_
                    auto itOneTrackTag = m_trackSegCtx.begin();
                    trackIdxTag = itOneTrackTag->first.GetIndex();
#endif

                    for (itOneTrack = m_trackSegCtx.begin();
                        itOneTrack != m_trackSegCtx.end();
                        itOneTrack++)
                    {
                        VCD::MP4::TrackId trackIndex = itOneTrack->first;
                        TrackSegmentCtx *segCtx = itOneTrack->second;
                        if (segCtx && !(segCtx->isAudio))
                        {
                            char rmFile[1024];
                            snprintf(rmFile, 1024, "%s%s_track%d.%d.mp4", m_segInfo->dirName, m_segInfo->outName, trackIndex.GetIndex(), removeCnt);
                            remove(rmFile);
                        }
                    }
                }
            }
        }

        if (m_isEOS)
        {
            if (m_segInfo->isLive)
            {
                if (hasAudio)
                {
                    uint32_t waitTimes = 10000;
                    uint32_t currWaitTime = 0;
                    while (currWaitTime < waitTimes)
                    {
                        {
                            std::lock_guard<std::mutex> lock(m_audioMutex);
                            if (m_audioSegNum >= m_segNum)
                            {
                                break;
                            }
                        }
                        usleep(50);
                        currWaitTime++;
                    }
                    if (currWaitTime >= waitTimes)
                    {
                        OMAF_LOG(LOG_ERROR, "Audio still hasn't generated all segments !\n");
                        OMAF_LOG(LOG_ERROR, "Video segments num %ld and audio segments num %ld\n", m_segNum, m_audioSegNum);
                        return OMAF_ERROR_TIMED_OUT;
                    }
                }
                int32_t ret = m_mpdWriter->UpdateMpd(m_segNum, m_framesNum);
                if (ret)
                    return ret;
            } else {
                if (hasAudio)
                {
                    uint32_t waitTimes = 10000;
                    uint32_t currWaitTime = 0;
                    while (currWaitTime < waitTimes)
                    {
                        {
                            std::lock_guard<std::mutex> lock(m_audioMutex);
                            if (m_audioSegNum >= m_segNum)
                            {
                                break;
                            }
                        }
                        usleep(50);
                        currWaitTime++;
                    }
                    if (currWaitTime >= waitTimes)
                    {
                        OMAF_LOG(LOG_ERROR, "Audio still hasn't generated all segments !\n");
                        OMAF_LOG(LOG_ERROR, "Video segments num %ld and audio segments num %ld\n", m_segNum, m_audioSegNum);
                        return OMAF_ERROR_TIMED_OUT;
                    }
                }

                int32_t ret = m_mpdWriter->WriteMpd(m_framesNum);
                if (ret)
                    return ret;
            }
            OMAF_LOG(LOG_INFO, "Totally write %ld frames into video tracks!\n", m_framesNum);
            break;
        }
#ifdef _USE_TRACE_
        string tag = "trackIdx:" + to_string(trackIdxTag);
        tracepoint(E2E_latency_tp_provider,
                   post_op_info,
                   m_framesNum,
                   tag.c_str());
#endif
        m_framesNum++;
    }

    return ERROR_NONE;
}

int32_t MultiViewSegmentation::AudioSegmentation()
{
    OMAF_LOG(LOG_INFO, "Launch audio segmentation thread !\n");
    uint64_t currentT = 0;
    int32_t ret = ConstructAudioTrackSegCtx();
    if (ret)
        return ret;

    OMAF_LOG(LOG_INFO, "Construction for audio track segmentation context DONE !\n");
    bool onlyAudio = OnlyAudio();
    if (onlyAudio)
    {
        /*
        m_mpdGen = new MpdGenerator(
                        &m_streamSegCtx,
                        NULL, //&m_extractorSegCtx,
                        m_segInfo,
                        m_projType,
                        m_frameRate,
                        0,
                        m_isCMAFEnabled);
        if (!m_mpdGen)
            return OMAF_ERROR_NULL_PTR;
        */

        ret = CreateDashMPDWriter();
        if (ret)
            return ret;

        ret = m_mpdWriter->Initialize();

        if (ret)
            return ret;

        m_isMpdGenInit = true;
    }
    else
    {
        bool mpdGenInitialized = false;
        while(!mpdGenInitialized)
        {
            {
                std::lock_guard<std::mutex> lock(m_audioMutex);
                if (m_isMpdGenInit)
                {
                    mpdGenInitialized = true;
                    break;
                }
            }

            usleep(50);
        }
    }

    std::map<MediaStream*, TrackSegmentCtx*>::iterator itStreamTrack;
    for (itStreamTrack = m_streamSegCtx.begin(); itStreamTrack != m_streamSegCtx.end(); itStreamTrack++)
    {
        MediaStream *stream = itStreamTrack->first;
        TrackSegmentCtx* trackSegCtx = itStreamTrack->second;

        if (stream && (stream->GetMediaType() == AUDIOTYPE))
        {
            DashInitSegmenter *initSegmenter = trackSegCtx->initSegmenter;
            if (!initSegmenter)
                return OMAF_ERROR_NULL_PTR;

            ret = initSegmenter->GenerateInitSegment(trackSegCtx, m_trackSegCtx);
            if (ret)
                return ret;
        }
    }

    OMAF_LOG(LOG_INFO, "Done audio initial segment !\n");
    m_audioPrevSegNum = m_audioSegNum;
    OMAF_LOG(LOG_INFO, "Initial audio segment num %ld\n", m_audioSegNum);

    bool nowEOS = false;
    bool eosWritten = false;
    uint64_t framesWritten = 0;
    while(1)
    {
        if (onlyAudio)
        {
            if (m_audioSegNum == 1)
            {
                if (m_segInfo->isLive)
                {
                    m_mpdWriter->UpdateMpd(m_audioSegNum, m_framesNum);
                }
            }
        }

        std::map<uint8_t, MediaStream*>::iterator itStream = m_streamMap->begin();
        for ( ; itStream != m_streamMap->end(); itStream++)
        {
            MediaStream *stream = itStream->second;
            if (stream && (stream->GetMediaType() == AUDIOTYPE))
            {
                AudioStream *as = (AudioStream*)stream;
                as->SetCurrFrameInfo();
                FrameBSInfo *currFrame = as->GetCurrFrameInfo();

                while (!currFrame)
                {
                    usleep(50);
                    as->SetCurrFrameInfo();
                    currFrame = as->GetCurrFrameInfo();
                    if (!currFrame && (as->GetEOS()))
                        break;
                }
                nowEOS = as->GetEOS();
                if (currFrame)
                {
                    WriteSegmentForEachAudio(as, currFrame, true, false);
                    framesWritten++;
                }
                else
                {
                    eosWritten = true;
                }
            }
        }

        for (itStream = m_streamMap->begin(); itStream != m_streamMap->end(); itStream++)
        {
            MediaStream *stream = itStream->second;
            if (stream && (stream->GetMediaType() == AUDIOTYPE))
            {
                AudioStream *as = (AudioStream*)stream;

                if (m_audioSegNum == (m_audioPrevSegNum + 1))
                {
                    as->DestroyCurrSegmentFrames();
                }

                as->AddFrameToSegment();
            }
        }

        if (m_audioSegNum == (m_audioPrevSegNum + 1))
        {
            m_audioPrevSegNum++;

            std::chrono::high_resolution_clock clock;
            uint64_t before = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
            OMAF_LOG(LOG_INFO, "Complete one seg for audio in %lld ms\n", (before - currentT));
            currentT = before;
        }

        if (m_segInfo->isLive)
        {
            if (m_segInfo->windowSize && m_segInfo->extraWindowSize)
            {
                int32_t removeCnt = m_audioSegNum - m_segInfo->windowSize - m_segInfo->extraWindowSize;
                if (removeCnt > 0)
                {
                    for (itStream = m_streamMap->begin(); itStream != m_streamMap->end(); itStream++)
                    {
                        MediaStream *stream = itStream->second;
                        if (stream && (stream->GetMediaType() == AUDIOTYPE))
                        {
                            TrackSegmentCtx* oneSegCtx = m_streamSegCtx[stream];
                            if (!oneSegCtx)
                                return OMAF_ERROR_NULL_PTR;

                            VCD::MP4::TrackId trackIndex = oneSegCtx->trackIdx;
                            char rmFile[1024];
                            snprintf(rmFile, 1024, "%s%s_track%d.%d.mp4", m_segInfo->dirName, m_segInfo->outName, trackIndex.GetIndex(), removeCnt);
                            remove(rmFile);
                        }
                    }
                }
            }
        }

        if (onlyAudio)
        {
            if (nowEOS && eosWritten)
            {
                if (m_segInfo->isLive)
                {
                    int32_t ret = m_mpdWriter->UpdateMpd(m_audioSegNum, m_framesNum);
                    if (ret)
                        return ret;
                } else {
                    int32_t ret = m_mpdWriter->WriteMpd(m_framesNum);
                    if (ret)
                        return ret;
                }
                OMAF_LOG(LOG_INFO, "Total %ld frames written into segments!\n", m_framesNum);
                break;
            }
            m_framesNum++;
        }

        //OMAF_LOG(LOG_INFO, "NOW eos %d \n", nowEOS);
        if (nowEOS && eosWritten)
        {
            std::map<uint8_t, MediaStream*>::iterator itStr = m_streamMap->begin();
            for ( ; itStr != m_streamMap->end(); itStr++)
            {
                MediaStream *stream = itStr->second;
                if (stream && (stream->GetMediaType() == AUDIOTYPE))
                {
                    AudioStream *as = (AudioStream*)stream;
                    WriteSegmentForEachAudio(as, NULL, false, true);
                }
            }

            break;
        }
    }

    OMAF_LOG(LOG_INFO, "Totally write %ld frames into audio track!\n", framesWritten);
    return ERROR_NONE;
}

int32_t MultiViewSegmentation::EndEachVideo(MediaStream *stream)
{
    if (!stream)
        return OMAF_ERROR_NULL_PTR;

    VideoStream *vs = (VideoStream*)stream;
    vs->SetEOS(true);

    return ERROR_NONE;
}

int32_t MultiViewSegmentation::EndEachAudio(MediaStream *stream)
{
    if (!stream)
        return OMAF_ERROR_NULL_PTR;

    AudioStream *as = (AudioStream*)stream;
    as->SetEOS(true);

    return ERROR_NONE;
}

VCD_NS_END
