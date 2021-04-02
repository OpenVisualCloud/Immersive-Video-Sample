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
//! \file:  VideoStreamPluginAPI.h
//! \brief: Video stream process plugin interfaces
//!
//! Created on November 6, 2020, 6:04 AM
//!

#ifndef _VIDEOSTREAMPLUGINAPI_H_
#define _VIDEOSTREAMPLUGINAPI_H_

#include "MediaStream.h"
#include "VROmafPacking_def.h"
#include "OmafStructure.h"


#include <list>
#include <mutex>

#define CUBEMAP_FACES_NUM 6

//!
//! \class VideoStream
//! \brief Define the interface class for video stream process plugin
//!

class VideoStream : public MediaStream
{
public:
    //!
    //! \brief  Constructor
    //!
    VideoStream() {};

    //!
    //! \brief  Destructor
    //!
    virtual ~VideoStream() {};

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
    virtual int32_t Initialize(uint8_t streamIdx, BSBuffer *bs, InitialInfo *initInfo) = 0;

    //!
    //! \brief  Get the width of the video frame
    //!
    //! \return uint16_t
    //!         the width of the video frame
    //!
    virtual uint16_t GetSrcWidth() = 0;

    //!
    //! \brief  Get the height of the video frame
    //!
    //! \return uint16_t
    //!         the height of the video frame
    //!
    virtual uint16_t GetSrcHeight() = 0;

    //!
    //! \brief  Get gop size of the video stream
    //!
    //! \return uint64_t
    //!         the gop size of the video stream
    //!
    virtual uint32_t GetGopSize() = 0;

    //!
    //! \brief  Get the tiles number in row in video frame
    //!
    //! \return uint8_t
    //!         the tiles number in row in video frame
    //!
    virtual uint8_t GetTileInRow() = 0;

    //!
    //! \brief  Get the tiles number in column in video frame
    //!
    //! \return uint8_t
    //!         the tiles number in column in video frame
    //!
    virtual uint8_t GetTileInCol() = 0;

    //!
    //! \brief  Get the region wise packing information of the video
    //!
    //! \return RegionWisePacking*
    //!         the pointer to the region wise packing information
    //!
    virtual RegionWisePacking* GetSrcRwpk() = 0;

    //!
    //! \brief  Get the content coverage information of the video
    //!
    //! \return ContentCoverage*
    //!         the pointer to the content coverage information
    //!
    virtual ContentCoverage* GetSrcCovi() = 0;

    //!
    //! \brief  Get the projection type information of the video
    //!
    //! \return uint16_t
    //!         0 is ERP, and 1 is CubeMap
    //!
    virtual VCD::OMAF::ProjectionFormat GetProjType() = 0;

    //!
    //! \brief  Get the video segment information of the video
    //!
    //! \return VideoSegmentInfo*
    //!         the pointer to the video segment information
    //!
    virtual VideoSegmentInfo* GetVideoSegInfo() = 0;

    //!
    //! \brief  Update the video segment information of the video
    //!
    //! \return void
    //!
    virtual void UpdateVideoSegInfo() = 0;

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
    virtual int32_t AddFrameInfo(FrameBSInfo *frameInfo) = 0;

    //!
    //! \brief  Fetch the front frame information in frame
    //!         information list as current frame information
    //!
    //! \return void
    //!
    virtual void SetCurrFrameInfo() = 0;

    //!
    //! \brief  Update tile nalu information according to
    //!         current frame bitstream data
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t UpdateTilesNalu() = 0;

    //!
    //! \brief  Get all tiles information
    //!
    //! \return TileInfo*
    //!         the pointer to the tile information of all tiles
    //!
    virtual TileInfo* GetAllTilesInfo() = 0;

    //!
    //! \brief  Get the current frame information
    //!
    //! \return FrameBSInfo*
    //!         the pointer to the current frame information
    //!
    virtual FrameBSInfo* GetCurrFrameInfo() = 0;

    //!
    //! \brief  Destroy current frame information
    //!
    //! \return void
    //!
    virtual void DestroyCurrFrameInfo() = 0;

    //!
    //! \brief  Destroy all frame information belong to current
    //!         segment
    //!
    //! \return void
    //!
    virtual void DestroyCurrSegmentFrames() = 0;

    //!
    //! \brief  Get the 360SCVP library handle
    //!
    //! \return void*
    //!         the 360SCVP library handle
    //!
    virtual void* Get360SCVPHandle() = 0;

    //!
    //! \brief  Get the 360SCVP library parameter
    //!
    //! \return param_360SCVP*
    //!         the pointer to 360SCVP library parameter
    //!
    virtual param_360SCVP* Get360SCVPParam() = 0;

    //!
    //! \brief  Get the VPS nalu information
    //!
    //! \return Nalu*
    //!         the pointer to the VPS nalu information
    //!
    virtual Nalu* GetVPSNalu() = 0;

    //!
    //! \brief  Get the SPS nalu information
    //!
    //! \return Nalu*
    //!         the pointer to SPS nalu information
    //!
    virtual Nalu* GetSPSNalu() = 0;

    //!
    //! \brief  Get the PPS nalu information
    //!
    //! \return Nalu*
    //!         the pointer to PPS nalu information
    //!
    virtual Nalu* GetPPSNalu() = 0;

    //!
    //! \brief  Get the frame rate
    //!
    //! \return Rational
    //!         the frame rate of the video stream
    //!
    virtual Rational GetFrameRate() = 0;

    //!
    //! \brief  Get the bit rate
    //!
    //! \return uint64_t
    //!         the bit rate of the video stream
    //!
    virtual uint64_t GetBitRate() = 0;

    //!
    //! \brief  Set the EOS status for the video stream
    //!
    //! \param  [in] isEOS
    //!         the status will be set to EOS of the video stream
    //!
    //! \return void
    //!
    virtual void SetEOS(bool isEOS) = 0;

    //!
    //! \brief  Get the EOS status of the video stream
    //!
    //! \return bool
    //!         the EOS status of the video stream
    //!
    virtual bool GetEOS() = 0;

    //!
    //! \brief  Add current frame to frames list for current
    //!         segment
    //!
    //! \return void
    //!
    virtual void AddFrameToSegment() = 0;

    //!
    //! \brief  Get current buffered frames number in
    //!         frame list which are not been written
    //!         into segments
    //!
    //! \return uint32_t
    //!         current buffered frames number
    //!
    virtual uint32_t GetBufferedFrameNum() = 0;
protected:
};

typedef VideoStream* CreateVideoStream();
typedef void DestroyVideoStream(VideoStream*);

#endif /* _VIDEOSTREAMPLUGINAPI_H_ */
