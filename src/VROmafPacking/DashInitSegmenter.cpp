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

VCD_NS_BEGIN


DashInitSegmenter::DashInitSegmenter(InitSegConfig *aConfig)
    : m_config(*aConfig)
{
    for (auto& trackIdAndConfig : m_config.tracks)
    {
        m_firstFrameRemaining.insert(trackIdAndConfig.first);
    }
}

DashInitSegmenter::~DashInitSegmenter() = default;

StreamSegmenter::Segmenter::InitSegment DashInitSegmenter::MakeInitSegment(bool flagFrag)
{
    StreamSegmenter::Segmenter::MovieDescription des_of_mov;
    des_of_mov.creationTime = 0;
    des_of_mov.modificationTime = 0;
    des_of_mov.matrix = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };
    StreamSegmenter::BrandSpec typeOfFile = { std::string("isom"), 512, { "isom", "iso6" } }; //Should be iso9 ?
    if (!m_omafVideoTrackBrand.empty())
    {
        typeOfFile.compatibleBrands.push_back(m_omafVideoTrackBrand);
    }
    if (!m_omafAudioTrackBrand.empty())
    {
        typeOfFile.compatibleBrands.push_back(m_omafAudioTrackBrand);
    }
    des_of_mov.fileType = typeOfFile;
    auto segInit = StreamSegmenter::Segmenter::makeInitSegment(m_trackDescriptions, des_of_mov, flagFrag);
    return segInit;
}

void DashInitSegmenter::AddH264VideoTrack(TrackId indexTracked, CodedMeta& metaData)
{
    std::vector<std::uint8_t> sps = metaData.decoderConfig.at(ConfigType::SPS);
    std::vector<std::uint8_t> pps = metaData.decoderConfig.at(ConfigType::PPS);
    StreamSegmenter::TrackMeta trackMeta = m_config.tracks.at(indexTracked).meta;
    StreamSegmenter::Segmenter::MediaDescription mediaDescription;
    //memset(&(mediaDescription), 0, sizeof(StreamSegmenter::Segmenter::MediaDescription));
    mediaDescription.creationTime = 0;
    mediaDescription.modificationTime = 0;

    StreamSegmenter::Segmenter::AvcVideoSampleEntry avcVidEnter;

    avcVidEnter.width = metaData.width;
    avcVidEnter.height = metaData.height;

    avcVidEnter.sps = sps;
    avcVidEnter.pps = pps;

    m_trackDescriptions.insert(std::make_pair(indexTracked, StreamSegmenter::Segmenter::TrackDescription(trackMeta, mediaDescription, avcVidEnter)));
}

void DashInitSegmenter::AddH265VideoTrack(TrackId indexTracked, CodedMeta& metaData)
{
    std::vector<std::uint8_t> vector_sps = metaData.decoderConfig.at(ConfigType::SPS);
    std::vector<std::uint8_t> vector_pps = metaData.decoderConfig.at(ConfigType::PPS);
    std::vector<std::uint8_t> vector_vps = metaData.decoderConfig.at(ConfigType::VPS);
    StreamSegmenter::TrackMeta metaDataTracked = m_config.tracks.at(indexTracked).meta;
    StreamSegmenter::Segmenter::MediaDescription mediaDescription;
    mediaDescription.creationTime = 0;
    mediaDescription.modificationTime = 0;
    StreamSegmenter::Segmenter::HevcVideoSampleEntry hevcVidEnter{};

    hevcVidEnter.width = metaData.width;
    hevcVidEnter.height = metaData.height;
    hevcVidEnter.framerate = metaData.duration.per1().asDouble();

    hevcVidEnter.sps = vector_sps;
    hevcVidEnter.pps = vector_pps;
    hevcVidEnter.vps = vector_vps;

    FillOmafStructures(indexTracked, metaData, hevcVidEnter, metaDataTracked);

    m_trackDescriptions.insert(std::make_pair(indexTracked, StreamSegmenter::Segmenter::TrackDescription(metaDataTracked, mediaDescription, hevcVidEnter)));
}

void DashInitSegmenter::AddH265ExtractorTrack(TrackId indexTracked, CodedMeta& metaData)
{
    std::vector<std::uint8_t> vector_sps = metaData.decoderConfig.at(ConfigType::SPS);
    std::vector<std::uint8_t> vector_pps = metaData.decoderConfig.at(ConfigType::PPS);
    std::vector<std::uint8_t> vector_vps = metaData.decoderConfig.at(ConfigType::VPS);
    StreamSegmenter::TrackMeta trackMeta = m_config.tracks.at(indexTracked).meta;
    StreamSegmenter::Segmenter::MediaDescription infoMedia;
    infoMedia.creationTime = 0;
    infoMedia.modificationTime = 0;
    StreamSegmenter::Segmenter::HevcVideoSampleEntry hevcVidEnter{};

    hevcVidEnter.width = metaData.width;
    hevcVidEnter.height = metaData.height;
    hevcVidEnter.framerate = metaData.duration.per1().asDouble();
    hevcVidEnter.sampleEntryType = "hvc2";

    hevcVidEnter.sps = vector_sps;
    hevcVidEnter.pps = vector_pps;
    hevcVidEnter.vps = vector_vps;

    FillOmafStructures(indexTracked, metaData, hevcVidEnter, trackMeta);

    auto& track = m_config.tracks.at(indexTracked);
    auto trackDescription = StreamSegmenter::Segmenter::TrackDescription(trackMeta, infoMedia, hevcVidEnter);
    trackDescription.trackReferences = track.trackReferences;
    m_trackDescriptions.insert(std::make_pair(indexTracked, std::move(trackDescription)));
}

int32_t DashInitSegmenter::GenerateInitSegment(
    TrackSegmentCtx *trackSegCtx,
    std::map<TrackId, TrackSegmentCtx*> tileTrackSegCtxs)
{
    TrackId trackId = trackSegCtx->trackIdx;

    bool hadFirstFramesRemaining = m_firstFrameRemaining.size();
    bool endOfStream = trackSegCtx->isEOS;
    Optional<CodedMeta> codedMeta;
    if (!(trackSegCtx->isExtractorTrack))
    {
        if (!endOfStream && m_firstFrameRemaining.count(trackId))
        {

            codedMeta = trackSegCtx->codedMeta;

            switch (codedMeta->format)
            {
            case CodedFormat::H264:
                AddH264VideoTrack(trackId, *codedMeta);
                break;
            case CodedFormat::H265:
                AddH265VideoTrack(trackId, *codedMeta);
                break;
            case CodedFormat::AAC:
                return OMAF_ERROR_UNDEFINED_OPERATION;
            case CodedFormat::TimedMetadata:
                return OMAF_ERROR_UNDEFINED_OPERATION;
            case CodedFormat::H265Extractor:
                AddH265ExtractorTrack(trackId, *codedMeta);
                break;
            case CodedFormat::NoneFormat:
                return OMAF_ERROR_UNDEFINED_OPERATION;
            }
        }
    }
    else
    {
        for (auto& normalTrack : m_config.tracks)
        {
            std::map<TrackId, TrackSegmentCtx*>::iterator itTrack;
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
                    case CodedFormat::H264:
                        AddH264VideoTrack(normalTrack.first, *codedMeta);
                        break;
                    case CodedFormat::H265:
                        AddH265VideoTrack(normalTrack.first, *codedMeta);
                        break;
                    case CodedFormat::AAC:
                        return OMAF_ERROR_UNDEFINED_OPERATION;
                    case CodedFormat::TimedMetadata:
                        return OMAF_ERROR_UNDEFINED_OPERATION;
                    case CodedFormat::H265Extractor:
                        AddH265ExtractorTrack(normalTrack.first, *codedMeta);
                        break;
                    case CodedFormat::NoneFormat:
                        return OMAF_ERROR_UNDEFINED_OPERATION;
                    }
                }
                m_firstFrameRemaining.erase(normalTrack.first);
            }
            else
            {
                if (trackId.get() != normalTrack.first.get())
                    return OMAF_ERROR_INVALID_TRACKSEG_CTX;

                if (!endOfStream && m_firstFrameRemaining.count(trackId))
                {

                    codedMeta = trackSegCtx->codedMeta;

                    switch (codedMeta->format)
                    {
                    case CodedFormat::H264:
                        AddH264VideoTrack(trackId, *codedMeta);
                        break;
                    case CodedFormat::H265:
                        AddH265VideoTrack(trackId, *codedMeta);
                        break;
                    case CodedFormat::AAC:
                        return OMAF_ERROR_UNDEFINED_OPERATION;
                    case CodedFormat::TimedMetadata:
                        return OMAF_ERROR_UNDEFINED_OPERATION;
                    case CodedFormat::H265Extractor:
                        AddH265ExtractorTrack(trackId, *codedMeta);
                        break;
                    case CodedFormat::NoneFormat:
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
                std::ostringstream frameStream;
                StreamSegmenter::Segmenter::writeInitSegment(frameStream, MakeInitSegment(m_config.fragmented));
                std::string frameString(frameStream.str());

                FILE *fp = fopen(trackSegCtx->dashInitCfg.initSegName, "wb+");
                if (!fp)
                    return OMAF_ERROR_NULL_PTR;
                fwrite(frameString.c_str(), 1, frameString.size(), fp);
                fclose(fp);
                fp = NULL;
            }
        }
    }

    return ERROR_NONE;
}

void DashInitSegmenter::FillOmafStructures(
    TrackId indexTracked,
    CodedMeta& metaData,
    StreamSegmenter::Segmenter::HevcVideoSampleEntry& hevcVidEnter,
    StreamSegmenter::TrackMeta& metaTracked)
{
    if (m_config.mode == OperatingMode::OMAF)
    {
        if (metaData.projection == OmafProjectionType::EQUIRECTANGULAR)
        {
            hevcVidEnter.projectionFormat = StreamSegmenter::Segmenter::ProjectionFormat::Equirectangular;
        }
        else
        {
            hevcVidEnter.projectionFormat = StreamSegmenter::Segmenter::ProjectionFormat::Cubemap;
        }
        if (m_config.packedSubPictures)
        {
            // viewport dependent
            StreamSegmenter::BrandSpec infoBrandSpec = { std::string("hevd"), 0,{ "hevd" } };
            metaTracked.trackType = infoBrandSpec;
            hevcVidEnter.compatibleSchemes.push_back({ "podv", 0, "" });
            hevcVidEnter.compatibleSchemes.push_back({ "ercm", 0, "" });
            m_omafVideoTrackBrand = "hevd";
        }
        else
        {
            StreamSegmenter::BrandSpec infoBrandSpec = { std::string("hevi"), 0,{ "hevi" } };
            metaTracked.trackType = infoBrandSpec;
            hevcVidEnter.compatibleSchemes.push_back({ "podv", 0, "" });
            hevcVidEnter.compatibleSchemes.push_back({ "erpv", 0, "" });
            m_omafVideoTrackBrand = "hevi";
        }

        if (m_config.tracks.at(indexTracked).pipelineOutput == DataInputFormat::VideoTopBottom)
        {
            hevcVidEnter.stvi = StreamSegmenter::Segmenter::PodvStereoVideoInfo::TopBottomPacking;
        }
        else if (m_config.tracks.at(indexTracked).pipelineOutput == DataInputFormat::VideoSideBySide)
        {
            hevcVidEnter.stvi = StreamSegmenter::Segmenter::PodvStereoVideoInfo::SideBySidePacking;
        }
        // temporal interleaving not supported


        if (metaData.sphericalCoverage)
        {
            hevcVidEnter.covi = StreamSegmenter::Segmenter::CoverageInformation();
            if (metaData.projection == OmafProjectionType::EQUIRECTANGULAR)
            {
                hevcVidEnter.covi->coverageShape = StreamSegmenter::Segmenter::CoverageInformationShapeType::TwoAzimuthAndTwoElevationCircles;
            }
            else
            {
                hevcVidEnter.covi->coverageShape = StreamSegmenter::Segmenter::CoverageInformationShapeType::FourGreatCircles;
            }
            if (hevcVidEnter.stvi)
            {
                hevcVidEnter.covi->defaultViewIdc = StreamSegmenter::Segmenter::ViewIdc::LEFT_AND_RIGHT;
            }
            else
            {
                hevcVidEnter.covi->defaultViewIdc = StreamSegmenter::Segmenter::ViewIdc::MONOSCOPIC;
            }
            hevcVidEnter.covi->viewIdcPresenceFlag = false;

            auto outputRegion = std::unique_ptr<StreamSegmenter::Segmenter::CoverageInformationRegion>(new StreamSegmenter::Segmenter::CoverageInformationRegion());
            auto& covOfSph = metaData.sphericalCoverage.get();
            outputRegion->centreAzimuth = covOfSph.cAzimuth;
            outputRegion->centreElevation = covOfSph.cElevation;
            outputRegion->centreTilt = covOfSph.cTilt;
            outputRegion->azimuthRange = covOfSph.rAzimuth;
            outputRegion->elevationRange = covOfSph.rElevation;
            outputRegion->interpolate = false;
            hevcVidEnter.covi->regions.push_back(std::move(outputRegion));
        }

        if (metaData.regionPacking)
        {
            hevcVidEnter.rwpk = StreamSegmenter::Segmenter::RegionWisePacking();

            auto& packArea = metaData.regionPacking.get();
            hevcVidEnter.rwpk->constituenPictureMatchingFlag = packArea.constituentPictMatching;
            hevcVidEnter.rwpk->projPictureHeight = packArea.projPictureHeight;
            hevcVidEnter.rwpk->projPictureWidth = packArea.projPictureWidth;
            hevcVidEnter.rwpk->packedPictureHeight = packArea.packedPictureHeight;
            hevcVidEnter.rwpk->packedPictureWidth = packArea.packedPictureWidth;
            for (auto& iutputRegion : packArea.regions)
            {
                auto outputRegion = std::unique_ptr<StreamSegmenter::Segmenter::RwpkRectRegion>(new StreamSegmenter::Segmenter::RwpkRectRegion());
                outputRegion->packedTop = iutputRegion.packedTop;
                outputRegion->packedLeft = iutputRegion.packedLeft;
                outputRegion->packedWidth = iutputRegion.packedWidth;
                outputRegion->packedHeight = iutputRegion.packedHeight;

                outputRegion->projTop = iutputRegion.projTop;
                outputRegion->projLeft = iutputRegion.projLeft;
                outputRegion->projWidth = iutputRegion.projWidth;
                outputRegion->projHeight = iutputRegion.projHeight;

                outputRegion->transformType = iutputRegion.transform;

                hevcVidEnter.rwpk->regions.push_back(std::move(outputRegion));
            }
        }
    }
}

VCD_NS_END
