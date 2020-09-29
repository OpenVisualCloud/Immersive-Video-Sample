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

 */

//!
//! \file:   OmafXMLParser.cpp
//! \brief:  generate basic informations with OMAF DASH standard
//!

#include "OmafXMLParser.h"

#include <fstream>

VCD_OMAF_BEGIN

using namespace tinyxml2;

OmafXMLParser::OmafXMLParser() {
  m_mpdReader = nullptr;
  m_xmlDoc = nullptr;
}

OmafXMLParser::~OmafXMLParser() {
  if (m_mpdReader) m_mpdReader->Close();
  SAFE_DELETE(m_mpdReader);
  SAFE_DELETE(m_xmlDoc);
}

size_t OmafXMLParser::WriteData(void* ptr, size_t size, size_t nmemb, FILE* fp) { return fwrite(ptr, size, nmemb, fp); }

std::string OmafXMLParser::DownloadXMLFile(string url, string cache_dir) {
  if (cache_dir.empty()) {
    cache_dir = ".";
  }

  std::string fileName =
      cache_dir + "/" + url.substr(url.find_last_of('/') + 1, url.length() - url.find_last_of('/') - 1);

  std::ofstream mpd_file;
  mpd_file.open(fileName, ios::out);

  OmafCurlEasyDownloader downloader(OmafCurlEasyDownloader::CurlWorkMode::EASY_MODE);
  int ret = downloader.init(m_curl_params);
  if (ret == ERROR_NONE) {
    OMAF_LOG(LOG_INFO, "To download the xml mpd with url: %s\n", url.c_str());
    OMAF_LOG(LOG_INFO, "Save the xml mpd file to path: %s\n", fileName.c_str());
    ret = downloader.open(url);
    if (ret == ERROR_NONE) {
      ret = downloader.start(
          0, 0,
          [&mpd_file, fileName](std::unique_ptr<StreamBlock> sb) {
            OMAF_LOG(LOG_INFO, "Receive the stream block, size=%lld\n", sb->size());
            if (mpd_file.is_open()) {
              mpd_file.write(sb->cbuf(), sb->size());
            } else {
              OMAF_LOG(LOG_ERROR, "The file is not in open state, file: %s\n", fileName.c_str());
            }
          },
          [url](OmafCurlEasyDownloader::State s) {
            OMAF_LOG(LOG_INFO, "Download state: %d for url: %s\n", static_cast<int>(s), url.c_str());
          });
      if (ret == ERROR_NONE) {
        OMAF_LOG(LOG_INFO, "Success to start the mpd downloader!\n");
      } else {
        OMAF_LOG(LOG_ERROR, "Failed to start the mpd downloader, err=%d\n", ret);
      }

      if (mpd_file.is_open()) {
        mpd_file.close();
      }
      return fileName;
    }
  }
  OMAF_LOG(LOG_ERROR, "Failed to download the mpd file, whose url:%s\n", url.c_str());
  return std::string();
}

ODStatus OmafXMLParser::Generate(string url, string cacheDir) {
  ODStatus ret = OD_STATUS_SUCCESS;

  m_path = url.substr(0, url.find_last_of('/'));

  // define the url is local or through network with prefix
  string url_prefix = "http";
  bool local = m_path.length() < url_prefix.length() || m_path.substr(0, 4) != url_prefix;

  string fileName = local ? url : DownloadXMLFile(url, cacheDir);
  if (!fileName.length()) return OD_STATUS_INVALID;

  m_xmlDoc = new XMLDocument();
  CheckNullPtr_PrintLog_ReturnStatus(m_xmlDoc, "Failed to create XMLDocument with tinyXML.\n", LOG_ERROR,
                                     OD_STATUS_OPERATION_FAILED);
  OMAF_LOG(LOG_INFO, "To parse the mpd file: %s\n", fileName.c_str());
  XMLError result = m_xmlDoc->LoadFile(fileName.c_str());
  if (result != XML_SUCCESS) return OD_STATUS_OPERATION_FAILED;

  XMLElement* elmt = m_xmlDoc->FirstChildElement();
  CheckNullPtr_PrintLog_ReturnStatus(elmt, "Failed to get element from XML Doc.\n", LOG_ERROR, OD_STATUS_OPERATION_FAILED);

  OmafXMLElement* root = BuildXMLElementTree(elmt);
  if (!root) {
    OMAF_LOG(LOG_ERROR, "Build XML elements tree failed!\n");
    return OD_STATUS_OPERATION_FAILED;
  }

  ret = BuildMPDwithXMLElements(root);
  if (ret != OD_STATUS_SUCCESS) {
    OMAF_LOG(LOG_ERROR, "Build MPD tree failed!\n");
    return OD_STATUS_OPERATION_FAILED;
  }

  // delete the downloaded mpd file after it's parsed.
  // if (!local && 0 != remove(fileName.c_str())) {
  //  LOG(WARNING) << "Failed to delete the downloaded MPD file: " << fileName);
  //}

  return ret;
}

OmafXMLElement* OmafXMLParser::BuildXMLElementTree(XMLElement* elmt) {
  CheckNullPtr_PrintLog_ReturnNullPtr(elmt, "Failed to get element from XML Doc.\n", LOG_WARNING);

  const char* name = elmt->Value();
  if (!name) return nullptr;

  OmafXMLElement* element = new OmafXMLElement();
  CheckNullPtr_PrintLog_ReturnNullPtr(element, "Failed to create element.\n", LOG_WARNING);

  element->SetName(name);
  element->SetPath(m_path);

  const char* text = elmt->GetText();
  if (text) element->SetText(text);

  this->ReadAttributes(element, elmt);

  if (elmt->NoChildren()) return element;

  // read all child element
  XMLElement* child = elmt->FirstChildElement();
  while (child) {
    OmafXMLElement* childElement = this->BuildXMLElementTree(child);
    // only add valid child element
    if (childElement) element->AddChildElement(childElement);

    child = child->NextSiblingElement();
  }

  return element;
}

ODStatus OmafXMLParser::BuildMPDwithXMLElements(OmafXMLElement* root) {
  ODStatus ret = OD_STATUS_SUCCESS;

  m_mpdReader = new OmafMPDReader(root);
  if (!m_mpdReader) return OD_STATUS_INVALID;

  m_mpdReader->BuildMPD();

  return ret;
}

void OmafXMLParser::ReadAttributes(OmafXMLElement* element, XMLElement* orgElement) {
  const XMLAttribute* attribute = orgElement->FirstAttribute();
  if (!attribute) return;

  // read all attributes
  while (attribute) {
    const char* attrKey = attribute->Name();
    const char* attrValue = attribute->Value();
    element->AddAttribute(attrKey, attrValue);

    attribute = attribute->Next();
  }
}

MPDElement* OmafXMLParser::GetGeneratedMPD() {
  if (!m_mpdReader) {
    OMAF_LOG(LOG_ERROR, "please generate MPD tree firstly.\n");
    return nullptr;
  }

  return m_mpdReader->GetMPD();
}

VCD_OMAF_END
