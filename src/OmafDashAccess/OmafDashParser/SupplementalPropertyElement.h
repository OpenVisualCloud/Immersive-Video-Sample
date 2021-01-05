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
//! \file:   MPDElement.h
//! \brief:  MPD file element class
//!

#ifndef SUPPLEMENTPROPERTYELEMENT_H
#define SUPPLEMENTPROPERTYELEMENT_H
#include "OmafElementBase.h"
#include "DescriptorElement.h"
#include "SphRegionQualityElement.h"
#include "TwoDRegionQualityElement.h"

VCD_OMAF_BEGIN

class SupplementalPropertyElement: public OmafElementBase, public DescriptorElement
{
public:

    //!
    //! \brief Constructor
    //!
    SupplementalPropertyElement();

    //!
    //! \brief Destructor
    //!
    virtual ~SupplementalPropertyElement();

    //!
    //! \brief    Parse SchemeIdUri and it's value
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus ParseSchemeIdUriAndValue();

    //!
    //! \brief   Set value to member m_sphRegionQuality
    //!
    //! \param    [in] sphRegionQuality
    //!           pointer of SphRegionQualityElement class
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus SetSphereRegionQuality(SphRegionQualityElement* sphRegionQuality);

    ODStatus SetTwoDRegionQuality(TwoDRegionQualityElement* twoDRegionQuality);
    //!
    //! \brief    Get member m_srd
    //!
    //! \return   OmafSrd*
    //!           pointer of OmafSrd class
    //!
    OmafSrd* GetSRD() {return m_srd;}

    //!
    //! \brief    Get member m_srqr
    //!
    //! \return   SphereQuality*
    //!           pointer of SphereQuality class
    //!
    SphereQuality* GetSRQR() {return m_srqr;}

    //!
    //! \brief    Get member m_preselection
    //!
    //! \return   PreselValue*
    //!           pointer of PreselValue class
    //!
    PreselValue* GetPreselection() {return m_preselection;}

    //!
    //! \brief    Get member m_sphRegionQuality
    //!
    //! \return   ContentCoverage*
    //!           pointer of ContentCoverage class
    //!
    ContentCoverage* GetContentCoverage() {return m_sphRegionQuality ? m_sphRegionQuality->GetContentCoverage() : nullptr;}

    map<int32_t, TwoDQualityInfo> GetTwoDRegionQualityInfos() { return m_twoDQualityInfos; }

private:
    SupplementalPropertyElement& operator=(const SupplementalPropertyElement& other) { return *this; };
    SupplementalPropertyElement(const SupplementalPropertyElement& other) { /* do not create copies */ };

private:

    SphRegionQualityElement      *m_sphRegionQuality; //!< pointer of the omaf:sphRegionQuality child elements
    OmafSrd                      *m_srd;              //!< pointer of the OmafSrd child elements
    SphereQuality                *m_srqr;             //!< pointer of the SphereQuality child elements
    PreselValue                  *m_preselection;     //!< pointer of the PreselValue child elements
    TwoDRegionQualityElement     *m_twoDRegionQuality;

    map<int32_t, TwoDQualityInfo> m_twoDQualityInfos;
};

VCD_OMAF_END;

#endif //SUPPLEMENTPROPERTYELEMENT_H
