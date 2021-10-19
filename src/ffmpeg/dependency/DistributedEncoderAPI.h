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
//! \file:   DistributedEncoderAPI.h
//! \brief:  distributed encoder library interfaces
//!

#ifndef _DISTRIBUTEDENCODERAPI_H_
#define _DISTRIBUTEDENCODERAPI_H_

#include "error_code.h"
#include "360SCVPAPI.h"
#include <stdbool.h>

#define DATA_NUM 8
#define GUARD_BAND_TYPE 4
#define IP_ADDRESS_LEN 16
#define MAX_SESSION_COUNT 32

//!
//! \enum   DispatchType
//! \brief  Indicate the task dispatch type
//!
typedef enum{
    Undefined = -1,
    Average,
    ResourceBalanced,
    Reserved
}DispatchType;

//!
//! \enum:  InputStreamType
//! \brief: type of input stream
//!
typedef enum{
    undefined = -1,
    encoded,
    raw,
    reserved
}InputStreamType;

//!
//! \enum:  StreamCodecID
//! \brief: codec ID of input stream
//!
typedef enum{
    CodecID_NONE = 0,
    CodecID_H264,
    CodecID_HEVC
}StreamCodecID;

//!
//! \enum:  PictureType
//! \brief: type of picture in stream
//!
typedef enum
{
    PictureType_NONE = 0,
    PictureType_I,
    PictureType_P,
    PictureType_B,
}PictureType;

//!
//! \enum:  PixelColorFormat
//! \brief: pixel color format of input stream
//!
typedef enum
{
    PixelColor_NONE = -1,
    PixelColor_YUV400,
    PixelColor_YUV420,
    PixelColor_YUV422,
    PixelColor_YUV444
}PixelColorFormat;

//!
//! \enum:  ParamType
//! \brief: parameter types from this library
//!
typedef enum
{
    Param_Header = 0
}ParamType;

typedef enum
{
    DecoderType_None = 0,
    DecoderType_openHEVC,
    DecoderType_ffmpeg,
}DecoderType;

typedef enum
{
    EncoderType_None = 0,
    EncoderType_SVTHEVC,
    EncoderType_Multiple_SVTHEVC,
}EncoderType;

//!
//! \struct: Headers
//! \brief:  headers of output stream
//! \details: VPS, SPS, PPS, SEI information of output stream
//!
typedef struct HEADERS
{
    uint8_t  *headerData;
    int64_t  headerSize;
}Headers;

//!
//! \struct: SessionInfo
//! \brief:  Information of work session
//!
typedef struct SESSIONINFO
{
    bool    isNative;                  //!< Native or remote mode
    char    ipAddress[IP_ADDRESS_LEN]; //!< IP address of worker
    int32_t port;                      //!< Port of worker
    int32_t targetSocket;              //!< Target CPU group
    int32_t gpuNode;                   //!< Assigned GPU node
}SessionInfo;

//!
//! \struct: StreamInfo
//! \brief:  Information of input stream
//! \details: the structure only supports uniform segmentation
//!          TODO: add support for non-uniform segmentation
//!
typedef struct STREAMINFO
{
    uint32_t            frameWidth;         //!< width of frame
    uint32_t            frameHeight;        //!< height of frame
    uint32_t            tileColumn;         //!< tile number in column
    uint32_t            tileRow;            //!< tile number in row
    bool                tileUniformSpacing; //!< tile uniform segmentation flag
    bool                tileOverlapped;     //!< overlap between tiles flag
    uint32_t            overlapWidth;       //!< horizontal overlap
    uint32_t            overlapHeight;      //!< vertical overlap
    InputStreamType     streamType;         //!< the type of input stream
    StreamCodecID       codecId;            //!< the code ID of input stream
}StreamInfo;

//!
//! \struct: EncoderParam
//! \brief:  input parameters for encoder
//!
typedef struct ENCODERPARAM{
    uint32_t bit_depth;                 //!< bit depth value, 8 bits or 10 bits
    PixelColorFormat    format;         //!< pixel color format
    uint32_t vui_info;                  //!< video usability information flag
    uint32_t hierarchical_level;        //!< the hierarchical level for to construct GOP
    uint32_t intra_period;              //!< the distance between two adjacent intra frame
    uint32_t la_depth;                  //!< the number of frames that used for look ahead
    uint32_t enc_mode;                  //!< the preset for quality and performance balance,
                                        //!< [0-12], 0 is best quality, 12 is best performance
    uint32_t pred_structure;            //!< [0-2], 0 is IPPP..., 1 is IBBB...(B is low-delay B), and 2 is IBBB...(B is normal bi-directional B)
    uint32_t rc_mode;                   //!< rate control mode, 0 is CQP mode and 1 is VBR mode
    uint32_t qp;                        //!< quantization value under CQP mode
    uint32_t bit_rate;                  //!< bitrate value under VBR mode
    uint32_t scd;                       //!< scene change detection flag
    uint32_t tune;                      //!< specific encoder tuning, 0 is visually optimized mode,
                                        //!< 1 is PSNR/SSIM optimized mode, 2 is VMAF optimized mode
    uint32_t profile;                   //!< the profile to create bitstream, 1 is Main with 8 bit depth,
                                        //!< 2 is Main 10 with 8-10 bit depth
    uint32_t base_layer_switch_mode;    //!< decide use P or B frame in base layer, 0 is B frame, 1 is P frame
    uint32_t intra_refresh_type;        //!< the type of intra frame refresh, 1 is CRA, 2 is IDR intra refresh type
    uint32_t tier;                      //!< limitation for max bitrate and max buffer size
    uint32_t level;                     //!< limitation for max bitrate and max buffer size
    uint32_t aud;                       //!< access unit delimiter flag
    uint32_t hrd;                       //!< high dynamic range flag
    uint32_t asm_type;                  //!< assembly instruction type
    int32_t  framerate_num;             //!< frame rate numerator
    int32_t  framerate_den;             //!< frame rate denominator
    uint8_t  deblocking_enable;         //!< deblocking loop filtering flag
    uint8_t  sao_enable;                //!< sample adaptive offset filtering flag
    uint8_t  MCTS_enable;               //!< motion vector constrains flag
    uint8_t  tile_columnCnt;            //!< tile column count when tile is enabled
    uint8_t  tile_rowCnt;               //!< tile row count when tile is enabled
    int8_t   target_socket;             //!< Target socket to run on
    bool     in_parallel;               //!< multiple tiles encoding in parallel
    bool     native_mode;                //!< flag of native mode for encoder
}EncoderParam;

typedef struct INPUTFRAME{
    char                *data[DATA_NUM];    //!< data array, if the input frame is encoded, it can be stored in data[0]
    uint32_t            stride[DATA_NUM];   //!< stride array, if the input frame is encoded, size of it can be stored in stride[0]
    int64_t             copysize[DATA_NUM]; //!< copy size array, YUV raw input only
    uint32_t            width;              //!< width of frame
    uint32_t            height;             //!< height of frame
    PixelColorFormat    format;             //!< pixel color format
    PictureType         picType;            //!< picture type,e.g. I/P/B/...
    bool                useSharedMem;       //!< if the data is using shared memory or not
}InputFrame;

// SEI related structures
typedef struct REGIONWISEPACKINGINFO{
    bool              enable;                     //!< indicate whether this information will be applied or not
    bool              guardBandEnable;            //!< indicate whether guard band is enabled or not
}RegionWisePackingInfo;

typedef struct PROJECTIONINFO{
    bool           enable;         //!< indicate whether this information will be applied or not
    H265SEIType    type;           //!< the type of projection
}ProjectionInfo;

typedef struct VIEWPORTINFO{
    bool           enable;         //!< indicate whether this information will be applied or not
}ViewPortInfo;

typedef struct FRAMEPACKINGINFO{
    bool           enable;         //!< indicate whether this information will be applied or not
    FramePacking   framePacking;   //!< information of frame packing arrangement
}FramePackingInfo;

typedef struct ROTATIONINFO{
    bool           enable;         //!< indicate whether this information will be applied or not
    SphereRotation sphereRotation; //!< information on rotation angles yaw (a), pitch , and roll
}RotationInfo;

typedef struct SUPPLEMENTALENHANCEMENTINFO{
    RegionWisePackingInfo rwpkInfo;     //!< region wise packing information
    ProjectionInfo        projInfo;     //!< projection information
    ViewPortInfo          vpInfo;       //!< viewport information
    FramePackingInfo      fpkInfo;      //!< frame packing information
    RotationInfo          roInfo;       //!< rotation information
}SupplementalEnhancementInfo;

typedef struct OHOPTION{
    int32_t threadCount;
    int32_t threadType;
}ohOption;

typedef struct FFMPEGOPTION{
    StreamCodecID codecID;
}ffmpegOption;

typedef struct DECODEROPTION{
    DecoderType decType;
    void        *decSetting;
}DecoderOption;

typedef struct ENCODEROPTION{
    EncoderType encType;
    void        *encSetting;
}EncoderOption;

typedef struct CODECAPPOPTION{
    DecoderOption   decOption;
    EncoderOption   encOption;
    void            *logFunction;       //!< External log callback function pointer, NULL if external log is not used
    uint32_t        minLogLevel;        //!< Minimal log level of output
}CodecAppOption;

//!
//! \struct:  DistributedEncoderParam
//! \brief:   parameters for distributed encoder library
//!
typedef struct DISTRIBUTEDENCODERPARAM{
    StreamInfo                  streamInfo;                      //!< Information of input stream
    EncoderParam                encoderParams;                   //!< Parameters for encoding
    DispatchType                type;                            //!< Task dispatch type
    SupplementalEnhancementInfo suppleEnhanceInfo;               //!< Supplemental Enhancement Information
    CodecAppOption              codecOption;                     //!< Choice and the settings of decoder/encoder
    bool                        glogInitialized;                 //!< Whether glog has been initialized
    SessionInfo                 *sessionList[MAX_SESSION_COUNT]; //!< Session info list
    uint8_t                     sessionCount;                    //!< Session info count
}DistributedEncoderParam;

#ifdef __cplusplus
extern "C" {
#endif

//!
//! \brief  Parse the input config file and fulfill its defined session info into param
//!
//! \param  [in] configFilePath
//!         Path of configure file
//! \param  [in] param
//!         DistributedEncoderParam
//!
//! \return DEStatus
//!         DE_STATUS_SUCCESS if success, else fail reason
//!
DEStatus DistributedEncoder_ParseConfigFile(const char* configFilePath, DistributedEncoderParam *param);

//!
//! \brief  Initialize resources and get distributed encoder library handle
//!
//! \param  [in] param
//!         DistributedEncoderParam
//!
//! \return DEHandle
//!         Distributed encoder library handle
//!
DEHandle DistributedEncoder_Init(DistributedEncoderParam *param);

//!
//! \brief  Process input frame
//!
//! \param  [in] handle
//!         Distributed encoder library handle
//! \param  [in] frame
//!         Input frame
//!
//! \return DEStatus
//!         DE_STATUS_SUCCESS if success, else fail reason
//!
DEStatus DistributedEncoder_Process(DEHandle handle, InputFrame *frame);

//!
//! \brief  Get if memory copy for input is needed
//!
//! \param  [in] handle
//!         Distributed encoder library handle
//!
//! \return bool
//!         true if need memcpy for input, else use input pointer directly
//!
bool DistributedEncoder_NeedMemCpyForInput(DEHandle handle);

//!
//! \brief  Get output packet
//!
//! \param  [in] handle
//!         Distributed encoder library handle
//! \param  [out] pktData
//!         Packet data
//! \param  [out] pktSize
//!         Packet size
//! \param  [out] pktPTS
//!         Packet PTS
//! \param  [out] pktDTS
//!         Packet DTS
//! \param  [out] eos
//!         EOS flag
//!
//! \return DEStatus
//!         DE_STATUS_SUCCESS if success, else fail reason
//!
DEStatus DistributedEncoder_GetPacket(DEHandle handle, char** pktData, uint64_t* pktSize,
                                      int64_t* pktPTS, int64_t* pktDTS, bool* eos);

//!
//! \brief  Set parameter to distributed encoder
//!
//! \param  [in] handle
//!         Distributed encoder library handle
//! \param  [in] type
//!         Input parameter type
//! \param  [in] param
//!         Input parameter value
//!
//! \return DEStatus
//!         DE_STATUS_SUCCESS if success, else fail reason
//!
DEStatus DistributedEncoder_SetParam(DEHandle handle, uint32_t type, uint64_t param);

//!
//! \brief  Get parameter from distributed encoder
//!
//! \param  [in] handle
//!         Distributed encoder library handle
//! \param  [in] type
//!         Type of wanted parameter
//! \param  [out] param
//!         The wanted parameter
//!
//! \return DEStatus
//!         DE_STATUS_SUCCESS if success, else fail reason
//!
DEStatus DistributedEncoder_GetParam(DEHandle handle, ParamType type, void** param);

//!
//! \brief  Set the logcallback funciton
//!
//! \param  [in] deHandle
//!         Distributed encoder library handle
//! \param  [in] externalLog
//!         The customized logging callback function pointer
//!
//! \return DEStatus
//!         DE_STATUS_SUCCESS if success, else fail reason
//!
DEStatus DistributedEncoder_SetLogCallBack(void* deHandle, void* externalLog);

//!
//! \brief  Close all the connections and clean all resources
//!
//! \param  [in] handle
//!         Distributed encoder library handle
//!
//! \return DEStatus
//!         DE_STATUS_SUCCESS if success, else fail reason
//!
DEStatus DistributedEncoder_Destroy(DEHandle handle);

#ifdef __cplusplus
}
#endif

#endif /* _DISTRIBUTEDENCODERAPI_H_ */

