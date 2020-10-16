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

#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include "../utils/tinyxml2.h"
#include "Common.h"
#include "Player.h"

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
      renderConfig.url = urlElem->GetText();
      // string fileType(renderConfig.url + strlen(renderConfig.url) - 3, renderConfig.url + strlen(renderConfig.url));
      string fileType = renderConfig.url.substr(renderConfig.url.size() - 3);
      if (fileType != "mpd") {
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
      else if (renderConfig.sourceType == 2) {
        LOG(ERROR) << "---INVALID source type input (webrtc is not enabled, please enable webrtc!)---" << std::endl;
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
      renderConfig.cachePath = pathElem->GetText();
    }
    else
    {
      LOG(ERROR) << " invalid params for cachePath! " << std::endl;
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
      renderConfig.predictPluginName = "";
      renderConfig.libPath = "";
      XMLElement* pluginElem = predictor->FirstChildElement("plugin");
      XMLElement* pPathElem = predictor->FirstChildElement("path");
      if (renderConfig.enablePredictor) {
        if (pluginElem != NULL && pPathElem != NULL)
        {
          renderConfig.predictPluginName = pluginElem->GetText();
          renderConfig.libPath = pPathElem->GetText();
        }
        else
        {
          LOG(ERROR) << "Invalid plugin name or path!" << endl;
          return RENDER_ERROR;
        }
      }
    }

    return RENDER_STATUS_OK;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception when parse the file: " << xml_file << std::endl;
    LOG(ERROR) << "Exception: " << ex.what() << std::endl;
    return RENDER_ERROR;
  }
}

int main(int32_t argc, char *argv[]) {
  // 1. input from xml configuration
  struct RenderConfig renderConfig;
  if (RENDER_STATUS_OK != parseRenderFromXml("config.xml", renderConfig)) {
    LOG(ERROR) << "Failed to parse the render xml config file!" << std::endl;
    return RENDER_ERROR;
  }
  GlogWrapper m_glogWrapper((char*)"glogRender", renderConfig.minLogLevel);

  string cacheDir = renderConfig.cachePath;
  DIR *dir = opendir(cacheDir.c_str());
  if (dir) {
    closedir(dir);
  } else {
    LOG(INFO) << "Failed to open the cache path: " << cacheDir << " , create a folder with this path!" << endl;
    int checkdir = mkdir(cacheDir.c_str(), 0777);
    if (checkdir) {
      LOG(ERROR) << "Uable to create cache path: " << cacheDir << endl;
      return RENDER_ERROR;
    }
  }

  // 2.initial player
  Player *player = new Player(renderConfig);
  // 3.open process
  if (RENDER_STATUS_OK != player->Open()) {
    delete player;
    player = NULL;
    return RENDER_ERROR;
  }
  // 4.play process
  player->Play();
  delete player;
  player = NULL;
  return 0;
}
