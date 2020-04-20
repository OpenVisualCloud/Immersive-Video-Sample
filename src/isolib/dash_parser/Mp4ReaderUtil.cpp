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
//! \file:   Mp4ReaderUtil.cpp
//! \brief:  Mp4 reader basic utility functions implementation
//!

#include "Mp4ReaderUtil.h"
#include "../atoms/AvcConfigAtom.h"
#include "../atoms/CleanApertureAtom.h"
#include "../atoms/FormAllocator.h"
#include "../atoms/HevcConfigAtom.h"

using namespace std;

VCD_MP4_BEGIN

ChnlPropertyInternal GenChnl(const ChannelLayoutAtom& inChnlBox)
{
    ChnlPropertyInternal chnl{};
    chnl.strStruct    = inChnlBox.GetStreamStructure();
    chnl.layOut      = inChnlBox.GetDefinedLayout();
    chnl.omitChnlMap = inChnlBox.GetOmittedChannelsMap();
    chnl.objCnt        = inChnlBox.GetObjectCount();
    chnl.chnlCnt       = inChnlBox.GetChannelNumber();
    chnl.chnlLayOuts.clear();

    if (chnl.chnlCnt > 0 && chnl.strStruct == 1 && chnl.layOut == 0)
    {
        std::vector<ChannelLayoutAtom::ChannelLayout> layout = inChnlBox.GetChannelLayouts();
        for (uint16_t i = 0; i < layout.size(); i++)
        {
            ChannelLayout channelPosition;
            channelPosition.speakerPosition = layout[i].speakerPosition;
            channelPosition.azimuth         = layout[i].azimuthAngle;
            channelPosition.elevation       = layout[i].elevationAngle;
            chnl.chnlLayOuts.push_back(channelPosition);
        }
    }

    return chnl;
}

SA3DPropertyInternal GenSA3D(const SpatialAudioAtom& inSpaAudioBox)
{
    SA3DPropertyInternal sa3d{};
    sa3d.version                  = inSpaAudioBox.GetVersion();
    sa3d.ambisonicType            = inSpaAudioBox.GetAmbisonicType();
    sa3d.ambisonicOrder           = inSpaAudioBox.GetAmbisonicOrder();
    sa3d.ambisonicChnlSeq = inSpaAudioBox.GetAmbisonicChannelOrdering();
    sa3d.ambisonicNorml   = inSpaAudioBox.GetAmbisonicNormalization();
    sa3d.chnlMap.clear();

    std::vector<uint32_t> channelMap = inSpaAudioBox.GetChannelMap();
    for (uint16_t i = 0; i < channelMap.size(); i++)
    {
        sa3d.chnlMap.push_back(channelMap.at(i));
    }

    return sa3d;
}

OmniStereoScopic3D Genst3d(const Stereoscopic3D* inStereo3DBox)
{
    OmniStereoScopic3D st3d(OmniStereoScopic3D::OMNI_MONOSCOPIC);
    if (inStereo3DBox != nullptr)
    {
        Stereoscopic3D::StereoTypeV2 v2stereomode = inStereo3DBox->GetStereoMode();
        if (v2stereomode == Stereoscopic3D::StereoTypeV2::STEREO_TOPBOTTOM)
        {
            st3d = OmniStereoScopic3D::OMNI_STEREOSCOPIC_TOP_BOTTOM;
        }
        else if (v2stereomode == Stereoscopic3D::StereoTypeV2::STEREO_LEFTRIGHT)
        {
            st3d = OmniStereoScopic3D::OMNI_STEREOSCOPIC_LEFT_RIGHT;
        }
        else if (v2stereomode == Stereoscopic3D::StereoTypeV2::STEREO_STEREOCUSTOM)
        {
            st3d = OmniStereoScopic3D::OMNI_STEREOSCOPIC_STEREO_CUSTOM;
        }
        else
        {
            st3d = OmniStereoScopic3D::OMNI_MONOSCOPIC;
        }
    }
    return st3d;
}

OmniStereoScopic3D Genst3d(const SphericalVideoV1Atom& inStereo3DBox)
{
    OmniStereoScopic3D st3d(OmniStereoScopic3D::OMNI_MONOSCOPIC);
    SphericalVideoV1Atom::StereoTypeV1 v1stereomode = inStereo3DBox.GetGeneralMetaData().stereoType;
    if (v1stereomode == SphericalVideoV1Atom::StereoTypeV1::STEREO_TOP_BOTTOM)
    {
        st3d = OmniStereoScopic3D::OMNI_STEREOSCOPIC_TOP_BOTTOM;
    }
    else if (v1stereomode == SphericalVideoV1Atom::StereoTypeV1::STEREO_LEFT_RIGHT)
    {
        st3d = OmniStereoScopic3D::OMNI_STEREOSCOPIC_LEFT_RIGHT;
    }
    else  // set UNDEFINED also as MONOSCOPIC (default in spec).
    {
        st3d = OmniStereoScopic3D::OMNI_MONOSCOPIC;
    }
    return st3d;
}

SphericalVideoV1Property GenSphericalVideoV1Property(const SphericalVideoV1Atom& inSpheVideoBox)
{
    SphericalVideoV1Property outSpheV1Prop{};
    outSpheV1Prop.isSphe           = true;  // always true for V1.0
    outSpheV1Prop.isStitched       = true;  // always true for V1.0
    outSpheV1Prop.projType         = OmniProjType::OMNI_ERP;
    outSpheV1Prop.srcCnt           = inSpheVideoBox.GetGeneralMetaData().sourceCount;
    outSpheV1Prop.initView.fixedPntYaw   = inSpheVideoBox.GetGeneralMetaData().initViewHead
                                                 << 16;
    outSpheV1Prop.initView.fixedPntPitch = inSpheVideoBox.GetGeneralMetaData().initViewPitch
                                                   << 16;
    outSpheV1Prop.initView.fixedPntRoll  = inSpheVideoBox.GetGeneralMetaData().initViewRoll
                                                  << 16;
    outSpheV1Prop.timestamp        = inSpheVideoBox.GetGeneralMetaData().timestamp;
    outSpheV1Prop.panoWidth        = inSpheVideoBox.GetGeneralMetaData().fullPanoWidth;
    outSpheV1Prop.panoHeight       = inSpheVideoBox.GetGeneralMetaData().fullPanoHeight;
    outSpheV1Prop.croppedWidth     =
        inSpheVideoBox.GetGeneralMetaData().croppedAreaImageWidth;
    outSpheV1Prop.croppedHeight    =
        inSpheVideoBox.GetGeneralMetaData().croppedAreaImageHeight;
    outSpheV1Prop.croppedLeftW     = inSpheVideoBox.GetGeneralMetaData().croppedAreaLeft;
    outSpheV1Prop.croppedTopH      = inSpheVideoBox.GetGeneralMetaData().croppedAreaTop;
    return outSpheV1Prop;
}

SphericalVideoV2Property Gensv3d(const SphericalVideoV2Atom* inSpheVideoBox)
{
    SphericalVideoV2Property outSpheV2Prop{};
    if (inSpheVideoBox != nullptr)
    {
        outSpheV2Prop.rotateDegree.fixedPntYaw   = inSpheVideoBox->GetProjectionAtom().GetProjectionHeaderAtom().GetPose().yaw;
        outSpheV2Prop.rotateDegree.fixedPntPitch = inSpheVideoBox->GetProjectionAtom().GetProjectionHeaderAtom().GetPose().pitch;
        outSpheV2Prop.rotateDegree.fixedPntRoll  = inSpheVideoBox->GetProjectionAtom().GetProjectionHeaderAtom().GetPose().roll;

        Projection::ProjectFormat type = inSpheVideoBox->GetProjectionAtom().GetProjectFormat();

        switch (type)
        {
        case Projection::ProjectFormat::CUBEMAP:
        {
            outSpheV2Prop.projType = OmniProjType::OMNI_CUBEMAP;
            outSpheV2Prop.projection.cubemap.layout =
                inSpheVideoBox->GetProjectionAtom().GetCubemapProjectionAtom().GetLayout();
            outSpheV2Prop.projection.cubemap.padding =
                inSpheVideoBox->GetProjectionAtom().GetCubemapProjectionAtom().GetPadding();
            break;
        }
        case Projection::ProjectFormat::ERP:
        {

            outSpheV2Prop.projType                         = OmniProjType::OMNI_ERP;
            outSpheV2Prop.projection.equirectangular.topBoundFP = inSpheVideoBox->GetProjectionAtom()
                                                              .GetEquirectangularProjectionAtom()
                                                              .GetProjectionBounds()
                                                              .top_32FP;
            outSpheV2Prop.projection.equirectangular.btmBoundFP = inSpheVideoBox->GetProjectionAtom()
                                                                 .GetEquirectangularProjectionAtom()
                                                                 .GetProjectionBounds()
                                                                 .bottom_32FP;
            outSpheV2Prop.projection.equirectangular.leftBoundFP = inSpheVideoBox->GetProjectionAtom()
                                                               .GetEquirectangularProjectionAtom()
                                                               .GetProjectionBounds()
                                                               .left_32FP;
            outSpheV2Prop.projection.equirectangular.rightBoundFP = inSpheVideoBox->GetProjectionAtom()
                                                                .GetEquirectangularProjectionAtom()
                                                                .GetProjectionBounds()
                                                                .right_32FP;
            break;
        }
        case Projection::ProjectFormat::MESH:
        {
            outSpheV2Prop.projType = OmniProjType::OMNI_MESH;
            break;
        }
        default:
        {
            outSpheV2Prop.projType = OmniProjType::OMNI_UNKOWN;
            break;
        }
        }
    }
    return outSpheV2Prop;
}

RWPKPropertyInternal Genrwpk(const RegionWisePackingAtom& inRWPKBox)
{
    RWPKPropertyInternal outRWPKProp{};
    outRWPKProp.constituentPicMatching     = inRWPKBox.GetConstituentPictureMatchingFlag();
    outRWPKProp.packedPicHeight            = inRWPKBox.GetPackedPictureHeight();
    outRWPKProp.packedPicWidth             = inRWPKBox.GetPackedPictureWidth();
    outRWPKProp.projPicHeight              = inRWPKBox.GetProjPictureHeight();
    outRWPKProp.projPicWidth               = inRWPKBox.GetProjPictureWidth();

    for (auto& regionFromBox : inRWPKBox.GetRegions())
    {
        RWPKRegion region{};
        region.guardBandFlag = regionFromBox->guardBandFlag;
        region.packingType   = (OmniRWPKType) regionFromBox->packingType;

        UniquePtr<RegionWisePackingAtom::RectangularRegionWisePacking> rwpkReg = std::move(regionFromBox->rectangularPacking);
        if (region.packingType == OmniRWPKType::OMNI_RECTANGULAR)
        {
            RectRWPKRegion rectRegion{};

            rectRegion.projRegWidth         = rwpkReg->projRegWidth;
            rectRegion.projRegHeight        = rwpkReg->projRegHeight;
            rectRegion.projRegTop           = rwpkReg->projRegTop;
            rectRegion.projRegLeft          = rwpkReg->projRegLeft;
            rectRegion.transformType        = rwpkReg->transformType;
            rectRegion.packedRegWidth       = rwpkReg->packedRegWidth;
            rectRegion.packedRegHeight      = rwpkReg->packedRegHeight;
            rectRegion.packedRegTop         = rwpkReg->packedRegTop;
            rectRegion.packedRegLeft        = rwpkReg->packedRegLeft;
            rectRegion.leftGbWidth          = rwpkReg->leftGbWidth;
            rectRegion.rightGbWidth         = rwpkReg->rightGbWidth;
            rectRegion.topGbHeight          = rwpkReg->topGbHeight;
            rectRegion.bottomGbHeight       = rwpkReg->bottomGbHeight;
            rectRegion.gbNotUsedForPredFlag = rwpkReg->gbNotUsedForPredFlag;
            rectRegion.gbType0              = rwpkReg->gbType0;
            rectRegion.gbType1              = rwpkReg->gbType1;
            rectRegion.gbType2              = rwpkReg->gbType2;
            rectRegion.gbType3              = rwpkReg->gbType3;

            region.region.rectReg = rectRegion;
        }

        regionFromBox->rectangularPacking = std::move(rwpkReg);
        outRWPKProp.regions.push_back(region);
    }

    return outRWPKProp;
}

COVIInformationInternal Gencovi(const CoverageInformationAtom& inCOVIBox)
{
    COVIInformationInternal outCOVIProp{};
    outCOVIProp.coviShapeType   = (COVIShapeType) inCOVIBox.GetCoverageShapeMode();
    outCOVIProp.defaultViewIdc      = (OmniViewIdc) inCOVIBox.GetDefaultViewIdc();
    outCOVIProp.viewIdcPresenceFlag = inCOVIBox.GetViewIdcPresenceFlag();

    for (auto& regionFromBox : inCOVIBox.GetSphereRegions())
    {
        COVIRegion region{};
        SphereRegion spheReg = regionFromBox->region;

        region.viewIdc         = (OmniViewIdc) regionFromBox->viewIdc;

        region.centAzimuth   = spheReg.centreAzimuth;
        region.centElevation = spheReg.centreElevation;
        region.centTilt      = spheReg.centreTilt;
        region.azimuthRange    = spheReg.azimuthRange;
        region.elevationRange  = spheReg.elevationRange;
        region.interpolate     = spheReg.interpolate;

        outCOVIProp.sphereRegions.push_back(region);
    }

    return outCOVIProp;
}

bool IsImageType(FourCCInt inType)
{
    // static const Set<FourCCInt> IMAGE_TYPES = {"avc1", "hvc1", "hev1", "grid", "iovl", "iden"};
    static const std::set<FourCCInt> IMAGE_TYPES = {"avc1", "hvc1", "hev1", "grid", "iovl", "iden"};

    return (IMAGE_TYPES.find(inType) != IMAGE_TYPES.end());
}

ParameterSetMap GenDecoderParameterSetMap(const AvcDecoderConfigurationRecord& avcRrd)
{
    std::vector<uint8_t> avcSPS;
    std::vector<uint8_t> avcPPS;
    avcRrd.GetOneParameterSet(avcSPS, AvcNalDefs::SPS);
    avcRrd.GetOneParameterSet(avcPPS, AvcNalDefs::PPS);

    ParameterSetMap paramsMap;
    paramsMap.insert(pair<MediaCodecInfoType, DataVector>(MediaCodecInfoType::AVC_SPS, move(avcSPS)));
    paramsMap.insert(pair<MediaCodecInfoType, DataVector>(MediaCodecInfoType::AVC_PPS, move(avcPPS)));

    return paramsMap;
}

ParameterSetMap GenDecoderParameterSetMap(const HevcDecoderConfigurationRecord& hevcRrd)
{
    std::vector<uint8_t> hevcSPS;
    std::vector<uint8_t> hevcPPS;
    std::vector<uint8_t> hevcVPS;
    hevcRrd.GetOneParameterSet(hevcSPS, HevcNalDefs::SPS);
    hevcRrd.GetOneParameterSet(hevcPPS, HevcNalDefs::PPS);
    hevcRrd.GetOneParameterSet(hevcVPS, HevcNalDefs::VPS);

    ParameterSetMap paramsMap;
    paramsMap.insert(pair<MediaCodecInfoType, DataVector>(MediaCodecInfoType::HEVC_SPS, move(hevcSPS)));
    paramsMap.insert(pair<MediaCodecInfoType, DataVector>(MediaCodecInfoType::HEVC_PPS, move(hevcPPS)));
    paramsMap.insert(pair<MediaCodecInfoType, DataVector>(MediaCodecInfoType::HEVC_VPS, move(hevcVPS)));

    return paramsMap;
}

ParameterSetMap GenDecoderParameterSetMap(const ElementaryStreamDescriptorAtom& eleDesRrd)
{
    std::vector<uint8_t> vcodecSpecInfo;
    ParameterSetMap paramsMap;
    if (eleDesRrd.GetOneParameterSet(vcodecSpecInfo))
    {
        paramsMap.insert(pair<MediaCodecInfoType, DataVector>(MediaCodecInfoType::AudioSpecificConfig,
                                                                      move(vcodecSpecInfo)));
    }
    return paramsMap;
}

VCD_MP4_END
