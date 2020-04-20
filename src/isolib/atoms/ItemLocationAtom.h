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
//! \file:   ItemLocationAtom.h
//! \brief:  Item Location atom definition.
//! \detail: Contains location item data structure
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _ITEMLOCATIONATOM_H_
#define _ITEMLOCATIONATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

#include <iterator>

VCD_MP4_BEGIN

struct ExtentParams //!< extent parameters
{
    std::uint64_t m_extentIndex  = 0;
    std::uint64_t m_extentOffset = 0;
    std::uint64_t m_extentLen = 0;
};

typedef std::vector<ExtentParams> ExtentList;  //!< std::vector of item location extents

class ItemLocation
{
public:

    //!
    //! \brief Constructor
    //!
    ItemLocation();

    //!
    //! \brief Destructor
    //!
    ~ItemLocation() = default;

    enum class ConstructType    //!< construction type
    {
        FILE_OFFSET = 0,
        IDAT_OFFSET = 1,
        ITEM_OFFSET = 2
    };

    //!
    //! \brief    Set and Get function for m_itemID member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_itemID
    //!           m_itemID member in class
    //! \param    [in] ItemID
    //!           m_itemID name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_itemID, ItemID, const);

    //!
    //! \brief    Set and Get function for m_constructionMethod member
    //!
    //! \param    [in] ConstructType
    //!           value to set
    //! \param    [in] m_constructionMethod
    //!           m_constructionMethod member in class
    //! \param    [in] ConstructType
    //!           m_constructionMethod name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(ConstructType, m_constructionMethod, ConstructType, const);

    //!
    //! \brief    Set and Get function for m_dataReferenceIndex member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_dataReferenceIndex
    //!           m_dataReferenceIndex member in class
    //! \param    [in] DataReferenceIndex
    //!           m_dataReferenceIndex name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint16_t, m_dataReferenceIndex, DataReferenceIndex, const);

    //!
    //! \brief    Set and Get function for m_baseOffset member
    //!
    //! \param    [in] std::uint64_t
    //!           value to set
    //! \param    [in] m_baseOffset
    //!           m_baseOffset member in class
    //! \param    [in] BaseOffset
    //!           m_baseOffset name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint64_t, m_baseOffset, BaseOffset, const);

    //!
    //! \brief    Get Extent Count
    //!
    //! \return   std::uint16_t
    //!           Extent Count
    //!
    std::uint16_t GetExtentCount() const;

    //!
    //! \brief    Add Extent
    //!
    //! \param    [in] const ExtentParams&
    //!           extent
    //!
    //! \return   void
    //!
    void AddExtent(const ExtentParams& extent);

    //!
    //! \brief    Get Extent List
    //!
    //! \return   const ExtentList&
    //!           Extent List
    //!
    const ExtentList& GetExtentList() const;

    //!
    //! \brief    Get Extent Params
    //!
    //! \param    [in] unsigned int
    //!           index
    //!
    //! \return   const ExtentParams&
    //!           Extent Params
    //!
    const ExtentParams& GetExtent(unsigned int i) const;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!           [in] unsigned int
    //!           version
    //!
    //! \return   void
    //!
    void ToStream(Stream& str, unsigned int version);

private:
    std::uint32_t m_itemID;                   //!< Item ID
    ConstructType m_constructionMethod;       //!< Construction method enumeration
    std::uint16_t m_dataReferenceIndex;       //!< Data reference index
    std::uint64_t m_baseOffset;               //!< Base offset value
    ExtentList m_extentList;                  //!< List of extents
};

typedef std::vector<ItemLocation> ItemLocationVector;  //!< std::vector of Item Locations

class ItemLocationAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ItemLocationAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~ItemLocationAtom() = default;

    //!
    //! \brief    Set and Get function for m_offSetSize member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_offSetSize
    //!           m_offSetSize member in class
    //! \param    [in] OffSetSize
    //!           m_offSetSize name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_offSetSize, OffSetSize, const);

    //!
    //! \brief    Set and Get function for m_lengthSize member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_lengthSize
    //!           m_lengthSize member in class
    //! \param    [in] LengthSize
    //!           m_lengthSize name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_lengthSize, LengthSize, const);

    //!
    //! \brief    Set and Get function for m_baseOffSetSize member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_baseOffSetSize
    //!           m_baseOffSetSize member in class
    //! \param    [in] BaseOffSetSize
    //!           m_baseOffSetSize name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_baseOffSetSize, BaseOffSetSize, const);

    //!
    //! \brief    Set and Get function for m_indexSize member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_indexSize
    //!           m_indexSize member in class
    //! \param    [in] IndexSize
    //!           m_indexSize name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_indexSize, IndexSize, const);

    //!
    //! \brief    Get Item Count
    //!
    //! \return   std::uint32_t
    //!           Item Count
    //!
    std::uint32_t GetItemCount() const;

    //!
    //! \brief    Add Location
    //!
    //! \param    [in] const ItemLocation&
    //!           item Location
    //!
    //! \return   void
    //!
    void AddLocation(const ItemLocation& itemLoc);

    //!
    //! \brief    Add Extent
    //!
    //! \param    [in] std::uint32_t
    //!           item Id
    //! \param    [in] const ExtentParams&
    //!           extent
    //!
    //! \return   void
    //!
    void AddExtent(std::uint32_t itemId, const ExtentParams& extent);

    //!
    //! \brief    Has Item Id Entry
    //!
    //! \param    [in] std::uint32_t
    //!           item id
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasItemIdEntry(std::uint32_t itemId) const;

    //!
    //! \brief    Set Item Data Reference Index
    //!
    //! \param    [in] std::uint32_t
    //!           item Id
    //! \param    [in] std::uint16_t
    //!           dataReferenceIndex
    //!
    //! \return   bool
    //!           return successful or not
    //!
    bool SetItemDataReferenceIndex(std::uint32_t itemId, std::uint16_t dataReferenceIndex);

    //!
    //! \brief    Get Item Locations
    //!
    //! \return   ItemLocationVector&
    //!           ItemLocationVector
    //!
    ItemLocationVector& GetItemLocations();

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
    //! \brief    Get Item Location For ID
    //!
    //! \param    [in] unsigned int
    //!           item id
    //!
    //! \return   const ItemLocation&
    //!           ItemLocation
    //!
    const ItemLocation& GetItemLocationForID(unsigned int itemID) const;

private:
    std::uint8_t m_offSetSize;             //!< Offset size {0,4, or 8}
    std::uint8_t m_lengthSize;             //!< Length size {0,4, or 8}
    std::uint8_t m_baseOffSetSize;         //!< Base offset size {0,4, or 8}
    std::uint8_t m_indexSize;              //!< Index size {0,4, or 8} and only if version == 1, otherwise reserved
    ItemLocationVector m_itemLocations;    //!< std::vector of item location entries
    ItemLocationVector::const_iterator
    findItem(std::uint32_t itemId) const;  //!< Find an item with given itemId and return as a const
    ItemLocationVector::iterator findItem(std::uint32_t itemId);  //!< Find an item with given itemId and return
};

VCD_MP4_END;
#endif /* _ITEMLOCATIONATOM_H_ */
