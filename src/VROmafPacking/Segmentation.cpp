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

#include <dlfcn.h>
#include "Segmentation.h"

VCD_NS_BEGIN

Segmentation::Segmentation()
{
    m_streamMap = NULL;
    m_extractorTrackMan = NULL;
    m_segInfo = NULL;
    m_trackIdStarter = 1;
    m_frameRate.num = 0;
    m_frameRate.den = 0;
    m_isCMAFEnabled = false;
    m_segWriterPluginPath = NULL;
    m_segWriterPluginName = NULL;
    m_segWriterPluginHdl  = NULL;
    m_sourceMode          = OMNIDIRECTIONAL_VIDEO_PACKING;
    m_mpdWriterPluginPath = NULL;
    m_mpdWriterPluginName = NULL;
    m_mpdWriterPluginHdl  = NULL;
}

Segmentation::Segmentation(
    std::map<uint8_t, MediaStream*> *streams,
    ExtractorTrackManager *extractorTrackMan,
    InitialInfo *initInfo,
    PackingSourceMode sourceMode)
{
    m_streamMap = streams;
    m_extractorTrackMan = extractorTrackMan;
    m_segInfo = initInfo->segmentationInfo;

    m_trackIdStarter = 1;
    m_frameRate.num = initInfo->bsBuffers[0].frameRate.num;
    m_frameRate.den = initInfo->bsBuffers[0].frameRate.den;

    m_isCMAFEnabled = initInfo->cmafEnabled;
    m_segWriterPluginPath = initInfo->segWriterPluginPath;
    m_segWriterPluginName = initInfo->segWriterPluginName;
    m_segWriterPluginHdl  = NULL;
    m_sourceMode          = sourceMode;
    m_mpdWriterPluginPath = initInfo->mpdWriterPluginPath;
    m_mpdWriterPluginName = initInfo->mpdWriterPluginName;
    m_mpdWriterPluginHdl  = NULL;
}

Segmentation::Segmentation(const Segmentation& src)
{
    m_streamMap = std::move(src.m_streamMap);
    m_extractorTrackMan = std::move(src.m_extractorTrackMan);
    m_segInfo = std::move(src.m_segInfo);

    m_trackIdStarter = src.m_trackIdStarter;
    m_frameRate.num = src.m_frameRate.num;
    m_frameRate.den = src.m_frameRate.den;
    m_isCMAFEnabled = src.m_isCMAFEnabled;
    m_segWriterPluginPath = std::move(src.m_segWriterPluginPath);
    m_segWriterPluginName = std::move(src.m_segWriterPluginName);
    m_segWriterPluginHdl  = std::move(src.m_segWriterPluginHdl);
    m_sourceMode          = src.m_sourceMode;
    m_mpdWriterPluginPath = std::move(src.m_mpdWriterPluginPath);
    m_mpdWriterPluginName = std::move(src.m_mpdWriterPluginName);
    m_mpdWriterPluginHdl  = std::move(src.m_mpdWriterPluginHdl);
}

Segmentation& Segmentation::operator=(Segmentation&& other)
{
    m_streamMap = std::move(other.m_streamMap);
    m_extractorTrackMan = std::move(other.m_extractorTrackMan);
    m_segInfo = std::move(other.m_segInfo);

    m_trackIdStarter = other.m_trackIdStarter;
    m_frameRate.num = other.m_frameRate.num;
    m_frameRate.den = other.m_frameRate.den;
    m_isCMAFEnabled = other.m_isCMAFEnabled;
    m_segWriterPluginPath = std::move(other.m_segWriterPluginPath);
    m_segWriterPluginName = std::move(other.m_segWriterPluginName);
    m_segWriterPluginHdl  = std::move(other.m_segWriterPluginHdl);
    m_sourceMode          = other.m_sourceMode;
    m_mpdWriterPluginPath = std::move(other.m_mpdWriterPluginPath);
    m_mpdWriterPluginName = std::move(other.m_mpdWriterPluginName);
    m_mpdWriterPluginHdl  = std::move(other.m_mpdWriterPluginHdl);

    return *this;
}

Segmentation::~Segmentation()
{
    if (m_segWriterPluginHdl)
    {
        dlclose(m_segWriterPluginHdl);
        m_segWriterPluginHdl = NULL;
    }

    if (m_mpdWriterPluginHdl)
    {
        dlclose(m_mpdWriterPluginHdl);
        m_mpdWriterPluginHdl = NULL;
    }
}

int32_t Segmentation::CreateSegWriterPluginHdl()
{
    if (!m_segWriterPluginPath || !m_segWriterPluginName)
    {
        OMAF_LOG(LOG_ERROR, "Segment generation plugin is not assigned ! \n");
        return OMAF_ERROR_NULL_PTR;
    }

    if (m_isCMAFEnabled)
    {
        if (0 == strcmp(m_segWriterPluginName, "SegmentWriter"))
        {
            OMAF_LOG(LOG_ERROR, "Plugin SegmentWriter can't generate CMAF segments !\n");
            return OMAF_ERROR_BAD_PARAM;
        }
    }

    uint32_t pathLen = strlen(m_segWriterPluginPath);

    char pluginLibName[1024];
    memset_s(pluginLibName, 1024, 0);
    if (m_segWriterPluginPath[pathLen - 1] == '/')
    {
        snprintf(pluginLibName, 1024, "%slib%s.so", m_segWriterPluginPath, m_segWriterPluginName);
    }
    else
    {
        snprintf(pluginLibName, 1024, "%s/lib%s.so", m_segWriterPluginPath, m_segWriterPluginName);
    }

    OMAF_LOG(LOG_INFO, "Segment generation plugin is %s\n", pluginLibName);

    m_segWriterPluginHdl = dlopen(pluginLibName, RTLD_LAZY);
    const char *dlsymErr1 = dlerror();
    if (!(m_segWriterPluginHdl))
    {
        OMAF_LOG(LOG_ERROR, "Failed to open segment writer plugin %s !\n", pluginLibName);
        if (dlsymErr1)
        {
            OMAF_LOG(LOG_ERROR, "Get error msg %s \n", dlsymErr1);
        }
        return OMAF_ERROR_DLOPEN;
    }

    return ERROR_NONE;
}

int32_t Segmentation::CreateMPDWriterPluginHdl()
{
    if (!m_mpdWriterPluginPath || !m_mpdWriterPluginName)
    {
        OMAF_LOG(LOG_ERROR, "DASH MPD writer plugin is not assigned ! \n");
        return OMAF_ERROR_NULL_PTR;
    }

    if (m_sourceMode == MULTIVIEW_VIDEO_PACKING)
    {
        if (0 == strcmp(m_mpdWriterPluginName, "MPDWriter"))
        {
            OMAF_LOG(LOG_ERROR, "Plugin MPDWriter can't generate MPD file for multi-view DASH packing !\n");
            return OMAF_ERROR_BAD_PARAM;
        }
    }

    uint32_t pathLen = strlen(m_mpdWriterPluginPath);

    char pluginLibName[1024];
    memset_s(pluginLibName, 1024, 0);
    if (m_mpdWriterPluginPath[pathLen - 1] == '/')
    {
        snprintf(pluginLibName, 1024, "%slib%s.so", m_mpdWriterPluginPath, m_mpdWriterPluginName);
    }
    else
    {
        snprintf(pluginLibName, 1024, "%s/lib%s.so", m_mpdWriterPluginPath, m_mpdWriterPluginName);
    }

    OMAF_LOG(LOG_INFO, "MPD generation plugin is %s\n", pluginLibName);

    m_mpdWriterPluginHdl = dlopen(pluginLibName, RTLD_LAZY);
    const char *dlsymErr1 = dlerror();
    if (!(m_mpdWriterPluginHdl))
    {
        OMAF_LOG(LOG_ERROR, "Failed to open MPD writer plugin %s !\n", pluginLibName);
        if (dlsymErr1)
        {
            OMAF_LOG(LOG_ERROR, "Get error msg %s \n", dlsymErr1);
        }
        return OMAF_ERROR_DLOPEN;
    }

    return ERROR_NONE;
}

int32_t Segmentation::Initialize()
{
    int32_t ret = ERROR_NONE;

    ret = CreateSegWriterPluginHdl();
    if (ret)
        return ret;

    ret = CreateMPDWriterPluginHdl();
    if (ret)
        return ret;

    return ERROR_NONE;
}

VCD_NS_END
