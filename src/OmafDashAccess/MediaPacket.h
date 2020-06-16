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

#include "general.h"
#include "iso_structure.h"

VCD_OMAF_BEGIN

class MediaPacket {
public:
    //!
    //! \brief  construct
    //!
    MediaPacket(){
        m_pPayload = NULL;
        m_nAllocSize = 0;
        m_type = -1;
        mPts = 0;
        m_nRealSize = 0;
        m_rwpk = NULL;
        m_segID = 0;
        m_qualityRanking = HIGHEST_QUALITY_RANKING;
        memset(&m_srd, 0, sizeof(SRDInfo));
        m_videoID = 0;
        m_codecType = VideoCodec_HEVC;
        m_videoWidth = 0;
        m_videoHeight = 0;
        m_numQuality = 0;
        m_qtyResolution = NULL;
        m_videoTileRows = 0;
        m_videoTileCols = 0;
        m_bEOS = false;
        m_hasVideoHeader = false;
        m_hrdSize = 0;
        m_VPSLen = 0;
        m_SPSLen = 0;
        m_PPSLen = 0;
    };

    //!
    //! \brief  construct
    //!
    MediaPacket(char* buf, int size) : MediaPacket() {
        m_nAllocSize = AllocatePacket( size );
        memcpy(m_pPayload, buf, size);
        m_type = -1;
        mPts = 0;
        m_rwpk = NULL;
        m_segID = 0;
        m_qualityRanking = HIGHEST_QUALITY_RANKING;
        memset(&m_srd, 0, sizeof(SRDInfo));
        m_videoID = 0;
        m_codecType = VideoCodec_HEVC;
        m_videoWidth = 0;
        m_videoHeight = 0;
        m_numQuality = 0;
        m_qtyResolution = NULL;
        m_videoTileRows = 0;
        m_videoTileCols = 0;
        m_bEOS = false;
        m_hasVideoHeader = false;
        m_VPSLen = 0;
        m_SPSLen = 0;
        m_PPSLen = 0;
    };

    MediaPacket(MediaPacket& packet)
    {
        m_nAllocSize = packet.m_nAllocSize;
        m_nRealSize = packet.m_nRealSize;
        m_type = packet.m_type;
        mPts = packet.mPts;
        m_segID = packet.m_segID;
        m_rwpk = NULL;
        m_qualityRanking = packet.m_qualityRanking;
        m_srd = packet.m_srd;

        m_videoID = packet.m_videoID;
        m_codecType = packet.m_codecType;
        m_videoWidth = packet.m_videoWidth;
        m_videoHeight = packet.m_videoHeight;
        m_numQuality = packet.m_numQuality;
        m_qtyResolution = NULL;
        m_videoTileRows = packet.m_videoTileRows;
        m_videoTileCols = packet.m_videoTileCols;
        m_bEOS = packet.m_bEOS;

        m_hasVideoHeader = packet.m_hasVideoHeader;
        m_VPSLen = packet.m_VPSLen;
        m_SPSLen = packet.m_SPSLen;
        m_PPSLen = packet.m_PPSLen;

        m_pPayload = (char*)malloc(m_nRealSize);
        memcpy(m_pPayload, packet.m_pPayload, m_nRealSize);
    }

    //!
    //! \brief  de-construct
    //!
    virtual ~MediaPacket(){
        if( NULL != m_pPayload ){
            free(m_pPayload);
            m_pPayload = NULL;
            m_nAllocSize = 0;
            m_type = -1;
            mPts = 0;
            m_nRealSize = 0;
            m_segID = 0;
        }
        if (m_rwpk != NULL)
            deleteRwpk();

        if (m_qtyResolution)
        {
            delete [] m_qtyResolution;
            m_qtyResolution = NULL;
        }
    };

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
    int AllocatePacket(int size, char fill = 0){
        if( NULL != m_pPayload ){
            free(m_pPayload);
            m_pPayload = NULL;
            m_nAllocSize = 0;
        }

        m_pPayload = (char*)malloc( size );

        if(NULL == m_pPayload) return -1;

        m_nAllocSize = size;
        memset(m_pPayload, fill, m_nAllocSize );
        m_nRealSize = 0;
        return size;
    };

    //!
    //! \brief  get the buffer pointer of the packet
    //!
    //! \return
    //!         the buffer pointer
    //!
    char* Payload(){ return m_pPayload; };

    //!
    //! \brief  get the size of the buffer
    //!
    //! \return
    //!         size of the packet's payload
    //!
    uint64_t Size(){ return m_nRealSize; };

    //!
    //! \brief  reallocate the payload buffer. if size > m_nAllocSize, keep the data
    //!         in old buf to new buf;
    //!
    //! \return
    //!         size of new allocated packet
    //!
    int ReAllocatePacket(int size){
        if(NULL==m_pPayload)
            return AllocatePacket(size);

        if( size < m_nAllocSize )
            return AllocatePacket(size);

        char* buf = m_pPayload;

        m_pPayload = (char*)malloc( size );

        if(NULL == m_pPayload) return -1;

        memcpy(m_pPayload, buf, m_nAllocSize);

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
    void SetType(int type){m_type = type;};

    //!
    //! \brief  Get the type of the payload
    //!
    //! \return
    //!         the type of the Packet payload
    //!
    int GetType(){return m_type;};

    uint64_t GetPTS() { return mPts; };
    void SetPTS(uint64_t pts){ mPts = pts; };

    void SetRealSize(uint64_t realSize) { m_nRealSize = realSize; };
    uint64_t GetRealSize() { return m_nRealSize; };

    void SetRwpk(RegionWisePacking *rwpk) { m_rwpk = rwpk; };
    RegionWisePacking* GetRwpk() { return m_rwpk; };

    int GetSegID() { return m_segID; };
    void SetSegID(int id){ m_segID = id; };

    void SetQualityRanking(uint32_t qualityRanking) { m_qualityRanking = qualityRanking; };
    uint32_t GetQualityRanking() { return m_qualityRanking; };

    void SetSRDInfo(SRDInfo srdInfo)
    {
        m_srd.left   = srdInfo.left;
        m_srd.top    = srdInfo.top;
        m_srd.width  = srdInfo.width;
        m_srd.height = srdInfo.height;
    };

    SRDInfo GetSRDInfo() { return m_srd; };

    void     SetVideoID(uint32_t videoId) { m_videoID = videoId; };

    uint32_t GetVideoID() { return m_videoID; };

    void     SetCodecType(Codec_Type codecType) { m_codecType = codecType; };

    Codec_Type GetCodecType() { return m_codecType; };

    void     SetVideoWidth(int32_t videoWidth) { m_videoWidth = videoWidth; };

    int32_t  GetVideoWidth() { return m_videoWidth; };

    void     SetVideoHeight(int32_t videoHeight) { m_videoHeight = videoHeight; };

    int32_t  GetVideoHeight() { return m_videoHeight; };

    int32_t  SetQualityNum(int32_t numQty)
    {
        m_numQuality = numQty;

        if (m_qtyResolution)
        {
            delete [] m_qtyResolution;
            m_qtyResolution = NULL;
        }

        m_qtyResolution = new SourceResolution[m_numQuality];
        if (!m_qtyResolution)
            return OMAF_ERROR_NULL_PTR;

        return ERROR_NONE;
    }

    int32_t  GetQualityNum() { return m_numQuality; };

    int32_t  SetSourceResolution(int32_t srcId, SourceResolution resolution)
    {
        if (!m_qtyResolution)
        {
            LOG(ERROR) << "NULL quality resolution !" << std::endl;
            return OMAF_ERROR_NULL_PTR;
        }

        if ((srcId < 0) || (srcId > (m_numQuality - 1)))
        {
            LOG(ERROR) << "Invalid source index  " << srcId << " !" << std::endl;
            return OMAF_ERROR_INVALID_DATA;
        }

        m_qtyResolution[srcId].qualityRanking = resolution.qualityRanking;
        m_qtyResolution[srcId].top            = resolution.top;
        m_qtyResolution[srcId].left           = resolution.left;
        m_qtyResolution[srcId].width          = resolution.width;
        m_qtyResolution[srcId].height         = resolution.height;

        return ERROR_NONE;
    };

    SourceResolution* GetSourceResolutions() { return m_qtyResolution; };

    void     SetVideoTileRowNum(uint32_t rowNum) { m_videoTileRows = rowNum; };

    uint32_t GetVideoTileRowNum() { return m_videoTileRows; };

    void     SetVideoTileColNum(uint32_t colNum) { m_videoTileCols = colNum; };

    uint32_t GetVideoTileColNum() { return m_videoTileCols; };

    void     SetEOS(bool isEOS) { m_bEOS = isEOS; };

    bool     GetEOS() { return m_bEOS; };

    void     SetHasVideoHeader(bool hasHeader) { m_hasVideoHeader = hasHeader; };

    void     SetVideoHeaderSize(uint32_t hrdSize) { m_hrdSize = hrdSize; };

    void     SetVPSLen(uint32_t vpsLen) { m_VPSLen = vpsLen; };

    void     SetSPSLen(uint32_t spsLen) { m_SPSLen = spsLen; };

    void     SetPPSLen(uint32_t ppsLen) { m_PPSLen = ppsLen; };

    bool     GetHasVideoHeader() { return m_hasVideoHeader; };

    uint32_t GetVideoHeaderSize() { return m_hrdSize; };

    uint32_t GetVPSLen() { return m_VPSLen; };

    uint32_t GetSPSLen() { return m_SPSLen; };

    uint32_t GetPPSLen() { return m_PPSLen; };

private:
    char* m_pPayload;                    //!<the payload buffer of the packet
    int   m_nAllocSize;                  //!<the allocated size of packet
    uint64_t m_nRealSize;                //!< real size of packet
    int   m_type;                        //!<the type of the payload
    uint64_t mPts;
    int   m_segID;
    RegionWisePacking *m_rwpk;
    uint32_t m_qualityRanking;
    SRDInfo m_srd;

    uint32_t   m_videoID;
    Codec_Type m_codecType;
    int32_t    m_videoWidth;
    int32_t    m_videoHeight;
    int32_t    m_numQuality;
    SourceResolution *m_qtyResolution;
    uint32_t   m_videoTileRows;
    uint32_t   m_videoTileCols;
    bool       m_bEOS;

    bool       m_hasVideoHeader;          //!< whether the media packet includes VPS/SPS/PPS
    uint32_t   m_hrdSize;
    uint32_t   m_VPSLen;
    uint32_t   m_SPSLen;
    uint32_t   m_PPSLen;

    void deleteRwpk()
    {
        if (m_rwpk != NULL)
        {
            if (m_rwpk->rectRegionPacking != NULL)
            {
                delete []m_rwpk->rectRegionPacking;
                m_rwpk->rectRegionPacking = NULL;
            }
            delete m_rwpk;
            m_rwpk = NULL;
        }
    }
};

VCD_OMAF_END;

#endif /* MEDIAPACKET_H */

