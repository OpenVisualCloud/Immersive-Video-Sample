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
//! \file:   TypeAtom.h
//! \brief:  TypeAtom definition
//! \detail: Contains brandAtom, FileTypeAtom, TrackTypeAtom, SegmentTypeAtom
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef _TYPEATOM_H_
#define _TYPEATOM_H_

#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

#include <string>

VCD_MP4_BEGIN

template <class T>
class BrandAtom : public T
{
public:

    //!
    //! \brief Constructor
    //!
    BrandAtom(FourCCInt AtomType);
    BrandAtom(FourCCInt AtomType, std::uint8_t version, std::uint32_t flags = 0);

    //!
    //! \brief Destructor
    //!
    virtual ~BrandAtom() = default;

    //!
    //! \brief    Set MajorBrand
    //!
    //! \param    [in] const std::string&
    //!           MajorBrand value
    //!
    //! \return   void
    //!
    void SetMajorBrand(const std::string& value);

    //!
    //! \brief    Get MajorBrand
    //!
    //! \return   const std::string&
    //!           MajorBrand
    //!
    const std::string& GetMajorBrand() const;

    //!
    //! \brief    Set MinorVersion
    //!
    //! \param    [in] std::uint32_t
    //!           MinorVersion value
    //!
    //! \return   void
    //!
    void SetMinorVersion(std::uint32_t version);

    //!
    //! \brief    Get MinorVersion
    //!
    //! \return   uint32_t
    //!           MinorVersion
    //!
    uint32_t GetMinorVersion() const;

    //!
    //! \brief    Add Compatible Brand
    //!
    //! \param    [in] const std::string&
    //!           Compatible Brand value
    //!
    //! \return   void
    //!
    void AddCompatibleBrand(const std::string& value);

    //!
    //! \brief    Get CompatibleBrands
    //!
    //! \return   std::vector<std::string>
    //!           CompatibleBrands
    //!
    std::vector<std::string> GetCompatibleBrands() const;

    //!
    //! \brief    Check CompatibleBrand existed
    //!
    //! \param    [in] const std::string&
    //!           value
    //!
    //! \return   bool
    //!           check result
    //!
    bool CheckCompatibleBrand(const std::string& value) const;

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

    //!
    //! \brief    Write Header information
    //!
    //! \param    [in] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void WriteHeader(Stream& str);

    //!
    //! \brief    Parse Header information
    //!
    //! \param    [in] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ParseHeader(Stream& str);

    std::string m_majorBrand;                     //!< Major Brand as string value
    std::uint32_t m_minorVersion;                 //!< Minor version as an unsigned integer
    std::vector<std::string> m_compatibleBrands;  //!< std::vector containing the Compatible Brands as strings
};


class FileTypeAtom : public BrandAtom<Atom>
{
public:

    //!
    //! \brief  Constructor
    //!
    FileTypeAtom();

    //!
    //! \brief  Destructor
    //!
    virtual ~FileTypeAtom() = default;

};

class TrackTypeAtom : public BrandAtom<FullAtom>
{
public:

    //!
    //! \brief  Constructor
    //!
    TrackTypeAtom();

    //!
    //! \brief  Constructor
    //!
    virtual ~TrackTypeAtom() = default;
};

class SegmentTypeAtom : public BrandAtom<Atom>
{
public:

    //!
    //! \brief  Constructor
    //!
    SegmentTypeAtom();

    //!
    //! \brief  Constructor
    //!
    virtual ~SegmentTypeAtom() = default;
};

VCD_MP4_END;
#endif /* _TYPEATOM_H_ */
