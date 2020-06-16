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

#include "OmafTilesStitch.h"
#include "math.h"

VCD_OMAF_BEGIN

OmafTilesStitch::OmafTilesStitch()
{
    m_360scvpParam  = NULL;
    m_360scvpHandle = NULL;
    m_fullResVideoHeader = NULL;
    m_fullResVPSSize  = 0;
    m_fullResSPSSize  = 0;
    m_fullResPPSSize  = 0;
    m_fullWidth     = 0;
    m_fullHeight    = 0;
    m_mainMergedWidth   = 0;
    m_mainMergedHeight  = 0;
    m_mainMergedTileRows = 0;
    m_mainMergedTileCols = 0;
    m_needHeaders        = false;
    m_isInitialized      = false;
}

OmafTilesStitch::~OmafTilesStitch()
{
    if (m_selectedTiles.size())
    {
        std::map<uint32_t, std::map<uint32_t, MediaPacket*>>::iterator it;
        for (it = m_selectedTiles.begin(); it != m_selectedTiles.end(); )
        {
            std::map<uint32_t, MediaPacket*> packets = it->second;
            if (packets.size())
            {
                std::map<uint32_t, MediaPacket*>::iterator it1;
                for (it1 = packets.begin(); it1 != packets.end(); )
                {
                    MediaPacket *onePacket = it1->second;
                    SAFE_DELETE(onePacket);
                    packets.erase(it1++);
                }
                packets.clear();
            }

            m_selectedTiles.erase(it++);
        }

        m_selectedTiles.clear();
    }

    if (m_initTilesMergeArr.size())
    {
        std::map<uint32_t, TilesMergeArrangement*>::iterator it;
        for (it = m_initTilesMergeArr.begin(); it != m_initTilesMergeArr.end(); )
        {
            TilesMergeArrangement *layOut = it->second;
            if (layOut)
            {
                if ((layOut->tilesLayout).tileRowHeight)
                {
                    delete [] ((layOut->tilesLayout).tileRowHeight);
                    (layOut->tilesLayout).tileRowHeight = NULL;
                }

                if ((layOut->tilesLayout).tileColWidth)
                {
                    delete [] ((layOut->tilesLayout).tileColWidth);
                    (layOut->tilesLayout).tileColWidth = NULL;
                }

                SAFE_DELETE(layOut);
            }

            m_initTilesMergeArr.erase(it++);
        }

        m_initTilesMergeArr.clear();
    }

    if (m_updatedTilesMergeArr.size())
    {
        std::map<uint32_t, TilesMergeArrangement*>::iterator it;
        for (it = m_updatedTilesMergeArr.begin(); it != m_updatedTilesMergeArr.end(); )
        {
            TilesMergeArrangement *layOut = it->second;
            if (layOut)
            {
                if ((layOut->tilesLayout).tileRowHeight)
                {
                    delete [] ((layOut->tilesLayout).tileRowHeight);
                    (layOut->tilesLayout).tileRowHeight = NULL;
                }

                if ((layOut->tilesLayout).tileColWidth)
                {
                    delete [] ((layOut->tilesLayout).tileColWidth);
                    (layOut->tilesLayout).tileColWidth = NULL;
                }

                SAFE_DELETE(layOut);
            }

            m_updatedTilesMergeArr.erase(it++);
        }

        m_updatedTilesMergeArr.clear();
    }

    SAFE_DELETE(m_360scvpParam);

    if (m_360scvpHandle)
    {
        I360SCVP_unInit(m_360scvpHandle);
        m_360scvpHandle = NULL;
    }

    m_allQualities.clear();
    if (m_fullResVideoHeader)
    {
        delete [] m_fullResVideoHeader;
        m_fullResVideoHeader = NULL;
    }

    if (m_mergedVideoHeaders.size())
    {
        std::map<uint32_t, std::map<uint32_t, uint8_t*>>::iterator itHrd;
        for (itHrd = m_mergedVideoHeaders.begin(); itHrd != m_mergedVideoHeaders.end(); )
        {
            std::map<uint32_t, uint8_t*> oneHeader = itHrd->second;
            std::map<uint32_t, uint8_t*>::iterator itParam;
            for (itParam = oneHeader.begin(); itParam != oneHeader.end(); )
            {
                uint8_t *headers = itParam->second;
                if (headers)
                {
                    delete [] headers;
                    headers = NULL;
                }
                oneHeader.erase(itParam++);
            }
            oneHeader.clear();
            m_mergedVideoHeaders.erase(itHrd++);
        }
        m_mergedVideoHeaders.clear();
    }
}

int32_t OmafTilesStitch::Initialize(
    std::map<uint32_t, MediaPacket*>& firstFramePackets,
    bool needParams)
{
    if (0 == firstFramePackets.size())
    {
        LOG(ERROR) << "There is no media packet for tiles stitch !" << std::endl;
        return OMAF_ERROR_INVALID_DATA;
    }

    if (m_selectedTiles.size())
    {
        LOG(ERROR) << "Non-empty selected tile track media packets at the beginning !" << std::endl;
        return OMAF_ERROR_INVALID_DATA;
    }

    m_needHeaders = needParams;

    std::map<uint32_t, MediaPacket*>::iterator it;
    for ( it = firstFramePackets.begin(); it != firstFramePackets.end(); it++)
    {
        MediaPacket *onePacket = it->second;
        uint32_t qualityRanking = onePacket->GetQualityRanking();
        m_allQualities.insert(qualityRanking);
    }

    std::set<uint32_t>::iterator itQuality = m_allQualities.begin();
    uint32_t firstQuality = *itQuality;
    if (firstQuality != HIGHEST_QUALITY_RANKING)
    {
         LOG(ERROR) << "Invalid quality ranking value  " << firstQuality << "  for the highest quality !" << std::endl;
         return OMAF_ERROR_INVALID_DATA;
    }

    for ( ; itQuality != m_allQualities.end(); itQuality++)
    {
        uint32_t oneQuality = *itQuality;
        std::map<uint32_t, MediaPacket*> packets;
        std::map<uint32_t, MediaPacket*> middlePackets;
        std::set<uint32_t> tracksID;
        for ( it = firstFramePackets.begin(); it != firstFramePackets.end(); it++)
        {
            MediaPacket *onePacket = it->second;
            uint32_t qualityRanking = onePacket->GetQualityRanking();
            if (qualityRanking == oneQuality)
            {
                tracksID.insert(it->first);
                middlePackets.insert(std::make_pair(it->first, onePacket));
            }
        }

        std::set<uint32_t>::iterator itId = tracksID.begin();
        for ( ; itId != tracksID.end(); itId++)
        {
            uint32_t oneID = *itId;
            MediaPacket *onePacket = middlePackets[oneID];
            packets.insert(std::make_pair(oneID, onePacket));
        }

        middlePackets.clear();
        tracksID.clear();
        LOG(INFO) << "For quality ranking  " << oneQuality << " , there are total  " << packets.size() << "  tiles needed to be merged !" << std::endl;

        m_selectedTiles.insert(std::make_pair(oneQuality, packets));
    }

    if (m_allQualities.size() != m_selectedTiles.size())
    {
        LOG(ERROR) << "Failed to differentiate media packets from different quality ranking !" << std::endl;
        return OMAF_ERROR_INVALID_DATA;
    }

    std::map<uint32_t, MediaPacket*> highestQualityPackets;
    highestQualityPackets =  m_selectedTiles[firstQuality];
    it = highestQualityPackets.begin();
    MediaPacket *tilePacket = it->second;
    if (!tilePacket)
    {
        LOG(ERROR) << "NULL Media Packet !" << std::endl;
        return OMAF_ERROR_NULL_PTR;
    }

    int32_t ret = ParseVideoHeader(tilePacket);

    if (ERROR_NONE == ret)
        m_isInitialized = true;

    return ret;
}

int32_t OmafTilesStitch::ParseVideoHeader(MediaPacket *tilePacket)
{
    if (!tilePacket)
        return OMAF_ERROR_NULL_PTR;

    if (m_360scvpParam || m_360scvpHandle)
    {
        LOG(ERROR) << "There should be no 360SCVP library usage before parsing video headers ! " << std::endl;
        return OMAF_ERROR_INVALID_DATA;
    }
    bool hasHeader = tilePacket->GetHasVideoHeader();
    if (!hasHeader)
    {
        LOG(ERROR) << "The first media packet should have video headers !" << std::endl;
        return OMAF_ERROR_INVALID_DATA;
    }

    m_fullResVPSSize = tilePacket->GetVPSLen();
    m_fullResSPSSize = tilePacket->GetSPSLen();
    m_fullResPPSSize = tilePacket->GetPPSLen();
    uint32_t fullResHeaderSize  = m_fullResVPSSize + m_fullResSPSSize + m_fullResPPSSize;

    m_fullResVideoHeader = new uint8_t[fullResHeaderSize];
    memset(m_fullResVideoHeader, 0, fullResHeaderSize);
    memcpy(m_fullResVideoHeader, (uint8_t*)(tilePacket->Payload()), fullResHeaderSize);
    m_360scvpParam = new param_360SCVP;
    if (!m_360scvpParam)
        return OMAF_ERROR_NULL_PTR;

    memset(m_360scvpParam, 0, sizeof(param_360SCVP));
    m_360scvpParam->usedType = E_PARSER_ONENAL;
    m_360scvpParam->pInputBitstream = m_fullResVideoHeader;
    m_360scvpParam->inputBitstreamLen = fullResHeaderSize;

    m_360scvpHandle = I360SCVP_Init(m_360scvpParam);
    if (!m_360scvpHandle)
    {
        LOG(ERROR) << "Failed to initialize 360SCVP library handle !" << std::endl;
        return OMAF_ERROR_NULL_PTR;
    }

    Nalu *oneNalu = new Nalu;
    if (!oneNalu)
        return OMAF_ERROR_NULL_PTR;

    oneNalu->data = m_fullResVideoHeader;
    oneNalu->dataSize = fullResHeaderSize;
    int32_t ret = I360SCVP_ParseNAL(oneNalu, m_360scvpHandle);
    if (ret)
    {
        SAFE_DELETE(oneNalu);
        return OMAF_ERROR_NALU_NOT_FOUND;
    }

    if (oneNalu->data != m_fullResVideoHeader)
    {
        SAFE_DELETE(oneNalu);
        return OMAF_ERROR_INVALID_HEADER;
    }

    if ((uint32_t)(oneNalu->dataSize) != m_fullResVPSSize)
    {
        SAFE_DELETE(oneNalu);
        return OMAF_ERROR_INVALID_HEADER;
    }

    oneNalu->data += m_fullResVPSSize;
    oneNalu->dataSize = fullResHeaderSize - m_fullResVPSSize;
    ret = I360SCVP_ParseNAL(oneNalu, m_360scvpHandle);
    if (ret)
    {
        SAFE_DELETE(oneNalu);
        return OMAF_ERROR_NALU_NOT_FOUND;
    }

    if (oneNalu->data != (m_fullResVideoHeader + m_fullResVPSSize))
    {
        SAFE_DELETE(oneNalu);
        return OMAF_ERROR_INVALID_HEADER;
    }

    if ((uint32_t)(oneNalu->dataSize) != m_fullResSPSSize)
    {
        SAFE_DELETE(oneNalu);
        return OMAF_ERROR_INVALID_HEADER;
    }

    oneNalu->data += m_fullResSPSSize;
    oneNalu->dataSize = fullResHeaderSize - m_fullResVPSSize - m_fullResSPSSize;
    ret = I360SCVP_ParseNAL(oneNalu, m_360scvpHandle);
    if (ret)
    {
        SAFE_DELETE(oneNalu);
        return OMAF_ERROR_NALU_NOT_FOUND;
    }

    if (oneNalu->data != (m_fullResVideoHeader + m_fullResVPSSize + m_fullResSPSSize))
    {
        SAFE_DELETE(oneNalu);
        return OMAF_ERROR_INVALID_HEADER;
    }

    if ((uint32_t)(oneNalu->dataSize) != m_fullResPPSSize)
    {
        SAFE_DELETE(oneNalu);
        return OMAF_ERROR_INVALID_HEADER;
    }

    SAFE_DELETE(oneNalu);

    Param_PicInfo *picInfo = new Param_PicInfo;
    if (!picInfo)
        return OMAF_ERROR_NULL_PTR;

    I360SCVP_GetParameter(m_360scvpHandle, ID_SCVP_PARAM_PICINFO, (void**)(&picInfo));

    m_fullWidth = picInfo->picWidth;
    m_fullHeight = picInfo->picHeight;
    LOG(INFO) << "Full resolution video has width  " << m_fullWidth << "  and height  " << m_fullHeight << " !" << std::endl;

    SAFE_DELETE(picInfo);

    return ERROR_NONE;
}

int32_t OmafTilesStitch::UpdateSelectedTiles(
    std::map<uint32_t, MediaPacket*>& currPackets,
    bool needParams)
{
    if (0 == currPackets.size())
        return OMAF_ERROR_INVALID_DATA;

    if (0 == m_allQualities.size())
    {
        LOG(ERROR) << "Tile track groups are invalid !" << std::endl;
        return OMAF_ERROR_INVALID_DATA;
    }

    if (m_selectedTiles.size())
    {
        std::map<uint32_t, std::map<uint32_t, MediaPacket*>>::iterator it;
        for (it = m_selectedTiles.begin(); it != m_selectedTiles.end(); )
        {
            std::map<uint32_t, MediaPacket*> packets = it->second;
            if (packets.size())
            {
                std::map<uint32_t, MediaPacket*>::iterator it1;
                for (it1 = packets.begin(); it1 != packets.end(); )
                {
                    MediaPacket *onePacket = it1->second;
                    SAFE_DELETE(onePacket);
                    packets.erase(it1++);
                }
                packets.clear();
            }

            m_selectedTiles.erase(it++);
        }

        m_selectedTiles.clear();
    }

    m_needHeaders = needParams;

    std::set<uint32_t> allQualities;
    std::map<uint32_t, MediaPacket*>::iterator it;
    for ( it = currPackets.begin(); it != currPackets.end(); it++)
    {
        MediaPacket *onePacket = it->second;
        uint32_t qualityRanking = onePacket->GetQualityRanking();
        allQualities.insert(qualityRanking);
    }

    if (m_allQualities.size() != allQualities.size())
    {
        LOG(INFO) << "Video qualities number has been changed !" << std::endl;
    }

    m_allQualities.clear();
    if (m_allQualities.size())
    {
        LOG(ERROR) << "Failed to clear all quality rankings !" << std::endl;
        return OMAF_ERROR_OPERATION;
    }

    m_allQualities = allQualities;
    allQualities.clear();

    std::set<uint32_t>::iterator itQuality = m_allQualities.begin();
    uint32_t firstQuality = *itQuality;
    if (firstQuality != HIGHEST_QUALITY_RANKING)
    {
         LOG(ERROR) << "Invalid quality ranking value  " << firstQuality << "  for the highest quality !" << std::endl;
         return OMAF_ERROR_INVALID_DATA;
    }

    for ( ; itQuality != m_allQualities.end(); itQuality++)
    {
        uint32_t oneQuality = *itQuality;
        std::map<uint32_t, MediaPacket*> packets;
        std::map<uint32_t, MediaPacket*> middlePackets;
        std::set<uint32_t> tracksID;
        for ( it = currPackets.begin(); it != currPackets.end(); it++)
        {
            MediaPacket *onePacket = it->second;
            uint32_t qualityRanking = onePacket->GetQualityRanking();
            if (qualityRanking == oneQuality)
            {
                tracksID.insert(it->first);
                middlePackets.insert(std::make_pair(it->first, onePacket));
            }
        }

        std::set<uint32_t>::iterator itId = tracksID.begin();
        for ( ; itId != tracksID.end(); itId++)
        {
            uint32_t oneID = *itId;
            MediaPacket *onePacket = middlePackets[oneID];
            packets.insert(std::make_pair(oneID, onePacket));
        }

        middlePackets.clear();
        tracksID.clear();

        LOG(INFO) << "In Update, For quality ranking  " << oneQuality << " , there are total  " << packets.size() << "  tiles needed to be merged !" << std::endl;

        m_selectedTiles.insert(std::make_pair(oneQuality, packets));
    }

    if (m_allQualities.size() != m_selectedTiles.size())
    {
        LOG(ERROR) << "Failed to differentiate media packets from different quality ranking !" << std::endl;
        return OMAF_ERROR_INVALID_DATA;
    }

    return ERROR_NONE;
}

std::map<uint32_t, TilesMergeArrangement*> OmafTilesStitch::CalculateTilesMergeArrangement()
{
    std::map<uint32_t, TilesMergeArrangement*> tilesMergeArr;

    if (0 == m_selectedTiles.size())
        return tilesMergeArr;

    std::map<uint32_t, std::map<uint32_t, MediaPacket*>>::iterator it;
    for (it = m_selectedTiles.begin(); it != m_selectedTiles.end(); it++)
    {
        uint32_t width  = 0;
        uint32_t height = 0;
        int32_t  oneTileWidth = 0;
        int32_t  oneTileHeight = 0;
        uint8_t  tileRowsNum = 0;
        uint8_t  tileColsNum = 0;
        int32_t  mostLeftPos = 0;
        int32_t  mostTopPos  = 0;
        uint32_t qualityRanking = it->first;
        std::map<uint32_t, MediaPacket*> packets = it->second;
        std::map<uint32_t, MediaPacket*>::iterator itPacket;

        itPacket = packets.begin();
        MediaPacket *onePacket = itPacket->second;
        SRDInfo srd = onePacket->GetSRDInfo();
        oneTileWidth = srd.width;
        oneTileHeight = srd.height;
        uint32_t packetsSize = packets.size();
        uint32_t sqrtedSize = (uint32_t)sqrt(packetsSize);
        while(sqrtedSize && packetsSize%sqrtedSize) { sqrtedSize--; }
        uint32_t divdedSize = packetsSize / sqrtedSize;
        LOG(INFO)<<"sqrtedSize  "<<sqrtedSize<<"  and divdedSize  "<<divdedSize<<endl;
        if (divdedSize > sqrtedSize)
        {
            tileColsNum = divdedSize;
            tileRowsNum = sqrtedSize;
        }
        else
        {
            tileColsNum = sqrtedSize;
            tileRowsNum = divdedSize;
        }
        width = oneTileWidth * tileColsNum;
        height = oneTileHeight * tileRowsNum;
        //LOG(INFO)<<"IN arrangement calculation, width  "<<width<<"  and height  "<<height<<"  tileRowsNum"<<tileRowsNum<<"  tileColsNum"<<tileColsNum<<endl;

        TilesMergeArrangement *oneArr = new TilesMergeArrangement;
        if (!oneArr)
            return tilesMergeArr;

        oneArr->mergedWidth  = width;
        oneArr->mergedHeight = height;
        oneArr->mostTopPos   = mostTopPos;
        oneArr->mostLeftPos  = mostLeftPos;
        oneArr->tilesLayout.tileRowsNum = tileRowsNum;
        oneArr->tilesLayout.tileColsNum = tileColsNum;
        oneArr->tilesLayout.tileRowHeight = new uint16_t[oneArr->tilesLayout.tileRowsNum];
        if (!(oneArr->tilesLayout.tileRowHeight))
            return tilesMergeArr;

        oneArr->tilesLayout.tileColWidth = new uint16_t[oneArr->tilesLayout.tileColsNum];
        if (!(oneArr->tilesLayout.tileColWidth))
        {
            delete [] (oneArr->tilesLayout.tileRowHeight);
            oneArr->tilesLayout.tileRowHeight = NULL;
            return tilesMergeArr;
        }

        for (uint8_t idx = 0; idx < oneArr->tilesLayout.tileRowsNum; idx++)
        {
            (oneArr->tilesLayout).tileRowHeight[idx] = oneTileHeight / LCU_SIZE;
        }

        for (uint8_t idx = 0; idx < oneArr->tilesLayout.tileColsNum; idx++)
        {
            (oneArr->tilesLayout).tileColWidth[idx] = oneTileWidth / LCU_SIZE;
        }

        tilesMergeArr.insert(std::make_pair(qualityRanking, oneArr));

        if ((!m_mainMergedWidth || !m_mainMergedHeight ||
            !m_mainMergedTileRows || !m_mainMergedTileCols) &&
            (qualityRanking == HIGHEST_QUALITY_RANKING))
        {
            m_mainMergedWidth = width;
            m_mainMergedHeight = height;
            m_mainMergedTileRows = tileRowsNum;
            m_mainMergedTileCols = tileColsNum;
            LOG(INFO) << "For Highest quality video, initial merged width is  " << m_mainMergedWidth << "  and height is  " << m_mainMergedHeight << " !" << std::endl;
            LOG(INFO) << "For Highest quality video, initial merged tile rows is  " << m_mainMergedTileRows << "  and tile cols is  " << m_mainMergedTileCols << " !" << std::endl;
        }
    }

    return tilesMergeArr;
}

RegionWisePacking* OmafTilesStitch::CalculateMergedRwpk(
    uint32_t qualityRanking,
    bool     hasPacketLost,
    bool     hasLayoutChanged)
{
    if (0 == m_selectedTiles.size())
        return NULL;

    if (hasPacketLost && hasLayoutChanged)
    {
        LOG(ERROR) << "Packet lost and layout change can't happen at the same time !" << std::endl;
        return NULL;
    }

    std::map<uint32_t, std::map<uint32_t, MediaPacket*>>::iterator it;
    it = m_selectedTiles.find(qualityRanking);
    if (it == m_selectedTiles.end())
    {
        LOG(ERROR) << "Can't find media packets of specified quality ranking !"<< std::endl;
        return NULL;
    }

    std::map<uint32_t, MediaPacket*> packets = it->second;
    if (0 == packets.size())
    {
        LOG(ERROR) << "Invalid media packets size for specified quality ranking !" << std::endl;
        return NULL;
    }

    TilesMergeArrangement *mergeLayout = NULL;
    std::map<uint32_t, TilesMergeArrangement*>::iterator itArr;
    std::map<uint32_t, MediaPacket*>::iterator itPacket;

    if (0 == m_initTilesMergeArr.size() && 0 == m_updatedTilesMergeArr.size())
    {
        LOG(ERROR) << "There is no tiles merge layout before calculating rwpk !" << std::endl;
        return NULL;
    }
    LOG(INFO)<<"hasPacketLost:"<<hasPacketLost<<" hasLayoutChanged:"<<hasLayoutChanged<<endl;
    if (!hasPacketLost && !hasLayoutChanged)
    {
        if (0 == m_updatedTilesMergeArr.size())
        {
            itArr = m_initTilesMergeArr.find(qualityRanking);
            if (itArr == m_initTilesMergeArr.end())
            {
                LOG(ERROR) << "Can't find tiles merge layout for specified quality ranking !" << std::endl;
                return NULL;
            }
            mergeLayout = itArr->second;
            if (!mergeLayout)
            {
                LOG(ERROR) << "NULL tiles merge layout for specified quality ranking !" << std::endl;
                return NULL;
            }
        }
        else
        {
            itArr = m_updatedTilesMergeArr.find(qualityRanking);
            if (itArr == m_updatedTilesMergeArr.end())
            {
                LOG(ERROR) << "Can't find tiles merge layout for specified quality ranking !" << std::endl;
                return NULL;
            }
            mergeLayout = itArr->second;
            if (!mergeLayout)
            {
                LOG(ERROR) << "NULL tiles merge layout for specified quality ranking !" << std::endl;
                return NULL;
            }
        }
    }
    else if (!hasPacketLost && hasLayoutChanged)
    {
        itArr = m_updatedTilesMergeArr.find(qualityRanking);
        if (itArr == m_updatedTilesMergeArr.end())
        {
            LOG(ERROR) << "Can't find tiles merge layout for specified quality ranking !" << std::endl;
            return NULL;
        }

        mergeLayout = itArr->second;
        if (!mergeLayout)
        {
            LOG(ERROR) << "NULL tiles merge layout for specified quality ranking !" << std::endl;
            return NULL;
        }
    }

    if (!mergeLayout)
    {
        LOG(ERROR) << "NULL tiles merge layout for specified quality ranking !" << std::endl;
        return NULL;
    }

    uint32_t width  = mergeLayout->mergedWidth;
    uint32_t height = mergeLayout->mergedHeight;
    uint8_t tileRowsNum = mergeLayout->tilesLayout.tileRowsNum;
    uint8_t tileColsNum = mergeLayout->tilesLayout.tileColsNum;
    // construct region-wise packing information
    RegionWisePacking *rwpk = new RegionWisePacking;
    if (!rwpk)
        return NULL;

    rwpk->constituentPicMatching = 0;
    rwpk->numRegions = (uint8_t)(tileRowsNum) * (uint8_t)(tileColsNum);
    rwpk->projPicWidth = m_fullWidth;
    rwpk->projPicHeight = m_fullHeight;
    rwpk->packedPicWidth = width;
    rwpk->packedPicHeight = height;

    rwpk->rectRegionPacking = new RectangularRegionWisePacking[rwpk->numRegions];
    if (!(rwpk->rectRegionPacking))
    {
        SAFE_DELETE(rwpk);
        return NULL;
    }

    uint8_t regIdx = 0;
    for (itPacket = packets.begin(); itPacket != packets.end(); itPacket++)
    {
        MediaPacket *onePacket = itPacket->second;
        RectangularRegionWisePacking *rectReg = &(rwpk->rectRegionPacking[regIdx]);
        memset(rectReg, 0, sizeof(RectangularRegionWisePacking));

        rectReg->transformType = 0;
        rectReg->guardBandFlag = false;

        SRDInfo srd = onePacket->GetSRDInfo();
        if (qualityRanking == HIGHEST_QUALITY_RANKING)
        {
            rectReg->projRegWidth  = srd.width;
            rectReg->projRegHeight = srd.height;
            rectReg->projRegTop    = srd.top;
            rectReg->projRegLeft   = srd.left;

            rectReg->packedRegWidth = srd.width;
            rectReg->packedRegHeight = srd.height;
            uint8_t rowIdx = regIdx / tileColsNum;
            uint8_t colIdx = regIdx % tileColsNum;
            rectReg->packedRegTop = rowIdx * srd.height;
            rectReg->packedRegLeft = colIdx * srd.width;
        }
        else
        {
            rectReg->projRegWidth  = (uint32_t)round((float)(srd.width * m_fullWidth) / (float)(width));
            rectReg->projRegHeight = (uint32_t)round((float)(srd.height * m_fullHeight) / (float)(height));
            rectReg->projRegTop    = (uint32_t)round((float)(srd.top * m_fullHeight) / (float)(height));
            rectReg->projRegLeft   = (uint32_t)round((float)(srd.left * m_fullWidth) / (float)(width));

            rectReg->packedRegWidth  = srd.width;
            rectReg->packedRegHeight = srd.height;
            rectReg->packedRegTop    = srd.top;
            rectReg->packedRegLeft   = srd.left;
        }

        rectReg->leftGbWidth = 0;
        rectReg->rightGbWidth = 0;
        rectReg->topGbHeight  = 0;
        rectReg->bottomGbHeight = 0;
        rectReg->gbNotUsedForPredFlag = true;
        rectReg->gbType0              = 0;
        rectReg->gbType1              = 0;
        rectReg->gbType2              = 0;
        rectReg->gbType3              = 0;

        regIdx++;
    }

    return rwpk;
}

int32_t OmafTilesStitch::GenerateTilesMergeArrangement()
{
    if (0 == m_selectedTiles.size())
        return OMAF_ERROR_INVALID_DATA;

    if (0 == m_initTilesMergeArr.size())
    {
        m_initTilesMergeArr = CalculateTilesMergeArrangement();
        if (0 == m_initTilesMergeArr.size())
        {
            LOG(ERROR) << "Failed to calculate tiles merged arrangement !" << std::endl;
            return OMAF_ERROR_TILES_MERGE_ARRANGEMENT;
        }
    }
    else
    {
        if (m_updatedTilesMergeArr.size())
        {
            if (m_initTilesMergeArr.size() != m_updatedTilesMergeArr.size())
                LOG(INFO) << "The number of tiles merged video streams has been changed compared with the number at the beginning !" << std::endl;

            std::map<uint32_t, TilesMergeArrangement*>::iterator itArr;
            for (itArr = m_updatedTilesMergeArr.begin(); itArr != m_updatedTilesMergeArr.end(); )
            {
                TilesMergeArrangement *layOut = itArr->second;
                if (layOut)
                {
                    if ((layOut->tilesLayout).tileRowHeight)
                    {
                        delete [] ((layOut->tilesLayout).tileRowHeight);
                        (layOut->tilesLayout).tileRowHeight = NULL;
                    }

                    if ((layOut->tilesLayout).tileColWidth)
                    {
                        delete [] ((layOut->tilesLayout).tileColWidth);
                        (layOut->tilesLayout).tileColWidth = NULL;
                    }

                    SAFE_DELETE(layOut);
                }

                m_updatedTilesMergeArr.erase(itArr++);
            }

            m_updatedTilesMergeArr.clear();
        }

        m_updatedTilesMergeArr = CalculateTilesMergeArrangement();
        if (0 == m_updatedTilesMergeArr.size())
        {
            LOG(ERROR) << "Failed to calculate tiles merged arrangement !" << std::endl;
            return OMAF_ERROR_TILES_MERGE_ARRANGEMENT;
        }
    }

    return ERROR_NONE;
}

int32_t OmafTilesStitch::GenerateOutputMergedPackets()
{
    if (m_outMergedStream.size())
    {
        m_outMergedStream.clear();
    }

    int32_t ret = GenerateTilesMergeArrangement();//GenerateTilesMergeArrAndRwpk();
    if (ret)
        return ret;

    if (0 == m_mergedVideoHeaders.size())
    {
        if (m_updatedTilesMergeArr.size())
        {
            LOG(ERROR) << "Incorrect operation in initialization stage !" << std::endl;
            return OMAF_ERROR_OPERATION;
        }
    }

    std::map<uint32_t, TilesMergeArrangement*> tilesMergeArr;
    if (0 == m_updatedTilesMergeArr.size())
    {
        tilesMergeArr = m_initTilesMergeArr;
    }
    else
    {
        tilesMergeArr = m_updatedTilesMergeArr;
    }

    bool isArrChanged = false;

    std::map<uint32_t, TilesMergeArrangement*>::iterator it;
    for (it = tilesMergeArr.begin(); it != tilesMergeArr.end(); it++)
    {
        uint32_t qualityRanking = it->first;
        TilesMergeArrangement *layOut = it->second;
        if (!layOut)
            return OMAF_ERROR_NULL_PTR;

        uint32_t width = layOut->mergedWidth;
        uint32_t height = layOut->mergedHeight;
        uint8_t  tileRowsNum = layOut->tilesLayout.tileRowsNum;
        uint8_t  tileColsNum = layOut->tilesLayout.tileColsNum;
        //LOG(INFO)<<"merged width  "<<width<<" and height  "<<height<< "  tileColsNum  "<<((uint32_t)tileColsNum)<<"  tileRowsNum" << ((uint32_t)tileRowsNum)<<endl;

        TilesMergeArrangement *initLayOut = m_initTilesMergeArr[qualityRanking];
        if (!initLayOut)
            return OMAF_ERROR_NULL_PTR;

        uint32_t initWidth = initLayOut->mergedWidth;
        uint32_t initHeight = initLayOut->mergedHeight;
        uint8_t  initTileRowsNum = initLayOut->tilesLayout.tileRowsNum;
        uint8_t  initTileColsNum = initLayOut->tilesLayout.tileColsNum;

        bool packetLost = false;
        bool arrangeChanged = false;
        if ((width == initWidth) && (height == initHeight)
            && (tileRowsNum == initTileRowsNum)
            && (tileColsNum == initTileColsNum))
        {
            if (m_selectedTiles[qualityRanking].size() < ((uint32_t)(initTileRowsNum) * (uint32_t)(initTileColsNum)))
                packetLost = true;
        }

        if ((width == initWidth) && (height < initHeight))
        {
            LOG(INFO) << "Packet not lost but tiles merge layout has been changed !" << std::endl;
            arrangeChanged = true;
            isArrChanged = true;
        }

        if ((height == initHeight) && (width < initWidth))
        {
            LOG(INFO) << "Packet not lost but tiles merge layout has been changed !" << std::endl;
            arrangeChanged = true;
            isArrChanged = true;
        }

        if ((width < initWidth) && (height < initHeight))
        {
            LOG(INFO) << "Packet not lost but tiles merge layout has been changed !" << std::endl;
            arrangeChanged = true;
            isArrChanged = true;
        }

        if ((width > initWidth) || (height > initHeight))
        {
            LOG(INFO) << "Packet not lost but tiles merge layout has been changed !" << std::endl;
            arrangeChanged = true;
            isArrChanged = true;
        }
        LOG(INFO)<<"arrangeChanged  "<<arrangeChanged<<"isArrChanged "<<isArrChanged<<std::endl;
        if (arrangeChanged && m_mergedVideoHeaders[qualityRanking].size())
        {
            std::map<uint32_t, uint8_t*> oneVideoHeader = m_mergedVideoHeaders[qualityRanking];
            std::map<uint32_t, uint8_t*>::iterator itHdr = oneVideoHeader.begin();
            uint8_t *headers = itHdr->second;
            if (headers)
            {
                delete [] headers;
                headers = NULL;
            }

            oneVideoHeader.clear();
            m_mergedVideoHeaders.erase(qualityRanking);
        }

        if (m_mergedVideoHeaders.size() < m_initTilesMergeArr.size())
        {
            if (qualityRanking == HIGHEST_QUALITY_RANKING)
            {
                if (!m_fullResVideoHeader)
                {
                    LOG(ERROR) << "NULL original video headers data !" << std::endl;
                    return OMAF_ERROR_NULL_PTR;
                }

                uint32_t headersSize = 0;
                uint8_t *headers = new uint8_t[1024];
                if (!headers)
                    return OMAF_ERROR_NULL_PTR;

                memset(headers, 0, 1024);
                memcpy(headers, m_fullResVideoHeader, m_fullResVPSSize);
                headersSize += m_fullResVPSSize;

                uint8_t *tmp = headers + m_fullResVPSSize;
                m_360scvpParam->pInputBitstream = m_fullResVideoHeader + m_fullResVPSSize;
                m_360scvpParam->inputBitstreamLen = m_fullResSPSSize;
                m_360scvpParam->destWidth = (arrangeChanged ? width : initWidth);
                m_360scvpParam->destHeight = (arrangeChanged ? height : initHeight);
                m_360scvpParam->pOutputBitstream = tmp;
                ret = I360SCVP_GenerateSPS(m_360scvpParam, m_360scvpHandle);
                if (ret)
                {
                    delete [] headers;
                    headers = NULL;
                    return OMAF_ERROR_SCVP_OPERATION_FAILED;
                }

                headersSize += m_360scvpParam->outputBitstreamLen;
                tmp += m_360scvpParam->outputBitstreamLen;
                m_360scvpParam->pInputBitstream = m_fullResVideoHeader + m_fullResVPSSize + m_fullResSPSSize;
                m_360scvpParam->inputBitstreamLen = m_fullResPPSSize;
                m_360scvpParam->pOutputBitstream = tmp;
                if (arrangeChanged)
                {
                    ret = I360SCVP_GeneratePPS(m_360scvpParam, &(layOut->tilesLayout), m_360scvpHandle);
                }
                else
                {
                    ret = I360SCVP_GeneratePPS(m_360scvpParam, &(initLayOut->tilesLayout), m_360scvpHandle);
                }

                if (ret)
                {
                    delete [] headers;
                    headers = NULL;
                    return OMAF_ERROR_SCVP_OPERATION_FAILED;
                }

                headersSize += m_360scvpParam->outputBitstreamLen;

                std::map<uint32_t, uint8_t*> oneVideoHeader;
                oneVideoHeader.insert(std::make_pair(headersSize, headers));
                m_mergedVideoHeaders.insert(std::make_pair(qualityRanking, oneVideoHeader));
            }
        }

        RegionWisePacking *rwpk = CalculateMergedRwpk(qualityRanking, packetLost, arrangeChanged);
        if (!rwpk)
            return OMAF_ERROR_GENERATE_RWPK;

        std::map<uint32_t, MediaPacket*> packets = m_selectedTiles[qualityRanking];
        std::map<uint32_t, MediaPacket*>::iterator itPacket;

        MediaPacket *mergedPacket = new MediaPacket();
        uint32_t packetSize = ((width * height * 3) / 2) / 2;
        mergedPacket->ReAllocatePacket(packetSize);
        mergedPacket->SetRwpk(rwpk);
        char *mergedData = mergedPacket->Payload();
        uint64_t realSize = 0;
        if (m_needHeaders)
        {
            std::map<uint32_t, uint8_t*> oneVideoHeader = m_mergedVideoHeaders[qualityRanking];
            if (qualityRanking != HIGHEST_QUALITY_RANKING)
            {
                if (0 == oneVideoHeader.size())
                {
                    itPacket = packets.begin();
                    MediaPacket *onePacket = itPacket->second;
                    if (!(onePacket->GetHasVideoHeader()))
                    {
                        LOG(ERROR) << "There should be video headers here !" << std::endl;
                        SAFE_DELETE(mergedPacket);
                        return OMAF_ERROR_INVALID_DATA;
                    }
                    uint32_t hrdSize = onePacket->GetVideoHeaderSize();
                    uint8_t *headersData = new uint8_t[hrdSize];
                    if (!headersData)
                    {
                        SAFE_DELETE(mergedPacket);
                        return OMAF_ERROR_NULL_PTR;
                    }
                    memcpy(headersData, onePacket->Payload(), hrdSize);
                    oneVideoHeader.insert(std::make_pair(hrdSize, headersData));
                    m_mergedVideoHeaders.insert(std::make_pair(qualityRanking, oneVideoHeader));
                }
            }
            std::map<uint32_t, uint8_t*>::iterator itHdr = oneVideoHeader.begin();
            uint8_t *headers = itHdr->second;
            uint32_t headersLen = itHdr->first;
            if (!headers)
            {
                delete mergedPacket;
                mergedPacket = NULL;
                return OMAF_ERROR_NULL_PTR;
            }
            memcpy(mergedData, headers, headersLen);
            realSize += headersLen;
        }

        TilesMergeArrangement *arrange = NULL;
        if (m_updatedTilesMergeArr.empty())
            arrange = m_initTilesMergeArr[qualityRanking];
        else
            arrange = m_updatedTilesMergeArr[qualityRanking];

        if (!arrange)
        {
            SAFE_DELETE(mergedPacket);
            return OMAF_ERROR_NULL_PTR;
        }

        int32_t tileWidth = 0;
        int32_t tileHeight = 0;
        tileColsNum = arrange->tilesLayout.tileColsNum;
        //LOG(INFO)<<"mostTopPos  " << mostTopPos << " and mostLeftPos  "<<mostLeftPos<<endl;

        itPacket = packets.begin();
        MediaPacket *firstPacket = itPacket->second;
        mergedPacket->SetVideoID((qualityRanking - 1));
        mergedPacket->SetCodecType(firstPacket->GetCodecType());
        mergedPacket->SetPTS(firstPacket->GetPTS());
        mergedPacket->SetSegID(firstPacket->GetSegID());
        mergedPacket->SetQualityRanking(qualityRanking);
        mergedPacket->SetVideoWidth((arrangeChanged ? width : initWidth));
        mergedPacket->SetVideoHeight((arrangeChanged ? height : initHeight));
        mergedPacket->SetQualityNum(1);
        SourceResolution resolution;
        resolution.qualityRanking = qualityRanking;
        resolution.top = 0;
        resolution.left = 0;
        resolution.width = (arrangeChanged ? width : initWidth);
        resolution.height = (arrangeChanged ? height : initHeight);
        mergedPacket->SetSourceResolution(0, resolution);
        mergedPacket->SetEOS(firstPacket->GetEOS());

        uint32_t tilesIdx = 0;
        for (itPacket = packets.begin(); itPacket != packets.end(); itPacket++)
        {
            MediaPacket *onePacket = itPacket->second;
            if (qualityRanking == HIGHEST_QUALITY_RANKING)
            {
                SRDInfo srd = onePacket->GetSRDInfo();
                if (!tileWidth || !tileHeight)
                {
                    tileWidth = srd.width;
                    tileHeight = srd.height;
                }

                uint8_t colIdx = tilesIdx % tileColsNum;
                uint8_t rowIdx = tilesIdx / tileColsNum;
                uint16_t ctuIdx = rowIdx * (tileHeight / LCU_SIZE) * ((tileWidth / LCU_SIZE) * tileColsNum) + colIdx * (tileWidth / LCU_SIZE);
                //LOG(INFO)<< "SRD: " << srd.top << " and  "<<srd.left<<endl;
                //LOG(INFO)<<"New ctuIdx  "<<ctuIdx<<endl;
                char *data = onePacket->Payload();
                int32_t dataSize = onePacket->Size();
                if (onePacket->GetHasVideoHeader())
                {
                    data += (onePacket->GetVPSLen() + onePacket->GetSPSLen() + onePacket->GetPPSLen());
                    dataSize -= (onePacket->GetVPSLen() + onePacket->GetSPSLen() + onePacket->GetPPSLen());
                }

                Nalu *nalu = new Nalu;
                if (!nalu)
                {
                    SAFE_DELETE(mergedPacket);
                    return OMAF_ERROR_NULL_PTR;
                }

                nalu->data = (uint8_t*)data;
                nalu->dataSize = dataSize;
                I360SCVP_ParseNAL(nalu, m_360scvpHandle);

                nalu->sliceHeaderLen = nalu->sliceHeaderLen - HEVC_NALUHEADER_LEN;

                m_360scvpParam->destWidth = (arrangeChanged ? width : initWidth);
                m_360scvpParam->destHeight = (arrangeChanged ? height : initHeight);
                m_360scvpParam->pInputBitstream = (uint8_t*)data;
                m_360scvpParam->inputBitstreamLen = dataSize;
                m_360scvpParam->pOutputBitstream = (uint8_t*)mergedData + realSize;
                I360SCVP_GenerateSliceHdr(m_360scvpParam, ctuIdx, m_360scvpHandle);
                realSize += m_360scvpParam->outputBitstreamLen;
                memcpy(mergedData + realSize,
                (nalu->data + HEVC_STARTCODES_LEN + HEVC_NALUHEADER_LEN + nalu->sliceHeaderLen),
                (nalu->dataSize - (HEVC_STARTCODES_LEN + HEVC_NALUHEADER_LEN + nalu->sliceHeaderLen)));

                realSize += nalu->dataSize - (HEVC_STARTCODES_LEN + HEVC_NALUHEADER_LEN + nalu->sliceHeaderLen);
                SAFE_DELETE(nalu);
                tilesIdx++;
            }
            else
            {
                char *data = onePacket->Payload();
                int32_t dataSize = onePacket->Size();
                if (onePacket->GetHasVideoHeader())
                {
                    data += onePacket->GetVideoHeaderSize();
                    dataSize -= onePacket->GetVideoHeaderSize();
                }
                memcpy(mergedData + realSize, data, dataSize);
                realSize += dataSize;
            }
        }

        mergedPacket->SetRealSize(realSize);

        m_outMergedStream.push_back(mergedPacket);
    }

    if (isArrChanged)
    {
        for (it = m_initTilesMergeArr.begin(); it != m_initTilesMergeArr.end(); )
        {
            TilesMergeArrangement *layOut = it->second;
            if (layOut)
            {
                if ((layOut->tilesLayout).tileRowHeight)
                {
                    delete [] ((layOut->tilesLayout).tileRowHeight);
                    (layOut->tilesLayout).tileRowHeight = NULL;
                }

                if ((layOut->tilesLayout).tileColWidth)
                {
                    delete [] ((layOut->tilesLayout).tileColWidth);
                    (layOut->tilesLayout).tileColWidth = NULL;
                }

                SAFE_DELETE(layOut);
            }

            m_initTilesMergeArr.erase(it++);
        }

        m_initTilesMergeArr.clear();

        for (it = m_updatedTilesMergeArr.begin(); it != m_updatedTilesMergeArr.end(); it++)
        {
            TilesMergeArrangement *existedArr = it->second;
            if (!existedArr)
                return OMAF_ERROR_NULL_PTR;

            TilesMergeArrangement *oneArr = new TilesMergeArrangement;
            if (!oneArr)
                return OMAF_ERROR_NULL_PTR;

            oneArr->mergedWidth = existedArr->mergedWidth;
            oneArr->mergedHeight = existedArr->mergedHeight;
            oneArr->mostTopPos = existedArr->mostTopPos;
            oneArr->mostLeftPos = existedArr->mostLeftPos;
            oneArr->tilesLayout.tileRowsNum = existedArr->tilesLayout.tileRowsNum;
            oneArr->tilesLayout.tileColsNum = existedArr->tilesLayout.tileColsNum;
            oneArr->tilesLayout.tileRowHeight = new uint16_t[oneArr->tilesLayout.tileRowsNum];
            if (!(oneArr->tilesLayout.tileRowHeight))
                return OMAF_ERROR_NULL_PTR;

            oneArr->tilesLayout.tileColWidth = new uint16_t[oneArr->tilesLayout.tileColsNum];
            if (!(oneArr->tilesLayout.tileColWidth))
            {
                delete [] (oneArr->tilesLayout.tileRowHeight);
                oneArr->tilesLayout.tileRowHeight = NULL;
                return OMAF_ERROR_NULL_PTR;
            }

            for (uint8_t idx = 0; idx < oneArr->tilesLayout.tileRowsNum; idx++)
                oneArr->tilesLayout.tileRowHeight[idx] = existedArr->tilesLayout.tileRowHeight[idx];

            for (uint8_t idx = 0; idx < oneArr->tilesLayout.tileColsNum; idx++)
                oneArr->tilesLayout.tileColWidth[idx] = existedArr->tilesLayout.tileColWidth[idx];

            m_initTilesMergeArr.insert(std::make_pair(it->first, oneArr));
        }
    }

    return ERROR_NONE;
}

std::list<MediaPacket*> OmafTilesStitch::GetTilesMergedPackets()
{
    this->GenerateOutputMergedPackets();
    return m_outMergedStream;
}

VCD_OMAF_END
