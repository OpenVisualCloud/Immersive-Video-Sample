/*
 * Copyright (c) 2018, Intel Corporation
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
#ifndef _360SCVP_API_H_
#define _360SCVP_API_H_
#include "stdint.h"
#include <stdbool.h>
#include "pose.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ID_SCVP_PARAM_PICINFO              1000
#define ID_SCVP_PARAM_VIEWPORT             1001
#define ID_SCVP_PARAM_SEI_PROJECTION       1002
#define ID_SCVP_PARAM_SEI_RWPK             1003
#define ID_SCVP_PARAM_SEI_ROTATION         1004
#define ID_SCVP_PARAM_SEI_FRAMEPACKING     1005
#define ID_SCVP_PARAM_SEI_VIEWPORT         1006
#define ID_SCVP_BITSTREAMS_HEADER          1007
#define ID_SCVP_RWPK_INFO                  1008
#define DEFAULT_REGION_NUM                 1000

/*!
 *
 * The enum type is defined followed with OMAF cubemap face ID order.
 *
 */
typedef enum OMAF_CUBE_FACE
{
    OMAF_FACE_PY = 0,
    OMAF_FACE_PX,
    OMAF_FACE_NY,
    OMAF_FACE_NZ,
    OMAF_FACE_NX,
    OMAF_FACE_PZ,
}OmafCubeFace;

typedef enum SliceType {
    E_SLICE_B   = 0,
    E_SLICE_P   = 1,
    E_SLICE_I   = 2,
    E_SLICE_IDR = 3,
}SliceType;

/*!
 *
 *  The enum type support both sphere projection and planar.
 *  The planar video, like pan-zoom usage, uses E_VIDEO_PLANAR
 *
 */
typedef enum EGeometryType
{
    E_SVIDEO_EQUIRECT = 0,
    E_SVIDEO_CUBEMAP,
    E_SVIDEO_VIEWPORT,
    E_SVIDEO_PLANAR,
    E_SVIDEO_TYPE_NUM,
}EGeometryType;

/*!
*
*  currently the library can support three types
*  0(only merge), similar to our before usage
*  1(merge + viewport), used in webrtc use case
*  2(parsing one nal), used in the omaf
*  3(parsing for client), used in the client player
*  4(only viewport calculation), can be used in OMAF DASH access
*/
typedef enum UsageType
{
    E_STREAM_STITCH_ONLY = 0,
    E_MERGE_AND_VIEWPORT,
    E_PARSER_ONENAL,
    E_PARSER_FOR_CLIENT,
    E_VIEWPORT_ONLY,
    E_PARSER_TYPE_NUM,
}UsageType;

/*!
*
*  currently the library can support five SEI types,
*  details please refer to the JCTVC-AC1005-v2.pdf
*
*  E_FRAMEPACKING_ARRANGEMENT, its value is 45
*  E_EQUIRECT_PROJECTION, its value is 150
*  E_CUBEMAP_PROJECTION, its value is 151
*  E_SPHERE_ROTATION, its value is 152
*  E_REGIONWISE_PACKING, its value is 153
*
*/
typedef enum H265SEIType
{
    E_FRAMEPACKING_ARRANGEMENT = 45,
    E_EQUIRECT_PROJECTION = 150,
    E_CUBEMAP_PROJECTION,
    E_SPHERE_ROTATION = 154,
    E_REGIONWISE_PACKING,
    E_OMNI_VIEWPORT
}H265SEIType;

/*！
 * Plugin Type definitions:
 *
 * E_PLUGIN_TILE_SELECTION: For plugins which implemented tile selection
 *
 */
typedef enum PluginType
{
    E_PLUGIN_TILE_SELECTION = 0,
    E_PLUGIN_TYPE_NUM,
}PluginType;

/*!
 * Plugin Format definitions:
 *
 * E_PLUGIN_EQUIRECT: For plugins which is used on ERP projection
 * E_PLUGIN_CUBEMAP: For plugins which is used on Cubemap projection
 * E_PLUGIN_PLANAR: For plugins which is used on Planar projection
 *
 */
typedef enum PluginFormat
{
    E_PLUGIN_EQUIRECT = 0,
    E_PLUGIN_CUBEMAP,
    E_PLUGIN_PLANAR,
    E_PLUGIN_FORMAT_NUM
}PluginFormat;

/*!
 * Plugin Defintion for 360SCVP
 *
 * PluginType: Tile selection, etc.
 * PluginFormat: Indicate the video projection: Equirect, cubemap, panzoom, etc.
 * PluginLibPath: Plugin library file full name
 *
 */
typedef struct PLUGIN_DEF
{
    PluginType      pluginType;
    PluginFormat    pluginFormat;
    char*           pluginLibPath;
}PluginDef;

/*!
 *
 *  The structure support both sphere projection and planar video
 *  x: The top left X-axis coordinate of the tile
 *  y: The top left Y-axis coordinate of the tile
 *  idx: The tile index.
 *  faceId: Which projection face the tile locates. Only used for 3D projections.
 *  streamId: Which stream the tile locates. Only used for 2D projection.
 *  Offset:
 *      The four offsets provide the viewport area points distance to the current tile points.
 *      Users can write their own implementation to utilize these information for render module
 *
 */
typedef struct TILE_DEF
{
    int32_t x;
    int32_t y;
    int32_t idx;
    int32_t faceId;
    int32_t streamId;       //for planar tile selection only
    int32_t upLeftXOffset;
    int32_t upLeftYOffset;
    int32_t downRightXOffset;
    int32_t downRightYOffset;
} TileDef;

typedef struct CC_DEF
{
    int32_t centreAzimuth;
    int32_t centreElevation;
    uint32_t azimuthRange;
    uint32_t elevationRange;
}CCDef;

typedef struct Nalu
{
    uint8_t  *data;  //pointer to nalu
    int32_t  dataSize; //total bytes number of the nalu
    uint8_t  startCodesSize; //nalu start codes size
    uint16_t naluType; //nalu type

    uint16_t seiPayloadType; //SEI payload type if nalu is for SEI
    uint16_t sliceHeaderLen; //slice header length if nalu is for slice
}Nalu;

//!
//! \struct: RectangularRegionWisePacking
//! \brief:  define rectangular region wise packing information
//!          for each region, including its source position and size,
//!          its destination position and size, and so on
//!
typedef struct RECTANGUALAR_REGION_WIZE_PACKING
{
    uint8_t  transformType;
    bool     guardBandFlag;
    uint32_t projRegWidth;
    uint32_t projRegHeight;
    uint32_t projRegTop;
    uint32_t projRegLeft;

    uint16_t packedRegWidth;
    uint16_t packedRegHeight;
    uint16_t packedRegTop;
    uint16_t packedRegLeft;

    //below fields take effect when .guardBandFlag. is true
    uint8_t leftGbWidth;
    uint8_t rightGbWidth;
    uint8_t topGbHeight;
    uint8_t bottomGbHeight;
    bool    gbNotUsedForPredFlag;
    uint8_t gbType0;
    uint8_t gbType1;
    uint8_t gbType2;
    uint8_t gbType3;
}RectangularRegionWisePacking;

//!
//! \struct: RegionWisePacking
//! \brief:  define the overall region wise packing information
//!          of each video stream, including regions number, detailed
//!          region wise packing information for each region, and so on
//! add three variables(high resolution region number, low resolution stream width and height) to support WeRTC sample player
//! add timestamp in SEI to track frame
//!
typedef struct REGION_WIZE_PACKING
{
    bool     constituentPicMatching;
    uint8_t  numRegions;
    uint32_t projPicWidth;
    uint32_t projPicHeight;
    uint16_t packedPicWidth;
    uint16_t packedPicHeight;
    RectangularRegionWisePacking *rectRegionPacking;
    uint8_t  numHiRegions;
    uint32_t lowResPicWidth;
    uint32_t lowResPicHeight;
    uint32_t timeStamp;
}RegionWisePacking;

typedef struct VIEW_PORT
{
    int32_t  AzimuthCentre;
    int32_t  ElevationCentre;
    int32_t  tiltCentre;
    uint32_t HorzRange;
    uint32_t VertRange;
}oneViewport;

typedef struct OMNI_VIEW_PORT
{
    uint32_t vpId;
    uint16_t viewportsSize;
    oneViewport* pViewports;
}OMNIViewPort;

//!
//! \struct: SphereRotation
//! \brief:   provides information on rotation angles yaw (a), pitch , and roll
//!           that are used for conversion between the global coordinate axes and the local coordinate axes.
//!           yaw expresses a rotation around the z (vertical, up) axis,
//!           pitch rotates around the y (lateral, side-to-side) axis,
//!           and roll rotates around the x (back-to-front) axis
//!
typedef struct SPHERE_ROTATION
{
    int32_t yawRotation;
    int32_t pitchRotation;
    int32_t rollRotation;
}SphereRotation;

//!
//! \struct: FramePacking
//! \brief:   informs the decoder that the output cropped decoded picture contains samples
//!           of multiple distinct spatially packed constituent frames that are packed into one frame,
//!           or that the output cropped decoded pictures in output order form a temporal interleaving of
//!           alternating first and second constituent frames,
//!           using an indicated frame packing arrangement scheme
//!
typedef struct FRAME_PACKING
{
    uint32_t fpArrangementId;
    uint8_t  fpArrangementType;
    bool     quincunxSamplingFlag;
    uint8_t  contentInterpretationType;
    bool     spatialFlipping;
    bool     frame0Flipped;
    bool     fieldViews;
    bool     currentFrameIsFrame0;
    bool     frame0SelfContained;
    bool     frame1SelfContained;
    uint8_t  frame0GridX;
    uint8_t  frame0GridY;
    uint8_t  frame1GridX;
    uint8_t  frame1GridY;
    bool     upsampledAspectRatio;
}FramePacking;

//!
//! \struct: TileArrangement
//! \brief:  define the tiles arrangement in picture
//!
typedef struct TILE_ARRANGEMENT
{
    uint8_t  tileRowsNum;     // the number of tile rows in picture
    uint8_t  tileColsNum;     // the number of tile columns in picture
    uint16_t *tileRowHeight; //  pointer to height of each tile row
    uint16_t *tileColWidth;  //  pointer to width of each tile column
}TileArrangement;

//!
//! \brief  This structure is for the bitstream header
//!
typedef struct PARAM_BSHEADER
{
    uint8_t *data;
    int64_t size;
}Param_BSHeader;

//!
//! \brief  This structure is for the pitcure parameters
//!
typedef struct PARAM_PICTURE
{
    int32_t picWidth;
    int32_t picHeight;
    int32_t tileWidthNum;
    int32_t tileHeightNum;
    int32_t tileIsUniform;
    int32_t maxCUWidth;
}Param_PicInfo;

//Enumeration type for indicating rotation information in input Cubemap projected picture
typedef enum
{
    NO_TRANSFORM = 0,
    MIRRORING_HORIZONTALLY,
    ROTATION_180_ANTICLOCKWISE,
    ROTATION_180_ANTICLOCKWISE_AFTER_MIRRORING_HOR,
    ROTATION_90_ANTICLOCKWISE_BEFORE_MIRRORING_HOR,
    ROTATION_90_ANTICLOCKWISE,
    ROTATION_270_ANTICLOCKWISE_BEFORE_MIRRORING_HOR,
    ROTATION_270_ANTICLOCKWISE,
}E_TransformType;

//!
//! \brief  This structure is for the face property setting of an input video
//!
//! \param    faceWidth,      input,     the width of current face
//! \param    faceHeight,     input,     the height of current face
//! \param    idFace,         input,     the assigned index of current face. use 0 for the only face of ERP projection, use the enum OmafCubeFace defined in this header file under cubemap projection
//! \param    rotFace,        input,     the rotation mode of current face, use the enum type E_TransformType definition
//!
//!
typedef struct PARAM_FACEPPROPERTY
{
    int faceWidth;
    int faceHeight;
    int idFace;
    E_TransformType rotFace;
}Param_FaceProperty;

//!
//! \brief  This structure is for the face property setting of an input video
//!
//! \param    rows,       input,     the maximum row number of the faces for the video
//! \param    cols,       input,     the maximum column number of the faces for the video
//! \param    faces,      input,     the face property of the faces of the video. the maximum area is a 6x6 rectangular
//!
//!
typedef struct PARAM_VIDEOFPSTRUCT
{
    int rows;
    int cols;
    Param_FaceProperty faces[6][6];
}Param_VideoFPStruct;

//!
//! \brief  This structure stores the image width/height and tile width/height for each resolution
//!         Utilized for multi-resolution use cases.
//!
//!
typedef struct STREAM_INFO
{
    unsigned int FrameWidth;
    unsigned int FrameHeight;
    int32_t      TileWidth;
    int32_t      TileHeight;
}Stream_Info;

//!
//! \brief  This structure is for the viewport output parameters
//!
//! \param    dstWidthAlignTile,     output,    the width inside the viewport, which may add the edge to allign with tile
//! \param    dstHeightAlignTile,    output,    the height inside the viewport, which may add the edge to allign with tile
//! \param    dstWidthNet,           output,    the width inside the viewport, which is the net width
//! \param    dstHeightNet,          output,    the height inside the viewport, which is the net height
//! \param    xTopLeftNet,           output,    the x position of the topleft, which is the net position
//! \param    yTopLeftNet,           output,    the y position of the topleft, which is the net position
typedef struct PARAM_VIEWPORT_OUTPUT
{
    int32_t dstWidthAlignTile;
    int32_t dstHeightAlignTile;
    int32_t dstWidthNet;
    int32_t dstHeightNet;
    int32_t xTopLeftNet;
    int32_t yTopLeftNet;
}Param_ViewportOutput;

//!
//! \brief  This structure is for the view port parameters
//!
//! \param    ViewportWidth,      input,    the width for the viewport
//! \param    ViewportHeight,     input,    the height for the viewport
//! \param    viewPortPitch,      input,    the angle rotated aroud x(-90 ~ 90)
//! \param    viewPortYaw,        input,    the angle rotated aroud z(-180 ~ 180)
//! \param    viewPortFOVH,       input,    the horizontal FOV angle
//! \param    viewPortFOVV,       input,    the vertical FOV angle
//! \param    geoTypeOutput,      input,    the type for the output projection(viewport)
//! \param    geoTypeInput,       input,    the type for the input projection(ERP)
//! \param    faceWidth,          input,    the width of the face
//! \param    faceHeight,         input,    the height of the face
//! \param    tileNumRow,         input,    the number of tile rows
//! \param    tileNumCol,         input,    the number of tile columns
//! \param    paramVideoFP,       input,    the face properties under the projection mode
typedef struct PARAM_VIEWPORT
{
    int32_t                viewportWidth;
    int32_t                viewportHeight;
    float                  viewPortPitch;
    float                  viewPortYaw;
    float                  viewPortFOVH;
    float                  viewPortFOVV;
    EGeometryType          geoTypeOutput;
    EGeometryType          geoTypeInput;
    uint32_t               faceWidth;
    uint32_t               faceHeight;
    uint32_t               tileNumRow;
    uint32_t               tileNumCol;
    UsageType              usageType;
    Param_VideoFPStruct    paramVideoFP;
}Param_ViewPortInfo;

//!
//! \brief  This structure is for one input bistream, which may contain one tile or multi-tiles
//!
//! \param    tilesWidthCount,       input,    the horiz tiles count
//! \param    tilesHeightCount,      input,    the vert  tiles count
//! \param    pTiledBitstreamBuffer, input,    the bitstream data, maybe including the vps, sps pps info
//! \param    inputBufferLen,        input,    the bitstream data lenght
//! \param    tilesIdx,              input,    the indx of the bitstream in the while frame
//! \param    curBufferLen,          output,   the real length of this tile
//! \param    outputBufferLen,       output,   the real length of the output bitsteam
typedef struct INPUT_ONESTREAM_IFO
{
    int32_t  tilesWidthCount;
    int32_t  tilesHeightCount;
    uint8_t* pTiledBitstreamBuffer;
    uint32_t inputBufferLen;
    uint32_t tilesIdx;
    uint32_t curBufferLen;
    uint32_t outputBufferLen;
}param_oneStream_info;

//!
//! \brief  This structure is for the whole frame parameters
//!
//! \param    AUD_enable,            input,           the flag indicates Access Unit Delimiter enable or not
//! \param    VUI_enable,            input,           the flag indicates Video Usability Information enable or not
//! \param    pTiledBitstream,       input,           this is pointer, which points all of the bistreams
//! \param    sliceType,             output,          the slice type[I(2), P(1)] of the input bistream
typedef struct PARAM_STREAMSTITCHINFO
{
    bool                   AUD_enable;
    bool                   VUI_enable;
    param_oneStream_info **pTiledBitstream;
    uint32_t               sliceType;
    uint32_t               pts;
}param_streamStitchInfo;

//!
//! \brief  This structure is for the stitch parameters
//!
//! \param    usedType,           input,    the type used, there are three type, E_STREAM_STITCH_ONLY(only stream stitch),
//!                                         E_MERGE_AND_VIEWPORT(merge+viewport), E_PARSER_ONENAL(parsing one nal)
//! \param    pInputBitstream,    input,    the buffer for the input frame
//! \param    inputBitstreamLen,  input,    the length of the input bistream
//! \param    pOutputBitstream,   output,   the buffer for the output frame
//! \param    outputBitstreamLen, output,   the length of the stitched bitstream
//! \param    paramPicInfo,       input,    the param for the picture,just used in the usedType=0 & 1
//! \param    paramViewPort,      input,    the param for the viewport, just used in the usedType=1.
//! \param    paramStitchInfo,    input,    the param for the streamStitch, just used in the usedType=0
//! \param    sourceResolutionNum,input,    the number of high res stream for usages of multi stream, Pan-zoom for instance
//! \param    accessInterval,     input,    the time interval of every tile selection interface is called, expressed in milisecond
//! \param    pStreamInfo,        input,    the param for the stream information of multi-stream usage
//! \param    destWidth,          input,    the width for the destination output area, just used in the usedType=2
//! \param    destHeight,         input,    the height for the destination output area, just used in the usedType=2
//! \param    frameWidth,         input,    the width of the frame, just used in the usedType=E_MERGE_AND_VIEWPORT
//! \param    frameHeight,        input,    the height of the frame, just used in the usedType=E_MERGE_AND_VIEWPORT
//! \param    frameWidthLow,      input,    the width of the low resolution frame, just used in the usedType=E_MERGE_AND_VIEWPORT
//! \param    frameHeightLow,     input,    the height of the low resolution frame, just used in the usedType=E_MERGE_AND_VIEWPORT
//! \param    pInputLowBitstream, input,    the buffer for the low resolution input frame, just used in the usedType=E_MERGE_AND_VIEWPORT
//! \param    inputLowBistreamLen,input,    the length of the low resolution input bistream, just used in the usedType=E_MERGE_AND_VIEWPORT
//! \param    pOutputSEI,         output,   the buffer for the output SEI bistream, mainly RWPK, just used in the usedType=E_MERGE_AND_VIEWPORT
//! \param    outputSEILen,       output,   the length of the output SEI bistream, just used in the usedType=E_MERGE_AND_VIEWPORT
//! \param    timeStamp,          input,    using timestamp to track frame, especially used in the E_MERGE_AND_VIEWPORT use case
//!
typedef struct PARAM_360SCVP
{
    uint8_t                usedType;
    uint8_t               *pInputBitstream;
    uint32_t               inputBitstreamLen;
    uint8_t               *pOutputBitstream;
    uint32_t               outputBitstreamLen;
    Param_PicInfo          paramPicInfo;
    Param_ViewPortInfo     paramViewPort;
    param_streamStitchInfo paramStitchInfo;
    int32_t                sourceResolutionNum;
    float                  accessInterval;
    Stream_Info           *pStreamInfo;
    int32_t                destWidth;
    int32_t                destHeight;
    unsigned int           frameWidth;
    unsigned int           frameHeight;
    unsigned int           frameWidthLow;
    unsigned int           frameHeightLow;
    unsigned char         *pInputLowBitstream;
    unsigned int           inputLowBistreamLen;
    unsigned char         *pOutputSEI;
    unsigned int           outputSEILen;
    PluginDef              pluginDef;
    uint32_t               timeStamp;
    void                  *logFunction;       //external log callback function pointer, NULL if external log is not used
}param_360SCVP;

//!
//! \brief      This function mainly do the initialization, pass the input paramters to the stitch stream library, malloc the needed memory
//!             and return the handle of the  stitch stream library
//! \param      param_360SCVP* pParam360SCVP, input, refer to the structure param_360SCVP
//!
//! \return     void *, this is the handle of the stitch stream library.
//!             not null, if the initialization is ok
//!             null, if the initialization fails
void * I360SCVP_Init(param_360SCVP* pParam360SCVP);

//!
//! \brief      This function create a new handle based on one existed handle
//!             and return the new handle of the stitch stream library
//! \param      void* p360SCVPHandle, input, one existed stitch library handle
//!
//! \return     void *, the new created stitch library handle
//!             not null, if the creation is ok
//!             null, if the creation fails
void * I360SCVP_New(void* p360SCVPHandle);

//!
//! \brief      This function completes the stitch, this is to say, according to the viewport information to select the tiles
//!             and then stitch each tiles into one frame bitstream.
//! \param      param_360SCVP*   pParam360SCVP,     input/output, refer to the structure param_360SCVP
//! \param      void *           p360SCVPHandle,    input,        which is created by the I360SVCP_Init function
//!
//! \return     int32_t, the status of the function.
//!     0,      if succeed
//!     not 0,  if fail
//!
int32_t I360SCVP_process(param_360SCVP* pParam360SCVP, void * p360SCVPHandle);

//!
//! \brief      This function sets the parameter of the viewPort.
//!
//! \param      void *  p360SCVPHandle,    input, which is created by the I360SVCP_Init function
//! \param      float   yaw,               input, the angle rotated aroud y
//! \param      float   pitch,             input, the angle rotated aroud z
//!
//! \return     int32_t, the status of the function.
//!     0,      if succeed
//!     not 0,  if fail
//!
int32_t I360SCVP_setViewPort(void * p360SCVPHandle, float yaw, float pitch);

//!
//! \brief      This function sets the parameter of the viewPort with extention
//!
//! \param      void *  p360SCVPHandle,    input, which is created by the I360SVCP_Init function
//! \param      void *   pViewportInfo,    input, the viewport information handle
//!
//! \return     int32_t, the status of the function.
//!     0,      if succeed
//!     not 0,  if fail
//!
int32_t I360SCVP_setViewPortEx(void * p360SCVPHandle, HeadPose* pose);

//!
//! \brief      This function completes the un-initialization, free the memory
//!
//! \param      void *           p360SCVPHandle,      input, which is created by the I360SVCP_Init function
//!
//! \return     int32_t, the status of the function.
//!     0,      if succeed
//!     not 0,  if fail
//!
int32_t   I360SCVP_unInit(void * p360SCVPHandle);

//!
//! \brief    This function output the fixed number tiles according to the viewPort information in the initialization phase,
//!           especially these tiles are put in the original picture order.
//!           for cube map source, currently support FOV range 100 ~70 degree
//!
//! \param    TileDef*   pOutTile,              output, the list for the tiles inside the viewport
//! \param    int32_t*   pParamViewPortOutput,  output, please refer to the structure Param_ViewportOutput
//! \param    void*      p360SCVPHandle,        input,  which is created by the I360SVCP_Init function
//!
//! \return   int32_t, the number of the tiles inside the viewport.
//!
int32_t I360SCVP_getFixedNumTiles(TileDef* pOutTile, Param_ViewportOutput* pParamViewPortOutput, void* p360SCVPHandle);

//!
//! \brief    This function output the selected tiles according to the timely changed viewPort information,
//!           especially these tiles are put in the original picture order.
//!           for cube map source, currently support FOV range 100 ~70 degree
//!
//! \param    TileDef*   pOutTile,              output, the list for the tiles inside the viewport
//! \param    int32_t*   pParamViewPortOutput,  output, please refer to the structure Param_ViewportOutput
//! \param    void*      p360SCVPHandle,        input,  which is created by the I360SVCP_Init function
//!
//! \return   int32_t, the number of the tiles inside the viewport.
//!
int32_t I360SCVP_getTilesInViewport(TileDef* pOutTile, Param_ViewportOutput* pParamViewPortOutput, void* p360SCVPHandle);

//!
//! \brief    This function provides the parsing NAL function, can give the slice type, tileCols number, tileRows number and nal information
//!           if it is the Slice, must provide the slice header length; if SEI, provide the SEI payload type
//! \param    Nalu* pNALU,                 output, refer to the structure Nalu
//! \param    void* p360SCVPHandle,        input, which is created by the I360SVCP_Init function
//!
//! \return   int32_t, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t I360SCVP_ParseNAL(Nalu* pNALU, void* p360SCVPHandle);

//!
//! \brief    geneate the new SPS bitstream, input include start code, output without startcode
//!
//! \param    param_360SCVP*   pParam360SCVP,     input/output,  refer to the structure param_360SCVP
//! \param    void*            p360SCVPHandle,    input,         which is created by the I360SVCP_Init function
//!
//! \return     int32_t, the status of the function.
//!     0,      if succeed
//!     not 0,  if fail
//!
int32_t I360SCVP_GenerateSPS(param_360SCVP* pParam360SCVP, void* p360SCVPHandle);

//!
//! \brief    geneate the new PPS bitstream, input include start code, output without startcode
//!
//! \param    param_360SCVP*      pParam360SCVP,     input/output,  refer to the structure param_360SCVP
//! \param    TileArrangement*    pTileArrange,      input,         refer to the structure TileArrangement
//! \param    void*               p360SCVPHandle,    input,         which is created by the I360SVCP_Init function
//!
//! \return     int32_t, the status of the function.
//!     0,      if succeed
//!     not 0,  if fail
//!
int32_t I360SCVP_GeneratePPS(param_360SCVP* pParam360SCVP, TileArrangement* pTileArrange, void* p360SCVPHandle);

//!
//! \brief    geneate the new slice header bitstream, input includes start code, output without startcode
//!
//! \param    param_360SCVP*      pParam360SCVP,     input/output,  refer to the structure param_360SCVP
//! \param    int32_t             newSliceAddr,      input,         the address for the current slice
//! \param    void*               p360SCVPHandle,    input,         which is created by the I360SVCP_Init function
//!
//! \return     int32_t, the status of the function.
//!     0,      if succeed
//!     not 0,  if fail
//!
int32_t I360SCVP_GenerateSliceHdr(param_360SCVP* pParam360SCVP, int32_t newSliceAddr, void* p360SCVPHandle);

//!
//! \brief    geneate the RWPK SEI bitstream
//!
//! \param    void*                p360SCVPHandle,    input,      which is created by the I360SVCP_Init function
//! \param    RegionWisePacking*   pRWPK,             input,      refer to the structure RegionWisePacking
//! \param    uint8_t*             pRWPKBits,         output,     the PWPK bitstream buffer pointer
//! \param    int32_t*             pRWPKBitsSize,     output,     the length for the PWPK bitstream
//!
//! \return     int32_t, the status of the function.
//!     0,      if succeed
//!     not 0,  if fail
//!
int32_t I360SCVP_GenerateRWPK(void* p360SCVPHandle, RegionWisePacking* pRWPK, uint8_t *pRWPKBits, int32_t* pRWPKBitsSize);

//!
//! \brief    geneate the projection SEI bitstream
//!
//! \param    void*                p360SCVPHandle,    input,      which is created by the I360SVCP_Init function
//! \param    int32_t              projType,          input,      the project type(ERP or Cubmap)
//! \param    uint8_t*             pProjBits,         output,     the project type bitstream buffer pointer
//! \param    int32_t*             pProjBitsSize,     output,     the length for the project type bitstream
//!
//! \return     int32_t, the status of the function.
//!     0,      if succeed
//!     not 0,  if fail
//!
int32_t I360SCVP_GenerateProj(void* p360SCVPHandle, int32_t projType, uint8_t *pProjBits, int32_t* pProjBitsSize);

//!
//! \brief This function sets the parameter of the viewPort.
//!
//! \param    void*              p360SCVPHandle,     input,  which is created by the I360SVCP_Init function
//! \param    RegionWisePacking* pRWPK,              output, the sturcture info got from the SEI bitstream
//! \param    uint8_t *          pRWPKBits,          input,  the RWPK SEI bitstream
//! \param    uint32_t           RWPKBitsSize        input,  the bytesize of the RWPK bitstream
//!
//!\return   int32_t, the status of the function.
//!          0,     if succeed
//!          not 0, if fail
//!
int32_t I360SCVP_ParseRWPK(void* p360SCVPHandle, RegionWisePacking* pRWPK, uint8_t *pRWPKBits, uint32_t RWPKBitsSize);

//!
//! \brief This function gets the content coverge for viewport.
//!
//! \param    void*              p360SCVPHandle,     input,  which is created by the I360SVCP_Init function
//! \param    CCDef*             pOutCC,             output, the centre and range of Azimuth and Elevation of 3D sphere
//!
//!\return   int32_t, the status of the function.
//!          0,     if succeed
//!          not 0, if fail
//!
int32_t I360SCVP_getContentCoverage(void* p360SCVPHandle, CCDef* pOutCC);

//!
//! \brief    This function can get the specified values, for example vps, sps, and pps.
//!
//! \param    void*      p360SCVPHandle,   input,     which is created by the I360SVCP_Init function
//! \param    uint32_t   paramID,          input,     refer to the above macro defination of ID_SCVP_PARAM_XXX
//! \param    uint8_t**  pValue,           output,    the specified data
//!
//! \return   int32_t, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t I360SCVP_GetParameter(void* p360SCVPHandle, int32_t paramID, void** pValue);

//!
//! \brief    This function can set the specified values.
//!
//! \param    void*     p360SCVPHandle,  input,     which is created by the I360SVCP_Init function
//! \param    uint32_t  paramID,         input,     refer to the above macro defination of ID_SCVP_PARAM_XXX
//! \param    void*     pValue,          input,     the specified value
//!
//! \return   int32_t, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t I360SCVP_SetParameter(void* p360SCVPHandle, int32_t paramID, void* pValue);

//!
//! \brief    This function can select tiles by legacy algorithm with good accuracy but low performance.
//!
//! \param    void*     p360SCVPHandle,  input,     which is created by the I360SVCP_Init function
//! \param    void*     pOutTile,        output,    the selected tiles
//!
//! \return   int32_t, the selected tiles number.
//!           >0, if succeed
//!           <0, if fail
//!
int32_t I360SCVP_GetTilesByLegacyWay(TileDef* pOutTile, void* p360SCVPHandle);

//! \brief    This function can set the logcallback funciton.
//!
//! \param    void*     p360SCVPHandle,  input,     which is created by the I360SVCP_Init function
//! \param    void*     externalLog,     input,     the specified logcallback function
//!
//! \return   int32_t, the status of the function.
//!           ERROR_NONE if success, else failed reason
int32_t I360SCVPSetLogCallBack(void* p360SCVPHandle, void* externalLog);

#ifdef __cplusplus
}
#endif

#endif //_360SCVP_API_H_
