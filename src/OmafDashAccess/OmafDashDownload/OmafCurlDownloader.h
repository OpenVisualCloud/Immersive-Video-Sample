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
//! \file:   OmafCurlDownloader.h
//! \brief:  downloader class with libcurl
//!

#ifndef OMAFCURLDOWNLOADER_H
#define OMAFCURLDOWNLOADER_H

#include <curl/curl.h>
#include "OmafDownloader.h"
#include "Stream.h"
#include "../OmafDashParser/SegmentElement.h"

VCD_USE_VRVIDEO;

VCD_OMAF_BEGIN

//!
//! \class:  OmafCurlDownloader
//! \brief:  downloader with libcurl
//!
class OmafCurlDownloader: public OmafDownloader, ThreadLock, Threadable
{
public:

    //!
    //! \brief Constructor
    //!
    OmafCurlDownloader();

    //!
    //! \brief Constructor with parameter
    //!
    OmafCurlDownloader(string url);

    //!
    //! \brief Destructor
    //!
    virtual ~OmafCurlDownloader();

    //!
    //! \brief    Stop download
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus Stop();

    //!
    //! \brief    start download
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus Start();

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
    virtual ODStatus Read(uint8_t* data, size_t size);

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
    virtual ODStatus Peek(uint8_t* data, size_t size);

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
    virtual ODStatus Peek(uint8_t* data, size_t size, size_t offset);

    //!
    //! \brief    Attach download observer
    //!
    //! \param    [in] observer
    //!           observer need to be attached
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus ObserverAttach(OmafDownloaderObserver *observer);

    //!
    //! \brief    Dettach download observer
    //!
    //! \param    [in] observer
    //!           observer need to be dettached
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus ObserverDetach(OmafDownloaderObserver* observer);

    //!
    //! \brief    Get download rate
    //!
    //! \return   double
    //!           download rate
    //!
    virtual double GetDownloadRate();

    //!
    //! \brief Interface implementation from base class: Threadable
    //!
    virtual void Run();

private:

    //!
    //! \brief    Notify observers the status has changed
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus NotifyStatus();

    //!
    //! \brief    Notify observers there is new downloaded data
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus NotifyDownloadedData();

    //!
    //! \brief    Initialize curl library
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus InitCurl();

    //!
    //! \brief    Clean up curl related resources
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus CleanUp();

    //!
    //! \brief    Callback function for curl
    //!
    //! \param    [in] downloadedData
    //!           pointer to downloaded data
    //! \param    [in] dataSize
    //!           size of data
    //! \param    [in] typeSize
    //!           size of data type
    //! \param    [in] handle
    //!           handle for this class
    //!
    //! \return   size_t
    //!           the downloaded size
    //!
    static size_t CallBackForCurl(void* downloadedData, size_t dataSize, size_t typeSize, void* handle);

    //!
    //! \brief    Download run in thread
    //!
    //! \param    [in] downloader
    //!           the downloader handle
    //!
    //! \return   void
    //!
    void* Download(void* downloader);

    //!
    //! \brief    Set download status
    //!
    //! \param    [in] status
    //!           download status
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus SetStatus(DownloaderStatus status);

    //!
    //! \brief    Set download rate
    //!
    //! \param    [in] rate
    //!           download rate
    //!
    //! \return   void
    //!
    void SetDownloadRate(double rate);

    //!
    //! \brief    Get download status
    //!
    //! \return   DownloaderStatus
    //!           status
    //!
    DownloaderStatus GetStatus();

    unordered_set<OmafDownloaderObserver*>  m_observers;    //!< attached downloader observers
    DownloaderStatus                        m_status;       //!< download status
    ThreadLock                              m_statusLock;   //!< locker for status
    ThreadLock                              m_observerLock; //!< locker for observers
    Stream                                  m_stream;       //!< download stream
    CURL*                                   m_curlHandler;  //!< curl handle
    string                                  m_url;          //!< download url

    chrono::high_resolution_clock           m_clock;        //!< clock for calculating rate
    uint64_t                                m_startTime;    //!< download start time
    uint64_t                                m_endTime;      //!< download end time
    double                                  m_downloadRate; //!< real-time download rate (bytes/s)
    ThreadLock                              m_rateLock;     //!< lock for download rate
};

VCD_OMAF_END;

#endif //OMAFCURLDOWNLOADER_H