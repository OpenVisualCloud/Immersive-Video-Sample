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
//! \file:   MediaInfoAtom.h
//! \brief:  Media Information Atom class.
//! \detail: 'minf' Atom
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _MEDIAINFORMATIONATOM_H_
#define _MEDIAINFORMATIONATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "DataInfoAtom.h"
#include "NullMediaHeaderAtom.h"
#include "SampTableAtom.h"
#include "SoundMediaHeaderAtom.h"
#include "VideoMediaHeaderAtom.h"

VCD_MP4_BEGIN

class MediaInformationAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    MediaInformationAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~MediaInformationAtom() = default;

    enum class MediaType    //!< media type
    {
        Null,
        Video,
        Sound
    };

    //!
    //! \brief    Set Media Type
    //!
    //! \param    [in] MediaType
    //!           type value
    //!
    //! \return   void
    //!
    void SetMediaType(MediaType type);

    //!
    //! \brief    Get Media Type
    //!
    //! \return   MediaType
    //!           Media Type
    //!
    MediaType GetMediaType() const;

    //!
    //! \brief    Get Video Media Header Atom
    //!
    //! \return   VideoMediaHeaderAtom&
    //!           Video Media Header Atom
    //!
    VideoMediaHeaderAtom& GetVideoMediaHeaderAtom();

    //!
    //! \brief    Get Video Media Header Atom
    //!
    //! \return   const VideoMediaHeaderAtom&
    //!           Video Media Header Atom
    //!
    const VideoMediaHeaderAtom& GetVideoMediaHeaderAtom() const;

    //!
    //! \brief    Get Data Information Atom
    //!
    //! \return   DataInformationAtom&
    //!           Data Information Atom
    //!
    DataInformationAtom& GetDataInformationAtom();

    //!
    //! \brief    Get Data Information Atom
    //!
    //! \return   const DataInformationAtom&
    //!           Data Information Atom
    //!
    const DataInformationAtom& GetDataInformationAtom() const;

    //!
    //! \brief    Get Sample Table Atom
    //!
    //! \return   SampleTableAtom&
    //!           Sample Table Atom
    //!
    SampleTableAtom& GetSampleTableAtom();

    //!
    //! \brief    Get Sample Table Atom
    //!
    //! \return   const SampleTableAtom&
    //!           Sample Table Atom
    //!
    const SampleTableAtom& GetSampleTableAtom() const;

    //!
    //! \brief    Get Null Media Header Atom
    //!
    //! \return   NullMediaHeaderAtom&
    //!           Null Media Header Atom
    //!
    NullMediaHeaderAtom& GetNullMediaHeaderAtom();

    //!
    //! \brief    Get Null Media Header Atom
    //!
    //! \return   const NullMediaHeaderAtom&
    //!           Null Media Header Atom
    //!
    const NullMediaHeaderAtom& GetNullMediaHeaderAtom() const;

    //!
    //! \brief    Get Sound Media Header Atom
    //!
    //! \return   SoundMediaHeaderAtom&
    //!           Sound Media Header Atom
    //!
    SoundMediaHeaderAtom& GetSoundMediaHeaderAtom();

    //!
    //! \brief    Get Sound Media Header Atom
    //!
    //! \return   const SoundMediaHeaderAtom&
    //!           Sound Media Header Atom
    //!
    const SoundMediaHeaderAtom& GetSoundMediaHeaderAtom() const;

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
    MediaType m_mediaType;                        //!< Which media type?
    VideoMediaHeaderAtom m_videoMediaHeaderAtom;  //!< Video media header Atom
    SoundMediaHeaderAtom m_soundMediaHeaderAtom;  //!< Sound Media Header Atom
    NullMediaHeaderAtom m_nullMediaHeaderAtom;    //!< Null Media Header Atom
    DataInformationAtom m_dataInfoAtom;           //!< Data information Atom
    SampleTableAtom m_sampleTableAtom;            //!< Sample Table Atom
};

VCD_MP4_END;
#endif /* end of include guard: MEDIAINFORMATIONATOM_HPP */
