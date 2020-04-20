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
//! \file:   MediaDataAtom.h
//! \brief:  Media Data Atom class.
//! \detail: 'mdat' Atom contains media data
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _MEDIADATAATOM_H_
#define _MEDIADATAATOM_H_

#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"

VCD_MP4_BEGIN

#include <cstdint>

class MediaDataAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    MediaDataAtom();

    //!
    //! \brief Destructor
    //!
    ~MediaDataAtom() = default;

    //!
    //! \brief    Write to ofstream
    //!
    //! \param    [out] std::ofstream&
    //!           output
    //!
    //! \return   void
    //!
    void Write(std::ofstream& output) const;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void FromStream(Stream& str);

    //!
    //! \brief    Update total atom size.
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void UpdateSize(Stream& str);

    //!
    //! \brief    Add media data to the atom
    //!
    //! \param    [in] const std::vector<std::uint8_t>&
    //!           data to be added
    //!
    //! \return   std::uint64_t
    //!           offset of the data
    //!
    std::uint64_t AddData(const std::vector<std::uint8_t>& srcData);

    //!
    //! \brief    Add media data to the atom
    //!
    //! \param    [in] const uint8_t*
    //!           buffer
    //! \param    [in] const std::vector<std::uint8_t>&
    //!           data to be added
    //!
    //! \return   std::uint64_t
    //!           offset of the data
    //!
    std::uint64_t AddData(const uint8_t* buffer, const uint64_t bufferSize);

    //!
    //! \brief    Add media nal data to the atom
    //!
    //! \param    [in] const std::vector<std::vector<std::uint8_t>>&
    //!           source data to be added
    //!
    //! \return   void
    //!
    void AddNalData(const std::vector<std::vector<std::uint8_t>>& srcData);

    //!
    //! \brief    Add media nal data to the atom
    //!
    //! \param    [in] const std::vector<std::uint8_t>&
    //!           source data to be added
    //!
    //! \return   void
    //!
    void AddNalData(const std::vector<std::uint8_t>& srcData);

private:
    Stream m_headerData;                          //!< header container
    std::list<std::vector<uint8_t>> m_mediaData;  //!< media data container
    uint64_t m_totalSize;                         //!< total size of m_mediaData vectors
    std::vector<std::uint64_t> m_dataOffsetList;  //!< offsets relative to the beginning of the media data Atom
    std::vector<std::uint64_t> m_dataLenList;     //!< vector of data lengths which are inserted to the media Atom

    //!
    //! \brief    Find Start Code
    //!
    //! \param    [in] const std::vector<std::uint8_t>&
    //!           source data
    //! \param    [in] std::uint64_t
    //!           initial Position
    //! \param    [in] std::uint64_t&
    //!           start Code Postion
    //!
    //! \return   std::uint64_t
    //!           offset of the data
    //!
    std::uint64_t FindStartCode(const std::vector<std::uint8_t>& srcData,
                                std::uint64_t initPos,
                                std::uint64_t& startCodePos);
};

VCD_MP4_END;
#endif /* _MEDIADATAATOM_H_ */
