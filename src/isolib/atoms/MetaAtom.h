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
//! \file:   MetaAtom.h
//! \brief:  Meta Atom class
//! \detail: 'meta' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _METAATOM_H_
#define _METAATOM_H_

#include "FormAllocator.h"
#include "DataInfoAtom.h"
#include "FullAtom.h"
#include "HandlerAtom.h"
#include "ItemDataAtom.h"
#include "ItemInfoAtom.h"
#include "ItemLocationAtom.h"
#include "ItemProtAtom.h"
#include "ItemRefAtom.h"
#include "PrimaryItemAtom.h"
#include <string>

VCD_MP4_BEGIN

class XmlAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    XmlAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~XmlAtom() = default;

    //!
    //! \brief    Set and Get function for m_contents member
    //!
    //! \param    [in] const std::string&
    //!           value to set
    //! \param    [in] m_contents
    //!           m_contents member in class
    //! \param    [in] Contents
    //!           m_contents name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::string&, m_contents, Contents, const);

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
    std::string m_contents;  //!< the XML content
};

class MetaAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    MetaAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~MetaAtom() = default;

    //!
    //! \brief    Get Handler Type
    //!
    //! \return   FourCCInt
    //!           Handler Type
    //!
    FourCCInt GetHandlerType() const;

    //!
    //! \brief    Set Handler Type
    //!
    //! \param    [in] FourCCInt
    //!           Handler value
    //!
    //! \return   void
    //!
    void SetHandlerType(FourCCInt handler);

    //!
    //! \brief    Get PrimaryItem Atom
    //!
    //! \return   const PrimaryItemAtom&
    //!           PrimaryItem Atom
    //!
    const PrimaryItemAtom& GetPrimaryItemAtom() const;

    //!
    //! \brief    Set PrimaryItem Atom
    //!
    //! \param    [in] std::uint32_t
    //!           item Id
    //!
    //! \return   void
    //!
    void SetPrimaryItem(std::uint32_t itemId);

    //!
    //! \brief    Get ItemInfo Atom
    //!
    //! \return   const ItemInfoAtom&
    //!           ItemInfo Atom
    //!
    const ItemInfoAtom& GetItemInfoAtom() const;

    //!
    //! \brief    Set ItemInfo Atom
    //!
    //! \param    [in] const ItemInfoAtom&
    //!           item Info Atom
    //!
    //! \return   void
    //!
    void SetItemInfoAtom(const ItemInfoAtom& itemInfoAtom);

    //!
    //! \brief    Get ItemLocation Atom
    //!
    //! \return   const ItemLocationAtom&
    //!           ItemLocation Atom
    //!
    const ItemLocationAtom& GetItemLocationAtom() const;

    //!
    //! \brief    Get ItemReference Atom
    //!
    //! \return   const ItemReferenceAtom&
    //!           ItemReference Atom
    //!
    const ItemReferenceAtom& GetItemReferenceAtom() const;

    //!
    //! \brief    Add Property
    //!
    //! \param    [in] std::shared_ptr<Atom>
    //!           atom pointer
    //! \param    [in] const std::vector<std::uint32_t>&
    //!           item Ids
    //! \param    [in] bool
    //!           essential or not
    //!
    //! \return   void
    //!
    void AddProperty(std::shared_ptr<Atom> Atom, const std::vector<std::uint32_t>& itemIds, bool essential);

    //!
    //! \brief    Add Property
    //!
    //! \param    [in] std::uint16_t
    //!           index
    //! \param    [in] const std::vector<std::uint32_t>&
    //!           item Ids
    //! \param    [in] bool
    //!           essential or not
    //!
    //! \return   void
    //!
    void AddProperty(std::uint16_t index, const std::vector<std::uint32_t>& itemIds, bool essential);

    //!
    //! \brief    Get ProtectionSchemeInfo Atom
    //!
    //! \param    [in] std::uint16_t
    //!           index
    //! \return   const ProtectionSchemeInfoAtom&
    //!           ProtectionSchemeInfo Atom
    //!
    const ProtectionSchemeInfoAtom& GetProtectionSchemeInfoAtom(std::uint16_t index) const;

    //!
    //! \brief    Get DataInformation Atom
    //!
    //! \return   const DataInformationAtom&
    //!           DataInformation Atom
    //!
    const DataInformationAtom& GetDataInformationAtom() const;

    //!
    //! \brief    Get ItemData Atom
    //!
    //! \return   const ItemDataAtom&
    //!           ItemData Atom
    //!
    const ItemDataAtom& GetItemDataAtom() const;

    //!
    //! \brief    Add Iloc
    //!
    //! \param    [in] std::uint32_t
    //!           item Ids
    //! \param    [in] std::uint32_t
    //!           offset
    //! \param    [in] std::uint32_t
    //!           length
    //! \param    [in] std::uint32_t
    //!           baseOffset
    //!
    //! \return   void
    //!
    void AddIloc(std::uint32_t itemId, std::uint32_t offset, std::uint32_t length, std::uint32_t baseOffset);

    //!
    //! \brief    Add Item
    //!
    //! \param    [in] std::uint32_t
    //!           item Ids
    //! \param    [in] FourCCInt
    //!           type
    //! \param    [in] const std::string&
    //!           name
    //! \param    [in] bool
    //!           hidden or not
    //!
    //! \return   void
    //!
    void AddItem(std::uint32_t itemId, FourCCInt type, const std::string& name, bool hidden = false);

    //!
    //! \brief    Add Item Reference
    //!
    //! \param    [in] FourCCInt
    //!           type
    //! \param    [in] std::uint32_t
    //!           fromId
    //! \param    [in] std::uint32_t
    //!           toId
    //!
    //! \return   void
    //!
    void AddItemReference(FourCCInt type, std::uint32_t fromId, std::uint32_t toId);

    //!
    //! \brief    Add Idat Item
    //!
    //! \param    [in] std::uint32_t
    //!           item Ids
    //! \param    [in] FourCCInt
    //!           type
    //! \param    [in] const std::string&
    //!           name
    //! \param    [in] const std::vector<uint8_t>&
    //!           data
    //!
    //! \return   void
    //!
    void AddIdatItem(std::uint32_t itemId, FourCCInt type, const std::string& name, const std::vector<uint8_t>& data);

    //!
    //! \brief    Add Item Idat Extent
    //!
    //! \param    [in] std::uint32_t
    //!           item Ids
    //! \param    [in] const std::vector<uint8_t>&
    //!           data
    //!
    //! \return   void
    //!
    void AddItemIdatExtent(std::uint32_t itemId, const std::vector<uint8_t>& data);

    //!
    //! \brief    Add Mdat Item
    //!
    //! \param    [in] std::uint32_t
    //!           item Ids
    //! \param    [in] FourCCInt
    //!           type
    //! \param    [in] const std::string&
    //!           name
    //! \param    [in] std::uint32_t
    //!           base Offset
    //!
    //! \return   void
    //!
    void AddMdatItem(std::uint32_t itemId, FourCCInt type, const std::string& name, std::uint32_t baseOffset);

    //!
    //! \brief    Add Item Extent
    //!
    //! \param    [in] std::uint32_t
    //!           item Ids
    //! \param    [in] std::uint32
    //!           offset
    //! \param    [in] std::uint32_t
    //!           length
    //!
    //! \return   void
    //!
    void AddItemExtent(std::uint32_t itemId, std::uint32_t offset, std::uint32_t length);

    //!
    //! \brief    Get Xml Atom
    //!
    //! \return   const XmlAtom&
    //!           Xml Atom
    //!
    const XmlAtom& GetXmlAtom() const;

    //!
    //! \brief    Set Xml Atom
    //!
    //! \param    [in] const XmlAtom&
    //!           Xml Atom
    //!
    //! \return   void
    //!
    void SetXmlAtom(const XmlAtom& xmlAtom);

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
    HandlerAtom m_handlerAtom;              //!< handler atom
    bool m_hasPrimaryItemAtom;              //!< has PrimaryItemAtom or not
    PrimaryItemAtom m_primaryItemAtom;      //!< primary Item Atom
    bool m_hasDataInfoAtom;                 //!< has DataInfoAtom or not
    DataInformationAtom m_dataInfoAtom;     //!< Data Information Atom
    bool m_hasItemLocationAtom;             //!< has ItemLocationAtom or not
    ItemLocationAtom m_itemLocationAtom;    //!< Item Location Atom
    bool m_hasItemProtectionAtom;           //!< has ItemProtectionAtom or not
    ItemProtectionAtom m_itemProtectionAtom;//!< Item Protection Atom
    bool m_hasItemInfoAtom;                 //!< has Item Info Atom
    ItemInfoAtom m_itemInfoAtom;            //!< Item Information Atom
    bool m_hasItemRefAtom;                  //!< has ItemRefAtom or not
    ItemReferenceAtom m_itemRefAtom;        //!< Item Reference Atom
    bool m_hasItemDataAtom;                 //!< has ItemDataAtom or not
    ItemDataAtom m_itemDataAtom;            //!< Item Data Atom
    bool m_hasXmlAtom;                      //!< has XmlAtom or not
    XmlAtom m_xmlAtom;                      //!< Xml Atom
};

VCD_MP4_END;
#endif /* _METAATOM_H_ */
