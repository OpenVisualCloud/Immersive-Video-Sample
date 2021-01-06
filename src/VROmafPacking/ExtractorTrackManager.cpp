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
//! \file:   ExtractorTrackManager.cpp
//! \brief:  Extractor track manager class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "ExtractorTrackManager.h"

VCD_NS_BEGIN

ExtractorTrackManager::ExtractorTrackManager()
{
    m_extractorTrackGen = NULL;
    m_initInfo = NULL;
    m_streams  = NULL;
}

ExtractorTrackManager::ExtractorTrackManager(InitialInfo *initInfo)
{
    m_extractorTrackGen = NULL;
    m_initInfo = initInfo;
    m_streams  = NULL;
}

ExtractorTrackManager::ExtractorTrackManager(const ExtractorTrackManager& src)
{
    m_extractorTrackGen = std::move(src.m_extractorTrackGen);
    m_initInfo = std::move(src.m_initInfo);
    m_streams  = std::move(src.m_streams);
}

ExtractorTrackManager& ExtractorTrackManager::operator=(ExtractorTrackManager&& other)
{
    m_extractorTrackGen = std::move(other.m_extractorTrackGen);
    m_initInfo = std::move(other.m_initInfo);
    m_streams  = std::move(other.m_streams);

    return *this;
}

ExtractorTrackManager::~ExtractorTrackManager()
{
    DELETE_MEMORY(m_extractorTrackGen);

    std::map<uint16_t, ExtractorTrack*>::iterator it;
    for (it = m_extractorTracks.begin(); it != m_extractorTracks.end();)
    {
        DELETE_MEMORY(it->second);

        m_extractorTracks.erase(it++);
    }
    m_extractorTracks.clear();
}

int32_t ExtractorTrackManager::AddExtractorTracks()
{
    int32_t ret = m_extractorTrackGen->GenerateExtractorTracks(m_extractorTracks, m_streams);
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t ExtractorTrackManager::Initialize(std::map<uint8_t, MediaStream*> *mediaStreams)
{
    if (!mediaStreams)
        return OMAF_ERROR_NULL_PTR;

    m_streams = mediaStreams;

    if (m_initInfo->packingPluginName)
    {
        OMAF_LOG(LOG_INFO, "Appoint plugin %s for extractor track generation !\n", (m_initInfo->packingPluginName));

        m_extractorTrackGen = new ExtractorTrackGenerator(m_initInfo, m_streams);
        if (!m_extractorTrackGen)
        {
            OMAF_LOG(LOG_ERROR, "Failed to create extractor track generator !\n");
            return OMAF_ERROR_NULL_PTR;
        }

        int32_t ret = m_extractorTrackGen->Initialize();
        if (ret)
            return ret;

        ret = AddExtractorTracks();
        if (ret)
            return ret;
    }
    else
    {
        OMAF_LOG(LOG_INFO, "No plugin appointed, so extractor track will not be generated !\n");
    }

    return ERROR_NONE;
}

VCD_NS_END
