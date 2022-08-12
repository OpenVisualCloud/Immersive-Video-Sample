// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "IMSVTHEVCEncoder.h"

#include <webrtc/api/video/video_frame.h>
#include <webrtc/api/video/video_frame_buffer.h>

#include <libyuv/convert.h>
#include <libyuv/planar_functions.h>
#include <libyuv/scale.h>

#include "MediaUtilities.h"

namespace owt_base {

DEFINE_LOGGER(SVTHEVCEncoder, "owt.SVTHEVCEncoder");

SVTHEVCEncoder::SVTHEVCEncoder(FrameFormat format, VideoCodecProfile profile, bool useSimulcast)
    : m_encoderReady(false)
    , m_dest(NULL)
    , m_width(0)
    , m_height(0)
    , m_frameRate(0)
    , m_bitrateKbps(0)
    , m_keyFrameIntervalSeconds(0)
    , m_encoded_frame_count(0)
{
    m_hevc_encoder = boost::make_shared<SVTHEVCEncoderBase>();

    m_srv       = boost::make_shared<boost::asio::io_service>();
    m_srvWork   = boost::make_shared<boost::asio::io_service::work>(*m_srv);
    m_thread    = boost::make_shared<boost::thread>(boost::bind(&boost::asio::io_service::run, m_srv));
}

SVTHEVCEncoder::~SVTHEVCEncoder()
{
    boost::unique_lock<boost::shared_mutex> ulock(m_mutex);

    m_srvWork.reset();
    m_srv->stop();
    m_thread.reset();
    m_srv.reset();

    m_dest = NULL;
}

bool SVTHEVCEncoder::canSimulcast(FrameFormat format, uint32_t width, uint32_t height)
{
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    return false;
}

bool SVTHEVCEncoder::isIdle()
{
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    return (m_dest == NULL);
}

bool SVTHEVCEncoder::initEncoder(uint32_t width, uint32_t height, uint32_t frameRate,
                                 uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds)
{
    m_encoderReady = m_hevc_encoder->init(width, height, frameRate, bitrateKbps, keyFrameIntervalSeconds, 0, 0, 9);
    return m_encoderReady;
}

bool SVTHEVCEncoder::initEncoderAsync(uint32_t width, uint32_t height, uint32_t frameRate,
                                      uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds)
{
    m_srv->post(boost::bind(&SVTHEVCEncoder::initEncoder, this, width, height, frameRate, bitrateKbps, keyFrameIntervalSeconds));
    return true;
}

int32_t SVTHEVCEncoder::generateStream(uint32_t width, uint32_t height, uint32_t frameRate,
                                       uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds,
                                       owt_base::FrameDestination* dest)
{
    boost::unique_lock<boost::shared_mutex> ulock(m_mutex);

    ELOG_INFO_T("generateStream: {.width=%d, .height=%d, .frameRate=%d, .bitrateKbps=%d, .keyFrameIntervalSeconds=%d}"
            , width, height, frameRate, bitrateKbps, keyFrameIntervalSeconds);

    if (m_dest) {
        ELOG_ERROR_T("Only support one stream!");
        return -1;
    }

    m_width = width;
    m_height = height;
    m_frameRate = frameRate;
    m_bitrateKbps = bitrateKbps;
    m_keyFrameIntervalSeconds = keyFrameIntervalSeconds;

    if (m_width != 0 && m_height != 0) {
        if (!initEncoderAsync(m_width, m_height, m_frameRate, m_bitrateKbps, m_keyFrameIntervalSeconds))
            return -1;
    }

    m_dest = dest;
    addVideoDestination(m_dest);

    return 0;
}

void SVTHEVCEncoder::degenerateStream(int32_t streamId)
{
    boost::unique_lock<boost::shared_mutex> ulock(m_mutex);

    ELOG_DEBUG_T("degenerateStream");

    assert(m_dest != NULL);
    removeVideoDestination(m_dest);
    m_dest = NULL;
}

void SVTHEVCEncoder::setBitrate(unsigned short kbps, int32_t streamId)
{
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    ELOG_WARN_T("%s", __FUNCTION__);
}

void SVTHEVCEncoder::requestKeyFrame(int32_t streamId)
{
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    ELOG_DEBUG_T("%s", __FUNCTION__);

    m_hevc_encoder->requestKeyFrame();
}

void SVTHEVCEncoder::onFrame(const Frame& frame)
{
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    int32_t ret;

    if (m_dest == NULL) {
        return;
    }

    if (m_width == 0 || m_height == 0) {
        m_width = frame.additionalInfo.video.width;
        m_height = frame.additionalInfo.video.height;

        if (m_bitrateKbps == 0)
            m_bitrateKbps = calcBitrate(m_width, m_height, m_frameRate);

        if (!initEncoderAsync(m_width, m_height, m_frameRate, m_bitrateKbps, m_keyFrameIntervalSeconds)) {
            return;
        }
    }

    if (!m_encoderReady) {
        ELOG_WARN_T("Encoder not ready!");
        return;
    }

    ret = m_hevc_encoder->sendFrame(frame);
    if (!ret) {
        ELOG_ERROR_T("SendPicture failed");
        return;
    }

    while (true) {
        boost::shared_ptr<SVTHEVCEncodedPacket> encoded_pkt = m_hevc_encoder->getEncodedPacket();
        if(!encoded_pkt)
            break;

        m_packet_queue.push(encoded_pkt);
    }

    while (m_packet_queue.size() > 0) {
        boost::shared_ptr<SVTHEVCEncodedPacket> pkt = m_packet_queue.front();
        m_packet_queue.pop();
        deliverVideoFrame(pkt);
    }
}

void SVTHEVCEncoder::deliverVideoFrame(boost::shared_ptr<SVTHEVCEncodedPacket> encoded_pkt)
{
    Frame outFrame;
    memset(&outFrame, 0, sizeof(outFrame));
    outFrame.format     = FRAME_FORMAT_H265;
    outFrame.payload    = encoded_pkt->data;
    outFrame.length     = encoded_pkt->length;
    outFrame.timeStamp = (m_encoded_frame_count++) * 1000 / m_frameRate * 90;
    outFrame.additionalInfo.video.width         = m_width;
    outFrame.additionalInfo.video.height        = m_height;
    outFrame.additionalInfo.video.isKeyFrame    = encoded_pkt->isKey;

    ELOG_TRACE_T("deliverFrame, %s, %dx%d(%s), length(%d)",
            getFormatStr(outFrame.format),
            outFrame.additionalInfo.video.width,
            outFrame.additionalInfo.video.height,
            outFrame.additionalInfo.video.isKeyFrame ? "key" : "delta",
            outFrame.length);

    deliverFrame(outFrame);
}

} // namespace owt_base
