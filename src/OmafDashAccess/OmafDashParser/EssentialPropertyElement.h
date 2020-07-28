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
//! \file:   EssetntialPropertyElement.h
//! \brief:  EssetntialProperty element class
//!

#ifndef ESSENTIALPROPERTYELEMENT_H
#define ESSENTIALPROPERTYELEMENT_H
#include"OmafElementBase.h"
#include "DescriptorElement.h"

VCD_OMAF_BEGIN

class EssentialPropertyElement: public OmafElementBase, public DescriptorElement
{
public:

    //!
    //! \brief Constructor
    //!
    EssentialPropertyElement();

    //!
    //! \brief Destructor
    //!
    virtual ~EssentialPropertyElement();

    //!
    //! \brief    Set function for m_projectionType member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_projectionType
    //!           m_projectionType member in class
    //! \param    [in] ProjectionType
    //!           m_projectionType name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_projectionType, ProjectionType);

    //!
    //! \brief    Set function for m_rwpkPackingType member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_rwpkPackingType
    //!           m_rwpkPackingType member in class
    //! \param    [in] RwpkPackingType
    //!           m_rwpkPackingType name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_rwpkPackingType, RwpkPackingType);

    //!
    //! \brief    Parse SchemeIdUri and it's value
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus ParseSchemeIdUriAndValue();

    //!
    //! \brief    Get value of member m_srd
    //!
    //! \return   OmafSrd*
    //!           A pointer of OmafSrd class
    //!
    OmafSrd* GetSRD() {return m_srd;}

    //!
    //! \brief    Get value of member m_projectionType
    //!
    //! \return   ProjectionFormat
    //!           An instance of ProjectionFormat class
    //!
    ProjectionFormat GetProjectionFormat(){return m_pf;}

    //!
    //! \brief    Get value of member m_rwpkType
    //!
    //! \return   RwpkType
    //!           An instance of RwpkType class
    //!
    RwpkType GetRwpkType(){return m_rwpkType;}

private:
    EssentialPropertyElement& operator=(const EssentialPropertyElement& other) { return *this; };
    EssentialPropertyElement(const EssentialPropertyElement& other) { /* do not create copies */ };

private:

    ProjectionFormat    m_pf;              //!< the ProjectionFormat attribute
    OmafSrd             *m_srd;            //!< the OmafSrd attribute
    RwpkType            m_rwpkType;        //!< the RwpkType attribute
    string              m_projectionType;  //!< the omaf:packing_type attribute
    string              m_rwpkPackingType; //!< the rwpkPackingType attribute
};

VCD_OMAF_END;

#endif //ESSENTIALPROPERTYELEMENT_H
