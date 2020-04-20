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
//! \file:   DataRefAtom.h
//! \brief:  Data Entry Atom class.
//! \detail: Used as data reference basic atom.
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _DATAREFERENCEATOM_H_
#define _DATAREFERENCEATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

#include <cstdint>

VCD_MP4_BEGIN

class DataEntryAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    DataEntryAtom(FourCCInt AtomType, std::uint8_t version, std::uint32_t flags);

    //!
    //! \brief Destructor
    //!
    virtual ~DataEntryAtom() = default;

    //!
    //! \brief    Set Location
    //!
    //! \param    [in] const std::string&
    //!           Location value
    //!
    //! \return   void
    //!
    void SetLocation(const std::string& location);

    //!
    //! \brief    Get Location
    //!
    //! \return   const std::string
    //!           Location
    //!
    const std::string GetLocation() const;

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

private:
    std::string m_location; //!< data location
};

class DataEntryUrlAtom : public DataEntryAtom
{
public:
    enum IsContained
    {
        NotContained,
        Contained
    };

    //!
    //! \brief Constructor
    //!
    DataEntryUrlAtom(IsContained isContained = NotContained);

    //!
    //! \brief Destructor
    //!
    virtual ~DataEntryUrlAtom() = default;

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
};

class DataEntryUrnAtom : public DataEntryAtom
{
public:

    //!
    //! \brief Constructor
    //!
    DataEntryUrnAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~DataEntryUrnAtom() = default;

    //!
    //! \brief    Set Name
    //!
    //! \param    [in] const std::string&
    //!           Name value
    //!
    //! \return   void
    //!
    void SetName(const std::string& name);

    //!
    //! \brief    Get Name
    //!
    //! \return   const std::string
    //!           Name
    //!
    const std::string GetName() const;

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
    std::string m_name; //!< data name
};

class DataReferenceAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    DataReferenceAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~DataReferenceAtom() = default;

    //!
    //! \brief    Add data Entry
    //!
    //! \param    [in] std::shared_ptr<DataEntryAtom>
    //!           data Entry Atom value
    //!
    //! \return   unsigned int
    //!           data entry size
    //!
    unsigned int AddEntry(std::shared_ptr<DataEntryAtom> dataEntryAtom);

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
    std::vector<std::shared_ptr<DataEntryAtom>> m_dataEntries;  //!< data entry array
};

VCD_MP4_END;
#endif /* _DATAREFERENCEATOM_H_ */