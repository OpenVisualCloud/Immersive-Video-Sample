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
//! \file:   ItemProtAtom.h
//! \brief:  Item Protection Atom class
//! \detail: 'ipro' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _ITEMPROTECTIONATOM_H_
#define _ITEMPROTECTIONATOM_H_

#include "FormAllocator.h"
#include "FullAtom.h"
#include <cstdint>

VCD_MP4_BEGIN

class Stream;

class ProtectionSchemeInfoAtom
{
public:

    //!
    //! \brief    Get data
    //!
    //! \return   std::vector<std::uint8_t>
    //!           data
    //!
    std::vector<std::uint8_t> GetData() const;

    //!
    //! \brief    Set data
    //!
    //! \param    [in] const std::vector<std::uint8_t>&
    //!           data value
    //!
    //! \return   void
    //!
    void SetData(const std::vector<std::uint8_t>& data);

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void ToStream(Stream& stream);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void FromStream(Stream& stream);

private:
    std::vector<std::uint8_t> m_data;  //!< Content of this Atom
};

class ItemProtectionAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ItemProtectionAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~ItemProtectionAtom() = default;

    //!
    //! \brief    Get size
    //!
    //! \return   std::uint16_t
    //!           size
    //!
    std::uint16_t GetSize() const;

    //!
    //! \brief    Get entry
    //!
    //! \param    [in] std::uint16_t
    //!           index value
    //!
    //! \return   const ProtectionSchemeInfoAtom&
    //!           ProtectionSchemeInfoAtom
    //!
    const ProtectionSchemeInfoAtom& GetEntry(std::uint16_t index) const;

    //!
    //! \brief    Aadd entry
    //!
    //! \param    [in] const ProtectionSchemeInfoAtom&
    //!           ProtectionSchemeInfoAtom
    //!
    //! \return   std::uint16_t
    //!           index
    //!
    std::uint16_t AddEntry(const ProtectionSchemeInfoAtom& sinf);

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& stream);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& stream);

private:
    std::vector<ProtectionSchemeInfoAtom> m_protectionInformation;  //!< 'sinf' Atoms
};

VCD_MP4_END;
#endif /* _ITEMPROTECTIONATOM_H_ */
