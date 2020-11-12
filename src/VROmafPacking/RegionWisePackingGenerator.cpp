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
//! \file:   RegionWisePackingGenerator.cpp
//! \brief:  Region wise packing generator wrapper class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <dlfcn.h>

#include "VideoStreamPluginAPI.h"
#include "RegionWisePackingGenerator.h"

VCD_NS_BEGIN

RegionWisePackingGenerator::RegionWisePackingGenerator()
{
    m_pluginHdl = NULL;
    m_rwpkGen = NULL;
}

RegionWisePackingGenerator::~RegionWisePackingGenerator()
{
    if (m_pluginHdl)
    {
        if (m_rwpkGen)
        {
            DestroyRWPKGenerator* destroyRWPKGen = NULL;
            destroyRWPKGen = (DestroyRWPKGenerator*)dlsym(m_pluginHdl, "Destroy");
            const char *dlsym_error = dlerror();
            if (dlsym_error)
            {
                OMAF_LOG(LOG_ERROR, "Failed to load symbol Destroy !\n");
                return;
            }

            if (!destroyRWPKGen)
            {
                OMAF_LOG(LOG_ERROR, "NULL RWPK destructor !\n");
                return;
            }
            destroyRWPKGen(m_rwpkGen);
        }

        dlclose(m_pluginHdl);
        m_pluginHdl = NULL;
    }
}

int32_t RegionWisePackingGenerator::Initialize(
    const char *rwpkGenPluginPath,
    const char *rwpkGenPluginName,
    std::map<uint8_t, MediaStream*> *streams,
    uint8_t *videoIdxInMedia,
    uint16_t tilesNumInViewport,
    uint16_t maxSelectedTilesNum,
    LogFunction logging)
{
    if (!streams || !videoIdxInMedia)
        return OMAF_ERROR_NULL_PTR;

    if (!rwpkGenPluginName)
    {
        OMAF_LOG(LOG_ERROR, "NULL OMAF DASH Packing plugin name !\n");
        return OMAF_INVALID_PLUGIN_PARAM;
    }

    if (!rwpkGenPluginPath)
        rwpkGenPluginPath = "./";

    int32_t ret = ERROR_NONE;

    uint32_t pathLen = strlen(rwpkGenPluginPath);

    char pluginLibName[1024];
    memset_s(pluginLibName, 1024, 0);
    if (rwpkGenPluginPath[pathLen - 1] == '/')
    {
        snprintf(pluginLibName, 1024, "%slib%s.so", rwpkGenPluginPath, rwpkGenPluginName);
    }
    else
    {
        snprintf(pluginLibName, 1024, "%s/lib%s.so", rwpkGenPluginPath, rwpkGenPluginName);
    }

    OMAF_LOG(LOG_INFO, "The plugin is %s\n", pluginLibName);

    m_pluginHdl = dlopen(pluginLibName, RTLD_LAZY);
    const char* dlsymErr1 = dlerror();
    if (!m_pluginHdl)
    {
        OMAF_LOG(LOG_ERROR, "Failed to open plugin lib %s\n", pluginLibName);
        if (dlsymErr1)
        {
            OMAF_LOG(LOG_ERROR, "Get error msg  %s\n", dlsymErr1);
        }
        return OMAF_ERROR_DLOPEN;
    }

    CreateRWPKGenerator* createRWPKGen = NULL;
    createRWPKGen = (CreateRWPKGenerator*)dlsym(m_pluginHdl, "Create");
    const char* dlsymErr2 = dlerror();
    if (dlsymErr2)
    {
        OMAF_LOG(LOG_ERROR, "Failed to load symbol Create: %s\n", dlsymErr2);
        return OMAF_ERROR_DLSYM;
    }

    if (!createRWPKGen)
    {
        OMAF_LOG(LOG_ERROR, "NULL RWPK generator !\n");
        return OMAF_ERROR_NULL_PTR;
    }
    m_rwpkGen = createRWPKGen();
    if (!m_rwpkGen)
    {
        OMAF_LOG(LOG_ERROR, "Failed to create RWPK generator !\n");
        return OMAF_ERROR_NULL_PTR;
    }

    std::map<uint8_t, VideoStreamInfo*> videoStreams;
    std::map<uint8_t, MediaStream*>::iterator itStr;
    for (itStr = streams->begin(); itStr != streams->end(); itStr++)
    {
        MediaStream *str = itStr->second;
        if (str->GetMediaType() == VIDEOTYPE)
        {
            VideoStream *vs = (VideoStream*)str;
            VideoStreamInfo *vsInfo = new VideoStreamInfo;
            if (!vsInfo)
            {
                std::map<uint8_t, VideoStreamInfo*>::iterator itInfo;
                for (itInfo = videoStreams.begin(); itInfo != videoStreams.end();)
                {
                    VideoStreamInfo *info = itInfo->second;
                    DELETE_MEMORY(info);
                    videoStreams.erase(itInfo++);
                }
                videoStreams.clear();
                return OMAF_ERROR_NULL_PTR;
            }

            vsInfo->tilesNumInRow = vs->GetTileInRow();
            vsInfo->tilesNumInCol = vs->GetTileInCol();
            vsInfo->srcRWPK       = vs->GetSrcRwpk();

            videoStreams.insert(std::make_pair(itStr->first, vsInfo));
        }
    }

    ret = m_rwpkGen->Initialize(
        &videoStreams, videoIdxInMedia,
        tilesNumInViewport, maxSelectedTilesNum, (void*)logging);
    if (ret)
    {
        OMAF_LOG(LOG_ERROR, "Failed to initialize RWPK generator !\n");
    }

    std::map<uint8_t, VideoStreamInfo*>::iterator itInfo1;
    for (itInfo1 = videoStreams.begin(); itInfo1 != videoStreams.end();)
    {
        VideoStreamInfo *info1 = itInfo1->second;
        DELETE_MEMORY(info1);
        videoStreams.erase(itInfo1++);
    }
    videoStreams.clear();

    return ret;
}

int32_t RegionWisePackingGenerator::GenerateDstRwpk(
    TileDef *tilesInViewport,
    RegionWisePacking *dstRwpk)
{
    int32_t ret = ERROR_NONE;
    if (m_rwpkGen)
    {
        ret = m_rwpkGen->GenerateDstRwpk(tilesInViewport, dstRwpk);
        if (ret)
        {
            OMAF_LOG(LOG_ERROR, "Failed to generate destinate RWPK !\n");
        }
    }
    else
    {
        OMAF_LOG(LOG_ERROR, "There is no RWPK generator !\n");
        ret = OMAF_ERROR_NULL_PTR;
    }
    return ret;
}

int32_t RegionWisePackingGenerator::GenerateTilesMergeDirection(
    TileDef *tilesInViewport,
    TilesMergeDirectionInCol *tilesMergeDir)
{
    int32_t ret = ERROR_NONE;
    if (m_rwpkGen)
    {
        ret = m_rwpkGen->GenerateTilesMergeDirection(tilesInViewport, tilesMergeDir);
        if (ret)
        {
            OMAF_LOG(LOG_ERROR, "Failed to generate tiles merge direction !\n");
        }
    }
    else
    {
        OMAF_LOG(LOG_ERROR, "There is no RWPK generator !\n");
        ret = OMAF_ERROR_NULL_PTR;
    }
    return ret;
}

uint32_t RegionWisePackingGenerator::GetTotalTilesNumInPackedPic()
{
    uint32_t num = 0;
    if (m_rwpkGen)
    {
        num = m_rwpkGen->GetTilesNumInPackedPic();
    }
    else
    {
        OMAF_LOG(LOG_ERROR, "There is no RWPK generator !\n");
    }

    return num;
}

uint32_t RegionWisePackingGenerator::GetPackedPicWidth()
{
    uint32_t width = 0;
    if (m_rwpkGen)
    {
        width =  m_rwpkGen->GetPackedPicWidth();
    }
    else
    {
        OMAF_LOG(LOG_ERROR, "There is no RWPK generator !\n");
    }

    return width;
}

uint32_t RegionWisePackingGenerator::GetPackedPicHeight()
{
    uint32_t height = 0;
    if (m_rwpkGen)
    {
        height = m_rwpkGen->GetPackedPicHeight();
    }
    else
    {
        OMAF_LOG(LOG_ERROR, "There is no RWPK generator !\n");
    }

    return height;
}

TileArrangement* RegionWisePackingGenerator::GetMergedTilesArrange()
{
    TileArrangement *tilesArr = NULL;
    if (m_rwpkGen)
    {
        tilesArr = m_rwpkGen->GetMergedTilesArrange();
    }
    else
    {
        OMAF_LOG(LOG_ERROR, "There is no RWPK generator !\n");
    }

    return tilesArr;
}

int32_t RegionWisePackingGenerator::GenerateMergedTilesArrange(TileDef *tilesInViewport)
{
    if (!tilesInViewport)
        return OMAF_ERROR_NULL_PTR;

    int32_t ret = ERROR_NONE;
    if (m_rwpkGen)
    {
        ret = m_rwpkGen->GenerateMergedTilesArrange(tilesInViewport);
        if (ret)
        {
            OMAF_LOG(LOG_ERROR, "Failed to generate merged tiles arrangement !\n");
        }
    }
    else
    {
        OMAF_LOG(LOG_ERROR, "There is no RWPK generator !\n");
        ret = OMAF_ERROR_NULL_PTR;
    }
    return ret;
}

VCD_NS_END
