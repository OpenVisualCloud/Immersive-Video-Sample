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

/*
 * File:   OmafSegment.cpp
 * Author: media
 *
 * Created on May 24, 2019, 11:07 AM
 */

#include <fstream>

#include "OmafSegment.h"
#include "DownloadManager.h"
#include "OmafReaderManager.h"

VCD_OMAF_BEGIN

OmafSegment::OmafSegment()
{
    pthread_mutex_init(&mMutex, NULL);
    pthread_cond_init(&mCond, NULL);
    mSeg         = NULL;
    mStoreFile   = true;
    mCacheFile   = "";
    mStatus      = SegUnknown;
    mSegSize     = 0;
    mInitSegment = false;
    mData        = NULL;
    mReEnabled   = false;
    mSegCnt      = 0;
    mInitSegID   = 0;
    mSegID       = 0;
    mQualityRanking = HIGHEST_QUALITY_RANKING;
}

OmafSegment::~OmafSegment()
{
    pthread_mutex_destroy( &mMutex );
    pthread_cond_destroy( &mCond );

    //SAFE_DELETE(mSeg);
    mSeg->StopDownloadSegment((OmafDownloaderObserver*) this);

    DOWNLOADMANAGER::GetInstance()->DeleteCacheFile(mCacheFile);
}

OmafSegment::OmafSegment(SegmentElement* pSeg, int segCnt, bool bInitSegment, bool reEnabled):OmafSegment()
{
    pthread_mutex_init(&mMutex, NULL);
    pthread_cond_init(&mCond, NULL);
    mSeg         = pSeg;
    mStoreFile   = false;
    mCacheFile   = "";
    mStatus      = SegUnknown;
    mSegSize     = 0;
    mInitSegment = bInitSegment;
    mReEnabled   = reEnabled;
    mSegCnt      = segCnt;
    mInitSegID   = 0;
    mSegID       = 0;
    mQualityRanking = HIGHEST_QUALITY_RANKING;
}

int OmafSegment::StartDownload()
{
    if(NULL == mSeg) return ERROR_NULL_PTR;

    mStoreFile = DOWNLOADMANAGER::GetInstance()->UseCache();

    mSegSize     = 0;

    mStatus = SegReady;

    mSeg->StartDownloadSegment((OmafDownloaderObserver*) this);

    return ERROR_NONE;
}

int OmafSegment::WaitComplete()
{
    if( mStatus == SegDownloaded ) return ERROR_NONE;

    int64_t waitTime = 0;

    /*pthread_mutex_lock(&m_mutex);
    pthread_cond_wait(&m_cond, &m_mutex);

    pthread_mutex_unlock(&m_mutex);*/
    // exit the waiting if segment downloaded or wait time is more than 10 mins
    while(mStatus != SegDownloaded && waitTime < 60000){
        ::usleep(10000);
        waitTime++;
    }

    return ERROR_NONE;
}

int OmafSegment::Open( )
{
    return StartDownload();
}

int OmafSegment::Open( SegmentElement* pSeg )
{
    mSeg = pSeg;

    return Open();
}

int OmafSegment::Read(uint8_t *data, size_t len)
{
    if(NULL == mSeg) return ERROR_NULL_PTR;

    if(mStatus != SegDownloaded) WaitComplete();

    return mSeg->Read(data, len);
}

int OmafSegment::Peek(uint8_t *data, size_t len)
{
    if(NULL == mSeg) return ERROR_NULL_PTR;

    if(mStatus != SegDownloaded) WaitComplete();

    return mSeg->Peek(data, len);
}

int OmafSegment::Peek(uint8_t *data, size_t len, size_t offset)
{
    if(NULL == mSeg) return ERROR_NULL_PTR;

    if(mStatus != SegReady) WaitComplete();

    return mSeg->Peek(data, len, offset);
}

int OmafSegment::Close()
{
    if(NULL == mSeg) return ERROR_NULL_PTR;

    if(mStatus != SegDownloading) mSeg->StopDownloadSegment((OmafDownloaderObserver*) this);

    //SAFE_DELETE( mSeg );

    return ERROR_NONE;
}

int OmafSegment::SaveToFile()
{
    mCacheFile = DOWNLOADMANAGER::GetInstance()->GetCacheFolder() + "/" + DOWNLOADMANAGER::GetInstance()->AssignCacheFileName();
    mFileStream.open(mCacheFile, ios::out|ios::binary);

    mData = (uint8_t*)malloc(mSegSize);
    Read( mData, mSegSize );

    mFileStream.write( (char *)mData, mSegSize);

    mFileStream.close();

    if (mData)
    {
        free(mData);
        mData = NULL;
    }

    LOG(INFO)<<"close saved cache "<<mCacheFile<<", size= "<<mSegSize<<std::endl;
    return ERROR_NONE;
}

void OmafSegment::DownloadDataNotify(uint64_t bytesDownloaded)
{
    // every time OnDownloadRateChanged called, the input bytesDownloaded
    // is the total bytes number includes previous downloaded bytes
    mSegSize = bytesDownloaded;
}

void OmafSegment::DownloadStatusNotify(DownloaderStatus state)
{
    switch(state){
        case DOWNLOADED:
            mStatus = SegDownloaded;
            if( mStoreFile ) SaveToFile();

            if(this->mInitSegment){
                READERMANAGER::GetInstance()->AddInitSegment(this, mInitSegID);
            }else{
                READERMANAGER::GetInstance()->AddSegment(this, mInitSegID, mSegID);
            }

            break;
        case NOT_START:
            mStatus = SegReady;
            break;
        case DOWNLOADING:
            mStatus = SegDownloading;
            break;
        case STOPPING:
        case STOPPED:
            mStatus = SegAborted;
            break;
        default:
            mStatus = SegUnknown;
            break;
    }
}

VCD_OMAF_END
