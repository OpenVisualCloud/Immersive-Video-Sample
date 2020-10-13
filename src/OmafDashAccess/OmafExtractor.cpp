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

/*
 * File:   OmafExtractor.cpp
 * Author: media
 *
 * Created on May 24, 2019, 4:04 PM
 */

#include "OmafExtractor.h"

VCD_OMAF_BEGIN

OmafExtractor::OmafExtractor() {}

OmafExtractor::~OmafExtractor() {}

OmafExtractor::OmafExtractor(AdaptationSetElement* pAdaptationSet, ProjectionFormat pf, bool isExtractorTrack)
    : OmafAdaptationSet(pAdaptationSet, pf, isExtractorTrack) {}

void OmafExtractor::AddDependAS(OmafAdaptationSet* as) {
  std::vector<int> vecID;

  if (nullptr != this->mPreselID) {
    vecID = mPreselID->SelAsIDs;
  }
  if (1 < this->mDependIDs.size()) {
    vecID = mDependIDs;
  }

  for (auto it = vecID.begin(); it != vecID.end(); it++) {
    int id = int(*it);
    if (id == as->GetID()) {
      this->mAdaptationSets[id] = as;
      return;
    }
  }
}

std::list<int> OmafExtractor::GetDependTrackID() {
  std::list<int> trackList;
  for (auto it = mAdaptationSets.begin(); it != mAdaptationSets.end(); it++) {
    OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
    trackList.push_back(pAS->GetTrackNumber());
  }

  return trackList;
}

SampleData* OmafExtractor::ReadSample() { return nullptr; }

VCD_OMAF_END;
