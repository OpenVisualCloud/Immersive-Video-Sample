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
//! \file:   Mp4DataTypes.cpp
//! \brief:  Mp4 file related data types operation implementation
//!

#define _SCL_SECURE_NO_WARNINGS

#include <algorithm>

#include "Mp4DataTypes.h"
#include "../atoms/Stream.h"
#include "../atoms/FormAllocator.h"

using namespace std;

VCD_MP4_BEGIN

template <typename T>
VarLenArray<T>::~VarLenArray()
{
    // CUSTOM_DELETE_ARRAY(arrayElets, T);
    if (NULL != arrayElets)
    {
        delete []arrayElets;
        arrayElets = NULL;
    }
}

template <typename T>
VarLenArray<T>::VarLenArray()
    : size(0)
    , arrayElets(nullptr)
{
}

template <typename T>
VarLenArray<T>::VarLenArray(size_t n)
    : size(n)
    , arrayElets(new T[n])
{
}

template <typename T>
VarLenArray<T>::VarLenArray(const VarLenArray& other)
    : size(other.size)
    , arrayElets(new T[other.size])
{
    copy(other.arrayElets, other.arrayElets + other.size, arrayElets);
}

template <typename T>
VarLenArray<T>& VarLenArray<T>::operator=(const VarLenArray<T>& other)
{
    if (this != &other)
    {
        // CUSTOM_DELETE_ARRAY(arrayElets, T);
        if (NULL != arrayElets)
        {
            delete []arrayElets;
            arrayElets = NULL;
        }
        size     = other.size;
        arrayElets = new T[size];
        copy(other.arrayElets, other.arrayElets + other.size, arrayElets);
    }
    return *this;
}

template struct VarLenArray<uint8_t>;
template struct VarLenArray<char>;
template struct VarLenArray<uint16_t>;
template struct VarLenArray<uint32_t>;
template struct VarLenArray<uint64_t>;
template struct VarLenArray<MediaCodecSpecInfo>;
template struct VarLenArray<ChannelLayout>;
template struct VarLenArray<FourCC>;
template struct VarLenArray<TStampID>;
template struct VarLenArray<TypeToTrackIDs>;
template struct VarLenArray<TrackSampInfo>;
template struct VarLenArray<TrackInformation>;
template struct VarLenArray<SegInfo>;
template struct VarLenArray<RWPKRegion>;
template struct VarLenArray<COVIRegion>;
template struct VarLenArray<SchemeType>;

VCD_MP4_END
