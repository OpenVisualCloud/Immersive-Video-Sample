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
#ifndef _GENTILESTREAM_API_H_
#define _GENTILESTREAM_API_H_
#include <stdint.h>
#include "360SCVPAPI.h"

#define ID_GEN_TILED_BITSTREAMS_VPS    100
#define ID_GEN_TILED_BITSTREAMS_SPS    101
#define ID_GEN_TILED_BITSTREAMS_PPS    102
#define ID_GEN_TILED_BITSTREAMS_HEADER 103

#ifdef __cplusplus
extern "C" {
#endif
    typedef struct HEVC_NAL_INFO
    {
        int32_t   nalLen;
        uint8_t*  pNalStream;
    }nal_info;

//!
//! \brief  This structure is for the whole frame parameters
//!
//! \param    frameWidth,            input,           the width of the frame
//! \param    frameHeight,           input,           the height of the frame
//! \param    tilesUniformSpacing,   input,           the flag indicates tiles are uniform segmented or not
//! \param    AUD_enable,            input,           the flag indicates Access Unit Delimiter enable or not
//! \param    VUI_enable,            input,           the flag indicates Video Usability Information enable or not
//! \param    tilesWidthCount,       input/output,    the horiz tiles count, if using in parsing, it's output; and if using in stitching, it's input
//! \param    tilesHeightCount,      input/output,    the vert  tiles count, if using in parsing, it's output; and if using in stitching, it's input
//! \param    pTiledBitstream,       input,           this is pointer, which points all of the bistreams
//! \param    inputBistreamsLen,     input,           the length of all the input bistreams
//! \param    parseType              input,           the flag to define  stitching(0), parsing all NALs(1) and parsing one NAL(2)
//! \param    pOutputTiledBitstream, output,          the generated tiled bitstream
//! \param    outputiledbistreamlen, output,          the length of the generated tiled bitstream
//! \param    sliceType,             output,          the slice type[I(2), P(1)] of the input bistream
//! \param    pNalInfo,              output,          the nal list for the input bistream, used when parseType== 1
//! \param    specialLen,            output,          the length for the vps, sps, pps
typedef struct PARAM_GEN_TILEDSTREAM
{
    uint32_t               frameWidth;
    uint32_t               frameHeight;
    bool                   tilesUniformSpacing;
    bool                   AUD_enable;
    bool                   VUI_enable;
    int32_t                tilesWidthCount;
    int32_t                tilesHeightCount;
    param_oneStream_info **pTiledBitstream;
    uint32_t               inputBistreamsLen;
    int32_t                parseType;
    uint8_t               *pOutputTiledBitstream;
    uint32_t               outputiledbistreamlen;
    uint32_t               sliceType;
    uint32_t               pts;
    nal_info              *pNalInfo;
    int32_t                specialLen;
    uint16_t               nalType;
    uint8_t                startCodesSize;
    uint16_t               seiPayloadType; //SEI payload type if nalu is for SEI
    uint16_t               sliceHeaderLen; //slice header length if nalu is for slice

}param_gen_tiledStream;

//!
//! \brief    This function mainly do the initialization, pass the input paramters to the genTiledstream library, malloc the needed memory
//!           and return the handle of the  genTiledstream library
//! \param    param_gen_tiledStream* pParamGenTiledStream, input, refer to the structure param_gen_tiledStream
//!
//! \return   void*, this is the handle of the genTiledstream library.
//!           not null, if the initialization is ok
//!           null, if the initialization fails
//!
void* genTiledStream_Init(param_gen_tiledStream* pParamGenTiledStream);

//!
//! \brief    This function completes the generation, this is to say, merge the input bistream into one tiled bitstream.
//!
//! \param    param_gen_tiledStream* pParamGenTiledStream,  output, refer to the structure param_gen_tiledStream
//! \param    void*                  pGenHandle,            input, which is created by the genTiledStream_Init function
//!
//! \return   int32_t, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t   genTiledStream_process(param_gen_tiledStream* pParamGenTiledStream, void* pGenHandle);

//!
//! \brief    This function provides the parsing NAL function, can give the slice type, tileCols number, tileRows number and nal information
//!
//! \param    param_gen_tiledStream* pParamGenTiledStream,  output, refer to the structure param_gen_tiledStream
//! \param    void*                  pGenHandle,            input, which is created by the genTiledStream_Init function
//!
//! \return   int32_t, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t   genTiledStream_parseNals(param_gen_tiledStream* pParamGenTiledStream, void* pGenHandle);

//!
//! \brief    This function can get the specified values, for example vps, sps, and pps.
//!
//! \param    void*     pGenHandle,   input,     which is created by the genTiledStream_Init function
//! \param    uint32_t  id,           input,     refer to the above macro defination of ID_GEN_TILED_BITSTREAMS_XXX
//! \param    uint8_t*  pValue,       output,    the specified data
//!
//! \return   int32_t, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t   genTiledStream_getParam(void* pGenHandle, uint32_t id, uint8_t** pValue);

//!
//! \brief    This function can set the specified values.
//!
//! \param    void*     pGenHandle, input,     which is created by the genTiledStream_Init function
//! \param    uint32_t  id,         input,     refer to the above macro defination of ID_GEN_TILED_BITSTREAMS_XXX
//! \param    uint8_t*  pValue,     input,     the specified value
//!
//! \return   int32_t, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t   genTiledStream_setParam(void* pGenHandle, uint32_t id, uint8_t* pValue);

//!
//! \brief    This function completes the un-initialization, free the memory
//!
//! \param    void*                 pGenHandle,            input, which is created by the genTiledStream_Init function
//!
//! \return   int32_t, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t   genTiledStream_unInit(void* pGenHandle);

#ifdef __cplusplus
}
#endif

#endif //_GENTILESTREAM_API_H_
