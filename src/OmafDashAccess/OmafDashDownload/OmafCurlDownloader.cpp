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
//! \file:   OmafCurlDownloader.cpp
//! \brief:  downloader class with libcurl
//!

#include "OmafCurlDownloader.h"

VCD_OMAF_BEGIN

OmafCurlDownloader::OmafCurlDownloader()
{
    m_status = NOT_START;
    m_downloadRate = 0;
    m_endTime      = 0;
    m_startTime    = 0;
    m_curlHandler  = NULL;
}

OmafCurlDownloader::OmafCurlDownloader(string url):OmafCurlDownloader()
{
    m_url = url;
}

OmafCurlDownloader::~OmafCurlDownloader()
{
    this->CleanUp();

    if(m_observers.size())
    {
        m_observers.clear();
    }
}

ODStatus OmafCurlDownloader::InitCurl()
{
    curl_global_init(CURL_GLOBAL_ALL);

    m_curlHandler = curl_easy_init();
    CheckNullPtr_PrintLog_ReturnStatus(m_curlHandler, "failed to init curl library.", ERROR, OD_STATUS_OPERATION_FAILED);

    curl_easy_setopt(m_curlHandler, CURLOPT_URL, m_url.c_str());
    curl_easy_setopt(m_curlHandler, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_curlHandler, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_curlHandler, CURLOPT_WRITEFUNCTION, CallBackForCurl);
    curl_easy_setopt(m_curlHandler, CURLOPT_WRITEDATA, (void*)this);
    return OD_STATUS_SUCCESS;
}


ODStatus OmafCurlDownloader::Start()
{
    ODStatus st = OD_STATUS_SUCCESS;

    if(GetStatus() != NOT_START)
        return OD_STATUS_INVALID;

    st = InitCurl();
    CheckAndReturn(st);

    StartThread(true);

    m_startTime = chrono::duration_cast<std::chrono::milliseconds>(m_clock.now().time_since_epoch()).count();

    SetStatus(DOWNLOADING);

    return st;
}

ODStatus OmafCurlDownloader::Stop()
{
    this->SetStatus(STOPPING);
    this->Join();
    return OD_STATUS_SUCCESS;
}

ODStatus OmafCurlDownloader::Read(uint8_t* data, size_t size)
{
    return m_stream.GetStream((char*)data, size);
}

ODStatus OmafCurlDownloader::Peek(uint8_t* data, size_t size)
{
    return m_stream.PeekStream((char*)data, size);
}

ODStatus OmafCurlDownloader::Peek(uint8_t* data, size_t size, size_t offset)
{
    return m_stream.PeekStream((char*)data, size, offset);
}

void* OmafCurlDownloader::Download(void* downloader)
{
    OmafCurlDownloader* curlDownloader = static_cast<OmafCurlDownloader*>(downloader);
    LOG(INFO)<<"now download "<<curlDownloader->m_url<<endl;
    curl_easy_perform(curlDownloader->m_curlHandler);
    curl_easy_cleanup(curlDownloader->m_curlHandler);
    curl_global_cleanup();

    if(curlDownloader->GetStatus() == STOPPING)
        curlDownloader->SetStatus(STOPPED);
    else
    {
        curlDownloader->SetStatus(DOWNLOADED);
    }

    m_stream.ReachedEOS();

    return nullptr;
}

void OmafCurlDownloader::Run()
{
    Download(this);
}

ODStatus OmafCurlDownloader::ObserverAttach(OmafDownloaderObserver *observer)
{
    m_observerLock.lock();

    m_observers.insert(observer);

    m_observerLock.unlock();

    return OD_STATUS_SUCCESS;
}

ODStatus OmafCurlDownloader::ObserverDetach(OmafDownloaderObserver* observer)
{
    ScopeLock tmplock(m_observerLock);

    if(!m_observers.size() || !observer)
        return OD_STATUS_INVALID;

    if(m_observers.find(observer) == m_observers.end())
    {
        //LOG(WARNING)<<"The observer is not recorded in this downloader!"<<endl;
        return OD_STATUS_INVALID;
    }

    m_observers.erase(observer);

    return OD_STATUS_SUCCESS;
}

ODStatus OmafCurlDownloader::NotifyStatus()
{
    m_observerLock.lock();
    for(auto observer: m_observers)
    {
        observer->DownloadStatusNotify(this->GetStatus());
    }
    m_observerLock.unlock();

    return OD_STATUS_SUCCESS;
}

ODStatus OmafCurlDownloader::NotifyDownloadedData()
{
    m_observerLock.lock();
    for(auto observer: m_observers)
    {
        observer->DownloadDataNotify(this->m_stream.GetTotalStreamLength());
    }
    m_observerLock.unlock();

    return OD_STATUS_SUCCESS;
}

ODStatus OmafCurlDownloader::CleanUp()
{
    // make sure downloader is stopped or wait time is more than 10 mins
    int64_t waitTime = 0;
    while(m_status != STOPPED && waitTime < 6000000)
    {
        usleep(100);
        waitTime++;
    }

    return OD_STATUS_SUCCESS;
}

size_t OmafCurlDownloader::CallBackForCurl(void* downloadedData, size_t dataSize, size_t typeSize, void* handle)
{
    OmafCurlDownloader* curlDownloder = (OmafCurlDownloader*) handle;
    // return 0 directly if the downloader status is stopping
    if(curlDownloder->GetStatus() == STOPPING)
        return 0;

    size_t size = dataSize * typeSize;
    char* data = new char[size];
    memcpy(data, downloadedData, size);
    curlDownloder->m_stream.AddSubStream(data, size);

    // notify all the observers that more data is downloaded
    curlDownloder->NotifyDownloadedData();

    //calculate the download rate
    uint64_t endTime = chrono::duration_cast<std::chrono::milliseconds>(curlDownloder->m_clock.now().time_since_epoch()).count();

    double downloadRate = curlDownloder->m_stream.GetTotalStreamLength() / ((endTime - curlDownloder->m_startTime) * 1000.0);
    curlDownloder->SetDownloadRate(downloadRate);

    return size;
}

ODStatus OmafCurlDownloader::SetStatus(DownloaderStatus status)
{
    ODStatus ret = OD_STATUS_SUCCESS;

    if(m_stream.IsEOS() && status == STOPPING)
        status = STOPPED;

    m_statusLock.lock();
    m_status = status;
    m_statusLock.unlock();

    // notify all the observers that status has been changed
    ret = NotifyStatus();

    return ret;
}

DownloaderStatus OmafCurlDownloader::GetStatus()
{
    DownloaderStatus status = NOT_START;

    m_statusLock.lock();
    status = m_status;
    m_statusLock.unlock();

    return status;
}

double OmafCurlDownloader::GetDownloadRate()
{
    m_rateLock.lock();
    double rate = m_downloadRate;
    m_rateLock.unlock();

    return rate;
}

void OmafCurlDownloader::SetDownloadRate(double rate)
{
    m_rateLock.lock();
    m_downloadRate = rate;
    m_rateLock.unlock();
}

VCD_OMAF_END
