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
//! \file:   SegmentWriter.cpp
//! \brief:  Writer related operation implementation
//!

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "Frame.h"
#include "AvcConfigAtom.h"
#include "Utils.h"

#include "BoxBlockAccess.h"

#include "AvcSampEntry.h"
#include "TypeAtom.h"
#include "HevcConfigAtom.h"
#include "HevcSampEntry.h"
#include "InitViewOrientationSampEntry.h"
#include "MediaDataAtom.h"
#include "MovieAtom.h"
#include "MovieFragAtom.h"
#include "Mp4AudSampEntryAtom.h"
#include "SegIndexAtom.h"
#include "UriMetaSampEntryAtom.h"
#include "UserDataAtom.h"

#include "AcquireTrackData.h"
#include "BoxWrapper.h"
#include "SegmentWriter.h"

using namespace std;

VCD_MP4_BEGIN

using TimeInterval = pair<FrameTime, FrameTime>;

typedef vector<TrackId> TrackIds;

struct Mp4MoofInfo
{
    TrackInfo trackInfo;
    int32_t moofToDataOffset;
};

typedef map<TrackId, Mp4MoofInfo> Mp4MoofInfos;

void WriteMoof(ostream& outStr,
               const TrackIds& trackIndex,
               const Segment& oneSeg,
               const Mp4MoofInfos& moofInfos,
               const map<TrackId, Frames>& framesMap)
{
    std::vector<SampleDefaults> sampleDefaults;
    for (auto trackId : trackIndex)
    {
        sampleDefaults.push_back(SampleDefaults{
            trackId.GetIndex(),
            1,
            0,
            0,
            {0}
        });
    }
    MovieFragmentAtom moof(sampleDefaults);
    moof.GetMovieFragmentHeaderAtom().SetSequenceNumber(oneSeg.sequenceId.GetIndex());
    for (auto trackId : trackIndex)
    {
        if (moofInfos.find(trackId) == moofInfos.end())
        {
            ISO_LOG(LOG_ERROR, "Failed to find moof info for designated track !\n");
            throw exception();
        }
        const auto& segmentMoofInfo = moofInfos.find(trackId)->second;
        const auto& trackMeta       = segmentMoofInfo.trackInfo.trackMeta;
        auto traf                   = MakeUnique<TrackFragmentAtom, TrackFragmentAtom>(sampleDefaults);
        auto trun                   = MakeUnique<TrackRunAtom, TrackRunAtom>(
            uint8_t(1), 0u | TrackRunAtom::TrackRunFlags::pSampleDuration |
                            TrackRunAtom::TrackRunFlags::pSampleSize |
                            TrackRunAtom::TrackRunFlags::pSampleCompTimeOffsets |
                            TrackRunAtom::TrackRunFlags::pSampleFlags);
        auto tfdt =
            MakeUnique<TrackFragmentBaseMediaDecodeTimeAtom, TrackFragmentBaseMediaDecodeTimeAtom>();

        tfdt->SetBaseMediaDecodeTime(
            uint64_t((segmentMoofInfo.trackInfo.tBegin.cast<FractU64>() / trackMeta.timescale).asDouble()));
        assert(segmentMoofInfo.trackInfo.tBegin >= FrameTime(0, 1));
        traf->GetTrackFragmentHeaderAtom().SetTrackId(trackMeta.trackId.GetIndex());
        traf->GetTrackFragmentHeaderAtom().SetFlags(0 | TrackFragmentHeaderAtom::IsBaseMoof);

        trun->SetDataOffset(segmentMoofInfo.moofToDataOffset);

        FrameTime time = segmentMoofInfo.trackInfo.tBegin;

        if (framesMap.find(trackId) == framesMap.end())
        {
            ISO_LOG(LOG_ERROR, "Can't find frame with designated trackId !\n");
            throw exception();
        }

        for (const auto& frame : framesMap.find(trackId)->second)
        {
            uint64_t frameSize = frame.GetSize();
            FrameInfo frameInfo     = frame.GetFrameInfo();

            TrackRunAtom::SampleDetails s;
            s.version1.pDuration =
                uint32_t((frameInfo.duration / trackMeta.timescale).asDouble());
            s.version1.pSize                  = uint32_t(frameSize);
            s.version1.pFlags                 = {frameInfo.sampleFlags.flagsAsUInt};
            s.version1.pCompTimeOffset = int32_t(
                ((frameInfo.cts.front() - time) / trackMeta.timescale.cast<FractS64>()).asDouble());
            trun->AddSampleDetails(s);

            time += frameInfo.duration.cast<FrameTime>();
        }
        trun->SetSampleNum((uint32_t) framesMap.find(trackId)->second.size());

        traf->SetTrackFragmentDecodeTimeAtom(move(tfdt));
        traf->AddTrackRunAtom(move(trun));
        moof.AddTrackFragmentAtom(move(traf));
    }

    Stream bs;
    moof.ToStream(bs);
    auto data = bs.GetStorage();
    outStr.write(reinterpret_cast<const char*>(&data[0]), streamsize(data.size()));
}

void FlushStream(Stream& inBS, ostream& outStr)
{
    auto data = inBS.GetStorage();
    outStr.write(reinterpret_cast<const char*>(&data[0]), streamsize(data.size()));
    inBS.Clear();
}

void CopyAtom(const Atom& srcAtom, Atom& dstAtom)
{
    Stream bs;
    const_cast<Atom&>(srcAtom).ToStream(bs);
    bs.Reset();
    dstAtom.FromStream(bs);
}

template <typename T>
UniquePtr<T> CloneAtom(const T& srcAtom)
{
    UniquePtr<T> dstAtom(new T);
    Stream bs;
    const_cast<T&>(srcAtom).ToStream(bs);
    bs.Reset();
    dstAtom->FromStream(bs);
    return dstAtom;
}

void FillTrackHeaderAtom(TrackHeaderAtom& theaAtom, const FileInfo& fileInfo)
{
    uint64_t creationTime     = fileInfo.creationTime;
    uint64_t modificationTime = fileInfo.modificationTime;
    FractU64 duration         = fileInfo.duration.GetMinimum();

    theaAtom.SetFlags(1 | 3);
    theaAtom.SetCreationTime(creationTime);
    theaAtom.SetModificationTime(modificationTime);
    theaAtom.SetDuration(uint32_t(duration.m_num));
}

void SetReferredTracks(TrackReferenceAtom& trefAtom,
                        const map<string, set<TrackId>>& trackReferences)
{
    for (auto& tref : trackReferences)
    {
        VectorT<uint32_t> trackIds;
        for (auto& track : tref.second)
        {
            trackIds.push_back(track.GetIndex());
        }
        TrackReferenceTypeAtom trefTypeAtom(FourCCInt{std::string{tref.first.begin(), tref.first.end()}});
        trefTypeAtom.SetTrackIds(trackIds);
        trefAtom.AddAtom(trefTypeAtom);
    }
}

InitialSegment::InitialSegment() = default;

InitialSegment::InitialSegment(const InitialSegment& initSeg)
{
    moov = NULL;
    if (initSeg.moov)
    {
        moov.reset(new MovieBoxWrapper{CloneAtom(*initSeg.moov->movieBox)});
    }

    ftyp = NULL;
    if (initSeg.ftyp)
    {
        ftyp.reset(new FileTypeBoxWrapper{CloneAtom(*initSeg.ftyp->fileTypeBox)});
    }
}

InitialSegment::~InitialSegment() = default;

InitialSegment& InitialSegment::operator=(const InitialSegment& initSeg)
{
    if (initSeg.moov)
    {
        moov.reset(new MovieBoxWrapper{CloneAtom(*initSeg.moov->movieBox)});
    }
    else
    {
        moov.reset();
    }
    return *this;
}

InitialSegment& InitialSegment::operator=(InitialSegment&& initSeg)
{
    moov = move(initSeg.moov);
    return *this;
}

void WriteSegmentHeader(ostream& outStr)
{
    SegmentTypeAtom stypAtom;
    Stream tempBS;

    stypAtom.SetMajorBrand("msdh");
    stypAtom.AddCompatibleBrand("msdh");
    stypAtom.AddCompatibleBrand("msix");
    stypAtom.ToStream(tempBS);

    FlushStream(tempBS, outStr);
}

void WriteSampleData(ostream& outStr, const Segment& oneSeg)
{
    TrackIds trackIds = Keys(oneSeg.tracks);
    map<TrackId, Frames> frameMap;

    map<TrackId, TrackOfSegment>::const_iterator iter = oneSeg.tracks.begin();
    for ( ; iter != oneSeg.tracks.end(); iter++)
    {
        frameMap.insert(make_pair(iter->first, iter->second.frames));
    }

    auto moofOffset = outStr.tellp();
    Mp4MoofInfos segMoofInfos;

    vector<TrackId>::iterator iter1 = trackIds.begin();
    for ( ; iter1 != trackIds.end(); iter1++)
    {
        Mp4MoofInfo moofInfo = {oneSeg.tracks.find(*iter1)->second.trackInfo, 0};
        segMoofInfos.insert(make_pair(*iter1, move(moofInfo)));
    }

    WriteMoof(outStr, trackIds, oneSeg, segMoofInfos, frameMap);

    vector<uint8_t> mdatHrd;
    mdatHrd.push_back(0);
    mdatHrd.push_back(0);
    mdatHrd.push_back(0);
    mdatHrd.push_back(0);
    mdatHrd.push_back(uint8_t('m'));
    mdatHrd.push_back(uint8_t('d'));
    mdatHrd.push_back(uint8_t('a'));
    mdatHrd.push_back(uint8_t('t'));
    auto mdatOffset = outStr.tellp();
    outStr.write(reinterpret_cast<const char*>(&mdatHrd[0]), streamsize(mdatHrd.size()));
    uint64_t mdatSize = mdatHrd.size();
    vector<TrackId>::iterator iter2 = trackIds.begin();
    for ( ; iter2 != trackIds.end(); iter2++)
    {
        Mp4MoofInfo moofInfo  = {oneSeg.tracks.find(*iter2)->second.trackInfo,
                                    int32_t(outStr.tellp() - streamoff(moofOffset))};
        segMoofInfos[*iter2] = move(moofInfo);
        if (frameMap.find(*iter2) == frameMap.end())
        {
            ISO_LOG(LOG_ERROR, "Failed to find frame with designated track Id !\n");
            throw exception();
        }
        for (const auto& frame : frameMap.find(*iter2)->second)
        {
            const auto& frameData = *frame;
            const auto& data      = frameData.frameBuf;
            outStr.write(reinterpret_cast<const char*>(&data[0]), streamsize(data.size()));
            mdatSize += data.size();
        }
    }
    streampos afterMdat = outStr.tellp();

    mdatHrd[0] = uint8_t((mdatSize >> 24) & 0xff);
    mdatHrd[1] = uint8_t((mdatSize >> 16) & 0xff);
    mdatHrd[2] = uint8_t((mdatSize >> 8) & 0xff);
    mdatHrd[3] = uint8_t((mdatSize >> 0) & 0xff);
    outStr.seekp(mdatOffset);
    outStr.write(reinterpret_cast<const char*>(&mdatHrd[0]), streamsize(4));

    outStr.seekp(moofOffset);
    WriteMoof(outStr, trackIds, oneSeg, segMoofInfos, frameMap);
    outStr.seekp(afterMdat);
}

void WriteInitSegment(ostream& outStr, const InitialSegment& initSegment)
{
    Stream stream;

    initSegment.ftyp->fileTypeBox->ToStream(stream);

    initSegment.moov->movieBox->ToStream(stream);

    auto data = stream.GetStorage();
    outStr.write(reinterpret_cast<const char*>(&data[0]), streamsize(data.size()));
}

TrackDescription::TrackDescription() = default;
TrackDescription::TrackDescription(TrackDescription&& other)
    : trackMeta(move(other.trackMeta))
    , sampleEntryBoxes(move(other.sampleEntryBoxes))
    , mediaHeaderBox(move(other.mediaHeaderBox))
    , handlerBox(move(other.handlerBox))
    , trackHeaderBox(move(other.trackHeaderBox))
    , trackReferences(move(other.trackReferences))
    , alternateGroup(move(other.alternateGroup))
{
}

TrackDescription::TrackDescription(TrackMeta inTrackMeta,
                                   list<unique_ptr<SampleEntryBoxWrapper>> smpEtyBoxList,
                                   unique_ptr<MediaHeaderBoxWrapper>&& mheaBox,
                                   unique_ptr<HandlerBoxWrapper>&& hdlrBox,
                                   unique_ptr<TrackHeaderBoxWrapper>&& theaBox)
    : trackMeta(inTrackMeta)
    , sampleEntryBoxes(move(smpEtyBoxList))
    , mediaHeaderBox(move(mheaBox))
    , handlerBox(move(hdlrBox))
    , trackHeaderBox(move(theaBox))
{
}

TrackDescription::~TrackDescription() = default;

OmniSampleEntry::OmniSampleEntry()  = default;
OmniSampleEntry::~OmniSampleEntry() = default;

uint8_t RwpkRectRegion::packingType() const
{
    return 0;
}

unique_ptr<RegionBlock>
RwpkRectRegion::GenRegion() const
{
    auto outReg         = MakeUnique<RegionWisePackingAtom::Region, RegionWisePackingAtom::Region>();
    outReg->packingType = (RegionWisePackingAtom::PackingType) packingType();
    outReg->rectangularPacking = MakeUnique<RegionWisePackingAtom::RectangularRegionWisePacking,
                                                     RegionWisePackingAtom::RectangularRegionWisePacking>();
    outReg->rectangularPacking->projRegWidth    = projRegWidth;
    outReg->rectangularPacking->projRegHeight   = projRegHeight;
    outReg->rectangularPacking->projRegTop      = projRegTop;
    outReg->rectangularPacking->projRegLeft     = projRegLeft;
    outReg->rectangularPacking->packedRegWidth  = packedRegWidth;
    outReg->rectangularPacking->packedRegHeight = packedRegHeight;
    outReg->rectangularPacking->packedRegTop    = packedRegTop;
    outReg->rectangularPacking->packedRegLeft   = packedRegLeft;
    outReg->rectangularPacking->transformType   = transformType;

    outReg->guardBandFlag = false;
    if (rwpkGuardBand)
    {
        outReg->guardBandFlag                            = true;
        outReg->rectangularPacking->gbNotUsedForPredFlag = rwpkGuardBand->gbNotUsedForPredFlag;
        outReg->rectangularPacking->topGbHeight          = rwpkGuardBand->topGbHeight;
        outReg->rectangularPacking->leftGbWidth          = rwpkGuardBand->leftGbWidth;
        outReg->rectangularPacking->bottomGbHeight       = rwpkGuardBand->bottomGbHeight;
        outReg->rectangularPacking->rightGbWidth         = rwpkGuardBand->rightGbWidth;
        outReg->rectangularPacking->gbType0              = rwpkGuardBand->gbType0;
        outReg->rectangularPacking->gbType1              = rwpkGuardBand->gbType1;
        outReg->rectangularPacking->gbType2              = rwpkGuardBand->gbType2;
        outReg->rectangularPacking->gbType3              = rwpkGuardBand->gbType3;
    }

    return MakeUnique<RegionBlock>(
        StaticCast<RegionWisePackingAtom::Region>(move(outReg)));
}

uint32_t VideoSampleEntry::GetWidthFP() const
{
    uint32_t fixedPntW = uint32_t(width) << 16;
    return fixedPntW;
}

uint32_t VideoSampleEntry::GetHeightFP() const
{
    uint32_t fixedPntH = uint32_t(height) << 16;
    return fixedPntH;
}

void VideoSampleEntry::GenPovdBoxes(unique_ptr<SampleEntryBoxWrapper>& box) const
{
    if (projFmt)
    {
        auto rinfAtom = MakeUnique<RestrictedSchemeInfoAtom, RestrictedSchemeInfoAtom>();

        rinfAtom->SetOriginalFormat(box->sampleEntryBox->GetType());

        auto schemeType = MakeUnique<SchemeTypeAtom, SchemeTypeAtom>();
        schemeType->SetSchemeType("podv");
        rinfAtom->AddSchemeTypeAtom(move(schemeType));

        auto povdAtom = MakeUnique<ProjectedOmniVideoAtom, ProjectedOmniVideoAtom>();

        povdAtom->GetProjectionFormatAtom().SetProjectFormat(
            (ProjectionFormatAtom::ProjectFormat) *projFmt);

        if (rwpk)
        {
            auto rwpkAtom = MakeUnique<RegionWisePackingAtom, RegionWisePackingAtom>();
            rwpkAtom->SetProjPictureWidth(rwpk->projPicWidth);
            rwpkAtom->SetProjPictureHeight(rwpk->projPicHeight);
            rwpkAtom->SetPackedPictureWidth(rwpk->packedPicWidth);
            rwpkAtom->SetPackedPictureHeight(rwpk->packedPicHeight);
            rwpkAtom->SetConstituentPictureMatchingFlag(rwpk->constituenPicMatching);

            for (auto& region : rwpk->regions)
            {
                rwpkAtom->AddRegion(move(region->GenRegion()->region));
            }

            povdAtom->SetRegionWisePackingAtom(move(rwpkAtom));
        }

        if (covi)
        {
            auto coviAtom = MakeUnique<CoverageInformationAtom, CoverageInformationAtom>();
            coviAtom->SetCoverageShapeMode((CoverageInformationAtom::CoverageShapeMode) covi->coverageShape);
            coviAtom->SetViewIdcPresenceFlag(covi->viewIdcPresenceFlag);
            coviAtom->SetDefaultViewIdc((ViewMode) covi->defaultViewIdc);

            for (auto& region : covi->sphereRegions)
            {
                auto sphereRegion     = MakeUnique<CoverageInformationAtom::CoverageSphereRegion,
                                                     CoverageInformationAtom::CoverageSphereRegion>();
                sphereRegion->viewIdc = (ViewMode) region->viewIdc;
                sphereRegion->region.centreAzimuth   = region->centAzimuth;
                sphereRegion->region.centreElevation = region->centElevation;
                sphereRegion->region.centreTilt      = region->centTilt;
                sphereRegion->region.azimuthRange    = region->azimuthRange;
                sphereRegion->region.elevationRange  = region->elevationRange;
                sphereRegion->region.interpolate     = region->interpolate;

                coviAtom->AddSphereRegion(move(sphereRegion));
            }

            povdAtom->SetCoverageInformationAtom(move(coviAtom));
        }

        for (auto& compatibleScheme : compatibleSchemes)
        {
            auto compatSchemeAtom = MakeUnique<CompatibleSchemeTypeAtom, CompatibleSchemeTypeAtom>();
            compatSchemeAtom->SetSchemeType(FourCCInt(compatibleScheme.type.c_str()));
            compatSchemeAtom->SetSchemeVersion(compatibleScheme.version);
            compatSchemeAtom->SetSchemeUri(compatibleScheme.uri.c_str());
            rinfAtom->AddCompatibleSchemeTypeAtom(move(compatSchemeAtom));
        }

        if (stvi)
        {
            auto stviAtom = MakeUnique<StereoVideoAtom, StereoVideoAtom>();
            stviAtom->SetStereoScheme(StereoVideoAtom::SchemeSpec::POVD);
            StereoVideoAtom::StereoIndicationType stereoIndicationType;
            stereoIndicationType.typePOVD.useQuincunxSampling = 0;
            stereoIndicationType.typePOVD.compositionType     = (StereoVideoAtom::POVDFrameCompType) *stvi;
            stviAtom->SetStereoIndicationType(stereoIndicationType);
            rinfAtom->AddStereoVideoAtom(move(stviAtom));
        }

        if (rotn)
        {
            auto rotnAtom = MakeUnique<RotationAtom, RotationAtom>();
            rotnAtom->SetRotation({rotn->yaw, rotn->pitch, rotn->roll});
            povdAtom->SetRotationAtom(move(rotnAtom));
        }

        rinfAtom->AddProjectedOmniVideoAtom(move(povdAtom));

        box->sampleEntryBox->SetType("resv");
        box->sampleEntryBox->AddRestrictedSchemeInfoAtom(move(rinfAtom));
    }
}

unique_ptr<HandlerBoxWrapper> AvcVideoSampleEntry::GenHandlerBox() const
{
    auto handlerBox = MakeUnique<HandlerBoxWrapper>(MakeUnique<HandlerAtom, HandlerAtom>());
    handlerBox->handlerBox->SetHandlerType("vide");
    handlerBox->handlerBox->SetName("VideoHandler");
    return handlerBox;
}

unique_ptr<SampleEntryBoxWrapper> AvcVideoSampleEntry::GenSampleEntryBox() const
{
    auto box = MakeUnique<AvcSampleEntry, SampleEntryAtom>();

    box->SetDataReferenceIndex(1);

    AvcConfigurationAtom& cfg = box->GetAvcConfigurationAtom();

    AvcDecoderConfigurationRecord decCfg = cfg.GetConfiguration();

    if (decCfg.ConfigSPS({sps.begin(), sps.end()}))
    {
        decCfg.AddNalUnit({sps.begin(), sps.end()}, AvcNalDefs::SPS);
        decCfg.AddNalUnit({pps.begin(), pps.end()}, AvcNalDefs::PPS);
        cfg.SetConfiguration(decCfg);
        box->SetWidth(width);
        box->SetHeight(height);

        auto wrappedSampleEntry =
            MakeUnique<SampleEntryBoxWrapper>(StaticCast<SampleEntryAtom>(move(box)));
        GenPovdBoxes(wrappedSampleEntry);
        return wrappedSampleEntry;
    }
    else
    {
        return NULL;
    }
}

unique_ptr<HandlerBoxWrapper> HevcVideoSampleEntry::GenHandlerBox() const
{
    auto handlerBox = MakeUnique<HandlerBoxWrapper>(MakeUnique<HandlerAtom, HandlerAtom>());
    handlerBox->handlerBox->SetHandlerType("vide");
    handlerBox->handlerBox->SetName("VideoHandler");
    return handlerBox;
}

unique_ptr<SampleEntryBoxWrapper> HevcVideoSampleEntry::GenSampleEntryBox() const
{
    auto box = MakeUnique<HevcSampleEntry, SampleEntryAtom>();

    HevcConfigurationAtom& cfg = box->GetHevcConfigurationAtom();

    box->SetDataReferenceIndex(1);

    HevcDecoderConfigurationRecord decCfg = cfg.GetConfiguration();

    decCfg.ConfigSPS({sps.begin(), sps.end()}, frameRate);
    uint8_t arrayCompleteness = 1;
    decCfg.AddNalUnit({vps.begin(), vps.end()}, HevcNalDefs::VPS, arrayCompleteness);
    decCfg.AddNalUnit({sps.begin(), sps.end()}, HevcNalDefs::SPS, arrayCompleteness);
    decCfg.AddNalUnit({pps.begin(), pps.end()}, HevcNalDefs::PPS, arrayCompleteness);
    cfg.SetConfiguration(decCfg);
    box->SetWidth(width);
    box->SetHeight(height);

    box->SetType(sampleEntryType.item);

    auto wrappedSampleEntry =
        MakeUnique<SampleEntryBoxWrapper>(StaticCast<SampleEntryAtom>(move(box)));
    GenPovdBoxes(wrappedSampleEntry);
    return wrappedSampleEntry;
}

FrameBuf HevcExtractorSampleConstructor::GenFrameData() const
{
    Stream sampleStream;

    sampleStream.Write8(trackId);
    sampleStream.Write8(sampleOffset);

    sampleStream.Write32(dataOffset);
    sampleStream.Write32(dataLength);

    return FrameBuf(sampleStream.GetStorage().begin(), sampleStream.GetStorage().end());
}

FrameBuf HevcExtractorInlineConstructor::GenFrameData() const
{
    FrameBuf frmBuf;

    frmBuf.push_back(inlineData.size());
    frmBuf.insert(frmBuf.end(), inlineData.begin(), inlineData.end());
    return frmBuf;
}

FrameBuf HevcExtractor::GenFrameData() const
{
    FrameBuf frmBuf;

    if (!sampleConstructor && !inlineConstructor)
    {
        ISO_LOG(LOG_ERROR, "Both of sample constructor and inline constructor are NULL !\n");
        throw exception();
    }
    if (!!inlineConstructor)
    {
        frmBuf.push_back(2);
        auto inlineBuf = inlineConstructor->GenFrameData();
        frmBuf.insert(frmBuf.end(), inlineBuf.begin(), inlineBuf.end());
    }
    if (!!sampleConstructor)
    {
        frmBuf.push_back(0);
        auto sampBuf = sampleConstructor->GenFrameData();
        frmBuf.insert(frmBuf.end(), sampBuf.begin(), sampBuf.end());
    }

    return frmBuf;
}

FrameBuf HevcExtractorTrackPackedData::GenFrameData() const
{
    FrameBuf frmBuf;

    frmBuf.push_back(0);
    frmBuf.push_back(0);
    frmBuf.push_back(0);
    frmBuf.push_back(0);

    const uint16_t forbiddenZero = (0 << 15) & 0b1000000000000000;
    const uint16_t nalUnitType   = (49 << 9) & 0b0111111000000000;
    const uint16_t nuhLayerId    = (0 << 3) & 0b0000000111111000;
    const uint16_t nuhTemporalId = (nuhTemporalIdPlus1 << 0) & 0b0000000000000111;

    uint16_t naluHrd = forbiddenZero | nalUnitType | nuhLayerId | nuhTemporalId;

    frmBuf.push_back(naluHrd >> 8);
    frmBuf.push_back(naluHrd & 0xff);

    vector<HevcExtractor>::const_iterator iter = samples.begin();
    for ( ; iter != samples.end(); iter++)
    {
        auto genedFrmBuf = iter->GenFrameData();
        frmBuf.insert(frmBuf.end(), genedFrmBuf.begin(), genedFrmBuf.end());
    }

    uint32_t bufSize = frmBuf.size() - 4;
    frmBuf[0]     = (bufSize >> 24) & 0xff;
    frmBuf[1]     = (bufSize >> 16) & 0xff;
    frmBuf[2]     = (bufSize >> 8) & 0xff;
    frmBuf[3]     = (bufSize >> 0) & 0xff;

    return frmBuf;
}

unique_ptr<SampleEntryBoxWrapper> MP4AudioSampleEntry::GenSampleEntryBox() const
{
    ISO_LOG(LOG_INFO, "Gen sample entry box for MP4AudioSampleEntry \n");
    auto audioAtom = MakeUnique<MP4AudioSampleEntryAtom, SampleEntryAtom>();
    ElementaryStreamDescriptorAtom& esdAtom = audioAtom->GetESDAtom();
    ElementaryStreamDescriptorAtom::ES_Params esParams{};
    audioAtom->SetSampleSize(sizeOfSample);
    audioAtom->SetChannelCount(cntOfChannels);
    audioAtom->SetSampleRate(rateOfSample);
    audioAtom->SetDataReferenceIndex(1);
    esParams.descrFlag       = 3;
    esParams.flags           = esIdOfDepends ? 0x80 : 0;
    esParams.id              = idOfES;
    esParams.depend          = esIdOfDepends;
    if (strUrl.size())
    {
        esParams.URLlen = static_cast<uint8_t>(strUrl.size());
        esParams.URL    = {strUrl.begin(), strUrl.end()};
    }

    esParams.decConfig.flag = 4;
    esParams.decConfig.strType = 0x05;
    esParams.decConfig.idc = 0x40;
    esParams.decConfig.bufferSizeDB = sizeOfBuf;
    esParams.decConfig.avgBitrate = avgBitrate;
    esParams.decConfig.maxBitrate = maxBitrate;
    esParams.decConfig.info.flag = 5;
    esParams.decConfig.info.size = static_cast<uint32_t>(decSpecificInfo.size());
    esParams.decConfig.info.info.resize(decSpecificInfo.size());
    for (size_t i = 0; i < decSpecificInfo.size(); ++i)
    {
        esParams.decConfig.info.info[i] = static_cast<uint8_t>(decSpecificInfo[i]);
    }

    esdAtom.SetESDescriptor(esParams);

    if (isNonDiegetic)
    {
        audioAtom->SetNonDiegeticAudioAtom(NonDiegeticAudioAtom());
    }
    if (ambisonicItem)
    {
        const auto& cfgOfAmb = *ambisonicItem;
        SpatialAudioAtom spaAudioAtom;
        spaAudioAtom.SetAmbisonicType(cfgOfAmb.type);
        spaAudioAtom.SetAmbisonicOrder(cfgOfAmb.order);
        spaAudioAtom.SetAmbisonicChannelOrdering(cfgOfAmb.channelOrdering);
        spaAudioAtom.SetAmbisonicNormalization(cfgOfAmb.normalization);
        spaAudioAtom.SetChannelMap(vector<uint32_t>{cfgOfAmb.channelMap.begin(), cfgOfAmb.channelMap.end()});
        audioAtom->SetSpatialAudioAtom(spaAudioAtom);
    }

    if (chnLayoutItem)
    {
        const ChannelLayout& chlLayout = *chnLayoutItem;
        ChannelLayoutAtom chlLayoutAtom;

        chlLayoutAtom.SetChannelNumber(cntOfChannels);
        if (chlLayout.streamStructure & 1)
        {
            chlLayoutAtom.SetDefinedLayout(static_cast<uint8_t>(chlLayout.layout));
            if (chlLayout.layout == 0)
            {
                for (ChannelPosition oneChlPosition : chlLayout.positions)
                {
                    ChannelLayoutAtom::ChannelLayout oneLayOut{};
                    oneLayOut.speakerPosition = static_cast<uint8_t>(oneChlPosition.speakerPosition);
                    oneLayOut.azimuthAngle    = static_cast<int16_t>(oneChlPosition.azimuth);
                    oneLayOut.elevationAngle  = static_cast<int8_t>(oneChlPosition.elevation);
                    chlLayoutAtom.AddChannelLayout(oneLayOut);
                }
            }
            else
            {
                uint64_t chlsMap{0};
                for (auto omitted : chlLayout.omitted)
                {
                    chlsMap = chlsMap | (1ull << omitted);
                }
                chlLayoutAtom.SetOmittedChannelsMap(chlsMap);
            }
        }
        if (chlLayout.streamStructure & 2)  // object structured
        {
            chlLayoutAtom.SetObjectCount(static_cast<uint8_t>(chlLayout.objectCount));
        }

        audioAtom->SetChannelLayoutAtom(chlLayoutAtom);
    }

    return MakeUnique<SampleEntryBoxWrapper>(StaticCast<SampleEntryAtom>(move(audioAtom)));
}

unique_ptr<HandlerBoxWrapper> MP4AudioSampleEntry::GenHandlerBox() const
{
    unique_ptr<HandlerBoxWrapper> handlerBox =
        MakeUnique<HandlerBoxWrapper>(MakeUnique<HandlerAtom, HandlerAtom>());
    handlerBox->handlerBox->SetHandlerType("soun");
    handlerBox->handlerBox->SetName("SoundHandler");
    return handlerBox;
}

uint32_t MP4AudioSampleEntry::GetWidthFP() const
{
    return 0;
}

uint32_t MP4AudioSampleEntry::GetHeightFP() const
{
    return 0;
}

TrackDescription::TrackDescription(TrackMeta inTrackMeta,
                                   FileInfo inFileInfo,
                                   const OmniSampleEntry& inSmpEty)
    : trackMeta(inTrackMeta)
{
    sampleEntryBoxes.push_back(inSmpEty.GenSampleEntryBox());
    UniquePtr<MediaHeaderAtom> box = MakeUnique<MediaHeaderAtom, MediaHeaderAtom>();
    //GenMediaHeaderAtom(box, inFileInfo, inTrackMeta.timescale);
    FractU64 timeScale = inTrackMeta.timescale;
    uint64_t creationTime     = inFileInfo.creationTime;
    uint64_t modificationTime = inFileInfo.modificationTime;
    FractU64 duration           = inFileInfo.duration;
    box->SetTimeScale(uint32_t(timeScale.per1().asDouble()));
    box->SetCreationTime(creationTime);
    box->SetModificationTime(modificationTime);
    box->SetDuration(uint32_t((duration / timeScale).asDouble()));
    mediaHeaderBox = move(MakeUnique<MediaHeaderBoxWrapper>(move(box)));//GenMediaHeaderAtom(inFileInfo, inTrackMeta.timescale);

    handlerBox     = move(inSmpEty.GenHandlerBox());

    UniquePtr<TrackHeaderAtom> thead = MakeUnique<TrackHeaderAtom, TrackHeaderAtom>();
    trackHeaderBox = move(MakeUnique<TrackHeaderBoxWrapper>(move(thead)));
    trackHeaderBox->trackHeaderBox->SetTrackID(inTrackMeta.trackId.GetIndex());
    trackHeaderBox->trackHeaderBox->SetWidth(inSmpEty.GetWidthFP());
    trackHeaderBox->trackHeaderBox->SetHeight(inSmpEty.GetHeightFP());
    FillTrackHeaderAtom(*trackHeaderBox->trackHeaderBox, inFileInfo);
}

void FillMovieHeaderAtom(MovieHeaderAtom& mheaAtom,
                       const MovieDescription& inMovieDes,
                       FractU64 timeScale)
{
    uint64_t creationTime     = inMovieDes.creationTime;
    uint64_t modificationTime = inMovieDes.modificationTime;
    FractU64 duration           = inMovieDes.duration;
    mheaAtom.SetTimeScale(uint32_t(timeScale.per1().asDouble()));
    mheaAtom.SetCreationTime(creationTime);
    mheaAtom.SetModificationTime(modificationTime);
    mheaAtom.SetDuration(uint32_t((duration / timeScale).asDouble()));
}

void UpdateMediaInfoAtom(MediaInformationAtom& minfAtom, const TrackDescription& inTrackDes)
{
    switch (inTrackDes.trackMeta.type)
    {
    case TypeOfMedia::Audio:
    {
        minfAtom.SetMediaType(MediaInformationAtom::MediaType::Sound);
        break;
    }
    case TypeOfMedia::Video:
    {
        minfAtom.SetMediaType(MediaInformationAtom::MediaType::Video);
        break;
    }
    default:
        minfAtom.SetMediaType(MediaInformationAtom::MediaType::Null);
    }
}

InitialSegment GenInitSegment(const TrackDescriptionsMap& inTrackDes,
                            const MovieDescription& inMovieDes,
                            const bool isFraged)
{
    InitialSegment initSeg;

    vector<FractU64> tscalesVec;
    vector<FractU64*> tscalesRefVec;
    map<TrackId, TrackDescription>::const_iterator iter1 = inTrackDes.begin();
    for ( ; iter1 != inTrackDes.end(); iter1++)
    {
        tscalesVec.push_back(iter1->second.trackMeta.timescale);
    }
    vector<FractU64>::iterator iter2 = tscalesVec.begin();
    for ( ; iter2 != tscalesVec.end(); iter2++)
    {
        tscalesRefVec.push_back(&(*iter2));
    }

    shareDenominators(tscalesRefVec.begin(), tscalesRefVec.end());
    sort(tscalesVec.begin(), tscalesVec.end());

    auto moovAtom = MakeUnique<MovieAtom, MovieAtom>();

    UniquePtr<MovieExtendsAtom> movieExtendsBoxOut(new MovieExtendsAtom());
    auto ftypAtom = MakeUnique<FileTypeAtom, FileTypeAtom>();
    if (inMovieDes.fileType)
    {
        ftypAtom->SetMajorBrand(inMovieDes.fileType->majorBrand.c_str());
        ftypAtom->SetMinorVersion(inMovieDes.fileType->minorVersion);
        for (auto& compatBrand : inMovieDes.fileType->compatibleBrands)
        {
            ftypAtom->AddCompatibleBrand(compatBrand.c_str());
        }
    }
    else
    {
        ftypAtom->SetMajorBrand("isom");
        ftypAtom->SetMinorVersion(512);
        ftypAtom->AddCompatibleBrand("isom");
        ftypAtom->AddCompatibleBrand("iso2");
        ftypAtom->AddCompatibleBrand("mp41");
        ftypAtom->AddCompatibleBrand("mp42");
    }

    FillMovieHeaderAtom(moovAtom->GetMovieHeaderAtom(), inMovieDes, {1, 1000});

    TrackId maxTrackId;
    if (inTrackDes.size())
    {
        maxTrackId = inTrackDes.begin()->first;
    }

    map<TrackId, TrackDescription>::const_iterator iter3 = inTrackDes.begin();
    for ( ; iter3 != inTrackDes.end(); iter3++)
    {
        const TrackId trackId                       = iter3->first;
        const TrackDescription& trackDes            = iter3->second;
        UniquePtr<TrackAtom> trackAtom                = MakeUnique<TrackAtom, TrackAtom>();
        MediaAtom& mediaAtom                       = trackAtom->GetMediaAtom();
        MediaInformationAtom& mediaInfoAtom = mediaAtom.GetMediaInformationAtom();
        SampleTableAtom& stblAtom           = mediaInfoAtom.GetSampleTableAtom();
        maxTrackId                                  = max(maxTrackId, trackId);

        UpdateMediaInfoAtom(mediaInfoAtom, trackDes);

        CopyAtom(*trackDes.trackHeaderBox->trackHeaderBox, trackAtom->GetTrackHeaderAtom());

        CopyAtom(*trackDes.mediaHeaderBox->mediaHeaderBox, mediaAtom.GetMediaHeaderAtom());

        CopyAtom(*trackDes.handlerBox->handlerBox, mediaAtom.GetHandlerAtom());

        if (trackDes.trackMeta.trackType)
        {
            trackAtom->SetHasTrackTypeAtom(true);
            trackAtom->GetTrackTypeAtom().SetMajorBrand(trackDes.trackMeta.trackType->majorBrand.c_str());
            trackAtom->GetTrackTypeAtom().SetMinorVersion(trackDes.trackMeta.trackType->minorVersion);
            for (const auto& brand : trackDes.trackMeta.trackType->compatibleBrands)
            {
                trackAtom->GetTrackTypeAtom().AddCompatibleBrand(brand.c_str());
            }
        }

        if (trackDes.trackReferences.size())
        {
            trackAtom->SetHasTrackReferences(true);
            SetReferredTracks(trackAtom->GetTrackReferenceAtom(), trackDes.trackReferences);
        }

        if (trackDes.trackMeta.type == TypeOfMedia::Audio)
        {
            trackAtom->GetTrackHeaderAtom().SetVolume(0x0100);
        }

        if (trackDes.alternateGroup)
        {
            trackAtom->GetTrackHeaderAtom().SetAlternateGroup(*trackDes.alternateGroup);
        }

        if (trackDes.vrType)
        {
            SphericalVideoV1Atom::GeneralMetaData data{};

            data.isSpherical         = true;
            data.isStitched          = true;
            data.stitchedSW          = "";
            data.projectionFormat    = SphericalVideoV1Atom::ProjectFormat::ERP;

            switch (*trackDes.vrType)
            {
            case OmniMediaType::OMNI_Mono:
            {
                data.stereoType = SphericalVideoV1Atom::StereoTypeV1::MONO_TYPE;
            }
            break;
            case OmniMediaType::OMNI_StereoLR:
            {
                data.stereoType = SphericalVideoV1Atom::StereoTypeV1::STEREO_LEFT_RIGHT;
            }
            break;
            case OmniMediaType::OMNI_StereoTB:
            {
                data.stereoType = SphericalVideoV1Atom::StereoTypeV1::STEREO_TOP_BOTTOM;
            }
            break;
            }
            trackAtom->GetSphericalVideoV1Atom().SetGeneralMetaData(data);
            trackAtom->SetHasSphericalVideoV1Atom(true);
        }

        shared_ptr<DataEntryUrlAtom> dataEntryBox(new DataEntryUrlAtom(DataEntryUrlAtom::Contained));
        mediaInfoAtom.GetDataInformationAtom().AddDataEntryAtom(dataEntryBox);

        SampleDescriptionAtom& sampleDescriptionBox = stblAtom.GetSampleDescriptionAtom();
        for (auto& sampleEntryBox : trackDes.sampleEntryBoxes)
        {
            UniquePtr<SampleEntryAtom> sampleEntryCopy(sampleEntryBox->sampleEntryBox->Clone());
            sampleDescriptionBox.AddSampleEntry(move(sampleEntryCopy));
        }

        UniquePtr<TrackExtendsAtom> trackExtendsBoxOut(new TrackExtendsAtom());
        SampleDefaults sampleDefaults = {
            trackDes.trackMeta.trackId.GetIndex(),
            1,
            0,
            0,
            {0}
        };
        trackExtendsBoxOut->SetFragmentSampleDefaults(sampleDefaults);
        movieExtendsBoxOut->AddTrackExtendsAtom(move(trackExtendsBoxOut));
        moovAtom->AddTrackAtom(move(trackAtom));
    }
    moovAtom->GetMovieHeaderAtom().SetNextTrackID(maxTrackId.GetIndex() + 1);

    if (isFraged)
    {
        moovAtom->AddMovieExtendsAtom(move(movieExtendsBoxOut));
    }

    initSeg.moov.reset(new MovieBoxWrapper{move(moovAtom)});
    initSeg.ftyp.reset(new FileTypeBoxWrapper{move(ftypAtom)});

    return initSeg;
}

TimeInterval GetFrameTimeInterval(const list<FrameWrapper>& frameList)
{
    FrameTime time0;
    FrameTime time1;

    bool first = true;
    for (auto& frame : frameList)
    {
        for (const auto& cts : frame.GetFrameInfo().cts)
        {
            auto frameT0 = cts;
            auto frameT1 = cts + frame.GetFrameInfo().duration.cast<FrameTime>();

            if (first || frameT0 < time0)
            {
                time0 = frameT0;
            }
            if (first || frameT1 > time1)
            {
                time1 = frameT1;
            }
            first = false;
        }
    }

    assert(!first);
    return {time0, time1};
}

FrameTime GetDtsCtsInterval(const list<FrameWrapper>& frameList)
{
    FrameTime delta;
    for (auto& frame : frameList)
    {
        const FrameInfo& info = frame.GetFrameInfo();
        if (info.dts && info.cts.size())
        {
            delta = max(delta, *info.dts - info.cts.front());
        }
    }
    return delta;
}

TimeInterval GetCtsInterval(const list<FrameWrapper>& frameList)
{
    FrameTime time0;
    FrameTime time1;

    bool first = true;
    for (auto& frame : frameList)
    {
        const FrameInfo& info = frame.GetFrameInfo();
        if (info.cts.size())
        {
            for (const auto& cts : info.cts)
            {
                auto frameT0 = cts;
                auto frameT1 = cts + info.duration.cast<FrameTime>();

                if (first || frameT0 < time0)
                {
                    time0 = frameT0;
                }
                if (first || frameT1 > time1)
                {
                    time1 = frameT1;
                }
                first = false;
            }
        }
        else
        {
            if (auto dts = info.dts)
            {
                auto frameT0 = *dts;
                auto frameT1 = *dts + info.duration.cast<FrameTime>();
                if (first || frameT0 < time0)
                {
                    time0 = frameT0;
                }
                if (first || frameT1 > time1)
                {
                    time1 = frameT1;
                }
                first = false;
            }
        }
    }

    assert(!first);
    return {time0, time1};
}

TimeInterval ExtendInterval(TimeInterval timeFirst, TimeInterval timeSecond)
{
    return {min(timeFirst.first, timeSecond.first), max(timeFirst.second, timeSecond.second)};
}

void SegmentWriter::Impl::TrackState::FeedEOS()
{
    if (!isEnd)
    {
        isEnd = true;
        if (frames.size())
        {
            SubSegment subsegmentful{move(frames)};
            SubSegments.push_back(subsegmentful);
        }
        FullSubSegments.push_back(move(SubSegments));
        subSegDur = {0, 1};
        segDur    = {0, 1};
    }
}

void SegmentWriter::Impl::TrackState::FeedOneFrame(FrameWrapper oneFrame)
{
    assert(!isEnd);

    if (hasSubSeg && oneFrame.GetFrameInfo().isIDR)
    {
        hasSubSeg = false;
        subSegDur -=
            CoalesceData(imple->m_config.subsegmentDuration, imple->m_config.segmentDuration)->cast<FrameTime>();
        if (insertSubSegNum >= imple->m_config.skipSubsegments)
        {
            SubSegment subsegmentful{move(frames)};
            SubSegments.push_back(subsegmentful);
            insertSubSegNum = 0;
        }
        else
        {
            ++insertSubSegNum;
        }

        while (segDur >= imple->m_config.segmentDuration.cast<FrameTime>())
        {
            FullSubSegments.push_back(move(SubSegments));
            segDur -= imple->m_config.segmentDuration.cast<FrameTime>();
        }
    }

    frames.push_back(oneFrame);
    segDur += oneFrame.GetFrameInfo().duration.cast<FrameTime>();
    subSegDur += oneFrame.GetFrameInfo().duration.cast<FrameTime>();

    hasSubSeg =
        hasSubSeg ||
        subSegDur >=
            CoalesceData(imple->m_config.subsegmentDuration, imple->m_config.segmentDuration)->cast<FrameTime>();
}

list<Frames> SegmentWriter::Impl::TrackState::TakeSegment()
{
    list<Frames> segmentFrames;
    if (FullSubSegments.size() > 0u)
    {
        SubSegment subsegmentful{};
        auto completeSegment = move(FullSubSegments.front());
        FullSubSegments.pop_front();

        for (SubSegment& subsegment : completeSegment)
        {
            segmentFrames.push_back(move(subsegment.frames));
        }
    }
    return segmentFrames;
}

bool SegmentWriter::Impl::TrackState::IsFinished() const
{
    bool isCompleted = (isEnd && (frames.size() == 0u)
                        && (SubSegments.size() == 0)
                        && (FullSubSegments.size() == 0));
    return isCompleted;
}

bool SegmentWriter::Impl::TrackState::IsIncomplete() const
{
    bool isFinished = (frames.size() > 0u);
    return isFinished;
}

bool SegmentWriter::Impl::TrackState::CanTakeSegment() const
{
    bool isSegReady = (FullSubSegments.size() > 0u);
    return isSegReady;
}

SegmentWriter::SegmentWriter(SegmentWriterCfg inCfg)
    : m_impl(new Impl())
    , m_sidxWriter(new SidxWriter)
{
    m_impl->m_config = inCfg;
}

SegmentWriter::~SegmentWriter()
{
}

void SegmentWriter::AddTrack(TrackId trackIndex, TrackMeta inTrackMeta)
{
    assert(trackIndex == inTrackMeta.trackId);
    (void) trackIndex;
    AddTrack(inTrackMeta);
}

void SegmentWriter::AddTrack(TrackMeta inTrackMeta)
{
    m_impl->m_trackSte[inTrackMeta.trackId]           = Impl::TrackState(m_impl.get());
    m_impl->m_trackSte[inTrackMeta.trackId].trackMeta = inTrackMeta;
}

SegmentWriter::Action SegmentWriter::FeedEOS(TrackId trackIndex)
{
    m_impl->m_trackSte[trackIndex].FeedEOS();
    return m_impl->AllTracksReadyForSegment() ? Action::ExtractSegment : Action::KeepFeeding;
}

bool SegmentWriter::Impl::AnyTrackIncomplete() const
{
    bool incomplete = false;
    for (auto& trackIdAndTrackFrames : m_imple->m_trackSte)
    {
        auto& stateOfTrack = trackIdAndTrackFrames.second;
        incomplete       = incomplete || stateOfTrack.IsIncomplete();
    }
    return incomplete;
}

bool SegmentWriter::Impl::AllTracksReadyForSegment() const
{
    bool ready = true;
    for (auto& trackIdAndTrackState : m_imple->m_trackSte)
    {
        auto& stateOfTrack = trackIdAndTrackState.second;
        ready            = ready && (stateOfTrack.isEnd || stateOfTrack.CanTakeSegment());
    }
    return ready;
}

bool SegmentWriter::Impl::AllTracksFinished() const
{
    bool ready = true;
    for (auto& trackIdAndTrackState : m_imple->m_trackSte)
    {
        auto& stateOfTrack = trackIdAndTrackState.second;
        ready            = ready && stateOfTrack.IsFinished();
    }
    return ready;
}

SegmentWriter::Action SegmentWriter::FeedOneFrame(TrackId trackIndex, FrameWrapper oneFrame)
{
    assert(!m_impl->m_trackSte[trackIndex].isEnd);

    if (!m_impl->m_trackSte.count(trackIndex))
    {
        ISO_LOG(LOG_ERROR, "Track is not correct !\n");
        throw exception();
    }

    m_impl->m_trackSte.at(trackIndex).FeedOneFrame(oneFrame);

    return m_impl->AllTracksReadyForSegment() ? Action::ExtractSegment : Action::KeepFeeding;
}

list<SegmentList> SegmentWriter::ExtractSubSegments()
{
    using SegGroup = list<SegmentList>;
    using FrmGroup = list<Frames>;
    SegGroup segGroup;
    while (m_impl->AllTracksReadyForSegment() && !m_impl->AllTracksFinished())
    {
        map<TrackId, FrmGroup> trackSegMap;
        map<TrackId, Impl::TrackState>::iterator iter1 = (m_impl->m_trackSte).begin();
        for ( ; iter1 != (m_impl->m_trackSte).end(); iter1++)
        {
            TrackId trackId              = iter1->first;
            Impl::TrackState& stateOfTrack = iter1->second;
            trackSegMap[trackId]       = stateOfTrack.TakeSegment();
        }

        SegmentList subSegGroup;
        map<TrackId, FrmGroup>::iterator iter2 = trackSegMap.begin();
        for ( ; iter2 != trackSegMap.end(); iter2++)
        {
            TrackId trackId                             = iter2->first;
            FrmGroup& frames                            = iter2->second;
            list<Segment>::iterator iter3 = subSegGroup.begin();
            SegmentWriter::Impl::TrackState& stateOfTrack = m_impl->m_trackSte.at(trackId);

            if (m_impl->m_isFirstSeg && frames.size())
            {
                Frames firstFrmGroup = *frames.begin();
                list<FrameWrapper>::iterator iter4 = firstFrmGroup.begin();
                for ( ; iter4 != firstFrmGroup.end(); iter4++)
                {
                    FrameInfo info = iter4->GetFrameInfo();
                    for (auto cts : info.cts)
                    {
                        if (info.dts)
                        {
                            stateOfTrack.trackOffset = max(stateOfTrack.trackOffset, *info.dts - cts);
                        }

                        stateOfTrack.trackOffset = max(stateOfTrack.trackOffset, -cts);
                    }
                }
            }

            FrmGroup::iterator iter5 = frames.begin();
            for ( ; iter5 != frames.end(); iter5++)
            {
                if (iter3 == subSegGroup.end())
                {
                    subSegGroup.push_back({});
                    iter3 = subSegGroup.end();
                    --iter3;
                }
                if (iter3 == subSegGroup.end())
                {
                    ISO_LOG(LOG_ERROR, "Failed to get sub segment group !\n");
                    throw exception();
                }
                TrackOfSegment& trackOfSegment = (*iter3).tracks[trackId];
                trackOfSegment.frames                     = move(*iter5);

                if (stateOfTrack.trackOffset.m_num)
                {
                    for (auto& frame : trackOfSegment.frames)
                    {
                        FrameInfo frameInfo = frame.GetFrameInfo();
                        for (auto& x : frameInfo.cts)
                        {
                            x += stateOfTrack.trackOffset;
                        }
                        if (frameInfo.dts)
                        {
                            *frameInfo.dts += stateOfTrack.trackOffset;
                        }
                        frame.SetFrameInfo(frameInfo);
                    }
                }

                ++iter3;
                trackOfSegment.trackInfo.trackMeta = m_impl->m_trackSte.at(trackId).trackMeta;
                auto dtsCtsOffset                  = GetDtsCtsInterval(trackOfSegment.frames);
                if (auto dts = trackOfSegment.frames.front().GetFrameInfo().dts)
                {
                    trackOfSegment.trackInfo.tBegin = *dts;
                }
                else
                {
                    trackOfSegment.trackInfo.tBegin = GetCtsInterval(trackOfSegment.frames).first - dtsCtsOffset;
                }
                trackOfSegment.trackInfo.dtsCtsOffset = dtsCtsOffset;
            }
        }

        list<Segment>::iterator iter6 = subSegGroup.begin();
        for ( ; iter6 != subSegGroup.end(); iter6++)
        {
            TimeInterval segTimeInterval;
            InvertTrue firstSegmentSpan;
            for (auto trackIdSegment : iter6->tracks)
            {
                TrackOfSegment& trackOfSegment = trackIdSegment.second;

                auto timeSpan = GetFrameTimeInterval(trackOfSegment.frames);
                if (firstSegmentSpan())
                {
                    segTimeInterval = timeSpan;
                }
                else
                {
                    segTimeInterval = ExtendInterval(timeSpan, segTimeInterval);
                }
            }

            iter6->sequenceId = m_impl->m_seqId;
            iter6->tBegin     = segTimeInterval.first;
            iter6->duration   = (segTimeInterval.second - segTimeInterval.first).cast<FractU64>();
            ++m_impl->m_seqId;
        }

        segGroup.push_back(subSegGroup);
        m_impl->m_isFirstSeg = false;
    }
    return segGroup;
}

SegmentList SegmentWriter::ExtractSegments()
{
    list<SegmentList> subsegments = ExtractSubSegments();
    SegmentList segments;
    for (auto& segment : subsegments)
    {
        segments.insert(segments.end(), segment.begin(), segment.end());
    }
    return segments;
}

void SegmentWriter::SetWriteSegmentHeader(bool toWriteHdr)
{
    m_needWriteSegmentHeader = toWriteHdr;
}

void SegmentWriter::WriteInitSegment(ostream& outStr, const InitialSegment& initSeg)
{
    WriteInitSegment(outStr, initSeg);
}

void SegmentWriter::WriteSubSegments(ostream& outStr, const list<Segment> subSegList)
{
    if (m_needWriteSegmentHeader)
    {
        WriteSegmentHeader(outStr);
    }
    for (auto& subsegment : subSegList)
    {
        m_sidxWriter->AddSubSeg(subsegment);
    }
    auto sidxInfo = m_sidxWriter->WriteSidx(outStr, {});
    for (auto& subsegment : subSegList)
    {
        auto before = outStr.tellp();
        WriteSampleData(outStr, subsegment);
        auto after = outStr.tellp();
        m_sidxWriter->AddSubSegSize(after - before);
    }
    if (sidxInfo)
    {
        m_sidxWriter->WriteSidx(outStr, sidxInfo->position);
    }

}

void SegmentWriter::WriteSegment(ostream& outStr, const Segment oneSeg)
{
    WriteSubSegments(outStr, {oneSeg});
}

VCD_MP4_END
