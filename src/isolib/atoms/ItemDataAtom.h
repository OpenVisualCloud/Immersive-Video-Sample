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
//! \file:   ItemDataAtom.h
//! \brief:  The ItemDataAtom 'idat' Atom
//! \detail: Contains atom data
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _ITEMDATAATOM_H_
#define _ITEMDATAATOM_H_

#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"

VCD_MP4_BEGIN

class ItemDataAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    ItemDataAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~ItemDataAtom() = default;

    //!
    //! \brief    Read data from atom
    //!
    //! \param    [in] std::vector<std::uint8_t>&
    //!           destination
    //! \param    [in] std::uint64_t
    //!           offset
    //! \param    [in] std::uint64_t
    //!           length
    //!
    //! \return   bool
    //!           return data is copied successfully or not.
    //!
    bool Read(std::vector<std::uint8_t>& destination, std::uint64_t offset, std::uint64_t length) const;

    //!
    //! \brief    Read data from atom
    //!
    //! \param    [in] uint8_t*
    //!           destination
    //! \param    [in] const std::uint64_t
    //!           offset
    //! \param    [in] const std::uint64_t
    //!           length
    //!
    //! \return   bool
    //!           return data is copied successfully or not.
    //!
    bool Read(uint8_t* destination, const std::uint64_t offset, const std::uint64_t length) const;

    //!
    //! \brief    Add item data to the atom
    //!
    //! \param    [in] const std::vector<std::uint8_t>&
    //!           data to be added
    //!
    //! \return   std::uint64_t
    //!           offset of the data
    //!
    std::uint64_t AddData(const std::vector<std::uint8_t>& data);

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
    std::vector<std::uint8_t> m_data;  //!< Data of stored items.
};

VCD_MP4_END;
#endif /* _ITEMDATAATOM_H_ */