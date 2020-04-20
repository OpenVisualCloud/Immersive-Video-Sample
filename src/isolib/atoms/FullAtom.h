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
//! \file:   FullAtom.h
//! \brief:  FullAtom class
//! \detail: Full basic Atom difinition.
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef FULLATOM_H
#define FULLATOM_H

#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"

#include <cstdint>

VCD_MP4_BEGIN

class FullAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    FullAtom(FourCCInt AtomType, std::uint8_t version, std::uint32_t flags = 0);

    //!
    //! \brief Destructor
    //!
    virtual ~FullAtom() = default;

    //!
    //! \brief    Set version
    //!
    //! \param    [in] std::uint8_t
    //!           version value
    //!
    //! \return   void
    //!
    void SetVersion(std::uint8_t version);

    //!
    //! \brief    Get version
    //!
    //! \return   std::uint8_t
    //!           version
    //!
    std::uint8_t GetVersion() const;

    //!
    //! \brief    Set Flags
    //!
    //! \param    [in] std::uint32_t
    //!           Flags value
    //!
    //! \return   void
    //!
    void SetFlags(std::uint32_t flags);

    //!
    //! \brief    Get Flags
    //!
    //! \return   std::uint32_t
    //!           Flags
    //!
    std::uint32_t GetFlags() const;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str) = 0;

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str) = 0;

    //!
    //! \brief    Parse Full atom header information
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void ParseFullAtomHeader(Stream& str);

protected:

    //!
    //! \brief    Write Full atom header information
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void WriteFullAtomHeader(Stream& str);

private:
    std::uint8_t m_version;  //!< version field of the full Atom header
    std::uint32_t m_flags;   //!< Flags field of the full Atom header.
};

VCD_MP4_END;
#endif /* FULLATOM_H */
