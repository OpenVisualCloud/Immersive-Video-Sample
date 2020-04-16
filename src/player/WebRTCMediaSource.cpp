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

 *
 */

//!
//! \file     WebRTCMediaSource.cpp
//! \brief    Implement class for WebRTCMediaSource.
//!
#ifdef _ENABLE_WEBRTC_SOURCE_

#include "Common.h"
#include "WebRTCMediaSource.h"

#include <sys/time.h>
#include <iostream>
#include <iomanip>
#include <malloc.h>
#include <algorithm>
#include <assert.h>

#include "owt/base/commontypes.h"
#include "owt/base/globalconfiguration.h"
#include "owt/base/network.h"
#include "owt/base/options.h"
#include "owt/conference/conferenceclient.h"
#include "owt/conference/remotemixedstream.h"
#include "http.h"
#include "../utils/tinyxml2.h"

#define SOURCENUMBER 2

VCD_NS_BEGIN
using namespace tinyxml2;

static inline int64_t currentTimeMs()
{
    timeval time;
    gettimeofday(&time, nullptr);
    return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

static int isValidStartCode(uint8_t *data, int length)
{
    if (length < 3)
        return -1;

    if (data[0] == 0 && data[1] == 0 && data[2] == 1)
        return 3;

    if (length >= 4 && data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 1)
        return 4;

    return 0;
}

static int findFirstNALU(uint8_t *data, int length, int *offset, int *size)
{
    int i = 0;
    int ret = -1;
    int start_code_len = 0;
    int nalu_offset = 0;
    int nalu_type = -1;
    int nalu_size = 0;

    while (true) {
        ret = isValidStartCode(data + i, length - i);
        if (ret < 0)
            return -1;
        else if (ret > 0) {
            start_code_len = ret;
            nalu_offset = i;
            break;
        }

        ++i;
    }

    i += start_code_len;
    while (true) {
        ret = isValidStartCode(data + i, length - i);
        if (ret < 0) {
            nalu_size = length - nalu_offset;
            break;
        } else if (ret > 0) {
            nalu_size = i - nalu_offset ;
            break;
        }

        ++i;
    }

    *offset = nalu_offset;
    *size = nalu_size;
    return (data[nalu_offset + start_code_len] & 0x7e) >> 1;
}

static int filterNALs(std::shared_ptr<SimpleBuffer> bitstream_buf, const std::vector<int> &remove_types, std::shared_ptr<SimpleBuffer> sei_buf)
{
    int remove_nals_size = 0;

    uint8_t *buffer_start = bitstream_buf->data();
    int buffer_length = bitstream_buf->size();
    int nalu_offset = 0;
    int nalu_size = 0;
    int nalu_type;

    while (buffer_length > 0) {
        nalu_type = findFirstNALU(buffer_start, buffer_length, &nalu_offset, &nalu_size);
        if (nalu_type < 0)
            break;

        if (std::find(remove_types.begin(), remove_types.end(), nalu_type) != remove_types.end()) {
            //copy
            sei_buf->insert(buffer_start + nalu_offset, nalu_size);

            //next
            memmove(buffer_start, buffer_start + nalu_offset + nalu_size, buffer_length - nalu_offset - nalu_size);
            buffer_length -= nalu_offset + nalu_size;

            remove_nals_size += nalu_offset + nalu_size;
            continue;
        }

        buffer_start += (nalu_offset + nalu_size);
        buffer_length -= (nalu_offset + nalu_size);
    }

    bitstream_buf->resize(bitstream_buf->size() - remove_nals_size);
    return bitstream_buf->size();
}

static void filter_RWPK_SEI(std::shared_ptr<SimpleBuffer> bitstream_buf, std::shared_ptr<SimpleBuffer> sei_buf)
{
    std::vector<int> sei_types;
    sei_types.push_back(38);
    sei_types.push_back(39);
    sei_types.push_back(30);

    filterNALs(bitstream_buf, sei_types, sei_buf);
}

RegionWisePacking *alloc_RegionWisePacking()
{
    RegionWisePacking *rwpk = new RegionWisePacking();
    assert(rwpk != NULL);
    memset(rwpk, 0, sizeof(RegionWisePacking));

    rwpk->rectRegionPacking = new RectangularRegionWisePacking[DEFAULT_REGION_NUM];
    assert(rwpk->rectRegionPacking != NULL);

    return rwpk;
}

void free_RegionWisePacking(RegionWisePacking *rwpk)
{
    assert(rwpk != NULL);
    assert(rwpk->rectRegionPacking != NULL);

    delete[] rwpk->rectRegionPacking;
    delete rwpk;
}

WebRTCVideoFrame::WebRTCVideoFrame(AVFrame *frame, RegionWisePacking *rwpk)
    : m_frame(NULL)
    , m_rwpk(NULL) {
    assert(frame != NULL);
    assert(rwpk != NULL);

    m_frame = av_frame_clone(frame);

#if 0
    LOG("linesize: %d-%d-%d-%d"
            , m_decFrame->linesize[0]
            , m_decFrame->linesize[1]
            , m_decFrame->linesize[2]
            , m_decFrame->linesize[3]);
#endif

    // if pitch, we need recopy this data
    m_buffer[0] = m_frame->data[0];
    m_buffer[1] = m_frame->data[1];
    m_buffer[2] = m_frame->data[2];

    m_rwpk = rwpk;

    return;
}

WebRTCVideoFrame::~WebRTCVideoFrame() {
    if (m_frame)
        av_frame_free(&m_frame);

    free_RegionWisePacking(m_rwpk);
}

WebRTCMediaSource *WebRTCMediaSource::s_CurObj;
uint32_t WebRTCMediaSource::fullwidth;
uint32_t WebRTCMediaSource::fullheight;

int WebRTCMediaSource::getParam(int *flag, int* projType)
{
    XMLDocument config;
    config.LoadFile("config.xml");
    XMLElement *info = config.RootElement();
    if(!strcmp(info->FirstChildElement("resolution")->GetText(),"8k"))
        *flag = 1;
    else if(!strcmp(info->FirstChildElement("resolution")->GetText(),"4k"))
        *flag = 2;
    else
    {
        LOG(ERROR) << "wrong resolution,must be 4k or 8k !" << std::endl;
        exit(-1);
    }
    const char* server_url = info->FirstChildElement("server_url")->GetText();
    if(!server_url)
    {
        LOG(ERROR) << "server_url can not null !" << std::endl;
        exit(-1);
    }
    this->m_serverAddress = std::string(server_url);

    int rate = atoi(info->FirstChildElement("frameRate")->GetText());
    int num = atoi(info->FirstChildElement("frameNum")->GetText());
    if(!rate || !num)
    {
        LOG(ERROR) << "wrong frameRate or frameNum !" << std::endl;
        exit(-1);
    }
    this->frameRate = rate;
    this->frameNum = num;

    if(!strcmp(info->FirstChildElement("ProjectionType")->GetText(),"ERP"))
        *projType = 1;
    else if(!strcmp(info->FirstChildElement("ProjectionType")->GetText(),"CUBMAP"))
        *projType = 2;
    else
    {
        LOG(ERROR) << "wrong ProjectionType,must be ERP or CUBMAP !" << std::endl;
        exit(-1);
    }

    LOG(INFO) << "server_url " << info->FirstChildElement("server_url")->GetText()
        << ", resolution " << info->FirstChildElement("resolution")->GetText()
        << ", frameRate " << info->FirstChildElement("frameRate")->GetText()
        << ", frameNum " << info->FirstChildElement("frameNum")->GetText()
        << ", ProjectionType " << info->FirstChildElement("ProjectionType")->GetText()
        << std::endl;

    return 0;
}

void WebRTCMediaSource::subscribe_on_success_callback(std::shared_ptr<owt::conference::ConferenceSubscription> sc) {
    LOG(INFO) << "subscribe_on_success_callback" << std::endl;

    s_CurObj->m_subId = sc->Id();

    std::unique_lock <std::mutex> ulock(s_CurObj->m_mutex);
    s_CurObj->m_ready = true;
    s_CurObj->m_cond.notify_all();
}

void WebRTCMediaSource::subscribe_on_failure_callback(std::unique_ptr<owt::base::Exception> err) {
    LOG(ERROR) << "subscribe_on_failure_callback: " << err->Message() << std::endl;
    exit(1);
}

void WebRTCMediaSource::join_on_success_callback(std::shared_ptr<owt::conference::ConferenceInfo> info)
{
    LOG(INFO) << "join_on_success_callback" << std::endl;

    std::vector<std::shared_ptr<owt::base::RemoteStream>> remoteStreams = info->RemoteStreams();
    for (auto &remoteStream : remoteStreams) {
        if (remoteStream->Source().video == owt::base::VideoSourceInfo::kMixed) {
            s_CurObj->m_mixed_stream = static_pointer_cast<owt::conference::RemoteMixedStream>(remoteStream);
            break;
        }
    }

    if (!s_CurObj->m_mixed_stream) {
        LOG(ERROR) << "No mixed stream!" << std::endl;
        exit(1);
    }

    owt::base::VideoCodecParameters codecParams;
    codecParams.name = owt::base::VideoCodec::kH265;

    owt::conference::SubscribeOptions options;
    options.video.codecs.push_back(codecParams);

    options.video.resolution.width = fullwidth;
    options.video.resolution.height = fullheight;

    LOG(INFO) << "Subscribe: " << options.video.resolution.width << "x" << options.video.resolution.height << std::endl;
    s_CurObj->m_roomId = info->Id();
    s_CurObj->m_room->Subscribe(s_CurObj->m_mixed_stream,
            options,
            subscribe_on_success_callback,
            subscribe_on_failure_callback);
}

void WebRTCMediaSource::join_on_failure_callback(std::unique_ptr<owt::base::Exception> err)
{
    LOG(ERROR) << "join_on_failure_callback: " << err->Message() << std::endl;
    exit(1);
}

int32_t WebRTCMediaSource::RenderFrame(AVFrame *avFrame, RegionWisePacking *rwpk) {
    std::shared_ptr<WebRTCVideoFrame> frame = make_shared<WebRTCVideoFrame>(avFrame, rwpk);
    if (!frame->isValid())
        return -1;

    {
        std::unique_lock <std::mutex> ulock(m_mutex);

        while (m_webrtc_render_frame_queue.size() >= 2) {
            LOG(INFO) << "drop frame, queue size: " << m_webrtc_render_frame_queue.size() << std::endl;
            //m_webrtc_render_frame_queue.pop_back();
            m_webrtc_render_frame_queue.clear();
        }

        m_webrtc_render_frame_queue.push_front(frame);

        if (m_webrtc_render_frame_queue.size() == 1)
            m_cond.notify_all();
    }

    return 0;
}

WebRTCMediaSource::WebRTCMediaSource()
    : m_yaw(0)
    , m_pitch(0)
    , m_ready(false)
{
    LOG(INFO) << __FUNCTION__ << std::endl;

    s_CurObj = this;
}

WebRTCMediaSource::~WebRTCMediaSource()
{
    LOG(INFO) << __FUNCTION__ << std::endl;
}

RenderStatus WebRTCMediaSource::GetFrame(uint8_t **buffer, struct RegionInfo *regionInfo)
{
    std::shared_ptr<WebRTCVideoFrame> frame;

    {
        std::unique_lock <std::mutex> ulock(m_mutex);
        while(m_webrtc_render_frame_queue.size() == 0)
            m_cond.wait(ulock);

        frame = m_webrtc_render_frame_queue.back();
        m_webrtc_render_frame_queue.pop_back();

        m_free_queue.push_front(frame);
    }

    //set video buffer
    buffer[0] = frame->m_buffer[0];
    buffer[1] = frame->m_buffer[1];
    buffer[2] = frame->m_buffer[2];

    //set regionInfo
    regionInfo->regionWisePacking = frame->m_rwpk;
    SetRegionInfo(regionInfo);

    return RENDER_STATUS_OK;
}

RenderStatus WebRTCMediaSource::SetRegionInfo(struct RegionInfo *regionInfo)
{
    if (NULL == regionInfo)
    {
        return RENDER_ERROR;
    }
    regionInfo->sourceNumber = SOURCENUMBER;
    regionInfo->sourceInfo = (struct SourceInfo *)malloc(sizeof(struct SourceInfo) * regionInfo->sourceNumber);
    if (NULL == regionInfo->sourceInfo)
    {
        return RENDER_ERROR;
    }

    regionInfo->sourceInfo[0].sourceWidth = regionInfo->regionWisePacking->projPicWidth;
    regionInfo->sourceInfo[0].sourceHeight = regionInfo->regionWisePacking->projPicHeight;
    regionInfo->sourceInfo[0].tileColumnNumber = regionInfo->sourceInfo[0].sourceWidth / regionInfo->regionWisePacking->rectRegionPacking[0].projRegWidth;
    regionInfo->sourceInfo[0].tileRowNumber = regionInfo->sourceInfo[0].sourceHeight / regionInfo->regionWisePacking->rectRegionPacking[0].projRegHeight;
    //low reso tile hard code
    regionInfo->sourceInfo[1].sourceWidth = this->lowwidth;//LOWWIDTH;
    regionInfo->sourceInfo[1].sourceHeight = this->lowheight;//LOWHEIGHT;
    regionInfo->sourceInfo[1].tileColumnNumber = regionInfo->sourceInfo[1].sourceWidth / regionInfo->regionWisePacking->rectRegionPacking[regionInfo->regionWisePacking->numRegions - 1].projRegWidth;
    regionInfo->sourceInfo[1].tileRowNumber = regionInfo->sourceInfo[1].sourceHeight / regionInfo->regionWisePacking->rectRegionPacking[regionInfo->regionWisePacking->numRegions - 1].projRegHeight;

    if(regionInfo->sourceInfo[0].tileColumnNumber ==0)
        LOG(INFO) << "tileColumnNumber" << regionInfo->sourceInfo[0].tileColumnNumber << std::endl;

    return RENDER_STATUS_OK;
}

RenderStatus WebRTCMediaSource::Initialize(struct RenderConfig renderConfig)
{
    int flag, projType;
    getParam(&flag, &projType);
    if(flag == 1){
        fullwidth = 7680;
        fullheight = 3840;
        this->lowwidth = 512;
        this->lowheight = 1280;
        this->packedwidth = 2816;
        this->packedheight = 2560;
    }else if(flag == 2){
        fullwidth = 3840;
        fullheight = 2048;
        this->lowwidth = 1280;
        this->lowheight = 768;
        this->packedwidth = 2304;
        this->packedheight = 1280;
    }
    if(projType == 1){
        this->projType = PT_ERP;
    }else if(projType == 2){
        this->projType = PT_CUBEMAP;
    }

    owt::base::GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
    unique_ptr<owt::base::VideoDecoderInterface> decoder(new WebRTCFFmpegVideoDecoder(this));
    owt::base::GlobalConfiguration::SetCustomizedVideoDecoderEnabled(std::move(decoder));

    owt::conference::ConferenceClientConfiguration configuration;

    owt::base::IceServer ice;
    ice.urls.push_back("stun:61.152.239.56");
    ice.username = "";
    ice.password = "";
    std::vector<owt::base::IceServer> ice_servers;
    ice_servers.push_back(ice);

    configuration.ice_servers = ice_servers;

    string roomId = "";
    string token = CHttp::getToken(m_serverAddress, roomId);

    if (token == "") {
        LOG(ERROR) << "invalid token!" << std::endl;
        return RENDER_ERROR;
    }
    m_room = owt::conference::ConferenceClient::Create(configuration);

    m_room->Join(token,
            join_on_success_callback,
            join_on_failure_callback);


    {
        std::unique_lock <std::mutex> ulock(m_mutex);
        while(m_webrtc_render_frame_queue.size() == 0)
            m_cond.wait(ulock);
    }

    memset(&m_mediaSourceInfo, 0, sizeof(m_mediaSourceInfo));

    m_mediaSourceInfo.width = this->packedwidth;
    m_mediaSourceInfo.height = this->packedheight;
    m_mediaSourceInfo.stride = m_mediaSourceInfo.width;
    m_mediaSourceInfo.projFormat = this->projType;
    m_mediaSourceInfo.pixFormat = PixelFormat::PIX_FMT_YUV420P;
    m_mediaSourceInfo.hasAudio = false;
    m_mediaSourceInfo.audioChannel = 0;
    m_mediaSourceInfo.numberOfStreams = 1;
    m_mediaSourceInfo.frameRate = this->frameRate;
    m_mediaSourceInfo.frameNum = this->frameNum;
    m_mediaSourceInfo.currentFrameNum = 0;
    m_mediaSourceInfo.sourceWH = new SourceWH;
    m_mediaSourceInfo.sourceWH->width = new uint32_t[SOURCENUMBER];
    m_mediaSourceInfo.sourceWH->width[0] = fullwidth;
    m_mediaSourceInfo.sourceWH->width[1] = this->lowwidth;
    m_mediaSourceInfo.sourceWH->height = new uint32_t[SOURCENUMBER];
    m_mediaSourceInfo.sourceWH->height[0] = fullheight;
    m_mediaSourceInfo.sourceWH->height[1] = this->lowheight;
    isAllValid = true;

    LOG(INFO) << "Initialized!" << std::endl;
    return RENDER_STATUS_OK;
}

RenderStatus WebRTCMediaSource::SetMediaSourceInfo(void *mediaInfo)
{
    LOG(INFO) << __FUNCTION__ << std::endl;
    return RENDER_STATUS_OK;
}

struct MediaSourceInfo WebRTCMediaSource::GetMediaSourceInfo()
{
    LOG(INFO) << __FUNCTION__ << std::endl;
    return m_mediaSourceInfo;
}

void *WebRTCMediaSource::GetSourceMetaData()
{
    LOG(INFO) << __FUNCTION__ << std::endl;
    return NULL;
}

bool WebRTCMediaSource::IsEOS()
{
    return false;
}

RenderStatus WebRTCMediaSource::ChangeViewport(float yaw, float pitch)
{
    if ((int)yaw == m_yaw && (int)pitch == m_pitch)
        return RENDER_STATUS_OK;

    LOG(INFO) << "yaw: " << yaw << ", pitch: " << pitch << std::endl;

    m_yaw = yaw;
    m_pitch = pitch;

    int yawValue = m_yaw + 180;
    int pitchValue = m_pitch;
    int value = (pitchValue << 16) | (yawValue & 0xffff);

    std::string path = "/media/video/fov";
    std::string url = m_serverAddress + "/rooms/" + m_roomId + "/recordings/" + m_subId;
    std::string content = "[{\"op\":\"replace\",\"path\":\"" + path + "\",\"value\":" + to_string(value) + "}]";

    std::string response = CHttp::http_patch(url.c_str(), content);

    return RENDER_STATUS_OK;
}

void WebRTCMediaSource::DeleteBuffer(uint8_t **buffer)
{
    std::unique_lock <std::mutex> ulock(m_mutex);
    m_free_queue.pop_back();

    return;
}

void WebRTCMediaSource::ClearRWPK(RegionWisePacking *rwpk)
{
    //LOG(INFO) << __FUNCTION__ << std::endl;

    return;
}

SimpleBuffer::SimpleBuffer()
    : m_data(NULL)
    , m_size(0)
    , m_max_size(0)
{
}

SimpleBuffer::~SimpleBuffer()
{
    if (m_data)
        free(m_data);
}

void SimpleBuffer::insert(const uint8_t *data, int size)
{
    if (!m_data) {
        m_max_size = 1024 * 4;
        m_data = (uint8_t *)malloc(m_max_size);
    }

    if (size > m_max_size - m_size) {
        int new_max_size = m_max_size;
        while (size > new_max_size - m_size) {
            new_max_size  += 1024;
        }
        m_data = (uint8_t *)realloc(m_data, new_max_size);
        m_max_size = new_max_size;
    }

    memcpy(m_data + m_size, data, size);
    m_size += size;
}

WebRTCFFmpegVideoDecoder::WebRTCFFmpegVideoDecoder(WebRTCVideoRenderer *renderer)
    : m_decCtx(NULL)
    , m_decFrame(NULL)
    , m_needKeyFrame(true)
    , m_renderer(renderer)
    , m_statistics_frames(0)
    , m_statistics_last_timestamp(0)
{
    LOG(INFO) << "avcodec version: "
        << ((avcodec_version() >> 16) & 0xff) << "."
        << ((avcodec_version() >> 8) & 0xff) << "."
        << ((avcodec_version()) & 0xff)
        << std::endl;

    m_parserRWPKParam.usedType = E_PARSER_FOR_CLIENT;
    m_parserRWPKHandle = I360SCVP_Init(&m_parserRWPKParam);
}

owt::base::VideoDecoderInterface* WebRTCFFmpegVideoDecoder::Copy()
{
    WebRTCFFmpegVideoDecoder* decoder = new WebRTCFFmpegVideoDecoder(m_renderer);
    return decoder;
}

WebRTCFFmpegVideoDecoder::~WebRTCFFmpegVideoDecoder()
{
    LOG(INFO) << __FUNCTION__ << std::endl;

    if(m_parserRWPKHandle)
        I360SCVP_unInit(m_parserRWPKHandle);
    m_parserRWPKHandle = NULL;
}

bool WebRTCFFmpegVideoDecoder::InitDecodeContext(owt::base::VideoCodec video_codec)
{
    LOG(INFO) << __FUNCTION__ << std::endl;

    if (!createDecoder(video_codec))
        return false;

    m_bitstream_buf = std::make_shared<SimpleBuffer>();
    return true;
}

bool WebRTCFFmpegVideoDecoder::Release()
{
    LOG(INFO) << __FUNCTION__ << std::endl;
    return true;
}

bool WebRTCFFmpegVideoDecoder::OnEncodedFrame(unique_ptr<owt::base::VideoEncodedFrame> frame)
{
    int ret;

    {
        //statistics
        if (m_statistics_last_timestamp == 0) {
            m_statistics_last_timestamp = currentTimeMs();
        }
        m_statistics_frames++;

        if (m_statistics_frames == 300) {
            uint64_t cur_time = currentTimeMs();

            LOG(INFO) << "Decoding fps "
                << (double)(1000 * m_statistics_frames) / (cur_time - m_statistics_last_timestamp)
                << std::endl;

            m_statistics_last_timestamp = cur_time;
            m_statistics_frames = 0;
        }
    }

    if (m_needKeyFrame) {
        if (!frame->is_key_frame)
            return false;

        m_needKeyFrame = false;
    }

    m_bitstream_buf->resize(0);
    m_bitstream_buf->insert(frame->buffer, frame->length);

    std::shared_ptr<SimpleBuffer> sei_buf = std::make_shared<SimpleBuffer>();
    filter_RWPK_SEI(m_bitstream_buf, sei_buf);
    if (sei_buf->size() <= 0) {
        LOG(ERROR) << "No valid rwpk sei in bitstream!" << std::endl;
        return true;
    }

    RegionWisePacking *rwpk = alloc_RegionWisePacking();
    assert(rwpk != NULL);

    I360SCVP_ParseRWPK(m_parserRWPKHandle, rwpk, sei_buf->data(), sei_buf->size());

    av_init_packet(&m_packet);
    m_packet.data = const_cast<uint8_t *>(m_bitstream_buf->data());
    m_packet.size = m_bitstream_buf->size();
    m_packet.dts = frame->time_stamp;
    m_packet.pts = frame->time_stamp;

    ret = avcodec_send_packet(m_decCtx, &m_packet);
    if (ret < 0) {
        LOG(ERROR) << "Error while send packet" << std::endl;

        free_RegionWisePacking(rwpk);
        return false;
    }

    m_rwpk_queue.push_back(rwpk);

    while(true) {
        ret = avcodec_receive_frame(m_decCtx, m_decFrame);
        if (ret == AVERROR(EAGAIN)) {
            return true;
        }else if (ret < 0) {
            LOG(ERROR) << "Error while receive frame" << std::endl;
            return false;
        }

        if (m_rwpk_queue.empty()) {
            LOG(ERROR) << "Empty rwpk queue!" << std::endl;
            continue;
        }

        rwpk = m_rwpk_queue.front();
        m_rwpk_queue.pop_front();

        if (m_renderer)
            m_renderer->RenderFrame(m_decFrame, rwpk);
    }

    return true;
}

bool WebRTCFFmpegVideoDecoder::createDecoder(const owt::base::VideoCodec video_codec)
{
    int ret = 0;
    AVCodecID codec_id = AV_CODEC_ID_NONE;
    AVCodec* dec = NULL;

    switch (video_codec) {
        case owt::base::VideoCodec::kH265:
            codec_id = AV_CODEC_ID_H265;
            LOG(INFO) << "video decoder HEVC" << std::endl;
            break;

        default:
            LOG(ERROR) << "Only support HEVC:" << (int32_t)video_codec << std::endl;
            return false;
    }

    dec = avcodec_find_decoder(codec_id);
    if (!dec) {
        LOG(ERROR) << "Could not find ffmpeg decoder: " << avcodec_get_name(codec_id) << std::endl;
        return false;
    }

    m_decCtx = avcodec_alloc_context3(dec);
    if (!m_decCtx ) {
        LOG(ERROR) << "Could not alloc ffmpeg decoder context" << std::endl;
        return false;
    }

    m_decCtx->thread_type = FF_THREAD_FRAME;
    m_decCtx->thread_count = 3;
    ret = avcodec_open2(m_decCtx, dec , NULL);
    if (ret < 0) {
        LOG(ERROR) << "Could not open ffmpeg decoder context" << std::endl;
        return false;
    }

    m_decFrame = av_frame_alloc();
    if (!m_decFrame) {
        LOG(ERROR) << "Could not allocate dec frame" << std::endl;
        return false;
    }

    memset(&m_packet, 0, sizeof(m_packet));

    LOG(INFO) << "Create decoder successfully" << std::endl;
    return true;
}

char *WebRTCFFmpegVideoDecoder::ff_err2str(int errRet)
{
    av_strerror(errRet, (char*)(&m_errbuff), 500);
    return m_errbuff;
}

VCD_NS_END

#endif /* _ENABLE_WEBRTC_SOURCE_ */
