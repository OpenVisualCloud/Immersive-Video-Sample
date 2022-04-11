/*
 * Copyright (c) 2022, Intel Corporation
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
//! \file     ClientSimulator.cpp
//! \brief    This is the client simulator for the application.
//!

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <thread>
#include <math.h>
#include <vector>
#include <algorithm>
#include <numeric>

#include "../../utils/error.h"
#include "../../utils/ns_def.h"
#include "../../utils/data_type.h"
#include "../../utils/GlogWrapper.h"
#include "../../OmafDashAccess/OmafDashAccessApi.h"
#include "../../OmafDashAccess/general.h"

#define WAIT_PACKET_TIME_OUT 10000 // 10s

using namespace std;

struct _inputParams {
    char *url;
    pair<uint32_t, uint32_t> fov;
    uint32_t viewport_w;
    uint32_t viewport_h;
    char *mode;
    pair<float, float> init_pose;
    char *out;
};

using InputParams = _inputParams;

class DashAccessWrapper
{
public:
    DashAccessWrapper(InputParams *params);

    virtual ~DashAccessWrapper();

    int32_t Init(DashStreamingClient *pCtxDashStreaming);

    int32_t Destroy();

    int32_t Open(DashStreamingClient *pCtxDashStreaming);

    int32_t Start();

    int32_t ChangeViewport();

    int32_t Retrive();

    bool IsQuit();

    uint32_t GetFramerate();

    void DataAnalysis();

private:

    InputParams *m_params;

    void* m_handler;

    bool m_bQuit;

    uint32_t m_framerate;

    fstream m_log;
};

DashAccessWrapper::DashAccessWrapper(InputParams *params) {
    m_params = params;
    m_handler = nullptr;
    m_bQuit = false;
    m_framerate = 0;
}

DashAccessWrapper::~DashAccessWrapper() {
    m_params = nullptr;
    m_handler = nullptr;
    m_bQuit = true;
    m_framerate = 0;
}

int32_t DashAccessWrapper::Init(DashStreamingClient *pCtxDashStreaming) {
    if (nullptr == pCtxDashStreaming) {
        return ERROR_NULL_PTR;
    }
    pCtxDashStreaming->media_url = m_params->url;
    pCtxDashStreaming->cache_path = "/tmp/cache";
    pCtxDashStreaming->source_type = MultiResSource;
    pCtxDashStreaming->enable_extractor = false;
    pCtxDashStreaming->log_callback = nullptr;

    // init the omaf params
    memset(&pCtxDashStreaming->omaf_params, 0, sizeof(pCtxDashStreaming->omaf_params));
    pCtxDashStreaming->omaf_params.http_params.ssl_verify_host = 0;
    pCtxDashStreaming->omaf_params.http_params.ssl_verify_peer = 0;
    pCtxDashStreaming->omaf_params.http_params.conn_timeout = -1;  // not set
    pCtxDashStreaming->omaf_params.http_params.retry_times = 3;
    pCtxDashStreaming->omaf_params.http_params.total_timeout = -1;  // not set

    pCtxDashStreaming->omaf_params.max_parallel_transfers = 256;
    pCtxDashStreaming->omaf_params.segment_open_timeout_ms = 3000;           // ms
    pCtxDashStreaming->omaf_params.statistic_params.enable = 0;              // enable statistic
    pCtxDashStreaming->omaf_params.statistic_params.window_size_ms = 10000;  // ms

    pCtxDashStreaming->omaf_params.synchronizer_params.enable = 0;               //  enable dash segment number syncer
    pCtxDashStreaming->omaf_params.synchronizer_params.segment_range_size = 20;  // 20
    pCtxDashStreaming->omaf_params.max_decode_width = 3840;
    pCtxDashStreaming->omaf_params.max_decode_height = 2560;
    pCtxDashStreaming->omaf_params.enable_in_time_viewport_update = false;
    pCtxDashStreaming->omaf_params.max_response_times_in_seg = 0;
    pCtxDashStreaming->omaf_params.max_catchup_width = 3840;
    pCtxDashStreaming->omaf_params.max_catchup_height = 2560;

    m_handler = OmafAccess_Init(pCtxDashStreaming);
    if (nullptr == m_handler) {
        LOG(ERROR) << "handler init failed!" << endl;
        return ERROR_NULL_PTR;
    }

    // initial viewport
    HeadSetInfo clientInfo;
    clientInfo.pose = new HeadPose;
    if (nullptr == clientInfo.pose) {
        LOG(ERROR) << "client info malloc failed!" << endl;
        return ERROR_NULL_PTR;
    }
    clientInfo.pose->yaw = 0;
    clientInfo.pose->pitch = 0;
    clientInfo.pose->centerX = 0;
    clientInfo.pose->centerY = 0;
    clientInfo.pose->speed = 0.0f;
    clientInfo.pose->zoomFactor = 1.0f;
    clientInfo.pose->viewOrient.mode = ORIENT_NONE;
    clientInfo.pose->viewOrient.orientation = 0.0f;
    clientInfo.pose->pts = 0;
    clientInfo.viewPort_hFOV = m_params->fov.first;
    clientInfo.viewPort_vFOV = m_params->fov.second;
    clientInfo.viewPort_Width = m_params->viewport_w;
    clientInfo.viewPort_Height = m_params->viewport_h;
    OmafAccess_SetupHeadSetInfo(m_handler, &clientInfo);
    SAFE_DELETE(clientInfo.pose);
    return ERROR_NONE;
}

int32_t DashAccessWrapper::Open(DashStreamingClient *pCtxDashStreaming) {
    // open media
    if (ERROR_NONE != OmafAccess_OpenMedia(m_handler, pCtxDashStreaming, false, (char *)"", (char *)"")) {
        LOG(ERROR) << "Open media failed!" << endl;
        return ERROR_INVALID;
    }
    // Get media info
    DashMediaInfo mediaInfo;
    OmafAccess_GetMediaInfo(m_handler, &mediaInfo);
    if (mediaInfo.stream_info[0].framerate_den != 0)
        m_framerate = round(float(mediaInfo.stream_info[0].framerate_num) / mediaInfo.stream_info[0].framerate_den);
    m_log.open(m_params->out, ios::out);
    m_log << "Source high resolution: " << mediaInfo.stream_info[0].width << " x " << mediaInfo.stream_info[0].height
          << " Tile partition: " << mediaInfo.stream_info[0].tileRowNum << " x " << mediaInfo.stream_info[0].tileColNum
          << " Tile resolution: " << mediaInfo.stream_info[0].width / mediaInfo.stream_info[0].tileColNum << " x " << mediaInfo.stream_info[0].height / mediaInfo.stream_info[0].tileRowNum << endl;
    m_log.close();
    return ERROR_NONE;
}

int32_t DashAccessWrapper::Start() {
    return OmafAccess_StartStreaming(m_handler);
}

int32_t DashAccessWrapper::ChangeViewport() {
    char *mode = m_params->mode;
    HeadPose pose;
    memset(&pose, 0, sizeof(HeadPose));
    static int32_t count = 0;
    static int32_t flag = 1;

    if (0 == strcmp(mode, "horizontal")) {
        pose.yaw = ((int32_t)m_params->init_pose.first + 180 + count++) % 360 - 180;
    }

    else if (0 == strcmp(mode, "vertical")) {
        static float s_pitch = m_params->init_pose.second;
        if (s_pitch >=90)
        {
            flag = 0-flag;
            s_pitch = 90;
        }
        else if (s_pitch <= -90)
        {
            flag = 0-flag;
            s_pitch = -90;
        }
        pose.pitch = s_pitch;
        s_pitch = s_pitch + flag;
    }

    else if (0 == strcmp(mode, "slope")) {
        pose.yaw = ((int32_t)m_params->init_pose.first + 180 + count++) % 360 - 180;
        static float s_pitch = m_params->init_pose.second;
        if (s_pitch >=90)
        {
            flag = 0-flag;
            s_pitch = 90;
        }
        else if (s_pitch <= -90)
        {
            flag = 0-flag;
            s_pitch = -90;
        }
        pose.pitch = s_pitch;
        s_pitch = s_pitch + flag;
    }
    // LOG(INFO) << "Yaw " << pose.yaw << " Pitch " << pose.pitch << endl;
    return OmafAccess_ChangeViewport(m_handler, &pose);
}

int32_t DashAccessWrapper::Retrive() {
    // get one packet from DashStreaming lib.
    DashPacket dashPkt[16];
    memset(dashPkt, 0, 16 * sizeof(DashPacket));
    int dashPktNum = 0;
    bool needHeaders = true;
    static uint64_t currentWaitTime = 0;
    uint64_t pts = 0;
    uint32_t maxWaitTimeout = WAIT_PACKET_TIME_OUT;
    int ret =
        OmafAccess_GetPacket(m_handler, 0, &(dashPkt[0]), &dashPktNum, (uint64_t *)&pts, needHeaders, false);
    if (ERROR_NONE != ret) {
        currentWaitTime++;
        if (currentWaitTime > maxWaitTimeout) // wait but get packet failed
        {
            LOG(ERROR) << " Wait too long to get packet from Omaf Dash Access library! Force to quit! " << endl;
        }
        return OMAF_ERROR_TIMED_OUT;
    }
    currentWaitTime = 0;
    if (dashPkt[0].bEOS && !dashPkt[0].bCatchup) {
        LOG(INFO) << "IS quit!" << endl;
        m_bQuit = true;
    }
    for (int i = 0; i < dashPktNum; i++) {
        LOG(INFO) << "Get packet has done! and pts is " << dashPkt[i].pts  << " video id " << dashPkt[i].videoID << endl;
    }

    for (int i = 0; i < dashPktNum; i++) {
        SAFE_FREE(dashPkt[i].buf);
        if (dashPkt[i].rwpk) SAFE_DELARRAY(dashPkt[i].rwpk->rectRegionPacking);
        SAFE_DELETE(dashPkt[i].rwpk);
        SAFE_DELETE(dashPkt[i].prft);
        SAFE_DELARRAY(dashPkt[i].qtyResolution);
    }
    return ERROR_NONE;
}

int32_t DashAccessWrapper::Destroy() {
    int32_t res = ERROR_NONE;
    res = OmafAccess_CloseMedia(m_handler);
    if (res != ERROR_NONE) {
        LOG(ERROR) << "Close media failed!" << endl;
        return res;
    }
    res = OmafAccess_Close(m_handler);
    if (res != ERROR_NONE) {
        LOG(ERROR) << "Close failed!" << endl;
        return res;
    }
    return ERROR_NONE;
}

bool DashAccessWrapper::IsQuit() {
    return m_bQuit;
}

uint32_t DashAccessWrapper::GetFramerate() {
    return m_framerate;
}

void DashAccessWrapper::DataAnalysis() {
    m_log.open(m_params->out, ios::in);
    m_log.seekg(0, ios::beg);
    vector<uint32_t> latencyArray;
    vector<uint32_t> sizeArray;
    string line;
    getline(m_log, line);
    while (getline(m_log, line)) {
        string attrib;
        istringstream readstr(line);
        uint32_t latency = 0;
        uint32_t size = 0;
        for (int i = 0; i < 8; i++) {
            getline(readstr, attrib, ',');
            if (i == 6) {//latency
                size_t pos = attrib.find_first_of('=');
                if (pos != string::npos) {
                    latency = atoi(attrib.substr(pos+1).c_str());
                    latencyArray.push_back(latency);
                }
            }
            else if (i == 7) {//size
                size_t pos = attrib.find_first_of('=');
                if (pos != string::npos) {
                    size = atoi(attrib.substr(pos+1).c_str());
                    sizeArray.push_back(size);
                }
            }
        }
    }

    //max, min, avg
    uint32_t max_latency = *max_element(latencyArray.begin(), latencyArray.end());
    uint32_t min_latency = *min_element(latencyArray.begin(), latencyArray.end());
    uint32_t max_size = *max_element(sizeArray.begin(), sizeArray.end());
    uint32_t min_size = *min_element(sizeArray.begin(), sizeArray.end());

    uint64_t sum_latency = accumulate(latencyArray.begin(), latencyArray.end(), 0);
    uint32_t avg_latency = sum_latency / latencyArray.size();
    uint64_t sum_size = accumulate(sizeArray.begin(), sizeArray.end(), 0);
    uint32_t avg_size = sum_size / sizeArray.size();
    m_log.close();
    m_log.open(m_params->out, ios::out | ios::app);
    m_log << "Data summary: latency_ms(max, min, avg): (" << max_latency << ", " << min_latency << ", " << avg_latency << "); size(max, min, avg): (" << max_size << ", " << min_size << ", " << avg_size << ")." << endl;
    m_log.close();
    return;
}

void Help() {
    cout << "Options:" << endl;
    cout << " -h, --help                 Print this message and exit." << endl;
    cout << " --url                      Input mpd url for CDN client test." << endl;
    cout << " --viewport                 Required viewport info, prefer 80,80,960,960." << endl;
    cout << " --mode                     assigned motion mode: horizontal, vertical, slope." << endl;
    cout << " --initpose                 an initial pose coordinate." << endl;
    cout << " -o, --out                  data output path." << endl;
}

int main(int argc, char* argv[]) {
    //option lists
    int32_t lopt = 0;
    struct option longOpts[] = {
        { "help", no_argument, nullptr, 'h' },
        { "url", required_argument, &lopt, 1 },
        { "viewport", required_argument, &lopt, 2 },
        { "mode", required_argument, &lopt, 3 },
        { "initpose", required_argument, &lopt, 4 },
        { "out", required_argument, nullptr, 'o' },
        { 0, 0, 0, 0 }
    };
    //input params
    char *url = nullptr;
    char *viewport = nullptr;
    char *mode = nullptr;
    char *initpose = nullptr;
    char *out = nullptr;
    //params analysis
    if (argc == 1) return 0;
    while(1) {
        int32_t res = getopt_long(argc, argv, "ho:", longOpts, nullptr);
        if(res == -1) {
            break;
        }
        switch (res) {
            case 'h': Help(); return 0;
            case 'o': out = optarg; break;
            case 0:
                switch (lopt) {
                case 1: url = optarg; break;
                case 2: viewport = optarg; break;
                case 3: mode = optarg; break;
                case 4: initpose = optarg; break;
                default: cout << "input parameters invalid!" << endl; break;
                }
        }
    }
    // cout << "url " << url << " viewport " << viewport << " mode " << mode << " initpose " << initpose << " out " << out << endl;

    GlogWrapper m_glogWrapper((char*)"glogClient");
    //Call OmafDashAccess APIs to simulate
    InputParams params;
    sscanf(viewport, "%d,%d,%d,%d", &params.fov.first, &params.fov.second, &params.viewport_w, &params.viewport_h);
    sscanf(initpose, "%f,%f", &params.init_pose.first, &params.init_pose.second);
    params.url = url;
    params.mode = mode;
    params.out = out;

    DashAccessWrapper *daWrapper = new DashAccessWrapper(&params);
    if (nullptr == daWrapper) return ERROR_NULL_PTR;

    int32_t res = ERROR_NONE;

    DashStreamingClient *dsClient = new DashStreamingClient;
    if (nullptr == dsClient) return ERROR_NULL_PTR;

    res = daWrapper->Init(dsClient);
    if (res != ERROR_NONE) {
        LOG(ERROR) << "Dash Access init failed!" << endl;
        SAFE_DELETE(daWrapper);
        SAFE_DELETE(dsClient);
        return ERROR_INVALID;
    }

    res = daWrapper->Open(dsClient);
    if (res != ERROR_NONE) {
        LOG(ERROR) << "Dash Access open failed!" << endl;
        SAFE_DELETE(daWrapper);
        SAFE_DELETE(dsClient);
        return ERROR_INVALID;
    }

    res = daWrapper->Start();
    if (res != ERROR_NONE) {
        LOG(ERROR) << "Dash Access start failed!" << endl;
        SAFE_DELETE(daWrapper);
        SAFE_DELETE(dsClient);
        return ERROR_INVALID;
    }

    //change viewport and retrive one packet
    uint32_t interval = 1000 / daWrapper->GetFramerate();
    while (!daWrapper->IsQuit()) {
        daWrapper->ChangeViewport();
        daWrapper->Retrive();
        usleep(interval * 1000);
    }

    SAFE_DELETE(dsClient);

    //process glog file
    string outfile = out;
    string cmd = "grep \"Task download data\" ./logfiles/WARNING/glogClient.WARNING >> " + outfile;
    int ret = system(cmd.c_str());
    if (ret) LOG(WARNING) << "Process glog file failed!" << endl;

    //data analysis
    daWrapper->DataAnalysis();

    daWrapper->Destroy();

    SAFE_DELETE(daWrapper);
    return res;
}