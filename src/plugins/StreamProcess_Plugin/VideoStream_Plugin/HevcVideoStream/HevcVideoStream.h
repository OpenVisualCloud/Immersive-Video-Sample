/*
 * Copyright (c) 2020, Intel Corporation
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
//! \file:   HevcVideoStream.h
//! \brief:  HEVC Video stream process class definition
//! \detail: Define the data and data process for HEVC video stream
//!
//! Created on November 6, 2020, 6:04 AM
//!

#ifndef _HEVCVIDEOSTREAM_H_
#define _HEVCVIDEOSTREAM_H_

#include "../VideoStreamPluginAPI.h"
#include "VideoSegmentInfoGenerator.h"
#include "HevcNaluParser.h"
#include "../../../../utils/safe_mem.h"

//!
//! \class HevcVideoStream
//! \brief Define the data and data operation for HEVC video stream
//!

class HevcVideoStream : public VideoStream
{
public:
    //!
    //! \brief  Constructor
    //!
    HevcVideoStream();

    HevcVideoStream(const HevcVideoStream& src);

    HevcVideoStream& operator=(HevcVideoStream&& other);

    //!
    //! \brief  Destructor
    //!
    virtual ~HevcVideoStream();

    //!
    //! \brief  Initialize the video stream
    //!
    //! \param  [in] streamIdx
    //!         the index of the video in all streams
    //! \param  [in] bs
    //!         pointer to the BSBuffer information of
    //!         the video stream, including header data
    //!         and bitrate, framerate and so on.
    //! \param  [in] initInfo
    //!         pointer to the initial information input
    //!         by the library interface
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize(uint8_t streamIdx, BSBuffer *bs, InitialInfo *initInfo);

    //!
    //! \brief  Get the width of the video frame
    //!
    //! \return uint16_t
    //!         the width of the video frame
    //!
    uint16_t GetSrcWidth() { return m_width; };

    //!
    //! \brief  Get the height of the video frame
    //!
    //! \return uint16_t
    //!         the height of the video frame
    //!
    uint16_t GetSrcHeight() { return m_height; };

    //!
    //! \brief  Get gop size of the video stream
    //!
    //! \return uint64_t
    //!         the gop size of the video stream
    //!
    uint32_t GetGopSize() { return m_gopSize; };

    //!
    //! \brief  Get the tiles number in row in video frame
    //!
    //! \return uint8_t
    //!         the tiles number in row in video frame
    //!
    uint8_t GetTileInRow() { return m_tileInRow; };

    //!
    //! \brief  Get the tiles number in column in video frame
    //!
    //! \return uint8_t
    //!         the tiles number in column in video frame
    //!
    uint8_t GetTileInCol() { return m_tileInCol; };

    //!
    //! \brief  Get the region wise packing information of the video
    //!
    //! \return RegionWisePacking*
    //!         the pointer to the region wise packing information
    //!
    RegionWisePacking* GetSrcRwpk() { return m_srcRwpk; };

    //!
    //! \brief  Get the content coverage information of the video
    //!
    //! \return ContentCoverage*
    //!         the pointer to the content coverage information
    //!
    ContentCoverage* GetSrcCovi() { return m_srcCovi; };

    //!
    //! \brief  Get the projection type information of the video
    //!
    //! \return uint16_t
    //!         0 is ERP, and 1 is CubeMap
    //!
    VCD::OMAF::ProjectionFormat GetProjType() { return m_projType; };

    //!
    //! \brief  Get the video segment information of the video
    //!
    //! \return VideoSegmentInfo*
    //!         the pointer to the video segment information
    //!
    VideoSegmentInfo* GetVideoSegInfo()
    {
        return m_videoSegInfoGen->GetVideoSegInfo();
    };

    //!
    //! \brief  Update the video segment information of the video
    //!
    //! \return void
    //!
    void UpdateVideoSegInfo()
    {
        return m_videoSegInfoGen->Update();
    };

    //!
    //! \brief  End the operation on the video stream
    //!
    //! \return void
    //!
    //void EndStream();

    //!
    //! \brief  Add frame information for a new frame into
    //!         frame information list of the video
    //!
    //! \param  [in] frameInfo
    //!         pointer to the frame information of the new frame
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t AddFrameInfo(FrameBSInfo *frameInfo);

    //!
    //! \brief  Fetch the front frame information in frame
    //!         information list as current frame information
    //!
    //! \return void
    //!
    void SetCurrFrameInfo();

    //!
    //! \brief  Update tile nalu information according to
    //!         current frame bitstream data
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t UpdateTilesNalu();

    //!
    //! \brief  Get all tiles information
    //!
    //! \return TileInfo*
    //!         the pointer to the tile information of all tiles
    //!
    TileInfo* GetAllTilesInfo();

    //!
    //! \brief  Get the current frame information
    //!
    //! \return FrameBSInfo*
    //!         the pointer to the current frame information
    //!
    FrameBSInfo* GetCurrFrameInfo();

    //!
    //! \brief  Destroy current frame information
    //!
    //! \return void
    //!
    void DestroyCurrFrameInfo();

    //!
    //! \brief  Destroy all frame information belong to current
    //!         segment
    //!
    //! \return void
    //!
    void DestroyCurrSegmentFrames();

    //!
    //! \brief  Get the 360SCVP library handle
    //!
    //! \return void*
    //!         the 360SCVP library handle
    //!
    void* Get360SCVPHandle() { return m_360scvpHandle; };

    //!
    //! \brief  Get the 360SCVP library parameter
    //!
    //! \return param_360SCVP*
    //!         the pointer to 360SCVP library parameter
    //!
    param_360SCVP* Get360SCVPParam() { return m_360scvpParam; };

    //!
    //! \brief  Get the VPS nalu information
    //!
    //! \return Nalu*
    //!         the pointer to the VPS nalu information
    //!
    Nalu* GetVPSNalu();

    //!
    //! \brief  Get the SPS nalu information
    //!
    //! \return Nalu*
    //!         the pointer to SPS nalu information
    //!
    Nalu* GetSPSNalu();

    //!
    //! \brief  Get the PPS nalu information
    //!
    //! \return Nalu*
    //!         the pointer to PPS nalu information
    //!
    Nalu* GetPPSNalu();

    //!
    //! \brief  Get the frame rate
    //!
    //! \return Rational
    //!         the frame rate of the video stream
    //!
    Rational GetFrameRate() { return m_frameRate; };

    //!
    //! \brief  Get the bit rate
    //!
    //! \return uint64_t
    //!         the bit rate of the video stream
    //!
    uint64_t GetBitRate() { return m_bitRate; };

    //!
    //! \brief  Set the EOS status for the video stream
    //!
    //! \param  [in] isEOS
    //!         the status will be set to EOS of the video stream
    //!
    //! \return void
    //!
    void SetEOS(bool isEOS) { m_isEOS = isEOS; };

    //!
    //! \brief  Get the EOS status of the video stream
    //!
    //! \return bool
    //!         the EOS status of the video stream
    //!
    bool GetEOS() { return m_isEOS; };

    //!
    //! \brief  Add current frame to frames list for current
    //!         segment
    //!
    //! \return void
    //!
    void AddFrameToSegment()
    {
        m_framesToOneSeg.push_back(m_currFrameInfo);
        m_currFrameInfo = NULL;
    };

    //!
    //! \brief  Get current buffered frames number in
    //!         frame list which are not been written
    //!         into segments
    //!
    //! \return uint32_t
    //!         current buffered frames number
    //!
    uint32_t GetBufferedFrameNum()
    {
        uint32_t bufferedFrmNum = 0;
        std::lock_guard<std::mutex> lock(m_mutex);
        bufferedFrmNum = m_frameInfoList.size();
        return bufferedFrmNum;
    }

private:
    //!
    //! \brief  Parse the header data of the video stream,
    //!         including SPS, PPS, ProjectionTypeSei,
    //!         RegionWisePackingSei and ViewportSei.
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ParseHeader();

    //!
    //! \brief  Fill source region wise packing information
    //!         according to tiles information for ERP
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t FillRegionWisePackingForERP();

    //!
    //! \brief  Fill source region wise packing information
    //!         according to tiles information for CubeMap
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t FillRegionWisePackingForCubeMap();

    //!
    //! \brief  Fill source content coverage information
    //!         according to region wise packing information
    //!         for ERP
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t FillContentCoverageForERP();

private:
    uint8_t                   m_streamIdx;        //!< the index of the video in all media streams
    CodecId                   m_codecId;          //!< codec type for the video, CODEC_ID_H264 or CODEC_ID_H265
    uint16_t                  m_width;            //!< width of the video frame
    uint16_t                  m_height;           //!< height of the video frame
    uint8_t                   m_tileInRow;        //!< tiles number in row in video frame
    uint8_t                   m_tileInCol;        //!< tiles number in column in video frame
    uint8_t                   m_tileRowsInFace;   //!< tile rows number in each CubeMap face
    uint8_t                   m_tileColsInFace;   //!< tile cols number in each CubeMap face
    TileInfo                  *m_tilesInfo;       //!< pointer to tile information of all tiles
    VCD::OMAF::ProjectionFormat m_projType;       //!< projection type of the video frame
    CubeMapFaceInfo           m_cubeMapInfo[CUBEMAP_FACES_NUM]; //!< pointer to the input CubeMap information
    RegionWisePacking         *m_srcRwpk;         //!< pointer to the region wise packing information of the video
    ContentCoverage           *m_srcCovi;         //!< pointer to the content coverage information of the video
    VideoSegmentInfoGenerator *m_videoSegInfoGen; //!< pointer to the video segment information generator
    std::list<FrameBSInfo*>   m_frameInfoList;    //!< frame information list of the video
    std::list<FrameBSInfo*>   m_framesToOneSeg;   //!< frames will be written into one segment
    FrameBSInfo               *m_currFrameInfo;   //!< pointer to the current frame information
    param_360SCVP             *m_360scvpParam;    //!< 360SCVP library initial parameter
    void                      *m_360scvpHandle;   //!< 360SCVP library handle
    NaluParser                *m_naluParser;      //!< NALU parser to parse the header data of the video
    Rational                  m_frameRate;        //!< the frame rate of the video stream
    uint64_t                  m_bitRate;          //!< the bit rate of the video stream
    bool                      m_isEOS;            //!< the EOS status of the video stream
    std::mutex                m_mutex;            //!< thread mutex for frame information list
    uint32_t                  m_gopSize;          //!< gop size of the video stream
    uint64_t                  m_lastKeyFramePTS;  //!< last key frame pts of the video stream
};

extern "C" VideoStream* Create();
extern "C" void Destroy(VideoStream* vs);

#endif /* _HEVCVIDEOSTREAM_H_ */
