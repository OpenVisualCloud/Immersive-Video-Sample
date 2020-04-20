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
//! \file:   ElemStreamDescAtom.h
//! \brief:  Elementary Stream Descriptor Atom
//! \detail: 'esds' Atom containing the ES_Params
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _ELEMENTARYSTREAMDESCRIPTORATOM_H_
#define _ELEMENTARYSTREAMDESCRIPTORATOM_H_

#include <vector>
#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class ElementaryStreamDescriptorAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ElementaryStreamDescriptorAtom();
    ElementaryStreamDescriptorAtom(const ElementaryStreamDescriptorAtom& Atom);

    ElementaryStreamDescriptorAtom& operator=(const ElementaryStreamDescriptorAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~ElementaryStreamDescriptorAtom() = default;

    struct DecodeSpec   //!< decoder specification
    {
        std::uint8_t flag  = 0;
        std::uint32_t size = 0;
        std::vector<uint8_t> info;
    };

    struct DecoderConfig    //!< decoder configuration
    {
        std::uint8_t flag                  = 0;
        std::uint32_t size                 = 0;
        std::uint8_t idc                   = 0;
        std::uint8_t strType               = 0;
        std::uint32_t bufferSizeDB         = 0;
        std::uint32_t maxBitrate           = 0;
        std::uint32_t avgBitrate           = 0;
        DecodeSpec info;
    };

    struct ES_Params    //!< ES parameters
    {
        std::uint8_t descrFlag        = 0;
        std::uint32_t size            = 0;
        std::uint16_t id              = 0;
        std::uint8_t flags            = 0;
        std::uint16_t depend          = 0;
        std::uint8_t URLlen           = 0;
        std::string URL;
        std::uint16_t OCR_Id          = 0;
        DecoderConfig decConfig;
    };

    //!
    //! \brief    get one parameters set
    //!
    //! \param    [in] std::vector<std::uint8_t>&
    //!           byte stream
    //!
    //! \return   bool
    //!           operation success or not
    //!
    bool GetOneParameterSet(std::vector<uint8_t>& byteStream) const;

    //!
    //! \brief    Set ES Descriptor
    //!
    //! \param    [in] ElementaryStreamDescriptorAtom::ES_Params
    //!           es Descriptor
    //!
    //! \return   void
    //!
    void SetESDescriptor(ElementaryStreamDescriptorAtom::ES_Params esDescriptor);

    //!
    //! \brief    Get ES Descriptor
    //!
    //! \return   ES_Params
    //!           ES Descriptor
    //!
    ES_Params GetESDescriptor() const;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& output);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& input);

private:
    ES_Params m_ES_Params;              //!< ES_Params
    std::list<DecodeSpec> m_otherinfo;  //!< other information
};

VCD_MP4_END;
#endif /* _ELEMENTARYSTREAMDESCRIPTORATOM_H_ */
