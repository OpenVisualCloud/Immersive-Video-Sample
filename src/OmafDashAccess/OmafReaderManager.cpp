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
 * File:   OmafReaderManager.cpp
 * Author: media
 *
 * Created on May 28, 2019, 1:41 PM
 */

#include "OmafReaderManager.h"
#include "OmafMP4VRReader.h"
#include <math.h>

VCD_OMAF_BEGIN

#define STATUS_UNKNOWN       0
#define STATUS_STOPPED       1
#define STATUS_RUNNING       2
#define STATUS_STOPPING      3
#define STATUS_SEEKING       4

static uint16_t GetTrackId(uint32_t id)
{
    return (id & 0xffff);
}

static uint32_t GetCombinedTrackId(uint16_t trackId, uint16_t initSegId)
{
    return ((initSegId << 16) | trackId);
}

OmafReaderManager::OmafReaderManager()
{
    mCurTrkCnt = 0;
    mEOS       = false;
    mSource    = NULL;
    mStatus    = STATUS_UNKNOWN;
    mReader    = NULL;
    mInitSegParsed = false;
    memset(mVPS, 0, 256);
    mVPSLen = 0;
    memset(mSPS, 0, 256);
    mSPSLen = 0;
    memset(mPPS, 0, 256);
    mPPSLen = 0;
    mWidth  = 0;
    mHeight = 0;
    mReadSync = false;
}

OmafReaderManager::~OmafReaderManager()
{
    Close();
}

int OmafReaderManager::Initialize( OmafMediaSource* pSource )
{
    mCurTrkCnt = 0;
    mEOS       = false;
    mSource    = pSource;
    mStatus    = STATUS_STOPPED;
    mReader    = new OmafMP4VRReader();
    //this->StartThread();
    return ERROR_NONE;
}

int OmafReaderManager::Close()
{
    if(mStatus == STATUS_RUNNING || mStatus == STATUS_SEEKING){
        mStatus = STATUS_STOPPING;
        this->Join();
    }

    SAFE_DELETE(mReader);
    releaseAllSegments();
    releasePacketQueue();

    for(auto &it:m_readSegMap)
    {
        std::map<uint32_t, OmafSegment*> initSegNormalSeg = it.second;
        for (auto& itRmSeg : initSegNormalSeg)
        {
            OmafSegment *rmSeg = itRmSeg.second;
            delete rmSeg;
            rmSeg = NULL;
        }
        m_readSegMap.erase(it.first);
    }
    m_readSegMap.clear();

    return ERROR_NONE;
}

int OmafReaderManager::AddInitSegment( OmafSegment* pInitSeg, uint32_t& nInitSegID )
{
    if(NULL == mReader) return ERROR_NULL_PTR;

    ScopeLock readerLock(mReaderLock);

    nInitSegID = mCurTrkCnt++;
    int32_t result = mReader->parseInitializationSegment(pInitSeg, nInitSegID);
    if (result != 0)
    {
        LOG(ERROR)<<"parse initialization segment failed! result= "<<result<<std::endl;
        mCurTrkCnt--;
        nInitSegID = -1;
        return ERROR_INVALID;
    }

    pInitSeg->SetInitSegID( nInitSegID );
    pInitSeg->SetSegID( nInitSegID );

    mMapSegCnt[nInitSegID] = 0;
    ///get track information if all initialize segmentation has been parsed
    if(mCurTrkCnt == mSource->GetTrackCount()){
        mReader->getTrackInformations( this->mTrackInfos );
        mLock.lock();
        UpdateSourceTrackID();
        mReader->setMapInitTrk(mMapInitTrk);
        SetupStatusMap();
        //mLock.lock();
        mInitSegParsed = true;
        mLock.unlock();
    }

    return ERROR_NONE;
}

void OmafReaderManager::UpdateSourceTrackID()
{
    for(auto it=mTrackInfos.begin(); it != mTrackInfos.end(); it++){
        TrackInformation *trackInfo = *it;
        for( int i = 0; i < mSource->GetStreamCount(); i++ ){
            OmafMediaStream* pStream = mSource->GetStream(i);
            std::map<int, OmafAdaptationSet*> pMediaAS = pStream->GetMediaAdaptationSet();
            auto as_it = pMediaAS.begin();
            for( ; as_it != pMediaAS.end(); as_it++){
                OmafAdaptationSet* pAS = (OmafAdaptationSet*) as_it->second;
                if(pAS->GetInitSegment()->GetSegID() == trackInfo->initSegId){
                    uint16_t actualTrackId = GetTrackId(trackInfo->trackId);
                    pAS->SetTrackNumber(actualTrackId);
                    mMapInitTrk[trackInfo->initSegId] = actualTrackId;//trackInfo->trackId;
                    mMapSegCnt[trackInfo->initSegId] = 0;
                    mMapSegStatus[actualTrackId].sampleIndex.mCurrentAddSegment = 0;
                    mMapSegStatus[actualTrackId].sampleIndex.mCurrentReadSegment = 1;
                    mMapSegStatus[actualTrackId].sampleIndex.mGlobalSampleIndex = 0;
                    mMapSegStatus[actualTrackId].sampleIndex.mGlobalStartSegment = 0;
                    mMapSegStatus[actualTrackId].sampleIndex.mSegmentSampleIndex = 0;
                    mMapSegStatus[actualTrackId].listActiveSeg.clear();
                    break;
                }
            }

            if (as_it != pMediaAS.end())
            {
                break;
            }

            std::map<int, OmafExtractor*> pExtratorAS = pStream->GetExtractors();
            for(auto extractor_it = pExtratorAS.begin(); extractor_it != pExtratorAS.end(); extractor_it++){
                OmafExtractor* pExAS = (OmafExtractor*) extractor_it->second;
                if(pExAS->GetInitSegment()->GetSegID() == trackInfo->initSegId){
                    uint16_t actualTrackId = GetTrackId(trackInfo->trackId);
                    pExAS->SetTrackNumber(actualTrackId);
                    mMapInitTrk[trackInfo->initSegId] = actualTrackId;//trackInfo->trackId;

                    mMapSegCnt[trackInfo->initSegId] = 0;
                    mMapSegStatus[actualTrackId].sampleIndex.mCurrentAddSegment = 0;
                    mMapSegStatus[actualTrackId].sampleIndex.mCurrentReadSegment = 1;
                    mMapSegStatus[actualTrackId].sampleIndex.mGlobalSampleIndex = 0;
                    mMapSegStatus[actualTrackId].sampleIndex.mGlobalStartSegment = 0;
                    mMapSegStatus[actualTrackId].sampleIndex.mSegmentSampleIndex = 0;
                    mMapSegStatus[actualTrackId].listActiveSeg.clear();

                    break;
                }
            }
        }
    }
}

int OmafReaderManager::AddSegment( OmafSegment* pSeg, uint32_t nInitSegID, uint32_t& nSegID)
{
    if(NULL == mReader) return ERROR_NULL_PTR;

    mLock.lock();

    int64_t segCnt = -1;
    // update the segment count for this track (get according to nInitSegID) with last element of m_readSegMap
    // , if it is not chose before
    if(pSeg->IsReEnabled())
    {
        if(m_readSegMap.size())
            segCnt = pSeg->GetSegCount();//m_readSegMap.rbegin()->first;
        else
            LOG(WARNING)<<"viewport changed but size of m_readSegMap is 0, failed to update segment count!!"<<endl;

        mMapSegCnt[nInitSegID] = segCnt - 1;
    }

    nSegID = ++(mMapSegCnt[nInitSegID]);
    //LOG(INFO)<<"now nSegID = "<<nSegID<<", pSeg->IsReEnabled() = "<<pSeg->IsReEnabled()<<", segCnt = "<<segCnt<<endl;

    auto it = m_readSegMap.begin();
    for ( ; it != m_readSegMap.end(); it++)
    {
        if (it->first == nSegID)
        {
            break;
        }
    }

    if (it == m_readSegMap.end())
    {
        std::map<uint32_t, OmafSegment*> initSegNormalSeg;
        initSegNormalSeg.insert(std::make_pair(nInitSegID, pSeg));
        m_readSegMap.insert(std::make_pair(nSegID, initSegNormalSeg));
    }
    else
    {
        std::map<uint32_t, OmafSegment*>* initSegNormalSeg;
        initSegNormalSeg = &(m_readSegMap[nSegID]);
        initSegNormalSeg->insert(std::make_pair(nInitSegID, pSeg));
    }

    mLock.unlock();

    /// need to check segment completion for each extractor in a stream;
    UpdateSegmentStatus(nInitSegID, nSegID, segCnt);

    return ERROR_NONE;
}

int OmafReaderManager::ParseSegment(uint32_t nSegID, uint32_t nInitSegID)
{
    if(NULL == mReader) return ERROR_NULL_PTR;

    int ret = ERROR_NONE;

    std::map<uint32_t, OmafSegment*> initSegNormalSeg;
    initSegNormalSeg = m_readSegMap[nSegID];
    OmafSegment *pSeg = initSegNormalSeg[nInitSegID];
    if(!pSeg)
    {
        LOG(ERROR) << "cannot get segment with ID "<<nSegID<<endl;
        return ERROR_INVALID;
    }

    ret = mReader->parseSegment(pSeg, nInitSegID, nSegID );

    if( 0 != ret )
    {
        LOG(ERROR) << "parseSegment return error "<<ret<<endl;
        return ERROR_INVALID;
    }

    for (int i = 0; i < mSource->GetStreamCount(); i++)
    {
        OmafMediaStream *pStream = mSource->GetStream(i);
        if (pStream->HasExtractor())
        {
            std::list<OmafExtractor*> extractors = pStream->GetEnabledExtractor();
            for (auto it = extractors.begin(); it != extractors.end(); it++)
            {
                OmafExtractor *pExt = (OmafExtractor*)(*it);

                uint32_t extractorTrackId = pExt->GetTrackNumber();
                OmafSegment *initSegment = pExt->GetInitSegment();
                uint32_t initSegIndex = initSegment->GetInitSegID();
                if (nInitSegID == initSegIndex)
                {
                    if ((uint32_t)(mMapSegStatus[extractorTrackId].segStatus[nSegID]) == (mMapSegStatus[extractorTrackId].depTrackIDs.size() + 1))
                    {
                        std::vector<TrackInformation*> readTrackInfos;
                        mReader->getTrackInformations( readTrackInfos );
                        mSegTrackInfos[nSegID] = readTrackInfos;
                    }
                }
            }
        }
    }
    return ERROR_NONE;
}

void OmafReaderManager::SetupStatusMap()
{
    for( int i = 0; i < mSource->GetStreamCount(); i++ ){
        OmafMediaStream* pStream = mSource->GetStream(i);
        std::map<int, OmafExtractor*> pExtratorAS = pStream->GetExtractors();
        for(auto as_it = pExtratorAS.begin(); as_it != pExtratorAS.end(); as_it++){
            OmafExtractor* pExAS = (OmafExtractor*) as_it->second;
            int trackID = pExAS->GetTrackNumber();
            std::list<int> listDepTracks = pExAS->GetDependTrackID();
            mMapSegStatus[trackID].depTrackIDs = listDepTracks;
        }
    }
}

void OmafReaderManager::UpdateSegmentStatus(uint32_t nInitSegID, uint32_t nSegID, int64_t segCnt)
{
    mLock.lock();
    int trackId = mMapInitTrk[nInitSegID];
    if (mMapSegStatus[trackId].depTrackIDs.size())
    {
        if (nSegID > mMapSegStatus[trackId].segStatus.size())
        {
            mMapSegStatus[trackId].segStatus[nSegID] = 0;
        }
        mMapSegStatus[trackId].segStatus[nSegID]++;
        // only update mCurrentReadSegment if segCnt is updated with viewport changed
        if(segCnt != -1)
        {
            mMapSegStatus[trackId].sampleIndex.mCurrentReadSegment = segCnt;
        }
    }

    for( auto it=mMapSegStatus.begin(); it!=mMapSegStatus.end(); it++ ){
        for(auto id  = mMapSegStatus[it->first].depTrackIDs.begin();
                 id != mMapSegStatus[it->first].depTrackIDs.end();
                 id++ ){
            if( *id == mMapInitTrk[nInitSegID] ){
                if (nSegID > mMapSegStatus[it->first].segStatus.size())
                {
                    mMapSegStatus[it->first].segStatus[nSegID] = 0;
                }
                mMapSegStatus[it->first].segStatus[nSegID]++;
                mMapSegStatus[it->first].listActiveSeg.push_back(nSegID);
                mMapSegStatus[it->first].sampleIndex.mCurrentAddSegment = nSegID;
                break;
            }
        }
    }

    mLock.unlock();
}

int OmafReaderManager::Seek( )
{
    DashMediaInfo info;
    mSource->GetMediaInfo( &info );
    int type = info.streaming_type;
    if(type != 1) return ERROR_INVALID;

    /// seek
    mStatus=STATUS_SEEKING;

    releaseAllSegments( );
    releasePacketQueue();

    mStatus=STATUS_RUNNING;

    return ERROR_NONE;
}

void OmafReaderManager::RemoveTrackFromPacketQueue(list<int>& trackIDs)
{
    mPacketLock.lock();
    for(auto &it : trackIDs)
    {
        PacketQueue pPackQ = mPacketQueues[it];
        for (auto iter : pPackQ)
        {
            MediaPacket *mediaPacket = iter;
            delete mediaPacket;
            mediaPacket = NULL;
        }
        mPacketQueues.erase(it);
    }
    mPacketLock.unlock();
}

int OmafReaderManager::GetNextFrame( int trackID, MediaPacket*& pPacket, bool needParams )
{
    mPacketLock.lock();
    if( mPacketQueues[trackID].empty()){
        pPacket = NULL;
        mPacketLock.unlock();
        return ERROR_NULL_PACKET;
    }

    if( !mPacketQueues[trackID].size() ){
        mPacketLock.unlock();
        return OMAF_ERROR_END_OF_STREAM;
    }
    pPacket = mPacketQueues[trackID].front();
    mPacketQueues[trackID].pop_front();
    LOG(INFO)<<"========mPacketQueues size========:"<<mPacketQueues[trackID].size()<<std::endl;
    mPacketLock.unlock();

    if (needParams)
    {
        if (!mVPSLen || !mSPSLen || !mPPSLen)
        {
            LOG(ERROR) << "Invalid VPS/SPS/PPS in getting packet ! " << endl;
            return OMAF_ERROR_INVALID_DATA;
        }

        MediaPacket *newPacket = new MediaPacket();
        uint32_t newSize = mVPSLen + mSPSLen + mPPSLen + pPacket->Size();
        newPacket->ReAllocatePacket(newSize);
        newPacket->SetRealSize(newSize);

        char *origData = pPacket->Payload();
        char *newData  = newPacket->Payload();
        memcpy(newData, mVPS, mVPSLen);
        memcpy(newData + mVPSLen, mSPS, mSPSLen);
        memcpy(newData + mVPSLen + mSPSLen, mPPS, mPPSLen);
        memcpy(newData + mVPSLen + mSPSLen + mPPSLen, origData, pPacket->Size());
        RegionWisePacking *newRwpk = new RegionWisePacking;
        RegionWisePacking *pRwpk = pPacket->GetRwpk();
        *newRwpk = *pRwpk;
        newRwpk->rectRegionPacking = new RectangularRegionWisePacking[newRwpk->numRegions];
        memcpy(newRwpk->rectRegionPacking, pRwpk->rectRegionPacking, pRwpk->numRegions * sizeof(RectangularRegionWisePacking));
        newPacket->SetRwpk(newRwpk);
        delete pPacket;
        pPacket = newPacket;
    }
    return ERROR_NONE;
}

int OmafReaderManager::ReadNextSegment(
    int trackID,
    uint16_t initSegID,
    bool isExtractor,
    std::vector<TrackInformation*> readTrackInfos,
    bool& segmentChanged )
{
    if(NULL == mReader) return ERROR_NULL_PTR;
    int32_t ret = ERROR_NONE;

    SampleIndex *sampleIdx = &(mMapSegStatus[trackID].sampleIndex);

    LOG(INFO) << "Begin to read segment " << sampleIdx->mCurrentReadSegment <<" for track "<<trackID<< endl;
    TrackInformation *trackInfo = nullptr;
    for ( auto &itTrack : readTrackInfos)
    {
        if (GetTrackId(itTrack->trackId) == trackID)
        {
            trackInfo = itTrack;
            break;
        }
    }
    if (!trackInfo)
    {
        LOG(ERROR) << "The specified track is not found " << endl;
        return ERROR_NOT_FOUND;
    }

    if (sampleIdx->mCurrentReadSegment > sampleIdx->mCurrentAddSegment)
    {
        LOG(ERROR) << "Can't read not added segment ! " << endl;
        return OMAF_ERROR_INVALID_DATA;
    }

    std::map<uint32_t, uint32_t> segSizeMap;

    for (auto& itSample : trackInfo->samplePropertyArrays)
    {
        segSizeMap[itSample->segmentId] = 0;
    }
    int32_t beginSampleId = -1;
    for (auto& itSample : trackInfo->samplePropertyArrays)
    {
        segSizeMap[itSample->segmentId]++;
        if (beginSampleId == -1 && itSample->segmentId == sampleIdx->mCurrentReadSegment)
        {
            beginSampleId = itSample->id;
        }
    }

    if(beginSampleId == -1) return OMAF_ERROR_INVALID_DATA;

    for ( ; beginSampleId < (int32_t)(segSizeMap[sampleIdx->mCurrentReadSegment]); beginSampleId++)
    {
        int sample = beginSampleId;

        uint32_t combinedTrackId = GetCombinedTrackId(trackID, initSegID);

        if (!mWidth || !mHeight)
        {
            ret = mReader->getWidth(combinedTrackId, sample, mWidth);
            if (ret)
            {
                LOG(ERROR) << "Failed to get sample width !" << endl;
                return ret;
            }

            if (mWidth == 0)
            {
                LOG(ERROR) << "Get invalid sample width !" << endl;
                return OMAF_ERROR_INVALID_DATA;
            }

            ret = mReader->getHeight(combinedTrackId, sample, mHeight);
            if (ret)
            {
                LOG(ERROR) << "Failed to get sample height !" << endl;
                return ret;
            }

            if (mHeight == 0)
            {
                LOG(ERROR) << "Get invalid sample height !" << endl;
                return OMAF_ERROR_INVALID_DATA;
            }

            LOG(INFO) << "Get sample width " << mWidth << " and sample height " << mHeight << " !" << endl;
        }

        MediaPacket* packet = new MediaPacket();
        uint32_t packetSize = ((mWidth * mHeight * 3) / 2 ) / 2;
        packet->ReAllocatePacket(packetSize);

        if (!mVPSLen || !mSPSLen || !mPPSLen)
        {
            memset(mVPS, 0, 256);
            memset(mSPS, 0, 256);
            memset(mPPS, 0, 256);
            mVPSLen = 0;
            mSPSLen = 0;
            mPPSLen = 0;

            std::vector<VCD::OMAF::DecoderSpecificInfo> parameterSets;
            ret = mReader->getDecoderConfiguration(combinedTrackId, sample, parameterSets);
            if (ret)
            {
                LOG(ERROR) << "Failed to get VPS/SPS/PPS ! " << endl;
                return ret;
            }

            for (auto const& parameter : parameterSets)
            {
                if (parameter.decodeSpecInfoType == VCD::OMAF::HEVC_VPS)
                {
                    mVPSLen = parameter.decodeSpecInfoData.size();
                    for (uint32_t i = 0; i < parameter.decodeSpecInfoData.size(); i++)
                    {
                        mVPS[i] = parameter.decodeSpecInfoData[i];
                    }
                }


                if (parameter.decodeSpecInfoType == VCD::OMAF::HEVC_SPS)
                {
                    mSPSLen = parameter.decodeSpecInfoData.size();
                    for (uint32_t i = 0; i < parameter.decodeSpecInfoData.size(); i++)
                    {
                        mSPS[i] = parameter.decodeSpecInfoData[i];
                    }
                }

                if (parameter.decodeSpecInfoType == VCD::OMAF::HEVC_PPS)
                {
                    mPPSLen = parameter.decodeSpecInfoData.size();
                    for (uint32_t i = 0; i < parameter.decodeSpecInfoData.size(); i++)
                    {
                        mPPS[i] = parameter.decodeSpecInfoData[i];
                    }
                }
            }
        }

        if (isExtractor)
        {
            ret = mReader->getExtractorTrackSampleData(combinedTrackId, sample, (char *)(packet->Payload()), packetSize );
        }
        else
        {
            ret =  mReader->getTrackSampleData(combinedTrackId, sample, (char *)(packet->Payload()), packetSize );
        }

        RegionWisePacking *pRwpk = new RegionWisePacking;

        ret = mReader->getPropertyRegionWisePacking(combinedTrackId, sample, pRwpk);

        packet->SetRwpk(pRwpk);

        if (ret == OMAF_MEMORY_TOO_SMALL_BUFFER )
        {
            LOG(ERROR) << "The frame size has exceeded the maximum packet size" << endl;
            return ret;
        }
        else if (ret)
        {
            LOG(ERROR) << "Failed to get packet " << (sampleIdx->mGlobalSampleIndex + beginSampleId) << " for track " << trackID << " and error is " << ret << endl;
            return ret;
        }
        packet->SetRealSize(packetSize);
        mPacketLock.lock();
        mPacketQueues[trackID].push_back(packet);
        mPacketLock.unlock();
    }

    LOG(INFO) << "Segment " << trackInfo->samplePropertyArrays[beginSampleId - 1]->segmentId << " for track " << trackID << " has been read !" << endl;
    sampleIdx->mCurrentReadSegment++;
    sampleIdx->mGlobalSampleIndex += beginSampleId;
    LOG(INFO) << "Total read " << sampleIdx->mGlobalSampleIndex << " samples for track " << trackID <<" now !" << endl;

    removeSegment(initSegID, sampleIdx->mCurrentReadSegment - 1);

    SegStatus st = mMapSegStatus[trackID];
    std::list<int>::iterator itRef = st.depTrackIDs.begin();
    for ( ; itRef != st.depTrackIDs.end(); itRef++)
    {
        int refTrack = *itRef;

        TrackInformation *refTrackInfo = nullptr;
        for ( auto &itRefInfo : readTrackInfos)
        {
            if (GetTrackId(itRefInfo->trackId) == refTrack)
            {
                refTrackInfo = itRefInfo;
                break;
            }
        }

        removeSegment(refTrackInfo->initSegId, sampleIdx->mCurrentReadSegment - 1);

    }

    for(auto &it : readTrackInfos)
    {
        for(auto &is : it->samplePropertyArrays)
        {
            SAFE_DELETE(is);
        }
        it->samplePropertyArrays.clear();
        SAFE_DELETE(it);
    }
    readTrackInfos.clear();

    return ERROR_NONE;
}

void OmafReaderManager::Run()
{
    bool go_on = true;

    if(NULL == mSource) return;

    bool bSegChange = false;

    mStatus = STATUS_RUNNING;

    while(go_on && mStatus != STATUS_STOPPED){
        mLock.lock();
        while (!mInitSegParsed)
        {
            mLock.unlock();
            ::usleep(1000);
            mLock.lock();
        }
        mLock.unlock();

        if( mStatus==STATUS_STOPPING ){
            mStatus = STATUS_STOPPED;
            break;
        }

        if(mStatus==STATUS_SEEKING){
            continue;
        }
        DashMediaInfo info;

        mSource->GetMediaInfo( &info );
        int type = info.streaming_type;

        if(type == 1 && mEOS) break;

        /// begin to read packet for each stream
        for( int i = 0; i < mSource->GetStreamCount(); i++ ){
            if( mStatus==STATUS_STOPPING ){
                mStatus = STATUS_STOPPED;
                break;
            }
            OmafMediaStream* pStream = mSource->GetStream(i);
            if(pStream->HasExtractor()){
                std::list<OmafExtractor*> extractors = pStream->GetEnabledExtractor();
                for(auto it=extractors.begin(); it!=extractors.end(); it++){
                    OmafExtractor* pExt = (OmafExtractor*)(*it);
                    SegStatus *st = &(mMapSegStatus[pExt->GetTrackNumber()]);

                    ///if static mode, check EOS
                    if(type == 1){
                        uint64_t segmentDur = pStream->GetSegmentDuration();
                        if (segmentDur == 0)
                        {
                            return;
                        }
                        float tmpSegNum = float(info.duration) / 1000 / segmentDur;
                        uint32_t totalSegNum = abs(tmpSegNum - uint32_t(tmpSegNum)) < 1e-6 ? uint32_t(tmpSegNum) : uint32_t(tmpSegNum) + 1;
                        if (st->sampleIndex.mCurrentReadSegment > totalSegNum)
                        {
                            mLock.lock();
                            this->mEOS = true;
                            mLock.unlock();
                            return;
                        }
                    }

                    mLock.lock();
                    while (st->sampleIndex.mCurrentReadSegment > st->sampleIndex.mCurrentAddSegment && mStatus!=STATUS_STOPPING)
                    {
                        LOG(INFO) << "New segment " << st->sampleIndex.mCurrentReadSegment << " hasn't come, then wait !" << endl;
                        mLock.unlock();
                        ::usleep(1000);
                        mLock.lock();
                    }
                    mLock.unlock();

                    if( mStatus==STATUS_STOPPING ){
                        mStatus = STATUS_STOPPED;
                          break;
                    }

                    if((uint32_t)(st->segStatus[st->sampleIndex.mCurrentReadSegment]) == (st->depTrackIDs.size() + 1)){
                        uint16_t trackID = pExt->GetTrackNumber();
                        uint16_t initSegID = 0;
                        for (auto& idPair : mMapInitTrk)
                        {
                            if (idPair.second == trackID)
                            {
                                initSegID = idPair.first;
                                break;
                            }
                        }


                        std::list<int>::iterator itRef = st->depTrackIDs.begin();
                        for ( ; itRef != st->depTrackIDs.end(); itRef++)
                        {
                            int refTrack = *itRef;
                            uint32_t initSegIndex = 0;

                            for (auto& idPair : mMapInitTrk)
                            {
                                if (idPair.second == refTrack)
                                {
                                    initSegIndex = idPair.first;
                                    break;
                                }
                            }

                            ParseSegment(st->sampleIndex.mCurrentReadSegment, initSegIndex);

                        }

                        ParseSegment(st->sampleIndex.mCurrentReadSegment, initSegID);
                        std::vector<TrackInformation*> readTrackInfos = mSegTrackInfos[st->sampleIndex.mCurrentReadSegment];
                        this->ReadNextSegment(trackID, initSegID, true, readTrackInfos, bSegChange);
                        mSegTrackInfos.erase(st->sampleIndex.mCurrentReadSegment - 1);

                        RemoveReadSegmentFromMap();
                    }else{
                        ::usleep(100);
                    }
                }
            }else{
                std::map<int, OmafAdaptationSet*> mapAS = pStream->GetMediaAdaptationSet();
                for(auto as_it=mapAS.begin(); as_it!=mapAS.end(); as_it++){
                    OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it->second);
                    SegStatus *st = &(mMapSegStatus[pAS->GetTrackNumber()]);
                    if(type == 1){
                        uint64_t segmentDur = pStream->GetSegmentDuration();
                        if (segmentDur == 0)
                        {
                            return;
                        }
                        float tmpSegNum = float(info.duration) / 1000 / segmentDur;
                        uint32_t totalSegNum = abs(tmpSegNum - uint32_t(tmpSegNum)) < 1e-6 ? uint32_t(tmpSegNum) : uint32_t(tmpSegNum) + 1;
                        if (st->sampleIndex.mCurrentReadSegment > totalSegNum)
                        {
                            mLock.lock();
                            this->mEOS = true;
                            mLock.unlock();
                            return;
                        }
                    }
                    uint16_t trackID = pAS->GetTrackNumber();
                    uint16_t initSegID = 0;
                    for (auto& idPair : mMapInitTrk)
                    {
                        if (idPair.second == trackID)
                        {
                            initSegID = idPair.first;
                            break;
                        }
                    }

                    mLock.lock();
                    while (st->sampleIndex.mCurrentReadSegment > st->sampleIndex.mCurrentAddSegment && mStatus!=STATUS_STOPPING)
                    {
                        LOG(INFO) << "New segment " << st->sampleIndex.mCurrentReadSegment << " hasn't come, then wait !" << endl;
                        mLock.unlock();
                        ::usleep(1000);
                        mLock.lock();
                    }
                    mLock.unlock();

                    if( mStatus==STATUS_STOPPING ){
                        mStatus = STATUS_STOPPED;
                          break;
                    }

                    std::vector<TrackInformation*> readTrackInfos = mSegTrackInfos[st->sampleIndex.mCurrentReadSegment];
                    this->ReadNextSegment(trackID, initSegID, false, readTrackInfos, bSegChange);
                }
            }
        }

        for(int i=0; i<info.stream_count; i++)
        {
            SAFE_DELETE(info.stream_info[i].codec);
            SAFE_DELETE(info.stream_info[i].mime_type);
        }
    }
}

// Keep more than 1 element in m_readSegMap for segment count update if viewport changed
void OmafReaderManager::RemoveReadSegmentFromMap()
{
    if(m_readSegMap.size() < 10) return;

    for(auto &it:m_readSegMap)
    {
        if(m_readSegMap.size() < 10)
            break;
        std::map<uint32_t, OmafSegment*> initSegNormalSeg = it.second;
        for (auto& itRmSeg : initSegNormalSeg)
        {
            OmafSegment *rmSeg = itRmSeg.second;
            delete rmSeg;
            rmSeg = NULL;
        }
        m_readSegMap.erase(it.first);
    }
}

void OmafReaderManager::releaseAllSegments( )
{
    ScopeLock managerLock(mLock);

     for(auto it=mMapSegStatus.begin(); it!=mMapSegStatus.end(); it++){

         SegStatus *s = &(it->second);
         s->sampleIndex.mCurrentAddSegment = 0;
         s->sampleIndex.mCurrentReadSegment = 0;
         s->sampleIndex.mGlobalSampleIndex = 0;
         s->sampleIndex.mGlobalStartSegment = 0;
         s->sampleIndex.mSegmentSampleIndex = 0;
         s->listActiveSeg.clear();
     }
}

void OmafReaderManager::setNextSampleId(int trackID, uint32_t id, bool& segmentChanged)
{
    size_t indexInSegment = 0;

    TrackInformation *trackInfo = nullptr;
    for (auto &itTrack : mTrackInfos)
    {
        if (GetTrackId(itTrack->trackId) == trackID)
        {
            trackInfo = itTrack;
            break;
        }
    }

    for (size_t index = 0; index < trackInfo->samplePropertyArrays.size(); index++)
    {
        if (trackInfo->samplePropertyArrays[index]->id == id)
        {
            mMapSegStatus[trackID].sampleIndex.mGlobalSampleIndex = static_cast<uint32_t>(index);
            mMapSegStatus[trackID].sampleIndex.mSegmentSampleIndex = static_cast<uint32_t>(indexInSegment);
            break;
        }
        if (trackInfo->samplePropertyArrays[index]->segmentId == mMapSegStatus[trackID].sampleIndex.mCurrentReadSegment)
        {
            indexInSegment++;
        }
    }
}

uint32_t OmafReaderManager::removeSegment(uint32_t initSegmentId, uint32_t segmentId)
{
    LOG(INFO) << "removeSegment " << segmentId << " for track " << mMapInitTrk[initSegmentId] << endl;

    ScopeLock managerLock(mLock);

    int32_t result = mReader->invalidateSegment(initSegmentId, segmentId);

    if (result != 0){
        LOG(ERROR) << "removeSegment Failed " << segmentId << endl;
        return ERROR_INVALID;
    }

    if (mMapSegStatus[mMapInitTrk[initSegmentId]].listActiveSeg.size() > 0)
    {
        std::list<int>::iterator it = mMapSegStatus[mMapInitTrk[initSegmentId]].listActiveSeg.begin();

        for ( ; it != mMapSegStatus[mMapInitTrk[initSegmentId]].listActiveSeg.end();)
        {
            if ((uint32_t)(*it) == segmentId)
            {
                it = mMapSegStatus[mMapInitTrk[initSegmentId]].listActiveSeg.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
    LOG(INFO) << "~~Now active list has segments " << mMapSegStatus[mMapInitTrk[initSegmentId]].listActiveSeg.size() << endl;
    return ERROR_NONE;
}

void OmafReaderManager::releasePacketQueue()
{
    ScopeLock packetLock(mPacketLock);
    for(auto it=mPacketQueues.begin(); it!=mPacketQueues.end(); it++){
        PacketQueue queue = (*it).second;
        for(auto qu_it=queue.begin(); qu_it!=queue.end(); qu_it++){
            MediaPacket* pkt = (MediaPacket*)(*qu_it);
            delete pkt;
            pkt = NULL;
        }
        queue.clear();
    }
}

VCD_OMAF_END
