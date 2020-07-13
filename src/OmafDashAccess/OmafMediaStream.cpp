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
#include "OmafReaderManager.h"
#include "OmafTileTracksSelector.h"

VCD_OMAF_BEGIN

OmafMediaStream::OmafMediaStream()
{
    mMainAdaptationSet     = NULL;
    mExtratorAdaptationSet = NULL;
    m_pStreamInfo          = NULL;
    m_bEOS                 = false;
    mStreamID              = 0;
    m_hasTileTracksSelected = false;
    m_stitchThread         = 0;
    m_enabledExtractor     = true;
    m_stitch               = NULL;
    m_needParams           = false;
    m_currFrameIdx         = 0;
    pthread_mutex_init(&mMutex, NULL);
    pthread_mutex_init(&mCurrentMutex, NULL);
    pthread_mutex_init(&m_packetsMutex, NULL);
}

OmafMediaStream::~OmafMediaStream()
{
    SAFE_DELETE(m_pStreamInfo->codec);
    SAFE_DELETE(m_pStreamInfo->mime_type);
    SAFE_FREE(m_pStreamInfo);
    SAFE_FREE(mMainAdaptationSet);
    m_selectedTileTracks.clear();
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

    if (m_stitchThread)
    {
        pthread_join(m_stitchThread, NULL);
        m_stitchThread = 0;
    }

    if (m_mergedPackets.size())
    {
        std::list<std::list<MediaPacket*>>::iterator it;
        for(it = m_mergedPackets.begin(); it != m_mergedPackets.end(); )
        {
            std::list<MediaPacket*> packets = *it;
            if (packets.size())
            {
                std::list<MediaPacket*>::iterator itPacket;
                for (itPacket = packets.begin(); itPacket != packets.end(); )
                {
                    MediaPacket *packet = *itPacket;
                    SAFE_DELETE(packet);
                    packets.erase(itPacket++);
                }
                packets.clear();
            }
            m_mergedPackets.erase(it++);
        }
        m_mergedPackets.clear();
    }

    SAFE_DELETE(m_stitch);
    pthread_mutex_destroy( &mMutex );
    pthread_mutex_destroy( &mCurrentMutex );
    pthread_mutex_destroy( &m_packetsMutex );
    m_sources.clear();
}

void OmafMediaStream::Close()
{
    if (m_status != STATUS_STOPPED)
    {
        m_status = STATUS_STOPPED;
        if (m_stitchThread)
        {
            pthread_join(m_stitchThread, NULL);
            m_stitchThread = 0;
        }
    }
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

    if (!m_enabledExtractor && !m_stitch)
    {
        m_stitch = new OmafTilesStitch();
        if (!m_stitch)
            return OMAF_ERROR_NULL_PTR;
    }

    UpdateStreamInfo();

    SetupExtratorDependency();

    if (!m_enabledExtractor)
    {
        int32_t ret = StartTilesStitching();
        if (ret)
        {
            LOG(ERROR) << "Failed to start tiles stitching !" << std::endl;
            return ret;
        }
    }

    return ERROR_NONE;
}

void OmafMediaStream::UpdateStreamInfo()
{
    if(!mMediaAdaptationSet.size()) return;
    if (m_enabledExtractor)
    {
        if(NULL != mMainAdaptationSet && NULL != mExtratorAdaptationSet){
            VideoInfo vi = mMainAdaptationSet->GetVideoInfo();
            AudioInfo ai = mMainAdaptationSet->GetAudioInfo();

            m_pStreamInfo->bit_rate      = vi.bit_rate;

            m_pStreamInfo->framerate_den = vi.frame_Rate.den;
            m_pStreamInfo->framerate_num = vi.frame_Rate.num;
            m_pStreamInfo->width         = mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos[0].orig_width;
            m_pStreamInfo->height        = mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos[0].orig_height;
            m_pStreamInfo->mime_type     = new char[1024];
            m_pStreamInfo->codec         = new char[1024];
            memcpy(const_cast<char*>(m_pStreamInfo->mime_type), mMainAdaptationSet->GetMimeType().c_str(), 1024);
            memcpy(const_cast<char*>(m_pStreamInfo->codec), mMainAdaptationSet->GetCodec()[0].c_str(), 1024);
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
    }
    else
    {
        if(NULL != mMainAdaptationSet){
            VideoInfo vi = mMainAdaptationSet->GetVideoInfo();
            AudioInfo ai = mMainAdaptationSet->GetAudioInfo();

            m_pStreamInfo->bit_rate      = vi.bit_rate;

            m_pStreamInfo->framerate_den = vi.frame_Rate.den;
            m_pStreamInfo->framerate_num = vi.frame_Rate.num;
            m_pStreamInfo->height        = vi.height;//mExtratorAdaptationSet->GetVideoInfo().height;
            m_pStreamInfo->width         = vi.width;//mExtratorAdaptationSet->GetVideoInfo().width;
            m_pStreamInfo->mime_type     = new char[1024];
            m_pStreamInfo->codec         = new char[1024];
            memcpy(const_cast<char*>(m_pStreamInfo->mime_type), mMainAdaptationSet->GetMimeType().c_str(), 1024);
            memcpy(const_cast<char*>(m_pStreamInfo->codec), mMainAdaptationSet->GetCodec()[0].c_str(), 1024);
            m_pStreamInfo->mFpt          = (int32_t)mMainAdaptationSet->GetFramePackingType();
            m_pStreamInfo->mProjFormat   = (int32_t)mMainAdaptationSet->GetProjectionFormat();
            m_pStreamInfo->segmentDuration = mMainAdaptationSet->GetSegmentDuration();

            m_pStreamInfo->channel_bytes = ai.channel_bytes;
            m_pStreamInfo->channels      = ai.channels;
            m_pStreamInfo->sample_rate   = ai.sample_rate;

            std::set<uint32_t> allQualities;
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

            for (itAS = mMediaAdaptationSet.begin(); itAS != mMediaAdaptationSet.end(); itAS++)
            {
                OmafAdaptationSet *adaptationSet = itAS->second;
                uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();
                allQualities.insert(qualityRanking);
            }
            std::set<uint32_t>::reverse_iterator itQuality;
            for (itQuality = allQualities.rbegin(); itQuality != allQualities.rend(); itQuality++)
            {
                uint32_t quality = *itQuality;
                int32_t width = 0;
                int32_t height = 0;
                for (itAS = mMediaAdaptationSet.begin(); itAS != mMediaAdaptationSet.end(); itAS++)
                {
                    OmafAdaptationSet *adaptationSet = itAS->second;
                    OmafSrd *srd = adaptationSet->GetSRD();
                    int32_t tileWidth = srd->get_W();
                    int32_t tileHeight = srd->get_H();
                    int32_t tileLeft = srd->get_X();
                    int32_t tileTop  = srd->get_Y();
                    uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();
                    if (qualityRanking == quality)
                    {
                        if (tileTop == 0)
                        {
                            width += tileWidth;
                        }

                        if (tileLeft == 0)
                        {
                            height += tileHeight;
                        }
                    }
                }

                SourceInfo oneSrc;
                oneSrc.qualityRanking = quality;
                oneSrc.width = width;
                oneSrc.height = height;
                m_sources.insert(make_pair(quality, oneSrc));
            }

            int sourceNumber = m_sources.size();
            m_pStreamInfo->source_number = sourceNumber;
            m_pStreamInfo->source_resolution = new SourceResolution[sourceNumber];
            std::map<uint32_t, SourceInfo>::iterator itSrc;
            itSrc = m_sources.begin();
            for (int i = 0; ((i < sourceNumber) && (itSrc != m_sources.end()));i++)
            {
                SourceInfo oneSrc = itSrc->second;

                m_pStreamInfo->source_resolution[i].qualityRanking = oneSrc.qualityRanking;
                m_pStreamInfo->source_resolution[i].width = oneSrc.width;
                m_pStreamInfo->source_resolution[i].height = oneSrc.height;
                itSrc++;
            }
        }
    }

    uint32_t rowNum = 0, colNum = 0;
    for(auto &it:mMediaAdaptationSet)
    {
        OmafAdaptationSet* as = it.second;
        uint32_t qr = as->GetRepresentationQualityRanking();
        // only calculate tile segmentation of the stream with highest resolution
        if(qr == HIGHEST_QUALITY_RANKING)
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

int OmafMediaStream::DownloadInitSegment()
{
    pthread_mutex_lock(&mMutex);
    for(auto it = mMediaAdaptationSet.begin();
             it != mMediaAdaptationSet.end();
             it++ ){
        OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
        pAS->DownloadInitializeSegment();
    }

    if (m_enabledExtractor)
    {
        for(auto extrator_it = mExtractors.begin();
                 extrator_it != mExtractors.end();
                 extrator_it++ ){
            OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
            extractor->DownloadInitializeSegment();
        }
    }

    pthread_mutex_unlock(&mMutex);
    return ERROR_NONE;
}

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

int OmafMediaStream::UpdateEnabledTileTracks(
    std::map<int, OmafAdaptationSet*> selectedTiles)
{
    if (selectedTiles.empty()) return ERROR_INVALID;

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
    m_selectedTileTracks.clear();
    for (auto itAS = selectedTiles.begin(); itAS != selectedTiles.end(); itAS++)
    {
        OmafAdaptationSet *adaptationSet = itAS->second;
        adaptationSet->Enable(true);
        m_selectedTileTracks.insert(make_pair(itAS->first, itAS->second));
    }

    m_hasTileTracksSelected = true;

    pthread_mutex_unlock(&mCurrentMutex);
    pthread_mutex_unlock(&mMutex);

    return ret;
}

int OmafMediaStream::GetTrackCount()
{
    int tracksCnt = 0;
    if (m_enabledExtractor)
    {
        tracksCnt = this->mMediaAdaptationSet.size() + this->mExtractors.size();
    }
    else
    {
        tracksCnt = this->mMediaAdaptationSet.size();
    }
    return tracksCnt;
}

int32_t OmafMediaStream::StartTilesStitching()
{
    int32_t ret = pthread_create(&m_stitchThread, NULL, TilesStitchingThread, this);
    if (ret)
    {
        LOG(ERROR) << "Failed to create tiles stitching thread !" << std::endl;
        return OMAF_ERROR_CREATE_THREAD;
    }

    return ERROR_NONE;
}

void *OmafMediaStream::TilesStitchingThread(void *pThis)
{
    OmafMediaStream *pStream = (OmafMediaStream*)pThis;

    pStream->TilesStitching();

    return NULL;
}

static bool IsSelectionChanged(TracksMap selection1, TracksMap selection2)
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

int32_t OmafMediaStream::TilesStitching()
{
    if (!m_stitch)
    {
        LOG(ERROR) << "Tiles stitching handle hasn't been created !" << std::endl;
        return OMAF_ERROR_NULL_PTR;
    }
    int ret = ERROR_NONE;

    pthread_mutex_lock(&mCurrentMutex);
    while(!m_hasTileTracksSelected)
    {
        pthread_mutex_unlock(&mCurrentMutex);
        usleep(100);
        pthread_mutex_lock(&mCurrentMutex);
    }
    pthread_mutex_unlock(&mCurrentMutex);

    uint64_t currFramePTS = 0;
    std::map<int, OmafAdaptationSet*> mapSelectedAS;
    bool isEOS = false;
    uint32_t waitTimes = 1000;
    bool prevPoseChanged = false;
    std::map<int, OmafAdaptationSet*> prevSelectedAS;
    while(!isEOS && m_status != STATUS_STOPPED)
    {
        //begin to generate tiles merged media packets for each frame
        uint32_t currWaitTimes = 0;

        pthread_mutex_lock(&mCurrentMutex);
        mapSelectedAS = m_selectedTileTracks;
        pthread_mutex_unlock(&mCurrentMutex);

        prevPoseChanged = prevSelectedAS.empty() ? false : IsSelectionChanged(mapSelectedAS, prevSelectedAS);

        bool hasPoseChanged = false;
        bool hasPktOutdated = false;
        std::map<uint32_t, MediaPacket*> selectedPackets;
        for (auto as_it = mapSelectedAS.begin(); as_it != mapSelectedAS.end(); as_it++)
        {
            OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it->second);
            int32_t trackID = pAS->GetTrackNumber();
            MediaPacket *onePacket = NULL;
            if (!(m_stitch->IsInitialized()))
                m_needParams = true;

            if (prevPoseChanged)
                m_needParams = true;

            if (as_it != mapSelectedAS.begin())
            {
                uint64_t pts = READERMANAGER::GetInstance()->GetOldestPacketPTSForTrack(trackID);
                if (pts > currFramePTS)
                {
	            LOG(INFO) << "Packet is outdated ! " << std::endl;
                    hasPktOutdated = true;
                    break;
                }
                else if (pts < currFramePTS)
                {
		    LOG(INFO) << "Old PTS  " << pts << "  occures ! " << std::endl;
                    READERMANAGER::GetInstance()->RemoveOutdatedPacketForTrack(trackID, currFramePTS);
                }
            }

            ret = READERMANAGER::GetInstance()->GetNextFrame(trackID, onePacket, m_needParams);
            currWaitTimes = 0;
            std::map<int, OmafAdaptationSet*> mapSelectedAS1;
            pthread_mutex_lock(&mCurrentMutex);
            mapSelectedAS1 = m_selectedTileTracks;
            pthread_mutex_unlock(&mCurrentMutex);
            bool isPoseChanged = false;
            isPoseChanged = IsSelectionChanged(mapSelectedAS, mapSelectedAS1);
            LOG(INFO)<<"In tiles stitching thread, pose changed: " << isPoseChanged<<endl;
            if ((ret == ERROR_NULL_PACKET) && isPoseChanged)
            {
                hasPoseChanged = true;
                break;
            }

            while ((ret == ERROR_NULL_PACKET) && (currWaitTimes < waitTimes) && m_status != STATUS_STOPPED)
            {
                std::map<int, OmafAdaptationSet*> mapSelectedAS2;
                pthread_mutex_lock(&mCurrentMutex);
                mapSelectedAS2 = m_selectedTileTracks;
                pthread_mutex_unlock(&mCurrentMutex);
                bool isPoseChanged1 = false;
                isPoseChanged1 = IsSelectionChanged(mapSelectedAS, mapSelectedAS2);
                if(isPoseChanged1)
                {
                    hasPoseChanged = true;
                    break;
                }

                usleep(1000);
                currWaitTimes++;
                ret = READERMANAGER::GetInstance()->GetNextFrame(trackID, onePacket, m_needParams);
            }
            if (hasPoseChanged)
            {
                prevPoseChanged = true;
                break;
            }

            if (ret == ERROR_NONE)
            {
                if (onePacket->GetEOS())
                {
                    LOG(INFO) << "EOS has been gotten !" << std::endl;
                    isEOS = true;
                    selectedPackets.insert(std::make_pair((uint32_t)(trackID), onePacket));
                    break;
                }
                if (as_it == mapSelectedAS.begin())
                {
                    currFramePTS = onePacket->GetPTS();
                    LOG(INFO) << "The PTS for this batch is  " << currFramePTS << std::endl;
                }
                selectedPackets.insert(std::make_pair((uint32_t)(trackID), onePacket));
            }
        }

        if (hasPoseChanged || hasPktOutdated)
        {
            for (auto it1 = selectedPackets.begin(); it1 != selectedPackets.end(); )
            {
                MediaPacket *pkt = it1->second;
                SAFE_DELETE(pkt);
                selectedPackets.erase(it1++);
            }
            selectedPackets.clear();

            continue;
        }

        if (!isEOS && (selectedPackets.size() != mapSelectedAS.size()) && (currWaitTimes >= waitTimes)  )
        {
            LOG(INFO) << "Incorrect selected tile tracks packets number for tiles stitching !" << std::endl;
            //return OMAF_ERROR_INVALID_DATA;
        }

        if (!isEOS && !(m_stitch->IsInitialized()))
        {
            ret = m_stitch->Initialize(selectedPackets, m_needParams);
            if (ret)
            {
                LOG(ERROR) << "Failed to initialize stitch class !" << std::endl;
                for (auto it1 = selectedPackets.begin(); it1 != selectedPackets.end(); )
                {
                    MediaPacket *pkt = it1->second;
                    SAFE_DELETE(pkt);
                    selectedPackets.erase(it1++);
                }
                selectedPackets.clear();
                return ret;
            }
        }
        else
        {
            if (!isEOS && m_status != STATUS_STOPPED)
            {
                ret = m_stitch->UpdateSelectedTiles(selectedPackets, m_needParams);
                if (ret)
                {
                    LOG(ERROR) << "Failed to update media packets for tiles merge !" << std::endl;
                    for (auto it1 = selectedPackets.begin(); it1 != selectedPackets.end(); )
                    {
                        MediaPacket *pkt = it1->second;
                        SAFE_DELETE(pkt);
                        selectedPackets.erase(it1++);
                    }
                    selectedPackets.clear();
                    return ret;
                }
            }
        }

        std::list<MediaPacket*> mergedPackets;

        if (isEOS)
        {
            std::map<uint32_t, MediaPacket*>::iterator itPacket1;
            for (itPacket1 = selectedPackets.begin(); itPacket1 != selectedPackets.end(); itPacket1++)
            {
                MediaPacket *packet = itPacket1->second;
                mergedPackets.push_back(packet);
            }
        }
        else
        {
            mergedPackets = m_stitch->GetTilesMergedPackets();
        }

        pthread_mutex_lock(&m_packetsMutex);
        m_mergedPackets.push_back(mergedPackets);
        pthread_mutex_unlock(&m_packetsMutex);

        selectedPackets.clear();
        prevPoseChanged = false;
        prevSelectedAS = mapSelectedAS;
    }

    return ERROR_NONE;
}

std::list<MediaPacket*> OmafMediaStream::GetOutTilesMergedPackets()
{
    std::list<MediaPacket*> outPackets;
    pthread_mutex_lock(&m_packetsMutex);
    if (m_mergedPackets.size())
    {
        outPackets = m_mergedPackets.front();
        m_mergedPackets.pop_front();
    }
    pthread_mutex_unlock(&m_packetsMutex);
    return outPackets;
}

VCD_OMAF_END
