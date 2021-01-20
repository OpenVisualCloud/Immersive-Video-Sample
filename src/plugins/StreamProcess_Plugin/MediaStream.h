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
//! \file:   MediaStream.h
//! \brief:  Media stream base class definition
//! \detail: Media stream base class includes the media type of the
//!          stream and its setting/getting operation.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _MEDIASTREAM_H_
#define _MEDIASTREAM_H_

#include "VROmafPacking_data.h"

//!
//! \class MediaStream
//! \brief The base class for media stream
//!

class MediaStream
{
public:
    //!
    //! \brief  Constructor
    //!
    MediaStream()
    {
        m_mediaType = VIDEOTYPE;
        m_codecId   = CODEC_ID_H265;
    };

    //!
    //! \brief  Destructor
    //!
    virtual ~MediaStream() {};

    //!
    //! \brief  Set the media type for stream
    //!
    //! \param  [in] mediaType
    //!         the media type will be set to the stream
    //!
    //! \return void
    //!
    void SetMediaType(MediaType mediaType)
    {
        m_mediaType = mediaType;
    };

    //!
    //! \brief  Get the media type of the stream
    //!
    //! \return MediaType
    //!         VIDEOTYPE if video, AUDIOTYPE if audio
    //!
    MediaType GetMediaType() { return m_mediaType; };

    //!
    //! \brief  Set the codec index for stream
    //!
    //! \param  [in] codecIdx
    //!         the codec index will be set to the stream
    //!
    //! \return void
    //!
    void SetCodecId(CodecId codecIdx)
    {
        m_codecId = codecIdx;
    };

    //!
    //! \brief  Get the codec index of the stream
    //!
    //! \return CodecId
    //!         codec index of the media stream
    //!
    CodecId GetCodecId() { return m_codecId; };

protected:
    MediaType   m_mediaType;    //!< media type of the media stream
    CodecId     m_codecId;      //!< codec index of the media stream
};

#endif /* _MEDIASTREAM_H_ */
