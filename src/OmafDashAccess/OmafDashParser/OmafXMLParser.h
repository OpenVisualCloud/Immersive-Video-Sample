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
//! \file:   OmafXMLParser.h
//! \brief:  generate basic informations with OMAF DASH standard
//!
#ifndef OMAFXMLPARSER_H
#define OMAFXMLPARSER_H

#include "../../utils/tinyxml2.h"
#include "../OmafDashDownload/OmafCurlEasyHandler.h"
#include "Common.h"

#include "OmafMPDReader.h"
#include "OmafXMLElement.h"

VCD_OMAF_BEGIN

//!
//! \class:  OmafXMLParser
//! \brief:  OMAF XML parser
//!
class OmafXMLParser {
 public:
  //!
  //! \brief Constructor
  //!
  OmafXMLParser();

  //!
  //! \brief Destructor
  //!
  virtual ~OmafXMLParser();

  //!
  //! \brief    Generate XML tree and MPD tree
  //!
  //! \param    [in] url
  //!           MPD file url
  //!           [in] cacheDir
  //!           cache directory
  //!
  //! \return   ODStatus
  //!           OD_STATUS_SUCCESS if success, else fail reason
  //!
  ODStatus Generate(string url, string cacheDir);

  //!
  //! \brief    Download MPD file
  //!
  //! \param    [in] url
  //!           MPD file url
  //!           [in] cacheDir
  //!           cache directory
  //!
  //! \return   string
  //!           the name of downloaded file
  //!
  std::string DownloadXMLFile(string url, string cacheDir);

  //!
  //! \brief    Generate XML tree
  //!
  //! \param    [in] elmt
  //!           root tinyxml element
  //!
  //! \return   OmafXMLElement
  //!           root OMAF XML element
  //!
  OmafXMLElement* BuildXMLElementTree(tinyxml2::XMLElement* elmt);

  //!
  //! \brief    Generate MPD tree with XML elements
  //!
  //! \param    [in] root
  //!           root XML element
  //!
  //! \return   ODStatus
  //!           OD_STATUS_SUCCESS if success, else fail reason
  //!
  ODStatus BuildMPDwithXMLElements(OmafXMLElement* root);

  //!
  //! \brief    Get generated MPD element
  //!
  //! \return   MPDElement
  //!           OMAF MPD element
  //!
  MPDElement* GetGeneratedMPD();

  void SetOmafHttpParams(const OmafDashHttpProxy& http_proxy, const OmafDashHttpParams& http_params) {
    m_curl_params.http_proxy_ = http_proxy;
    m_curl_params.http_params_ = http_params;
  }

private:
    OmafXMLParser& operator=(const OmafXMLParser& other) { return *this; };
    OmafXMLParser(const OmafXMLParser& other) { /* do not create copies */ };

 private:
  //!
  //! \brief    Read attributes from tinyxml element
  //!
  //! \param    [in] element
  //!           OMAF XML element
  //! \param    [in] orgElement
  //!           tinyxml element
  //!
  //! \return   void
  //!
  void ReadAttributes(OmafXMLElement* element, tinyxml2::XMLElement* orgElement);

  //!
  //! \brief    Write data to file
  //!
  //! \param    [in] ptr
  //!           data pointer
  //! \param    [in] size
  //!           data size
  //! \param    [in] nmemb
  //!           data type size
  //! \param    [in] fp
  //!           file handle
  //!
  //! \return   size_t
  //!           size of wrote data
  //!
  static size_t WriteData(void* ptr, size_t size, size_t nmemb, FILE* fp);

  tinyxml2::XMLDocument* m_xmlDoc = nullptr;  //!< tinyxml document
  string m_path;                              //!< url path
  OmafReaderBase* m_mpdReader;                //!< MPD reader
  CurlParams m_curl_params;
};

VCD_OMAF_END

#endif  // OMAFXMLPARSER_H
