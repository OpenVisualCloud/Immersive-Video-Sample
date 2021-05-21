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
//! \file:   VideoSegmentInfoGenerator.h
//! \brief:  Video segment information generator class definition
//! \detail: Define the initializaiton and update process for video
//!          segment information.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _VIDEOSEGMENTINFOGENERATOR_H_
#define _VIDEOSEGMENTINFOGENERATOR_H_

#include "VROmafPacking_data.h"
#include "VROmafPacking_def.h"
#include "../../../../utils/safe_mem.h"

//!
//! \class VideoSegmentInfoGenerator
//! \brief Define the initializaiton and update process for video
//!        segment information, called by VideoStream.
//!

class VideoSegmentInfoGenerator
{
public:
    //!
    //! \brief  Constructor
    //!
    VideoSegmentInfoGenerator();

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] bs
    //!         pointer to the BSBuffer information of
    //!         the video stream, including header data
    //!         and bitrate, framerate and so on.
    //! \param  [in] initInfo
    //!         initial information input by library interface
    //!         which includs segmentation information
    //! \param  [in] streamIdx
    //!         the index of the video stream
    //! \param  [in] videoWidth
    //!         the width of the video frame
    //! \param  [in] videoHeight
    //!         the height of the video frame
    //! \param  [in] tileInRow
    //!         tiles number in row in video frame
    //! \param  [in] tileInCol
    //!         tiles number in column in video frame
    //!
    VideoSegmentInfoGenerator(
        BSBuffer *bs,
        InitialInfo *initInfo,
        uint8_t streamIdx,
        uint16_t videoWidth,
        uint16_t videoHeight,
        uint8_t tileInRow,
        uint8_t tileInCol);


    VideoSegmentInfoGenerator(const VideoSegmentInfoGenerator& src);

    VideoSegmentInfoGenerator& operator=(const VideoSegmentInfoGenerator&) = default;

    //!
    //! \brief  Destructor
    //!
    ~VideoSegmentInfoGenerator();

    //!
    //! \brief  Initialize the video segment information
    //!         according to video bitstream information
    //!         and overall segmentation information
    //!
    //! \param  [in] tilesInfo
    //!         the pointer to tile information of all
    //!         tiles in the video stream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize(TileInfo *tilesInfo);

    //!
    //! \brief  Update the video segment information
    //!         during the segmentation process
    //!
    //! \return void
    //!
    void Update();

    //!
    //! \brief  Get the video segment information
    //!
    //! \return VideoSegmentInfo*
    //!         the pointer to the video segment information
    //!
    VideoSegmentInfo* GetVideoSegInfo() { return m_videoSegInfo; };

private:
    //!
    //! \brief  Initialize the segment information for the first
    //!         track, which isn't corresponding to any tile and
    //!         used to generate the initial segment
    //!
    //! \param  [in] trackSegInfo
    //!         pointer to the segment information for the first
    //!         track
    //! \return void
    //!
    void InitFirstTrackInfo(TrackSegmentInfo *trackSegInfo);

    //!
    //! \brief  Initialize the segment information for the
    //!         specified tile
    //!
    //! \param  [in] trackSegInfo
    //!         pointer to the segment information for the
    //!         specified tile
    //! \param  [in] tileInfo
    //!         pointer to the tile information corresponding
    //!         to the specified tile
    //!
    //! \return void
    //!
    void InitTileTrackSegInfo(TrackSegmentInfo *trackSegInfo, TileInfo *tileInfo);

    //!
    //! \brief  Update the segment information for the
    //!         specified track during the segmentation
    //!         process
    //!
    //! \param  [in] trackSegInfo
    //!         pointer to the segment information for the
    //!         specified track
    //! \return void
    //!
    void UpdateTrackSegInfo(TrackSegmentInfo *trackSegInfo);
private:
    BSBuffer         *m_bsBuffer;         //!< pointer to the BSBuffer information
    SegmentationInfo *m_segmentationInfo; //!< pointer to the overall segmentation information
    uint8_t          m_streamIdx;         //!< the video stream index
    VideoSegmentInfo *m_videoSegInfo;     //!< pointer to the video segment information
};

#endif /* _VIDEOSEGMENTINFOGENERATOR_H_ */
