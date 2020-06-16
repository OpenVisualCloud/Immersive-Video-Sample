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
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
#include "../trace/Bandwidth_tp.h"
#include "../trace/MtHQ_tp.h"
#endif
#endif

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
    mVPSLen = 0;
    mSPSLen = 0;
    mPPSLen = 0;
    mWidth  = 0;
    mHeight = 0;
    mReadSync = false;
    mExtractorEnabled = true;
    mGlobalReadSegId = 0;
}

OmafReaderManager::~OmafReaderManager()
{
    if (m_videoHeaders.size())
    {
        std::map<uint32_t, std::map<uint32_t, uint8_t*>>::iterator it;
        for (it = m_videoHeaders.begin(); it != m_videoHeaders.end(); )
        {
            std::map<uint32_t, uint8_t*> oneHeader = it->second;
            std::map<uint32_t, uint8_t*>::iterator itHrd;
            for (itHrd = oneHeader.begin(); itHrd != oneHeader.end(); )
            {
                uint8_t *header = itHrd->second;
                if (header)
                {
                    delete [] header;
                    header = NULL;
                }
                oneHeader.erase(itHrd++);
            }
            oneHeader.clear();
            m_videoHeaders.erase(it++);
        }
        m_videoHeaders.clear();
    }

    if (mPacketQueues.size())
    {
        std::map<int, PacketQueue>::iterator it;
        for (it = mPacketQueues.begin(); it != mPacketQueues.end(); )
        {
            PacketQueue queue = it->second;
            if (queue.size())
            {
                std::list<MediaPacket*>::iterator itPacket;
                for (itPacket = queue.begin(); itPacket != queue.end(); )
                {
                    MediaPacket *packet = *itPacket;
                    SAFE_DELETE(packet);
                    queue.erase(itPacket++);
                }
                queue.clear();
            }
            mPacketQueues.erase(it++);
        }
        mPacketQueues.clear();
    }

    Close();
}

int OmafReaderManager::Initialize( OmafMediaSource* pSource )
{
    mCurTrkCnt = 0;
    mEOS       = false;
    mSource    = pSource;
    mStatus    = STATUS_STOPPED;
    mReader    = new OmafMP4VRReader();
    return ERROR_NONE;
}

int OmafReaderManager::Close()
{
    mStatus = STATUS_STOPPING;
    usleep(1000);
    mLock.lock();
    std::map<uint32_t, std::map<uint32_t, OmafSegment*>>::iterator it;
    for (it = m_readSegMap.begin(); it != m_readSegMap.end();)
    {
        std::map<uint32_t, OmafSegment*> initSegNormalSeg = it->second;
        for (auto& itRmSeg : initSegNormalSeg)
        {
            OmafSegment *rmSeg = itRmSeg.second;
            rmSeg->Close();
            delete rmSeg;
            rmSeg = NULL;
        }
        m_readSegMap.erase(it++);
    }
    m_readSegMap.clear();
    mLock.unlock();

    SAFE_DELETE(mReader);
    releaseAllSegments();
    releasePacketQueue();

    return ERROR_NONE;
}

int OmafReaderManager::AddInitSegment( OmafSegment* pInitSeg, uint32_t& nInitSegID )
{
    if(NULL == mReader) return ERROR_NULL_PTR;

    ScopeLock readerLock(mReaderLock);
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    //trace
    const char *trackType = "init_track";
    uint64_t segSize = pInitSeg->GetSegSize();
    char tileRes[128] = { 0 };
    snprintf(tileRes, 128, "%s", "none");
    tracepoint(bandwidth_tp_provider, packed_segment_size, 0, trackType, tileRes, 0, segSize);
#endif
#endif

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
                if(pAS->GetInitSegment()->GetSegID() == trackInfo->initSegmentId){
                    uint16_t actualTrackId = GetTrackId(trackInfo->trackId);
                    pAS->SetTrackNumber(actualTrackId);
                    mMapInitTrk[trackInfo->initSegmentId] = actualTrackId;//trackInfo->trackId;
                    mMapSegCnt[trackInfo->initSegmentId] = 0;
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
                if(pExAS->GetInitSegment()->GetSegID() == trackInfo->initSegmentId){
                    uint16_t actualTrackId = GetTrackId(trackInfo->trackId);
                    pExAS->SetTrackNumber(actualTrackId);
                    mMapInitTrk[trackInfo->initSegmentId] = actualTrackId;//trackInfo->trackId;

                    mMapSegCnt[trackInfo->initSegmentId] = 0;
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
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    //trace
    int trackIndex = mMapInitTrk[nInitSegID];
    if (mMapSegStatus[trackIndex].depTrackIDs.size())
    {
        const char *trackType = "extractor_track";
        uint64_t segSize = pSeg->GetSegSize();
        char tileRes[128] = { 0 };
        snprintf(tileRes, 128, "%s", "none");
        tracepoint(bandwidth_tp_provider, packed_segment_size, trackIndex, trackType, tileRes, nSegID, segSize);
    }
    else
    {
        const char *trackType = "tile_track";
        uint64_t segSize = pSeg->GetSegSize();
        char tileRes[128] = { 0 };
        snprintf(tileRes, 128, "%s", "none");
        tracepoint(bandwidth_tp_provider, packed_segment_size, trackIndex, trackType, tileRes, nSegID, segSize);
    }
#endif
#endif

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

int OmafReaderManager::ParseSegment(
    uint32_t nSegID,
    uint32_t nInitSegID,
    uint32_t& qualityRanking,
    SRDInfo *srd)
{
    if (NULL == mReader) return ERROR_NULL_PTR;
    if (!mExtractorEnabled && !srd)
    {
        LOG(ERROR) << "SRD can't be NULL !" << endl;
        return ERROR_INVALID;
    }

    int ret = ERROR_NONE;

    std::map<uint32_t, OmafSegment*> initSegNormalSeg;
    initSegNormalSeg = m_readSegMap[nSegID];
    OmafSegment *pSeg = initSegNormalSeg[nInitSegID];
    if(!pSeg)
    {
        LOG(ERROR) << "cannot get segment with ID "<<nSegID<<endl;
        return ERROR_INVALID;
    }
    pSeg->Close();
    if (!mExtractorEnabled)
    {
        qualityRanking = pSeg->GetQualityRanking();

        SRDInfo srdInfo = pSeg->GetSRDInfo();
        srd->left = srdInfo.left;
        srd->top  = srdInfo.top;
        srd->width = srdInfo.width;
        srd->height = srdInfo.height;
    }
    else
    {
        qualityRanking = 0;
    }

    ret = mReader->parseSegment(pSeg, nInitSegID, nSegID );

    if( 0 != ret )
    {
        LOG(ERROR) << "parseSegment return error "<<ret<<endl;
        return ERROR_INVALID;
    }

    if (mExtractorEnabled)
    {
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
    }
    else
    {
        std::vector<TrackInformation*> readTrackInfos;
        mReader->getTrackInformations( readTrackInfos );
        std::map<uint32_t, std::vector<TrackInformation*>>::iterator itTrackInfo;
        itTrackInfo = mSegTrackInfos.find(nSegID);
        if (itTrackInfo != mSegTrackInfos.end())
        {
            mSegTrackInfos.erase(itTrackInfo);
        }

        mSegTrackInfos[nSegID] = readTrackInfos;
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
    if (mExtractorEnabled)
    {
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
    }
    else
    {
        for( auto it = mMapSegStatus.begin(); it != mMapSegStatus.end(); it++ ){
            if( (it->first) == trackId ){
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
        for (std::list<MediaPacket*>::iterator iter = pPackQ.begin() ; iter != pPackQ.end(); iter++)
        {
            MediaPacket *mediaPacket = *iter;
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
        LOG(INFO)<< "track for " << trackID << " Empty packets queue !" <<endl;
        DashMediaInfo info;
        mSource->GetMediaInfo(&info);
        if (info.streaming_type == 1)
        {
            bool isEOSNow = false;
            mLock.lock();
            isEOSNow = mEOS;
            mLock.unlock();
            if (isEOSNow)
            {
                LOG(INFO)<< "EOS Now !" << endl;
                pPacket = new MediaPacket();
                pPacket->SetEOS(true);
                mPacketLock.unlock();
                return ERROR_NONE;
            }
            else
            {
                pPacket = NULL;
                mPacketLock.unlock();
                return ERROR_NULL_PACKET;
            }
        }
        else
        {
            pPacket = NULL;
            mPacketLock.unlock();
            return ERROR_NULL_PACKET;
        }
    }

    pPacket = mPacketQueues[trackID].front();
    mPacketQueues[trackID].pop_front();
    LOG(INFO)<<"========mPacketQueues size========:"<<mPacketQueues[trackID].size() << " for track "<< trackID <<std::endl;
    mPacketLock.unlock();

    if (needParams)
    {
        uint32_t qualityRanking = pPacket->GetQualityRanking();
        std::map<uint32_t, uint8_t*> header = m_videoHeaders[qualityRanking];
        if (0 == header.size())
        {
            LOG(ERROR) << "Invalid VPS/SPS/PPS in getting packet ! " << endl;
            return OMAF_ERROR_INVALID_DATA;
        }
        std::map<uint32_t, uint8_t*>::iterator itHrd;
        itHrd = header.begin();
        uint32_t headerSize = itHrd->first;
        uint8_t *headerData = itHrd->second;
        MediaPacket *newPacket = new MediaPacket();
        uint32_t newSize = headerSize + pPacket->Size();
        newPacket->ReAllocatePacket(newSize);
        newPacket->SetRealSize(newSize);

        char *origData = pPacket->Payload();
        char *newData  = newPacket->Payload();
        if(!origData || !newData)
        {
            LOG(ERROR)<<" data ptr is null !"<<endl;
            SAFE_DELETE(newPacket);
            return OMAF_ERROR_NULL_PTR;
        }
        memcpy(newData, headerData, headerSize);
        memcpy(newData + headerSize, origData, pPacket->Size());
        RegionWisePacking *newRwpk = new RegionWisePacking;
        RegionWisePacking *pRwpk = pPacket->GetRwpk();
        if(!newRwpk || !pRwpk)
        {
            LOG(ERROR)<<"rwpk ptr is null!"<<endl;
            SAFE_DELETE(newPacket);
            SAFE_DELETE(newRwpk);
            return OMAF_ERROR_NULL_PTR;
        }
        *newRwpk = *pRwpk;
        newRwpk->rectRegionPacking = new RectangularRegionWisePacking[newRwpk->numRegions];
        memcpy(newRwpk->rectRegionPacking, pRwpk->rectRegionPacking, pRwpk->numRegions * sizeof(RectangularRegionWisePacking));
        newPacket->SetRwpk(newRwpk);
        newPacket->SetType(pPacket->GetType());
        newPacket->SetPTS(pPacket->GetPTS());
        newPacket->SetSegID(pPacket->GetSegID());
        newPacket->SetQualityNum(pPacket->GetQualityNum());
        SourceResolution* newSrcRes = new SourceResolution[pPacket->GetQualityNum()];
        memcpy(newSrcRes, pPacket->GetSourceResolutions(), pPacket->GetQualityNum() * sizeof(SourceResolution));
        for (int32_t i=0;i<pPacket->GetQualityNum();i++)
        {
            newPacket->SetSourceResolution(i, newSrcRes[i]);
            //cout<<"copy sourceRes:"<<newPacket->GetSourceResolutions()[i].qualityRanking<<" "<<newPacket->GetSourceResolutions()[i].top<<" "<<newPacket->GetSourceResolutions()[i].left<<" "<<newPacket->GetSourceResolutions()[i].width<<" "<<newPacket->GetSourceResolutions()[i].height<<endl;
        }
        newPacket->SetQualityRanking(pPacket->GetQualityRanking());
        newPacket->SetSRDInfo(pPacket->GetSRDInfo());
        newPacket->SetCodecType(pPacket->GetCodecType());
        newPacket->SetHasVideoHeader(true);
        //LOG(INFO)<<"SetHasVideoHeader: "<<trackID<<endl;
        newPacket->SetVideoHeaderSize(headerSize);
        if (qualityRanking == HIGHEST_QUALITY_RANKING)
        {
            newPacket->SetVPSLen(mVPSLen);
            newPacket->SetSPSLen(mSPSLen);
            newPacket->SetPPSLen(mPPSLen);
        }

        delete pPacket;
        pPacket = newPacket;
    }
    return ERROR_NONE;
}

int OmafReaderManager::GetPacketQueueSize(int trackID, int& size)
{
    mPacketLock.lock();
    size = mPacketQueues[trackID].size();
    mPacketLock.unlock();
    return ERROR_NONE;
}

int OmafReaderManager::ReadNextSegment(
    int trackID,
    uint16_t initSegID,
    bool isExtractor,
    std::vector<TrackInformation*> readTrackInfos,
    bool& segmentChanged,
    uint32_t qualityRanking,
    SRDInfo *srd)
{
    if(NULL == mReader) return ERROR_NULL_PTR;
    int32_t ret = ERROR_NONE;

    SampleIndex *sampleIdx = &(mMapSegStatus[trackID].sampleIndex);

    LOG(INFO) << "Begin to read segment " << sampleIdx->mCurrentReadSegment <<" for track "<<trackID<< endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    tracepoint(mthq_tp_provider, T5_read_start_time, sampleIdx->mCurrentReadSegment);
#endif
#endif
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

    if (!isExtractor && (0 == qualityRanking))
    {
        LOG(ERROR) << "INCORRECT quality ranking for tile track !" << endl;
        return OMAF_ERROR_INVALID_DATA;
    }

    if (!isExtractor && !srd)
    {
        LOG(ERROR) << "NULL SRD info for tile track !" << endl;
        return OMAF_ERROR_INVALID_DATA;
    }

    std::map<uint32_t, uint32_t> segSizeMap;

    for (uint32_t sampIndex = 0; sampIndex < trackInfo->sampleProperties.size; sampIndex++)
    {
        segSizeMap[trackInfo->sampleProperties[sampIndex].segmentId] = 0;
    }
    int32_t beginSampleId = -1;

    for (uint32_t sampIndex = 0; sampIndex < trackInfo->sampleProperties.size; sampIndex++)
    {
        segSizeMap[trackInfo->sampleProperties[sampIndex].segmentId]++;
        if (beginSampleId == -1 && trackInfo->sampleProperties[sampIndex].segmentId == sampleIdx->mCurrentReadSegment)
        {
            beginSampleId = trackInfo->sampleProperties[sampIndex].sampleId;
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

        std::map<uint32_t, std::map<uint32_t, uint8_t*>>::iterator itVideoHrd;
        itVideoHrd = m_videoHeaders.find(qualityRanking);
        if (itVideoHrd == m_videoHeaders.end())
        {
            uint8_t vps[256];
            uint8_t vpsLen = 0;
            uint8_t sps[256];
            uint8_t spsLen = 0;
            uint8_t pps[256];
            uint8_t ppsLen = 0;
            memset(vps, 0, 256);
            memset(sps, 0, 256);
            memset(pps, 0, 256);

            std::vector<VCD::OMAF::DecoderSpecificInfo> parameterSets;
            ret = mReader->getDecoderConfiguration(combinedTrackId, sample, parameterSets);
            if (ret)
            {
                LOG(ERROR) << "Failed to get VPS/SPS/PPS ! " << endl;
                return ret;
            }

            for (auto const& parameter : parameterSets)
            {
                if (parameter.codecSpecInfoType == VCD::MP4::HEVC_VPS)
                {
                    vpsLen = parameter.codecSpecInfoBits.size;
                    for (uint32_t i = 0; i < parameter.codecSpecInfoBits.size; i++)
                    {
                        vps[i] = parameter.codecSpecInfoBits[i];
                    }
                }


                if (parameter.codecSpecInfoType == VCD::MP4::HEVC_SPS)
                {
                    spsLen = parameter.codecSpecInfoBits.size;
                    for (uint32_t i = 0; i < parameter.codecSpecInfoBits.size; i++)
                    {
                        sps[i] = parameter.codecSpecInfoBits[i];
                    }
                }

                if (parameter.codecSpecInfoType == VCD::MP4::HEVC_PPS)
                {
                    ppsLen = parameter.codecSpecInfoBits.size;
                    for (uint32_t i = 0; i < parameter.codecSpecInfoBits.size; i++)
                    {
                        pps[i] = parameter.codecSpecInfoBits[i];
                    }
                }
            }

            std::map<uint32_t, uint8_t*> oneHeader;
            uint32_t headerSize = vpsLen + spsLen + ppsLen;
            uint8_t *headerData = new uint8_t[headerSize];
            if (!headerData)
                return OMAF_ERROR_NULL_PTR;

            memcpy(headerData, vps, vpsLen);
            memcpy(headerData + vpsLen, sps, spsLen);
            memcpy(headerData + vpsLen + spsLen, pps, ppsLen);
            oneHeader.insert(std::make_pair(headerSize, headerData));
            m_videoHeaders.insert(std::make_pair(qualityRanking, oneHeader));

            if (qualityRanking == HIGHEST_QUALITY_RANKING)
            {
                mVPSLen = vpsLen;
                mSPSLen = spsLen;
                mPPSLen = ppsLen;
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

        packet->SetQualityRanking(qualityRanking);
        if (!isExtractor)
        {
            packet->SetSRDInfo(*srd);
        }
        else
        {
            packet->SetQualityNum(2);
            vector<uint32_t> boundLeft(1);// num of quality is limited to 2.
            vector<uint32_t> boundTop(1);
            for (int j = packet->GetRwpk()->numRegions - 1; j >= 0; j--)
            {
                if (packet->GetRwpk()->rectRegionPacking[j].projRegLeft == 0 && packet->GetRwpk()->rectRegionPacking[j].projRegTop == 0
                    && !(packet->GetRwpk()->rectRegionPacking[j].packedRegLeft == 0 && packet->GetRwpk()->rectRegionPacking[j].packedRegTop == 0))
                {
                    boundLeft.push_back(packet->GetRwpk()->rectRegionPacking[j].packedRegLeft);
                    boundTop.push_back(packet->GetRwpk()->rectRegionPacking[j].packedRegTop);
                    break;
                }
            }
            for (int idx = 0; idx < packet->GetQualityNum(); idx++)
            {
                SourceResolution srcRes;
                srcRes.qualityRanking = idx + 1;
                srcRes.top = boundTop[idx];
                srcRes.left = boundLeft[idx];
                srcRes.height = packet->GetRwpk()->packedPicHeight;
                if (idx == 0)
                {
                    srcRes.width = boundLeft[idx+1] - boundLeft[idx];
                }
                else
                {
                    srcRes.width = packet->GetRwpk()->packedPicWidth - boundLeft[idx];
                }
                packet->SetSourceResolution(idx, srcRes);
                //cout<<"sourceRes:"<<srcRes.qualityRanking<<" "<<srcRes.top<<" "<<srcRes.left<<" "<<srcRes.width<<" "<<srcRes.height<<endl;
            }
        }

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
        packet->SetSegID(trackInfo->sampleProperties[beginSampleId - 1].segmentId);
        mPacketLock.lock();
        mPacketQueues[trackID].push_back(packet);
        mPacketLock.unlock();
    }

    LOG(INFO) << "Segment " << trackInfo->sampleProperties[beginSampleId - 1].segmentId << " for track " << trackID << " has been read !" << endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    tracepoint(mthq_tp_provider, T6_read_end_time, trackInfo->sampleProperties[beginSampleId - 1].segmentId);
#endif
#endif
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

        if(refTrackInfo) removeSegment(refTrackInfo->initSegmentId, sampleIdx->mCurrentReadSegment - 1);

    }

    for(auto &it : readTrackInfos)
    {
        SAFE_DELETE(it);
    }
    readTrackInfos.clear();

    return ERROR_NONE;
}

static bool IsSelectionChanged(std::map<int, OmafAdaptationSet*> selection1, std::map<int, OmafAdaptationSet*> selection2)
{
    bool isChanged = false;

    if (selection1.size() && selection2.size())
    {
        if (selection1.size() != selection2.size())
        {
            isChanged = true;
        }
        else
        {
            std::map<int, OmafAdaptationSet*>::iterator it1;
            for (it1 = selection1.begin(); it1 != selection1.end(); it1++)
            {
                OmafAdaptationSet *as1 = it1->second;
                std::map<int, OmafAdaptationSet*>::iterator it2;
                for (it2 = selection2.begin(); it2 != selection2.end(); it2++)
                {
                    OmafAdaptationSet *as2 = it2->second;
                    if (as1 == as2)
                    {
                        break;
                    }
                }
                if (it2 == selection2.end())
                {
                    isChanged = true;
                    break;
                }
            }
        }
    }

    return isChanged;
}

void OmafReaderManager::Run()
{
    bool go_on = true;
    if(NULL == mSource) return;

    bool bSegChange = false;

    mStatus = STATUS_RUNNING;

    bool prevPoseChanged = false;

    while(go_on && mStatus != STATUS_STOPPED){
        mLock.lock();

        // exit the waiting if segment is parsed or wait time is more than 10 mins
        int64_t waitTime = 0;
        while (!mInitSegParsed && waitTime < 600000)
        {
            mLock.unlock();
            ::usleep(1000);
            mLock.lock();
            waitTime++;
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
                    uint32_t qualityRanking = 0;

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
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
                            //trace
                            uint32_t dependentTracksNum = st->depTrackIDs.size();
                            uint32_t *dependentTracksIdx = new uint32_t[dependentTracksNum];
                            if (!dependentTracksIdx)
                                return;

                            memset(dependentTracksIdx, 0, dependentTracksNum * sizeof(uint32_t));
                            std::list<int>::iterator itDep = st->depTrackIDs.begin();
                            uint32_t index = 0;
                            for ( ; itDep != st->depTrackIDs.end(); itDep++)
                            {
                                dependentTracksIdx[index] = (uint32_t)(*itDep);
                                index++;
                            }

                            tracepoint(bandwidth_tp_provider, extractor_track_dependent_info,
                                pExt->GetTrackNumber(), dependentTracksNum, dependentTracksIdx);
                            delete [] dependentTracksIdx;
                            dependentTracksIdx = NULL;

                            const char *dashMode = "static";
                            uint32_t streamsNum = (uint32_t)(info.stream_count);
                            uint64_t *streamBitrate = new uint64_t[streamsNum];
                            if (!streamBitrate)
                                return;

                            memset(streamBitrate, 0, streamsNum * sizeof(uint64_t));

                            float frameRate = (float)(info.stream_info[0].framerate_num) / (float)(info.stream_info[0].framerate_den);
                            uint32_t totalFramesNum = (uint32_t)((info.duration * frameRate) / 1000);
                            tracepoint(bandwidth_tp_provider, segmentation_info,
                                dashMode, segmentDur, frameRate,
                                streamsNum, streamBitrate, totalFramesNum, totalSegNum);

                            delete [] streamBitrate;
                            streamBitrate = NULL;
                            printf("Run here ~~~~~~\n");
#endif
#endif

                            mLock.lock();
                            this->mEOS = true;
                            mLock.unlock();
                            return;
                        }
                    }

                    // exit the waiting if segment downloaded or wait time is more than 10 mins
                    int64_t waitTime = 0;
                    mLock.lock();
                    while (st->sampleIndex.mCurrentReadSegment > st->sampleIndex.mCurrentAddSegment && mStatus!=STATUS_STOPPING && waitTime < 600000)
                    {
                        LOG(INFO) << "New segment " << st->sampleIndex.mCurrentReadSegment << " hasn't come, then wait !" << endl;
                        mLock.unlock();
                        ::usleep(1000);
                        mLock.lock();
                        waitTime++;
                    }
                    mLock.unlock();

                    if( mStatus==STATUS_STOPPING ){
                        mStatus = STATUS_STOPPED;
                          break;
                    }

                    if((uint32_t)(st->segStatus[st->sampleIndex.mCurrentReadSegment]) == (st->depTrackIDs.size() + 1)){
                        LOG(INFO)<<"Now will parse Segment "<<st->sampleIndex.mCurrentReadSegment<<endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
                        tracepoint(mthq_tp_provider, T4_parse_start_time, st->sampleIndex.mCurrentReadSegment);
#endif
#endif
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

                            ParseSegment(st->sampleIndex.mCurrentReadSegment, initSegIndex, qualityRanking, NULL);

                        }

                        ParseSegment(st->sampleIndex.mCurrentReadSegment, initSegID, qualityRanking, NULL);
                        std::vector<TrackInformation*> readTrackInfos = mSegTrackInfos[st->sampleIndex.mCurrentReadSegment];
                        this->ReadNextSegment(trackID, initSegID, true, readTrackInfos, bSegChange, HIGHEST_QUALITY_RANKING, NULL);
                        mSegTrackInfos.erase(st->sampleIndex.mCurrentReadSegment - 1);

                        RemoveReadSegmentFromMap();
                    }else{
                        ::usleep(100);
                    }
                }
            }else{
                std::map<int, OmafAdaptationSet*> mapAS = pStream->GetSelectedTileTracks();
                LOG(INFO)<<"Selected tile tracks number is: "<<mapAS.size()<<" in reader manager!"<<endl;
		        bool hasPoseChanged = false;
		        uint32_t processedNum = 0;
                for(auto as_it = mapAS.begin(); as_it != mapAS.end(); as_it++){
                    OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it->second);
                    SRDInfo srdInfo;
                    //LOG(INFO)<<"Selected track: "<<pAS->GetTrackNumber()<<endl;
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
                            printf("EOS has been gotten \n");
                            return;
                        }
                    }
                    uint16_t trackID = pAS->GetTrackNumber();
                    uint16_t initSegID = 0;
                    uint32_t qualityRanking = 0;
                    for (auto& idPair : mMapInitTrk)
                    {
                        if (idPair.second == trackID)
                        {
                            initSegID = idPair.first;
                            break;
                        }
                    }
                    // exit the waiting if segment downloaded or wait time is more than 10 mins
                    int64_t waitTime = 0;
                    mLock.lock();
                    if (prevPoseChanged)
                    {
                        st->sampleIndex.mCurrentReadSegment = mGlobalReadSegId;
                    }
                    //LOG(INFO)<<st->sampleIndex.mCurrentReadSegment<<" "<<st->sampleIndex.mCurrentAddSegment<<endl;
                    while (st->sampleIndex.mCurrentReadSegment > st->sampleIndex.mCurrentAddSegment && mStatus!=STATUS_STOPPING && waitTime < 600000)
                    {
                        std::map<int, OmafAdaptationSet*> mapAS1 = pStream->GetSelectedTileTracks();
                        bool isPoseChanged = IsSelectionChanged(mapAS, mapAS1);
                        if (isPoseChanged)
                        {
                            LOG(INFO) << "When track  "<< pAS->GetTrackNumber()<< " , pose changed" <<endl;
                            hasPoseChanged = true;
                            mLock.unlock();
                            break;
                        }
                        LOG(INFO) << "New segment " << st->sampleIndex.mCurrentReadSegment << " hasn't come, then wait !" << endl;
                        mLock.unlock();
                        ::usleep(1000);
                        mLock.lock();
                        waitTime++;
                    }
                    if (hasPoseChanged)
                    {
                        prevPoseChanged = true;
                        break;
                    }
                    mLock.unlock();

                    if( mStatus==STATUS_STOPPING ){
                        mStatus = STATUS_STOPPED;
                        break;
                    }

                    LOG(INFO) << "Now will parse Segment  " << st->sampleIndex.mCurrentReadSegment << endl;
                    ParseSegment(st->sampleIndex.mCurrentReadSegment, initSegID, qualityRanking, &srdInfo);

                    std::vector<TrackInformation*> readTrackInfos = mSegTrackInfos[st->sampleIndex.mCurrentReadSegment];
                    this->ReadNextSegment(trackID, initSegID, false, readTrackInfos, bSegChange, qualityRanking, &srdInfo);
                    mSegTrackInfos.erase(st->sampleIndex.mCurrentReadSegment - 1);
                    RemoveReadSegmentFromMap();
                    processedNum++;
                    //LOG(INFO) << "For current frame,  " << processedNum << "  tiles have been read !" << endl;
                    if (processedNum == mapAS.size())
                    {
	                    mGlobalReadSegId = st->sampleIndex.mCurrentReadSegment;
		            }
                }
                if (prevPoseChanged && !hasPoseChanged)
                    prevPoseChanged = false;
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
    if(m_readSegMap.size() < 2) return;

    for(auto &it:m_readSegMap)
    {
        if(m_readSegMap.size() < 2)
            break;
        std::map<uint32_t, OmafSegment*> initSegNormalSeg = it.second;
        if (it.first < mGlobalReadSegId)
        {
            for (auto& itRmSeg : initSegNormalSeg)
            {
                OmafSegment *rmSeg = itRmSeg.second;
                //LOG(INFO)<<"Now will delete seg for track "<<itRmSeg.first<<endl;
                delete rmSeg;
                rmSeg = NULL;
            }
            initSegNormalSeg.clear();
            //LOG(INFO)<<"Now will erase "<<it.first<<" segs"<<endl;
            m_readSegMap.erase(it.first);
        }
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

    if (!trackInfo)
    {
        LOG(ERROR) << "Failed to get track information !" << std::endl;
        return;
    }

    for (size_t index = 0; index < trackInfo->sampleProperties.size; index++)
    {
        if (trackInfo->sampleProperties[index].sampleId == id)
        {
            mMapSegStatus[trackID].sampleIndex.mGlobalSampleIndex = static_cast<uint32_t>(index);
            mMapSegStatus[trackID].sampleIndex.mSegmentSampleIndex = static_cast<uint32_t>(indexInSegment);
            break;
        }
        if (trackInfo->sampleProperties[index].segmentId == mMapSegStatus[trackID].sampleIndex.mCurrentReadSegment)
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
