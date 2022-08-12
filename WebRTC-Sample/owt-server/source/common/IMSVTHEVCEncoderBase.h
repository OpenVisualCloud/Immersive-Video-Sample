// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SVTHEVCEncoderBase_h
#define SVTHEVCEncoderBase_h

#include <queue>

#include <boost/make_shared.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <webrtc/api/video/video_frame.h>
#include <webrtc/api/video/video_frame_buffer.h>

#include "logger.h"
//#include "lttng_logger.h"
#include "MediaFramePipeline.h"

#include "svt-hevc/EbApi.h"

namespace owt_base {

class SVTHEVCEncodedPacket {
public:
    SVTHEVCEncodedPacket(EB_BUFFERHEADERTYPE* pBuffer);
    ~SVTHEVCEncodedPacket();

    uint8_t *data;
    int32_t length;
    bool isKey;
    int64_t pts;

private:
    boost::shared_array<uint8_t> m_array;
};

class SVTHEVCEncoderBase {
    DECLARE_LOGGER();

public:
    SVTHEVCEncoderBase();
    ~SVTHEVCEncoderBase();

    bool init(uint32_t width, uint32_t height, uint32_t frameRate,
              uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds,
              uint32_t tiles_col, uint32_t tiles_row,  uint32_t encMode);

    bool init(const EB_H265_ENC_CONFIGURATION& conf);

    bool sendFrame(const Frame& frame);
    bool sendVideoFrame(boost::shared_ptr<webrtc::VideoFrame> video_frame);

    boost::shared_ptr<SVTHEVCEncodedPacket> getEncodedPacket(void);

    void requestKeyFrame();

    void encoding_loop();
    int frame_queue_size();

protected:
    void initDefaultParameters();
    void updateParameters(uint32_t width, uint32_t height, uint32_t frameRate,
                          uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds,  uint32_t encMode);
    void setMCTSParameters(uint32_t tiles_col, uint32_t tiles_row);
    void updateMCTSParameters(const EB_H265_ENC_CONFIGURATION conf);

    bool allocateBuffers();
    void deallocateBuffers();

    EB_BUFFERHEADERTYPE *getOutBuffer(void);
    void releaseOutBuffer(EB_BUFFERHEADERTYPE *pOutBuffer);

    bool encodeVideoFrame(boost::shared_ptr<webrtc::VideoFrame> videoFrame);
    bool encodeVideoBuffer(rtc::scoped_refptr<webrtc::VideoFrameBuffer> video_buffer, int64_t pts);
    bool encodePicture(const uint8_t *y_plane, uint32_t y_stride,
            const uint8_t *u_plane, uint32_t u_stride,
            const uint8_t *v_plane, uint32_t v_stride,
            int64_t pts);

    void dump(uint8_t *buf, int len);

private:
    EB_COMPONENTTYPE            *m_handle;
    EB_H265_ENC_CONFIGURATION   m_encParameters;

    EB_H265_ENC_INPUT m_inputFrameBuffer;
    EB_BUFFERHEADERTYPE m_inputBuffer;
    EB_BUFFERHEADERTYPE m_outputBuffer;

    uint8_t *m_scaling_frame_buffer;
    bool m_forceIDR;

    boost::scoped_ptr<boost::thread> m_thread;
    std::queue<boost::shared_ptr<webrtc::VideoFrame>> m_frame_queue;
    boost::mutex m_queue_mutex;
    boost::condition_variable m_queue_cond;
    bool m_quit;

    bool m_enableBsDump;
    FILE *m_bsDumpfp;
};

} /* namespace owt_base */
#endif /* SVTHEVCEncoderBase_h */
