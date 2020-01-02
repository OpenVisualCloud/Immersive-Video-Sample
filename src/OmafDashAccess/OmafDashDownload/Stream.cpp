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
//! \file:   Stream.cpp
//! \brief:  stream class for storing downloaded sub-streams
//!

#include "Stream.h"

VCD_OMAF_BEGIN

Stream::Stream()
{
    m_eos = false;
    m_totalLength = 0;
}

Stream::~Stream()
{
    if(m_listDownloadedStreams.size())
    {
        for(auto ds : m_listDownloadedStreams)
        {
            SAFE_DELETE(ds);
        }
        m_listDownloadedStreams.clear();
    }
}

ODStatus Stream::AddSubStream(char* streamData, uint64_t streamLen)
{
    StreamInfo *sInfo = new StreamInfo();
    sInfo->data = streamData;
    sInfo->length = streamLen;

    m_lock.lock();
    m_totalLength += streamLen;
    m_listDownloadedStreams.push_back(sInfo);
    m_lock.unlock();

    // notify other threads
    m_cv.notify_all();
    return OD_STATUS_SUCCESS;
}

ODStatus Stream::GetStream(char* streamData, uint64_t streamDataLen)
{
    CheckNullPtr_PrintLog_ReturnStatus(streamData, "the data pointer for getting output stream is null!", ERROR, OD_STATUS_INVALID);

    while(!GetDownloadedListSize() && !IsEOS())
    {
        mutex mtx;
        unique_lock<mutex> lck(mtx);

        // wait for time out or is notified
        m_cv.wait_for(lck, chrono::milliseconds(2));
    }

    m_lock.lock();

    uint64_t gotSize = 0, gotCnt = 0;
    for(auto stream: m_listDownloadedStreams)
    {
        if(gotSize >= streamDataLen)
            break;

        uint64_t copysize = 0;
        if((streamDataLen - gotSize) >= stream->length)
        {
            copysize = stream->length;
            memcpy(streamData + gotSize, stream->data, copysize);
            gotCnt++;
        }
        else
        {
            copysize = (streamDataLen - gotSize);
            memcpy(streamData + gotSize, stream->data, copysize);

            // remove the copied part of list element
            auto leftSize = stream->length - copysize;
            char* newData = new char(leftSize);
            memcpy(newData, stream->data, leftSize);
            SAFE_DELETE(stream->data);
            stream->data = newData;
            stream->length = leftSize;
        }

        gotSize += copysize;
    }

    while(gotCnt)
    {
        auto downStream = m_listDownloadedStreams.front();
        m_listDownloadedStreams.pop_front();
        SAFE_DELETE(downStream);
        gotCnt--;
    }

    m_totalLength -= streamDataLen;

    m_lock.unlock();

    return OD_STATUS_SUCCESS;
}

ODStatus Stream::PeekStream(char* streamData, uint64_t streamDataLen)
{
    CheckNullPtr_PrintLog_ReturnStatus(streamData, "The data pointer for getting output stream is null!", ERROR, OD_STATUS_INVALID);

    while(!GetDownloadedListSize() && !IsEOS())
    {
        mutex mtx;
        unique_lock<mutex> lck(mtx);

        // wait for time out or is notified
        m_cv.wait_for(lck, chrono::milliseconds(2));
    }

    m_lock.lock();

    uint64_t gotSize = 0;
    for(auto stream: m_listDownloadedStreams)
    {
        if(gotSize >= streamDataLen)
            break;

        uint64_t copysize = (streamDataLen - gotSize) >= stream->length ? stream->length : (streamDataLen - gotSize);

        memcpy(streamData + gotSize, stream->data, copysize);
        gotSize += copysize;
    }

    m_lock.unlock();

    return OD_STATUS_SUCCESS;
}

ODStatus Stream::PeekStream(char* streamData, uint64_t streamDataLen, size_t offset)
{
    CheckNullPtr_PrintLog_ReturnStatus(streamData, "The data pointer for getting output stream is null!", ERROR, OD_STATUS_INVALID);

    while(!GetDownloadedListSize() && !IsEOS())
    {
        mutex mtx;
        unique_lock<mutex> lck(mtx);

        // wait for time out or is notified
        m_cv.wait_for(lck, chrono::milliseconds(2));
    }

    m_lock.lock();

    // find the block that offset refers
    auto it = m_listDownloadedStreams.begin();
    while(it != m_listDownloadedStreams.end())
    {
        if(offset - (*it)->length <= 0) break;
        offset -= (*it)->length;

        it++;
    }

    uint64_t gotSize = 0;
    if (it == m_listDownloadedStreams.end())
    {
        return OD_STATUS_OPERATION_FAILED;
    }
    // copy the left data in offset block
    memcpy(streamData, (*it)->data + offset, (*it)->length - offset);
    gotSize += (*it)->length - offset;
    it++;

    for(; it!= m_listDownloadedStreams.end(); it++)
    {
        auto stream = (*it);
        if(gotSize >= streamDataLen)
            break;

        uint64_t copysize = (streamDataLen - gotSize) >= stream->length ? stream->length : (streamDataLen - gotSize);

        memcpy(streamData + gotSize, stream->data, copysize);
        gotSize += copysize;
    }

    m_lock.unlock();

    return OD_STATUS_SUCCESS;
}

ODStatus Stream::ReachedEOS()
{
    m_eos = true;

    m_cv.notify_all();

    return OD_STATUS_SUCCESS;
}

VCD_OMAF_END
