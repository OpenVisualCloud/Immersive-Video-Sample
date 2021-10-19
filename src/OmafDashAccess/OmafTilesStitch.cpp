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

#include "common.h"
VCD_OMAF_BEGIN

OmafTilesStitch::OmafTilesStitch() {
  m_360scvpParam = nullptr;
  m_360scvpHandle = nullptr;
  m_fullResVideoHeader = nullptr;
  m_fullResVPSSize = 0;
  m_fullResSPSSize = 0;
  m_fullResPPSSize = 0;
  m_fullWidth = 0;
  m_fullHeight = 0;
  m_mainMergedWidth = 0;
  m_mainMergedHeight = 0;
  m_mainMergedTileRows = 0;
  m_mainMergedTileCols = 0;
  m_needHeaders = false;
  m_isInitialized = false;
  m_projFmt = PF_UNKNOWN;
  m_tmpRegionrwpk = nullptr;
  m_maxStitchWidth = 0;
  m_maxStitchHeight = 0;
}

OmafTilesStitch::~OmafTilesStitch() {
  if (m_selectedTiles.size()) {
    std::list<MediaPacket *> allPackets;
    std::map<QualityRank, std::map<uint32_t, MediaPacket *>>::iterator it;
    for (it = m_selectedTiles.begin(); it != m_selectedTiles.end();) {
      std::map<uint32_t, MediaPacket *> packets = it->second;
      if (packets.size()) {
        std::map<uint32_t, MediaPacket *>::iterator it1;
        for (it1 = packets.begin(); it1 != packets.end();) {
          MediaPacket *onePacket = it1->second;
          std::list<MediaPacket *>::iterator pktIter;
          pktIter = std::find(allPackets.begin(), allPackets.end(), onePacket);
          if (pktIter == allPackets.end())
          {
            allPackets.push_back(onePacket);
            SAFE_DELETE(onePacket);
          }
          packets.erase(it1++);
        }
        packets.clear();
      }
      allPackets.clear();
      m_selectedTiles.erase(it++);
    }

    m_selectedTiles.clear();
  }

  if (m_initTilesMergeArr.size()) {
    std::map<QualityRank, vector<TilesMergeArrangement *>>::iterator it;
    for (it = m_initTilesMergeArr.begin(); it != m_initTilesMergeArr.end();) {

        vector<TilesMergeArrangement *> layOut = it->second;

        for (uint32_t i = 0; i < layOut.size(); i++) {
            if (layOut[i]) {
                DELETE_ARRAY(layOut[i]->tilesLayout.tileRowHeight);
                DELETE_ARRAY(layOut[i]->tilesLayout.tileColWidth);
            }
            SAFE_DELETE(layOut[i]);
        }
        layOut.clear();
        m_initTilesMergeArr.erase(it++);
    }
    m_initTilesMergeArr.clear();
  }

  if (m_updatedTilesMergeArr.size()) {
    std::map<QualityRank, vector<TilesMergeArrangement *>>::iterator it;
    for (it = m_updatedTilesMergeArr.begin(); it != m_updatedTilesMergeArr.end();) {

      vector<TilesMergeArrangement *> layOut = it->second;

      for (uint32_t i = 0; i < layOut.size(); i++) {
        if (layOut[i]) {
          DELETE_ARRAY(layOut[i]->tilesLayout.tileRowHeight);
          DELETE_ARRAY(layOut[i]->tilesLayout.tileColWidth);
        }
        SAFE_DELETE(layOut[i]);
      }
      layOut.clear();
      m_updatedTilesMergeArr.erase(it++);
    }

    m_updatedTilesMergeArr.clear();
  }

  SAFE_DELETE(m_360scvpParam);

  if (m_360scvpHandle) {
    I360SCVP_unInit(m_360scvpHandle);
    m_360scvpHandle = nullptr;
  }

  m_allQualities.clear();
  if (m_fullResVideoHeader) {
    delete[] m_fullResVideoHeader;
    m_fullResVideoHeader = nullptr;
  }

  if (m_mergedVideoHeaders.size()) {
    std::map<QualityRank, vector<std::map<uint32_t, uint8_t *>>>::iterator itHrd;
    for (itHrd = m_mergedVideoHeaders.begin(); itHrd != m_mergedVideoHeaders.end();) {
      vector<std::map<uint32_t, uint8_t *>> oneHeader = itHrd->second;
      for (uint32_t i = 0; i < oneHeader.size(); i++) {
        std::map<uint32_t, uint8_t *>::iterator itParam;
        for (itParam = oneHeader[i].begin(); itParam != oneHeader[i].end();) {
          uint8_t *headers = itParam->second;
          DELETE_ARRAY(headers);
          oneHeader[i].erase(itParam++);
        }
        oneHeader[i].clear();
      }
      m_mergedVideoHeaders.erase(itHrd++);
    }
    m_mergedVideoHeaders.clear();
  }
  m_tmpRegionrwpk = nullptr;
  m_sources.clear();
}

int32_t OmafTilesStitch::Initialize(std::map<uint32_t, MediaPacket *> &firstFramePackets, bool needParams,
                                    VCD::OMAF::ProjectionFormat projFmt, std::map<uint32_t, SourceInfo> allSources) {
  if (0 == firstFramePackets.size()) {
    OMAF_LOG(LOG_ERROR, "There is no media packet for tiles stitch !\n");
    return OMAF_ERROR_INVALID_DATA;
  }

  if (m_selectedTiles.size()) {
    OMAF_LOG(LOG_ERROR, "Non-empty selected tile track media packets at the beginning !\n");
    return OMAF_ERROR_INVALID_DATA;
  }

  m_needHeaders = needParams;
  m_projFmt = projFmt;
  m_sources = allSources;

  std::map<uint32_t, MediaPacket *>::iterator it;
  for (it = firstFramePackets.begin(); it != firstFramePackets.end(); it++) {
    MediaPacket *onePacket = it->second;
    auto qualityRanking = onePacket->GetQualityRanking();
    m_allQualities.insert(qualityRanking);
  }

  std::set<QualityRank>::iterator itQuality = m_allQualities.begin();
  if (itQuality == m_allQualities.end())
  {
      OMAF_LOG(LOG_ERROR, " Quality set is empty!\n");
      return OMAF_ERROR_INVALID_DATA;
  }
  auto firstQuality = *itQuality;

  for (; itQuality != m_allQualities.end(); itQuality++) {
    auto oneQuality = *itQuality;
    std::map<uint32_t, MediaPacket *> packets;
    std::map<uint32_t, MediaPacket *> middlePackets;
    std::set<uint32_t> tracksID;
    for (it = firstFramePackets.begin(); it != firstFramePackets.end(); it++) {
      MediaPacket *onePacket = it->second;
      auto qualityRanking = onePacket->GetQualityRanking();
      if (qualityRanking == oneQuality) {
        tracksID.insert(it->first);
        middlePackets.insert(std::make_pair(it->first, onePacket));
      }
    }

    std::set<uint32_t>::iterator itId = tracksID.begin();
    for (; itId != tracksID.end(); itId++) {
      uint32_t oneID = *itId;
      MediaPacket *onePacket = middlePackets[oneID];
      packets.insert(std::make_pair(oneID, onePacket));
    }

    middlePackets.clear();
    tracksID.clear();
    OMAF_LOG(LOG_INFO, "For quality ranking %d, total tiles number needed to be merged is %lld\n", static_cast<int>(oneQuality), packets.size());

    m_selectedTiles.insert(std::make_pair(oneQuality, packets));
  }

  if (m_allQualities.size() != m_selectedTiles.size()) {
    OMAF_LOG(LOG_ERROR, "Failed to differentiate media packets from different quality ranking !\n");
    return OMAF_ERROR_INVALID_DATA;
  }

  std::map<uint32_t, MediaPacket *> highestQualityPackets;
  highestQualityPackets = m_selectedTiles[firstQuality];
  it = highestQualityPackets.begin();
  if (it == highestQualityPackets.end())
  {
      OMAF_LOG(LOG_ERROR, "Highest quality packets map is empty!\n");
      return OMAF_ERROR_INVALID_DATA;
  }
  MediaPacket *tilePacket = it->second;
  if (!tilePacket) {
    OMAF_LOG(LOG_ERROR, "nullptr Media Packet !\n");
    return OMAF_ERROR_NULL_PTR;
  }

  int32_t ret = ParseVideoHeader(tilePacket);

  if (ERROR_NONE == ret) m_isInitialized = true;

  return ret;
}

int32_t OmafTilesStitch::ParseVideoHeader(MediaPacket *tilePacket) {
  if (!tilePacket) return OMAF_ERROR_NULL_PTR;

  if (m_360scvpParam || m_360scvpHandle) {
    OMAF_LOG(LOG_ERROR, "There should be no 360SCVP library usage before parsing video headers !\n");
    return OMAF_ERROR_INVALID_DATA;
  }
  bool hasHeader = tilePacket->GetHasVideoHeader();
  if (!hasHeader) {
    OMAF_LOG(LOG_ERROR, "The first media packet should have video headers !\n");
    return OMAF_ERROR_INVALID_DATA;
  }

  m_fullResVPSSize = tilePacket->GetVPSLen();
  m_fullResSPSSize = tilePacket->GetSPSLen();
  m_fullResPPSSize = tilePacket->GetPPSLen();
  uint32_t fullResHeaderSize = m_fullResVPSSize + m_fullResSPSSize + m_fullResPPSSize;

  m_fullResVideoHeader = new uint8_t[fullResHeaderSize];
  memset(m_fullResVideoHeader, 0, fullResHeaderSize);
  memcpy_s(m_fullResVideoHeader, fullResHeaderSize, (uint8_t *)(tilePacket->Payload()), fullResHeaderSize);
  m_360scvpParam = new param_360SCVP;
  if (!m_360scvpParam) return OMAF_ERROR_NULL_PTR;

  memset(m_360scvpParam, 0, sizeof(param_360SCVP));
  m_360scvpParam->usedType = E_PARSER_ONENAL;
  m_360scvpParam->pInputBitstream = m_fullResVideoHeader;
  m_360scvpParam->inputBitstreamLen = fullResHeaderSize;
  m_360scvpParam->logFunction = (void*)logCallBack;

  m_360scvpHandle = I360SCVP_Init(m_360scvpParam);
  if (!m_360scvpHandle) {
    OMAF_LOG(LOG_ERROR, "Failed to initialize 360SCVP library handle !\n");
    return OMAF_ERROR_NULL_PTR;
  }

  Nalu *oneNalu = new Nalu;
  if (!oneNalu) return OMAF_ERROR_NULL_PTR;

  oneNalu->data = m_fullResVideoHeader;
  oneNalu->dataSize = fullResHeaderSize;
  int32_t ret = I360SCVP_ParseNAL(oneNalu, m_360scvpHandle);
  if (ret) {
    SAFE_DELETE(oneNalu);
    return OMAF_ERROR_NALU_NOT_FOUND;
  }

  if (oneNalu->data != m_fullResVideoHeader) {
    SAFE_DELETE(oneNalu);
    return OMAF_ERROR_INVALID_HEADER;
  }

  if ((uint32_t)(oneNalu->dataSize) != m_fullResVPSSize) {
    SAFE_DELETE(oneNalu);
    return OMAF_ERROR_INVALID_HEADER;
  }

  oneNalu->data += m_fullResVPSSize;
  oneNalu->dataSize = fullResHeaderSize - m_fullResVPSSize;
  ret = I360SCVP_ParseNAL(oneNalu, m_360scvpHandle);
  if (ret) {
    SAFE_DELETE(oneNalu);
    return OMAF_ERROR_NALU_NOT_FOUND;
  }

  if (oneNalu->data != (m_fullResVideoHeader + m_fullResVPSSize)) {
    SAFE_DELETE(oneNalu);
    return OMAF_ERROR_INVALID_HEADER;
  }

  if ((uint32_t)(oneNalu->dataSize) != m_fullResSPSSize) {
    SAFE_DELETE(oneNalu);
    return OMAF_ERROR_INVALID_HEADER;
  }

  oneNalu->data += m_fullResSPSSize;
  oneNalu->dataSize = fullResHeaderSize - m_fullResVPSSize - m_fullResSPSSize;
  ret = I360SCVP_ParseNAL(oneNalu, m_360scvpHandle);
  if (ret) {
    SAFE_DELETE(oneNalu);
    return OMAF_ERROR_NALU_NOT_FOUND;
  }

  if (oneNalu->data != (m_fullResVideoHeader + m_fullResVPSSize + m_fullResSPSSize)) {
    SAFE_DELETE(oneNalu);
    return OMAF_ERROR_INVALID_HEADER;
  }

  if ((uint32_t)(oneNalu->dataSize) != m_fullResPPSSize) {
    SAFE_DELETE(oneNalu);
    return OMAF_ERROR_INVALID_HEADER;
  }

  SAFE_DELETE(oneNalu);

  Param_PicInfo *picInfo = new Param_PicInfo;
  if (!picInfo) return OMAF_ERROR_NULL_PTR;

  I360SCVP_GetParameter(m_360scvpHandle, ID_SCVP_PARAM_PICINFO, (void **)(&picInfo));

  m_fullWidth = picInfo->picWidth;
  m_fullHeight = picInfo->picHeight;
  OMAF_LOG(LOG_INFO, "Full resolution video has width %u and height %u\n", m_fullWidth, m_fullHeight);

  SAFE_DELETE(picInfo);

  return ERROR_NONE;
}

int32_t OmafTilesStitch::UpdateSelectedTiles(std::map<uint32_t, MediaPacket *> &currPackets, bool needParams) {
  if (0 == currPackets.size()) return OMAF_ERROR_INVALID_DATA;

  if (0 == m_allQualities.size()) {
    OMAF_LOG(LOG_ERROR, "Tile track groups are invalid !\n");
    return OMAF_ERROR_INVALID_DATA;
  }

  if (m_selectedTiles.size()) {
    std::list<MediaPacket *> allPackets;
    std::map<QualityRank, std::map<uint32_t, MediaPacket *>>::iterator it;
    for (it = m_selectedTiles.begin(); it != m_selectedTiles.end();) {
      std::map<uint32_t, MediaPacket *> packets = it->second;
      if (packets.size()) {
        std::map<uint32_t, MediaPacket *>::iterator it1;
        for (it1 = packets.begin(); it1 != packets.end();) {
          MediaPacket *onePacket = it1->second;
          std::list<MediaPacket *>::iterator pktIter;
          pktIter = std::find(allPackets.begin(), allPackets.end(), onePacket);
          if (pktIter == allPackets.end())
          {
            allPackets.push_back(onePacket);
            SAFE_DELETE(onePacket);
          }
          packets.erase(it1++);
        }
        packets.clear();
      }
      allPackets.clear();
      m_selectedTiles.erase(it++);
    }

    m_selectedTiles.clear();
  }

  m_needHeaders = needParams;

  std::set<QualityRank> allQualities;
  std::map<uint32_t, MediaPacket *>::iterator it;
  for (it = currPackets.begin(); it != currPackets.end(); it++) {
    MediaPacket *onePacket = it->second;
    auto qualityRanking = onePacket->GetQualityRanking();
    allQualities.insert(qualityRanking);
  }

  if (m_allQualities.size() != allQualities.size()) {
    OMAF_LOG(LOG_INFO, "Video qualities number has been changed !\n");
  }

  m_allQualities.clear();
  if (m_allQualities.size()) {
    OMAF_LOG(LOG_ERROR, "Failed to clear all quality rankings !\n");
    return OMAF_ERROR_OPERATION;
  }

  m_allQualities = allQualities;
  allQualities.clear();

  std::set<QualityRank>::iterator itQuality = m_allQualities.begin();
  if (itQuality == m_allQualities.end())
  {
      OMAF_LOG(LOG_ERROR, "Quality set is empty!\n");
      return OMAF_ERROR_INVALID_DATA;
  }

  OMAF_LOG(LOG_INFO, "Selected stream has quality ranking %d\n", *itQuality);

  for (; itQuality != m_allQualities.end(); itQuality++) {
    auto oneQuality = *itQuality;
    std::map<uint32_t, MediaPacket *> packets;
    std::map<uint32_t, MediaPacket *> middlePackets;
    std::set<uint32_t> tracksID;
    for (it = currPackets.begin(); it != currPackets.end(); it++) {
      MediaPacket *onePacket = it->second;
      auto qualityRanking = onePacket->GetQualityRanking();
      if (qualityRanking == oneQuality) {
        tracksID.insert(it->first);
        middlePackets.insert(std::make_pair(it->first, onePacket));
      }
    }

    std::set<uint32_t>::iterator itId = tracksID.begin();
    for (; itId != tracksID.end(); itId++) {
      uint32_t oneID = *itId;
      MediaPacket *onePacket = middlePackets[oneID];
      packets.insert(std::make_pair(oneID, onePacket));
    }

    middlePackets.clear();
    tracksID.clear();

    OMAF_LOG(LOG_INFO, "In Update, For quality ranking %d, total tiles number needed to be merged is %lld\n", static_cast<int>(oneQuality), packets.size());

    m_selectedTiles.insert(std::make_pair(oneQuality, packets));
  }

  if (m_allQualities.size() != m_selectedTiles.size()) {
    OMAF_LOG(LOG_ERROR, "Failed to differentiate media packets from different quality ranking !\n");
    return OMAF_ERROR_INVALID_DATA;
  }

  return ERROR_NONE;
}

vector<pair<uint32_t, uint32_t>> OmafTilesStitch::GenerateRowAndColArr(uint32_t packetsSize, uint32_t splitNum, uint32_t maxTile_x, uint32_t maxTile_y, QualityRank ranking)
{
    vector<pair<uint32_t, uint32_t>> arrangementArr;
    if (packetsSize < splitNum || splitNum == 0)
    {
        OMAF_LOG(LOG_ERROR, " invalid split num OR packet size!\n");
        return arrangementArr;
    }

    uint32_t normalSplitPacketSize = 0;
    uint32_t lastSplitPacketSize = 0;
    if (splitNum > 1)
    {
        normalSplitPacketSize = maxTile_x * maxTile_y;
        lastSplitPacketSize = normalSplitPacketSize;
        if (packetsSize % splitNum)
        {
            lastSplitPacketSize = packetsSize - normalSplitPacketSize * (splitNum - 1);
        }
    }
    else
    {
        normalSplitPacketSize = packetsSize / splitNum;
        lastSplitPacketSize = normalSplitPacketSize;
    }

    for (uint32_t i = 0; i < splitNum; i++)
    {
        pair<uint32_t, uint32_t> oneArrangement; // pair < row, col >
        uint32_t size = i < splitNum - 1 ? normalSplitPacketSize : lastSplitPacketSize;

        // 1. tranverse all the proximate number and find the required split layout
        uint32_t supplementedNum = 0;

        uint32_t maxDividedSize = 0, maxSqrtedSize = 0;
        // notice: diviedSize >= sqrtedSize
        if (maxTile_x > maxTile_y) { maxDividedSize = maxTile_x; maxSqrtedSize = maxTile_y; }
        else { maxDividedSize = maxTile_y; maxSqrtedSize = maxTile_x; }

        uint32_t sqrtedSize = 0, dividedSize = 0;
        while (size <= maxDividedSize * maxSqrtedSize)
        {
          sqrtedSize = sqrt(size);
          while (sqrtedSize > 0)
          {
            if (size % sqrtedSize == 0)
            {
              dividedSize = size / sqrtedSize;
              // 2. find required split layout
              if (dividedSize <= maxDividedSize && sqrtedSize <= maxSqrtedSize)
              {
                break;
              }
            }
	    sqrtedSize--;
          }
          if (sqrtedSize != 0) break;
          else { size++; supplementedNum++; } // add tile number to remap
        }
        if (size > maxDividedSize * maxSqrtedSize)
        {
          OMAF_LOG(LOG_ERROR, "Sub-picture width/height for split is failed!\n");
          OMAF_LOG(LOG_ERROR, "maxDividedSize %d, maxSqrtedSize %d\n", maxDividedSize, maxSqrtedSize);
          OMAF_LOG(LOG_ERROR, "size %d \n", size);
          return arrangementArr;
        }
        if (supplementedNum > 0)
        {
            OMAF_LOG(LOG_INFO, "Repeat %u tiles to make sure normal packed sub-picture width/height ratio for split %d\n", supplementedNum, i);
        }

        OMAF_LOG(LOG_INFO, "one arrangement has the tile division of %u x %u\n", sqrtedSize, dividedSize);
        if (maxTile_x > maxTile_y || ranking != HIGHEST_QUALITY_RANKING) {
          oneArrangement = std::make_pair(sqrtedSize, dividedSize); //height , width
        } else {
          oneArrangement = std::make_pair(dividedSize, sqrtedSize);
        }
        arrangementArr.push_back(oneArrangement);
    }
    return arrangementArr;
}

std::map<QualityRank, std::vector<TilesMergeArrangement *>> OmafTilesStitch::CalculateTilesMergeArrangement() {
  std::map<QualityRank, std::vector<TilesMergeArrangement *>> tilesMergeArr;

  if (0 == m_selectedTiles.size()) return tilesMergeArr;

  std::map<QualityRank, std::map<uint32_t, MediaPacket *>>::iterator it;
  for (it = m_selectedTiles.begin(); it != m_selectedTiles.end(); it++) {
    int32_t oneTileWidth = 0;
    int32_t oneTileHeight = 0;
    int32_t mostLeftPos = 0;
    int32_t mostTopPos = 0;
    auto qualityRanking = it->first;
    std::map<uint32_t, MediaPacket *> packets = it->second;
    std::map<uint32_t, MediaPacket *>::iterator itPacket;

    itPacket = packets.begin();
    if (itPacket == packets.end())
    {
        OMAF_LOG(LOG_ERROR, "Packet map is empty!\n");
        return tilesMergeArr;
    }
    MediaPacket *onePacket = itPacket->second;
    SRDInfo srd = onePacket->GetSRDInfo();
    oneTileWidth = srd.width;
    oneTileHeight = srd.height;

    // 1 find the max tile split
    if (oneTileWidth == 0 || oneTileHeight == 0) {
      OMAF_LOG(LOG_ERROR, "Tile width or height in srd is zero!\n");
      return tilesMergeArr;
    }
    uint32_t maxTile_x = m_maxStitchWidth / oneTileWidth;
    uint32_t maxTile_y = m_maxStitchHeight / oneTileHeight;

    uint32_t packetsSize = packets.size();

    // 2. check if need to split into multiple videos
    uint32_t splitNum = ceil(float(packetsSize) / (maxTile_x * maxTile_y));

    // 3. generate row and col arrays according to split num and maxTile_x & maxTile_y
    vector<pair<uint32_t, uint32_t>> rowAndColArr = GenerateRowAndColArr(packetsSize, splitNum, maxTile_x, maxTile_y, qualityRanking);
    vector<TilesMergeArrangement*> mergeArrList;
    for (uint32_t i = 0; i < splitNum; i++)
    {
      TilesMergeArrangement *oneArr = new TilesMergeArrangement;
      if (!oneArr) return tilesMergeArr;
      if (rowAndColArr[i].first > maxTile_y || rowAndColArr[i].second > maxTile_x)
      {
        OMAF_LOG(LOG_WARNING, "split limitation broke! tile y %d, tile x %d\n", rowAndColArr[i].first, rowAndColArr[i].second);
      }
      oneArr->mergedWidth = oneTileWidth * rowAndColArr[i].second;
      oneArr->mergedHeight = oneTileHeight * rowAndColArr[i].first;
      oneArr->mostTopPos = mostTopPos;
      oneArr->mostLeftPos = mostLeftPos;
      oneArr->tilesLayout.tileRowsNum = rowAndColArr[i].first;
      oneArr->tilesLayout.tileColsNum = rowAndColArr[i].second;
      oneArr->tilesLayout.tileRowHeight = new uint16_t[oneArr->tilesLayout.tileRowsNum];
      if (!(oneArr->tilesLayout.tileRowHeight))
      {
        SAFE_DELETE(oneArr);
        return tilesMergeArr;
      }
      oneArr->tilesLayout.tileColWidth = new uint16_t[oneArr->tilesLayout.tileColsNum];
      if (!(oneArr->tilesLayout.tileColWidth)) {
        delete[](oneArr->tilesLayout.tileRowHeight);
        oneArr->tilesLayout.tileRowHeight = nullptr;
        SAFE_DELETE(oneArr);
        return tilesMergeArr;
      }

      for (uint8_t idx = 0; idx < oneArr->tilesLayout.tileRowsNum; idx++) {
        (oneArr->tilesLayout).tileRowHeight[idx] = oneTileHeight / LCU_SIZE;
      }

      for (uint8_t idx = 0; idx < oneArr->tilesLayout.tileColsNum; idx++) {
        (oneArr->tilesLayout).tileColWidth[idx] = oneTileWidth / LCU_SIZE;
      }
      mergeArrList.push_back(oneArr);
      OMAF_LOG(LOG_INFO, "FOR quality %d video, the total merged packets size is %u\n", qualityRanking, splitNum);
      if ((!m_mainMergedWidth || !m_mainMergedHeight || !m_mainMergedTileRows || !m_mainMergedTileCols) &&
          (qualityRanking == HIGHEST_QUALITY_RANKING)) {
        m_mainMergedWidth = oneTileWidth * rowAndColArr[i].second;
        m_mainMergedHeight = oneTileHeight * rowAndColArr[i].first;
        m_mainMergedTileRows = rowAndColArr[i].first;
        m_mainMergedTileCols = rowAndColArr[i].second;
        OMAF_LOG(LOG_INFO, "For Highest quality video, initial merged width is %u and height is %u\n", m_mainMergedWidth, m_mainMergedHeight);
        OMAF_LOG(LOG_INFO, "For Highest quality video, initial merged tile rows is %u and tile cols is %u\n", m_mainMergedTileRows, m_mainMergedTileCols);
      }
    }
    tilesMergeArr.insert(std::make_pair(qualityRanking, mergeArrList));
  }

  return tilesMergeArr;
}

vector<std::unique_ptr<RegionWisePacking>> OmafTilesStitch::CalculateMergedRwpkForERP(QualityRank qualityRanking,
                                                                              bool hasPacketLost,
                                                                              bool hasLayoutChanged) {
  vector<std::unique_ptr<RegionWisePacking>> ret;
  if (0 == m_selectedTiles.size()) return ret;

  if (hasPacketLost && hasLayoutChanged) {
    OMAF_LOG(LOG_ERROR, "Packet lost and layout change can't happen at the same time !\n");
    return ret;
  }

  std::map<QualityRank, std::map<uint32_t, MediaPacket *>>::iterator it;
  it = m_selectedTiles.find(qualityRanking);
  if (it == m_selectedTiles.end()) {
    OMAF_LOG(LOG_ERROR, "Can't find media packets of specified quality ranking !\n");
    return ret;
  }

  std::map<uint32_t, MediaPacket *> packets = it->second;
  if (0 == packets.size()) {
    OMAF_LOG(LOG_ERROR, "Invalid media packets size for specified quality ranking !\n");
    return ret;
  }

  vector<TilesMergeArrangement *> mergeLayout;
  std::map<QualityRank, vector<TilesMergeArrangement *>>::iterator itArr;
  std::map<uint32_t, MediaPacket *>::iterator itPacket;

  if (0 == m_initTilesMergeArr.size() && 0 == m_updatedTilesMergeArr.size()) {
    OMAF_LOG(LOG_ERROR, "There is no tiles merge layout before calculating rwpk !\n");
    return ret;
  }
  OMAF_LOG(LOG_INFO, "hasPacketLost: %d, hasLayoutChanged: %d\n", hasPacketLost, hasLayoutChanged);
  if (!hasPacketLost && !hasLayoutChanged) {
    if (0 == m_updatedTilesMergeArr.size()) {
      itArr = m_initTilesMergeArr.find(qualityRanking);
      if (itArr == m_initTilesMergeArr.end()) {
        OMAF_LOG(LOG_ERROR, "Can't find tiles merge layout for specified quality ranking !\n");
        return ret;
      }
      mergeLayout = itArr->second;
      if (mergeLayout.empty()) {
        OMAF_LOG(LOG_ERROR, "nullptr tiles merge layout for specified quality ranking !\n");
        return ret;
      }
    } else {
      itArr = m_updatedTilesMergeArr.find(qualityRanking);
      if (itArr == m_updatedTilesMergeArr.end()) {
        OMAF_LOG(LOG_ERROR, "Can't find tiles merge layout for specified quality ranking !\n");
        return ret;
      }
      mergeLayout = itArr->second;
      if (mergeLayout.empty()) {
        OMAF_LOG(LOG_ERROR, "nullptr tiles merge layout for specified quality ranking !\n");
        return ret;
      }
    }
  } else if (!hasPacketLost && hasLayoutChanged) {
    itArr = m_updatedTilesMergeArr.find(qualityRanking);
    if (itArr == m_updatedTilesMergeArr.end()) {
      OMAF_LOG(LOG_ERROR, "Can't find tiles merge layout for specified quality ranking !\n");
      return ret;
    }

    mergeLayout = itArr->second;
    if (mergeLayout.empty()) {
      OMAF_LOG(LOG_ERROR, "nullptr tiles merge layout for specified quality ranking !\n");
      return ret;
    }
  }

  if (mergeLayout.empty()) {
    OMAF_LOG(LOG_ERROR, "nullptr tiles merge layout for specified quality ranking !\n");
    return ret;
  }

  uint32_t arrangeNum = mergeLayout.size();
  vector<std::unique_ptr<RegionWisePacking>> rwpk; // = make_unique_vcd<RegionWisePacking>();
  for (uint32_t i = 0; i < arrangeNum; i++) {
    std::unique_ptr<RegionWisePacking> onerwpk = make_unique_vcd<RegionWisePacking>();
    if (!onerwpk) return ret;
    rwpk.push_back(std::move(onerwpk));
  }
  for (uint32_t i = 0; i < mergeLayout.size(); i++)
  {
    uint32_t width = mergeLayout[i]->mergedWidth;
    uint32_t height = mergeLayout[i]->mergedHeight;
    uint8_t tileRowsNum = mergeLayout[i]->tilesLayout.tileRowsNum;
    uint8_t tileColsNum = mergeLayout[i]->tilesLayout.tileColsNum;

    rwpk[i]->constituentPicMatching = 0;
    rwpk[i]->numRegions = (uint8_t)(tileRowsNum) * (uint8_t)(tileColsNum);
    rwpk[i]->projPicWidth = m_fullWidth;
    rwpk[i]->projPicHeight = m_fullHeight;
    rwpk[i]->packedPicWidth = width;
    rwpk[i]->packedPicHeight = height;
    DELETE_ARRAY(rwpk[i]->rectRegionPacking);
    m_tmpRegionrwpk = new RectangularRegionWisePacking[rwpk[i]->numRegions];
    if (!m_tmpRegionrwpk) {
      return ret;
    }
    rwpk[i]->rectRegionPacking = m_tmpRegionrwpk;
  }
  vector<uint32_t> packetSizeOfEachArr(arrangeNum, 0);
  uint32_t totalPacketNum = 0;
  for (uint32_t i = 0; i < arrangeNum; i++){
    totalPacketNum += mergeLayout[i]->tilesLayout.tileColsNum * mergeLayout[i]->tilesLayout.tileRowsNum;
    packetSizeOfEachArr[i] = totalPacketNum;
  }

  uint8_t regIdx = 0;
  uint32_t packetArrCnt = 0;
  for (itPacket = packets.begin(); ; itPacket++) {
    if (regIdx >= totalPacketNum)
        break;

    if (itPacket == packets.end())
    {
        if (totalPacketNum > packets.size())
        {
            itPacket = packets.begin();
        }
        else if (totalPacketNum == packets.size())
        {
            break;
        }
        else
        {
            OMAF_LOG(LOG_ERROR, "total packet number is less than seleceted packet size!\n");
            return ret;
        }
    }

    if (regIdx >= packetSizeOfEachArr[packetArrCnt]){
      packetArrCnt++;
    }
    MediaPacket *onePacket = itPacket->second;
    uint32_t realIdx = packetArrCnt == 0 ? regIdx : regIdx - packetSizeOfEachArr[packetArrCnt - 1];
    RectangularRegionWisePacking *rectReg = &(rwpk[packetArrCnt]->rectRegionPacking[realIdx]);
    memset(rectReg, 0, sizeof(RectangularRegionWisePacking));

    rectReg->transformType = 0;
    rectReg->guardBandFlag = false;

    SRDInfo srd = onePacket->GetSRDInfo();
    if (qualityRanking == HIGHEST_QUALITY_RANKING) {
      rectReg->projRegWidth = srd.width;
      rectReg->projRegHeight = srd.height;
      rectReg->projRegTop = srd.top;
      rectReg->projRegLeft = srd.left;

      rectReg->packedRegWidth = srd.width;
      rectReg->packedRegHeight = srd.height;
      uint8_t rowIdx = realIdx / mergeLayout[packetArrCnt]->tilesLayout.tileColsNum;
      uint8_t colIdx = realIdx % mergeLayout[packetArrCnt]->tilesLayout.tileColsNum;
      rectReg->packedRegTop = rowIdx * srd.height;
      rectReg->packedRegLeft = colIdx * srd.width;
    } else {
      rectReg->projRegWidth = (uint32_t)round((float)(srd.width * m_fullWidth) / (float)(mergeLayout[packetArrCnt]->mergedWidth));
      rectReg->projRegHeight = (uint32_t)round((float)(srd.height * m_fullHeight) / (float)(mergeLayout[packetArrCnt]->mergedHeight));
      rectReg->projRegTop = (uint32_t)round((float)(srd.top * m_fullHeight) / (float)(mergeLayout[packetArrCnt]->mergedHeight));
      rectReg->projRegLeft = (uint32_t)round((float)(srd.left * m_fullWidth) / (float)(mergeLayout[packetArrCnt]->mergedWidth));

      rectReg->packedRegWidth = srd.width;
      rectReg->packedRegHeight = srd.height;
      rectReg->packedRegTop = srd.top;
      rectReg->packedRegLeft = srd.left;
    }

    rectReg->leftGbWidth = 0;
    rectReg->rightGbWidth = 0;
    rectReg->topGbHeight = 0;
    rectReg->bottomGbHeight = 0;
    rectReg->gbNotUsedForPredFlag = true;
    rectReg->gbType0 = 0;
    rectReg->gbType1 = 0;
    rectReg->gbType2 = 0;
    rectReg->gbType3 = 0;

    regIdx++;
  }
  packetSizeOfEachArr.clear();
  return rwpk;
}

vector<std::unique_ptr<RegionWisePacking>> OmafTilesStitch::CalculateMergedRwpkForCubeMap(QualityRank qualityRanking,
                                                                                  bool hasPacketLost,
                                                                                  bool hasLayoutChanged) {
  vector<std::unique_ptr<RegionWisePacking>> ret;
  if (0 == m_selectedTiles.size()) return ret;

  if (hasPacketLost && hasLayoutChanged) {
    OMAF_LOG(LOG_ERROR, "Packet lost and layout change can't happen at the same time !\n");
    return ret;
  }

  std::map<QualityRank, std::map<uint32_t, MediaPacket *>>::iterator it;
  it = m_selectedTiles.find(qualityRanking);
  if (it == m_selectedTiles.end()) {
    OMAF_LOG(LOG_ERROR, "Can't find media packets of specified quality ranking !\n");
    return ret;
  }

  std::map<uint32_t, MediaPacket *> packets = it->second;
  if (0 == packets.size()) {
    OMAF_LOG(LOG_ERROR, "Invalid media packets size for specified quality ranking !\n");
    return ret;
  }

  vector<TilesMergeArrangement *> mergeLayout;
  std::map<QualityRank, vector<TilesMergeArrangement *>>::iterator itArr;
  std::map<uint32_t, MediaPacket *>::iterator itPacket;

  if (0 == m_initTilesMergeArr.size() && 0 == m_updatedTilesMergeArr.size()) {
    OMAF_LOG(LOG_ERROR, "There is no tiles merge layout before calculating rwpk !\n");
    return ret;
  }
  OMAF_LOG(LOG_INFO, "hasPacketLost: %d, hasLayoutChanged: %d\n", hasPacketLost, hasLayoutChanged);
  if (!hasPacketLost && !hasLayoutChanged) {
    if (0 == m_updatedTilesMergeArr.size()) {
      itArr = m_initTilesMergeArr.find(qualityRanking);
      if (itArr == m_initTilesMergeArr.end()) {
        OMAF_LOG(LOG_ERROR, "Can't find tiles merge layout for specified quality ranking !\n");
        return ret;
      }
      mergeLayout = itArr->second;
      if (mergeLayout.empty()) {
        OMAF_LOG(LOG_ERROR, "NULL tiles merge layout for specified quality ranking !\n");
        return ret;
      }
    } else {
      itArr = m_updatedTilesMergeArr.find(qualityRanking);
      if (itArr == m_updatedTilesMergeArr.end()) {
        OMAF_LOG(LOG_ERROR, "Can't find tiles merge layout for specified quality ranking !\n");
        return ret;
      }
      mergeLayout = itArr->second;
      if (mergeLayout.empty()) {
        OMAF_LOG(LOG_ERROR, "NULL tiles merge layout for specified quality ranking !\n");
        return ret;
      }
    }
  } else if (!hasPacketLost && hasLayoutChanged) {
    itArr = m_updatedTilesMergeArr.find(qualityRanking);
    if (itArr == m_updatedTilesMergeArr.end()) {
      OMAF_LOG(LOG_ERROR, "Can't find tiles merge layout for specified quality ranking !\n");
      return ret;
    }

    mergeLayout = itArr->second;
    if (mergeLayout.empty()) {
      OMAF_LOG(LOG_ERROR, "NULL tiles merge layout for specified quality ranking !\n");
      return ret;
    }
  }

  if (mergeLayout.empty()) {
    OMAF_LOG(LOG_ERROR, "NULL tiles merge layout for specified quality ranking !\n");
    return ret;
  }

  uint32_t arrangeNum = mergeLayout.size();
  vector<std::unique_ptr<RegionWisePacking>> rwpk; // = make_unique_vcd<RegionWisePacking>();
  for (uint32_t i = 0; i < arrangeNum; i++) {
    std::unique_ptr<RegionWisePacking> onerwpk = make_unique_vcd<RegionWisePacking>();
    if (!onerwpk) return ret;
    rwpk.push_back(std::move(onerwpk));
  }
  for (uint32_t i = 0; i < mergeLayout.size(); i++)
  {
    uint32_t width = mergeLayout[i]->mergedWidth;
    uint32_t height = mergeLayout[i]->mergedHeight;
    uint8_t tileRowsNum = mergeLayout[i]->tilesLayout.tileRowsNum;
    uint8_t tileColsNum = mergeLayout[i]->tilesLayout.tileColsNum;

    rwpk[i]->constituentPicMatching = 0;
    rwpk[i]->numRegions = (uint8_t)(tileRowsNum) * (uint8_t)(tileColsNum);
    rwpk[i]->projPicWidth = m_fullWidth;
    rwpk[i]->projPicHeight = m_fullHeight;
    rwpk[i]->packedPicWidth = width;
    rwpk[i]->packedPicHeight = height;
    DELETE_ARRAY(rwpk[i]->rectRegionPacking);
    m_tmpRegionrwpk = new RectangularRegionWisePacking[rwpk[i]->numRegions];
    if (!m_tmpRegionrwpk) {
      return ret;
    }
    rwpk[i]->rectRegionPacking = m_tmpRegionrwpk;
  }

  vector<uint32_t> packetSizeOfEachArr(arrangeNum, 0);
  uint32_t totalPacketNum = 0;
  for (uint32_t i = 0; i < arrangeNum; i++){
    totalPacketNum += mergeLayout[i]->tilesLayout.tileColsNum * mergeLayout[i]->tilesLayout.tileRowsNum;
    packetSizeOfEachArr[i] = totalPacketNum;
  }

  uint8_t regIdx = 0;
  uint32_t packetArrCnt = 0;
  for (itPacket = packets.begin(); ; itPacket++) {
    if (regIdx >= totalPacketNum)
        break;

    if (itPacket == packets.end())
    {
        if (totalPacketNum > packets.size())
        {
            itPacket = packets.begin();
        }
        else if (totalPacketNum == packets.size())
        {
            break;
        }
        else
        {
            OMAF_LOG(LOG_ERROR, " total packet number is less than seleceted packet size!\n");
            return ret;
        }
    }

    if (regIdx >= packetSizeOfEachArr[packetArrCnt]){
      packetArrCnt++;
    }
    MediaPacket *onePacket = itPacket->second;
    uint32_t realIdx = packetArrCnt == 0 ? regIdx : regIdx - packetSizeOfEachArr[packetArrCnt - 1];
    RectangularRegionWisePacking *rectReg = &(rwpk[packetArrCnt]->rectRegionPacking[realIdx]);
    memset(rectReg, 0, sizeof(RectangularRegionWisePacking));

    const RegionWisePacking &tileRwpk = onePacket->GetRwpk();
    rectReg->transformType = tileRwpk.rectRegionPacking[0].transformType;
    rectReg->guardBandFlag = false;

    SRDInfo srd = onePacket->GetSRDInfo();
    if (qualityRanking == HIGHEST_QUALITY_RANKING) {
      rectReg->projRegWidth = tileRwpk.rectRegionPacking[0].projRegWidth;
      rectReg->projRegHeight = tileRwpk.rectRegionPacking[0].projRegHeight;
      rectReg->projRegTop = tileRwpk.rectRegionPacking[0].projRegTop;
      rectReg->projRegLeft = tileRwpk.rectRegionPacking[0].projRegLeft;

      rectReg->packedRegWidth = tileRwpk.rectRegionPacking[0].packedRegWidth;
      rectReg->packedRegHeight = tileRwpk.rectRegionPacking[0].packedRegHeight;
      uint8_t rowIdx = realIdx / mergeLayout[packetArrCnt]->tilesLayout.tileColsNum;
      uint8_t colIdx = realIdx % mergeLayout[packetArrCnt]->tilesLayout.tileColsNum;
      rectReg->packedRegTop = rowIdx * srd.height;
      rectReg->packedRegLeft = colIdx * srd.width;
    } else {
      rectReg->projRegWidth =
          (uint32_t)round((float)(tileRwpk.rectRegionPacking[0].projRegWidth * m_fullWidth) / (float)(mergeLayout[packetArrCnt]->mergedWidth));
      rectReg->projRegHeight =
          (uint32_t)round((float)(tileRwpk.rectRegionPacking[0].projRegHeight * m_fullHeight) / (float)(mergeLayout[packetArrCnt]->mergedHeight));
      rectReg->projRegTop =
          (uint32_t)round((float)(tileRwpk.rectRegionPacking[0].projRegTop * m_fullHeight) / (float)(mergeLayout[packetArrCnt]->mergedHeight));
      rectReg->projRegLeft =
          (uint32_t)round((float)(tileRwpk.rectRegionPacking[0].projRegLeft * m_fullWidth) / (float)(mergeLayout[packetArrCnt]->mergedWidth));

      rectReg->packedRegWidth = tileRwpk.rectRegionPacking[0].packedRegWidth;
      rectReg->packedRegHeight = tileRwpk.rectRegionPacking[0].packedRegHeight;
      rectReg->packedRegTop = tileRwpk.rectRegionPacking[0].packedRegTop;
      rectReg->packedRegLeft = tileRwpk.rectRegionPacking[0].packedRegLeft;
    }

    rectReg->leftGbWidth = 0;
    rectReg->rightGbWidth = 0;
    rectReg->topGbHeight = 0;
    rectReg->bottomGbHeight = 0;
    rectReg->gbNotUsedForPredFlag = true;
    rectReg->gbType0 = 0;
    rectReg->gbType1 = 0;
    rectReg->gbType2 = 0;
    rectReg->gbType3 = 0;

    regIdx++;
  }
  packetSizeOfEachArr.clear();
  return rwpk;
}

vector<std::unique_ptr<RegionWisePacking>> OmafTilesStitch::CalculateMergedRwpkForPlanar(QualityRank qualityRanking,
                                                                              bool hasPacketLost,
                                                                              bool hasLayoutChanged) {
  vector<std::unique_ptr<RegionWisePacking>> ret;
  if (0 == m_selectedTiles.size()) return ret;

  if (hasPacketLost && hasLayoutChanged) {
    OMAF_LOG(LOG_ERROR, "Packet lost and layout change can't happen at the same time !\n");
    return ret;
  }

  std::map<QualityRank, std::map<uint32_t, MediaPacket *>>::iterator it;
  it = m_selectedTiles.find(qualityRanking);
  if (it == m_selectedTiles.end()) {
    OMAF_LOG(LOG_ERROR, "Can't find media packets of specified quality ranking !\n");
    return ret;
  }

  std::map<uint32_t, MediaPacket *> packets = it->second;
  if (0 == packets.size()) {
    OMAF_LOG(LOG_ERROR, "Invalid media packets size for specified quality ranking !\n");
    return ret;
  }

  vector<TilesMergeArrangement *> mergeLayout;
  std::map<QualityRank, vector<TilesMergeArrangement *>>::iterator itArr;
  std::map<uint32_t, MediaPacket *>::iterator itPacket;

  if (0 == m_initTilesMergeArr.size() && 0 == m_updatedTilesMergeArr.size()) {
    OMAF_LOG(LOG_ERROR, "There is no tiles merge layout before calculating rwpk !\n");
    return ret;
  }
  OMAF_LOG(LOG_INFO, "hasPacketLost: %d, hasLayoutChanged: %d\n", hasPacketLost, hasLayoutChanged);
  if (!hasPacketLost && !hasLayoutChanged) {
    if (0 == m_updatedTilesMergeArr.size()) {
      itArr = m_initTilesMergeArr.find(qualityRanking);
      if (itArr == m_initTilesMergeArr.end()) {
        OMAF_LOG(LOG_ERROR, "Can't find tiles merge layout for specified quality ranking !\n");
        return ret;
      }
      mergeLayout = itArr->second;
      if (mergeLayout.empty()) {
        OMAF_LOG(LOG_ERROR, "nullptr tiles merge layout for specified quality ranking !\n");
        return ret;
      }
    } else {
      itArr = m_updatedTilesMergeArr.find(qualityRanking);
      if (itArr == m_updatedTilesMergeArr.end()) {
        OMAF_LOG(LOG_ERROR, "Can't find tiles merge layout for specified quality ranking !\n");
        return ret;
      }
      mergeLayout = itArr->second;
      if (mergeLayout.empty()) {
        OMAF_LOG(LOG_ERROR, "nullptr tiles merge layout for specified quality ranking !\n");
        return ret;
      }
    }
  } else if (!hasPacketLost && hasLayoutChanged) {
    itArr = m_updatedTilesMergeArr.find(qualityRanking);
    if (itArr == m_updatedTilesMergeArr.end()) {
      OMAF_LOG(LOG_ERROR, "Can't find tiles merge layout for specified quality ranking !\n");
      return ret;
    }

    mergeLayout = itArr->second;
    if (mergeLayout.empty()) {
      OMAF_LOG(LOG_ERROR, "nullptr tiles merge layout for specified quality ranking !\n");
      return ret;
    }
  }

  if (mergeLayout.empty()) {
    OMAF_LOG(LOG_ERROR, "nullptr tiles merge layout for specified quality ranking !\n");
    return ret;
  }

  uint32_t arrangeNum = mergeLayout.size();
  vector<std::unique_ptr<RegionWisePacking>> rwpk; // = make_unique_vcd<RegionWisePacking>();
  for (uint32_t i = 0; i < arrangeNum; i++) {
    std::unique_ptr<RegionWisePacking> onerwpk = make_unique_vcd<RegionWisePacking>();
    if (!onerwpk) return ret;
    rwpk.push_back(std::move(onerwpk));
  }
  for (uint32_t i = 0; i < mergeLayout.size(); i++)
  {
    uint32_t width = mergeLayout[i]->mergedWidth;
    uint32_t height = mergeLayout[i]->mergedHeight;
    uint8_t tileRowsNum = mergeLayout[i]->tilesLayout.tileRowsNum;
    uint8_t tileColsNum = mergeLayout[i]->tilesLayout.tileColsNum;

    rwpk[i]->constituentPicMatching = 0;
    rwpk[i]->numRegions = (uint8_t)(tileRowsNum) * (uint8_t)(tileColsNum);
    rwpk[i]->packedPicWidth = width;
    rwpk[i]->packedPicHeight = height;
    DELETE_ARRAY(rwpk[i]->rectRegionPacking);
    m_tmpRegionrwpk = new RectangularRegionWisePacking[rwpk[i]->numRegions];
    if (!m_tmpRegionrwpk) {
      return ret;
    }
    rwpk[i]->rectRegionPacking = m_tmpRegionrwpk;
  }
  vector<uint32_t> packetSizeOfEachArr(arrangeNum, 0);
  uint32_t totalPacketNum = 0;
  for (uint32_t i = 0; i < arrangeNum; i++){
    totalPacketNum += mergeLayout[i]->tilesLayout.tileColsNum * mergeLayout[i]->tilesLayout.tileRowsNum;
    packetSizeOfEachArr[i] = totalPacketNum;
  }

  uint8_t regIdx = 0;
  uint32_t packetArrCnt = 0;
  for (itPacket = packets.begin(); ; itPacket++) {
    if (regIdx >= totalPacketNum)
        break;

    if (itPacket == packets.end())
    {
        if (totalPacketNum > packets.size())
        {
            itPacket = packets.begin();
        }
        else if (totalPacketNum == packets.size())
        {
            break;
        }
        else
        {
            OMAF_LOG(LOG_ERROR, "total packet number is less than seleceted packet size!\n");
            return ret;
        }
    }

    if (regIdx >= packetSizeOfEachArr[packetArrCnt]){
      packetArrCnt++;
    }
    MediaPacket *onePacket = itPacket->second;
    uint32_t origProjPicWidth = (onePacket->GetRwpk()).projPicWidth;
    uint32_t origProjPicHeight = (onePacket->GetRwpk()).projPicHeight;
    OMAF_LOG(LOG_INFO, "Orig ProjPicWidth %d and ProjPicHeight %d\n", origProjPicWidth, origProjPicHeight);
    rwpk[packetArrCnt]->projPicWidth = origProjPicWidth;
    rwpk[packetArrCnt]->projPicHeight = origProjPicHeight;

    uint32_t realIdx = packetArrCnt == 0 ? regIdx : regIdx - packetSizeOfEachArr[packetArrCnt - 1];
    RectangularRegionWisePacking *rectReg = &(rwpk[packetArrCnt]->rectRegionPacking[realIdx]);
    memset(rectReg, 0, sizeof(RectangularRegionWisePacking));

    rectReg->transformType = 0;
    rectReg->guardBandFlag = false;

    SRDInfo srd = onePacket->GetSRDInfo();

    rectReg->projRegWidth = srd.width;
    rectReg->projRegHeight = srd.height;
    rectReg->projRegTop = srd.top;
    rectReg->projRegLeft = srd.left;

    rectReg->packedRegWidth = srd.width;
    rectReg->packedRegHeight = srd.height;
    uint8_t rowIdx = realIdx / mergeLayout[packetArrCnt]->tilesLayout.tileColsNum;
    uint8_t colIdx = realIdx % mergeLayout[packetArrCnt]->tilesLayout.tileColsNum;
    rectReg->packedRegTop = rowIdx * srd.height;
    rectReg->packedRegLeft = colIdx * srd.width;

    rectReg->leftGbWidth = 0;
    rectReg->rightGbWidth = 0;
    rectReg->topGbHeight = 0;
    rectReg->bottomGbHeight = 0;
    rectReg->gbNotUsedForPredFlag = true;
    rectReg->gbType0 = 0;
    rectReg->gbType1 = 0;
    rectReg->gbType2 = 0;
    rectReg->gbType3 = 0;

    regIdx++;
  }
  packetSizeOfEachArr.clear();
  return rwpk;
}

int32_t OmafTilesStitch::GenerateTilesMergeArrangement() {
  if (0 == m_selectedTiles.size()) return OMAF_ERROR_INVALID_DATA;

  if (0 == m_initTilesMergeArr.size()) {
    m_initTilesMergeArr = CalculateTilesMergeArrangement();
    if (0 == m_initTilesMergeArr.size()) {
      OMAF_LOG(LOG_ERROR, "Failed to calculate tiles merged arrangement !\n");
      return OMAF_ERROR_TILES_MERGE_ARRANGEMENT;
    }
  } else {
    if (m_updatedTilesMergeArr.size()) {
      if (m_initTilesMergeArr.size() != m_updatedTilesMergeArr.size())
        OMAF_LOG(LOG_INFO, "The number of tiles merged video streams has been changed compared with the number at the beginning !\n");

      std::map<QualityRank, vector<TilesMergeArrangement *>>::iterator itArr;
      for (itArr = m_updatedTilesMergeArr.begin(); itArr != m_updatedTilesMergeArr.end();) {
        vector<TilesMergeArrangement *> layOutArr = itArr->second;
        for (uint32_t i = 0; i < layOutArr.size(); i++)
        {
          if (layOutArr[i]) {
            DELETE_ARRAY(layOutArr[i]->tilesLayout.tileRowHeight);
            DELETE_ARRAY(layOutArr[i]->tilesLayout.tileColWidth);
            SAFE_DELETE(layOutArr[i]);
          }
        }
        m_updatedTilesMergeArr.erase(itArr++);
      }
      m_updatedTilesMergeArr.clear();
    }

    m_updatedTilesMergeArr = CalculateTilesMergeArrangement();
    if (0 == m_updatedTilesMergeArr.size()) {
      OMAF_LOG(LOG_ERROR, "Failed to calculate tiles merged arrangement\n");
      return OMAF_ERROR_TILES_MERGE_ARRANGEMENT;
    }
  }

  return ERROR_NONE;
}

int32_t OmafTilesStitch::IsArrChanged(QualityRank qualityRanking, vector<TilesMergeArrangement *> layOut, vector<TilesMergeArrangement *> initLayOut, bool *isArrChanged, bool *packetLost, bool *arrangeChanged)
{
    if (layOut.empty()) {
        OMAF_LOG(LOG_ERROR, " Invalid tile merge arrangement data!\n");
        return OMAF_ERROR_NULL_PTR;
    }
    if (isArrChanged == NULL || packetLost == NULL || arrangeChanged == NULL) {
        OMAF_LOG(LOG_ERROR, "Invalid flags for arrangement!\n");
        return OMAF_ERROR_INVALID_DATA;
    }
    if (layOut.size() != initLayOut.size()) {
      *isArrChanged = true;
      *arrangeChanged = true;
    }
    else {
      for (uint32_t i = 0; i < layOut.size(); i++) {
        uint32_t width = layOut[i]->mergedWidth;
        uint32_t height = layOut[i]->mergedHeight;

        uint32_t initWidth = initLayOut[i]->mergedWidth;
        uint32_t initHeight = initLayOut[i]->mergedHeight;

        if ((width == initWidth) && (height < initHeight)) {
          OMAF_LOG(LOG_INFO, "Packet not lost but tiles merge layout has been changed !\n");
          *arrangeChanged = true;
          *isArrChanged = true;
        }

        if ((height == initHeight) && (width < initWidth)) {
          OMAF_LOG(LOG_INFO, "Packet not lost but tiles merge layout has been changed !\n");
          *arrangeChanged = true;
          *isArrChanged = true;
        }

        if ((width < initWidth) && (height < initHeight)) {
          OMAF_LOG(LOG_INFO, "Packet not lost but tiles merge layout has been changed !\n");
          *arrangeChanged = true;
          *isArrChanged = true;
        }

        if ((width > initWidth) || (height > initHeight)) {
          OMAF_LOG(LOG_INFO, "Packet not lost but tiles merge layout has been changed !\n");
          *arrangeChanged = true;
          *isArrChanged = true;
        }
      }
    }
    OMAF_LOG(LOG_INFO, "arrangeChanged %d, isArrChanged %d\n", *arrangeChanged, *isArrChanged);
    return ERROR_NONE;
}

int32_t OmafTilesStitch::GenerateMergedVideoHeaders(bool arrangeChanged, QualityRank qualityRanking,
    vector<TilesMergeArrangement *> layOut,
    vector<TilesMergeArrangement *> initLayOut,
    std::map<uint32_t, MediaPacket *> packets) {
    int32_t ret = ERROR_NONE;
    if (layOut.empty()) {
        OMAF_LOG(LOG_ERROR, "INVALID tile merge arrangement data!\n");
        return OMAF_ERROR_NULL_PTR;
    }
    // 1. clear all headers
    std::map<QualityRank, vector<std::map<uint32_t, uint8_t *>>>::iterator itMergeHrd;
    for (itMergeHrd = m_mergedVideoHeaders.begin(); itMergeHrd != m_mergedVideoHeaders.end(); ) {
      QualityRank oneQualityRanking = itMergeHrd->first;
      vector<std::map<uint32_t, uint8_t *>> oneVideoHeaderArr = m_mergedVideoHeaders[oneQualityRanking];
      for (uint32_t i = 0; i < oneVideoHeaderArr.size(); i++) {
        std::map<uint32_t, uint8_t *>::iterator itHdr = oneVideoHeaderArr[i].begin();
        if (itHdr == oneVideoHeaderArr[i].end())
        {
            OMAF_LOG(LOG_ERROR, "Video header map is empty!\n");
            return OMAF_ERROR_INVALID_DATA;
        }
        uint8_t *headers = itHdr->second;
        DELETE_ARRAY(headers);
        oneVideoHeaderArr[i].clear();
      }
      oneVideoHeaderArr.clear();
      m_mergedVideoHeaders.erase(itMergeHrd++);
    }
    // 2. generate new headers.
    if ((qualityRanking == HIGHEST_QUALITY_RANKING) && (0 == m_mergedVideoHeaders[qualityRanking].size())) {
      if (qualityRanking == HIGHEST_QUALITY_RANKING) {
        if (!m_fullResVideoHeader) {
          OMAF_LOG(LOG_ERROR, "nullptr original video headers data !\n");
          return OMAF_ERROR_NULL_PTR;
        }
        vector<std::map<uint32_t, uint8_t*>> videoHeaders;
        for (uint32_t i = 0; i < layOut.size(); i++) {
          uint32_t headersSize = 0;
          uint8_t *headers = new uint8_t[1024];
          if (!headers) return OMAF_ERROR_NULL_PTR;

          memset(headers, 0, 1024);
          memcpy_s(headers, 1024, m_fullResVideoHeader, m_fullResVPSSize);
          headersSize += m_fullResVPSSize;

          uint8_t *tmp = headers + m_fullResVPSSize;
          m_360scvpParam->pInputBitstream = m_fullResVideoHeader + m_fullResVPSSize;
          m_360scvpParam->inputBitstreamLen = m_fullResSPSSize;
          m_360scvpParam->destWidth = (arrangeChanged || initLayOut.size() < i + 1 ? layOut[i]->mergedWidth : initLayOut[i]->mergedWidth);
          m_360scvpParam->destHeight = (arrangeChanged || initLayOut.size() < i + 1 ? layOut[i]->mergedHeight : initLayOut[i]->mergedHeight);
          m_360scvpParam->pOutputBitstream = tmp;
          ret = I360SCVP_GenerateSPS(m_360scvpParam, m_360scvpHandle);
          if (ret) {
            DELETE_ARRAY(headers);
            return OMAF_ERROR_SCVP_OPERATION_FAILED;
          }

          headersSize += m_360scvpParam->outputBitstreamLen;
          tmp += m_360scvpParam->outputBitstreamLen;
          m_360scvpParam->pInputBitstream = m_fullResVideoHeader + m_fullResVPSSize + m_fullResSPSSize;
          m_360scvpParam->inputBitstreamLen = m_fullResPPSSize;
          m_360scvpParam->pOutputBitstream = tmp;
          if (arrangeChanged) {
            ret = I360SCVP_GeneratePPS(m_360scvpParam, &(layOut[i]->tilesLayout), m_360scvpHandle);
          } else {
            ret = I360SCVP_GeneratePPS(m_360scvpParam, &(initLayOut[i]->tilesLayout), m_360scvpHandle);
          }

          if (ret) {
            DELETE_ARRAY(headers);
            return OMAF_ERROR_SCVP_OPERATION_FAILED;
          }

          headersSize += m_360scvpParam->outputBitstreamLen;

          std::map<uint32_t, uint8_t *> oneVideoHeader;
          oneVideoHeader.insert(std::make_pair(headersSize, headers));
          videoHeaders.push_back(oneVideoHeader);
        }
        m_mergedVideoHeaders[qualityRanking] = std::move(videoHeaders);
      }
    }

    if ((qualityRanking > HIGHEST_QUALITY_RANKING) && (0 == m_mergedVideoHeaders[qualityRanking].size())) {
      if (qualityRanking > HIGHEST_QUALITY_RANKING) {
        std::map<uint32_t, MediaPacket *>::iterator itPacket;
        itPacket = packets.begin();
        if (itPacket == packets.end())
        {
            OMAF_LOG(LOG_ERROR, "Packets map is empty!\n");
            return OMAF_ERROR_INVALID_DATA;
        }
        MediaPacket *onePacket = itPacket->second;
        if (!(onePacket->GetHasVideoHeader())) {
            OMAF_LOG(LOG_ERROR, "There should be video headers here !\n");
            return OMAF_ERROR_INVALID_DATA;
        }
        //get original VPS/SPS/PPS
        uint32_t hrdSize = onePacket->GetVideoHeaderSize();
        uint8_t *headersData = new uint8_t[hrdSize];
        if (!headersData) {
          return OMAF_ERROR_NULL_PTR;
        }
        memcpy_s(headersData, hrdSize, onePacket->Payload(), hrdSize);
        vector<std::map<uint32_t, uint8_t*>> videoHeaders;
        for (uint32_t i = 0; i < layOut.size(); i++) {
        //generate new VPS/SPS/PPS for merged video
          uint32_t headersSize = 0;
          uint8_t *headers = new uint8_t[1024];
          if (!headers)
          {
              DELETE_ARRAY(headersData);
              return OMAF_ERROR_NULL_PTR;
          }
          memset(headers, 0, 1024);
          uint32_t vpsLen = onePacket->GetVPSLen();
          uint32_t spsLen = onePacket->GetSPSLen();
          uint32_t ppsLen = onePacket->GetPPSLen();
          memcpy_s(headers, vpsLen, headersData, vpsLen);
          headersSize += vpsLen;

          uint8_t *tmp = headers + vpsLen;
          m_360scvpParam->pInputBitstream = headersData + vpsLen;
          m_360scvpParam->inputBitstreamLen = spsLen;
          m_360scvpParam->destWidth = layOut[i]->mergedWidth;
          m_360scvpParam->destHeight = layOut[i]->mergedHeight;
          m_360scvpParam->pOutputBitstream = tmp;
          ret = I360SCVP_GenerateSPS(m_360scvpParam, m_360scvpHandle);
          if (ret) {
            DELETE_ARRAY(headersData);
            DELETE_ARRAY(headers);
            return OMAF_ERROR_SCVP_OPERATION_FAILED;
          }

          headersSize += m_360scvpParam->outputBitstreamLen;
          tmp += m_360scvpParam->outputBitstreamLen;
          m_360scvpParam->pInputBitstream = headersData + vpsLen + spsLen;
          m_360scvpParam->inputBitstreamLen = ppsLen;
          m_360scvpParam->pOutputBitstream = tmp;

          ret = I360SCVP_GeneratePPS(m_360scvpParam, &(layOut[i]->tilesLayout), m_360scvpHandle);
          if (ret) {
            DELETE_ARRAY(headersData);
            DELETE_ARRAY(headers);
            return OMAF_ERROR_SCVP_OPERATION_FAILED;
          }

          headersSize += m_360scvpParam->outputBitstreamLen;

          std::map<uint32_t, uint8_t *> oneVideoHeader;
          oneVideoHeader.insert(std::make_pair(headersSize, headers));
          videoHeaders.push_back(oneVideoHeader);
        }
        m_mergedVideoHeaders[qualityRanking] = std::move(videoHeaders);
        DELETE_ARRAY(headersData);
      }
    }
    return ret;
}

vector<std::unique_ptr<RegionWisePacking>> OmafTilesStitch::GenerateMergedRWPK(QualityRank qualityRanking, bool packetLost, bool arrangeChanged) {
    vector<std::unique_ptr<RegionWisePacking>> rwpk;
    if (m_projFmt == VCD::OMAF::ProjectionFormat::PF_ERP) {
      rwpk = CalculateMergedRwpkForERP(qualityRanking, packetLost, arrangeChanged);
    } else if (m_projFmt == VCD::OMAF::ProjectionFormat::PF_CUBEMAP) {
      rwpk = CalculateMergedRwpkForCubeMap(qualityRanking, packetLost, arrangeChanged);
    } else if (m_projFmt == VCD::OMAF::ProjectionFormat::PF_PLANAR) {
      rwpk = CalculateMergedRwpkForPlanar(qualityRanking, packetLost, arrangeChanged);
    }

    return rwpk;
}

int32_t OmafTilesStitch::UpdateMergedVideoHeadersForLowQualityRank(bool isEmptyHeader,
    std::map<uint32_t, MediaPacket *> packets, QualityRank qualityRanking,
    TilesMergeArrangement *layOut) {
    int32_t ret = ERROR_NONE;
    std::map<uint32_t, MediaPacket *>::iterator itPacket;
    if (isEmptyHeader) {
        itPacket = packets.begin();
        if (itPacket == packets.end())
        {
            OMAF_LOG(LOG_ERROR, "Packets map is empty!\n");
            return OMAF_ERROR_INVALID_DATA;
        }
        MediaPacket *onePacket = itPacket->second;
        if (!(onePacket->GetHasVideoHeader())) {
            OMAF_LOG(LOG_ERROR, "There should be video headers here !\n");
            return OMAF_ERROR_INVALID_DATA;
        }
        //get original VPS/SPS/PPS
        uint32_t hrdSize = onePacket->GetVideoHeaderSize();
        uint8_t *headersData = new uint8_t[hrdSize];
        if (!headersData) {
          return OMAF_ERROR_NULL_PTR;
        }
        memcpy_s(headersData, hrdSize, onePacket->Payload(), hrdSize);

        //generate new VPS/SPS/PPS for merged video
        uint32_t headersSize = 0;
        uint8_t *headers = new uint8_t[1024];
        if (!headers)
        {
            DELETE_ARRAY(headersData);
            return OMAF_ERROR_NULL_PTR;
        }
        memset(headers, 0, 1024);
        uint32_t vpsLen = onePacket->GetVPSLen();
        uint32_t spsLen = onePacket->GetSPSLen();
        uint32_t ppsLen = onePacket->GetPPSLen();
        memcpy_s(headers, vpsLen, headersData, vpsLen);
        headersSize += vpsLen;

        uint8_t *tmp = headers + vpsLen;
        m_360scvpParam->pInputBitstream = headersData + vpsLen;
        m_360scvpParam->inputBitstreamLen = spsLen;
        m_360scvpParam->destWidth = layOut->mergedWidth;
        m_360scvpParam->destHeight = layOut->mergedHeight;
        m_360scvpParam->pOutputBitstream = tmp;
        ret = I360SCVP_GenerateSPS(m_360scvpParam, m_360scvpHandle);
        if (ret) {
          DELETE_ARRAY(headersData);
          DELETE_ARRAY(headers);
          return OMAF_ERROR_SCVP_OPERATION_FAILED;
        }

        headersSize += m_360scvpParam->outputBitstreamLen;
        tmp += m_360scvpParam->outputBitstreamLen;
        m_360scvpParam->pInputBitstream = headersData + vpsLen + spsLen;
        m_360scvpParam->inputBitstreamLen = ppsLen;
        m_360scvpParam->pOutputBitstream = tmp;

        ret = I360SCVP_GeneratePPS(m_360scvpParam, &(layOut->tilesLayout), m_360scvpHandle);
        if (ret) {
          DELETE_ARRAY(headersData);
          DELETE_ARRAY(headers);
          return OMAF_ERROR_SCVP_OPERATION_FAILED;
        }

        headersSize += m_360scvpParam->outputBitstreamLen;

        std::map<uint32_t, uint8_t *> oneVideoHeader;
        oneVideoHeader.insert(std::make_pair(headersSize, headers));
        m_mergedVideoHeaders[qualityRanking].push_back(std::move(oneVideoHeader));
        DELETE_ARRAY(headersData);
    }
    return ret;
}

int32_t OmafTilesStitch::InitMergedDataAndRealSize(QualityRank qualityRanking, std::map<uint32_t, MediaPacket *> packets,
    char* mergedData, uint64_t* realSize, uint32_t index,
    TilesMergeArrangement *tilesArr) {
    if (packets.empty()) {
        OMAF_LOG(LOG_ERROR, "packets is empty!\n");
        return OMAF_ERROR_INVALID_DATA;
    }
    if (mergedData == NULL || realSize == NULL) {
        OMAF_LOG(LOG_ERROR, "merged data or real size is null ptr!\n");
        return OMAF_ERROR_NULL_PTR;
    }
    if (m_needHeaders) {
      vector<std::map<uint32_t, uint8_t *>> videoHeaders = m_mergedVideoHeaders[qualityRanking];
      bool isEmptyHeaders = (videoHeaders.empty() ? true : videoHeaders[index].empty());
      if (qualityRanking != HIGHEST_QUALITY_RANKING) {
        if (ERROR_NONE != UpdateMergedVideoHeadersForLowQualityRank(isEmptyHeaders, packets, qualityRanking, tilesArr)) {
            OMAF_LOG(LOG_ERROR, "Update merged video headers for low quality ranking failed!\n");
            return OMAF_ERROR_OPERATION;
        }
      }
      if (m_mergedVideoHeaders[qualityRanking].empty()) {
        OMAF_LOG(LOG_ERROR, "Video headers for Quality %d is empty!\n", qualityRanking);
        return OMAF_ERROR_INVALID_DATA;
      }
      if (m_mergedVideoHeaders[qualityRanking][index].empty()) {
        OMAF_LOG(LOG_ERROR, "Failed to generate merged video headers for quality ranking %d split %d\n", qualityRanking, index);
        return OMAF_ERROR_INVALID_DATA;
      }
      std::map<uint32_t, uint8_t *> oneVideoHeader = (m_mergedVideoHeaders[qualityRanking][index]);
      std::map<uint32_t, uint8_t *>::iterator itHdr = oneVideoHeader.begin();
      if (itHdr == oneVideoHeader.end())
      {
          OMAF_LOG(LOG_ERROR, "Video header map is empty!\n");
          return OMAF_ERROR_INVALID_DATA;
      }
      uint8_t *headers = itHdr->second;
      uint32_t headersLen = itHdr->first;
      if (!headers) {
        return OMAF_ERROR_NULL_PTR;
      }
      memcpy_s(mergedData, headersLen, headers, headersLen);
      *realSize += headersLen;
    }
    return ERROR_NONE;
}

int32_t OmafTilesStitch::UpdateMergedDataAndRealSize(
    QualityRank qualityRanking, std::map<uint32_t, MediaPacket *> packets,
    uint8_t tileColsNum, bool arrangeChanged, uint32_t width, uint32_t height,
    uint32_t initWidth, uint32_t initHeight, char *mergedData, uint64_t *realSize,
    uint32_t index, vector<uint32_t> needPacketSize, uint64_t layoutNum) {

    uint32_t tilesIdx = 0;
    int32_t tileWidth = 0;
    int32_t tileHeight = 0;
    if (tileColsNum == 0) {
        OMAF_LOG(LOG_ERROR, "tile column number cannot be zero!\n");
        return OMAF_ERROR_INVALID_DATA;
    }
    if (mergedData == NULL || realSize == NULL) {
        OMAF_LOG(LOG_ERROR, "merged data or realSize is null ptr!\n");
        return OMAF_ERROR_NULL_PTR;
    }
    // calculate real size for merged packets
    std::map<uint32_t, MediaPacket *>::iterator itPacket = packets.begin();
    if (index > 0)
      std::advance(itPacket, needPacketSize[index - 1]);
    if (itPacket == packets.end())
    {
        if (needPacketSize[layoutNum-1] > packets.size())
        {
            itPacket = packets.begin();
        }
        else
        {
            OMAF_LOG(LOG_ERROR, "ERROR in selected media packets for tiles stitching !\n");
            return OMAF_ERROR_INVALID_DATA;
        }
    }

    uint32_t packetSize = index == 0 ? needPacketSize[index] : needPacketSize[index] - needPacketSize[index - 1];
    uint32_t cnt = 0;
    for (; ; itPacket++) {
      if (itPacket == packets.end())
      {
          if (needPacketSize[layoutNum-1] > packets.size())
              itPacket = packets.begin();
      }
      cnt++;
      if (cnt > packetSize) break;
      if (itPacket == packets.end())
      {
          OMAF_LOG(LOG_ERROR, "There is mismatch in needed packets size and actually selected media packets !\n");
          return OMAF_ERROR_INVALID_DATA;
      }
      MediaPacket *onePacket = NULL;
      onePacket = itPacket->second;
      if (!onePacket)
      {
          OMAF_LOG(LOG_ERROR, "Selected media packet is NULL !\n");
          return OMAF_ERROR_NULL_PTR;
      }
      //if (qualityRanking == HIGHEST_QUALITY_RANKING) {
      std::map<uint32_t, SourceInfo>::iterator itSrc;
      itSrc = m_sources.find(qualityRanking);
      if (itSrc == m_sources.end())
      {
        OMAF_LOG(LOG_ERROR, "Can't find source information corresponding to quality ranking %d\n", qualityRanking);
        return OMAF_ERROR_INVALID_DATA;
      }
      SourceInfo srcInfo = itSrc->second;

      OMAF_LOG(LOG_INFO, "Original source width %d, height %d\n", srcInfo.width, srcInfo.height);
      OMAF_LOG(LOG_INFO, "Merged source width %d, height %d\n", width, height);
      if ((width != (uint32_t)(srcInfo.width)) || (height != (uint32_t)(srcInfo.height))) {
        SRDInfo srd = onePacket->GetSRDInfo();
        if (!tileWidth || !tileHeight) {
          tileWidth = srd.width;
          tileHeight = srd.height;
        }

        uint8_t colIdx = tilesIdx % tileColsNum;
        uint8_t rowIdx = tilesIdx / tileColsNum;
        uint16_t ctuIdx =
            rowIdx * (tileHeight / LCU_SIZE) * ((tileWidth / LCU_SIZE) * tileColsNum) + colIdx * (tileWidth / LCU_SIZE);
        // LOG(INFO)<< "SRD: " << srd.top << " and  "<<srd.left<<endl;
        // LOG(INFO)<<"New ctuIdx  "<<ctuIdx<<endl;
        char *data = onePacket->Payload();
        int32_t dataSize = onePacket->Size();
        if (!data || !dataSize)
        {
            OMAF_LOG(LOG_ERROR, "Invalid data in selected media packet !\n");
            return OMAF_ERROR_INVALID_DATA;
        }

        if (onePacket->GetHasVideoHeader()) {
          data += (onePacket->GetVPSLen() + onePacket->GetSPSLen() + onePacket->GetPPSLen());
          dataSize -= (onePacket->GetVPSLen() + onePacket->GetSPSLen() + onePacket->GetPPSLen());
        }
        if (!data || !dataSize)
        {
            OMAF_LOG(LOG_ERROR, "After video headers (VPS/SPS/PPS) are moved, invalid data in selected media packet !\n");
            return OMAF_ERROR_INVALID_DATA;
        }

        Nalu *nalu = new Nalu;
        if (!nalu) {
          return OMAF_ERROR_NULL_PTR;
        }

        nalu->data = (uint8_t *)data;
        nalu->dataSize = dataSize;
        I360SCVP_ParseNAL(nalu, m_360scvpHandle);

        nalu->sliceHeaderLen = nalu->sliceHeaderLen - HEVC_NALUHEADER_LEN;

        m_360scvpParam->destWidth = (arrangeChanged ? width : initWidth);
        m_360scvpParam->destHeight = (arrangeChanged ? height : initHeight);
        m_360scvpParam->pInputBitstream = (uint8_t *)data;
        m_360scvpParam->inputBitstreamLen = dataSize;
        m_360scvpParam->pOutputBitstream = (uint8_t *)mergedData + *realSize;
        I360SCVP_GenerateSliceHdr(m_360scvpParam, ctuIdx, m_360scvpHandle);
        *realSize += m_360scvpParam->outputBitstreamLen;
        memcpy_s(mergedData + *realSize,
                 (size_t(nalu->dataSize) - (HEVC_STARTCODES_LEN + HEVC_NALUHEADER_LEN + nalu->sliceHeaderLen)),
                 (nalu->data + HEVC_STARTCODES_LEN + HEVC_NALUHEADER_LEN + nalu->sliceHeaderLen),
                 (size_t(nalu->dataSize) - (HEVC_STARTCODES_LEN + HEVC_NALUHEADER_LEN + nalu->sliceHeaderLen)));

        *realSize += nalu->dataSize - (HEVC_STARTCODES_LEN + HEVC_NALUHEADER_LEN + nalu->sliceHeaderLen);
        SAFE_DELETE(nalu);
        tilesIdx++;
      } else {
        char *data = onePacket->Payload();
        int32_t dataSize = onePacket->Size();
        if (!data || !dataSize)
        {
            OMAF_LOG(LOG_ERROR, "Invalid data in selected media packet !\n");
            return OMAF_ERROR_INVALID_DATA;
        }

        if (onePacket->GetHasVideoHeader()) {
          data += onePacket->GetVideoHeaderSize();
          dataSize -= onePacket->GetVideoHeaderSize();
        }
        if (!data || !dataSize)
        {
            OMAF_LOG(LOG_ERROR, "After video headers (VPS/SPS/PPS) are moved, invalid data in selected media packet !\n");
            return OMAF_ERROR_INVALID_DATA;
        }
        memcpy_s(mergedData + *realSize, dataSize, data, dataSize);
        *realSize += dataSize;
      }
    }
    return ERROR_NONE;
}

int32_t OmafTilesStitch::UpdateInitTilesMergeArr() {

    for (auto it = m_initTilesMergeArr.begin(); it != m_initTilesMergeArr.end();) {
        vector<TilesMergeArrangement *> layOut = it->second;

        for (uint32_t i = 0; i < layOut.size(); i++) {
            if (layOut[i]) {
                DELETE_ARRAY(layOut[i]->tilesLayout.tileRowHeight);
                DELETE_ARRAY(layOut[i]->tilesLayout.tileColWidth);
            }
            SAFE_DELETE(layOut[i]);
        }
        layOut.clear();
        m_initTilesMergeArr.erase(it++);
    }
    m_initTilesMergeArr.clear();

    for (auto it = m_updatedTilesMergeArr.begin(); it != m_updatedTilesMergeArr.end(); it++) { // for each quality
      vector<TilesMergeArrangement *> existedArr = it->second;
      if (existedArr.empty()) return OMAF_ERROR_NULL_PTR;
      vector<TilesMergeArrangement*> arr;
      for (uint32_t i = 0; i < existedArr.size(); i++) { // for each merged packet
        TilesMergeArrangement *oneArr = new TilesMergeArrangement;
        if (!oneArr) return OMAF_ERROR_NULL_PTR;

        oneArr->mergedWidth = existedArr[i]->mergedWidth;
        oneArr->mergedHeight = existedArr[i]->mergedHeight;
        oneArr->mostTopPos = existedArr[i]->mostTopPos;
        oneArr->mostLeftPos = existedArr[i]->mostLeftPos;
        oneArr->tilesLayout.tileRowsNum = existedArr[i]->tilesLayout.tileRowsNum;
        oneArr->tilesLayout.tileColsNum = existedArr[i]->tilesLayout.tileColsNum;
        oneArr->tilesLayout.tileRowHeight = new uint16_t[oneArr->tilesLayout.tileRowsNum];
        if (!(oneArr->tilesLayout.tileRowHeight))
        {
            SAFE_DELETE(oneArr);
            return OMAF_ERROR_NULL_PTR;
        }
        oneArr->tilesLayout.tileColWidth = new uint16_t[oneArr->tilesLayout.tileColsNum];
        if (!(oneArr->tilesLayout.tileColWidth)) {
          delete[](oneArr->tilesLayout.tileRowHeight);
          oneArr->tilesLayout.tileRowHeight = nullptr;
          SAFE_DELETE(oneArr);
          return OMAF_ERROR_NULL_PTR;
        }

        for (uint8_t idx = 0; idx < oneArr->tilesLayout.tileRowsNum; idx++)
          oneArr->tilesLayout.tileRowHeight[idx] = existedArr[i]->tilesLayout.tileRowHeight[idx];

        for (uint8_t idx = 0; idx < oneArr->tilesLayout.tileColsNum; idx++)
          oneArr->tilesLayout.tileColWidth[idx] = existedArr[i]->tilesLayout.tileColWidth[idx];
        arr.push_back(oneArr);
      } // for each merged packet
      m_initTilesMergeArr.insert(std::make_pair(it->first, arr));
    } // for each quality
    return ERROR_NONE;
}

int32_t OmafTilesStitch::GenerateOutputMergedPackets() {
  if (m_outMergedStream.size()) {
    m_outMergedStream.clear();
  }
  // 1. generate m_updatedTilesMergeArr
  int32_t ret = GenerateTilesMergeArrangement();  // GenerateTilesMergeArrAndRwpk();
  if (ret) return ret;

  if (0 == m_mergedVideoHeaders.size()) {
    if (m_updatedTilesMergeArr.size()) {
      OMAF_LOG(LOG_ERROR, "Incorrect operation in initialization stage !\n");
      return OMAF_ERROR_OPERATION;
    }
  }

  std::map<QualityRank, vector<TilesMergeArrangement *>> tilesMergeArr;
  if (0 == m_updatedTilesMergeArr.size()) {
    tilesMergeArr = m_initTilesMergeArr;
  } else {
    tilesMergeArr = m_updatedTilesMergeArr;
  }

  bool isArrChanged = false;
  // for each quality ranking
  std::map<QualityRank, vector<TilesMergeArrangement *>>::iterator it;
  for (it = tilesMergeArr.begin(); it != tilesMergeArr.end(); it++) {
    auto qualityRanking = it->first;
    bool packetLost = false;
    bool arrangeChanged = false;
    vector<TilesMergeArrangement *> layOut = it->second;
    if (layOut.empty()) return OMAF_ERROR_NULL_PTR;
    vector<TilesMergeArrangement *> initLayOut = m_initTilesMergeArr[qualityRanking];

    // 1. check isArrChanged, packetLost and arrangeChanged flag.
    ret = IsArrChanged(qualityRanking, layOut, initLayOut, &isArrChanged, &packetLost, &arrangeChanged);
    if (ret != ERROR_NONE)
    {
        OMAF_LOG(LOG_ERROR, "error ocurrs in checking arrange changed!\n");
        return OMAF_ERROR_OPERATION;
    }

    std::map<uint32_t, MediaPacket *> packets = m_selectedTiles[qualityRanking];
    // 2. if arrangeChanged, then generate new merged video headers
    ret = GenerateMergedVideoHeaders(arrangeChanged, qualityRanking, layOut, initLayOut, packets);
    if (ret != ERROR_NONE)
    {
        OMAF_LOG(LOG_ERROR, "generate merged video headers failed! and error code is %d\n", ret);
        return OMAF_ERROR_OPERATION;
    }
    // 3. generate rwpk structure for ERP/Cubemap
    vector<std::unique_ptr<RegionWisePacking>> rwpk = GenerateMergedRWPK(qualityRanking, packetLost, arrangeChanged);
    if (rwpk.empty()) {
        OMAF_LOG(LOG_ERROR, "Failed to generate merged rwpk!\n");
        return OMAF_ERROR_GENERATE_RWPK;
    }
    // 4. init mergedData and realSize with headers
    std::map<uint32_t, MediaPacket *>::iterator itPacket;
    std::vector<uint32_t> needAccumPacketSize(layOut.size(), 0);
    for (uint32_t index = 0; index < layOut.size(); index++) {
      uint32_t width = layOut[index]->mergedWidth;
      uint32_t height = layOut[index]->mergedHeight;
      uint32_t initWidth = initLayOut.size() < index + 1 ? 0 : initLayOut[index]->mergedWidth;
      uint32_t initHeight = initLayOut.size() < index + 1 ? 0 : initLayOut[index]->mergedHeight;
      MediaPacket *mergedPacket = new MediaPacket();
      uint32_t packetSize = ((width * height * 3) / 2) / 2;
      mergedPacket->ReAllocatePacket(packetSize);
      mergedPacket->SetRwpk(std::move(rwpk[index]));
      char *mergedData = mergedPacket->Payload();
      uint64_t realSize = 0;
      if (ERROR_NONE != InitMergedDataAndRealSize(qualityRanking, packets, mergedData, &realSize, index, layOut[index])) {
          SAFE_DELETE(mergedPacket);
          OMAF_LOG(LOG_ERROR, "Failed to calculated mergedData and realSize!\n");
          return OMAF_ERROR_OPERATION;
      }
      // 5. set params for merged packets
      TilesMergeArrangement *arrange = nullptr;
      if (m_updatedTilesMergeArr.empty())
        arrange = m_initTilesMergeArr[qualityRanking][index];
      else
        arrange = m_updatedTilesMergeArr[qualityRanking][index];

      if (!arrange) {
        SAFE_DELETE(mergedPacket);
        return OMAF_ERROR_NULL_PTR;
      }

      uint8_t tileColsNum = arrange->tilesLayout.tileColsNum;
      uint8_t tileRowsNum = arrange->tilesLayout.tileRowsNum;
      uint32_t packetSizeForOneLayout = tileColsNum * tileRowsNum;
      needAccumPacketSize[index] = index == 0 ? packetSizeForOneLayout : packetSizeForOneLayout + needAccumPacketSize[index - 1];
      itPacket = packets.begin();
      if (itPacket == packets.end())
      {
        OMAF_LOG(LOG_ERROR, "Packet map is empty!\n");
        SAFE_DELETE(mergedPacket);
        return OMAF_ERROR_INVALID_DATA;
      }
      MediaPacket *firstPacket = itPacket->second;
      mergedPacket->SetVideoID(static_cast<uint32_t>(qualityRanking) - 1);
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
      mergedPacket->SetCatchupFlag(firstPacket->IsCatchup());

      if (ERROR_NONE != UpdateMergedDataAndRealSize(
                            qualityRanking, packets, tileColsNum,
                            arrangeChanged, width, height, initWidth,
                            initHeight, mergedData, &realSize, index,
                            needAccumPacketSize, layOut.size())) {
          OMAF_LOG(LOG_ERROR, "Failed to update mergedData and realSize!\n");
          SAFE_DELETE(mergedPacket);
          return OMAF_ERROR_OPERATION;
      }

      mergedPacket->SetRealSize(realSize);

      m_outMergedStream.push_back(mergedPacket);
    }
  } // each quality ranking

  if (isArrChanged) {
    if (ERROR_NONE != UpdateInitTilesMergeArr()) {
        OMAF_LOG(LOG_ERROR, "Failed to update init tiles merge arrangement!\n");
        return OMAF_ERROR_OPERATION;
    }
  }

  return ERROR_NONE;
}

std::list<MediaPacket *> OmafTilesStitch::GetTilesMergedPackets() {
  this->GenerateOutputMergedPackets();
  return m_outMergedStream;
}

VCD_OMAF_END
