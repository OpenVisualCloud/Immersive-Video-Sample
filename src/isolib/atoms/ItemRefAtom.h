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
//! \file:   ItemRefAtom.h
//! \brief:  Single Item Reference Atom class.
//! \detail: Definitions for Item Reference Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _ITEMREFERENCEATOM_H_
#define _ITEMREFERENCEATOM_H_

#include "Atom.h"
#include "FormAllocator.h"
#include "FullAtom.h"

#include <cstdint>
#include <list>

VCD_MP4_BEGIN

class SingleItemTypeReferenceAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    SingleItemTypeReferenceAtom(bool isLarge = false);

    //!
    //! \brief Destructor
    //!
    virtual ~SingleItemTypeReferenceAtom() = default;

    //!
    //! \brief    Set Reference Type
    //!
    //! \param    [in] FourCCInt
    //!           Reference Type value
    //!
    //! \return   void
    //!
    void SetReferenceType(FourCCInt referenceType);

    //!
    //! \brief    Set and Get function for m_fromItemId member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_fromItemId
    //!           m_fromItemId member in class
    //! \param    [in] FromItemID
    //!           m_fromItemId name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_fromItemId, FromItemID, const);

    //!
    //! \brief    Add To Item ID
    //!
    //! \param    [in] std::uint32_t
    //!           item ID value
    //!
    //! \return   void
    //!
    void AddToItemID(std::uint32_t itemID);

    //!
    //! \brief    Get version
    //!
    //! \return   std::vector<std::uint32_t>
    //!           Item array
    //!
    std::vector<std::uint32_t> GetToItemIds() const;

    //!
    //! \brief    Clear To Item IDs
    //!
    //! \return   void
    //!
    void ClearToItemIDs();

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
    std::uint32_t m_fromItemId;              //!< item Id value
    std::vector<std::uint32_t> m_toItemIds;  //!< std::vector of item Id values
    bool m_isLarge;                          //!< True if this is a SingleItemTypeReferenceAtomLarge
};

class ItemReferenceAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ItemReferenceAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~ItemReferenceAtom() = default;

    //!
    //! \brief    Add Items
    //!
    //! \param    [in] FourCCInt
    //!           type value
    //! \param    [in] std::uint32_t
    //!           fromId value
    //! \param    [in] std::uint32_t
    //!           toId value
    //!
    //! \return   void
    //!
    void AddItems(FourCCInt type, std::uint32_t fromId, std::uint32_t toId);

    //!
    //! \brief    Get Reference Of Type
    //!
    //! \param    [in] FourCCInt
    //!           type value
    //!
    //! \return   std::vector<SingleItemTypeReferenceAtom>
    //!           SingleItemTypeReferenceAtom array
    //!
    std::vector<SingleItemTypeReferenceAtom> GetRefOfType(FourCCInt type) const;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str);

private:
    void AddItemRef(const SingleItemTypeReferenceAtom& ref);  //!< Add an item reference to the ItemReferenceAtom
    std::list<SingleItemTypeReferenceAtom> m_refList;         //!< reference atom list
};

VCD_MP4_END;
#endif /* _ITEMREFERENCEATOM_H_ */
