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
//! \file:   OmafExtractor.h
//! \brief:
//! \detail:
//! Created on May 24, 2019, 4:04 PM
//!

#ifndef OMAFEXTRACTOR_H
#define OMAFEXTRACTOR_H

#include "OmafAdaptationSet.h"

VCD_OMAF_BEGIN

//!
//! \class:   OmafExtractor
//! \brief:   Extractor derived from OmafAdaptationSet since Extractor is also
//!           one adpationSet in MPD
//!
class OmafExtractor : public OmafAdaptationSet {
 public:
  //!
  //! \brief  construct
  //!
  OmafExtractor();

  //!
  //! \brief  construct from AdaptationSetElement
  //!
  OmafExtractor(AdaptationSetElement* pAdaptationSet, ProjectionFormat pf, bool isExtractorTrack);

  //!
  //! \brief  de-construct
  //!
  virtual ~OmafExtractor();

 public:
  //!
  //! \brief  Reading a Sample from sample list.
  //! \return the SampleData read from Sample list
  //!
  virtual SampleData* ReadSample();

  virtual OmafExtractor* GetClassType() { return this; };

  //!
  //! \brief  add Omaf Adaptation Set which is used by the extractor, it will
  //!         be called by OmafMediaStream when it is initialization
  //!
  void AddDependAS(OmafAdaptationSet* as);

  //!
  //! \brief  Get all depended adaptation sets
  //!
  std::map<int, OmafAdaptationSet*> GetDependAdaptationSets() { return mAdaptationSets; };

  //!
  //! \brief  get the list of depended track IDs
  //!
  std::list<int> GetDependTrackID();

  std::map<int, OmafAdaptationSet*> GetCurrentTracksMap()
  {
    return mAdaptationSets;
  }

  //bool IsExtractor() override { return true; }

 private:
  std::map<int, OmafAdaptationSet*> mAdaptationSets;  //<! the Adapation lists the extractor depends
};

VCD_OMAF_END;

#endif /* OMAFEXTRACTOR_H */
