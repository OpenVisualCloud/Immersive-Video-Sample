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
//! \file:   SampDescAtom.h
//! \brief:  SampDescAtom class.
//! \detail: support different sample entry types
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _SAMPLEDESCRIPTIONATOM_H_
#define _SAMPLEDESCRIPTIONATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"
#include "SampEntryAtom.h"

VCD_MP4_BEGIN

class SampleDescriptionAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    SampleDescriptionAtom();

    //!
    //! \brief Destructor
    //!
    ~SampleDescriptionAtom() = default;

    //!
    //! \brief    Add Sample Entry
    //!
    //! \param    [in] UniquePtr<SampleEntryAtom>
    //!           sample Entry value
    //!
    //! \return   void
    //!
    void AddSampleEntry(UniquePtr<SampleEntryAtom> sampleEntry);

    //!
    //! \brief    Get Sample Entries template
    //!
    //! \return   std::vector<T*>
    //!           entries
    //!
    template <typename T>
    std::vector<T*> GetSampleEntries() const;

    //!
    //! \brief    Get Sample Entry template
    //!
    //! \param    [in] unsigned int
    //!           index value
    //!
    //! \return   T*
    //!           entry pointer
    //!
    template <typename T>
    T* GetSampleEntry(unsigned int index) const;

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
    std::vector<UniquePtr<SampleEntryAtom>> m_index;  //!< std::vector of sample entries
};

template <typename T>
std::vector<T*> SampleDescriptionAtom::GetSampleEntries() const
{
    std::vector<T*> ret;
    for (auto& entry : m_index)
    {
        T* p = dynamic_cast<T*>(entry.get());
        if (p)
        {
            ret.push_back(p);
        }
    }
    return ret;
}

template <typename T>
T* SampleDescriptionAtom::GetSampleEntry(const unsigned int index) const
{
    if (m_index.size() < index || index == 0)
    {
        ISO_LOG(LOG_ERROR, "SampleDescriptionAtom::GetSampleEntry invalid sample entry index.\n");
        throw Exception();
    }

    T* pPtr = dynamic_cast<T*>(m_index.at(index - 1).get());
    return pPtr;
}

VCD_MP4_END;
#endif /* _SAMPLEDESCRIPTIONATOM_H_ */
