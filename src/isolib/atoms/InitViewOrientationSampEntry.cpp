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
//! \file:   InitViewOrientationSampEntry.cpp
//! \brief:  InitViewOrientationSampEntry class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "InitViewOrientationSampEntry.h"

VCD_MP4_BEGIN

InitViewOrient::InitViewOrient()
    : SphereRegionSampleEntryAtom("invo")
{
    auto& config = GetSphereRegionConfig();
    config.SetShapeMode(SphereRegionConfigAtom::ShapeMode::FourGreatCircles);
    config.SetDynamicRangeFlag(false);
    config.SetStaticAzimuthRange(0);
    config.SetStaticElevationRange(0);
}

InitViewOrient* InitViewOrient::Clone() const
{
    return (new InitViewOrient(*this));
}

void InitViewOrient::ToStream(Stream& str)
{
    SphereRegionSampleEntryAtom::ToStream(str);
    return;
}

void InitViewOrient::FromStream(Stream& str)
{
    SphereRegionSampleEntryAtom::FromStream(str);
    return;
}

InitViewOrient::InitViewSample::InitViewSample()
    : SphereRegionSample()
{
    regions.push_back(SphereRegion());
}

VCD_MP4_END