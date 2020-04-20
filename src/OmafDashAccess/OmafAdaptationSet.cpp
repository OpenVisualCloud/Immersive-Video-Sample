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
 * File:   OmafAdaptationSet.cpp
 * Author: media
 *
 * Created on May 24, 2019, 10:19 AM
 */

#include "OmafAdaptationSet.h"
#include <sys/time.h>
//#include <sys/timeb.h>

VCD_OMAF_BEGIN

using namespace VCD::OMAF;

OmafAdaptationSet::OmafAdaptationSet()
{
    mAdaptationSet     = NULL;
    mRepresentation    = NULL;
    mInitSegment       = NULL;
    mSRD               = NULL;
    mPreselID          = NULL;
    mTwoDQuality       = NULL;
    mSrqr              = NULL;
    mCC                = NULL;
    mEnable            = true;
    m_bMain            = false;
    mActiveSegNum      = 1;
    mSegNum            = 1;
    mReEnable          = false;
    mPF                = PF_UNKNOWN;
    mSegmentDuration   = 0;
    mTrackNumber       = 0;
    mStartNumber       = 0;
    mID                = 0;
    mType              = MediaType_NONE;
    mFpt               = FP_UNKNOWN;
    mRwpkType          = RWPK_UNKNOWN;
    memset(&mVideoInfo, 0, sizeof(VideoInfo));
    memset(&mAudioInfo, 0, sizeof(AudioInfo));
    pthread_mutex_init(&mMutex, NULL);
}

OmafAdaptationSet::~OmafAdaptationSet()
{
    this->ClearSegList();
    pthread_mutex_destroy(&mMutex);

    SAFE_DELETE(mInitSegment);

    if(mBaseURL.size())
    {
        mBaseURL.clear();
    }
}

OmafAdaptationSet::OmafAdaptationSet( AdaptationSetElement* pAdaptationSet ):OmafAdaptationSet()
{
    Initialize(pAdaptationSet);
}

int OmafAdaptationSet::Initialize(AdaptationSetElement* pAdaptationSet)
{
    mAdaptationSet = pAdaptationSet;

    SelectRepresentation( );

    // if( ERROR_NOT_FOUND == OmafProperty::Get2DQualityRanking(mAdaptationSet->GetAdditionalSubNodes(), &mTwoDQuality) ){
    //     OmafProperty::Get2DQualityRanking(mRepresentation->GetAdditionalSubNodes(), &mTwoDQuality);
    // }

    // OmafProperty::GetFramePackingType(mAdaptationSet->GetAdditionalSubNodes(), this->mFpt);
    // if(FP_UNKNOWN==mFpt)
    //     OmafProperty::GetFramePackingType(mRepresentation->GetAdditionalSubNodes(), this->mFpt);

    mSrqr = mAdaptationSet->GetSphereQuality();
    mSRD = mAdaptationSet->GetSRD();
    mPreselID = mAdaptationSet->GetPreselection();
    mRwpkType = mAdaptationSet->GetRwpkType();
    mPF = mAdaptationSet->GetProjectionFormat();
    mCC = mAdaptationSet->GetContentCoverage();
    mID = stoi(mAdaptationSet->GetId());


    for(auto it = mRepresentation->GetDependencyIDs().begin(); it != mRepresentation->GetDependencyIDs().end(); it++ ){
        std::string id = *it;
        mDependIDs.push_back(atoi(id.c_str()));
    }

    SegmentElement* segment = mRepresentation->GetSegment();

    if(NULL != segment){
        mStartNumber       = segment->GetStartNumber();
        mSegmentDuration = segment->GetDuration() / segment->GetTimescale();
    }

    // mAudioInfo.sample_rate = parse_int( mRepresentation->GetAudioSamplingRate().c_str() );
    // mAudioInfo.channels    = mRepresentation->GetAudioChannelConfiguration().size();
    // mAudioInfo.channel_bytes = 2;

    mVideoInfo.bit_rate       = mRepresentation->GetBandwidth();
    mVideoInfo.height         = mRepresentation->GetHeight();
    mVideoInfo.width          = mRepresentation->GetWidth();
    mVideoInfo.frame_Rate.num = atoi(GetSubstr(mRepresentation->GetFrameRate(), '/', true).c_str());
    mVideoInfo.frame_Rate.den = atoi(GetSubstr(mRepresentation->GetFrameRate(), '/', false).c_str());
    mVideoInfo.sar.num        = atoi(GetSubstr(mRepresentation->GetSar(), ':', true).c_str());
    mVideoInfo.sar.den        = atoi(GetSubstr(mRepresentation->GetSar(), ':', false).c_str());

    mMimeType  = mAdaptationSet->GetMimeType();
    mCodec = mAdaptationSet->GetCodecs();

    mType   = MediaType_Video;
    if( GetSubstr(mRepresentation->GetMimeType(), '/', true) == "audio")
        mType = MediaType_Audio;

    JudgeMainAdaptationSet();

    return ERROR_NONE;
}

int  OmafAdaptationSet::SelectRepresentation( )
{
    std::vector<RepresentationElement*> pRep = mAdaptationSet->GetRepresentations();

    ///FIX; so far choose the first rep in the Representation list
    this->mRepresentation = pRep[0];

    return ERROR_NONE;
}

void OmafAdaptationSet::JudgeMainAdaptationSet()
{
    if(NULL == mAdaptationSet || !mSRD) return ;

    if( mType == MediaType_Video ){
        if( this->mSRD->get_H() == 0 && mSRD->get_Y() == 0){
            m_bMain = true;
            return ;
        }
    }else{
        m_bMain = true;
        return ;
    }
    m_bMain = false;
}

int OmafAdaptationSet::LoadLocalInitSegment()
{
    int ret = ERROR_NONE;

    for (auto it = mBaseURL.begin(); it != mBaseURL.end(); it++)
    {
        BaseUrlElement *baseURL = *it;
        std::string url = baseURL->GetPath();
    }

    SegmentElement* seg = mRepresentation->GetSegment();

    if( NULL == seg ){
        LOG(ERROR) << "Create Initial SegmentElement for AdaptationSet:" << this->mID
                   << " failed" << endl;
        return ERROR_NULL_PTR;
    }

    auto repID = mRepresentation->GetId();

    mInitSegment = new OmafSegment(seg, mSegNum, true);

    if(NULL == mInitSegment ) {
        LOG(ERROR) << "New Initial OmafSegment for AdaptationSet:" << this->mID
                   << " failed"  << endl;
        return ERROR_NULL_PTR;
    }

    LOG(INFO)<<"Load Initial OmafSegment for AdaptationSet "<<this->mID<<endl;

    return ret;
}

int OmafAdaptationSet::LoadLocalSegment()
{
    int ret = ERROR_NONE;

    if(!mEnable){
        mActiveSegNum++;
        return ret;
    }

    SegmentElement* seg = mRepresentation->GetSegment();

    if( NULL == seg ){
        LOG(ERROR) << "Create Initial SegmentElement for AdaptationSet:" << this->mID
                   << " failed" << endl;
        return ERROR_NULL_PTR;
    }

    auto repID = mRepresentation->GetId();

    OmafSegment* pSegment = new OmafSegment(seg, mSegNum, false);

    if(NULL == pSegment ) {
        LOG(ERROR) << "Create OmafSegment for AdaptationSet: " << this->mID
                   <<" Number: " << mActiveSegNum
                   << " failed" << endl;

        return ERROR_NULL_PTR;
    }

    pSegment->SetInitSegID(this->mInitSegment->GetInitSegID());

    LOG(INFO)<<"Load OmafSegment for AdaptationSet "<<this->mID<<endl;

    mSegments.push_back(pSegment);

    mActiveSegNum++;

    return ret;
}

int OmafAdaptationSet::LoadAssignedInitSegment(std::string assignedSegment)
{
    int ret = ERROR_NONE;

    ret = LoadLocalInitSegment();
    if (ret)
        return ret;

    OmafSegment *initSeg = GetInitSegment();
    if (!initSeg)
    {
        LOG(ERROR) << "Failed to get local init segment" << endl;
        return ERROR_NOT_FOUND;
    }

    initSeg->SetSegmentCacheFile(assignedSegment);
    initSeg->SetSegStored();

    return ret;
}

OmafSegment* OmafAdaptationSet::LoadAssignedSegment(std::string assignedSegment)
{
    int ret = ERROR_NONE;

    ret = LoadLocalSegment();
    if (ret)
    {
        LOG(ERROR) << "Failed to load local segment " << endl;
        return NULL;
    }

    OmafSegment *newSeg = GetLocalNextSegment();
    if (!newSeg)
    {
        LOG(ERROR) << "Failed to get local segment" << endl;
        return NULL;
    }

    OmafSegment *initSeg = GetInitSegment();
    if (!initSeg)
    {
        LOG(ERROR) << "Failed to get local init segment" << endl;
        return NULL;
    }

    newSeg->SetSegmentCacheFile(assignedSegment);
    newSeg->SetSegStored();

    return newSeg;
}

/////Download relative methods

int OmafAdaptationSet::DownloadInitializeSegment()
{
    int ret = ERROR_NONE;

    SegmentElement* seg = mRepresentation->GetSegment();

    if( NULL == seg ){
        LOG(ERROR) << "Create Initial SegmentElement for AdaptationSet:" << this->mID
                   << " failed" << endl;
        return ERROR_NULL_PTR;
    }

    auto repID = mRepresentation->GetId();
    ret = seg->InitDownload(mBaseURL, repID, 0);

    if( ERROR_NONE != ret ){
        SAFE_DELETE(seg);
        LOG(ERROR) << "Fail to Init OmafSegment Download for AdaptationSet:" << this->mID
                   << endl;
    }

    mInitSegment = new OmafSegment(seg, mSegNum, true);

    if(NULL == mInitSegment ) {
        LOG(ERROR) << "New Initial OmafSegment for AdaptationSet:" << this->mID
                   << " failed"  << endl;
        return ERROR_NULL_PTR;
    }

    ret = mInitSegment->Open();

    if( ERROR_NONE != ret ){
        SAFE_DELETE(mInitSegment);
        LOG(ERROR) << "Fail to Download Initial OmafSegment for AdaptationSet:" << this->mID
                   << endl;
    }

    LOG(INFO)<<"Download Initial OmafSegment for AdaptationSet "<<this->mID<<endl;

    return ret;
}

int OmafAdaptationSet::DownloadSegment( )
{
    int ret = ERROR_NONE;

    if(!mEnable){
        mActiveSegNum++;
        mSegNum++;
        return ret;
    }

    SegmentElement* seg = mRepresentation->GetSegment();

    if( NULL == seg ){
        LOG(ERROR) << "Create Initial SegmentElement for AdaptationSet:" << this->mID
                   << " failed" << endl;
        return ERROR_NULL_PTR;
    }

    auto repID = mRepresentation->GetId();
    ret = seg->InitDownload(mBaseURL, repID, mActiveSegNum);

    if( ERROR_NONE != ret ){
        SAFE_DELETE(seg);
        LOG(ERROR) << "Fail to Init OmafSegment Download for AdaptationSet:" << this->mID
                   << endl;
    }

    OmafSegment* pSegment = new OmafSegment(seg, mSegNum, false, mReEnable);

    // reset the re-enable flag, since it will be updated with different viewport
    if(mReEnable) mReEnable = false;

    if(NULL == pSegment ) {
        LOG(ERROR) << "Create OmafSegment for AdaptationSet: " << this->mID
                   <<" Number: " << mActiveSegNum
                   << " failed" << endl;

        return ERROR_NULL_PTR;
    }

    pSegment->SetInitSegID(this->mInitSegment->GetInitSegID());

    ret = pSegment->Open();

    if( ERROR_NONE != ret ){
        SAFE_DELETE(pSegment);
        LOG(ERROR) << "Fail to Download OmafSegment for AdaptationSet:" << this->mID
                   << endl;
    }

    LOG(INFO)<<"Download OmafSegment for AdaptationSet "<<this->mID<<endl;

    pthread_mutex_lock(&mMutex);
    // NOTE: won't record segments in adaption set since GetNextSegment() not be called
    //       , and this will lead to memory growth.
    //mSegments.push_back(pSegment);
    pthread_mutex_unlock(&mMutex);

    mActiveSegNum++;
    mSegNum++;

    return ret;
}

/////read relative methods
int OmafAdaptationSet::UpdateStartNumberByTime(uint64_t nAvailableStartTime)
{
    time_t gTime;
    struct tm *t;
    struct timeval now;
    struct timezone tz;
    gettimeofday(&now, &tz);
    //struct timeb timeBuffer;
    //ftime(&timeBuffer);
    //now.tv_sec = (long)(timeBuffer.time);
    //now.tv_usec = timeBuffer.millitm * 1000;
    gTime = now.tv_sec;
    t = gmtime(&gTime);

    uint64_t current = timegm(t);
    current *= 1000;

    if (current < nAvailableStartTime)
    {
        LOG(ERROR) << "Unreasonable current time " << current
                   << "which is earlier than available time " << nAvailableStartTime;

        return -1;
    }
    mActiveSegNum = (current - nAvailableStartTime) / (mSegmentDuration * 1000) + mStartNumber;

    LOG(INFO) << "current " << current << " and available time " << nAvailableStartTime << " Start segment index " << mActiveSegNum << endl;
    return mActiveSegNum;
}

OmafSegment* OmafAdaptationSet::GetNextSegment()
{
    OmafSegment* seg = NULL;

    pthread_mutex_lock(&mMutex);
    seg = (OmafSegment*) mSegments.front();
    mSegments.pop_front();
    pthread_mutex_unlock(&mMutex);

    return seg;
}

OmafSegment* OmafAdaptationSet::GetLocalNextSegment()
{
    OmafSegment* seg = NULL;

    seg = (OmafSegment*) mSegments.front();
    mSegments.pop_front();

    return seg;
}

void OmafAdaptationSet::ClearSegList()
{
    std::list<OmafSegment*>::iterator it;
    pthread_mutex_lock(&mMutex);
    for(auto it = mSegments.begin(); it != mSegments.end(); it++){
        delete *it;
        *it = NULL;
    }
    mSegments.clear();
    pthread_mutex_unlock(&mMutex);
}

int OmafAdaptationSet::SeekTo( int seg_num )
{
    mActiveSegNum = seg_num;
    ClearSegList();
    return ERROR_NONE;
}

VCD_OMAF_END
