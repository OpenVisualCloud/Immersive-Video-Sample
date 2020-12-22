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

//! \file:   RegionData.cpp
//! \brief:  the class for Region Info
//! \detail: it's class to describe region info
//!
//! Created on May 21, 2020, 1:18 PM
//!

#include "RegionData.h"
#include <string.h>
#include "Common.h"

VCD_NS_BEGIN

RegionData::RegionData(RegionWisePacking* rwpk, uint32_t sourceNumber, SourceResolution* qtyRes) {
  m_sourceInRegion = sourceNumber;

  m_regionWisePacking = new RegionWisePacking;
  *m_regionWisePacking = *rwpk;
  m_regionWisePacking->rectRegionPacking = new RectangularRegionWisePacking[rwpk->numRegions];
  memcpy_s(m_regionWisePacking->rectRegionPacking, rwpk->numRegions * sizeof(RectangularRegionWisePacking),
           rwpk->rectRegionPacking, rwpk->numRegions * sizeof(RectangularRegionWisePacking));

  m_sourceInfo = new SourceResolution[sourceNumber];
  for (uint32_t i = 0; i < sourceNumber; i++) {
    m_sourceInfo[i].qualityRanking = qtyRes[i].qualityRanking;
    m_sourceInfo[i].left = qtyRes[i].left;
    m_sourceInfo[i].top = qtyRes[i].top;
    m_sourceInfo[i].width = qtyRes[i].width;
    m_sourceInfo[i].height = qtyRes[i].height;
  }
}

RegionData::~RegionData() {
  m_sourceInRegion = 0;
  if (m_regionWisePacking != NULL) {
    if (m_regionWisePacking->rectRegionPacking != NULL) {
      delete[] m_regionWisePacking->rectRegionPacking;
      m_regionWisePacking->rectRegionPacking = NULL;
    }
    delete m_regionWisePacking;
    m_regionWisePacking = NULL;
  }
  if (m_sourceInfo != NULL) {
    delete[] m_sourceInfo;
    m_sourceInfo = NULL;
  }
}

VCD_NS_END
