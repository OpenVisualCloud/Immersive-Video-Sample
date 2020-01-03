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
//! \file     render.cpp
//! \brief    This is the main function for the application.
//!

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include "Common.h"
#include "Player.h"
#include "../utils/tinyxml2.h"

using namespace tinyxml2;

VCD_USE_VRVIDEO;

int main(int32_t argc, char *argv[])
{
    //1. input from xml configuration
    XMLDocument config;
    config.LoadFile("config.xml");
    XMLElement *info = config.RootElement();

    struct RenderConfig renderConfig;

    renderConfig.windowWidth     = atoi(info->FirstChildElement("windowWidth")->GetText());
    renderConfig.windowHeight    = atoi(info->FirstChildElement("windowHeight")->GetText());
    if (renderConfig.windowWidth < 500 || renderConfig.windowWidth > 2000 || renderConfig.windowHeight < 500 || renderConfig.windowHeight > 2000)
    {
        LOG(ERROR)<<"---XML input invalid--- window width or height must be in range (500-2000)!"<<std::endl;
        return RENDER_ERROR;
    }
    renderConfig.url = info->FirstChildElement("url")->GetText();
    string fileType(renderConfig.url+strlen(renderConfig.url)-3, renderConfig.url+strlen(renderConfig.url));
    if (fileType != "mpd")
    {
        LOG(ERROR)<<"---INVALID url input! (only remote mpd file supported)---"<<std::endl;
        return RENDER_ERROR;
    }
    renderConfig.sourceType = atoi(info->FirstChildElement("sourceType")->GetText());// FFMPEG_SOURCE=1 or DASH_SOURCE=0
    if (renderConfig.sourceType == 1)
    {
        LOG(ERROR)<<"---NOT support local file for now---"<<std::endl;
        return RENDER_ERROR;
    }
    else if (renderConfig.sourceType > 2 || renderConfig.sourceType < 0)
    {
        LOG(ERROR)<<"---INVALID source type input (0:remote mpd 2:webrtc support)---"<<std::endl;
        return RENDER_ERROR;
    }
#ifndef LOW_LATENCY_USAGE
    else if (renderConfig.sourceType == 2)
    {
        LOG(ERROR)<<"---INVALID source type input (webrtc is not enabled, please enable webrtc!)---"<<std::endl;
        return RENDER_ERROR;
    }
#endif /* LOW_LATENCY_USAGE */
    renderConfig.decoderType = atoi(info->FirstChildElement("decoderType")->GetText());// VAAPI_DECODER=1 or SW_DECODER=0
    if (renderConfig.decoderType != 0)
    {
        LOG(ERROR)<<"---INVALID decoder type input (0:software decoder)---"<<std::endl;
        return RENDER_ERROR;
    }
    renderConfig.contextType = atoi(info->FirstChildElement("contextType")->GetText());// EGL_CONTEXT=1 or GLFW_CONTEXT=0
    if (renderConfig.contextType != 0)
    {
        LOG(ERROR)<<"---INVALID context type input (0:GLFW context)---"<<std::endl;
        return RENDER_ERROR;
    }
    renderConfig.useDMABuffer = atoi(info->FirstChildElement("useDMABuffer")->GetText());// It is only valid for hardware decoding + EGL_CONTEXT if it is set as 1
    if (renderConfig.useDMABuffer != 0)
    {
        LOG(ERROR)<<"---INVALID useDMABuffer input (0:not use DMA buffer)---"<<std::endl;
        return RENDER_ERROR;
    }
    renderConfig.viewportHFOV = atoi(info->FirstChildElement("viewportHFOV")->GetText());
    renderConfig.viewportVFOV = atoi(info->FirstChildElement("viewportVFOV")->GetText());
    if (renderConfig.viewportHFOV < 80 || renderConfig.viewportHFOV > 120 || renderConfig.viewportVFOV < 80 || renderConfig.viewportVFOV > 120)
    {
        LOG(ERROR)<<"---INVALID viewportHFOV or viewportVFOV input!(reference-80/80 or 90/90)---"<<std::endl;
        return RENDER_ERROR;
    }
    renderConfig.viewportWidth = atoi(info->FirstChildElement("viewportWidth")->GetText());
    renderConfig.viewportHeight = atoi(info->FirstChildElement("viewportHeight")->GetText());
    if (renderConfig.viewportWidth < 800 || renderConfig.viewportWidth > 1200 || renderConfig.viewportHeight < 800 || renderConfig.viewportHeight > 1200)
    {
        LOG(ERROR)<<"---INVALID viewportWidth or viewportHeight input! (reference-960/960 or 1024/1024)---"<<std::endl;
        return RENDER_ERROR;
    }
    renderConfig.cachePath = info->FirstChildElement("cachePath")->GetText();
    string cacheDir = renderConfig.cachePath;
    DIR* dir = opendir(cacheDir.c_str());
    if(dir)
    {
        closedir(dir);
    }
    else
    {
        LOG(INFO)<<"Failed to open the cache path: " << cacheDir <<" , create a folder with this path!"<<endl;
        int checkdir = mkdir(cacheDir.c_str(), 0777);
        if(checkdir)
        {
            LOG(ERROR)<<"Uable to create cache path: "<<cacheDir <<endl;
            return RENDER_ERROR;
        }
    }
    //2.initial player
    Player *player = new Player(renderConfig);
    //3.open process
    if (RENDER_STATUS_OK != player->Open())
    {
        delete player;
        player = NULL;
        return RENDER_ERROR;
    }
    //4.play process
    player->Play();
    delete player;
    player = NULL;
    return 0;
}
