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
//! \file:   DefaultSegmentation.cpp
//! \brief:  Default segmentation class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "DefaultSegmentation.h"
#include "../isolib/dash_writer/Fraction.h"

#include <unistd.h>
#include <chrono>
#include <cstdint>
#include <sys/time.h>

#include "../trace/Bandwidth_tp.h"

VCD_NS_BEGIN

DefaultSegmentation::~DefaultSegmentation()
{
    std::map<MediaStream*, TrackSegmentCtx*>::iterator itTrackCtx;
    for (itTrackCtx = m_streamSegCtx.begin();
        itTrackCtx != m_streamSegCtx.end();
        itTrackCtx++)
    {
        TrackSegmentCtx *trackSegCtxs = itTrackCtx->second;
        MediaStream *stream = itTrackCtx->first;
        VideoStream *vs = (VideoStream*)stream;
        uint32_t tilesNum = vs->GetTileInRow() * vs->GetTileInCol();
        for (uint32_t i = 0; i < tilesNum; i++)
        {
           DELETE_MEMORY(trackSegCtxs[i].initSegmenter);
           DELETE_MEMORY(trackSegCtxs[i].dashSegmenter);
        }

        delete[] trackSegCtxs;
        trackSegCtxs = NULL;
    }
    m_streamSegCtx.clear();

    std::map<ExtractorTrack*, TrackSegmentCtx*>::iterator itExtractorCtx;
    for (itExtractorCtx = m_extractorSegCtx.begin();
        itExtractorCtx != m_extractorSegCtx.end();
        itExtractorCtx++)
    {
        TrackSegmentCtx *trackSegCtx = itExtractorCtx->second;
        if (trackSegCtx->extractorTrackNalu.data)
        {
            free(trackSegCtx->extractorTrackNalu.data);
            trackSegCtx->extractorTrackNalu.data = NULL;
        }

        DELETE_MEMORY(trackSegCtx->initSegmenter);
        DELETE_MEMORY(trackSegCtx->dashSegmenter);

        DELETE_MEMORY(trackSegCtx);
    }

    m_extractorSegCtx.clear();

    DELETE_ARRAY(m_videosBitrate);

    int32_t ret = pthread_mutex_destroy(&m_mutex);
    if (ret)
    {
        LOG(ERROR) << "Failed to destroy mutex of default segmentation !" << std::endl;
        return;
    }
}

int32_t ConvertRwpk(RegionWisePacking *rwpk, CodedMeta *codedMeta)
{
    if (!rwpk || !codedMeta)
        return OMAF_ERROR_NULL_PTR;

    RegionPacking regionPacking;
    regionPacking.constituentPictMatching = rwpk->constituentPicMatching;
    regionPacking.projPictureWidth = rwpk->projPicWidth;
    regionPacking.projPictureHeight = rwpk->projPicHeight;
    regionPacking.packedPictureWidth = rwpk->packedPicWidth;
    regionPacking.packedPictureHeight = rwpk->packedPicHeight;

    for (uint8_t i = 0; i < rwpk->numRegions; i++)
    {
        Region region;
        region.projTop = rwpk->rectRegionPacking[i].projRegTop;
        region.projLeft = rwpk->rectRegionPacking[i].projRegLeft;
        region.projWidth = rwpk->rectRegionPacking[i].projRegWidth;
        region.projHeight = rwpk->rectRegionPacking[i].projRegHeight;
        region.transform = rwpk->rectRegionPacking[i].transformType;
        region.packedTop = rwpk->rectRegionPacking[i].packedRegTop;
        region.packedLeft = rwpk->rectRegionPacking[i].packedRegLeft;
        region.packedWidth = rwpk->rectRegionPacking[i].packedRegWidth;
        region.packedHeight = rwpk->rectRegionPacking[i].packedRegHeight;

        regionPacking.regions.push_back(region);
    }

    codedMeta->regionPacking = regionPacking;

    return ERROR_NONE;
}

int32_t ConvertCovi(SphereRegion *spr, CodedMeta *codedMeta)
{
    if (!spr || !codedMeta)
        return OMAF_ERROR_NULL_PTR;

    Spherical sphericalCov;
    sphericalCov.cAzimuth = spr->centreAzimuth;
    sphericalCov.cElevation = spr->centreElevation;
    sphericalCov.cTilt = spr->centreTilt;
    sphericalCov.rAzimuth = spr->azimuthRange;
    sphericalCov.rElevation = spr->elevationRange;

    codedMeta->sphericalCoverage = sphericalCov;

    return ERROR_NONE;
}

int32_t FillQualityRank(CodedMeta *codedMeta, std::list<PicResolution> *picResList)
{
    if (!picResList)
        return OMAF_ERROR_NULL_PTR;

    Quality3d qualityRankCov;

    std::list<PicResolution>::iterator it;
    uint8_t qualityRankStarter = MAINSTREAM_QUALITY_RANK;
    uint8_t resNum = 0;
    for (it = picResList->begin(); it != picResList->end(); it++)
    {
        QualityInfo info;
        PicResolution picRes = *it;
        info.origWidth = picRes.width;
        info.origHeight = picRes.height;
        info.qualityRank = qualityRankStarter + resNum;
        Spherical sphere;
        sphere.cAzimuth = codedMeta->sphericalCoverage.get().cAzimuth;
        sphere.cElevation = codedMeta->sphericalCoverage.get().cElevation;
        sphere.cTilt = codedMeta->sphericalCoverage.get().cTilt;
        sphere.rAzimuth = codedMeta->sphericalCoverage.get().rAzimuth;
        sphere.rElevation = codedMeta->sphericalCoverage.get().rElevation;
        info.sphere = sphere;
        qualityRankCov.qualityInfo.push_back(info);
        resNum++;
    }
    qualityRankCov.remainingArea = true;
    codedMeta->qualityRankCoverage = qualityRankCov;

    return ERROR_NONE;
}

int32_t DefaultSegmentation::ConstructTileTrackSegCtx()
{
    std::set<uint64_t> bitRateRanking;

    std::map<uint8_t, MediaStream*>::iterator it;
    for (it = m_streamMap->begin(); it != m_streamMap->end(); it++)
    {
        MediaStream *stream = it->second;
        if (stream->GetMediaType() == VIDEOTYPE)
        {
            m_videosNum++;
            VideoStream *vs = (VideoStream*)stream;
            uint64_t bitRate = vs->GetBitRate();
            bitRateRanking.insert(bitRate);
        }
    }

    if (m_videosNum != bitRateRanking.size())
    {
        LOG(ERROR) << "Invalid video streams number !" << std::endl;
        return OMAF_ERROR_VIDEO_NUM;
    }
    m_videosBitrate = new uint64_t[m_videosNum];
    if (!m_videosBitrate)
        return OMAF_ERROR_NULL_PTR;

    memset(m_videosBitrate, 0, m_videosNum * sizeof(uint64_t));
    std::set<uint64_t>::reverse_iterator rateIter = bitRateRanking.rbegin();
    uint32_t index = 0;
    for ( ; rateIter != bitRateRanking.rend(); rateIter++)
    {
        m_videosBitrate[index] = *rateIter;
        index++;
    }

    for (it = m_streamMap->begin(); it != m_streamMap->end(); it++)
    {
        MediaStream *stream = it->second;
        if (stream->GetMediaType() == VIDEOTYPE)
        {
            VideoStream *vs = (VideoStream*)stream;
            //TrackSegmentCtx *trackSegCtxs = vs->GetAllTrackSegCtxs();
            TileInfo *tilesInfo = vs->GetAllTilesInfo();
            Rational frameRate = vs->GetFrameRate();
            m_frameRate = frameRate;
            uint64_t bitRate = vs->GetBitRate();
            uint8_t qualityLevel = bitRateRanking.size();
            std::set<uint64_t>::iterator itBitRate;
            for (itBitRate = bitRateRanking.begin();
                itBitRate != bitRateRanking.end();
                itBitRate++, qualityLevel--)
            {
                if (*itBitRate == bitRate)
                    break;
            }
            m_projType = (VCD::OMAF::ProjectionFormat)vs->GetProjType();
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

            uint32_t tilesNum = vs->GetTileInRow() * vs->GetTileInCol();

            RegionWisePacking *rwpk = vs->GetSrcRwpk();

            uint64_t tileBitRate = bitRate / tilesNum;

            TrackSegmentCtx *trackSegCtxs = new TrackSegmentCtx[tilesNum];
            if (!trackSegCtxs)
                return OMAF_ERROR_NULL_PTR;
            std::map<uint32_t, VCD::MP4::TrackId> tilesTrackIndex;
            for (uint32_t i = 0; i < tilesNum; i++)
            {
                trackSegCtxs[i].isExtractorTrack = false;
                trackSegCtxs[i].tileInfo = &(tilesInfo[i]);
                trackSegCtxs[i].tileIdx = i;
                trackSegCtxs[i].trackIdx = m_trackIdStarter + i;

                //set InitSegConfig
                TrackConfig trackConfig{};
                trackConfig.meta.trackId = m_trackIdStarter + i;
                trackConfig.meta.timescale = VCD::MP4::FractU64(frameRate.den, frameRate.num * 1000); //?
                trackConfig.meta.type = VCD::MP4::TypeOfMedia::Video;
                trackConfig.pipelineOutput = DataInputFormat::VideoMono;
                trackSegCtxs[i].dashInitCfg.tracks.insert(std::make_pair(trackSegCtxs[i].trackIdx, trackConfig));
                m_allTileTracks.insert(std::make_pair(trackSegCtxs[i].trackIdx, trackConfig));
                trackSegCtxs[i].dashInitCfg.fragmented = true;
                trackSegCtxs[i].dashInitCfg.writeToBitstream = true;
                trackSegCtxs[i].dashInitCfg.packedSubPictures = true;
                trackSegCtxs[i].dashInitCfg.mode = OperatingMode::OMAF;
                trackSegCtxs[i].dashInitCfg.streamIds.push_back(trackConfig.meta.trackId.GetIndex());
                snprintf(trackSegCtxs[i].dashInitCfg.initSegName, 1024, "%s%s_track%ld.init.mp4", m_segInfo->dirName, m_segInfo->outName, m_trackIdStarter + i);

                //set GeneralSegConfig
                trackSegCtxs[i].dashCfg.sgtDuration = VCD::MP4::FractU64(m_videoSegInfo->segDur, 1); //?
                trackSegCtxs[i].dashCfg.subsgtDuration = trackSegCtxs[i].dashCfg.sgtDuration / VCD::MP4::FrameDuration{ 1, 1}; //?
                trackSegCtxs[i].dashCfg.needCheckIDR = true;

                VCD::MP4::TrackMeta trackMeta{};
                trackMeta.trackId = trackSegCtxs[i].trackIdx;
                trackMeta.timescale = VCD::MP4::FractU64(frameRate.den, frameRate.num * 1000); //?
                trackMeta.type = VCD::MP4::TypeOfMedia::Video;
                trackSegCtxs[i].dashCfg.tracks.insert(std::make_pair(trackSegCtxs[i].trackIdx, trackMeta));

                trackSegCtxs[i].dashCfg.useSeparatedSidx = false;
                trackSegCtxs[i].dashCfg.streamsIdx.push_back(it->first);
                snprintf(trackSegCtxs[i].dashCfg.tileSegBaseName, 1024, "%s%s_track%ld", m_segInfo->dirName, m_segInfo->outName, m_trackIdStarter + i);

                //setup DashInitSegmenter
                trackSegCtxs[i].initSegmenter = new DashInitSegmenter(&(trackSegCtxs[i].dashInitCfg));
                if (!(trackSegCtxs[i].initSegmenter))
                {
                    for (uint32_t id = 0; id < i; id++)
                    {
                        DELETE_MEMORY(trackSegCtxs[id].initSegmenter);
                        DELETE_MEMORY(trackSegCtxs[id].dashSegmenter);
                    }
                    DELETE_ARRAY(trackSegCtxs);
                    return OMAF_ERROR_NULL_PTR;
                }

                //setup DashSegmenter
                trackSegCtxs[i].dashSegmenter = new DashSegmenter(&(trackSegCtxs[i].dashCfg), true);
                if (!(trackSegCtxs[i].dashSegmenter))
                {
                    for (uint32_t id = 0; id < i; id++)
                    {
                        DELETE_MEMORY(trackSegCtxs[id].initSegmenter);
                        DELETE_MEMORY(trackSegCtxs[id].dashSegmenter);
                    }

                    DELETE_MEMORY(trackSegCtxs[i].initSegmenter);
                    DELETE_ARRAY(trackSegCtxs);
                    return OMAF_ERROR_NULL_PTR;
                }

                trackSegCtxs[i].qualityRanking = qualityLevel;

                //setup CodedMeta
                trackSegCtxs[i].codedMeta.presIndex = 0;
                trackSegCtxs[i].codedMeta.codingIndex = 0;
                trackSegCtxs[i].codedMeta.codingTime = VCD::MP4::FrameTime{ 0, 1 };
                trackSegCtxs[i].codedMeta.presTime = VCD::MP4::FrameTime{ 0, 1000 };
                trackSegCtxs[i].codedMeta.duration = VCD::MP4::FrameDuration{ frameRate.den * 1000, frameRate.num * 1000};
                trackSegCtxs[i].codedMeta.trackId = trackSegCtxs[i].trackIdx;
                trackSegCtxs[i].codedMeta.inCodingOrder = true;
                trackSegCtxs[i].codedMeta.format = CodedFormat::H265;
                trackSegCtxs[i].codedMeta.decoderConfig.insert(std::make_pair(ConfigType::VPS, vpsData));
                trackSegCtxs[i].codedMeta.decoderConfig.insert(std::make_pair(ConfigType::SPS, spsData));
                trackSegCtxs[i].codedMeta.decoderConfig.insert(std::make_pair(ConfigType::PPS, ppsData));
                trackSegCtxs[i].codedMeta.width = tilesInfo[i].tileWidth;
                trackSegCtxs[i].codedMeta.height = tilesInfo[i].tileHeight;
                trackSegCtxs[i].codedMeta.bitrate.avgBitrate = tileBitRate;
                trackSegCtxs[i].codedMeta.bitrate.maxBitrate = 0;
                trackSegCtxs[i].codedMeta.type = FrameType::IDR;
                trackSegCtxs[i].codedMeta.segmenterMeta.segmentDuration = VCD::MP4::FrameDuration{ 0, 1 }; //?


                RegionWisePacking regionPacking;
                regionPacking.constituentPicMatching = rwpk->constituentPicMatching;
                regionPacking.numRegions = 1;
                regionPacking.projPicWidth = rwpk->projPicWidth;
                regionPacking.projPicHeight = rwpk->projPicHeight;
                regionPacking.packedPicWidth = rwpk->packedPicWidth;
                regionPacking.packedPicHeight = rwpk->packedPicHeight;
                regionPacking.rectRegionPacking = new RectangularRegionWisePacking[1];
                if (!(regionPacking.rectRegionPacking))
                {
                    for (uint32_t id = 0; id < (i + 1); id++)
                    {
                        DELETE_MEMORY(trackSegCtxs[id].initSegmenter);
                        DELETE_MEMORY(trackSegCtxs[id].dashSegmenter);
                    }

                    DELETE_ARRAY(trackSegCtxs);
                    return OMAF_ERROR_NULL_PTR;
                }

                memcpy(&(regionPacking.rectRegionPacking[0]), &(rwpk->rectRegionPacking[i]), sizeof(RectangularRegionWisePacking));
                ConvertRwpk(&(regionPacking), &(trackSegCtxs[i].codedMeta));
                DELETE_ARRAY(regionPacking.rectRegionPacking);


                if (m_projType == VCD::OMAF::ProjectionFormat::PF_ERP)
                {
                    trackSegCtxs[i].codedMeta.projection = OmafProjectionType::EQUIRECTANGULAR;
                }
                else if (m_projType == VCD::OMAF::ProjectionFormat::PF_CUBEMAP)
                {
                    trackSegCtxs[i].codedMeta.projection = OmafProjectionType::CUBEMAP;
                }
                else
                {
                    for (uint32_t id = 0; id < (i + 1); id++)
                    {
                        DELETE_MEMORY(trackSegCtxs[id].initSegmenter);
                        DELETE_MEMORY(trackSegCtxs[id].dashSegmenter);
                    }

                    DELETE_ARRAY(trackSegCtxs);
                    return OMAF_ERROR_INVALID_PROJECTIONTYPE;
                }

                trackSegCtxs[i].codedMeta.isEOS = false;

                tilesTrackIndex.insert(std::make_pair(i, trackSegCtxs[i].trackIdx));

                m_trackSegCtx.insert(std::make_pair(trackSegCtxs[i].trackIdx, &(trackSegCtxs[i])));
            }
            m_trackIdStarter += tilesNum;
            m_streamSegCtx.insert(std::make_pair(stream, trackSegCtxs));
            m_framesIsKey.insert(std::make_pair(stream, true));
            m_streamsIsEOS.insert(std::make_pair(stream, false));
            m_tilesTrackIdxs.insert(std::make_pair(it->first, tilesTrackIndex));
        }
    }

    return ERROR_NONE;
}

int32_t DefaultSegmentation::ConstructExtractorTrackSegCtx()
{
    std::map<uint8_t, ExtractorTrack*> *extractorTracks = m_extractorTrackMan->GetAllExtractorTracks();
    std::map<uint8_t, ExtractorTrack*>::iterator it1;
    for (it1 = extractorTracks->begin(); it1 != extractorTracks->end(); it1++)
    {
        ExtractorTrack *extractorTrack = it1->second;
        Nalu *vpsNalu = extractorTrack->GetVPS();
        Nalu *spsNalu = extractorTrack->GetSPS();
        Nalu *ppsNalu = extractorTrack->GetPPS();

        std::vector<uint8_t> vpsData(
            static_cast<const uint8_t*>(vpsNalu->data),
            static_cast<const uint8_t*>(vpsNalu->data) + vpsNalu->dataSize);
        std::vector<uint8_t> spsData(
            static_cast<const uint8_t*>(spsNalu->data),
            static_cast<const uint8_t*>(spsNalu->data) + spsNalu->dataSize);
        std::vector<uint8_t> ppsData(
            static_cast<const uint8_t*>(ppsNalu->data),
            static_cast<const uint8_t*>(ppsNalu->data) + ppsNalu->dataSize);

        RegionWisePacking *rwpk = extractorTrack->GetRwpk();
        ContentCoverage   *covi = extractorTrack->GetCovi();
        std::list<PicResolution> *picResList = extractorTrack->GetPicRes();
        Nalu *projSEI = extractorTrack->GetProjectionSEI();
        Nalu *rwpkSEI = extractorTrack->GetRwpkSEI();

        TrackSegmentCtx *trackSegCtx = new TrackSegmentCtx;
        if (!trackSegCtx)
            return OMAF_ERROR_NULL_PTR;

        trackSegCtx->isExtractorTrack = true;
        trackSegCtx->extractorTrackIdx = it1->first;
        trackSegCtx->extractors = extractorTrack->GetAllExtractors();
        memset(&(trackSegCtx->extractorTrackNalu), 0, sizeof(Nalu));
        trackSegCtx->extractorTrackNalu.dataSize = projSEI->dataSize + rwpkSEI->dataSize;
        trackSegCtx->extractorTrackNalu.data = new uint8_t[trackSegCtx->extractorTrackNalu.dataSize];
        if (!(trackSegCtx->extractorTrackNalu.data))
        {
            DELETE_MEMORY(trackSegCtx);
            return OMAF_ERROR_NULL_PTR;
        }

        memcpy(trackSegCtx->extractorTrackNalu.data, projSEI->data, projSEI->dataSize);
        memcpy(trackSegCtx->extractorTrackNalu.data + projSEI->dataSize, rwpkSEI->data, rwpkSEI->dataSize);

        TilesMergeDirectionInCol *tilesMergeDir = extractorTrack->GetTilesMergeDir();
        std::list<TilesInCol*>::iterator itCol;
        for (itCol = tilesMergeDir->tilesArrangeInCol.begin();
            itCol != tilesMergeDir->tilesArrangeInCol.end(); itCol++)
        {
            TilesInCol *tileCol = *itCol;
            std::list<SingleTile*>::iterator itTile;
            for (itTile = tileCol->begin(); itTile != tileCol->end(); itTile++)
            {
                SingleTile *tile = *itTile;
                uint8_t vsIdx    = tile->streamIdxInMedia;
                uint8_t origTileIdx  = tile->origTileIdx;

                std::map<uint8_t, std::map<uint32_t, VCD::MP4::TrackId>>::iterator itTilesIdxs;
                itTilesIdxs = m_tilesTrackIdxs.find(vsIdx);
                if (itTilesIdxs == m_tilesTrackIdxs.end())
                {
                    DELETE_ARRAY(trackSegCtx->extractorTrackNalu.data);
                    DELETE_MEMORY(trackSegCtx);
                    return OMAF_ERROR_STREAM_NOT_FOUND;
                }
                std::map<uint32_t, VCD::MP4::TrackId> tilesIndex = itTilesIdxs->second;
                VCD::MP4::TrackId foundTrackId = tilesIndex[origTileIdx];
                trackSegCtx->refTrackIdxs.push_back(foundTrackId);
            }
        }

        trackSegCtx->trackIdx = DEFAULT_EXTRACTORTRACK_TRACKIDBASE + trackSegCtx->extractorTrackIdx;

        //set up InitSegConfig
        std::set<VCD::MP4::TrackId> allTrackIds;
        std::map<VCD::MP4::TrackId, TrackConfig>::iterator itTrack;
        for (itTrack = m_allTileTracks.begin(); itTrack != m_allTileTracks.end(); itTrack++)
        {
            trackSegCtx->dashInitCfg.tracks.insert(std::make_pair(itTrack->first, itTrack->second));
            allTrackIds.insert(itTrack->first);
        }

        TrackConfig trackConfig{};
        trackConfig.meta.trackId = trackSegCtx->trackIdx;
        trackConfig.meta.timescale = VCD::MP4::FractU64(m_frameRate.den, m_frameRate.num * 1000); //?
        trackConfig.meta.type = VCD::MP4::TypeOfMedia::Video;
        trackConfig.trackReferences.insert(std::make_pair("scal", allTrackIds));
        trackConfig.pipelineOutput = DataInputFormat::VideoMono;
        trackSegCtx->dashInitCfg.tracks.insert(std::make_pair(trackSegCtx->trackIdx, trackConfig));

        trackSegCtx->dashInitCfg.fragmented = true;
        trackSegCtx->dashInitCfg.writeToBitstream = true;
        trackSegCtx->dashInitCfg.packedSubPictures = true;
        trackSegCtx->dashInitCfg.mode = OperatingMode::OMAF;
        trackSegCtx->dashInitCfg.streamIds.push_back(trackSegCtx->trackIdx.GetIndex());
        std::set<VCD::MP4::TrackId>::iterator itId;
        for (itId = allTrackIds.begin(); itId != allTrackIds.end(); itId++)
        {
            trackSegCtx->dashInitCfg.streamIds.push_back((*itId).GetIndex());
        }
        snprintf(trackSegCtx->dashInitCfg.initSegName, 1024, "%s%s_track%d.init.mp4", m_segInfo->dirName, m_segInfo->outName, trackSegCtx->trackIdx.GetIndex());

        //set up GeneralSegConfig
        trackSegCtx->dashCfg.sgtDuration = VCD::MP4::FractU64(m_videoSegInfo->segDur, 1); //?
        trackSegCtx->dashCfg.subsgtDuration = trackSegCtx->dashCfg.sgtDuration / VCD::MP4::FrameDuration{ 1, 1}; //?
        trackSegCtx->dashCfg.needCheckIDR = true;

        VCD::MP4::TrackMeta trackMeta{};
        trackMeta.trackId = trackSegCtx->trackIdx;
        trackMeta.timescale = VCD::MP4::FractU64(m_frameRate.den, m_frameRate.num * 1000); //?
        trackMeta.type = VCD::MP4::TypeOfMedia::Video;
        trackSegCtx->dashCfg.tracks.insert(std::make_pair(trackSegCtx->trackIdx, trackMeta));

        trackSegCtx->dashCfg.useSeparatedSidx = false;
        trackSegCtx->dashCfg.streamsIdx.push_back(trackSegCtx->trackIdx.GetIndex());
        snprintf(trackSegCtx->dashCfg.tileSegBaseName, 1024, "%s%s_track%d", m_segInfo->dirName, m_segInfo->outName, trackSegCtx->trackIdx.GetIndex());

        //set up DashInitSegmenter
        trackSegCtx->initSegmenter = new DashInitSegmenter(&(trackSegCtx->dashInitCfg));
        if (!(trackSegCtx->initSegmenter))
        {
            DELETE_ARRAY(trackSegCtx->extractorTrackNalu.data);
            DELETE_MEMORY(trackSegCtx);
            return OMAF_ERROR_NULL_PTR;
        }

        //set up DashSegmenter
        trackSegCtx->dashSegmenter = new DashSegmenter(&(trackSegCtx->dashCfg), true);
        if (!(trackSegCtx->dashSegmenter))
        {
            DELETE_ARRAY(trackSegCtx->extractorTrackNalu.data);
            DELETE_MEMORY(trackSegCtx->initSegmenter);
            DELETE_MEMORY(trackSegCtx);
            return OMAF_ERROR_NULL_PTR;
        }

        //set up CodedMeta
        trackSegCtx->codedMeta.presIndex = 0;
        trackSegCtx->codedMeta.codingIndex = 0;
        trackSegCtx->codedMeta.codingTime = VCD::MP4::FrameTime{ 0, 1 };
        trackSegCtx->codedMeta.presTime = VCD::MP4::FrameTime{ 0, 1000 };
        trackSegCtx->codedMeta.duration = VCD::MP4::FrameDuration{ m_frameRate.den * 1000, m_frameRate.num * 1000};
        trackSegCtx->codedMeta.trackId = trackSegCtx->trackIdx;
        trackSegCtx->codedMeta.inCodingOrder = true;
        trackSegCtx->codedMeta.format = CodedFormat::H265Extractor;
        trackSegCtx->codedMeta.decoderConfig.insert(std::make_pair(ConfigType::VPS, vpsData));
        trackSegCtx->codedMeta.decoderConfig.insert(std::make_pair(ConfigType::SPS, spsData));
        trackSegCtx->codedMeta.decoderConfig.insert(std::make_pair(ConfigType::PPS, ppsData));
        trackSegCtx->codedMeta.width = rwpk->packedPicWidth;//tilesInfo[i].tileWidth;
        trackSegCtx->codedMeta.height = rwpk->packedPicHeight;//tilesInfo[i].tileHeight;
        trackSegCtx->codedMeta.bitrate.avgBitrate = 0;
        trackSegCtx->codedMeta.bitrate.maxBitrate = 0;
        trackSegCtx->codedMeta.type = FrameType::IDR;
        trackSegCtx->codedMeta.segmenterMeta.segmentDuration = VCD::MP4::FrameDuration{ 0, 1 }; //?
        ConvertRwpk(rwpk, &(trackSegCtx->codedMeta));
        ConvertCovi(covi->sphereRegions, &(trackSegCtx->codedMeta));

        FillQualityRank(&(trackSegCtx->codedMeta), picResList);

        if (m_projType == VCD::OMAF::ProjectionFormat::PF_ERP)
        {
            trackSegCtx->codedMeta.projection = OmafProjectionType::EQUIRECTANGULAR;
        }
        else if (m_projType == VCD::OMAF::ProjectionFormat::PF_CUBEMAP)
        {
            trackSegCtx->codedMeta.projection = OmafProjectionType::CUBEMAP;
        }
        else
        {
            DELETE_ARRAY(trackSegCtx->extractorTrackNalu.data);
            DELETE_MEMORY(trackSegCtx->initSegmenter);
            DELETE_MEMORY(trackSegCtx->dashSegmenter);
            DELETE_MEMORY(trackSegCtx);
            return OMAF_ERROR_INVALID_PROJECTIONTYPE;
        }

        trackSegCtx->codedMeta.isEOS = false;

        m_extractorSegCtx.insert(std::make_pair(extractorTrack, trackSegCtx));
    }

    return ERROR_NONE;
}

int32_t DefaultSegmentation::VideoEndSegmentation()
{
    std::map<uint8_t, MediaStream*>::iterator it = m_streamMap->begin();
    for ( ; it != m_streamMap->end(); it++)
    {
        MediaStream *stream = it->second;
        if (stream->GetMediaType() == VIDEOTYPE)
        {
            int32_t ret = EndEachVideo(stream);
            if (ret)
                return ret;
        }
    }

    return ERROR_NONE;
}

int32_t DefaultSegmentation::WriteSegmentForEachVideo(MediaStream *stream, bool isKeyFrame, bool isEOS)
{
    if (!stream)
        return OMAF_ERROR_NULL_PTR;

    VideoStream *vs = (VideoStream*)stream;

    std::map<MediaStream*, TrackSegmentCtx*>::iterator itStreamTrack;
    itStreamTrack = m_streamSegCtx.find(stream);
    if (itStreamTrack == m_streamSegCtx.end())
        return OMAF_ERROR_STREAM_NOT_FOUND;

    TrackSegmentCtx *trackSegCtxs = itStreamTrack->second;

    uint32_t tilesNum = vs->GetTileInRow() * vs->GetTileInCol();
    for (uint32_t tileIdx = 0; tileIdx < tilesNum; tileIdx++)
    {
        DashSegmenter *dashSegmenter = trackSegCtxs[tileIdx].dashSegmenter;
        if (!dashSegmenter)
            return OMAF_ERROR_NULL_PTR;

        if (isKeyFrame)
            trackSegCtxs[tileIdx].codedMeta.type = FrameType::IDR;
        else
            trackSegCtxs[tileIdx].codedMeta.type = FrameType::NONIDR;

        trackSegCtxs[tileIdx].codedMeta.isEOS = isEOS;

        int32_t ret = dashSegmenter->SegmentData(&(trackSegCtxs[tileIdx]));
        if (ret)
            return ret;

        trackSegCtxs[tileIdx].codedMeta.presIndex++;
        trackSegCtxs[tileIdx].codedMeta.codingIndex++;
        trackSegCtxs[tileIdx].codedMeta.presTime.m_num += 1000 / (m_frameRate.num / m_frameRate.den);
        trackSegCtxs[tileIdx].codedMeta.presTime.m_den = 1000;

        m_segNum = dashSegmenter->GetSegmentsNum();

        //trace
        if (m_segNum == (m_prevSegNum + 1))
        {
            uint64_t segSize = dashSegmenter->GetSegmentSize();
            uint32_t trackIndex = trackSegCtxs[tileIdx].trackIdx.GetIndex();
            const char *trackType = "tile_track";
            char tileRes[128] = { 0 };
            snprintf(tileRes, 128, "%d x %d", (trackSegCtxs[tileIdx].tileInfo)->tileWidth, (trackSegCtxs[tileIdx].tileInfo)->tileHeight);

            tracepoint(bandwidth_tp_provider, packed_segment_size, trackIndex, trackType, tileRes, m_segNum, segSize);
        }

    }

    return ERROR_NONE;
}

int32_t DefaultSegmentation::WriteSegmentForEachExtractorTrack(
    ExtractorTrack *extractorTrack,
    bool isKeyFrame,
    bool isEOS)
{
    if (!extractorTrack)
        return OMAF_ERROR_NULL_PTR;

    std::map<ExtractorTrack*, TrackSegmentCtx*>::iterator it;
    it = m_extractorSegCtx.find(extractorTrack);
    if (it == m_extractorSegCtx.end())
        return OMAF_ERROR_EXTRACTORTRACK_NOT_FOUND;

    TrackSegmentCtx *trackSegCtx = it->second;
    if (!trackSegCtx)
        return OMAF_ERROR_NULL_PTR;

    if (isKeyFrame)
        trackSegCtx->codedMeta.type = FrameType::IDR;
    else
        trackSegCtx->codedMeta.type = FrameType::NONIDR;

    trackSegCtx->codedMeta.isEOS = isEOS;

    DashSegmenter *dashSegmenter = trackSegCtx->dashSegmenter;
    if (!dashSegmenter)
       return OMAF_ERROR_NULL_PTR;

    int32_t ret = dashSegmenter->SegmentData(trackSegCtx);
    if (ret)
        return ret;

    trackSegCtx->codedMeta.presIndex++;
    trackSegCtx->codedMeta.codingIndex++;
    trackSegCtx->codedMeta.presTime.m_num += 1000 / (m_frameRate.num / m_frameRate.den);
    trackSegCtx->codedMeta.presTime.m_den = 1000;

    uint64_t currSegNum = dashSegmenter->GetSegmentsNum();
    if (currSegNum == (m_prevSegNum + 1))
    {
        uint64_t segSize = dashSegmenter->GetSegmentSize();
        uint32_t trackIndex = trackSegCtx->trackIdx.GetIndex();
        const char *trackType = "extractor_track";
        char tileRes[128] = { 0 };
        snprintf(tileRes, 128, "%s", "none");

        tracepoint(bandwidth_tp_provider, packed_segment_size, trackIndex, trackType, tileRes, currSegNum, segSize);
    }

    return ERROR_NONE;
}

int32_t DefaultSegmentation::StartExtractorTrackSegmentation(
    ExtractorTrack *extractorTrack)
{
    pthread_t threadId;
    int32_t ret = pthread_create(&threadId, NULL, ExtractorTrackSegThread, this);

    if (ret)
    {
        LOG(ERROR) << "Failed to create extractor track segmentation thread !" << std::endl;
        return OMAF_ERROR_CREATE_THREAD;
    }

    m_extractorThreadIds.insert(std::make_pair(threadId, extractorTrack));
    return ERROR_NONE;
}

int32_t DefaultSegmentation::StartLastExtractorTrackSegmentation(
    ExtractorTrack *extractorTrack)
{
    pthread_t threadId;
    int32_t ret = pthread_create(&threadId, NULL, LastExtractorTrackSegThread, this);

    if (ret)
    {
        LOG(ERROR) << "Failed to create extractor track segmentation thread !" << std::endl;
        return OMAF_ERROR_CREATE_THREAD;
    }

    m_extractorThreadIds.insert(std::make_pair(threadId, extractorTrack));
    return ERROR_NONE;
}

void *DefaultSegmentation::ExtractorTrackSegThread(void *pThis)
{
    DefaultSegmentation *defaultSegmentation = (DefaultSegmentation*)pThis;

    defaultSegmentation->ExtractorTrackSegmentation();

    return NULL;
}

void *DefaultSegmentation::LastExtractorTrackSegThread(void *pThis)
{
    DefaultSegmentation *defaultSegmentation = (DefaultSegmentation*)pThis;

    defaultSegmentation->LastExtractorTrackSegmentation();

    return NULL;
}

int32_t DefaultSegmentation::ExtractorTrackSegmentation()
{
    while(1)
    {

        std::map<uint8_t, ExtractorTrack*> *extractorTracks = m_extractorTrackMan->GetAllExtractorTracks();
        std::map<uint8_t, ExtractorTrack*>::iterator itExtractorTrack;

        pthread_t threadId = pthread_self();
        ExtractorTrack *extractorTrack = m_extractorThreadIds[threadId];
        if (!extractorTrack)
            return OMAF_ERROR_NULL_PTR;

        for (itExtractorTrack = extractorTracks->begin();
            itExtractorTrack != extractorTracks->end(); itExtractorTrack++)
        {
            if (itExtractorTrack->second == extractorTrack)
                break;
        }
        if (itExtractorTrack == extractorTracks->end())
        {
            LOG(ERROR) << "Can't find specified Extractor Track! " << std::endl;
            return OMAF_ERROR_INVALID_DATA;
        }
        while (!(extractorTrack->GetFramesReadyStatus()))
        {
            usleep(50);
        }

        uint8_t etId = 0;
        for ( ; etId < m_aveETPerSegThread; etId++)
        {

            if (itExtractorTrack == extractorTracks->end())
            {
                LOG(ERROR) << "Can't find specified Extractor Track! " << std::endl;
                return OMAF_ERROR_INVALID_DATA;
            }

            ExtractorTrack *extractorTrack1 = itExtractorTrack->second;

            extractorTrack1->ConstructExtractors();
            WriteSegmentForEachExtractorTrack(extractorTrack1, m_nowKeyFrame, m_isEOS);

            std::map<ExtractorTrack*, TrackSegmentCtx*>::iterator itET;
            itET = m_extractorSegCtx.find(extractorTrack1);
            if (itET == m_extractorSegCtx.end())
            {
                LOG(ERROR) << "Can't find segmentation context for specified extractor track !" << std::endl;
                return OMAF_ERROR_INVALID_DATA;
            }
            TrackSegmentCtx *trackSegCtx = itET->second;

            if (m_segNum == (m_prevSegNum + 1))
            {
                extractorTrack1->DestroyCurrSegNalus();
            }

            if (trackSegCtx->extractorTrackNalu.data)
            {
                extractorTrack1->AddExtractorsNaluToSeg(trackSegCtx->extractorTrackNalu.data);
                trackSegCtx->extractorTrackNalu.data = NULL;
            }
            trackSegCtx->extractorTrackNalu.dataSize = 0;

            extractorTrack1->IncreaseProcessedFrmNum();
            itExtractorTrack++;
        }
        if (m_isEOS)
            //return ERROR_NONE;
            break;
    }

    return ERROR_NONE;
}

int32_t DefaultSegmentation::LastExtractorTrackSegmentation()
{
    while(1)
    {
        std::map<uint8_t, ExtractorTrack*> *extractorTracks = m_extractorTrackMan->GetAllExtractorTracks();
        std::map<uint8_t, ExtractorTrack*>::iterator itExtractorTrack;

        pthread_t threadId = pthread_self();
        ExtractorTrack *extractorTrack = m_extractorThreadIds[threadId];
        if (!extractorTrack)
            return OMAF_ERROR_NULL_PTR;

        for (itExtractorTrack = extractorTracks->begin();
            itExtractorTrack != extractorTracks->end(); itExtractorTrack++)
        {
            if (itExtractorTrack->second == extractorTrack)
                break;
        }
        if (itExtractorTrack == extractorTracks->end())
        {
            LOG(ERROR) << "Can't find specified Extractor Track! " << std::endl;
            return OMAF_ERROR_INVALID_DATA;
        }

        while (!(extractorTrack->GetFramesReadyStatus()))
        {
            usleep(50);
        }

        uint8_t etId = 0;
        for ( ; etId < m_lastETPerSegThread; etId++)
        {

            if (itExtractorTrack == extractorTracks->end())
            {
                LOG(ERROR) << "Can't find specified Extractor Track! " << std::endl;
                return OMAF_ERROR_INVALID_DATA;
            }

            ExtractorTrack *extractorTrack1 = itExtractorTrack->second;

            extractorTrack1->ConstructExtractors();
            WriteSegmentForEachExtractorTrack(extractorTrack1, m_nowKeyFrame, m_isEOS);

            std::map<ExtractorTrack*, TrackSegmentCtx*>::iterator itET;
            itET = m_extractorSegCtx.find(extractorTrack1);
            if (itET == m_extractorSegCtx.end())
            {
                LOG(ERROR) << "Can't find segmentation context for specified extractor track !" << std::endl;
                return OMAF_ERROR_INVALID_DATA;
            }
            TrackSegmentCtx *trackSegCtx = itET->second;

            if (m_segNum == (m_prevSegNum + 1))
            {
                extractorTrack1->DestroyCurrSegNalus();
            }

            if (trackSegCtx->extractorTrackNalu.data)
            {
                extractorTrack1->AddExtractorsNaluToSeg(trackSegCtx->extractorTrackNalu.data);
                trackSegCtx->extractorTrackNalu.data = NULL;
            }
            trackSegCtx->extractorTrackNalu.dataSize = 0;

            extractorTrack1->IncreaseProcessedFrmNum();
            itExtractorTrack++;
        }
        if (m_isEOS)
            //return ERROR_NONE;
            break;
    }

    return ERROR_NONE;
}

int32_t DefaultSegmentation::VideoSegmentation()
{
    uint64_t currentT = 0;
    int32_t ret = ConstructTileTrackSegCtx();
    if (ret)
        return ret;

    ret = ConstructExtractorTrackSegCtx();
    if (ret)
        return ret;

    m_mpdGen = new MpdGenerator(
                    &m_streamSegCtx,
                    &m_extractorSegCtx,
                    m_segInfo,
                    m_projType,
                    m_frameRate);
    if (!m_mpdGen)
        return OMAF_ERROR_NULL_PTR;

    ret = m_mpdGen->Initialize();

    if (ret)
        return ret;


    std::map<MediaStream*, TrackSegmentCtx*>::iterator itStreamTrack;
    for (itStreamTrack = m_streamSegCtx.begin(); itStreamTrack != m_streamSegCtx.end(); itStreamTrack++)
    {
        MediaStream *stream = itStreamTrack->first;
        TrackSegmentCtx* trackSegCtxs = itStreamTrack->second;

        if (stream->GetMediaType() == VIDEOTYPE)
        {
            VideoStream *vs = (VideoStream*)stream;
            uint32_t tilesNum = vs->GetTileInRow() * vs->GetTileInCol();
            for (uint32_t tileIdx = 0; tileIdx < tilesNum; tileIdx++)
            {
                DashInitSegmenter *initSegmenter = trackSegCtxs[tileIdx].initSegmenter;
                if (!initSegmenter)
                    return OMAF_ERROR_NULL_PTR;

                ret = initSegmenter->GenerateInitSegment(&(trackSegCtxs[tileIdx]), m_trackSegCtx);
                if (ret)
                    return ret;

                //trace
                uint64_t initSegSize = initSegmenter->GetInitSegmentSize();
                uint32_t trackIndex  = trackSegCtxs[tileIdx].trackIdx.GetIndex();
                const char *trackType = "init_track";
                char tileRes[128] = { 0 };
                snprintf(tileRes, 128, "%s", "none");

                tracepoint(bandwidth_tp_provider, packed_segment_size,
                            trackIndex, trackType, tileRes, 0, initSegSize);
            }
        }
    }

    std::map<ExtractorTrack*, TrackSegmentCtx*>::iterator itExtractorTrack;
    for (itExtractorTrack = m_extractorSegCtx.begin();
        itExtractorTrack != m_extractorSegCtx.end();
        itExtractorTrack++)
    {
        TrackSegmentCtx *trackSegCtx =  itExtractorTrack->second;

        DashInitSegmenter *initSegmenter = trackSegCtx->initSegmenter;
        if (!initSegmenter)
            return OMAF_ERROR_NULL_PTR;

        ret = initSegmenter->GenerateInitSegment(trackSegCtx, m_trackSegCtx);
        if (ret)
            return ret;

        //trace
        uint64_t initSegSize = initSegmenter->GetInitSegmentSize();
        uint32_t trackIndex  = trackSegCtx->trackIdx.GetIndex();
        const char *trackType = "init_track";
        char tileRes[128] = { 0 };
        snprintf(tileRes, 128, "%s", "none");

        tracepoint(bandwidth_tp_provider, packed_segment_size,
                    trackIndex, trackType, tileRes, 0, initSegSize);
    }

    m_prevSegNum = m_segNum;

    uint16_t extractorTrackNum = m_extractorSegCtx.size();
    if (extractorTrackNum % m_segInfo->extractorTracksPerSegThread == 0)
    {
        m_aveETPerSegThread = m_segInfo->extractorTracksPerSegThread;
        m_lastETPerSegThread = m_segInfo->extractorTracksPerSegThread;
        m_threadNumForET = extractorTrackNum / m_segInfo->extractorTracksPerSegThread;
    }
    else
    {
        m_aveETPerSegThread = m_segInfo->extractorTracksPerSegThread;
        m_lastETPerSegThread = extractorTrackNum % m_segInfo->extractorTracksPerSegThread;
        m_threadNumForET = extractorTrackNum / m_segInfo->extractorTracksPerSegThread + 1;
    }

    LOG(INFO) << "Lanuch  " << m_threadNumForET << " threads for Extractor Track segmentation!" << std::endl;
    LOG(INFO) << "Average Extractor Track number per thread is  " << m_aveETPerSegThread << std::endl;
    LOG(INFO) << "The last thread involves  " << m_lastETPerSegThread << " Extractor Tracks !" << std::endl;

    while (1)
    {
        if (m_segNum == 1)
        {
            if (m_segInfo->isLive)
            {
                m_mpdGen->UpdateMpd(m_segNum, m_framesNum);
            }
        }

        std::map<uint8_t, MediaStream*>::iterator itStream = m_streamMap->begin();
        for ( ; itStream != m_streamMap->end(); itStream++)
        {
            MediaStream *stream = itStream->second;
            if (stream->GetMediaType() == VIDEOTYPE)
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

                    //trace
                    char resolution[1024] = { 0 };
                    snprintf(resolution, 1024, "%d x %d", vs->GetSrcWidth(), vs->GetSrcHeight());
                    char tileSplit[1024] = { 0 };
                    snprintf(tileSplit, 1024, "%d x %d", vs->GetTileInCol(), vs->GetTileInRow());
                    tracepoint(bandwidth_tp_provider, encoded_frame_size,
                                &resolution[0], &tileSplit[0], m_framesNum, currFrame->dataSize);

                    vs->UpdateTilesNalu();
                    WriteSegmentForEachVideo(vs, currFrame->isKeyFrame, false);
                }
                else
                {
                    m_framesIsKey[vs] = false;
                    m_streamsIsEOS[vs] = true;

                    WriteSegmentForEachVideo(vs, false, true);
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
            return OMAF_ERROR_STREAM_NOT_FOUND;
        bool nowEOS = itEOS->second;
        itEOS++;
        for ( ; itEOS != m_streamsIsEOS.end(); itEOS++)
        {
            bool eos = itEOS->second;
            if (nowEOS != eos)
                return OMAF_ERROR_INVALID_DATA;
        }
        m_isEOS = nowEOS;

        std::map<uint8_t, ExtractorTrack*> *extractorTracks = m_extractorTrackMan->GetAllExtractorTracks();
        std::map<uint8_t, ExtractorTrack*>::iterator itExtractorTrack = extractorTracks->begin();
        for ( ; itExtractorTrack != extractorTracks->end(); /*itExtractorTrack++*/)
        {
            ExtractorTrack *extractorTrack = itExtractorTrack->second;
            extractorTrack->SetFramesReady(true);
            if (m_extractorThreadIds.size() < m_threadNumForET)
            {
                if (m_aveETPerSegThread == m_lastETPerSegThread)
                {
                    int32_t retET = StartExtractorTrackSegmentation(extractorTrack);
                    if (retET)
                        return retET;

                    for (uint16_t num = 0; num < m_aveETPerSegThread; num++)
                    {
                        itExtractorTrack++;
                    }
                }
                else
                {
                    if ((uint16_t)(m_extractorThreadIds.size()) < (m_threadNumForET - 1))
                    {
                        int32_t retET = StartExtractorTrackSegmentation(extractorTrack);
                        if (retET)
                            return retET;

                        for (uint16_t num = 0; num < m_aveETPerSegThread; num++)
                        {
                            itExtractorTrack++;
                        }
                    }
                    else
                    {
                        int32_t retET = StartLastExtractorTrackSegmentation(extractorTrack);
                        if (retET)
                            return retET;

                        for ( ; itExtractorTrack != extractorTracks->end(); )
                        {
                            itExtractorTrack++;
                        }
                    }
                }
            }
            else
            {
                itExtractorTrack++;
            }
        }
        if (m_extractorThreadIds.size() != m_threadNumForET)
        {
            LOG(ERROR) << "Launched threads number  " << (m_extractorThreadIds.size()) << " doesn't match calculated threads number  " << m_threadNumForET << std::endl;
        }

        usleep(2000);

        for (itExtractorTrack = extractorTracks->begin();
            itExtractorTrack != extractorTracks->end();
            itExtractorTrack++)
        {
            ExtractorTrack *extractorTrack = itExtractorTrack->second;
            while (extractorTrack->GetProcessedFrmNum() == m_framesNum)
            {
                usleep(1);

                if (extractorTrack->GetProcessedFrmNum() == (m_framesNum + 1))
                    break;
            }
        }

        for (itStream = m_streamMap->begin(); itStream != m_streamMap->end(); itStream++)
        {
            MediaStream *stream = itStream->second;
            if (stream->GetMediaType() == VIDEOTYPE)
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
            LOG(INFO) << "Complete one seg in  " << (before - currentT) << " ms" << std::endl;
            currentT = before;
        }

        if (m_segInfo->isLive)
        {
            if (m_segInfo->windowSize && m_segInfo->extraWindowSize)
            {
                int32_t removeCnt = m_segNum - m_segInfo->windowSize - m_segInfo->extraWindowSize;
                if (removeCnt > 0)
                {
                    std::map<VCD::MP4::TrackId, TrackConfig>::iterator itOneTrack;
                    for (itOneTrack = m_allTileTracks.begin();
                        itOneTrack != m_allTileTracks.end();
                        itOneTrack++)
                    {
                        VCD::MP4::TrackId trackIndex = itOneTrack->first;
                        char rmFile[1024];
                        snprintf(rmFile, 1024, "%s%s_track%d.%d.mp4", m_segInfo->dirName, m_segInfo->outName, trackIndex.GetIndex(), removeCnt);
                        remove(rmFile);
                    }
                    std::map<ExtractorTrack*, TrackSegmentCtx*>::iterator itOneExtractorTrack;
                    for (itOneExtractorTrack = m_extractorSegCtx.begin();
                        itOneExtractorTrack != m_extractorSegCtx.end();
                        itOneExtractorTrack++)
                    {
                        TrackSegmentCtx *trackSegCtx = itOneExtractorTrack->second;
                        VCD::MP4::TrackId trackIndex = trackSegCtx->trackIdx;
                        char rmFile[1024];
                        snprintf(rmFile, 1024, "%s%s_track%d.%d.mp4", m_segInfo->dirName, m_segInfo->outName, trackIndex.GetIndex(), removeCnt);
                        remove(rmFile);
                    }
                }
            }
        }

        if (m_isEOS)
        {
            if (m_segInfo->isLive)
            {
                int32_t ret = m_mpdGen->UpdateMpd(m_segNum, m_framesNum);
                if (ret)
                    return ret;
            } else {
                //trace
                const char *dashMode = "static";
                float currFrameRate = (float)(m_frameRate.num) / (float)(m_frameRate.den);
                tracepoint(bandwidth_tp_provider, segmentation_info,
                    dashMode, m_segInfo->segDuration, currFrameRate,
                    m_videosNum, m_videosBitrate,
                    m_framesNum, m_segNum);

                int32_t ret = m_mpdGen->WriteMpd(m_framesNum);
                if (ret)
                    return ret;
            }
            LOG(INFO) << "Total  " << m_framesNum << " frames written into segments!" << std::endl;
            //return ERROR_NONE;
            break;
        }
        m_framesNum++;
    }

    return ERROR_NONE;
}

int32_t DefaultSegmentation::EndEachVideo(MediaStream *stream)
{
    if (!stream)
        return OMAF_ERROR_NULL_PTR;

    VideoStream *vs = (VideoStream*)stream;
    vs->SetEOS(true);

    return ERROR_NONE;
}

VCD_NS_END
