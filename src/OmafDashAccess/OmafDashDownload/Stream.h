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
//! \file:   Stream.h
//! \brief:  stream class for storing downloaded sub-streams
//!

#ifndef STREAM_H
#define STREAM_H

#include "../OmafDashParser/Common.h"

VCD_USE_VRVIDEO;

VCD_OMAF_BEGIN

//!
//! \class  StreamInfo
//! \brief  Stream Information, including data and data size
//!
class StreamInfo
{
public:

    //!
    //! \brief Constructor
    //!
    StreamInfo()
    {
        data   = NULL;
        length = 0;
    }

    //!
    //! \brief Constructor with parameter
    //!
    StreamInfo(char* d, uint64_t l):StreamInfo()
    {
        data = d;
        length = l;
    }

    //!
    //! \brief Destructor
    //!
    ~StreamInfo()
    {
        SAFE_DELETE(data);
        length = 0;
    }

    char*            data;   //<! stream data
    uint64_t         length; //<! length of data
};


//!
//! \class  Stream
//! \brief  Stream class, which will store sub-streams
//!
class Stream: public ThreadLock
{
public:

    //!
    //! \brief Constructor
    //!
    Stream();

    //!
    //! \brief Destructor
    //!
    ~Stream();

    //!
    //! \brief    Add sub-stream
    //!
    //! \param    [in] streamData
    //!           sub-stream data
    //! \param    [in] streamLen
    //!           sub-stream length
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus AddSubStream(char* streamData, uint64_t streamLen);

    //!
    //! \brief    Get given size sub-stream
    //!
    //! \param    [in] streamData
    //!           sub-stream data
    //! \param    [in] streamDataLen
    //!           sub-stream length
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus GetStream(char* streamData, uint64_t streamDataLen);

    //!
    //! \brief    Peek given size sub-stream
    //!
    //! \param    [in] streamData
    //!           sub-stream data
    //! \param    [in] streamDataLen
    //!           sub-stream length
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus PeekStream(char* streamData, uint64_t streamDataLen);

    //!
    //! \brief    Peek given size sub-stream with offset
    //!
    //! \param    [in] streamData
    //!           sub-stream data
    //! \param    [in] streamDataLen
    //!           sub-stream length
    //! \param    [in] offset
    //!           stream offset that read should start
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus PeekStream(char* streamData, uint64_t streamDataLen, size_t offset);

    //!
    //! \brief    Mark this stream reached EOS
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus ReachedEOS();

    //!
    //! \brief    Get if the strean reached EOS
    //!
    //! \return   bool
    //!           true if EOS reached, else false
    //!
    bool IsEOS(){return m_eos;}

    //!
    //! \brief    Get total stream length
    //!
    //! \return   uint64_t
    //!           total stream length
    //!
    uint64_t GetTotalStreamLength(){return m_totalLength;}

private:

    //!
    //! \brief    get number of sub-streams
    //!
    //! \return   size_t
    //!           number of sub-streams
    //!
    size_t GetDownloadedListSize()
    {
        m_lock.lock();
        auto listSize = m_listDownloadedStreams.size();
        m_lock.unlock();

        return listSize;
    }

    list<StreamInfo*>       m_listDownloadedStreams;    //!< list stores all downloaded sub-streams
    ThreadLock              m_lock;                     //!< for downloaded streams synchronize
    bool                    m_eos;                      //!< flag for end of stream
    condition_variable      m_cv;                       //!< condition variable for streams
    uint64_t                m_totalLength;              //!< the total length of stream
};

VCD_OMAF_END;

#endif //STREAM_H