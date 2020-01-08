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
//! \file:   SegmentElement.cpp
//! \brief:  SegmentTemplate element class
//!

#include "SegmentElement.h"
#include "../OmafDashDownload/OmafCurlDownloader.h"

VCD_OMAF_BEGIN

SegmentElement::SegmentElement()
{
    m_downloader = nullptr;

    m_duration = 0;
    m_startNumber = 0;
    m_timescale = 0;
}

SegmentElement::~SegmentElement()
{
    m_media.clear();
    m_initialization.clear();
    m_duration = 0;
    m_startNumber = 0;
    m_timescale = 0;

    m_url.clear(); // TBD

    ResetDownload();
}


ODStatus SegmentElement::InitDownload(vector<BaseUrlElement*>& baseURL, string& representationID, int32_t number, int32_t bandwidth, int32_t time)
{
    this->ResetDownload();

    string completeURL = GenerateCompleteURL(baseURL, representationID, number, bandwidth, time);
    //LOG(INFO)<<"Download "<<completeURL<<endl;
    if(!completeURL.length())
    {
        LOG(ERROR)<<"Cannot get the url for this segment!"<<endl;
        return OD_STATUS_OPERATION_FAILED;
    }

    m_url = completeURL;

    m_downloader = new OmafCurlDownloader(completeURL);
    CheckNullPtr_PrintLog_ReturnStatus(m_downloader, "Failed to create downloader.", ERROR, OD_STATUS_OPERATION_FAILED);

    return OD_STATUS_SUCCESS;
}

ODStatus SegmentElement::ResetDownload()
{
    if(m_downloader)
    {
        m_downloader->Stop();
    }
    SAFE_DELETE(m_downloader);

    m_url.clear();
    return OD_STATUS_SUCCESS;
}

ODStatus SegmentElement::StartDownloadSegment(OmafDownloaderObserver* observer)
{
    CheckNullPtr_PrintLog_ReturnStatus(m_downloader, "The downloader is not created yet!", ERROR, OD_STATUS_INVALID);

    //attach the observers to downloader
    CheckAndReturn(m_downloader->ObserverAttach(observer));

    m_downloader->Start();

    return OD_STATUS_SUCCESS;
}

ODStatus SegmentElement::StopDownloadSegment(OmafDownloaderObserver* observer)
{
    if(!m_downloader)
        return OD_STATUS_INVALID;

    m_downloader->Stop();
    if(observer) m_downloader->ObserverDetach(observer);

    return OD_STATUS_SUCCESS;
}

ODStatus SegmentElement::Read(uint8_t* data, size_t size)
{
    CheckNullPtr_PrintLog_ReturnStatus(m_downloader, "The downloader is not created yet!", ERROR, OD_STATUS_INVALID);

    return m_downloader->Read(data, size);
}

ODStatus SegmentElement::Peek(uint8_t* data, size_t size)
{
    CheckNullPtr_PrintLog_ReturnStatus(m_downloader, "The downloader is not created yet!", ERROR, OD_STATUS_INVALID);

    return m_downloader->Peek(data, size);
}

ODStatus SegmentElement::Peek(uint8_t* data, size_t size, size_t offset)
{
    CheckNullPtr_PrintLog_ReturnStatus(m_downloader, "The downloader is not created yet!", ERROR, OD_STATUS_INVALID);

    return m_downloader->Peek(data, size, offset);
}

string SegmentElement::GenerateCompleteURL(vector<BaseUrlElement*>& baseURL, string& representationID, int32_t number, int32_t bandwidth, int32_t time)
{
    string combinedBaseURL;
    for(uint32_t i = 0; i < baseURL.size() - 1 ; i++)
    {
        auto url = baseURL[i];
        combinedBaseURL = PathSplice(combinedBaseURL, url->GetPath());
    }

    // check if it is getting the initialization segment
    string fileName = number ? m_media : m_initialization;

    // replace the reference value with real value in file name
    vector<string> subNames;
    SplitString(fileName, subNames, "$");
    fileName.clear();
    for( uint32_t i = 0; i < subNames.size();i++)
    {
        string sn = subNames[i];
        if(sn == SEGMENT_NUMBER)
        {
            sn = to_string(number);
        }
        else if(sn == SEGMENT_REPRESENTATIONID)
        {
            sn = representationID;
        }
        else if(sn == SEGMENT_BANDWIDTH)
        {
            sn = to_string(bandwidth);
        }
        else if (sn == SEGMENT_TIME)
        {
            sn = to_string(time);
        }
        fileName += sn;
    }

    combinedBaseURL = PathSplice(combinedBaseURL, fileName);

    return combinedBaseURL;
}

VCD_OMAF_END;
