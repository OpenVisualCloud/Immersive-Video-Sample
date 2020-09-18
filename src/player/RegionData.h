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

//! \file:   RegionData.h
//! \brief:  the class for Region Info
//! \detail: it's class to describe region info
//!
//! Created on May 21, 2020, 1:18 PM
//!

#ifndef _REGIONDATA_H_
#define _REGIONDATA_H_

#include "ns_def.h"
#include <stdint.h>
#include <string>
#include "data_type.h"

VCD_NS_BEGIN

class RegionData {
public:
    //!
    //! \brief  construct
    //!
    RegionData(){
        m_sourceInRegion = 0;
        m_regionWisePacking = NULL;
        m_sourceInfo = NULL;
    };
    RegionData(RegionWisePacking* rwpk, uint32_t sourceNumber, SourceResolution* qtyRes);
    //!
    //! \brief  de-construct
    //!
    ~RegionData();

    uint32_t GetSourceInRegion() { return m_sourceInRegion; };
    void SetSourceInRegion(uint32_t num){ m_sourceInRegion = num; };

    RegionWisePacking* GetRegionWisePacking() { return m_regionWisePacking; };

    SourceResolution* GetSourceInfo() { return m_sourceInfo; }

private:
    RegionData& operator=(const RegionData& other) { return *this; };
    RegionData(const RegionData& other) { /* do not create copies */ };

private:

    uint32_t m_sourceInRegion;
    RegionWisePacking *m_regionWisePacking;
    SourceResolution *m_sourceInfo;
};

VCD_NS_END;

#endif /* _REGIONDATA_H_ */
