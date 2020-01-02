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

 *
 */

//!
//! \file     MediaSource.cpp
//! \brief    Implement class for MediaSource.
//!

#include "MediaSource.h"

VCD_NS_BEGIN

MediaSource::MediaSource()
{
    m_mediaSourceInfo.width           = 0;
    m_mediaSourceInfo.height          = 0;
    m_mediaSourceInfo.projFormat      = VCD::OMAF::PF_UNKNOWN;
    m_mediaSourceInfo.pixFormat       = PixelFormat::INVALID;
    m_mediaSourceInfo.hasAudio        = false;
    m_mediaSourceInfo.audioChannel    = 0;
    m_mediaSourceInfo.stride          = 0;
    m_mediaSourceInfo.numberOfStreams = 0;
    m_mediaSourceInfo.frameRate       = 0;
    m_mediaSourceInfo.duration        = 0;
    m_mediaSourceInfo.frameNum        = 0;
    m_mediaSourceInfo.currentFrameNum = 0;
    m_mediaSourceInfo.sourceWH        = NULL;
    m_mediaSourceInfo.sourceNumber    = 0;
    isAllValid                        = false;
    m_sourceType                      = MediaSourceType::SOURCE_NONE;
}

MediaSource::~MediaSource()
{
    if (m_mediaSourceInfo.sourceWH != NULL)
    {
        delete m_mediaSourceInfo.sourceWH;
        m_mediaSourceInfo.sourceWH = NULL;
    }
}

VCD_NS_END
