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
//! \file:   EditAtom.h
//! \brief:  Edit List Atom class
//! \detail: 'elst' Atom implementation
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef EDITATOM_H
#define EDITATOM_H

#include "Atom.h"
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class EditListAtom : public FullAtom
{
public:
    struct EntryVersion0    //!< Edit List Entry Format version 0
    {
        std::uint32_t m_segDuration;
        std::int32_t m_mediaTime;
        std::uint16_t m_mediaRateInt;
        std::uint16_t m_mediaRateFraction;
    };

    struct EntryVersion1    //!< Edit List Entry Format version 1
    {
        std::uint64_t m_segDuration;
        std::int64_t m_mediaTime;
        std::uint16_t m_mediaRateInt;
        std::uint16_t m_mediaRateFraction;
    };

    //!
    //! \brief Constructor
    //!
    EditListAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~EditListAtom() = default;

    //!
    //! \brief    Add Entry
    //!
    //! \param    [in] const EntryVersion0&
    //!           entry
    //!
    //! \return   void
    //!
    void AddEntry(const EntryVersion0& entry);

    //!
    //! \brief    Add Entry
    //!
    //! \param    [in] const EntryVersion1&
    //!           entry
    //!
    //! \return   void
    //!
    void AddEntry(const EntryVersion1& entry);

    //!
    //! \brief    Get number of entries
    //!
    //! \return   std::uint32_t
    //!           number of entries
    //!
    std::uint32_t numEntry() const;

    //!
    //! \brief    Get Entry
    //!
    //! \param    [in] const std::uint32_t
    //!           index
    //!
    //! \return   const T&
    //!           entry
    //!
    template <typename T>
    const T& GetEntry(const std::uint32_t index) const;

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

private:
    std::vector<EntryVersion0> m_entryVersion0;  //!< vector of edit list entries of type verison 0
    std::vector<EntryVersion1> m_entryVersion1;  //!< vector of edit list entries of type verison 1
};

template <typename T>
const T& EditListAtom::GetEntry(const std::uint32_t index) const
{
    if (std::is_same<T, EditListAtom::EntryVersion0>::value)
    {
        return (const T&) m_entryVersion0.at(index);
    }

    if (std::is_same<T, EditListAtom::EntryVersion1>::value)
    {
        return (const T&) m_entryVersion1.at(index);
    }
}

class EditAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    EditAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~EditAtom() = default;

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
    //! \brief    Set Edit List Atom
    //!
    //! \param    [in] std::shared_ptr<EditListAtom>
    //!           edit List Atom
    //!
    //! \return   void
    //!
    void SetEditListAtom(std::shared_ptr<EditListAtom> editListAtom);

    //!
    //! \brief    Get Edit List Atom
    //!
    //! \return   const EditListAtom*
    //!           EditListAtom
    //!
    const EditListAtom* GetEditListAtom() const;

private:
    std::shared_ptr<EditListAtom> m_editListAtom;  //!< Edit List Atom pointer
};

VCD_MP4_END;
#endif /* EDITATOM_H */
