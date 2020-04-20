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
//! \file:   UriMetaSampEntryAtom.h
//! \brief:  UriMetaSampEntryAtom class.
//! \detail: Uri Meta Sample Entry Atom
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _URIMETASAMPLEENTRY_H_
#define _URIMETASAMPLEENTRY_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "MetaDataSampEntryAtom.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class UriAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    UriAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~UriAtom() = default;

    //!
    //! \brief    Set Uri
    //!
    //! \param    [in] const std::string&
    //!           Uri value
    //!
    //! \return   void
    //!
    void SetUri(const std::string& uri);

    //!
    //! \brief    Get Uri
    //!
    //! \return   const std::string&
    //!           Uri
    //!
    const std::string& GetUri() const;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str);

private:
    std::string m_uri;  //!< the URI
};

class UriInitAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    UriInitAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~UriInitAtom() = default;

    enum class InitAtomMode //!< Init Atom Mode
    {
        UNKNOWN = 0
    };

    //!
    //! \brief    Get Init Atom Mode
    //!
    //! \return   InitAtomMode
    //!           Init Atom Mode
    //!
    InitAtomMode GetInitAtomMode() const;

    //!
    //! \brief    Set Init Atom Mode
    //!
    //! \param    [in] InitAtomMode
    //!           Init Atom Mode value
    //!
    //! \return   void
    //!
    void SetInitAtomMode(InitAtomMode dataType);

    //!
    //! \brief    Set Uri Initialization Data
    //!
    //! \param    [in] const std::vector<std::uint8_t>&
    //!           uri Init Data
    //!
    //! \return   void
    //!
    void SetUriInitializationData(const std::vector<std::uint8_t>& uriInitData);

    //!
    //! \brief    Get Uri Initialization Data
    //!
    //! \return   std::vector<std::uint8_t>
    //!           Uri Initialization Data
    //!
    std::vector<std::uint8_t> GetUriInitializationData() const;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str);

private:
    InitAtomMode m_initAtomType;                        //!< Init Atom Mode
    std::vector<std::uint8_t> m_uriInitializationData;  //!< uri Initialization Data
};

class UriMetaSampleEntryAtom : public MetaDataSampleEntryAtom
{
public:

    //!
    //! \brief Constructor
    //!
    UriMetaSampleEntryAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~UriMetaSampleEntryAtom() = default;

    enum class VRTMDType    //!< VRTMD Type
    {
        UNKNOWN = 0
    };

    //!
    //! \brief    Get UriAtom
    //!
    //! \return   UriAtom&
    //!           UriAtom
    //!
    UriAtom& GetUriAtom();

    //!
    //! \brief    has Uri init Atom
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasUriInitAtom();

    //!
    //! \brief    get Uri init Atom
    //!
    //! \return   UriInitAtom&
    //!           Uri Init Atom
    //!
    UriInitAtom& GetUriInitAtom();

    //!
    //! \brief    get VRTMD Type
    //!
    //! \return   VRTMDType
    //!           VRTMD Type
    //!
    VRTMDType GetVRTMDType() const;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str);

    //!
    //! \brief    Get Copy of UriMetaSampleEntryAtom
    //!
    //! \return   UriMetaSampleEntryAtom*
    //!           UriMetaSampleEntry Atom
    //!
    virtual UriMetaSampleEntryAtom* Clone() const;

    //!
    //! \brief    Get ConfigurationRecord
    //!
    //! \return   const DecoderConfigurationRecord*
    //!           DecoderConfigurationRecord value
    //!
    virtual const DecoderConfigurationRecord* GetConfigurationRecord() const override;

    //!
    //! \brief    Get Configuration Atom
    //!
    //! \return   const Atom*
    //!           Configuration Atom
    //!
    virtual const Atom* GetConfigurationAtom() const override;

private:
    UriAtom m_uriAtom;          //!< Uri Atom
    VRTMDType m_vRMetaDataType; //!< VRTMD Type
    bool m_hasUriInitAtom;      //!< has Uri Init Atom
    UriInitAtom m_uriInitAtom;  //!< Uri Init Atom
};

VCD_MP4_END;
#endif /* _URIMETASAMPLEENTRY_H_ */
