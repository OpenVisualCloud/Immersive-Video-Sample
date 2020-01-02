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

#include "OmafMediaStream.h"

VCD_OMAF_BEGIN

OmafMediaStream::OmafMediaStream()
{
    //mCurrentExtractor      = NULL;
    mMainAdaptationSet     = NULL;
    mExtratorAdaptationSet = NULL;
    m_pStreamInfo          = NULL;
    m_bEOS                 = false;
    mStreamID              = 0;
    pthread_mutex_init(&mMutex, NULL);
    pthread_mutex_init(&mCurrentMutex, NULL);
}

OmafMediaStream::~OmafMediaStream()
{
    SAFE_FREE(m_pStreamInfo);
    SAFE_FREE(mMainAdaptationSet);
    if(mMediaAdaptationSet.size())
    {
        for(auto &it: mMediaAdaptationSet)
        {
            SAFE_DELETE(it.second);
            mMediaAdaptationSet.erase(it.first);
        }
        mMediaAdaptationSet.clear();
    }
    if(mExtractors.size())
    {
        for(auto &it: mExtractors)
        {
            SAFE_DELETE(it.second);
            mExtractors.erase(it.first);
        }
        mExtractors.clear();
    }
    pthread_mutex_destroy( &mMutex );
    pthread_mutex_destroy( &mCurrentMutex );
}

int OmafMediaStream::AddExtractor(OmafExtractor* pAS)
{
    if( NULL != pAS ) mExtractors[pAS->GetID()] = pAS;

    return ERROR_NONE;
}

int OmafMediaStream::AddAdaptationSet(OmafAdaptationSet* pAS)
{
    if( NULL != pAS ) mMediaAdaptationSet[pAS->GetID()] = pAS;
    return ERROR_NONE;
}

int OmafMediaStream::InitStream(std::string type)
{
    if(NULL== m_pStreamInfo )
        m_pStreamInfo = (DashStreamInfo*)malloc(sizeof(DashStreamInfo));

    if (NULL == m_pStreamInfo)
    {
        return ERROR_NULL_PTR;
    }

    if( type == "video")
    {
        m_pStreamInfo->stream_type   = MediaType_Video;
    }
    else if( type == "audio")
    {
        m_pStreamInfo->stream_type = MediaType_Audio;
    }
    else
    {
        return ERROR_INVALID;
    }

    UpdateStreamInfo();

    SetupExtratorDependency();

    return ERROR_NONE;
}

void OmafMediaStream::UpdateStreamInfo()
{
    if(!mMediaAdaptationSet.size()) return;

    if(NULL != mMainAdaptationSet && NULL != mExtratorAdaptationSet){
        VideoInfo vi = mMainAdaptationSet->GetVideoInfo();
        AudioInfo ai = mMainAdaptationSet->GetAudioInfo();

        m_pStreamInfo->bit_rate      = vi.bit_rate;

        m_pStreamInfo->framerate_den = vi.frame_Rate.den;
        m_pStreamInfo->framerate_num = vi.frame_Rate.num;
        m_pStreamInfo->height        = mExtratorAdaptationSet->GetVideoInfo().height;
        m_pStreamInfo->width         = mExtratorAdaptationSet->GetVideoInfo().width;
        m_pStreamInfo->mime_type     = mMainAdaptationSet->GetMimeType().c_str();
        m_pStreamInfo->codec         = mMainAdaptationSet->GetCodec()[0].c_str();
        m_pStreamInfo->mFpt          = (int32_t)mMainAdaptationSet->GetFramePackingType();
        m_pStreamInfo->mProjFormat   = (int32_t)mMainAdaptationSet->GetProjectionFormat();
        m_pStreamInfo->segmentDuration = mMainAdaptationSet->GetSegmentDuration();

        m_pStreamInfo->channel_bytes = ai.channel_bytes;
        m_pStreamInfo->channels      = ai.channels;
        m_pStreamInfo->sample_rate   = ai.sample_rate;

        int sourceNumber = mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos.size();
        m_pStreamInfo->source_number = sourceNumber;
        m_pStreamInfo->source_resolution = new SourceResolution[sourceNumber];
        for (int i=0;i<sourceNumber;i++)
        {
            m_pStreamInfo->source_resolution[i].qualityRanking = mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos[i].quality_ranking;
            m_pStreamInfo->source_resolution[i].width = mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos[i].orig_width;
            m_pStreamInfo->source_resolution[i].height = mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos[i].orig_height;
        }
        std::map<int, OmafAdaptationSet*>::iterator itAS;
        for (itAS = mMediaAdaptationSet.begin();
            itAS != mMediaAdaptationSet.end(); itAS++)
        {
            if (mMainAdaptationSet == (itAS->second))
                break;
        }
        if (itAS != mMediaAdaptationSet.end())
        {
            mMediaAdaptationSet.erase(itAS);
        }
    }

    uint32_t rowNum = 0, colNum = 0;
    for(auto &it:mMediaAdaptationSet)
    {
        OmafAdaptationSet* as = it.second;
        uint32_t qr = as->GetRepresentationQualityRanking();
        // only calculate tile segmentation of the stream with highest resolution
        if(qr == 1)
        {
            OmafSrd* srd = as->GetSRD();
            if(srd->get_X() == 0)
                rowNum++;
            if(srd->get_Y() == 0)
                colNum++;
        }
    }
    m_pStreamInfo->tileRowNum = rowNum;
    m_pStreamInfo->tileColNum = colNum;

}

void OmafMediaStream::SetupExtratorDependency()
{
    for(auto extrator_it = mExtractors.begin();
             extrator_it != mExtractors.end();
             extrator_it++ ){
        OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
        for(auto it = mMediaAdaptationSet.begin();
                 it != mMediaAdaptationSet.end();
                 it++ ){
            OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
            extractor->AddDependAS(pAS);
        }
    }
}

int OmafMediaStream::UpdateStartNumber(uint64_t nAvailableStartTime)
{
    int ret = ERROR_NONE;
    pthread_mutex_lock(&mMutex);
    for(auto it = mMediaAdaptationSet.begin();
             it != mMediaAdaptationSet.end();
             it++ ){
        OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
        pAS->UpdateStartNumberByTime(nAvailableStartTime);
    }

    for(auto extrator_it = mExtractors.begin();
             extrator_it != mExtractors.end();
             extrator_it++ ){
        OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
        extractor->UpdateStartNumberByTime(nAvailableStartTime);
    }
    pthread_mutex_unlock(&mMutex);
    return ret;
}
/*
int OmafMediaStream::LoadLocalInitSegment()
{
    int ret = ERROR_NONE;
    pthread_mutex_lock(&mMutex);
    for(auto it = mMediaAdaptationSet.begin();
             it != mMediaAdaptationSet.end();
             it++ ){
        OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
        //pAS->DownloadInitializeSegment();
        pAS->LoadLocalInitSegment();
    }

    for(auto extrator_it = mExtractors.begin();
             extrator_it != mExtractors.end();
             extrator_it++ ){
        OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
        //extractor->DownloadInitializeSegment();
        extractor->LoadLocalInitSegment();
    }
    pthread_mutex_unlock(&mMutex);
    return ret;
}
*/
int OmafMediaStream::DownloadInitSegment()
{
    pthread_mutex_lock(&mMutex);
    for(auto it = mMediaAdaptationSet.begin();
             it != mMediaAdaptationSet.end();
             it++ ){
        OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
        pAS->DownloadInitializeSegment();
    }

    for(auto extrator_it = mExtractors.begin();
             extrator_it != mExtractors.end();
             extrator_it++ ){
        OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
        extractor->DownloadInitializeSegment();
    }
    pthread_mutex_unlock(&mMutex);
    return ERROR_NONE;
}
/*
int OmafMediaStream::LoadLocalSegments()
{
    int ret = ERROR_NONE;
    pthread_mutex_lock(&mMutex);
    for(auto it = mMediaAdaptationSet.begin();
             it != mMediaAdaptationSet.end();
             it++ ){
        OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
        //pAS->DownloadSegment();
        pAS->LoadLocalSegment();
    }

    // NOTE: this function should be in the same thread with UpdateEnabledExtractors
    //       , otherwise mCurrentExtractors need a mutex lock
    for(auto extrator_it = mExtractors.begin();
             extrator_it != mExtractors.end();
             extrator_it++ ){
        OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
        //extractor->DownloadSegment();
        extractor->LoadLocalSegment();
    }
    pthread_mutex_unlock(&mMutex);
    return ret;
}
*/
int OmafMediaStream::DownloadSegments()
{
    int ret = ERROR_NONE;
    pthread_mutex_lock(&mMutex);
    for(auto it = mMediaAdaptationSet.begin();
             it != mMediaAdaptationSet.end();
             it++ ){
        OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
        pAS->DownloadSegment();
    }

    // NOTE: this function should be in the same thread with UpdateEnabledExtractors
    //       , otherwise mCurrentExtractors need a mutex lock
    //pthread_mutex_lock(&mCurrentMutex);
    for(auto extrator_it = mExtractors.begin();
             extrator_it != mExtractors.end();
             extrator_it++ ){
        OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
        extractor->DownloadSegment();
    }
    //pthread_mutex_unlock(&mCurrentMutex);
    pthread_mutex_unlock(&mMutex);
    return ret;
}

int OmafMediaStream::SeekTo( int seg_num)
{
    int ret = ERROR_NONE;
    pthread_mutex_lock(&mMutex);
    for(auto it = mMediaAdaptationSet.begin();
             it != mMediaAdaptationSet.end();
             it++ ){
        OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
        pAS->SeekTo(seg_num);
    }

    for(auto extrator_it = mExtractors.begin();
             extrator_it != mExtractors.end();
             extrator_it++ ){
        OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
        extractor->SeekTo(seg_num);
    }
    pthread_mutex_unlock(&mMutex);
    return ret;
}

int OmafMediaStream::UpdateEnabledExtractors(std::list<OmafExtractor*> extractors)
{
    if( extractors.empty() ) return ERROR_INVALID;

    int ret = ERROR_NONE;

    pthread_mutex_lock(&mMutex);
    for(auto as_it1 = mMediaAdaptationSet.begin(); as_it1 != mMediaAdaptationSet.end(); as_it1++){
        OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it1->second);
        pAS->Enable(false);
    }
    for(auto extrator_it = mExtractors.begin();
             extrator_it != mExtractors.end();
             extrator_it++ ){
        OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
        extractor->Enable(false);
    }

    pthread_mutex_lock(&mCurrentMutex);
    mCurrentExtractors.clear();
    for( auto it = extractors.begin(); it != extractors.end(); it++ ){
        OmafExtractor* tmp = (OmafExtractor*) (*it);
        tmp->Enable(true);
        mCurrentExtractors.push_back(tmp);
        std::map<int, OmafAdaptationSet*> AS = tmp->GetDependAdaptationSets();
        for(auto as_it = AS.begin(); as_it != AS.end(); as_it++ ){
            OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it->second);
            pAS->Enable(true);
        }
    }

    pthread_mutex_unlock(&mCurrentMutex);
    pthread_mutex_unlock(&mMutex);

    return ret;
}

int OmafMediaStream::GetTrackCount()
{
    return this->mMediaAdaptationSet.size() + this->mExtractors.size();
}

VCD_OMAF_END
