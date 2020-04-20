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
//! \file:   BasicAudAtom.h
//! \brief:  Basic Audio Atom class.
//! \detail: Definitions for audio atoms.
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _SPATIALAUDIOATOM_H_
#define _SPATIALAUDIOATOM_H_

#include <stdint.h>
#include <vector>
#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"

VCD_MP4_BEGIN

class SpatialAudioAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    SpatialAudioAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~SpatialAudioAtom() = default;

    //!
    //! \brief    Set and Get function for m_version member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_version
    //!           m_version member in class
    //! \param    [in] Version
    //!           m_version name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_version, Version, const);

    //!
    //! \brief    Set and Get function for m_type member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_type
    //!           m_type member in class
    //! \param    [in] AmbisonicType
    //!           m_type name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_type, AmbisonicType, const);

    //!
    //! \brief    Set and Get function for m_order member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_order
    //!           m_order member in class
    //! \param    [in] AmbisonicOrder
    //!           m_order name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_order, AmbisonicOrder, const);

    //!
    //! \brief    Set and Get function for m_channelOrder member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_channelOrder
    //!           m_channelOrder member in class
    //! \param    [in] AmbisonicChannelOrdering
    //!           m_channelOrder name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_channelOrder, AmbisonicChannelOrdering, const);

    //!
    //! \brief    Set and Get function for m_norm member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_norm
    //!           m_norm member in class
    //! \param    [in] AmbisonicNormalization
    //!           m_norm name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_norm, AmbisonicNormalization, const);

    //!
    //! \brief    Set Channel Map
    //!
    //! \param    [in] const std::vector<uint32_t>&
    //!           channel Map value
    //!
    //! \return   void
    //!
    void SetChannelMap(const std::vector<uint32_t>& channelMap);

    //!
    //! \brief    Get Channel Map
    //!
    //! \return   std::vector<uint32_t>
    //!           Channel Map
    //!
    std::vector<uint32_t> GetChannelMap() const;

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
    uint8_t m_version;                  //!< version
    uint8_t m_type;                     //!< type
    uint32_t m_order;                   //!< order
    uint8_t m_channelOrder;             //!< channel order
    uint8_t m_norm;                     //!< norm
    std::vector<uint32_t> m_channelMap; //!< size = number of channels
};

class NonDiegeticAudioAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    NonDiegeticAudioAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~NonDiegeticAudioAtom() = default;

    //!
    //! \brief    Set and Get function for m_version member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_version
    //!           m_version member in class
    //! \param    [in] Version
    //!           m_version name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_version, Version, const);

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
    uint8_t m_version;  //!< version
};

VCD_MP4_END;
#endif /* _SPATIALAUDIOATOM_H_ */
