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
//! \file:   Mp4ReaderUtil.h
//! \brief:  Mp4 reader basic utility functions definition
//! \detail: Define basic utility functions in mp4 reader
//!

#ifndef _MP4READERUTIL_H_
#define _MP4READERUTIL_H_

#include <functional>
#include <iterator>
#include <memory>

#include "Mp4DataTypes.h"
#include "Mp4ReaderImpl.h"
#include "../atoms/UriMetaSampEntryAtom.h"

using namespace std;

VCD_MP4_BEGIN

ChnlPropertyInternal GenChnl(const ChannelLayoutAtom& inChnlBox);

SA3DPropertyInternal GenSA3D(const SpatialAudioAtom& inSpaAudioBox);

OmniStereoScopic3D Genst3d(const Stereoscopic3D* inStereo3DBox);

OmniStereoScopic3D Genst3d(const SphericalVideoV1Atom& inStereo3DBox);

SphericalVideoV1Property GenSphericalVideoV1Property(const SphericalVideoV1Atom& inSpheVideoBox);

SphericalVideoV2Property Gensv3d(const SphericalVideoV2Atom* inSpheVideoBox);

RWPKPropertyInternal Genrwpk(const RegionWisePackingAtom& inRWPKBox);

COVIInformationInternal Gencovi(const CoverageInformationAtom& inCOVIBox);

bool IsImageType(FourCCInt inType);

ParameterSetMap GenDecoderParameterSetMap(const AvcDecoderConfigurationRecord& avcRrd);

ParameterSetMap GenDecoderParameterSetMap(const HevcDecoderConfigurationRecord& hevcRrd);

ParameterSetMap GenDecoderParameterSetMap(const ElementaryStreamDescriptorAtom& eleDesRrd);

template <typename InType,
          template <typename U> class InContainer,
          template <typename V> class OutContainer = InContainer,
          typename OutType                         = InType>
OutContainer<OutType> map(const InContainer<InType>& input, function<OutType(const InType&)> func)
{
    OutContainer<OutType> output;
    transform(input.begin(), input.end(), back_inserter(output), func);
    return output;
}

VCD_MP4_END;
#endif  // _MP4READERUTIL_H_
