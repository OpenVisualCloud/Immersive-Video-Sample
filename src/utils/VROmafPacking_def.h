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
//! \file:   VROmafPacking_def.h
//! \brief:  Common structures definition used only internally
//!          of the library
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <stdint.h>
#include "360SCVPAPI.h"

#define FACE_PX_IN_360SCVP 0
#define FACE_NX_IN_360SCVP 1
#define FACE_PY_IN_360SCVP 2
#define FACE_NY_IN_360SCVP 3
#define FACE_PZ_IN_360SCVP 4
#define FACE_NZ_IN_360SCVP 5

struct PicResolution
{
    uint16_t width;
    uint16_t height;
};

//!
//! \struct: TileInfo
//! \brief:  define basic tile information including
//!          tile position and width, height
//!
struct TileInfo
{
    uint16_t horizontalPos;
    uint16_t verticalPos;
    uint16_t tileWidth;
    uint16_t tileHeight;

    Nalu     *tileNalu;

    int32_t  projFormat;
    //this value denotes the tile horizontal position in
    //default Cube-3x2 defined in OMAF spec
    uint16_t defaultHorPos;
    //this value denotes the tile vertical position in
    //default Cube-3x2 defined in OMAF spec
    uint16_t defaultVerPos;

    //this value denotes the corresponding horizontal position
    //of the tile used in 360SCVP library for tiles selection
    int32_t  corresHorPosTo360SCVP;
    //this value denotes the corresponding vertical position
    //of the tile used in 360SCVP library for tiles selection
    int32_t  corresVerPosTo360SCVP;
    //this value denotes the corresponding face index of the
    //tile used in 360SCVP library for tiles selection
    uint8_t  corresFaceIdTo360SCVP;

    uint8_t  tileIdxInProjPic;
};

//!
//! \struct: TrackSegmentInfo
//! \brief:  define the segment information for each track,
//!          including corresponding tile index, current
//!          segment index and so on
//!
struct TrackSegmentInfo
{
    uint8_t        tileIdx;
    uint32_t       dashTrackId;
    uint32_t       isoTrackId;
    uint32_t       nbSegments;
    int32_t        segmentIndex;
    char           repId[64];
    int32_t        dependencyId;
    TileInfo       tile;
    int32_t        bitRate;
    char           segInitName[1024];
    char           segMediaName[1024];
    char           segMediaNameTmpl[1024];
    int32_t        useSourceTiming;
    int32_t        sampleCount;
    int32_t        isoCreated;
};

//!
//! \struct: VideoSegmentInfo
//! \brief:  define the segment information for each video
//!          stream, including the number of frames currently
//!          written into segments and so on
//!
struct VideoSegmentInfo {
    int32_t            totalFrames;
    int32_t            nbFrames;
    int32_t            bitRate;
    uint16_t           tilesNum;
    uint8_t            tileInRow;
    uint8_t            tileInCol;
    TrackSegmentInfo   *tracksSegInfo[1024];
    uint16_t           maxWidth;
    uint16_t           maxHeight;
    double             frameRate;

    int64_t            firstDtsInFragment;
    int32_t            framePerFragment;
    int32_t            framePerSegment;
    bool               fragmentStarted;
    bool               segmentStarted;
    uint32_t           segDur;
    uint32_t           fragDur;
    uint64_t           frameDur;
    char               outName[256];
    char               dirName[1024];
};

//!
//! \struct: SphereRegion
//! \brief:  define the content coverage information of each viewport
//!          on the sphere, including viewport index and so on
//!
struct SphereRegion
{
    uint8_t  viewIdc; //corresponding to view_idc[i] when view_idc_presence_flag is equal to 1
    int32_t  centreAzimuth;
    int32_t  centreElevation;
    int32_t  centreTilt;
    uint32_t azimuthRange;
    uint32_t elevationRange;
    bool     interpolate; // can only be 0 here
};

//!
//! \struct: ContentCoverage
//! \brief:  define the overall content coverage information for
//!          all regions
//!
struct ContentCoverage
{
    uint8_t      coverageShapeType;
    uint16_t     numRegions;
    bool         viewIdcPresenceFlag;
    uint8_t      defaultViewIdc;
    SphereRegion *sphereRegions;
};

#define DELETE_MEMORY(x) \
    if (x)               \
    {                    \
        delete x;        \
        x = NULL;        \
    }

#define DELETE_ARRAY(x)  \
    if (x)               \
    {                    \
        delete[] x;      \
        x = NULL;        \
    }

#endif /* _DEFINITIONS_H_ */
