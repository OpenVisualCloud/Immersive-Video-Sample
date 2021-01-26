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
//! \file:   Segmentation.cpp
//! \brief:  Segmentation base class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "Segmentation.h"

VCD_NS_BEGIN

Segmentation::Segmentation()
{
    m_streamMap = NULL;
    m_extractorTrackMan = NULL;
    m_mpdGen = NULL;
    m_segInfo = NULL;
    m_trackIdStarter = 1;
    m_frameRate.num = 0;
    m_frameRate.den = 0;
}

Segmentation::Segmentation(
    std::map<uint8_t, MediaStream*> *streams,
    ExtractorTrackManager *extractorTrackMan,
    InitialInfo *initInfo)
{
    m_streamMap = streams;
    m_extractorTrackMan = extractorTrackMan;
    m_segInfo = initInfo->segmentationInfo;

    m_mpdGen = NULL;
    m_trackIdStarter = 1;
    m_frameRate.num = initInfo->bsBuffers[0].frameRate.num;
    m_frameRate.den = initInfo->bsBuffers[0].frameRate.den;
}

Segmentation::Segmentation(const Segmentation& src)
{
    m_streamMap = std::move(src.m_streamMap);
    m_extractorTrackMan = std::move(src.m_extractorTrackMan);
    m_segInfo = std::move(src.m_segInfo);

    m_mpdGen = std::move(src.m_mpdGen);
    m_trackIdStarter = src.m_trackIdStarter;
    m_frameRate.num = src.m_frameRate.num;
    m_frameRate.den = src.m_frameRate.den;
}

Segmentation& Segmentation::operator=(Segmentation&& other)
{
    m_streamMap = std::move(other.m_streamMap);
    m_extractorTrackMan = std::move(other.m_extractorTrackMan);
    m_segInfo = std::move(other.m_segInfo);

    m_mpdGen = std::move(other.m_mpdGen);
    m_trackIdStarter = other.m_trackIdStarter;
    m_frameRate.num = other.m_frameRate.num;
    m_frameRate.den = other.m_frameRate.den;

    return *this;
}

Segmentation::~Segmentation()
{
    DELETE_MEMORY(m_mpdGen);
}

VCD_NS_END
