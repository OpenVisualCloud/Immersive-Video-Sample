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
//! \file:   OmafDownloader.h
//! \brief:  downloader abstract class
//!

#ifndef OMAFDOWNLOADER_H
#define OMAFDOWNLOADER_H

#include "../OmafDashParser/Common.h"
#include "OmafDownloaderObserver.h"

VCD_OMAF_BEGIN

//!
//! \class:  OmafDownloader
//! \brief:  downloader base
//!
class OmafDownloader
{
public:

    //!
    //! \brief Constructor
    //!
    OmafDownloader(){};

    //!
    //! \brief Destructor
    //!
    virtual ~OmafDownloader(){};

    //!
    //! \brief    Stop download
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus Stop() = 0;

    //!
    //! \brief    start download
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus Start() = 0;

    //!
    //! \brief    Read given size stream to data pointer
    //!
    //! \param    [in] size
    //!           size of stream that should read
    //! \param    [out] data
    //!           pointer stores read stream data
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus Read(uint8_t* data, size_t size) = 0;

    //!
    //! \brief    Peek given size stream to data pointer
    //!
    //! \param    [in] size
    //!           size of stream that should read
    //! \param    [out] data
    //!           pointer stores read stream data
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus Peek(uint8_t* data, size_t size) = 0;

    //!
    //! \brief    Peek given size stream to data pointer start from offset
    //!
    //! \param    [in] size
    //!           size of stream that should read
    //! \param    [in] offset
    //!           stream offset that read should start
    //! \param    [out] data
    //!           pointer stores read stream data
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus Peek(uint8_t* data, size_t size, size_t offset) = 0;

    //!
    //! \brief    Attach download observer
    //!
    //! \param    [in] observer
    //!           observer need to be attached
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus ObserverAttach(OmafDownloaderObserver *observer) = 0;

    //!
    //! \brief    Dettach download observer
    //!
    //! \param    [in] observer
    //!           observer need to be dettached
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus ObserverDetach(OmafDownloaderObserver* observer) = 0;

    //!
    //! \brief    Get download rate
    //!
    //! \return   double
    //!           download rate
    //!
    virtual double GetDownloadRate() = 0;
};

VCD_OMAF_END;

#endif //OMAFDOWNLOADER_H