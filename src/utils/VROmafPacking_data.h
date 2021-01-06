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
//! \file:   VROmafPacking_data.h
//! \brief:  Common structures definition used both external
//!          and internal of the library
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _DATATYPE_H_
#define _DATATYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "360SCVPAPI.h"

//!
//! \enum:  MediaType
//! \brief: media type definitions for video, audio and so on
//!
typedef enum
{
    VIDEOTYPE = 0,
    AUDIOTYPE,
    SUBTITLETYPE,
}MediaType;

//!
//! \enum:  CodecId
//! \brief: media codec type
//!
typedef enum
{
    CODEC_ID_H264 = 0,
    CODEC_ID_H265 = 1,

    CODEC_ID_AAC  = 100,
}CodecId;

//!
//! \struct: Rational
//! \brief:  rational definition
//!
typedef struct
{
    uint64_t num;
    uint64_t den;
}Rational;

//!
//! \struct: BSBuffer
//! \brief:  bitstream information including header data and
//!          data size, meida type, codec type, bitrate and
//!          framerate
//!
typedef struct
{
    uint8_t   *data; //mandatory
    int32_t   dataSize; //mandatory
    MediaType mediaType; //mandatory

    CodecId   codecId; //mandatory
    uint64_t  bitRate; //mandatory
    Rational  frameRate; //mandatory
    uint32_t  audioObjType;
    uint32_t  sampleRate;
    uint8_t   channelNum;
}BSBuffer;

//!
//! \struct: ViewportInfo
//! \brief:  define viewport information including the width and the
//!          height of the viewport, related angle information of
//!          the viewport, the 2D projected video frame information
//!          and so on
//!
typedef struct ViewportInformation
{
    int32_t viewportWidth; //mandatory
    int32_t viewportHeight; //mandatory
    float   viewportPitch; //mandatory
    float   viewportYaw; //mandatory
    float   horizontalFOVAngle; //mandatory
    float   verticalFOVAngle; //mandatory
    int32_t outGeoType;
    int32_t inGeoType;
    int32_t inWidth;
    int32_t inHeight;
    int32_t tileInRow;
    int32_t tileInCol;
}ViewportInformation;

//!
//! \struct: SegmentationInfo
//! \brief:  define the segmentation information set by the
//!          library interface externally, including segment
//!          duration, segment files storage location and so on
//!
typedef struct SegmentationInfo
{
    int32_t       windowSize;
    int32_t       extraWindowSize;
    int32_t       needBufedFrames;
    int64_t       segDuration;      //segment duration in second
    uint8_t       extractorTracksPerSegThread;
    int32_t       removeAtExit;
    int32_t       useTemplate;
    int32_t       useTimeline;
    const char    *dirName;
    const char    *outName;
    const char    *baseUrl;
    const char    *utcTimingUrl;
    bool          isLive;
    int32_t       splitTile;
    bool          hasMainAS;
}SegmentationInfo;

//!
//! \struct: CubeMapFaceInfo
//! \brief:  define the information of one face from input
//!          Cube-3x2 which may not be the standard Cube-3x2
//!          defined in OMAF. The information includes mapped
//!          face id in standard Cube-3x2 and the transfrom
//!          type of the face
//!
typedef struct CubeMapFaceInfo
{
    uint8_t mappedStandardFaceId; //the corresponding face id in standard Cube-3x2
    E_TransformType transformType; //face transform type
}CubeMapFaceInfo;

//!
//! \struct: InputCubeMapInfo
//! \brief:  define all six faces information in input Cube-3x2
//!
typedef struct InputCubeMapInfo
{
    CubeMapFaceInfo face0MapInfo;
    CubeMapFaceInfo face1MapInfo;
    CubeMapFaceInfo face2MapInfo;
    CubeMapFaceInfo face3MapInfo;
    CubeMapFaceInfo face4MapInfo;
    CubeMapFaceInfo face5MapInfo;
}InputCubeMapInfo;

//!
//! \struct: InitialInfo
//! \brief:  define the overall initial information set by
//!          library interface externally used for media
//!          streams packing process
//!
typedef struct InitialInfo
{
    uint8_t                 bsNumVideo;              //mandatory
    uint8_t                 bsNumAudio;              //mandatory
    BSBuffer                *bsBuffers;              //mandatory

    const char              *packingPluginPath;      //needed for region-wise packing information generation if extractor track will be generated, use default plugin if no set
    const char              *packingPluginName;      //needed for region-wise packing information generation if extractor track will be generated, use default plugin if no set
    bool                    fixedPackedPicRes;       //needed to set whether all extractor tracks have the same resolution if extractor track will be generated

    const char              *videoProcessPluginPath; //needed for video stream process, use default plugin if no set
    const char              *videoProcessPluginName; //needed for video stream process, use default plugin if no set
    const char              *audioProcessPluginPath; //needed for audio stream process, mandatory if there is audio input
    const char              *audioProcessPluginName; //needed for audio stream process, mandatory if there is auido input

    ViewportInformation     *viewportInfo;           //mandatory
    SegmentationInfo        *segmentationInfo;       //mandatory

    EGeometryType           projType;                //mandatory
    InputCubeMapInfo        *cubeMapInfo;            //needed if projType is E_SVIDEO_CUBEMAP

    void                    *logFunction;            //external log callback function pointer, NULL if external log is not used
}InitialInfo;

//!
//! \struct: FrameBSInfo
//! \brief:  define information for each frame of the media
//!          stream, including frame data, data size, pts
//!          and key frame information
//!
typedef struct FrameBSInfo
{
    uint8_t  *data;
    int32_t  dataSize;
    int64_t  pts;
    bool     isKeyFrame;
}FrameBSInfo;

#ifdef __cplusplus
}
#endif

#endif /* _DATATYPE_H_ */
