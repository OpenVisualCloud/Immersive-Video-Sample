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
//! \file:   AvcParser.cpp
//! \brief:  AvcParser class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "AvcParser.h"
#include "Stream.h"

VCD_MP4_BEGIN

bool parseHRD(Stream& str, HRDParams& retHdr)
{
    HRDParams params{};

    params.cpbCntMinus1 = str.ReadExpGolombCode();                // 0 | 5 	ue(v)
    params.bitRateScale = static_cast<uint8_t>(str.Read1(4));  // 0 | 5 	u(4)
    params.cpbSizeScale = static_cast<uint8_t>(str.Read1(4));  // 0 | 5 	u(4)
    params.bitRateValueMinus1.resize(params.cpbCntMinus1 + 1);
    params.cpbSizeValueMinus1.resize(params.cpbCntMinus1 + 1);
    params.cbrFlag.resize(params.cpbCntMinus1 + 1);
    for (size_t idx = 0; idx <= params.cpbCntMinus1; idx++)
    {
        params.bitRateValueMinus1[idx] = str.ReadExpGolombCode();                // 0 | 5 	ue(v)
        params.cpbSizeValueMinus1[idx] = str.ReadExpGolombCode();                // 0 | 5 	ue(v)
        params.cbrFlag[idx]              = static_cast<uint8_t>(str.Read1(1));  // 0 | 5 	u(1)
    }
    params.initialCpbRemovalDelayLengthMinus1 = static_cast<uint8_t>(str.Read1(5));  // 0 | 5 	u(5)
    params.cpbRemovalDelayLengthMinus1         = static_cast<uint8_t>(str.Read1(5));  // 0 | 5 	u(5)
    params.dpbOutputDelayLengthMinus1          = static_cast<uint8_t>(str.Read1(5));  // 0 | 5 	u(5)
    params.timeOffsetLength                      = static_cast<uint8_t>(str.Read1(5));  // 0 | 5 	u(5)

    retHdr = params;
    return true;
}

bool parseVUI(Stream& str, VUIParams& retVui)
{
    VUIParams params{};
    params.aspectRatioInfoPresentFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
    if (params.aspectRatioInfoPresentFlag)
    {
        params.aspectRatioIdc = static_cast<uint8_t>(str.Read1(8));  // 0  u(8)
        if (params.aspectRatioIdc == 255 /* Extended_SAR */)
        {
            params.sarWidth  = static_cast<uint16_t>(str.Read1(16));  // 0  u(16)
            params.sarHeight = static_cast<uint16_t>(str.Read1(16));  // 0  u(16)
        };
    };
    params.overscanInfoPresentFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
    if (params.overscanInfoPresentFlag)
        params.overscanAppropriateFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
    params.videoSignalTypePresentFlag = static_cast<uint8_t>(str.Read1(1));
    ;  // 0  u(1)
    if (params.videoSignalTypePresentFlag)
    {
        params.videoFormat                    = static_cast<uint8_t>(str.Read1(3));  // 0  u(3)
        params.videoFullRangeFlag             = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
        params.colourDescriptionPresentFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
        if (params.colourDescriptionPresentFlag)
        {
            params.colourPrimaries         = static_cast<uint8_t>(str.Read1(8));  // 0  u(8)
            params.transferCharacteristics = static_cast<uint8_t>(str.Read1(8));  // 0  u(8)
            params.matrixCoefficients      = static_cast<uint8_t>(str.Read1(8));  // 0  u(8)
        };
    };
    params.chromaLocInfoPresentFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
    if (params.chromaLocInfoPresentFlag)
    {
        params.chromaSampleLocTypeTopField    = static_cast<uint8_t>(str.ReadExpGolombCode());  // 0  ue(v)
        params.chromaSampleLocTypeBottomField = static_cast<uint8_t>(str.ReadExpGolombCode());  // 0  ue(v)
    };
    params.timingInfoPresentFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
    if (params.timingInfoPresentFlag)
    {
        params.numUnitsInTick     = str.Read32();                       // 0  u(32)
        params.timeScale            = str.Read32();                       // 0  u(32)
        params.fixedFrameRateFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
    };
    params.nalHrdParametersPresentFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
    if (params.nalHrdParametersPresentFlag)
    {
        if (!parseHRD(str, params.nalHrdParameters))
        {
            return false;
        }
    }
    params.vclHrdParametersPresentFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
    if (params.vclHrdParametersPresentFlag)
    {
        if (!parseHRD(str, params.vclHrdParameters))
        {
            return false;
        }
    }
    if (params.vclHrdParametersPresentFlag || params.vclHrdParametersPresentFlag)
    {
        params.lowDelayHrdFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1);
    }
    params.picStructPresentFlag    = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
    params.bitstreamRestrictionFlag = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
    if (params.bitstreamRestrictionFlag)
    {
        params.motionVectorsOverPicBoundariesFlag  = static_cast<uint8_t>(str.Read1(1));  // 0  u(1)
        params.maxBytesPerPicDenom                 = str.ReadExpGolombCode();                // 0  ue(v)
        params.maxBitsPerMbDenom                   = str.ReadExpGolombCode();                // 0  ue(v)
        params.log2MaxMvLengthHorizontal           = str.ReadExpGolombCode();                // 0  ue(v)
        params.log2MaxMvLengthVertical             = str.ReadExpGolombCode();                // 0  ue(v)
        params.maxNumReorderFrames                 = str.ReadExpGolombCode();                // 0  ue(v)
        params.maxDecFrameBuffering                = str.ReadExpGolombCode();                // 0  ue(v)
    }

    retVui = params;
    return true;
}

bool parseSPS(Stream& str, SPSCfgs& retSps)
{
    SPSCfgs params{};
    params.profileIdc           = static_cast<uint8_t>(str.Read1(8));  // 0 u(8)
    params.profileCompatibility = static_cast<uint8_t>(str.Read1(8));  // contains a bunch of flags
    params.levelIdc             = static_cast<uint8_t>(str.Read1(8));  // 0 u(8)
    params.seqParameterSetId  = str.ReadExpGolombCode();                // 0 ue(v)
    if (params.profileIdc == 100 || params.profileIdc == 110 || params.profileIdc == 122 || params.profileIdc == 244 ||
        params.profileIdc == 44 || params.profileIdc == 83 || params.profileIdc == 86 || params.profileIdc == 118 ||
        params.profileIdc == 128 || params.profileIdc == 138 || params.profileIdc == 139 || params.profileIdc == 134 ||
        params.profileIdc == 135)
    {
        params.chromaFormatIdc = str.ReadExpGolombCode();  // 0  ue(v)
        if (params.chromaFormatIdc == 3)
        {
            params.separateColourPlaneFlag = static_cast<uint8_t>(str.Read1(1));  // 0 u(1)
        }
        params.bitDepthChromaMinus8              = str.ReadExpGolombCode();                // 0 ue(v)
        params.qpprimeYZeroTransformBypassFlag = static_cast<uint8_t>(str.Read1(1));  // 0 u(1)
        params.seqScalingMatrixPresentFlag      = static_cast<uint8_t>(str.Read1(1));  // 0 u(1)
        if (params.seqScalingMatrixPresentFlag)
        {
            ISO_LOG(LOG_ERROR, "seq scale matrix is not supported!!\n");
            return false;
        }
    }
    params.log2MaxFrameNumMinus4 = str.ReadExpGolombCode();  // 0 ue(v)
    params.picOrderCntType        = str.ReadExpGolombCode();  // 0 ue(v)
    if (params.picOrderCntType == 0)
    {
        params.log2MaxPicOrderCntLsbMinus4 = str.ReadExpGolombCode();  // 0 ue(v)
    }
    else
    {
        if (params.picOrderCntType == 1)
        {
            params.deltaPicOrderAlwaysZeroFlag      = static_cast<uint8_t>(str.Read1(1));  // 0 u(1)
            params.offsetForNonRefPic                = str.ReadSignedExpGolombCode();          // 0 se(v)
            params.offsetForTopToBottomField        = str.ReadSignedExpGolombCode();          // 0 se(v)
            params.numRefFramesInPicOrderCntCycle = str.ReadExpGolombCode();                // 0 ue(v)
            params.offsetForRefFrame.resize(params.numRefFramesInPicOrderCntCycle);
            for (size_t i = 0; i < params.numRefFramesInPicOrderCntCycle; i++)
            {
                params.offsetForRefFrame[i] = str.ReadSignedExpGolombCode();  // 0 se(v)
            }
        }
    }
    params.maxNumRefFrames                   = str.ReadExpGolombCode();                // 0  ue(v)
    params.gapsInFrameNumValueAllowedFlag = static_cast<uint8_t>(str.Read1(1));  // 0        u(1)
    params.picWidthInMbsMinus1              = str.ReadExpGolombCode();                // 0     ue(v)
    params.picHeightInMapUnitsMinus1       = str.ReadExpGolombCode();                // 0      ue(v)
    params.frameMbsOnlyFlag                  = static_cast<uint8_t>(str.Read1(1));  // 0 u(1)
    if (!params.frameMbsOnlyFlag)
    {
        params.mbAdaptiveFrameFieldFlag = static_cast<uint8_t>(str.Read1(1));  // 0        u(1)
    }
    params.direct8x8InferenceFlag = static_cast<uint8_t>(str.Read1(1));  // 0   u(1)
    params.frameCroppingFlag       = static_cast<uint8_t>(str.Read1(1));  // 0 u(1)
    if (params.frameCroppingFlag)
    {
        params.frameCropLeftOffset   = str.ReadExpGolombCode();  // 0  ue(v)
        params.frameCropRightOffset  = str.ReadExpGolombCode();  // 0 ue(v)
        params.frameCropTopOffset    = str.ReadExpGolombCode();  // 0   ue(v)
        params.frameCropBottomOffset = str.ReadExpGolombCode();  // 0        ue(v)
    }
    params.vuiParametersPresentFlag = static_cast<uint8_t>(str.Read1(1));  // 0 u(1)
    if (params.vuiParametersPresentFlag)
    {
        ISO_LOG(LOG_ERROR, "the feather doesn't work!!\n");
    }
    retSps = params;
    return true;
}

VCD_MP4_END