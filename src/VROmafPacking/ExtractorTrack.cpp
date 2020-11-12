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
//! \file:   ExtractorTrack.cpp
//! \brief:  Extractor track class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "360SCVPAPI.h"
#include "ExtractorTrack.h"
#include "VideoStreamPluginAPI.h"

VCD_NS_BEGIN

ExtractorTrack::ExtractorTrack()
{
    m_streams = NULL;
    m_viewportIdx = 0;
    m_projType = VCD::OMAF::ProjectionFormat::PF_ERP;

    m_dstRwpk = NULL;
    m_dstCovi = NULL;
    m_tilesMergeDir = NULL;
    m_vps = NULL;
    m_sps = NULL;
    m_pps = NULL;
    m_projSEI = NULL;
    m_rwpkSEI = NULL;

    m_processedFrmNum = 0;
    m_360scvpParam = NULL;
    m_dstWidth = 0;
    m_dstHeight = 0;
}

int32_t ExtractorTrack::Initialize()
{
    m_dstRwpk = new RegionWisePacking;
    if (!m_dstRwpk)
        return OMAF_ERROR_NULL_PTR;
    memset_s(m_dstRwpk, sizeof(RegionWisePacking), 0);

    m_dstCovi = new ContentCoverage;
    if (!m_dstCovi)
        return OMAF_ERROR_NULL_PTR;
    memset_s(m_dstCovi, sizeof(ContentCoverage), 0);

    m_tilesMergeDir = new TilesMergeDirectionInCol;
    if (!m_tilesMergeDir)
        return OMAF_ERROR_NULL_PTR;

    m_vps = new Nalu;
    if (!m_vps)
        return OMAF_ERROR_NULL_PTR;
    memset_s(m_vps, sizeof(Nalu), 0);

    m_sps = new Nalu;
    if (!m_sps)
        return OMAF_ERROR_NULL_PTR;
    memset_s(m_sps, sizeof(Nalu), 0);

    m_pps = new Nalu;
    if (!m_pps)
        return OMAF_ERROR_NULL_PTR;
    memset_s(m_pps, sizeof(Nalu), 0);

    m_projSEI = new Nalu;
    if (!m_projSEI)
        return OMAF_ERROR_NULL_PTR;
    memset_s(m_projSEI, sizeof(Nalu), 0);

    m_rwpkSEI = new Nalu;
    if (!m_rwpkSEI)
        return OMAF_ERROR_NULL_PTR;
    memset_s(m_rwpkSEI, sizeof(Nalu), 0);

    m_360scvpParam = new param_360SCVP;
    if (!m_360scvpParam)
        return OMAF_ERROR_NULL_PTR;
    memset_s(m_360scvpParam, sizeof(param_360SCVP), 0);

    return ERROR_NONE;
}

ExtractorTrack::ExtractorTrack(uint8_t viewportIdx, std::map<uint8_t, MediaStream*> *streams, uint16_t projType)
{
    m_streams = streams;
    m_viewportIdx = viewportIdx;
    m_projType = (VCD::OMAF::ProjectionFormat)projType;

    m_dstRwpk = NULL;
    m_dstCovi = NULL;
    m_tilesMergeDir = NULL;
    m_vps = NULL;
    m_sps = NULL;
    m_pps = NULL;
    m_projSEI = NULL;
    m_rwpkSEI = NULL;

    m_processedFrmNum = 0;
    m_360scvpParam = NULL;
    m_dstWidth = 0;
    m_dstHeight = 0;
}

ExtractorTrack::ExtractorTrack(const ExtractorTrack& src)
{
    m_streams = std::move(src.m_streams);
    m_viewportIdx = src.m_viewportIdx;
    m_projType = src.m_projType;

    m_dstRwpk = std::move(src.m_dstRwpk);
    m_dstCovi = std::move(src.m_dstCovi);
    m_tilesMergeDir = std::move(src.m_tilesMergeDir);
    m_vps = std::move(src.m_vps);
    m_sps = std::move(src.m_sps);
    m_pps = std::move(src.m_pps);
    m_projSEI = std::move(src.m_projSEI);
    m_rwpkSEI = std::move(src.m_rwpkSEI);

    m_processedFrmNum = src.m_processedFrmNum;
    m_360scvpParam = std::move(src.m_360scvpParam);
    m_dstWidth = src.m_dstWidth;
    m_dstHeight = src.m_dstHeight;
}

ExtractorTrack& ExtractorTrack::operator=(ExtractorTrack&& other)
{
    m_streams = std::move(other.m_streams);
    m_viewportIdx = other.m_viewportIdx;
    m_projType = other.m_projType;

    m_dstRwpk = std::move(other.m_dstRwpk);
    m_dstCovi = std::move(other.m_dstCovi);
    m_tilesMergeDir = std::move(other.m_tilesMergeDir);
    m_vps = std::move(other.m_vps);
    m_sps = std::move(other.m_sps);
    m_pps = std::move(other.m_pps);
    m_projSEI = std::move(other.m_projSEI);
    m_rwpkSEI = std::move(other.m_rwpkSEI);

    m_processedFrmNum = other.m_processedFrmNum;
    m_360scvpParam = std::move(other.m_360scvpParam);
    m_dstWidth = other.m_dstWidth;
    m_dstHeight = other.m_dstHeight;

    return *this;
}

ExtractorTrack::~ExtractorTrack()
{
    if (m_dstRwpk)
    {
        DELETE_ARRAY(m_dstRwpk->rectRegionPacking);
        delete m_dstRwpk;
        m_dstRwpk = NULL;
    }
    if (m_dstCovi)
    {
        DELETE_ARRAY(m_dstCovi->sphereRegions);
        delete m_dstCovi;
        m_dstCovi = NULL;
    }

    if (m_tilesMergeDir)
    {
        std::list<TilesInCol*>::iterator itCol;
        for (itCol = m_tilesMergeDir->tilesArrangeInCol.begin();
            itCol != m_tilesMergeDir->tilesArrangeInCol.end();)
        {
            TilesInCol *tileCol = *itCol;
            std::list<SingleTile*>::iterator itTile;
            for (itTile = tileCol->begin(); itTile != tileCol->end();)
            {
                SingleTile *tile = *itTile;
                DELETE_MEMORY(tile);
                itTile = tileCol->erase(itTile);
            }

            delete tileCol;
            tileCol = NULL;

            itCol = m_tilesMergeDir->tilesArrangeInCol.erase(itCol);
        }

        delete m_tilesMergeDir;
        m_tilesMergeDir = NULL;
    }

    std::map<uint8_t, Extractor*>::iterator it;
    for (it = m_extractors.begin(); it != m_extractors.end();)
    {
        if (it->second)
        {
            Extractor *extractor = it->second;
            std::list<SampleConstructor*>::iterator it1;
            for (it1 = extractor->sampleConstructor.begin();
                it1 != extractor->sampleConstructor.end();)
            {
                SampleConstructor *sampleCtor = *it1;
                DELETE_MEMORY(sampleCtor);
                it1 = extractor->sampleConstructor.erase(it1);
            }
            std::list<InlineConstructor*>::iterator it2;
            for (it2 = extractor->inlineConstructor.begin();
                it2 != extractor->inlineConstructor.end();)
            {
                InlineConstructor *inlineCtor = *it2;
                DELETE_ARRAY(inlineCtor->inlineData);
                DELETE_MEMORY(inlineCtor);
                it2 = extractor->inlineConstructor.erase(it2);
            }

            delete it->second;
            it->second = NULL;
        }
        m_extractors.erase(it++);
    }
    m_extractors.clear();

    if (m_vps)
    {
        DELETE_ARRAY(m_vps->data);
        delete m_vps;
        m_vps = NULL;
    }
    if (m_sps)
    {
        DELETE_ARRAY(m_sps->data);
        delete m_sps;
        m_sps = NULL;
    }
    if (m_pps)
    {
        DELETE_ARRAY(m_pps->data);
        delete m_pps;
        m_pps = NULL;
    }
    if (m_projSEI)
    {
        DELETE_ARRAY(m_projSEI->data);
        delete m_projSEI;
        m_projSEI = NULL;
    }
    if (m_rwpkSEI)
    {
        DELETE_ARRAY(m_rwpkSEI->data);
        delete m_rwpkSEI;
        m_rwpkSEI = NULL;
    }

    DELETE_MEMORY(m_360scvpParam);
    std::map<MediaStream*, void*>::iterator itHandle;
    for (itHandle = m_360scvpHandles.begin(); itHandle != m_360scvpHandles.end(); itHandle++)
    {
        void *scvpHandle = itHandle->second;
        if (scvpHandle)
            I360SCVP_unInit(scvpHandle);
    }
    m_360scvpHandles.clear();

    DestroyCurrSegNalus();
}

int32_t ExtractorTrack::ConstructExtractors()
{
    if (this->m_extractors.size() == 0)
    {
        int32_t ret = this->GenerateExtractors();
        if (ret)
            return ret;
    }
    else
    {
        int32_t ret = this->UpdateExtractors();
        if (ret)
            return ret;
    }

    return ERROR_NONE;
}

int32_t ExtractorTrack::GenerateExtractors()
{
    if (!m_tilesMergeDir)
        return OMAF_ERROR_NULL_PTR;

    std::list<TilesInCol*>::iterator itCol;
    uint16_t tileIdx = 0;
    for (itCol = m_tilesMergeDir->tilesArrangeInCol.begin();
        itCol != m_tilesMergeDir->tilesArrangeInCol.end(); itCol++)
    {
        TilesInCol *tileCol = *itCol;
        std::list<SingleTile*>::iterator itTile;
        for (itTile = tileCol->begin(); itTile != tileCol->end(); itTile++)
        {
            Extractor *extractor = new Extractor;
            if (!extractor)
                return OMAF_ERROR_NULL_PTR;

            SingleTile *tile = *itTile;
            uint8_t  vsIdx    = tile->streamIdxInMedia;
            uint8_t  origTileIdx  = tile->origTileIdx;
            uint16_t ctuIdx   = tile->dstCTUIndex;

            std::map<uint8_t, MediaStream*>::iterator itStream;
            itStream = m_streams->find(vsIdx);
            if (itStream == m_streams->end())
            {
                DELETE_MEMORY(extractor);
                return OMAF_ERROR_STREAM_NOT_FOUND;
            }

            VideoStream *video = (VideoStream*)(itStream->second);
            TileInfo *allTiles = video->GetAllTilesInfo();
            TileInfo *tileInfo = &(allTiles[origTileIdx]);

            InlineConstructor *inlineCtor = new InlineConstructor;
            if (!inlineCtor)
            {
                DELETE_MEMORY(extractor);
                return OMAF_ERROR_NULL_PTR;
            }

            memset_s(inlineCtor, sizeof(InlineConstructor), 0);

            inlineCtor->inlineData = new uint8_t[256];
            if (!inlineCtor->inlineData)
            {
                DELETE_MEMORY(extractor);
                DELETE_MEMORY(inlineCtor);
                return OMAF_ERROR_NULL_PTR;
            }
            memset_s(inlineCtor->inlineData, 256, 0);

            std::map<MediaStream*, void*>::iterator itHdl;
            itHdl = m_360scvpHandles.find((MediaStream*)video);
            if (itHdl == m_360scvpHandles.end())
            {
                void *handle = I360SCVP_New(video->Get360SCVPHandle());
                m_360scvpHandles.insert(std::make_pair((MediaStream*)video, handle));
            }
            void *m_360scvpHandle = m_360scvpHandles[(MediaStream*)video];
            memcpy_s(m_360scvpParam, sizeof(param_360SCVP), video->Get360SCVPParam(), sizeof(param_360SCVP));

            if (!m_dstWidth || !m_dstHeight)
            {
                DELETE_MEMORY(extractor);
                DELETE_ARRAY(inlineCtor->inlineData);
                DELETE_MEMORY(inlineCtor);
                return OMAF_ERROR_INVALID_DATA;
            }

            m_360scvpParam->destWidth = m_dstWidth;
            m_360scvpParam->destHeight = m_dstHeight;

            uint8_t *tempData = new uint8_t[tileInfo->tileNalu->dataSize];
            if (!tempData)
            {
                DELETE_MEMORY(extractor);
                DELETE_ARRAY(inlineCtor->inlineData);
                DELETE_MEMORY(inlineCtor);
                return OMAF_ERROR_NULL_PTR;
            }
            memcpy_s(tempData, tileInfo->tileNalu->dataSize, tileInfo->tileNalu->data, tileInfo->tileNalu->dataSize);

            tempData[0] = 0;
            tempData[1] = 0;
            tempData[2] = 0;
            tempData[3] = 1;

            m_360scvpParam->pInputBitstream = tempData;
            m_360scvpParam->inputBitstreamLen = tileInfo->tileNalu->dataSize;
            m_360scvpParam->pOutputBitstream = inlineCtor->inlineData;

            int32_t ret = I360SCVP_GenerateSliceHdr(m_360scvpParam, ctuIdx, m_360scvpHandle);
            if (ret)
            {
                DELETE_MEMORY(extractor);
                DELETE_ARRAY(inlineCtor->inlineData);
                DELETE_MEMORY(inlineCtor);
                DELETE_ARRAY(tempData);
                return OMAF_ERROR_SCVP_OPERATION_FAILED;
            }

            inlineCtor->length = DASH_SAMPLELENFIELD_SIZE + m_360scvpParam->outputBitstreamLen - HEVC_STARTCODES_LEN;

            memset_s(inlineCtor->inlineData, DASH_SAMPLELENFIELD_SIZE, 0xff);

            extractor->inlineConstructor.push_back(inlineCtor);

            SampleConstructor *sampleCtor = new SampleConstructor;
            if (!sampleCtor)
            {
                DELETE_MEMORY(extractor);
                DELETE_ARRAY(inlineCtor->inlineData);
                DELETE_MEMORY(inlineCtor);
                DELETE_ARRAY(tempData);
                return OMAF_ERROR_NULL_PTR;
            }

            sampleCtor->streamIdx = vsIdx;
            sampleCtor->trackRefIndex = origTileIdx; //changed later in segmentation
            sampleCtor->sampleOffset  = 0;
            sampleCtor->dataOffset    = DASH_SAMPLELENFIELD_SIZE + HEVC_NALUHEADER_LEN + tileInfo->tileNalu->sliceHeaderLen;
            sampleCtor->dataLength    = tileInfo->tileNalu->dataSize -
                                        tileInfo->tileNalu->startCodesSize -
                                        HEVC_NALUHEADER_LEN - tileInfo->tileNalu->sliceHeaderLen;

            extractor->sampleConstructor.push_back(sampleCtor);

            m_extractors.insert(std::make_pair(tileIdx, extractor));

            tileIdx++;
            DELETE_ARRAY(tempData);
        }
    }
    return ERROR_NONE;
}

int32_t ExtractorTrack::DestroyExtractors()
{
    std::map<uint8_t, Extractor*>::iterator itExtractor;
    for (itExtractor = m_extractors.begin(); itExtractor != m_extractors.end();)
    {
        Extractor *extractor = itExtractor->second;

        std::list<InlineConstructor*>::iterator itInlineCtor;
        for (itInlineCtor = extractor->inlineConstructor.begin();
            itInlineCtor != extractor->inlineConstructor.end(); )
        {
            InlineConstructor *inlineCtor = *itInlineCtor;
            inlineCtor->length = 0;
            DELETE_ARRAY(inlineCtor->inlineData);
            DELETE_MEMORY(inlineCtor);
            extractor->inlineConstructor.erase(itInlineCtor++);
        }
        extractor->inlineConstructor.clear();

        std::list<SampleConstructor*>::iterator itSmpCtor;
        for (itSmpCtor = extractor->sampleConstructor.begin();
            itSmpCtor != extractor->sampleConstructor.end(); )
        {
            SampleConstructor *sampleCtor = *itSmpCtor;
            DELETE_MEMORY(sampleCtor);
            extractor->sampleConstructor.erase(itSmpCtor++);
        }
        extractor->sampleConstructor.clear();

        DELETE_MEMORY(extractor);

        m_extractors.erase(itExtractor++);
    }
    m_extractors.clear();

    return ERROR_NONE;
}

void ExtractorTrack::DestroyCurrSegNalus()
{
    std::list<uint8_t*>::iterator it;
    for (it = m_naluDataForOneSeg.begin();
        it != m_naluDataForOneSeg.end(); )
    {
        uint8_t *data = *it;
        if (data)
        {
            free(data);
            data = NULL;
        }

        m_naluDataForOneSeg.erase(it++);
    }

    m_naluDataForOneSeg.clear();
}

int32_t ExtractorTrack::UpdateExtractors()
{
    if (!m_tilesMergeDir)
        return OMAF_ERROR_NULL_PTR;

    if (m_extractors.size() == 0)
        return OMAF_ERROR_INVALID_DATA;

    std::map<uint8_t, Extractor*>::iterator itExtractor;
    std::list<TilesInCol*>::iterator itCol;
    uint16_t tileIdx = 0;
    for (itCol = m_tilesMergeDir->tilesArrangeInCol.begin();
        itCol != m_tilesMergeDir->tilesArrangeInCol.end(); itCol++)
    {
        TilesInCol *tileCol = *itCol;
        std::list<SingleTile*>::iterator itTile;
        for (itTile = tileCol->begin(); itTile != tileCol->end(); itTile++)
        {
            itExtractor = m_extractors.find(tileIdx);
            if (itExtractor == m_extractors.end())
                return OMAF_ERROR_EXTRACTOR_NOT_FOUND;

            Extractor *extractor = itExtractor->second;
            if (!extractor)
                return OMAF_ERROR_NULL_PTR;

            SingleTile *tile = *itTile;
            uint8_t  vsIdx    = tile->streamIdxInMedia;
            uint8_t  origTileIdx  = tile->origTileIdx;
            uint16_t ctuIdx   = tile->dstCTUIndex;

            std::map<uint8_t, MediaStream*>::iterator itStream;
            itStream = m_streams->find(vsIdx);
            if (itStream == m_streams->end())
                return OMAF_ERROR_STREAM_NOT_FOUND;

            VideoStream *video = (VideoStream*)(itStream->second);
            TileInfo *allTiles = video->GetAllTilesInfo();
            TileInfo *tileInfo = &(allTiles[origTileIdx]);

            InlineConstructor *inlineCtor = extractor->inlineConstructor.front();
            if (!inlineCtor)
                return OMAF_ERROR_NULL_PTR;

            if (!(inlineCtor->inlineData))
                return OMAF_ERROR_NULL_PTR;
            memset_s(inlineCtor->inlineData, 256, 0);

            void *m_360scvpHandle = m_360scvpHandles[(MediaStream*)video];
            memcpy_s(m_360scvpParam, sizeof(param_360SCVP), video->Get360SCVPParam(), sizeof(param_360SCVP));

            m_360scvpParam->destWidth = m_dstWidth;
            m_360scvpParam->destHeight = m_dstHeight;

            uint8_t *tempData = new uint8_t[tileInfo->tileNalu->dataSize];
            if (!tempData)
                return OMAF_ERROR_NULL_PTR;

            memcpy_s(tempData, tileInfo->tileNalu->dataSize, tileInfo->tileNalu->data, tileInfo->tileNalu->dataSize);

            tempData[0] = 0;
            tempData[1] = 0;
            tempData[2] = 0;
            tempData[3] = 1;

            m_360scvpParam->pInputBitstream = tempData;
            m_360scvpParam->inputBitstreamLen = tileInfo->tileNalu->dataSize;
            m_360scvpParam->pOutputBitstream  = inlineCtor->inlineData;

            int32_t ret = I360SCVP_GenerateSliceHdr(m_360scvpParam, ctuIdx, m_360scvpHandle);
            if (ret)
            {
                DELETE_ARRAY(tempData);
                return OMAF_ERROR_SCVP_OPERATION_FAILED;
            }

            inlineCtor->length = DASH_SAMPLELENFIELD_SIZE + m_360scvpParam->outputBitstreamLen - HEVC_STARTCODES_LEN;

            memset_s(inlineCtor->inlineData, DASH_SAMPLELENFIELD_SIZE, 0xff);

            SampleConstructor *sampleCtor = extractor->sampleConstructor.front();
            if (!sampleCtor)
            {
                DELETE_ARRAY(tempData);
                return OMAF_ERROR_NULL_PTR;
            }

            sampleCtor->dataOffset    = DASH_SAMPLELENFIELD_SIZE + HEVC_NALUHEADER_LEN + tileInfo->tileNalu->sliceHeaderLen;
            sampleCtor->dataLength = tileInfo->tileNalu->dataSize -
                                     tileInfo->tileNalu->startCodesSize -
                                     HEVC_NALUHEADER_LEN - tileInfo->tileNalu->sliceHeaderLen;


            tileIdx++;
            DELETE_ARRAY(tempData);
        }
    }
    return ERROR_NONE;
}

int32_t ExtractorTrack::GenerateProjectionSEI()
{
    std::map<uint8_t, MediaStream*>::iterator itStream;
    MediaStream *stream = NULL;
    for (itStream = m_streams->begin(); itStream != m_streams->end(); itStream++)
    {
        stream = itStream->second;
        if (!stream)
            return OMAF_ERROR_NULL_PTR;

        if (stream->GetMediaType() == VIDEOTYPE)
            break;
    }

    if (itStream == m_streams->end())
        return OMAF_ERROR_STREAM_NOT_FOUND;

    VideoStream *vs = (VideoStream*)stream;
    if (!vs)
        return OMAF_ERROR_NULL_PTR;

    void *scvpHandle = vs->Get360SCVPHandle();
    if (!scvpHandle)
        return OMAF_ERROR_NULL_PTR;

    param_360SCVP *scvpParam = vs->Get360SCVPParam();
    if (!scvpParam)
        return OMAF_ERROR_NULL_PTR;

    m_projSEI->data = new uint8_t[256];
    if (!(m_projSEI->data))
        return OMAF_ERROR_NULL_PTR;

    scvpParam->pOutputBitstream = m_projSEI->data;

    if (vs->GetProjType() == VCD::OMAF::ProjectionFormat::PF_ERP)
    {
        int32_t ret = I360SCVP_GenerateProj(
                         scvpHandle,
                         E_EQUIRECT_PROJECTION,
                         scvpParam->pOutputBitstream,
                         (int32_t*)&(scvpParam->outputBitstreamLen));
        if (ret)
            return OMAF_ERROR_SCVP_PROCESS_FAILED;
    }
    else if (vs->GetProjType() == VCD::OMAF::ProjectionFormat::PF_CUBEMAP)
    {

        int32_t ret = I360SCVP_GenerateProj(
                         scvpHandle,
                         E_CUBEMAP_PROJECTION,
                         scvpParam->pOutputBitstream,
                         (int32_t*)&(scvpParam->outputBitstreamLen));
        if (ret)
            return OMAF_ERROR_SCVP_PROCESS_FAILED;
    }
    else
    {
        return OMAF_ERROR_UNDEFINED_OPERATION;
    }

    m_projSEI->dataSize = scvpParam->outputBitstreamLen;

    uint32_t actualSize = m_projSEI->dataSize - HEVC_STARTCODES_LEN;
    m_projSEI->data[0] = (uint8_t)((actualSize >> 24) & 0x000000ff);
    m_projSEI->data[1] = (uint8_t)((actualSize >> 16) & 0x000000ff);
    m_projSEI->data[2] = (uint8_t)((actualSize >> 8)  & 0x000000ff);
    m_projSEI->data[3] = (uint8_t)((actualSize >> 0)  & 0x000000ff);

    return ERROR_NONE;
}

Nalu* ExtractorTrack::GetProjectionSEI()
{
    if (m_projSEI->data)
    {
        return m_projSEI;
    }
    else
    {
        GenerateProjectionSEI();
        return m_projSEI;
    }
}

int32_t ExtractorTrack::GenerateRwpkSEI()
{
    std::map<uint8_t, MediaStream*>::iterator itStream;
    MediaStream *stream = NULL;
    for (itStream = m_streams->begin(); itStream != m_streams->end(); itStream++)
    {
        stream = itStream->second;
        if (!stream)
            return OMAF_ERROR_STREAM_NOT_FOUND;

        if (stream->GetMediaType() == VIDEOTYPE)
            break;
    }

    if (itStream == m_streams->end())
        return OMAF_ERROR_STREAM_NOT_FOUND;

    VideoStream *vs = (VideoStream*)stream;
    if (!vs)
        return OMAF_ERROR_STREAM_NOT_FOUND;

    void *scvpHandle = vs->Get360SCVPHandle();
    if (!scvpHandle)
        return OMAF_ERROR_NULL_PTR;

    param_360SCVP *scvpParam = vs->Get360SCVPParam();
    if (!scvpParam)
        return OMAF_ERROR_NULL_PTR;

    m_rwpkSEI->data = new uint8_t[256];
    if (!(m_rwpkSEI->data))
        return OMAF_ERROR_NULL_PTR;

    scvpParam->pOutputBitstream = m_rwpkSEI->data;

    int32_t ret = I360SCVP_GenerateRWPK(
                     scvpHandle,
                     m_dstRwpk,
                     scvpParam->pOutputBitstream,
                     (int32_t*)&(scvpParam->outputBitstreamLen));
    if (ret)
        return OMAF_ERROR_SCVP_PROCESS_FAILED;

    m_rwpkSEI->dataSize = scvpParam->outputBitstreamLen;

    uint32_t actualSize = m_rwpkSEI->dataSize - HEVC_STARTCODES_LEN;
    m_rwpkSEI->data[0] = (uint8_t)((actualSize >> 24) & 0x000000ff);
    m_rwpkSEI->data[1] = (uint8_t)((actualSize >> 16) & 0x000000ff);
    m_rwpkSEI->data[2] = (uint8_t)((actualSize >> 8)  & 0x000000ff);
    m_rwpkSEI->data[3] = (uint8_t)((actualSize >> 0)  & 0x000000ff);

    return ERROR_NONE;
}

Nalu* ExtractorTrack::GetRwpkSEI()
{
    if (m_rwpkSEI->data)
    {
        return m_rwpkSEI;
    }
    else
    {
        GenerateRwpkSEI();
        return m_rwpkSEI;
    }
}

int32_t ExtractorTrack::SetNalu(Nalu *srcNalu, Nalu *dstNalu)
{
    if (!srcNalu || !dstNalu)
        return OMAF_ERROR_NULL_PTR;

    if (!(srcNalu->data) || !(srcNalu->dataSize))
        return OMAF_ERROR_INVALID_DATA;

    if (dstNalu->data || dstNalu->dataSize)
        return OMAF_ERROR_INVALID_DATA;

    dstNalu->dataSize = srcNalu->dataSize;
    dstNalu->data = new uint8_t[dstNalu->dataSize];
    if (!(dstNalu->data))
        return OMAF_ERROR_NULL_PTR;

    memcpy_s(dstNalu->data, srcNalu->dataSize, srcNalu->data, srcNalu->dataSize);

    dstNalu->startCodesSize = srcNalu->startCodesSize;
    dstNalu->naluType       = srcNalu->naluType;
    dstNalu->seiPayloadType = srcNalu->seiPayloadType;
    dstNalu->sliceHeaderLen = srcNalu->sliceHeaderLen;

    return ERROR_NONE;
}

Nalu* ExtractorTrack::GetVPS()
{
    return m_vps;
}

Nalu* ExtractorTrack::GetSPS()
{
    return m_sps;
}

Nalu* ExtractorTrack::GetPPS()
{
    return m_pps;
}

VCD_NS_END
