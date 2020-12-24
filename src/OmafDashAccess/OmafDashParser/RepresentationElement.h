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
//! \file:   RepresentationElement.h
//! \brief:  Representation element class
//!

#ifndef REPRESENTATIONELEMENT_H
#define REPRESENTATIONELEMENT_H
#include "OmafElementBase.h"
#include "AudioChannelCfgElement.h"
#include "SegmentElement.h"

VCD_OMAF_BEGIN

class RepresentationElement : public OmafElementBase {
 public:
  //!
  //! \brief Constructor
  //!
  RepresentationElement();

  //!
  //! \brief Destructor
  //!
  virtual ~RepresentationElement();

  //!
  //! \brief    Set function for m_id member
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_id
  //!           m_id member in class
  //! \param    [in] Id
  //!           m_id name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_id, Id);

  //!
  //! \brief    Set function for m_codecs member
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_codecs
  //!           m_codecs member in class
  //! \param    [in] Codecs
  //!           m_codecs name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_codecs, Codecs);

  //!
  //! \brief    Set function for m_mimeType member
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_mimeType
  //!           m_mimeType member in class
  //! \param    [in] MimeType
  //!           m_mimeType name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_mimeType, MimeType);

  //!
  //! \brief    Set function for m_width member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_width
  //!           m_width member in class
  //! \param    [in] Width
  //!           m_width name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_width, Width);

  //!
  //! \brief    Set function for m_height member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_height
  //!           m_height member in class
  //! \param    [in] Height
  //!           m_height name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_height, Height);

  MEMBER_SET_AND_GET_FUNC(int32_t, m_audioSamplingRate, AudioSamplingRate);
  //!
  //! \brief    Set function for m_frameRate member
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_frameRate
  //!           m_frameRate member in class
  //! \param    [in] FrameRate
  //!           m_frameRate name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_frameRate, FrameRate);

  //!
  //! \brief    Set function for m_sar member
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_sar
  //!           m_sar member in class
  //! \param    [in] Sar
  //!           m_sar name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_sar, Sar);

  //!
  //! \brief    Set function for m_startWithSAP member
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_startWithSAP
  //!           m_xmlns_omaf member in class
  //! \param    [in] StartWithSAP
  //!           m_startWithSAP name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_startWithSAP, StartWithSAP);

  //!
  //! \brief    Set function for m_qualityRanking member
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_qualityRanking
  //!           m_qualityRanking member in class
  //! \param    [in] QualityRanking
  //!           m_qualityRanking name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_qualityRanking, QualityRanking);

  //!
  //! \brief    Set function for m_bandwidth member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_bandwidth
  //!           m_bandwidth member in class
  //! \param    [in] Bandwidth
  //!           m_bandwidth name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_bandwidth, Bandwidth);

  //!
  //! \brief    Set function for m_segment member
  //!
  //! \param    [in] SegmentElement
  //!           value to set
  //! \param    [in] m_segment
  //!           m_segment member in class
  //! \param    [in] Segment
  //!           m_segment name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(SegmentElement*, m_segment, Segment);

  MEMBER_SET_AND_GET_FUNC(AudioChannelConfigurationElement*, m_audioChlCfg, AudioChlCfg);

  //!
  //! \brief    Set value to m_dependencyId member
  //!
  //! \param    [in] ids
  //!           A string of value to set
  //!
  //! \return   void
  //!
  void SetDependencyID(string ids) { SplitString(ids, m_dependencyId, ","); }

  //!
  //! \brief    Get all items in m_dependencyId
  //!
  //! \return   vector<string>
  //!           vector of string
  //!
  vector<string> GetDependencyIDs() { return m_dependencyId; }

private:
    RepresentationElement& operator=(const RepresentationElement& other) { return *this; };
    RepresentationElement(const RepresentationElement& other) { /* do not create copies */ };

 private:
  string m_id;                    //!< the id attribute
  string m_codecs;                //!< the codecs attribute
  string m_mimeType;              //!< the mimeType attribute
  int32_t m_width;                //!< the width attribute
  int32_t m_height;               //!< the height attribute
  string m_frameRate;             //!< the frameRate attribute
  int32_t m_audioSamplingRate;   //!< the audio sampling rate attribute
  string m_sar;                   //!< the sar attribute
  string m_startWithSAP;          //!< the startWithSAP attribute
  string m_qualityRanking;        //!< the qualityRanking attribute
  int32_t m_bandwidth;            //!< the bandwidth attribute
  vector<string> m_dependencyId;  //!< the dependencyId attribute

  SegmentElement* m_segment;  //!< the SegmentTemplate child elements
  AudioChannelConfigurationElement* m_audioChlCfg;
};

VCD_OMAF_END;

#endif  // REPRESENTATIONELEMENT_H
