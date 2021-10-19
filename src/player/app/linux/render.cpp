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

#ifdef _LINUX_OS_

#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>
#include "../../../utils/tinyxml2.h"
#include "../../player_lib/Common/Common.h"
#include "../../player_lib/Api/MediaPlayer_Linux.h"
#include "../../player_lib/Render/RenderContext.h"
#include "GLFWRenderContext.h"

#define MAXFOV 140
#define MINFOV 50
#define MAXVIEWPORTLEN 2000
#define MINVIEWPORTLEN 800
#define MAXWINDOWLEN 2000
#define MINWINDOWLEN 500

using namespace tinyxml2;

VCD_USE_VRVIDEO;
bool parseRenderFromXml(std::string xml_file, struct RenderConfig &renderConfig) noexcept {
  try {
    XMLDocument config;
    config.LoadFile(xml_file.c_str());
    XMLElement *info = config.RootElement();
    if (NULL == info)
    {
      LOG(ERROR) << " XML parse failed! " << std::endl;
      return RENDER_ERROR;
    }
    XMLElement* wWidthElem = info->FirstChildElement("windowWidth");
    XMLElement* wHeightElem = info->FirstChildElement("windowHeight");
    if (wWidthElem != NULL && wHeightElem != NULL)
    {
      renderConfig.windowWidth = atoi(wWidthElem->GetText());
      renderConfig.windowHeight = atoi(wHeightElem->GetText());
      if (renderConfig.windowWidth < MINWINDOWLEN || renderConfig.windowWidth > MAXWINDOWLEN ||
          renderConfig.windowHeight < MINWINDOWLEN || renderConfig.windowHeight > MAXWINDOWLEN) {
        LOG(ERROR) << "---XML input invalid--- window width or height must be in range (" << MINWINDOWLEN << "-"
                  << MAXWINDOWLEN << ")!" << std::endl;
        return RENDER_ERROR;
      }
    }
    else
    {
      LOG(ERROR) << " invalid params for windowWidth OR windowHeight! " << std::endl;
      return RENDER_ERROR;
    }
    XMLElement* urlElem = info->FirstChildElement("url");
    if (urlElem != NULL)
    {
      renderConfig.url = new char[1024];
      memset_s(renderConfig.url, 1024*sizeof(char), 0);
      int n = std::min(int(strlen(urlElem->GetText())), 1024 - 1);
      memcpy_s(renderConfig.url, n * sizeof(char), (char *)urlElem->GetText(), n * sizeof(char));
      // check url is "mpd" or not
      if ((n <= 3) || (n - 3 > 0 && (renderConfig.url[n-1] != 'd' || renderConfig.url[n-2] != 'p' || renderConfig.url[n-3] != 'm'))) {
        LOG(ERROR) << "---INVALID url input! (only remote mpd file supported)---" << std::endl;
        return RENDER_ERROR;
      }
    }
    else
    {
      LOG(ERROR) << " invalid params for url! " << std::endl;
      return RENDER_ERROR;
    }
    XMLElement* stElem = info->FirstChildElement("sourceType");
    if (stElem != NULL)
    {
      renderConfig.sourceType =
        atoi(stElem->GetText());  // FFMPEG_SOURCE=1 or DASH_SOURCE=0
      if (renderConfig.sourceType == 1) {
        LOG(ERROR) << "---NOT support local file for now---" << std::endl;
        return RENDER_ERROR;
      } else if (renderConfig.sourceType > 2 || renderConfig.sourceType < 0) {
        LOG(ERROR) << "---INVALID source type input (0:remote mpd 2:webrtc support)---" << std::endl;
        return RENDER_ERROR;
      }
    }
    else
    {
      LOG(ERROR) << " invalid params for sourceType! " << std::endl;
      return RENDER_ERROR;
    }

    XMLElement* exElem = info->FirstChildElement("enableExtractor");
    if (exElem != NULL)
    {
      renderConfig.enableExtractor =
        atoi(exElem
                 ->GetText());  // 1: for LaterBinding 0: for extractor track
      if (renderConfig.enableExtractor != 0 && renderConfig.enableExtractor != 1) {
        LOG(ERROR) << "---INVALID enableExtractor input (1: for LaterBinding 0: for extractor track)---" << std::endl;
        return RENDER_ERROR;
      }
    }
    else
    {
      LOG(ERROR) << " invalid params for enableExtractor! " << std::endl;
      return RENDER_ERROR;
    }
    XMLElement* hFOVElem = info->FirstChildElement("viewportHFOV");
    XMLElement* vFOVElem = info->FirstChildElement("viewportVFOV");
    if (hFOVElem != NULL && vFOVElem != NULL)
    {
      renderConfig.viewportHFOV = atoi(hFOVElem->GetText());
      renderConfig.viewportVFOV = atoi(vFOVElem->GetText());
      if (renderConfig.viewportHFOV < MINFOV || renderConfig.viewportHFOV > MAXFOV ||
        renderConfig.viewportVFOV < MINFOV || renderConfig.viewportVFOV > MAXFOV)
      {
        LOG(ERROR) << "---INVALID viewportHFOV or viewportVFOV input!---" << std::endl;
        return RENDER_ERROR;
      }
    }
    else
    {
      LOG(ERROR) << " invalid params for viewportHFOV or viewportVFOV! " << std::endl;
      return RENDER_ERROR;
    }
    XMLElement* vWidthElem = info->FirstChildElement("viewportWidth");
    XMLElement* vHeightElem = info->FirstChildElement("viewportHeight");
    if (vWidthElem != NULL && vHeightElem != NULL)
    {
      renderConfig.viewportWidth = atoi(vWidthElem->GetText());
      renderConfig.viewportHeight = atoi(vHeightElem->GetText());
      if (renderConfig.viewportWidth < MINVIEWPORTLEN || renderConfig.viewportWidth > MAXVIEWPORTLEN ||
          renderConfig.viewportHeight < MINVIEWPORTLEN || renderConfig.viewportHeight > MAXVIEWPORTLEN) {
        LOG(ERROR) << "---INVALID viewportWidth or viewportHeight input! (reference-960/960 or 1024/1024)---"
                  << std::endl;
        return RENDER_ERROR;
      }
    }
    else
    {
      LOG(ERROR) << " invalid params for viewportWidth or viewportHeight! " << std::endl;
      return RENDER_ERROR;
    }
    XMLElement* pathElem = info->FirstChildElement("cachePath");
    if (pathElem != NULL)
    {
      renderConfig.cachePath = new char[1024];
      memset_s(renderConfig.cachePath, 1024 * sizeof(char), 0);
      int n = std::min(int(strlen(pathElem->GetText())), 1024 -1);
      memcpy_s(renderConfig.cachePath, n * sizeof(char), (char *)pathElem->GetText(), n * sizeof(char));
    }
    else
    {
      LOG(ERROR) << " invalid params for cachePath! " << std::endl;
      return RENDER_ERROR;
    }
    XMLElement* maxWidthElement = info->FirstChildElement("maxVideoDecodeWidth");
    XMLElement* maxHeightElement = info->FirstChildElement("maxVideoDecodeHeight");
    if (nullptr == maxWidthElement || nullptr == maxHeightElement)
    {
      return RENDER_ERROR;
    }
    renderConfig.maxVideoDecodeWidth = atoi(maxWidthElement->GetText());  // limited video decoder width of device.
    if (renderConfig.maxVideoDecodeWidth <= 0 || renderConfig.maxVideoDecodeWidth >= UINT32_MAX) {
      LOG(ERROR) << "---INVALID maxVideoDecodeWidth input---" << std::endl;
      return RENDER_ERROR;
    }
    renderConfig.maxVideoDecodeHeight = atoi(maxHeightElement->GetText());  // limited video decoder height of device.
    if (renderConfig.maxVideoDecodeHeight <= 0 || renderConfig.maxVideoDecodeHeight >= UINT32_MAX) {
      LOG(ERROR) << "---INVALID maxVideoDecodeHeight input---" << std::endl;
      return RENDER_ERROR;
    }
    XMLElement* viewportHFOVElement = info->FirstChildElement("viewportHFOV");
    XMLElement* viewportVFOVElement = info->FirstChildElement("viewportVFOV");
    if (nullptr == viewportHFOVElement || nullptr == viewportVFOVElement)
    {
      return RENDER_ERROR;
    }
    renderConfig.viewportHFOV = atoi(viewportHFOVElement->GetText());
    renderConfig.viewportVFOV = atoi(viewportVFOVElement->GetText());
    if (renderConfig.viewportHFOV < MINFOV || renderConfig.viewportHFOV > MAXFOV ||
        renderConfig.viewportVFOV < MINFOV || renderConfig.viewportVFOV > MAXFOV) {
      LOG(ERROR) << "---INVALID viewportHFOV or viewportVFOV input!---" << std::endl;
      return RENDER_ERROR;
    }

    XMLElement* logLevelElem = info->FirstChildElement("minLogLevel");
    if (logLevelElem != NULL)
    {
      std::string logLevel = logLevelElem->GetText();
      if (logLevel == "INFO")
      {
        renderConfig.minLogLevel = google::INFO;
      }
      else if (logLevel == "WARNING")
      {
        renderConfig.minLogLevel = google::WARNING;
      }
      else if (logLevel == "ERROR")
      {
        renderConfig.minLogLevel = google::ERROR;
      }
      else if (logLevel == "FATAL")
      {
        renderConfig.minLogLevel = google::FATAL;
      }
      else
      {
        LOG(ERROR) << "Invalid min log level setting!" << endl;
        return RENDER_ERROR;
      }
    }
    else
    {
      LOG(ERROR) << " invalid params for minLogLevel! " << std::endl;
      return RENDER_ERROR;
    }

    // predictor option
    XMLElement *predictor = info->FirstChildElement("predict");
    if (predictor) {
      const XMLAttribute *enable = predictor->FirstAttribute();
      if (NULL == enable) return RENDER_ERROR;
      renderConfig.enablePredictor = atoi(enable->Value());
      renderConfig.predictPluginName = (char *)"";
      renderConfig.libPath = (char *)"";
      XMLElement* pluginElem = predictor->FirstChildElement("plugin");
      XMLElement* pPathElem = predictor->FirstChildElement("path");
      if (renderConfig.enablePredictor) {
        if (pluginElem != NULL && pPathElem != NULL)
        {
          renderConfig.predictPluginName = new char[1024];
          memcpy_s(renderConfig.predictPluginName, 1024, (char *)pluginElem->GetText(), 1024);
          renderConfig.libPath = new char[1024];
          memcpy_s(renderConfig.libPath, 1024, (char *)pPathElem->GetText(), 1024);
        }
        else
        {
          LOG(ERROR) << "Invalid plugin name or path!" << endl;
          return RENDER_ERROR;
        }
      }
    }

    // in time viewport update option
    XMLElement *viewportUpdate = info->FirstChildElement("intimeviewportupdate");
    if (viewportUpdate) {
      const XMLAttribute *enable = viewportUpdate->FirstAttribute();
      if (NULL == enable) return RENDER_ERROR;
      renderConfig.enableInTimeViewportUpdate = atoi(enable->Value());
      renderConfig.maxResponseTimesInOneSeg = 0;
      renderConfig.maxCatchupWidth = 0;
      renderConfig.maxCatchupHeight = 0;
      XMLElement* responseElem = viewportUpdate->FirstChildElement("responseTimesInOneSeg");
      XMLElement* maxWidthElem = viewportUpdate->FirstChildElement("maxCatchupWidth");
      XMLElement* maxHeightElem = viewportUpdate->FirstChildElement("maxCatchupHeight");
      if (renderConfig.enableInTimeViewportUpdate) {
        if (responseElem != NULL && maxWidthElem != NULL && maxHeightElem != NULL)
        {
          renderConfig.maxResponseTimesInOneSeg = atoi(responseElem->GetText());
          renderConfig.maxCatchupWidth = atoi(maxWidthElem->GetText());
          renderConfig.maxCatchupHeight = atoi(maxHeightElem->GetText());
        }
        else
        {
          LOG(ERROR) << "Invalid params in viewport update!" << endl;
          return RENDER_ERROR;
        }
      }
    }

    // PathOf360SCVPPlugins
    XMLElement* pathof360SCVPPlugin = info->FirstChildElement("PathOf360SCVPPlugins");
    if (pathof360SCVPPlugin != NULL)
    {
      renderConfig.pathof360SCVPPlugin = new char[1024];
      memcpy_s(renderConfig.pathof360SCVPPlugin, 1024, (char *)pathof360SCVPPlugin->GetText(), 1024);
    }
    else
    {
      renderConfig.pathof360SCVPPlugin = nullptr;
      // LOG(INFO) << " not settings for PathOf360SCVPPlugins! " << std::endl;
    }

    return RENDER_STATUS_OK;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception when parse the file: " << xml_file << std::endl;
    LOG(ERROR) << "Exception: " << ex.what() << std::endl;
    return RENDER_ERROR;
  }
}

RenderContext* InitRenderContext(struct RenderConfig config)
{
  RenderContext *context = new GLFWRenderContext(config);
  if (context == nullptr)
  {
    LOG(ERROR) << "Error in contex create!" << endl;
    return nullptr;
  }
  void *window = context->InitContext();
  if (window == nullptr)
  {
    LOG(ERROR) << "Failed to initial render context!" << endl;
    SAFE_DELETE(context);
    return nullptr;
  }
  return context;
}

int main(int32_t argc, char *argv[]) {
  // 1. input from xml configuration
  struct RenderConfig renderConfig;
  if (RENDER_STATUS_OK != parseRenderFromXml("config.xml", renderConfig)) {
    LOG(ERROR) << "Failed to parse the render xml config file!" << std::endl;
    SAFE_DELETE_ARRAY(renderConfig.pathof360SCVPPlugin);
    SAFE_DELETE_ARRAY(renderConfig.url);
    SAFE_DELETE_ARRAY(renderConfig.cachePath);
    if (renderConfig.enablePredictor) {
      SAFE_DELETE_ARRAY(renderConfig.libPath);
      SAFE_DELETE_ARRAY(renderConfig.predictPluginName);
    }
    return RENDER_ERROR;
  }
  GlogWrapper m_glogWrapper((char*)"glogRender", renderConfig.minLogLevel);

  DIR *dir = opendir(renderConfig.cachePath);
  if (dir) {
    closedir(dir);
  } else {
    LOG(INFO) << "Failed to open the cache path: " << renderConfig.cachePath << " , create a folder with this path!" << endl;
    int checkdir = mkdir(renderConfig.cachePath, 0777);
    if (checkdir) {
      SAFE_DELETE_ARRAY(renderConfig.pathof360SCVPPlugin);
      SAFE_DELETE_ARRAY(renderConfig.url);
      SAFE_DELETE_ARRAY(renderConfig.cachePath);
      if (renderConfig.enablePredictor) {
        SAFE_DELETE_ARRAY(renderConfig.libPath);
        SAFE_DELETE_ARRAY(renderConfig.predictPluginName);
      }
      LOG(ERROR) << "Uable to create cache path! " << endl;
      return RENDER_ERROR;
    }
  }
  // 2.initial player
  MediaPlayer_Linux *player = new MediaPlayer_Linux();
  player->Create(renderConfig);
  RenderContext* context = InitRenderContext(renderConfig);
  // 3.open process
  if (RENDER_STATUS_OK != player->Start(context)) {
    delete player;
    player = NULL;
    return RENDER_ERROR;
  }
  // 4.play process
  player->Play();
  delete player;
  player = NULL;
  SAFE_DELETE_ARRAY(renderConfig.pathof360SCVPPlugin);
  SAFE_DELETE_ARRAY(renderConfig.url);
  SAFE_DELETE_ARRAY(renderConfig.cachePath);
  if (renderConfig.enablePredictor) {
    SAFE_DELETE_ARRAY(renderConfig.libPath);
    SAFE_DELETE_ARRAY(renderConfig.predictPluginName);
  }
  return 0;
}
#endif
