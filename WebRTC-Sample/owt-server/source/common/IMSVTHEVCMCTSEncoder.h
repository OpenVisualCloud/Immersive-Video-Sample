// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SVTHEVCMCTSEncoder_h
#define SVTHEVCMCTSEncoder_h

#include <queue>

#include <boost/make_shared.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <webrtc/system_wrappers/include/clock.h>

#include "logger.h"
//#include "lttng_logger.h"
#include "MediaFramePipeline.h"
#include "IMSVTHEVCEncoderBase.h"

namespace owt_base {

static inline int64_t currentTimeMs()
{
    timeval time;
    gettimeofday(&time, nullptr);
    return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

class SVTHEVCMCTSEncoder : public VideoFrameEncoder, public FrameSource {
    DECLARE_LOGGER();

    const uint32_t OverflowThresholdFrames = 5;
    const uint32_t OverflowThresholdAllFrames = 100;

public:
    SVTHEVCMCTSEncoder(FrameFormat format, VideoCodecProfile profile, bool useSimulcast = false);
    ~SVTHEVCMCTSEncoder();

    FrameFormat getInputFormat() {return FRAME_FORMAT_I420;}

    // Implements VideoFrameEncoder.
    void onFrame(const Frame&);
    bool canSimulcast(FrameFormat format, uint32_t width, uint32_t height);
    bool isIdle();
    int32_t generateStream(uint32_t width, uint32_t height, uint32_t frameRate,
                           uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds,
                           FrameDestination* dest);
    void degenerateStream(int32_t streamId);
    void setBitrate(unsigned short kbps, int32_t streamId);
    void requestKeyFrame(int32_t streamId);

protected:
    bool initEncoder(uint32_t width, uint32_t height, uint32_t frameRate,
                     uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds);
    bool initEncoderAsync(uint32_t width, uint32_t height, uint32_t frameRate,
                          uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds);
    void fetchOutput();
    void deliverOutput();
    void deliverMCTSFrame(boost::shared_ptr<SVTHEVCEncodedPacket> hi_res_pkt,
                         boost::shared_ptr<SVTHEVCEncodedPacket> low_res_pkt);
    bool loadConf(const std::string name, EB_H265_ENC_CONFIGURATION& conf);

private:
    bool                        m_encoderReady;
    FrameDestination            *m_dest;

    uint32_t m_width_hi;
    uint32_t m_height_hi;
    uint32_t m_width_low;
    uint32_t m_height_low;
    uint32_t m_frameRate;
    uint32_t m_bitrateKbps;
    uint32_t m_keyFrameIntervalSeconds;

    uint32_t m_encodedFrameCount;

    boost::shared_ptr<SVTHEVCEncoderBase> m_hi_res_encoder;
    boost::shared_ptr<SVTHEVCEncoderBase> m_low_res_encoder;

    boost::shared_mutex m_mutex;

    std::queue<boost::shared_ptr<SVTHEVCEncodedPacket>> m_hi_res_packet_queue;
    std::queue<boost::shared_ptr<SVTHEVCEncodedPacket>> m_low_res_packet_queue;

    uint8_t *m_payload_buffer;
    uint32_t m_payload_buffer_length;

    boost::shared_ptr<boost::asio::io_service> m_srv;
    boost::shared_ptr<boost::asio::io_service::work> m_srvWork;
    boost::shared_ptr<boost::thread> m_thread;

    int64_t m_stat_timestamp;
    int32_t m_stat_frame_count;

    const webrtc::Clock *m_clock;

    uint32_t m_max_output_frames;
    uint32_t m_stat_overflow_frames;
    uint32_t m_stat_overflow_all_frames;

    std::string m_conf_file;
};

} /* namespace owt_base */
#endif /* SVTHEVCMCTSEncoder_h */
