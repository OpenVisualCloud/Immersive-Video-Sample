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
//! \file:   AvcParser.h
//! \brief:  AvcParser class
//! \detail: AVC parser difinition
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _AVCPARSER_H_
#define _AVCPARSER_H_

#include <cstdint>

#include "FormAllocator.h"

VCD_MP4_BEGIN

class Stream;

struct HRDParams    //!< HRD Parameters
{
    uint32_t cpbCntMinus1;
    uint8_t bitRateScale;
    uint8_t cpbSizeScale;
    std::vector<uint32_t> bitRateValueMinus1;
    std::vector<uint32_t> cpbSizeValueMinus1;
    std::vector<uint8_t> cbrFlag;
    uint8_t initialCpbRemovalDelayLengthMinus1;
    uint8_t cpbRemovalDelayLengthMinus1;
    uint8_t dpbOutputDelayLengthMinus1;
    uint8_t timeOffsetLength;
};

struct VUIParams    //!< VUI Parameters
{
    uint8_t aspectRatioInfoPresentFlag;
    uint8_t aspectRatioIdc;
    uint16_t sarWidth;
    uint16_t sarHeight;
    uint8_t overscanInfoPresentFlag;
    uint8_t overscanAppropriateFlag;
    uint8_t videoSignalTypePresentFlag;
    uint8_t videoFormat;
    uint8_t videoFullRangeFlag;
    uint8_t colourDescriptionPresentFlag;
    uint8_t colourPrimaries;
    uint8_t transferCharacteristics;
    uint8_t matrixCoefficients;
    uint8_t chromaLocInfoPresentFlag;
    uint32_t chromaSampleLocTypeTopField;
    uint32_t chromaSampleLocTypeBottomField;
    uint8_t timingInfoPresentFlag;
    uint32_t numUnitsInTick;
    uint32_t timeScale;
    uint8_t fixedFrameRateFlag;
    uint8_t nalHrdParametersPresentFlag;
    HRDParams nalHrdParameters;
    uint8_t vclHrdParametersPresentFlag;
    HRDParams vclHrdParameters;
    uint8_t lowDelayHrdFlag;
    uint8_t picStructPresentFlag;
    uint8_t bitstreamRestrictionFlag;
    uint8_t motionVectorsOverPicBoundariesFlag;
    uint32_t maxBytesPerPicDenom;
    uint32_t maxBitsPerMbDenom;
    uint32_t log2MaxMvLengthHorizontal;
    uint32_t log2MaxMvLengthVertical;
    uint32_t maxNumReorderFrames;
    uint32_t maxDecFrameBuffering;
};

struct SPSCfgs  //!< SPS configuration
{
    uint8_t profileIdc;
    uint8_t profileCompatibility;
    uint8_t levelIdc;
    uint32_t seqParameterSetId;
    uint32_t chromaFormatIdc;
    uint8_t separateColourPlaneFlag;
    uint32_t bitDepthLumaMinus8;
    uint32_t bitDepthChromaMinus8;
    uint8_t qpprimeYZeroTransformBypassFlag;
    uint8_t seqScalingMatrixPresentFlag;
    uint32_t log2MaxFrameNumMinus4;
    uint32_t picOrderCntType;
    uint32_t log2MaxPicOrderCntLsbMinus4;
    uint8_t deltaPicOrderAlwaysZeroFlag;
    int32_t offsetForNonRefPic;
    int32_t offsetForTopToBottomField;
    uint32_t numRefFramesInPicOrderCntCycle;
    std::vector<int32_t> offsetForRefFrame;
    uint32_t maxNumRefFrames;
    uint8_t gapsInFrameNumValueAllowedFlag;
    uint32_t picWidthInMbsMinus1;
    uint32_t picHeightInMapUnitsMinus1;
    uint8_t frameMbsOnlyFlag;
    uint8_t mbAdaptiveFrameFieldFlag;
    uint8_t direct8x8InferenceFlag;
    uint8_t frameCroppingFlag;
    uint32_t frameCropLeftOffset;
    uint32_t frameCropRightOffset;
    uint32_t frameCropTopOffset;
    uint32_t frameCropBottomOffset;
    uint8_t vuiParametersPresentFlag;
    VUIParams vuiParameters;
};

//!
//! \brief    Parse HRD Parameters
//!
//! \param    [in] Stream&
//!           bitstream
//! \param    [out] HRDParams&
//!           parsed HRD parameters
//!
//! \return   bool
//!           parse success or not
//!
bool parseHRD(Stream& str, HRDParams& retHdr);

//!
//! \brief    Parse VUI Parameters
//!
//! \param    [in] Stream&
//!           bitstream
//! \param    [out] VUIParams&
//!           parsed VUI parameters
//!
//! \return   bool
//!           parse success or not
//!
bool parseVUI(Stream& str, VUIParams& retVui);

//!
//! \brief    Parse SPS Parameters
//!
//! \param    [in] Stream&
//!           bitstream
//! \param    [out] SPSCfgs&
//!           parsed SPS parameters
//!
//! \return   bool
//!           parse success or not
//!
bool parseSPS(Stream& str, SPSCfgs& retSps);

VCD_MP4_END;
#endif /* _AVCPARSER_H_ */
