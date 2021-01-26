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

DashInitSegmenter::~DashInitSegmenter() = default;

VCD::MP4::InitialSegment DashInitSegmenter::MakeInitSegment(bool isFraged)
{
    VCD::MP4::MovieDescription moovDes;
    moovDes.creationTime = 0;
    moovDes.modificationTime = 0;
    vector<int32_t> tempVec(16, 0);
    moovDes.matrix = tempVec;
    moovDes.matrix[0]  = 1;
    moovDes.matrix[5]  = 1;
    moovDes.matrix[10] = 1;
    moovDes.matrix[15] = 1;

    VCD::MP4::BrandSpec brandSpec = { string("isom"), 512, { "isom", "iso6" } }; //Should be iso9 ?
    if (!m_omafVideoTrackBrand.empty())
    {
        brandSpec.compatibleBrands.push_back(m_omafVideoTrackBrand);
    }
    if (!m_omafAudioTrackBrand.empty())
    {
        brandSpec.compatibleBrands.push_back(m_omafAudioTrackBrand);
    }
    moovDes.fileType = brandSpec;
    VCD::MP4::InitialSegment initialSeg = VCD::MP4::GenInitSegment(m_trackDescriptions, moovDes, isFraged);
    return initialSeg;
}

void DashInitSegmenter::AddH264VideoTrack(VCD::MP4::TrackId trackId, CodedMeta& inMetaData)
{
    vector<uint8_t> avcSPS = inMetaData.decoderConfig.at(ConfigType::SPS);
    vector<uint8_t> avcPPS = inMetaData.decoderConfig.at(ConfigType::PPS);
    VCD::MP4::TrackMeta trackMeta = m_config.tracks.at(trackId).meta;
    VCD::MP4::FileInfo trackFileInfo;
    trackFileInfo.creationTime = 0;
    trackFileInfo.modificationTime = 0;
    VCD::MP4::AvcVideoSampleEntry avcEntry;

    avcEntry.width = inMetaData.width;
    avcEntry.height = inMetaData.height;

    avcEntry.sps = avcSPS;
    avcEntry.pps = avcPPS;

    m_trackDescriptions.insert(make_pair(trackId, VCD::MP4::TrackDescription(trackMeta, trackFileInfo, avcEntry)));
}

void DashInitSegmenter::AddH265VideoTrack(VCD::MP4::TrackId trackId, CodedMeta& inMetaData)
{
    vector<uint8_t> hevcSPS = inMetaData.decoderConfig.at(ConfigType::SPS);
    vector<uint8_t> hevcPPS = inMetaData.decoderConfig.at(ConfigType::PPS);
    vector<uint8_t> hevcVPS = inMetaData.decoderConfig.at(ConfigType::VPS);
    VCD::MP4::TrackMeta trackMeta = m_config.tracks.at(trackId).meta;
    VCD::MP4::FileInfo trackFileInfo;
    trackFileInfo.creationTime = 0;
    trackFileInfo.modificationTime = 0;
    VCD::MP4::HevcVideoSampleEntry hevcEntry{};

    hevcEntry.width = inMetaData.width;
    hevcEntry.height = inMetaData.height;
    hevcEntry.frameRate = inMetaData.duration.per1().asDouble();

    hevcEntry.sps = hevcSPS;
    hevcEntry.pps = hevcPPS;
    hevcEntry.vps = hevcVPS;

    FillOmafStructures(trackId, inMetaData, hevcEntry, trackMeta);

    m_trackDescriptions.insert(make_pair(trackId, VCD::MP4::TrackDescription(trackMeta, trackFileInfo, hevcEntry)));
}

void DashInitSegmenter::AddH265ExtractorTrack(VCD::MP4::TrackId trackId, CodedMeta& inMetaData)
{
    vector<uint8_t> hevcSPS = inMetaData.decoderConfig.at(ConfigType::SPS);
    vector<uint8_t> hevcPPS = inMetaData.decoderConfig.at(ConfigType::PPS);
    vector<uint8_t> hevcVPS = inMetaData.decoderConfig.at(ConfigType::VPS);
    VCD::MP4::TrackMeta trackMeta = m_config.tracks.at(trackId).meta;
    VCD::MP4::FileInfo trackFileInfo;
    trackFileInfo.creationTime = 0;
    trackFileInfo.modificationTime = 0;
    VCD::MP4::HevcVideoSampleEntry hevcEntry{};

    hevcEntry.width = inMetaData.width;
    hevcEntry.height = inMetaData.height;
    hevcEntry.frameRate = inMetaData.duration.per1().asDouble();
    hevcEntry.sampleEntryType = "hvc2";

    hevcEntry.sps = hevcSPS;
    hevcEntry.pps = hevcPPS;
    hevcEntry.vps = hevcVPS;

    FillOmafStructures(trackId, inMetaData, hevcEntry, trackMeta);

    using TrackId = VCD::MP4::TrackId;
    map<TrackId, TrackConfig>::const_iterator iter = m_config.tracks.find(trackId);
    if (iter == m_config.tracks.end())
    {
        OMAF_LOG(LOG_ERROR, "Can't find specified track index !\n");
        return;
    }
    TrackConfig trackCfg = iter->second;
    VCD::MP4::TrackDescription trackDes = VCD::MP4::TrackDescription(trackMeta, trackFileInfo, hevcEntry);
    trackDes.trackReferences = trackCfg.trackReferences;
    m_trackDescriptions.insert(make_pair(trackId, move(trackDes)));
}

void DashInitSegmenter::AddAACTrack(VCD::MP4::TrackId trackId, CodedMeta& inMetaData)
{
    std::vector<uint8_t> audioSpecInfo = inMetaData.decoderConfig.at(ConfigType::AudioSpecificConfig);
    VCD::MP4::TrackMeta trackMeta = m_config.tracks.at(trackId).meta;
    VCD::MP4::FileInfo trackFileInfo;
    trackFileInfo.creationTime = 0;
    trackFileInfo.modificationTime = 0;
    VCD::MP4::MP4AudioSampleEntry sampleEntry;

    sampleEntry.sizeOfSample = 16;
    sampleEntry.cntOfChannels = inMetaData.channelCfg;
    sampleEntry.rateOfSample = inMetaData.samplingFreq;
    sampleEntry.idOfES = 1;
    sampleEntry.esIdOfDepends = 0;
    sampleEntry.sizeOfBuf = 0;
    sampleEntry.maxBitrate = inMetaData.bitrate.maxBitrate;
    sampleEntry.avgBitrate = inMetaData.bitrate.avgBitrate;
    for (auto byte : audioSpecInfo)
    {
        sampleEntry.decSpecificInfo.push_back(static_cast<char>(byte));
    }

    if (m_config.mode == OperatingMode::OMAF)
    {
        m_omafAudioTrackBrand = "oa2d";
    }

    m_trackDescriptions.insert(make_pair(trackId, VCD::MP4::TrackDescription(trackMeta, trackFileInfo, sampleEntry)));
    OMAF_LOG(LOG_INFO, "Done adding AAC track !\n");
}

int32_t DashInitSegmenter::GenerateInitSegment(
    TrackSegmentCtx *trackSegCtx,
    map<VCD::MP4::TrackId, TrackSegmentCtx*> tileTrackSegCtxs)
{
    VCD::MP4::TrackId trackId = trackSegCtx->trackIdx;
    OMAF_LOG(LOG_INFO, "Generate initial segment for track %d!\n", trackId.GetIndex());
    bool hadFirstFramesRemaining = m_firstFrameRemaining.size();
    bool endOfStream = trackSegCtx->codedMeta.isEOS;
    VCD::MP4::DataItem<CodedMeta> codedMeta;
    //OMAF_LOG(LOG_INFO, "Is audio %d\n", trackSegCtx->isAudio);
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
                //OMAF_LOG(LOG_INFO, "To add AAC track !\n");
                AddAACTrack(trackId, *codedMeta);
                break;
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
                    case CodedFormat::H264:
                        AddH264VideoTrack(normalTrack.first, *codedMeta);
                        break;
                    case CodedFormat::H265:
                        AddH265VideoTrack(normalTrack.first, *codedMeta);
                        break;
                    case CodedFormat::AAC:
                        AddAACTrack(normalTrack.first, *codedMeta);
                        break;
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
                if (trackId.GetIndex() != normalTrack.first.GetIndex())
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
                        AddAACTrack(trackId, *codedMeta);
                        break;
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
            //OMAF_LOG(LOG_INFO, "WRITE TO BS FOR track %d\n", trackId.GetIndex());
            if (!endOfStream)
            {
                ostringstream frameStream;
                VCD::MP4::WriteInitSegment(frameStream, MakeInitSegment(m_config.fragmented));
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

void DashInitSegmenter::FillOmafStructures(
    VCD::MP4::TrackId inTrackId,
    CodedMeta& inMetaData,
    VCD::MP4::HevcVideoSampleEntry& hevcEntry,
    VCD::MP4::TrackMeta& inMeta)
{
    if (m_config.mode == OperatingMode::OMAF)
    {
        if (inMetaData.projection == OmafProjectionType::EQUIRECTANGULAR)
        {
            hevcEntry.projFmt = VCD::MP4::OmniProjFormat::OMNI_ERP;
        }
        else if (inMetaData.projection == OmafProjectionType::CUBEMAP)
        {
            hevcEntry.projFmt = VCD::MP4::OmniProjFormat::OMNI_Cubemap;
        }
        else if (inMetaData.projection == OmafProjectionType::PLANAR)
        {
            hevcEntry.projFmt = VCD::MP4::OmniProjFormat::OMNI_Planar;
        }

        if (m_config.packedSubPictures)
        {
            // viewport dependent
            VCD::MP4::BrandSpec trackType = { string("&apos;hevd&apos;"), 0,{ "&apos;hevd&apos;" } };
            inMeta.trackType = trackType;
            hevcEntry.compatibleSchemes.push_back({ "podv", 0, "" });
            hevcEntry.compatibleSchemes.push_back({ "ercm", 0, "" });
            m_omafVideoTrackBrand = "&apos;hevd&apos;";
        }
        else
        {
            VCD::MP4::BrandSpec trackType = { string("hevi"), 0,{ "hevi" } };
            inMeta.trackType = trackType;
            hevcEntry.compatibleSchemes.push_back({ "podv", 0, "" });
            hevcEntry.compatibleSchemes.push_back({ "erpv", 0, "" });
            m_omafVideoTrackBrand = "hevi";
        }

        if (m_config.tracks.at(inTrackId).pipelineOutput == DataInputFormat::VideoTopBottom)
        {
            hevcEntry.stvi = VCD::MP4::VideoFramePackingType::OMNI_TOPBOTTOM;
        }
        else if (m_config.tracks.at(inTrackId).pipelineOutput == DataInputFormat::VideoSideBySide)
        {
            hevcEntry.stvi = VCD::MP4::VideoFramePackingType::OMNI_SIDEBYSIDE;
        }

        if (inMetaData.sphericalCoverage)
        {
            hevcEntry.covi = VCD::MP4::CoverageInformation();
            if (inMetaData.projection == OmafProjectionType::EQUIRECTANGULAR)
            {
                hevcEntry.covi->coverageShape = VCD::MP4::COVIShapeType::TWO_AZIMUTH_AND_TWO_ELEVATION_CIRCLES;
            }
            else
            {
                hevcEntry.covi->coverageShape = VCD::MP4::COVIShapeType::FOUR_GREAT_CIRCLES;
            }
            if (hevcEntry.stvi)
            {
                hevcEntry.covi->defaultViewIdc = VCD::MP4::OmniViewIdc::OMNI_LEFT_AND_RIGHT;
            }
            else
            {
                hevcEntry.covi->defaultViewIdc = VCD::MP4::OmniViewIdc::OMNI_MONOSCOPIC;
            }
            hevcEntry.covi->viewIdcPresenceFlag = false;

            auto coviReg = unique_ptr<VCD::MP4::COVIRegion>(new VCD::MP4::COVIRegion());
            auto& sphericalCoverage = inMetaData.sphericalCoverage.get();
            coviReg->centAzimuth = sphericalCoverage.cAzimuth;
            coviReg->centElevation = sphericalCoverage.cElevation;
            coviReg->centTilt = sphericalCoverage.cTilt;
            coviReg->azimuthRange = sphericalCoverage.rAzimuth;
            coviReg->elevationRange = sphericalCoverage.rElevation;
            coviReg->interpolate = false;
            hevcEntry.covi->sphereRegions.push_back(move(coviReg));
        }

        if (inMetaData.regionPacking)
        {
            hevcEntry.rwpk = VCD::MP4::RegionWisePacking();

            auto& regionPacking = inMetaData.regionPacking.get();
            hevcEntry.rwpk->constituenPicMatching = regionPacking.constituentPictMatching;
            hevcEntry.rwpk->projPicHeight = regionPacking.projPictureHeight;
            hevcEntry.rwpk->projPicWidth = regionPacking.projPictureWidth;
            hevcEntry.rwpk->packedPicHeight = regionPacking.packedPictureHeight;
            hevcEntry.rwpk->packedPicWidth = regionPacking.packedPictureWidth;
            for (auto& regionIn : regionPacking.regions)
            {
                auto rwpkReg = unique_ptr<VCD::MP4::RwpkRectRegion>(new VCD::MP4::RwpkRectRegion());
                rwpkReg->packedRegTop = regionIn.packedTop;
                rwpkReg->packedRegLeft = regionIn.packedLeft;
                rwpkReg->packedRegWidth = regionIn.packedWidth;
                rwpkReg->packedRegHeight = regionIn.packedHeight;

                rwpkReg->projRegTop = regionIn.projTop;
                rwpkReg->projRegLeft = regionIn.projLeft;
                rwpkReg->projRegWidth = regionIn.projWidth;
                rwpkReg->projRegHeight = regionIn.projHeight;

                rwpkReg->transformType = regionIn.transform;

                hevcEntry.rwpk->regions.push_back(std::move(rwpkReg));
            }
        }
    }
}

VCD_NS_END
