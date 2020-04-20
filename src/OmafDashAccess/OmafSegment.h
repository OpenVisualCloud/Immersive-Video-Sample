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

 *
 */


//!
//! \file:   OmafSegment.h
//! \brief:
//! \detail:
//! Created on May 24, 2019, 11:07 AM
//!

#ifndef OMAFSEGMENT_H
#define OMAFSEGMENT_H

#include "general.h"
#include "OmafDashDownload/OmafDownloaderObserver.h"
#include "OmafDashParser/SegmentElement.h"

#include <fstream>

VCD_OMAF_BEGIN

typedef enum{
    SegUnknown = 0,
    SegReady,
    SegDownloading,
    SegDownloaded,
    SegAborted,
    SegEOS,
}SEGSTATUS;

class OmafSegment : public OmafDownloaderObserver {
public:
    //!
    //! \brief  construct
    //!
    OmafSegment();

    //!
    //! \brief  construct with SegmentElement
    //!
    OmafSegment( SegmentElement* pSeg, int segCnt, bool bInitSegment = false, bool reEnabled = false );

    //!
    //! \brief  de-construct
    //!
    virtual ~OmafSegment();

public:

    //!
    //!  \brief basic Get/Set methods for the properties.
    //!
    std::string GetSegmentCacheFile()             { return mCacheFile;       };

    void        SetSegmentCacheFile(std::string cacheFileName)
    {
        mCacheFile = cacheFileName;
    };

    SEGSTATUS   GetSegStatus()                    { return mStatus;          };
    void        SetSegStatus(SEGSTATUS status)    { mStatus = status;        };

    void        SetSegment( SegmentElement* pSeg )      { mSeg = pSeg;             };
    SegmentElement*   GetSegment()                      { return mSeg;             };

    bool        bInitSegment()                    { return mInitSegment;     };
    void        SetInitSegment(bool bInit)        { mInitSegment = bInit;    };

    //!
    //!  \brief Informs the OmafDownloaderObserver object that the download rate has changed.
    //!  @param      bytesDownloaded     the number of downloaded bytes
    //!
    virtual void DownloadDataNotify  (uint64_t bytesDownloaded);

    //!
    //!  \brief Informs the OmafDownloaderObserver object that the download state has changed.
    //!  @param state               the download state
    //!
    virtual void DownloadStatusNotify (DownloaderStatus state);

    //!
    //!  \brief Basic operation to read / write the segment data.
    //!
    int     Open( );
    int     Open( SegmentElement* pSeg );
    int     Read(uint8_t *data, size_t len);
    int     Peek(uint8_t *data, size_t len);
    int     Peek(uint8_t *data, size_t len, size_t offset);
    int     Close();

    void     SetSegID( uint32_t id )     { mSegID = id;        };
    uint32_t GetSegID()                  { return mSegID;      };
    void     SetInitSegID( uint32_t id ) { mInitSegID = id;    };
    uint32_t GetInitSegID()              { return mInitSegID;  };
    void     SetSegStored()              { mStoreFile = true;  };

    bool    IsReEnabled(){return mReEnabled;};
    int     GetSegCount(){return mSegCnt;};
    void     SetSegSize(uint64_t segSize) { mSegSize = segSize; };
    uint64_t GetSegSize() { return mSegSize; };

private:
    //!
    //!  \brief save the memory data to file.
    //!
    int SaveToFile();

    //!
    //!  \brief start downloading process.
    //!
    int StartDownload();

    //!
    //!  \brief waiting for all data downloaded.
    //!
    int WaitComplete();

private:
    SegmentElement*                   mSeg;               //<! SegmentElement
    bool                              mStoreFile;         //<! flag to indicate whether the segment should be stored in disk
    std::string                       mCacheFile;         //<! the file name for downloaded segment file
    SEGSTATUS                         mStatus;            //<! status of the segment
    std::ofstream                     mFileStream;        //<! file handle for writing
    pthread_mutex_t                   mMutex;             //<! for synchronization
    pthread_cond_t                    mCond;              //<! for synchronization
    uint64_t                          mSegSize;           //<! the total size of data downloaded for this segment
    bool                              mInitSegment;       //<! flag to indicate whether this segment is initialize MP4
    uint32_t                          mSegID;             //<! the Segment ID used for segment reading
    uint32_t                          mInitSegID;         //<! the init Segement ID relative to this segment
    uint8_t                           *mData;             //<! memory for saving segment data
    bool                              mReEnabled;         //<! flag to indicate whether the segment is re-enabled
    int                               mSegCnt;            //<! the count for this segment
};

VCD_OMAF_END

#endif /* MEDIASEGMENT_H */

