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
//! \file:   Mp4AudDecConfigRecord.h
//! \brief:  Mp4AudDecConfigRecord class
//! \detail: Mp4 Audio Decode Configuration Record
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _MP4AUDIODECODERCONFIGRECORD_H_
#define _MP4AUDIODECODERCONFIGRECORD_H_

#include "DecConfigRecord.h"

VCD_MP4_BEGIN

class ElementaryStreamDescriptorAtom;

class MP4AudioDecoderConfigurationRecord : public DecoderConfigurationRecord
{
public:

    //!
    //! \brief Constructor
    //!
    MP4AudioDecoderConfigurationRecord(ElementaryStreamDescriptorAtom& aAtom);

    //!
    //! \brief Destructor
    //!
    virtual ~MP4AudioDecoderConfigurationRecord() = default;

    //!
    //! \brief    Get Configuration Map
    //!
    //! \param    [in] ConfigurationMap&
    //!           map
    //! \return   void
    //!
    virtual void GetConfigurationMap(ConfigurationMap& aMap) const override;

protected:
    const ElementaryStreamDescriptorAtom& m_ESDAtom;    //!< Elementary Stream Descriptor Atom
};

VCD_MP4_END;
#endif /* _MP4AUDIODECODERCONFIGRECORD_H_ */
