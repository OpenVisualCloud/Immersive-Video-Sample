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
//! \file:   OmafDownloaderObserver.h
//! \brief:  download observer base
//!

#ifndef OMAFDOWNLOADEROBSERVER_H
#define OMAFDOWNLOADEROBSERVER_H

#include "../OmafDashParser/Common.h"

VCD_OMAF_BEGIN

//!
//! \class  OmafDownloaderObserver
//! \brief  download observer abstract class
//!
class OmafDownloaderObserver
{
public:

    //!
    //! \brief Constructor
    //!
    OmafDownloaderObserver(){};

    //!
    //! \brief Destructor
    //!
    virtual ~OmafDownloaderObserver(){};

    //!
    //! \brief    Will be notified by downloader there is new downloaded data
    //!
    //! \param    [in] downloadedDataLength
    //!           length of downloaded data
    //!
    //! \return   void
    //!
    virtual void DownloadDataNotify(uint64_t downloadedDataLength) = 0;

    //!
    //! \brief    Will be notified by downloader the status has changed
    //!
    //! \param    [in] status
    //!           the changed status
    //!
    //! \return   void
    //!
    virtual void DownloadStatusNotify(DownloaderStatus status) = 0;
};

VCD_OMAF_END;

#endif //OMAFDOWNLOADEROBSERVER_H