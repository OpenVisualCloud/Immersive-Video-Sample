
/** \fopyright (c) 2018, Intel Corporation
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

#ifndef __360SCVP_COMMONDEF__
#define __360SCVP_COMMONDEF__
#include "stdint.h"

#include "../utils/safe_mem.h"

#define SAFE_DELETE(x) if(NULL != (x)) { delete (x); (x)=NULL; };
#define SAFE_FREE(x)   if(NULL != (x)) { free((x));    (x)=NULL; };
#define SAFE_DELETE_ARRAY(x) if(NULL != (x)) { delete[] (x); (x)=NULL; };

typedef enum SLICE_TYPE {
    SLICE_B = 0,
    SLICE_P = 1,
    SLICE_I = 2,
    SLICE_IDR = 3,
}slice_type;

enum GeometryType
{
    SVIDEO_EQUIRECT = 0,
    SVIDEO_CUBEMAP,
    SVIDEO_VIEWPORT,
    SVIDEO_TYPE_NUM,
};

/*!
*
*  currently the library can support three types
*  0(only merge), similar to our before usage
*  1(merge + viewport), used in webrtc use case
*  2(parsing one nal), used in the omaf
*
*/
/*
enum ParserType
{
    PARSER_MERGE = 0,
    PARSER_ALLNALS,
    PARSER_ONENAL,
    PARSER_TYPE_NUM,
};
*/
typedef struct HEVC_SPECIALINFO
{
    uint8_t *ptr;
    uint32_t ptr_size;

    uint8_t  startCodesSize; //nalu start codes size
    uint8_t  naluType; //nalu type

    uint16_t seiPayloadType; //SEI payload type if nalu is for SEI
    uint16_t sliceHeaderLen; //slice header length if nalu is for slice

    uint8_t temporal_id;
    uint8_t layer_id;

}hevc_specialInfo;


#endif //  __360SCVP_COMMONDEF__

