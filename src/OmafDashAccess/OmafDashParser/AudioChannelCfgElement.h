/*
 * Copyright (c) 2020, Intel Corporation
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
//! \file:   AudioChannelCfgElement.h
//! \brief:  AudioChannelConfiguration element class
//!

#ifndef AUDIOCHANNELCONFIGURATIONELEMENT_H
#define AUDIOCHANNELCONFIGURATIONELEMENT_H
#include "OmafElementBase.h"
#include "DescriptorElement.h"

VCD_OMAF_BEGIN

class AudioChannelConfigurationElement: public DescriptorElement, public OmafElementBase
{
public:

    //!
    //! \brief Constructor
    //!
    AudioChannelConfigurationElement()
    {
        m_chlCfg = 0;
    };

    //!
    //! \brief Destructor
    //!
    virtual ~AudioChannelConfigurationElement();

    //!
    //! \brief    Parse SchemeIdUri and it's value
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus ParseSchemeIdUriAndValue();

    int32_t GetChannelCfg() { return m_chlCfg; };

private:

    int32_t m_chlCfg;
};

VCD_OMAF_END;

#endif //AUDIOCHANNELCONFIGURATIONELEMENT_H
