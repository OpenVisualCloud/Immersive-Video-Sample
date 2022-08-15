// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "IMSVTHEVCMCTSEncoder.h"

#include <webrtc/api/video/video_frame.h>
#include <webrtc/api/video/video_frame_buffer.h>

#include <libyuv/convert.h>
#include <libyuv/planar_functions.h>
#include <libyuv/scale.h>
#include <boost/filesystem.hpp>

#include "MediaUtilities.h"
#include "yaml-cpp/yaml.h"

#define LTTNG_TRACE_INFO    ELOG_TRACE_T
#define LTTNG_TRACE_WARNING ELOG_INFO_T
#define NONE_NULL_ASSIGN(assignee, assignerr) \
if (assignerr) { \
    assignee = assignerr.as<uint32_t>(); \
    ELOG_INFO_T("%s : %u", #assignee, assignee); \
}

namespace owt_base {

DEFINE_LOGGER(SVTHEVCMCTSEncoder, "owt.SVTHEVCMCTSEncoder");

SVTHEVCMCTSEncoder::SVTHEVCMCTSEncoder(FrameFormat format, VideoCodecProfile profile, bool useSimulcast)
    : m_encoderReady(false)
    , m_dest(NULL)
    , m_width_hi(0)
    , m_height_hi(0)
    , m_width_low(0)
    , m_height_low(0)
    , m_frameRate(0)
    , m_bitrateKbps(0)
    , m_keyFrameIntervalSeconds(0)
    , m_payload_buffer(NULL)
    , m_payload_buffer_length(0)
    , m_stat_timestamp(0)
    , m_stat_frame_count(0)
    , m_clock(webrtc::Clock::GetRealTimeClock())
    , m_max_output_frames(3)
    , m_stat_overflow_frames(0)
    , m_stat_overflow_all_frames(0)
    , m_conf_file("mcts_encoder.yaml")
{
    m_hi_res_encoder = boost::make_shared<SVTHEVCEncoderBase>();
    m_low_res_encoder = boost::make_shared<SVTHEVCEncoderBase>();

    m_srv       = boost::make_shared<boost::asio::io_service>();
    m_srvWork   = boost::make_shared<boost::asio::io_service::work>(*m_srv);
    m_thread    = boost::make_shared<boost::thread>(boost::bind(&boost::asio::io_service::run, m_srv));
}

SVTHEVCMCTSEncoder::~SVTHEVCMCTSEncoder()
{
    m_srvWork.reset();
    m_srv->stop();
    m_srv.reset();
    m_thread.reset();

    if (m_payload_buffer) {
        free(m_payload_buffer);
    }

    m_dest = NULL;
}

bool SVTHEVCMCTSEncoder::canSimulcast(FrameFormat format, uint32_t width, uint32_t height)
{
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    return false;
}

bool SVTHEVCMCTSEncoder::isIdle()
{
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    return (m_dest == NULL);
}

bool SVTHEVCMCTSEncoder::loadConf(const std::string name, EB_H265_ENC_CONFIGURATION& conf)
{

    YAML::Node cfg;
    try {
        cfg = YAML::LoadFile(m_conf_file);
    } catch (...) {
        ELOG_ERROR_T("load config file %s fail, file error or not exist", m_conf_file.c_str());
        return false;
    }

    if (!cfg[name]) {
        ELOG_ERROR_T("%s part option not exist", name.c_str());
        return false;
    }

    // Encoding Preset
    NONE_NULL_ASSIGN(conf.encMode, cfg[name]["encMode"])
    NONE_NULL_ASSIGN(conf.tune, cfg[name]["tune"])
    NONE_NULL_ASSIGN(conf.asmType, cfg[name]["asmType"])

    // GOP Structure
    NONE_NULL_ASSIGN(conf.intraPeriodLength, cfg[name]["intraPeriodLength"])
    NONE_NULL_ASSIGN(conf.intraRefreshType, cfg[name]["intraRefreshType"])
    NONE_NULL_ASSIGN(conf.hierarchicalLevels, cfg[name]["hierarchicalLevels"])
    NONE_NULL_ASSIGN(conf.predStructure, cfg[name]["predStructure"])

    // Input Info
    NONE_NULL_ASSIGN(conf.sourceWidth, cfg[name]["sourceWidth"])
    NONE_NULL_ASSIGN(conf.sourceHeight, cfg[name]["sourceHeight"])
    NONE_NULL_ASSIGN(conf.frameRate, cfg[name]["frameRate"])
    NONE_NULL_ASSIGN(conf.encoderBitDepth, cfg[name]["encoderBitDepth"])
    if (cfg[name]["encoderColorFormat"]) {
        conf.encoderColorFormat = static_cast<EB_COLOR_FORMAT>(cfg[name]["encoderColorFormat"].as<uint32_t>());
        ELOG_INFO_T("%s : %u", "conf.encoderColorFormat", conf.encoderColorFormat);
    }

    // Rate Control
    NONE_NULL_ASSIGN(conf.rateControlMode, cfg[name]["rateControlMode"])
    NONE_NULL_ASSIGN(conf.sceneChangeDetection, cfg[name]["sceneChangeDetection"])
    NONE_NULL_ASSIGN(conf.lookAheadDistance, cfg[name]["lookAheadDistance"])
    NONE_NULL_ASSIGN(conf.targetBitRate, cfg[name]["targetBitRate"])
    NONE_NULL_ASSIGN(conf.maxQpAllowed, cfg[name]["maxQpAllowed"])
    NONE_NULL_ASSIGN(conf.minQpAllowed, cfg[name]["minQpAllowed"])

    // Bitstream Options
    NONE_NULL_ASSIGN(conf.profile, cfg[name]["profile"])
    NONE_NULL_ASSIGN(conf.tier, cfg[name]["tier"])
    NONE_NULL_ASSIGN(conf.level, cfg[name]["level"])

    // Deblock Filter
    NONE_NULL_ASSIGN(conf.disableDlfFlag, cfg[name]["disableDlfFlag"])

    NONE_NULL_ASSIGN(conf.enableSaoFlag, cfg[name]["enableSaoFlag"])

    NONE_NULL_ASSIGN(conf.targetSocket, cfg[name]["targetSocket"])

    // Tile Info
    NONE_NULL_ASSIGN(conf.tileColumnCount, cfg[name]["tileColumnCount"])
    NONE_NULL_ASSIGN(conf.tileRowCount, cfg[name]["tileRowCount"])

    return true;
}

bool SVTHEVCMCTSEncoder::initEncoder(uint32_t width, uint32_t height, uint32_t frameRate,
                                     uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds)
{
    EB_H265_ENC_CONFIGURATION high_conf;
    memset(&high_conf, 0, sizeof(EB_H265_ENC_CONFIGURATION));
    EB_H265_ENC_CONFIGURATION low_conf;
    memset(&low_conf, 0, sizeof(EB_H265_ENC_CONFIGURATION));

    if (!loadConf("high", high_conf)) {
        ELOG_ERROR_T("load high resolution config fail!");
        return false;
    }
    if (!loadConf("low", low_conf)) {
        ELOG_ERROR_T("load low resolution config fail!");
        return false;
    }

    m_width_hi = high_conf.sourceWidth;
    m_height_hi = high_conf.sourceHeight;

    m_width_low = low_conf.sourceWidth;
    m_height_low = low_conf.sourceHeight;

    int ready = false;
    ready = m_hi_res_encoder->init(high_conf);
    if (!ready)
        return false;
    ready = m_low_res_encoder->init(low_conf);
    if (!ready)
        return false;

    m_encoderReady = true;

    return true;
}

bool SVTHEVCMCTSEncoder::initEncoderAsync(uint32_t width, uint32_t height, uint32_t frameRate,
                                          uint32_t bitrateKbps, uint32_t keyFrameIntervalSeconds)
{
    m_srv->post(boost::bind(&SVTHEVCMCTSEncoder::initEncoder, this, width, height, frameRate, bitrateKbps, keyFrameIntervalSeconds));
    return true;
}

int32_t SVTHEVCMCTSEncoder::generateStream(uint32_t width, uint32_t height, uint32_t frameRate,
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

    m_frameRate = frameRate;
    m_bitrateKbps = bitrateKbps;
    m_keyFrameIntervalSeconds = keyFrameIntervalSeconds;

    if (width != 0 && height != 0) {
        if (!initEncoderAsync(width, height, m_frameRate, m_bitrateKbps, m_keyFrameIntervalSeconds))
            return -1;
    }

    m_dest = dest;
    addVideoDestination(m_dest);

    return 0;
}

void SVTHEVCMCTSEncoder::degenerateStream(int32_t streamId)
{
    boost::unique_lock<boost::shared_mutex> ulock(m_mutex);

    ELOG_DEBUG_T("degenerateStream");

    assert(m_dest != NULL);
    removeVideoDestination(m_dest);
    m_dest = NULL;
}

void SVTHEVCMCTSEncoder::setBitrate(unsigned short kbps, int32_t streamId)
{
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    ELOG_WARN_T("%s", __FUNCTION__);
}

void SVTHEVCMCTSEncoder::requestKeyFrame(int32_t streamId)
{
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    ELOG_DEBUG_T("%s", __FUNCTION__);
    //todo
}

void SVTHEVCMCTSEncoder::onFrame(const Frame& frame)
{
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    if (frame.format != FRAME_FORMAT_I420) {
        ELOG_ERROR_T("Frame format not supported");
        return;
    }

    if (m_dest == NULL) {
        return;
    }

    if (!m_encoderReady) {
        LTTNG_TRACE_WARNING("SVTHEVCMCTSEncoder: Not ready, Discard frame, pts(%ld)", (long int)frame.timeStamp / 90);
        ELOG_WARN_T("Encoder not ready!");
        return;
    }

    if (m_hi_res_encoder->frame_queue_size() > 1 || m_low_res_encoder->frame_queue_size() > 1) {
        LTTNG_TRACE_WARNING("SVTHEVCMCTSEncoder: Too many pending frames, Discard frame, pts(%ld)", (long int)frame.timeStamp / 90);
        ELOG_WARN_T("Too many pending frames");
    } else {
        LTTNG_TRACE_INFO("SVTHEVCMCTSEncoder: Receive frame, pts(%ld)", (long int)frame.timeStamp / 90);

        m_hi_res_encoder->sendFrame(frame);
        m_low_res_encoder->sendFrame(frame);
    }

    fetchOutput();
    deliverOutput();
}

void SVTHEVCMCTSEncoder::fetchOutput()
{
    boost::shared_ptr<SVTHEVCEncodedPacket> pkt;

    while(true) {
        pkt = m_hi_res_encoder->getEncodedPacket();
        if (!pkt)
            break;

        m_hi_res_packet_queue.push(pkt);
        LTTNG_TRACE_INFO("SVTHEVCMCTSEncoder: HR-Encoding Done, pts(%ld), %s"
                   , pkt->pts
                   , pkt->isKey ? "key" : "delta");
    }

    while(true) {
        pkt = m_low_res_encoder->getEncodedPacket();
        if (!pkt)
            break;

        m_low_res_packet_queue.push(pkt);
        LTTNG_TRACE_INFO("SVTHEVCMCTSEncoder: LR-Encoding Done, pts(%ld), %s"
                   , pkt->pts
                   , pkt->isKey ? "key" : "delta");
    }
}

void SVTHEVCMCTSEncoder::deliverOutput()
{
    int deliver_frames = 0;

    if (m_hi_res_packet_queue.size() == 0 ||
        m_low_res_packet_queue.size() == 0) {
        ELOG_TRACE_T("Output frame not ready");
        return;
    }

    while (m_hi_res_packet_queue.size() > 0
           && m_low_res_packet_queue.size() > 0) {
        boost::shared_ptr<SVTHEVCEncodedPacket> hi_res = m_hi_res_packet_queue.front();
        m_hi_res_packet_queue.pop();

        boost::shared_ptr<SVTHEVCEncodedPacket> low_res = m_low_res_packet_queue.front();
        m_low_res_packet_queue.pop();

        deliverMCTSFrame(hi_res, low_res);

        deliver_frames++;
        if (deliver_frames > 1) {
            ELOG_TRACE_T("Deliver extra frame");
            m_stat_overflow_frames++;
        }

        m_stat_overflow_all_frames++;
        if (m_stat_overflow_all_frames >= OverflowThresholdAllFrames) {
            // if overflow > 5%, increase buffered frames
            // if overflow < 4%, decrease buffered frames
            if (m_stat_overflow_frames > OverflowThresholdFrames) {
                m_max_output_frames++;
                ELOG_TRACE_T("Increase max buffered frame to %d", m_max_output_frames);
            } else if  (m_stat_overflow_frames < OverflowThresholdFrames - 1) {
                m_max_output_frames--;
                ELOG_TRACE_T("Decrease max buffered frame to %d", m_max_output_frames);
            }

            m_stat_overflow_all_frames = 0;
            m_stat_overflow_frames = 0;
        }

        if (m_hi_res_packet_queue.size() <= m_max_output_frames
            && m_low_res_packet_queue.size() <= m_max_output_frames) {
            break;
        }
    }
}

void SVTHEVCMCTSEncoder::deliverMCTSFrame(boost::shared_ptr<SVTHEVCEncodedPacket> hi_res_pkt,
                                          boost::shared_ptr<SVTHEVCEncodedPacket> low_res_pkt)
{
    uint32_t length = 12 + hi_res_pkt->length + 12 + low_res_pkt->length;

    while (m_payload_buffer_length < length) {
        if (m_payload_buffer) {
            m_payload_buffer_length = m_width_hi * m_height_hi * 2;
            m_payload_buffer = (uint8_t *)malloc(m_payload_buffer_length);
            continue;
        }

        m_payload_buffer_length *= 2;
        m_payload_buffer = (uint8_t *)realloc(m_payload_buffer, m_payload_buffer_length);
    }

    uint8_t *payload = m_payload_buffer;

    int offset = 0;

    // hi_res
    payload[offset + 0] = m_width_hi & 0xff;
    payload[offset + 1] = (m_width_hi >> 8) & 0xff;
    payload[offset + 2] = (m_width_hi >> 16) & 0xff;
    payload[offset + 3] = (m_width_hi >> 24) & 0xff;
    offset += 4;

    payload[offset + 0] = m_height_hi & 0xff;
    payload[offset + 1] = (m_height_hi >> 8) & 0xff;
    payload[offset + 2] = (m_height_hi >> 16) & 0xff;
    payload[offset + 3] = (m_height_hi >> 24) & 0xff;
    offset += 4;

    payload[offset + 0] = hi_res_pkt->length & 0xff;
    payload[offset + 1] = (hi_res_pkt->length >> 8) & 0xff;
    payload[offset + 2] = (hi_res_pkt->length >> 16) & 0xff;
    payload[offset + 3] = (hi_res_pkt->length >> 24) & 0xff;
    offset += 4;

    memcpy(payload + offset, hi_res_pkt->data, hi_res_pkt->length);
    offset += hi_res_pkt->length;

    // low_res
    payload[offset + 0] = m_width_low & 0xff;
    payload[offset + 1] = (m_width_low >> 8) & 0xff;
    payload[offset + 2] = (m_width_low >> 16) & 0xff;
    payload[offset + 3] = (m_width_low >> 24) & 0xff;
    offset += 4;

    payload[offset + 0] = m_height_low & 0xff;
    payload[offset + 1] = (m_height_low >> 8) & 0xff;
    payload[offset + 2] = (m_height_low >> 16) & 0xff;
    payload[offset + 3] = (m_height_low >> 24) & 0xff;
    offset += 4;

    payload[offset + 0] = low_res_pkt->length & 0xff;
    payload[offset + 1] = (low_res_pkt->length >> 8) & 0xff;
    payload[offset + 2] = (low_res_pkt->length >> 16) & 0xff;
    payload[offset + 3] = (low_res_pkt->length >> 24) & 0xff;
    offset += 4;

    memcpy(payload + offset, low_res_pkt->data, low_res_pkt->length);
    offset += low_res_pkt->length;

    m_encodedFrameCount++;
    // out
    Frame outFrame;
    memset(&outFrame, 0, sizeof(outFrame));
    outFrame.format     = FRAME_FORMAT_H265;
    outFrame.payload    = payload;
    outFrame.length     = length;
    outFrame.timeStamp = (m_encodedFrameCount * 1000 / m_frameRate) * 90;
    //outFrame.timeStamp = m_clock->TimeInMilliseconds() * 90;
   // outFrame.ori_timeStamp = hi_res_pkt->pts * 90;
    outFrame.additionalInfo.video.width         = m_width_hi;
    outFrame.additionalInfo.video.height        = m_height_hi;
    outFrame.additionalInfo.video.isKeyFrame    = hi_res_pkt->isKey;

    LTTNG_TRACE_INFO("SVTHEVCMCTSEncoder: Deliver frame, pts(%ld), %s"
            , hi_res_pkt->pts
            , hi_res_pkt->isKey ? "key" : "delta");

    ELOG_TRACE_T("deliverFrame %s, hi-res %dx%d(%s), length(%d), low-res (%s), length(%d) ",
            getFormatStr(outFrame.format),
            outFrame.additionalInfo.video.width,
            outFrame.additionalInfo.video.height,
            hi_res_pkt->isKey ? "key" : "delta",
            hi_res_pkt->length,
            low_res_pkt->isKey ? "key" : "delta",
            low_res_pkt->length
            );

    deliverFrame(outFrame);

    //FPS stat
    if (m_stat_timestamp == 0) {
        m_stat_timestamp = currentTimeMs();
    }

    m_stat_frame_count++;
    if (m_stat_frame_count == 100) {
        int64_t cur_time = currentTimeMs();
        ELOG_INFO_T("SVTHEVCMCTSEncoder: FPS %.2f",
                    (double)(1000 * m_stat_frame_count) / (cur_time - m_stat_timestamp));
        m_stat_timestamp = cur_time;
        m_stat_frame_count = 0;
    }
}

} // namespace owt_base
