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
#ifndef _GENMERGESTREAM_API_H_
#define _GENMERGESTREAM_API_H_

#include "360SCVPTiledstreamAPI.h"

//!
//! \\brief    structure for one resolution parameters
//!
typedef struct ONE_RES_PARAM
{
    uint32_t               width;                  //!< stream width
    uint32_t               height;                 //!< stream height
    int32_t                totalTilesCount;        //!< total tiles number of stream
    int32_t                selectedTilesCount;     //!< selected tiles number of stream
    param_oneStream_info **pTiledBitstreams;       //!< pointer to array of all tiles stream data
    param_oneStream_info  *pHeader;                //!< pointer to stream headers data
    int32_t                tile_height;            //!< Height of each tile(Suppose all tiles have same resolution)
    int32_t                tile_width;             //!< Width of each tile(Suppose all tiles have same resolution)
    int32_t                num_tile_columns;       //!< Number of tiles in column
    int32_t                num_tile_rows;          //!< Number of tiles in row
    bool                   bOrdered;               //!< flag for whether tiles are merged with order
}one_res_param;

//!
//! \\brief   structure for all needed parameters to get merged stream
//!
typedef struct PARAM_MERGEBITSTREAM
{
    one_res_param          highRes;                //!< parameters of high resolution stream
    one_res_param          lowRes;                 //!< parameters of low resolution stream
    uint32_t               inputBistreamsLen;      //!< input bitstream length
    uint8_t               *pOutputBitstream;       //!< pointer to output bitstream
    uint32_t               outputiledbistreamlen;  //!< length of output bitstream
    bool                   bWroteHeader;           //!< flag for whether Headers need to be wrote
}param_mergeStream;

//!
//! \brief    Initialize tile merge library
//! \details  Allocate memory for pointer members of stream data and parameters,
//!           and get handle
//!
//! \param    [in] mergeStreamParams
//!           Input pointer to stream parameters
//!
//! \return   void*
//!           library handle if success, else NULL
//!
void* tile_merge_Init(param_mergeStream *mergeStreamParams);

//!
//! \brief    Close tile merge library
//! \details  Free memory for pointer members of stream data and parameters
//!           for handle
//!
//! \param    [in] handle
//!           Pointer to library handle
//!
//! \return   int32_t
//!           0 if success, else non-zero value
//!
int32_t tile_merge_Close(void* handle);

//!
//! \brief    Process tile merge library//! \details  Merge all the input tiled streams in a proper way
//!           and get the merged stream
//!
//! \param    [in] mergeStreamParams
//!           Input pointer to stream parameters
//! \param    [in] handle
//!           Pointer to library handle
//!
//! \return   int32_t
//!           0 if success, else non-zero value
//!
int32_t tile_merge_Process(param_mergeStream *mergeStreamParams, void* handle);

//!
//! \brief    reset the merge function
//!
//! \param    [in] handle
//!           Pointer to library handle
//!
//! \return   int32_t
//!           0 if success, else non-zero value
//!
int32_t tile_merge_reset(void* handle);

#endif //_GENMERGESTREAM_API_H_
