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
//! \file:   ChannelLayoutAtom.h
//! \brief:  Channel Layout Atom class
//! \detail: 'chnl' Atom implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _CHANNELLAYOUTATOM_H_
#define _CHANNELLAYOUTATOM_H_

#include <vector>
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

/**
 * @brief Channel Layout Atom class
 * @details 'chnl' Atom implementation as specified in the ISOBMFF specification.
 */
class ChannelLayoutAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ChannelLayoutAtom();
    ChannelLayoutAtom(const ChannelLayoutAtom& Atom);

    ChannelLayoutAtom& operator=(const ChannelLayoutAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~ChannelLayoutAtom() = default;

    /// A helper for Getting and setting class data channel layout
    struct ChannelLayout
    {
        std::uint8_t speakerPosition = 127;  // undefined / unknown
        std::int16_t azimuthAngle    = 0;
        std::int8_t  elevationAngle  = 0;
    };

    //!
    //! \brief    Get ChannelLayouts
    //!
    //! \return   std::vector<ChannelLayout>
    //!           ChannelLayout array
    //!
    std::vector<ChannelLayout> GetChannelLayouts() const;

    //!
    //! \brief    Add ChannelLayout
    //!
    //! \param    [in] ChannelLayout&
    //!           ChannelLayout value
    //!
    //! \return   void
    //!
    void AddChannelLayout(ChannelLayout& channelLayout);

    //!
    //! \brief    Get Stream Structure
    //!
    //! \return   std::uint8_t
    //!           Stream Structure
    //!
    std::uint8_t GetStreamStructure() const;

    //!
    //! \brief    Get Defined Layout
    //!
    //! \return   std::uint8_t
    //!           Defined Layout
    //!
    std::uint8_t GetDefinedLayout() const;

    //!
    //! \brief    Set Defined Layout
    //!
    //! \param    [in] std::uint8_t
    //!           Defined Layout value
    //!
    //! \return   void
    //!
    void SetDefinedLayout(std::uint8_t definedLayout);

    //!
    //! \brief    Get Defined Layout
    //!
    //! \return   std::uint8_t
    //!           Defined Layout
    //!
    std::uint64_t GetOmittedChannelsMap() const;

    //!
    //! \brief    Set Omitted Channels Map
    //!
    //! \param    [in] std::uint64_t
    //!           omitted Channels Map
    //!
    //! \return   void
    //!
    void SetOmittedChannelsMap(std::uint64_t omittedChannelsMap);

    //!
    //! \brief    Get Object Count
    //!
    //! \return   std::uint8_t
    //!           Object Count
    //!
    std::uint8_t GetObjectCount() const;

    //!
    //! \brief    Set Object Count
    //!
    //! \param    [in] std::uint8_t
    //!           Object Count
    //!
    //! \return   void
    //!
    void SetObjectCount(std::uint8_t objectCount);

    //!
    //! \brief    Set and Get function for m_channelNumber member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_channelNumber
    //!           m_channelNumber member in class
    //! \param    [in] ChannelNumber
    //!           m_channelNumber name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint16_t, m_channelNumber, ChannelNumber, const);

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
    std::uint8_t m_strStructured;                                   //!< string structured
    std::uint8_t m_definedLayout;                                   //!< defined Layout
    std::uint64_t m_omittedChannelsMap;                             //!< omitted Channels Map
    std::uint8_t m_objectCount;                                     //!< object Count
    std::uint16_t m_channelNumber;                                  //!< channel Number
    std::vector<ChannelLayoutAtom::ChannelLayout> m_channelLayouts; //!< channel Layouts
};

VCD_MP4_END;
#endif /* _CHANNELLAYOUTATOM_H_ */
