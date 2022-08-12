// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "IMSVTHEVCEncoderBase.h"

#include <libyuv/convert.h>
#include <libyuv/planar_functions.h>
#include <libyuv/scale.h>

#include "MediaUtilities.h"

#define LTTNG_TRACE_INFO    ELOG_TRACE_T

namespace owt_base {

DEFINE_LOGGER(SVTHEVCEncoderBase, "owt.SVTHEVCEncoderBase");

SVTHEVCEncodedPacket::SVTHEVCEncodedPacket(EB_BUFFERHEADERTYPE *pBuffer)
    : data(NULL)
    , length(0)
    , isKey(false)
{
    if (pBuffer && pBuffer->pBuffer && pBuffer->nFilledLen) {
        m_array.reset(new uint8_t[pBuffer->nFilledLen]);
        memcpy(m_array.get(), pBuffer->pBuffer, pBuffer->nFilledLen);

        data = m_array.get();
        length = pBuffer->nFilledLen;
        isKey = (pBuffer->sliceType == EB_IDR_PICTURE);
        pts = pBuffer->pts;
    }
}

SVTHEVCEncodedPacket::~SVTHEVCEncodedPacket()
{
}

SVTHEVCEncoderBase::SVTHEVCEncoderBase()
    : m_handle(NULL)
    , m_scaling_frame_buffer(NULL)
    , m_forceIDR(false)
    , m_quit(false)
    , m_enableBsDump(false)
    , m_bsDumpfp(NULL)
{
    memset(&m_encParameters, 0, sizeof(m_encParameters));
}

SVTHEVCEncoderBase::~SVTHEVCEncoderBase()
{
    if (m_thread) {
        m_quit = true;
        m_queue_cond.notify_all();
        m_thread->join();
    }

    if (m_handle) {
        ELOG_INFO_T("EbDeinit* started, if NOT see EbDeinitEncoder ok1 + EbDeinitHandle ok2, crash/leak happens. Check with dmesg/free");
        EB_ERRORTYPE return_error = EbDeinitEncoder(m_handle);
        if (return_error != EB_ErrorNone) {
            ELOG_ERROR_T("EbDeinitEncoder failed, ret 0x%x", return_error);
            //TODO error handling, shall i continue to EbDeinitHandle?
        } else {
            ELOG_INFO_T("EbDeinitEncoder ok1");
        }

        return_error = EbDeinitHandle(m_handle);
        if (return_error != EB_ErrorNone) {
            ELOG_ERROR_T("EbDeinitHandle failed, ret 0x%x", return_error);
            //TODO error handling
        } else {
            ELOG_INFO_T("EbDeinitHandle ok2");
        }
        m_handle = NULL;

        deallocateBuffers();

        if (m_scaling_frame_buffer) {
            free(m_scaling_frame_buffer);
            m_scaling_frame_buffer = NULL;
        }

        if (m_bsDumpfp) {
            fclose(m_bsDumpfp);
            m_bsDumpfp = NULL;
        }
    }
}

void SVTHEVCEncoderBase::initDefaultParameters()
{
    // Encoding preset
    m_encParameters.encMode                         = 9;
    m_encParameters.tune                            = 1;
    //m_encParameters.latencyMode                     = 1;

    // GOP Structure
    m_encParameters.intraPeriodLength               = 255;
    m_encParameters.intraRefreshType                = 0;
    m_encParameters.hierarchicalLevels              = 0;

    m_encParameters.predStructure                   = 0;
    m_encParameters.baseLayerSwitchMode             = 0;

    // Input Info
    m_encParameters.sourceWidth                     = 0;
    m_encParameters.sourceHeight                    = 0;
    m_encParameters.frameRate                       = 0;
    m_encParameters.frameRateNumerator              = 0;
    m_encParameters.frameRateDenominator            = 0;
    m_encParameters.encoderBitDepth                 = 8;
    m_encParameters.encoderColorFormat              = EB_YUV420;
    m_encParameters.compressedTenBitFormat          = 0;
    m_encParameters.framesToBeEncoded               = 0;

    // Visual quality optimizations only applicable when tune = 1
    m_encParameters.bitRateReduction                = 1;
    m_encParameters.improveSharpness                = 1;

    // Interlaced Video
    m_encParameters.interlacedVideo                 = 0;

#if 0
    // Quantization
    m_encParameters.qp                              = 32;
    m_encParameters.useQpFile                       = 0;
#endif

    // Deblock Filter
    m_encParameters.disableDlfFlag                  = 0;

    // SAO
    m_encParameters.enableSaoFlag                   = 1;

    // Motion Estimation Tools
    m_encParameters.useDefaultMeHme                 = 1;
    m_encParameters.enableHmeFlag                   = 1;

#if 0
    // ME Parameters
    m_encParameters.searchAreaWidth                 = 16;
    m_encParameters.searchAreaHeight                = 7;
#endif

    // MD Parameters
    m_encParameters.constrainedIntra        = 0;

    // Rate Control
    m_encParameters.rateControlMode         = 1; //0 : CQP , 1 : VBR
    m_encParameters.sceneChangeDetection    = 1;
    m_encParameters.lookAheadDistance       = 0;
    m_encParameters.targetBitRate           = 0;
    m_encParameters.maxQpAllowed            = 48;
    m_encParameters.minQpAllowed            = 10;

    // bitstream options
    m_encParameters.codeVpsSpsPps           = 1;
    m_encParameters.codeEosNal              = 0;
    m_encParameters.videoUsabilityInfo      = 0;
    m_encParameters.highDynamicRangeInput   = 0;
    m_encParameters.accessUnitDelimiter     = 0;
    m_encParameters.bufferingPeriodSEI      = 0;
    m_encParameters.pictureTimingSEI        = 0;
    m_encParameters.registeredUserDataSeiFlag   = 0;
    m_encParameters.unregisteredUserDataSeiFlag = 0;
    m_encParameters.recoveryPointSeiFlag    = 0;
    m_encParameters.enableTemporalId        = 1;
    m_encParameters.profile                 = 2;
    m_encParameters.tier                    = 0;
    m_encParameters.level                   = 0;
    m_encParameters.fpsInVps                = 0;

    // Application Specific parameters
    m_encParameters.channelId               = 0;
    m_encParameters.activeChannelCount      = 1;

    // Threads management
    m_encParameters.logicalProcessors           = 0;
    m_encParameters.targetSocket                = -1;
    m_encParameters.switchThreadsToRtPriority   = 1;

    // ASM Type
    m_encParameters.asmType = 1;

    // Demo features
    m_encParameters.speedControlFlag        = 0;
    m_encParameters.injectorFrameRate       = m_encParameters.frameRate << 16;

    // Debug tools
    m_encParameters.reconEnabled            = false;
}

void SVTHEVCEncoderBase::updateParameters(uint32_t width, uint32_t height, uint32_t frameRate,
                                          uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds,  uint32_t encMode)
{
    m_encParameters.encMode             = encMode;
    //resolution
    m_encParameters.sourceWidth         = width;
    m_encParameters.sourceHeight        = height;

    //gop
    m_encParameters.intraPeriodLength   = 5 - 1;  //GoP=5

    //framerate
    m_encParameters.frameRate           = frameRate << 16;

    //bitrate
    m_encParameters.rateControlMode     = 1; //0 : CQP , 1 : VBR
    m_encParameters.intraRefreshType    = 0; //0 for CQP, >=0 for VBR.  FOV cannot work with default value -1:CRA Open GOP
    m_encParameters.targetBitRate       = bitrateKbps * 1000;
    m_encParameters.maxQpAllowed        = 26; // we heard TV broadcasting is using qp=23 as experience data
    m_encParameters.minQpAllowed        = 20; // so we narrow qp range down to 20-26 to make latively stable quality

    //lower the latency
    m_encParameters.sceneChangeDetection    = 0;
    m_encParameters.lookAheadDistance       = 0;
    m_encParameters.predStructure           = 0; //0: Low Delay P frame, reduced client decoder latency

    //performance tuning
    m_encParameters.targetSocket = (m_encParameters.sourceWidth == 7680 || m_encParameters.sourceWidth == 3840); //bind HR encode to cpu 1
    m_encParameters.switchThreadsToRtPriority   = 0; //do not use realtime thread. it seemes unfair to decoding/stitching/networking
    //m_encParameters.disableDlfFlag          = 1; //disable de-block feature may improve ~0.5 fps (by sacrifice video quality?)
}

void SVTHEVCEncoderBase::updateMCTSParameters(const EB_H265_ENC_CONFIGURATION conf)
{
    // Encoding Preset
    m_encParameters.encMode             = conf.encMode;
    m_encParameters.tune                = conf.tune ? conf.tune : 1;
    m_encParameters.asmType             = conf.asmType ? conf.asmType : 1;

    // GOP Structure
    m_encParameters.intraPeriodLength   = conf.intraPeriodLength ? conf.intraPeriodLength : 5 - 1;  //GoP=5
    m_encParameters.intraRefreshType    = conf.intraRefreshType ? conf.intraRefreshType : 0;
    m_encParameters.hierarchicalLevels  = conf.hierarchicalLevels ? conf.hierarchicalLevels : 3;
    m_encParameters.predStructure       = conf.predStructure ? conf.predStructure : 0;

    // Input Info
    m_encParameters.sourceWidth         = conf.sourceWidth;
    m_encParameters.sourceHeight        = conf.sourceHeight;
    m_encParameters.frameRate           = conf.frameRate << 16;
    m_encParameters.encoderBitDepth     = conf.encoderBitDepth ? conf.encoderBitDepth : 8;
    m_encParameters.encoderColorFormat  = static_cast<EB_COLOR_FORMAT>(conf.encoderColorFormat ? conf.encoderColorFormat : 1);

    // Rate Control
    m_encParameters.rateControlMode      = conf.rateControlMode ? conf.rateControlMode : 1; //0 : CQP , 1 : VBR
    m_encParameters.sceneChangeDetection = conf.sceneChangeDetection ? conf.sceneChangeDetection : 0;
    m_encParameters.lookAheadDistance    = conf.lookAheadDistance ? conf.lookAheadDistance : 0;
    m_encParameters.targetBitRate        = conf.targetBitRate;
    m_encParameters.maxQpAllowed         = conf.maxQpAllowed ? conf.maxQpAllowed : 26; // we heard TV broadcasting is using qp=23 as experience data
    m_encParameters.minQpAllowed         = conf.minQpAllowed ? conf.minQpAllowed : 20; // so we narrow qp range down to 20-26 to make latively stable quality

    // Bitstream Options
    m_encParameters.profile              = conf.profile ? conf.profile : 2;
    m_encParameters.level                = conf.level ? conf.level : 0;
    m_encParameters.tier                 = conf.tier ? conf.tier : 0;

    // Deblocking
    m_encParameters.disableDlfFlag       = conf.disableDlfFlag ? conf.disableDlfFlag : 0;

    // SAO
    m_encParameters.enableSaoFlag        = conf.enableSaoFlag ? conf.enableSaoFlag : 0;

    //performance tuning
    m_encParameters.targetSocket = conf.targetSocket? conf.targetSocket : 1; //bind HR encode to cpu 1
    m_encParameters.switchThreadsToRtPriority = conf.switchThreadsToRtPriority? conf.switchThreadsToRtPriority : 0; //do not use realtime thread. it seemes unfair to decoding/stitching/networking
    m_encParameters.unrestrictedMotionVector    = conf.unrestrictedMotionVector? conf.unrestrictedMotionVector : 0;
    m_encParameters.tileSliceMode               = conf.tileSliceMode? conf.tileSliceMode : 1;
    m_encParameters.tileColumnCount             = conf.tileColumnCount;
    m_encParameters.tileRowCount                = conf.tileRowCount;
}

void SVTHEVCEncoderBase::setMCTSParameters(uint32_t tiles_col, uint32_t tiles_row)
{
    if (tiles_col * tiles_row > 1) {
        m_encParameters.unrestrictedMotionVector    = 0;
        m_encParameters.tileSliceMode               = 1;
        m_encParameters.tileColumnCount             = tiles_col;
        m_encParameters.tileRowCount                = tiles_row;
#if 0
        m_encParameters.hierarchicalLevels          = 3;

        m_encParameters.disableDlfFlag              = 1;

        m_encParameters.bitRateReduction            = 0;
        m_encParameters.improveSharpness            = 0;
#endif
        ELOG_DEBUG_T("set MCTS, tiles_col=%d, tiles_row=%d",
                tiles_col, tiles_row);
    }
}

bool SVTHEVCEncoderBase::init(uint32_t width, uint32_t height, uint32_t frameRate,
                              uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds,
                              uint32_t tiles_col, uint32_t tiles_row, uint32_t encMode)
{
    EB_ERRORTYPE return_error = EB_ErrorNone;

    ELOG_DEBUG_T("initEncoder: width=%d, height=%d, frameRate=%d, bitrateKbps=%d, .keyFrameIntervalSeconds=%d}"
            , width, height, frameRate, bitrateKbps, keyFrameIntervalSeconds);

    return_error = EbInitHandle(&m_handle, this, &m_encParameters);
    if (return_error != EB_ErrorNone) {
        ELOG_ERROR_T("InitHandle failed, ret 0x%x", return_error);

        m_handle = NULL;
        return false;
    } else {
        ELOG_INFO_T("EbInitHandle ok");
    }

    //initDefaultParameters();
    updateParameters(width, height, frameRate, bitrateKbps, keyFrameIntervalSeconds, encMode);
    setMCTSParameters(tiles_col, tiles_row);

    return_error = EbH265EncSetParameter(m_handle, &m_encParameters);
    if (return_error != EB_ErrorNone) {
        ELOG_ERROR_T("SetParameter failed, ret 0x%x", return_error);

        EbDeinitHandle(m_handle);
        m_handle = NULL;
        return false;
    }

    return_error = EbInitEncoder(m_handle);
    if (return_error != EB_ErrorNone) {
        ELOG_ERROR_T("InitEncoder failed, ret 0x%x", return_error);

        EbDeinitHandle(m_handle);
        m_handle = NULL;
        return false;
    }

    if (!allocateBuffers()) {
        ELOG_ERROR_T("allocateBuffers failed");

        deallocateBuffers();
        EbDeinitEncoder(m_handle);
        EbDeinitHandle(m_handle);
        m_handle = NULL;
        return false;
    }

    if (m_enableBsDump) {
        char dumpFileName[128];

        snprintf(dumpFileName, 128, "/tmp/SVTHEVCEncoderBase-%dx%d-%p.%s", width, height, this, "hevc");
        m_bsDumpfp = fopen(dumpFileName, "wb");
        if (m_bsDumpfp) {
            ELOG_INFO_T("Enable bitstream dump, %s", dumpFileName);
        } else {
            ELOG_INFO_T("Can not open dump file, %s", dumpFileName);
        }
    }

    m_thread.reset(new boost::thread(&SVTHEVCEncoderBase::encoding_loop, this));

    return true;
}

bool SVTHEVCEncoderBase::init(const EB_H265_ENC_CONFIGURATION& conf)
{
    EB_ERRORTYPE return_error = EB_ErrorNone;

    return_error = EbInitHandle(&m_handle, this, &m_encParameters);
    if (return_error != EB_ErrorNone) {
        ELOG_ERROR_T("InitHandle failed, ret 0x%x", return_error);

        m_handle = NULL;
        return false;
    } else {
        ELOG_INFO_T("EbInitHandle ok");
    }

    updateMCTSParameters(conf);

    return_error = EbH265EncSetParameter(m_handle, &m_encParameters);
    if (return_error != EB_ErrorNone) {
        ELOG_ERROR_T("SetParameter failed, ret 0x%x", return_error);

        EbDeinitHandle(m_handle);
        m_handle = NULL;
        return false;
    }

    return_error = EbInitEncoder(m_handle);
    if (return_error != EB_ErrorNone) {
        ELOG_ERROR_T("InitEncoder failed, ret 0x%x", return_error);

        EbDeinitHandle(m_handle);
        m_handle = NULL;
        return false;
    }

    if (!allocateBuffers()) {
        ELOG_ERROR_T("allocateBuffers failed");

        deallocateBuffers();
        EbDeinitEncoder(m_handle);
        EbDeinitHandle(m_handle);
        m_handle = NULL;
        return false;
    }

    if (m_enableBsDump) {
        char dumpFileName[128];

        snprintf(dumpFileName, 128, "/tmp/SVTHEVCEncoderBase-%dx%d-%p.%s", conf.sourceWidth, conf.sourceHeight, this, "hevc");
        m_bsDumpfp = fopen(dumpFileName, "wb");
        if (m_bsDumpfp) {
            ELOG_INFO_T("Enable bitstream dump, %s", dumpFileName);
        } else {
            ELOG_INFO_T("Can not open dump file, %s", dumpFileName);
        }
    }

    m_thread.reset(new boost::thread(&SVTHEVCEncoderBase::encoding_loop, this));

    return true;
}

void SVTHEVCEncoderBase::encoding_loop()
{
    ELOG_INFO_T("Enter loop");

    while(!m_quit) {
        boost::shared_ptr<webrtc::VideoFrame> video_frame;
        {
            boost::mutex::scoped_lock lock(m_queue_mutex);
            while (m_frame_queue.size() == 0) {
                m_queue_cond.wait(lock);
                if (m_quit)
                    goto exit;
            }
            video_frame = m_frame_queue.front();
            m_frame_queue.pop();
        }

        encodeVideoFrame(video_frame);
    }

exit:
    ELOG_INFO_T("sending EOS"); //https://github.com/OpenVisualCloud/SVT-HEVC/issues/535
    m_inputBuffer.nAllocLen = 0;
    m_inputBuffer.nFilledLen = 0;
    m_inputBuffer.nTickCount = 0;
    m_inputBuffer.pBuffer = NULL;
    m_inputBuffer.nFlags = EB_BUFFERFLAG_EOS;
    int return_error = EbH265EncSendPicture(m_handle, &m_inputBuffer);
    if (return_error != EB_ErrorNone) {
        ELOG_ERROR_T("send EOS failed, ret 0x%x", return_error);
    } else {
        ELOG_INFO_T("send EOS done");
    }
    usleep(1000);//1ms

    ELOG_INFO_T("Exit loop");
}

bool SVTHEVCEncoderBase::sendFrame(const Frame& frame)
{
    switch (frame.format) {
        case FRAME_FORMAT_I420: {
            boost::shared_ptr<webrtc::VideoFrame> videoFrame =
                boost::make_shared<webrtc::VideoFrame>(*reinterpret_cast<webrtc::VideoFrame*>(frame.payload));
            return sendVideoFrame(videoFrame);
        }

        default:
            ELOG_ERROR_T("Unspported video frame format %s(%d)",
                         getFormatStr(frame.format), frame.format);
            return false;
    }
}

bool SVTHEVCEncoderBase::sendVideoFrame(boost::shared_ptr<webrtc::VideoFrame> videoFrame)
{
    boost::mutex::scoped_lock lock(m_queue_mutex);
    m_frame_queue.push(videoFrame);
    if (m_frame_queue.size() >= 1)
        m_queue_cond.notify_all();

    return true;
}

int SVTHEVCEncoderBase::frame_queue_size()
{
    boost::mutex::scoped_lock lock(m_queue_mutex);
    return m_frame_queue.size();
}

bool SVTHEVCEncoderBase::encodeVideoFrame(boost::shared_ptr<webrtc::VideoFrame> videoFrame)
{
    rtc::scoped_refptr<webrtc::VideoFrameBuffer> videoBuffer = videoFrame->video_frame_buffer();
    return encodeVideoBuffer(videoBuffer, videoFrame->timestamp_us() / 1000);
}

bool SVTHEVCEncoderBase::encodeVideoBuffer(rtc::scoped_refptr<webrtc::VideoFrameBuffer> videoBuffer,
                                           int64_t pts)
{
    int ret;

    if ((uint32_t)videoBuffer->width() == m_encParameters.sourceWidth
            && (uint32_t)videoBuffer->height() == m_encParameters.sourceHeight) {
        LTTNG_TRACE_INFO("SVTHEVCEncoderBase: Send HR-pic to HR encoder, pts(%ld)"
                , pts);
        return encodePicture(videoBuffer->DataY(), videoBuffer->StrideY(),
                videoBuffer->DataU(), videoBuffer->StrideU(),
                videoBuffer->DataV(), videoBuffer->StrideV(),
                pts);
    } else {
        if (!m_scaling_frame_buffer) {
            m_scaling_frame_buffer =
                (uint8_t *)malloc(m_encParameters.sourceWidth * m_encParameters.sourceHeight * 3 / 2);
        }

        LTTNG_TRACE_INFO("SVTHEVCEncoderBase: Down scale start, pts(%ld)", pts);
        ret = libyuv::I420Scale(
                videoBuffer->DataY(),   videoBuffer->StrideY(),
                videoBuffer->DataU(),   videoBuffer->StrideU(),
                videoBuffer->DataV(),   videoBuffer->StrideV(),
                videoBuffer->width(),   videoBuffer->height(),
                m_scaling_frame_buffer,
                m_encParameters.sourceWidth,
                m_scaling_frame_buffer + m_encParameters.sourceWidth * m_encParameters.sourceHeight,
                m_encParameters.sourceWidth / 2,
                m_scaling_frame_buffer + m_encParameters.sourceWidth * m_encParameters.sourceHeight * 5 / 4,
                m_encParameters.sourceWidth / 2,
                m_encParameters.sourceWidth, m_encParameters.sourceHeight,
                libyuv::kFilterBox);
        if (ret != 0) {
            ELOG_ERROR_T("Convert frame failed(%d), %dx%d -> %dx%d", ret
                    , videoBuffer->width()
                    , videoBuffer->height()
                    , m_encParameters.sourceWidth
                    , m_encParameters.sourceHeight
                    );
            return false;
        }
        LTTNG_TRACE_INFO("SVTHEVCEncoderBase: Down scale done, pts(%ld)", pts);
        LTTNG_TRACE_INFO("SVTHEVCEncoderBase: Send LR-pic to LR encoder, pts(%ld)", pts);
        return encodePicture(
                m_scaling_frame_buffer,
                m_encParameters.sourceWidth,
                m_scaling_frame_buffer + m_encParameters.sourceWidth * m_encParameters.sourceHeight,
                m_encParameters.sourceWidth / 2,
                m_scaling_frame_buffer + m_encParameters.sourceWidth * m_encParameters.sourceHeight * 5 / 4,
                m_encParameters.sourceWidth / 2,
                pts);
    }
}

bool SVTHEVCEncoderBase::encodePicture(const uint8_t *y_plane, uint32_t y_stride,
        const uint8_t *u_plane, uint32_t u_stride,
        const uint8_t *v_plane, uint32_t v_stride,
        int64_t pts)
{
    // frame buffer
    m_inputFrameBuffer.luma = const_cast<uint8_t *>(y_plane);
    m_inputFrameBuffer.cb = const_cast<uint8_t *>(u_plane);
    m_inputFrameBuffer.cr = const_cast<uint8_t *>(v_plane);

    m_inputFrameBuffer.yStride   = y_stride;
    m_inputFrameBuffer.crStride  = u_stride;
    m_inputFrameBuffer.cbStride  = v_stride;

    m_inputBuffer.pts = pts;
    m_inputBuffer.dts = pts;

    // frame type
    if (m_forceIDR) {
        m_inputBuffer.sliceType = EB_IDR_PICTURE;
        m_forceIDR = false;

        ELOG_DEBUG_T("encodePicture, IDR");
    } else {
        m_inputBuffer.sliceType = EB_INVALID_PICTURE;
    }

    int return_error = EbH265EncSendPicture(m_handle, &m_inputBuffer);
    if (return_error != EB_ErrorNone) {
        ELOG_ERROR_T("encodePicture failed, ret 0x%x", return_error);
        return false;
    }

    return true;
}

EB_BUFFERHEADERTYPE *SVTHEVCEncoderBase::getOutBuffer(void)
{
    EB_BUFFERHEADERTYPE *pOutBuffer = &m_outputBuffer;
    int return_error;

    return_error = EbH265GetPacket(m_handle, &pOutBuffer , false);
    if (return_error == EB_ErrorMax) {
        ELOG_ERROR_T("Error while encoding, code 0x%x", pOutBuffer->nFlags);
        return NULL;
    } else if (return_error != EB_NoErrorEmptyQueue) {
        ELOG_TRACE_T("nFilledLen(%d), nTickCount %d(ms), dts(%ld), pts(%ld), nFlags(0x%x), qpValue(%d), sliceType(%d)"
                , pOutBuffer->nFilledLen
                , pOutBuffer->nTickCount
                , pOutBuffer->dts
                , pOutBuffer->pts
                , pOutBuffer->nFlags
                , pOutBuffer->qpValue
                , pOutBuffer->sliceType
                );

        dump(pOutBuffer->pBuffer, pOutBuffer->nFilledLen);

        return pOutBuffer;
    } else {
        return NULL;
    }
}

void SVTHEVCEncoderBase::releaseOutBuffer(EB_BUFFERHEADERTYPE *pOutBuffer)
{
    EbH265ReleaseOutBuffer(&pOutBuffer);
}

boost::shared_ptr<SVTHEVCEncodedPacket> SVTHEVCEncoderBase::getEncodedPacket(void)
{
    EB_BUFFERHEADERTYPE *pOutBuffer = getOutBuffer();
    if (!pOutBuffer)
        return NULL;

    boost::shared_ptr<SVTHEVCEncodedPacket> encoded_pkt =
        boost::make_shared<SVTHEVCEncodedPacket>(pOutBuffer);
    releaseOutBuffer(pOutBuffer);
    return encoded_pkt;
}

void SVTHEVCEncoderBase::requestKeyFrame()
{
    ELOG_DEBUG_T("%s", __FUNCTION__);

    m_forceIDR = true;
}

bool SVTHEVCEncoderBase::allocateBuffers()
{
    memset(&m_inputFrameBuffer, 0, sizeof(m_inputFrameBuffer));
    memset(&m_inputBuffer, 0, sizeof(m_inputBuffer));
    memset(&m_outputBuffer, 0, sizeof(m_outputBuffer));

    // output buffer
    m_inputBuffer.nSize        = sizeof(EB_BUFFERHEADERTYPE);
    m_inputBuffer.nAllocLen    = 0;
    m_inputBuffer.pAppPrivate  = NULL;
    m_inputBuffer.sliceType    = EB_INVALID_PICTURE;
    m_inputBuffer.pBuffer      = (unsigned char *)&m_inputFrameBuffer;

    // output buffer
    size_t outputStreamBufferSize = m_encParameters.sourceWidth * m_encParameters.sourceHeight * 2;
    m_outputBuffer.nSize = sizeof(EB_BUFFERHEADERTYPE);
    m_outputBuffer.nAllocLen   = outputStreamBufferSize;
    m_outputBuffer.pAppPrivate = this;
    m_outputBuffer.sliceType   = EB_INVALID_PICTURE;
    m_outputBuffer.pBuffer = (unsigned char *)malloc(outputStreamBufferSize);
    if (!m_outputBuffer.pBuffer) {
        ELOG_ERROR_T("Can not alloc mem, size(%ld)", outputStreamBufferSize);
        return false;
    }

    return true;
}

void SVTHEVCEncoderBase::deallocateBuffers()
{
    if (!m_outputBuffer.pBuffer) {
        free(m_outputBuffer.pBuffer);
        m_outputBuffer.pBuffer = NULL;
    }
}

void SVTHEVCEncoderBase::dump(uint8_t *buf, int len)
{
    if (m_bsDumpfp) {
        fwrite(buf, 1, len, m_bsDumpfp);
    }
}

} // namespace owt_base
