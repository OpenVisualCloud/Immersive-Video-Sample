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
//! \file:   ItemInfoAtom.h
//! \brief:  Item Information Atom class.
//! \detail: 'iinf' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _ITEMINFOATOM_H_
#define _ITEMINFOATOM_H_

#include <cstdint>
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class ItemInfoEntry;
class ItemInfoExtension;

class ItemInfoAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ItemInfoAtom();
    ItemInfoAtom(uint8_t version);

    //!
    //! \brief Destructor
    //!
    virtual ~ItemInfoAtom() = default;

    //!
    //! \brief    Clear contents
    //!
    //! \return   void
    //!
    void Clear();

    //!
    //! \brief    Get Entry Count
    //!
    //! \return   std::uint32_t
    //!           number of entries
    //!
    std::uint32_t GetEntryCount() const;

    //!
    //! \brief    Get Item Ids
    //!
    //! \return   std::vector<std::uint32_t>
    //!           All Item IDs in this ItemInfoBox
    //!
    std::vector<std::uint32_t> GetItemIds() const;

    //!
    //! \brief    Add Item Info Entry
    //!
    //! \param    [in] const ItemInfoEntry&
    //!           infoEntry
    //!
    //! \return   void
    //!
    void AddItemInfoEntry(const ItemInfoEntry& infoEntry);

    //!
    //! \brief    Get Item Info Entry
    //!
    //! \param    [in] const std::uint32_t
    //!           index
    //!
    //! \return   const ItemInfoEntry&
    //!           ItemInformationEntry
    //!
    const ItemInfoEntry& GetItemInfoEntry(const std::uint32_t idx) const;

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

    //!
    //! \brief    Get Items Number
    //!
    //! \param    [in] FourCCInt
    //!           item Type
    //! \param    [in] unsigned int
    //!           index
    //!
    //! \return   ItemInfoEntry*
    //!           ItemInfoEntry
    //!
    ItemInfoEntry* GetItemsNumber(FourCCInt itemType, unsigned int index = 0);

    //!
    //! \brief    Find Item With Type And ID
    //!
    //! \param    [in] FourCCInt
    //!           item Type
    //! \param    [in] unsigned int
    //!           ID
    //! \param    [in] unsigned int&
    //!           index
    //!
    //! \return   ItemInfoEntry*
    //!           ItemInfoEntry
    //!
    ItemInfoEntry* FindItemWithTypeAndID(FourCCInt itemType, unsigned int itemID, unsigned int& index);

    //!
    //! \brief    Get the number of items
    //!
    //! \param    [in] FourCCInt
    //!           item Type
    //!
    //! \return   unsigned int
    //!           number of items
    //!
    unsigned int CountNumberOfItems(FourCCInt itemType);

    //!
    //! \brief    Get Items By Type
    //!
    //! \param    [in] FourCCInt
    //!           item Type
    //!
    //! \return   std::vector<ItemInfoEntry>
    //!           vector of items
    //!
    std::vector<ItemInfoEntry> GetItemsByType(FourCCInt itemType) const;

    //!
    //! \brief    Get Items By Id
    //!
    //! \param    [in] uint32_t
    //!           item id
    //!
    //! \return   ItemInfoEntry
    //!           ItemInfoEntry
    //!
    ItemInfoEntry GetItemById(uint32_t itemId) const;

private:
    std::vector<ItemInfoEntry> m_itemInfoList;  //!< std::vector of the ItemInfoEntry Atoms
};

class ItemInfoEntry : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ItemInfoEntry();

    //!
    //! \brief Destructor
    //!
    virtual ~ItemInfoEntry();

    ItemInfoEntry(const ItemInfoEntry& itemInfoEntry) = default;
    ItemInfoEntry& operator=(const ItemInfoEntry&) = default;

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
    //! \brief    Set and Get function for m_itemProtectionIndex member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_itemProtectionIndex
    //!           m_itemProtectionIndex member in class
    //! \param    [in] ItemProtectionIndex
    //!           m_itemProtectionIndex name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint16_t, m_itemProtectionIndex, ItemProtectionIndex, const);

    //!
    //! \brief    Set and Get function for m_itemName member
    //!
    //! \param    [in] const std::string&
    //!           value to set
    //! \param    [in] m_itemName
    //!           m_itemName member in class
    //! \param    [in] ItemName
    //!           m_itemName name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::string&, m_itemName, ItemName, const);

    //!
    //! \brief    Set and Get function for m_contentType member
    //!
    //! \param    [in] const std::string&
    //!           value to set
    //! \param    [in] m_contentType
    //!           m_contentType member in class
    //! \param    [in] ContentType
    //!           m_contentType name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::string&, m_contentType, ContentType, const);

    //!
    //! \brief    Set and Get function for m_contentEncoding member
    //!
    //! \param    [in] const std::string&
    //!           value to set
    //! \param    [in] m_contentEncoding
    //!           m_contentEncoding member in class
    //! \param    [in] ContentEncoding
    //!           m_contentEncoding name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::string&, m_contentEncoding, ContentEncoding, const);

    //!
    //! \brief    Set and Get function for m_extensionType member
    //!
    //! \param    [in] const std::string&
    //!           value to set
    //! \param    [in] m_extensionType
    //!           m_extensionType member in class
    //! \param    [in] ExtensionType
    //!           m_extensionType name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::string&, m_extensionType, ExtensionType, const);

    //!
    //! \brief    Set and Get function for m_itemType member
    //!
    //! \param    [in] FourCCInt
    //!           value to set
    //! \param    [in] m_itemType
    //!           m_itemType member in class
    //! \param    [in] ItemType
    //!           m_itemType name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(FourCCInt, m_itemType, ItemType, const);

    //!
    //! \brief    Set and Get function for m_itemUriType member
    //!
    //! \param    [in] const std::string&
    //!           value to set
    //! \param    [in] m_itemUriType
    //!           m_itemUriType member in class
    //! \param    [in] ItemUriType
    //!           m_itemUriType name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::string&, m_itemUriType, ItemUriType, const);

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
    std::uint32_t m_itemID;                                  //!< ID of the item
    std::uint16_t m_itemProtectionIndex;                     //!< Item protection index
    std::string m_itemName;                                  //!< Item name
    std::string m_contentType;                               //!< Content type
    std::string m_contentEncoding;                           //!< Content encoding

    std::string m_extensionType;                             //!< The extension type
    std::shared_ptr<ItemInfoExtension> m_itemInfoExtension;  //!< Item info extension

    FourCCInt m_itemType;                                    //!< Item type
    std::string m_itemUriType;                               //!< Item UIR type
};

class ItemInfoExtension
{
public:

    //!
    //! \brief Constructor
    //!
    ItemInfoExtension()          = default;

    //!
    //! \brief Destructor
    //!
    virtual ~ItemInfoExtension() = default;

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
};

class FDItemInfoExtension : public ItemInfoExtension
{
public:

    //!
    //! \brief Constructor
    //!
    FDItemInfoExtension();

    //!
    //! \brief Destructor
    //!
    virtual ~FDItemInfoExtension() = default;

    //!
    //! \brief    Set and Get function for m_contentLocation member
    //!
    //! \param    [in] const std::string&
    //!           value to set
    //! \param    [in] m_contentLocation
    //!           m_contentLocation member in class
    //! \param    [in] ContentLocation
    //!           m_contentLocation name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::string&, m_contentLocation, ContentLocation, );

    //!
    //! \brief    Set and Get function for m_contentMD5 member
    //!
    //! \param    [in] const std::string&
    //!           value to set
    //! \param    [in] m_contentMD5
    //!           m_contentMD5 member in class
    //! \param    [in] ContentMD5
    //!           m_contentMD5 name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::string&, m_contentMD5, ContentMD5, );

    //!
    //! \brief    Set and Get function for m_contentLength member
    //!
    //! \param    [in] uint64_t
    //!           value to set
    //! \param    [in] m_contentLength
    //!           m_contentLength member in class
    //! \param    [in] ContentLength
    //!           m_contentLength name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint64_t, m_contentLength, ContentLength, );

    //!
    //! \brief    Set and Get function for m_transferLength member
    //!
    //! \param    [in] uint64_t
    //!           value to set
    //! \param    [in] m_transferLength
    //!           m_transferLength member in class
    //! \param    [in] TranferLength
    //!           m_transferLength name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint64_t, m_transferLength, TranferLength, );

    //!
    //! \brief    Set and Get function for m_entryCount member
    //!
    //! \param    [in] uint8_t
    //!           value to set
    //! \param    [in] m_entryCount
    //!           m_entryCount member in class
    //! \param    [in] NumGroupID
    //!           m_entryCount name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint8_t, m_entryCount, NumGroupID, );

    //!
    //! \brief    Set Group ID
    //!
    //! \param    [in] const std::uint32_t
    //!           idx value
    //! \param    [in] const std::uint32_t
    //!           id value
    //!
    //! \return   void
    //!
    void SetGroupID(const std::uint32_t idx, const uint32_t id);

    //!
    //! \brief    Get Group ID
    //!
    //! \param    [in] const std::uint32_t
    //!           idx value
    //!
    //! \return   uint32_t
    //!           group id
    //!
    uint32_t GetGroupID(const std::uint32_t idx);

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
    std::string m_contentLocation;    //!< Content location
    std::string m_contentMD5;         //!< MD5 value
    uint64_t m_contentLength;         //!< Content length
    uint64_t m_transferLength;        //!< Transfer length
    uint8_t m_entryCount;             //!< Entry count
    std::vector<uint32_t> m_groupID;  //!< std::vector of Group ID values
};

VCD_MP4_END;
#endif /* _ITEMINFOATOM_H_ */
