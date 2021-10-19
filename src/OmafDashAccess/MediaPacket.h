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

//! \file:   MediaPacket.h
//! \brief:  the class for media packet
//! \detail: it's class to hold buffer for streams.
//!
//! Created on May 22, 2019, 1:18 PM
//!

#ifndef MEDIAPACKET_H_
#define MEDIAPACKET_H_

#include "../utils/ns_def.h"
#include "common.h"
#include "general.h"
#include "iso_structure.h"

#include <memory>

namespace VCD {
namespace OMAF {

class MediaPacket : public VCD::NonCopyable {
 public:
  //!
  //! \brief  construct
  //!
  MediaPacket() = default;

  //!
  //! \brief  construct
  //!
  MediaPacket(char* buf, int size) {
    if (buf && size > 0) {
      m_nAllocSize = AllocatePacket(size);
      memcpy_s(m_pPayload, size, buf, size);
    }
  };

  //!
  //! \brief  de-construct
  //!
  virtual ~MediaPacket() {
    if (nullptr != m_pPayload) {
      free(m_pPayload);
      m_pPayload = nullptr;
      m_nAllocSize = 0;
      m_type = -1;
      mPts = 0;
      m_nRealSize = 0;
      m_segID = 0;
    }
    if (m_rwpk) deleteRwpk();
  };

  MediaPacket* InsertParams(std::vector<uint8_t> params) {
    char* new_dest = nullptr;
    // FIXME align size?
    if (m_nAllocSize >= m_nRealSize + params.size()) {
      new_dest = m_pPayload;
    } else {
      void* tmp = malloc(m_nRealSize + params.size());
      new_dest = reinterpret_cast<char*>(tmp);
      m_nAllocSize = m_nRealSize + params.size();
    }

    // 1. move origin payload
    memmove(new_dest + params.size(), m_pPayload, m_nRealSize);
    memcpy_s(new_dest, m_nAllocSize - m_nRealSize, params.data(), params.size());
    m_nRealSize += params.size();

    // this is a new buffer
    if (new_dest != m_pPayload) {
      free(m_pPayload);
      m_pPayload = new_dest;
    }
    return this;
  }

  MediaPacket* InsertADTSHdr() {
    char* new_dest = nullptr;
    // FIXME align size?
    if (m_nAllocSize >= m_nRealSize + m_audioADTSHdr.size()) {
      new_dest = m_pPayload;
    } else {
      void* tmp = malloc(m_nRealSize + m_audioADTSHdr.size());
      new_dest = reinterpret_cast<char*>(tmp);
      m_nAllocSize = m_nRealSize + m_audioADTSHdr.size();
    }

    // 1. move origin payload
    memmove(new_dest + m_audioADTSHdr.size(), m_pPayload, m_nRealSize);
    memcpy_s(new_dest, m_nAllocSize - m_nRealSize, m_audioADTSHdr.data(), m_audioADTSHdr.size());
    m_nRealSize += m_audioADTSHdr.size();

    // this is a new buffer
    if (new_dest != m_pPayload) {
      free(m_pPayload);
      m_pPayload = new_dest;
    }
    return this;
  }

  //!
  //! \brief  Allocate the packet buffer, and fill the buffer with fill
  //!
  //! \param  [in] size
  //!         the buffer size to be allocated
  //! \param  [in] fill
  //!         the init value for the buffer
  //!
  //! \return
  //!         size of new allocated packet
  //!
  int AllocatePacket(int size, char fill = 0) {
    if (nullptr != m_pPayload) {
      free(m_pPayload);
      m_pPayload = nullptr;
      m_nAllocSize = 0;
    }

    m_pPayload = (char*)malloc(size);

    if (nullptr == m_pPayload) return -1;

    m_nAllocSize = size;
    memset(m_pPayload, fill, m_nAllocSize);
    m_nRealSize = 0;
    return size;
  };

  //!
  //! \brief  get the buffer pointer of the packet
  //!
  //! \return
  //!         the buffer pointer
  //!
  char* Payload() { return m_pPayload; };
  char* MovePayload() {
    char* tmp = m_pPayload;
    m_pPayload = nullptr;
    return tmp;
  }
  //!
  //! \brief  get the size of the buffer
  //!
  //! \return
  //!         size of the packet's payload
  //!
  uint64_t Size() { return m_nRealSize; };

  //!
  //! \brief  reallocate the payload buffer. if size > m_nAllocSize, keep the data
  //!         in old buf to new buf;
  //!
  //! \return
  //!         size of new allocated packet
  //!
  int ReAllocatePacket(size_t size) {
    if (nullptr == m_pPayload) return AllocatePacket(size);

    if (size < m_nAllocSize) return AllocatePacket(size);

    char* buf = m_pPayload;

    m_pPayload = (char*)malloc(size);

    if (nullptr == m_pPayload) return -1;

    memcpy_s(m_pPayload, m_nAllocSize, buf, m_nAllocSize);

    free(buf);

    m_nAllocSize = size;
    m_nRealSize = 0;
    return 0;
  };

  //!
  //! \brief  Set the type for packet
  //!
  //! \return
  //!
  //!
  void SetType(int type) { m_type = type; };

  //!
  //! \brief  Get the type of the payload
  //!
  //! \return
  //!         the type of the Packet payload
  //!
  int GetType() { return m_type; };

  uint64_t GetPTS() { return mPts; };
  void SetPTS(uint64_t pts) { mPts = pts; };

  void SetRealSize(uint64_t realSize) { m_nRealSize = realSize; };
  uint64_t GetRealSize() { return m_nRealSize; };
  // FIXME, refine and optimize
  void SetRwpk(std::unique_ptr<RegionWisePacking> rwpk) { m_rwpk = std::move(rwpk); };
  // RegionWisePacking* GetRwpk() { return m_rwpk.get(); };
  const RegionWisePacking& GetRwpk() const { return *m_rwpk.get(); };
  void copyRwpk(RegionWisePacking* to) {
    if (to && m_rwpk.get()) {
      if (to->rectRegionPacking) {
        delete[] to->rectRegionPacking;
      }
      *to = *m_rwpk.get();
      to->rectRegionPacking = new RectangularRegionWisePacking[m_rwpk->numRegions];
      memcpy_s(to->rectRegionPacking, m_rwpk->numRegions * sizeof(RectangularRegionWisePacking),
               m_rwpk->rectRegionPacking, m_rwpk->numRegions * sizeof(RectangularRegionWisePacking));
    }
  }
  void moveRwpk(RegionWisePacking* to) {
    if (to && m_rwpk.get()) {
      if (to->rectRegionPacking) {
        delete[] to->rectRegionPacking;
      }
      *to = *m_rwpk.get();
      m_rwpk->rectRegionPacking = nullptr;
    }
  }
  int GetSegID() { return m_segID; };
  void SetSegID(int id) { m_segID = id; };

  void SetQualityRanking(QualityRank qualityRanking) { m_qualityRanking = qualityRanking; };
  QualityRank GetQualityRanking() { return m_qualityRanking; };

  void SetSRDInfo(const SRDInfo& srdInfo) {
#if 0
    m_srd.left = srdInfo.left;
    m_srd.top = srdInfo.top;
    m_srd.width = srdInfo.width;
    m_srd.height = srdInfo.height;
#endif
    m_srd = srdInfo;
  };

  SRDInfo GetSRDInfo() { return m_srd; };

  void SetVideoID(uint32_t videoId) { m_videoID = videoId; };

  uint32_t GetVideoID() { return m_videoID; };

  void SetCodecType(Codec_Type codecType) { m_codecType = codecType; };

  Codec_Type GetCodecType() { return m_codecType; };

  void SetVideoWidth(int32_t videoWidth) { m_videoWidth = videoWidth; };

  int32_t GetVideoWidth() { return m_videoWidth; };

  void SetVideoHeight(int32_t videoHeight) { m_videoHeight = videoHeight; };

  int32_t GetVideoHeight() { return m_videoHeight; };

  int32_t SetQualityNum(int32_t numQty) {
    if (numQty <= 0) {
      return OMAF_ERROR_INVALID_DATA;
    }

    m_qtyResolution.resize(numQty);

    return ERROR_NONE;
  }

  int32_t GetQualityNum() { return m_qtyResolution.size(); };

  int32_t SetSourceResolution(int32_t srcId, SourceResolution resolution) {
    if (srcId >= 0 && static_cast<size_t>(srcId) < m_qtyResolution.size()) {
      m_qtyResolution[srcId] = resolution;
    } else {
      OMAF_LOG(LOG_ERROR, "Invalid source index %d !\n", srcId);
      return OMAF_ERROR_INVALID_DATA;
    }

    return ERROR_NONE;
  };

  SourceResolution* GetSourceResolutions() { return m_qtyResolution.data(); };

  void SetVideoTileRowNum(uint32_t rowNum) { m_videoTileRows = rowNum; };

  uint32_t GetVideoTileRowNum() { return m_videoTileRows; };

  void SetVideoTileColNum(uint32_t colNum) { m_videoTileCols = colNum; };

  uint32_t GetVideoTileColNum() { return m_videoTileCols; };

  void SetEOS(bool isEOS) { m_bEOS = isEOS; };

  bool GetEOS() { return m_bEOS; };

  void SetHasVideoHeader(bool hasHeader) { m_hasVideoHeader = hasHeader; };

  void SetVideoHeaderSize(uint32_t hrdSize) {
    m_hrdSize = hrdSize;
    m_hasVideoHeader = hrdSize > 0 ? true : false;
  };

  void SetVPSLen(uint32_t vpsLen) { m_VPSLen = vpsLen; };

  void SetSPSLen(uint32_t spsLen) { m_SPSLen = spsLen; };

  void SetPPSLen(uint32_t ppsLen) { m_PPSLen = ppsLen; };

  bool GetHasVideoHeader() { return m_hasVideoHeader; };

  uint32_t GetVideoHeaderSize() { return m_hrdSize; };

  uint32_t GetVPSLen() { return m_VPSLen; };

  uint32_t GetSPSLen() { return m_SPSLen; };

  uint32_t GetPPSLen() { return m_PPSLen; };

  void     SetSegmentEnded(bool isEnded) { m_segmentEnded = isEnded; };

  bool     GetSegmentEnded() { return m_segmentEnded; };

  void     SetCatchupFlag(bool isCatchup) { m_isCatchup = isCatchup; };

  bool     IsCatchup() { return m_isCatchup; };

  void     SetKeyFrame(bool isKeyFrame) { m_isKeyFrame = isKeyFrame; };

  bool     IsKeyFrame() { return m_isKeyFrame; };

  void     SetMediaType(MediaType mediaType) { m_mediaType = mediaType; };

  MediaType GetMediaType() { return m_mediaType; };

  void     SetADTSHdr(std::vector<uint8_t> audioParams)
  {
      m_audioADTSHdr = audioParams;
      printf("m_audioADTSHdr size %ld\n", m_audioADTSHdr.size());
  };

private:
    MediaPacket& operator=(const MediaPacket& other) { return *this; };
    MediaPacket(const MediaPacket& other) { /* do not create copies */ };

 private:
  char* m_pPayload = nullptr;  //!< the payload buffer of the packet
  size_t m_nAllocSize = 0;     //!< the allocated size of packet
  size_t m_nRealSize = 0;      //!< real size of packet
  int m_type = -1;             //!< the type of the payload
  uint64_t mPts = 0;
  int m_segID = 0;
  // RegionWisePacking* m_rwpk;
  std::unique_ptr<RegionWisePacking> m_rwpk;
  QualityRank m_qualityRanking = HIGHEST_QUALITY_RANKING;
  SRDInfo m_srd;

  uint32_t m_videoID = 0;
  Codec_Type m_codecType = VideoCodec_HEVC;
  int32_t m_videoWidth = 0;
  int32_t m_videoHeight = 0;
  // int32_t m_numQuality = 0;
  // SourceResolution* m_qtyResolution = nullptr_t;
  std::vector<SourceResolution> m_qtyResolution;
  uint32_t m_videoTileRows = 0;
  uint32_t m_videoTileCols = 0;
  bool m_bEOS = false;

  bool m_hasVideoHeader = false;  //!< whether the media packet includes VPS/SPS/PPS
  uint32_t m_hrdSize = 0;
  uint32_t m_VPSLen = 0;
  uint32_t m_SPSLen = 0;
  uint32_t m_PPSLen = 0;
  bool     m_segmentEnded = false;
  MediaType m_mediaType = MediaType_Video;
  std::vector<uint8_t> m_audioADTSHdr;
  bool m_isCatchup = false;
  bool m_isKeyFrame = false;

  void deleteRwpk() {
    if (m_rwpk) {
      if (m_rwpk->rectRegionPacking != nullptr) {
        delete[] m_rwpk->rectRegionPacking;
        m_rwpk->rectRegionPacking = nullptr;
      }
      m_rwpk.reset();
    }
  }
};
}  // namespace OMAF
}  // namespace VCD

#endif /* MEDIAPACKET_H */
