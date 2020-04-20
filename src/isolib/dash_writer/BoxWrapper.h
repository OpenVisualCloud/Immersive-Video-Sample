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
//! \file:   BoxWrapper.h
//! \brief:  Upper layer box definition
//! \detail: Define upper layer box structures
//!

#ifndef _BOXDEF_H_
#define _BOXDEF_H_

#include <memory>

#include "FormAllocator.h"
#include "TypeAtom.h"
#include "HandlerAtom.h"
#include "MediaHeaderAtom.h"
#include "MovieAtom.h"
#include "ProjRelatedAtom.h"
#include "TrackHeaderAtom.h"

#include "SegmentWriter.h"

using namespace std;

VCD_MP4_BEGIN

struct FileTypeBoxWrapper
{
    UniquePtr<FileTypeAtom> fileTypeBox;
    FileTypeBoxWrapper(UniquePtr<FileTypeAtom>&& aFileTypeBox)
        : fileTypeBox(move(aFileTypeBox))
    {
    }
};

struct MovieBoxWrapper
{
    UniquePtr<MovieAtom> movieBox;
    MovieBoxWrapper(UniquePtr<MovieAtom>&& aMovieBox)
        : movieBox(move(aMovieBox))
    {
    }
};

struct MediaHeaderBoxWrapper
{
    UniquePtr<MediaHeaderAtom> mediaHeaderBox;
    MediaHeaderBoxWrapper(UniquePtr<MediaHeaderAtom>&& aMediaHeaderBox)
        : mediaHeaderBox(move(aMediaHeaderBox))
    {
    }
};

struct HandlerBoxWrapper
{
    UniquePtr<HandlerAtom> handlerBox;
    HandlerBoxWrapper(UniquePtr<HandlerAtom>&& aHandlerBox)
        : handlerBox(move(aHandlerBox))
    {
    }
};

struct TrackHeaderBoxWrapper
{
    UniquePtr<TrackHeaderAtom> trackHeaderBox;
    TrackHeaderBoxWrapper(UniquePtr<TrackHeaderAtom>&& aTrackHeaderBox)
        : trackHeaderBox(move(aTrackHeaderBox))
    {
    }
};

struct SampleEntryBoxWrapper
{
    UniquePtr<SampleEntryAtom> sampleEntryBox;
    SampleEntryBoxWrapper(UniquePtr<SampleEntryAtom>&& aSampleEntryBox)
        : sampleEntryBox(move(aSampleEntryBox))
    {
    }
};

struct RegionBlock
{
    UniquePtr<RegionWisePackingAtom::Region> region;
    RegionBlock(UniquePtr<RegionWisePackingAtom::Region>&& aRegion)
        : region(move(aRegion))
    {
    }
};

VCD_MP4_END;
#endif  // _BOXDEF_H_
