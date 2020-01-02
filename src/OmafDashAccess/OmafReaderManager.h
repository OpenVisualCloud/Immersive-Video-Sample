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
//! \file:   OmafReaderManager.h
//! \brief:
//! \detail:
//!
//! Created on May 28, 2019, 1:41 PM
//!

#ifndef OMAFEXTRATORREADER_H
#define OMAFEXTRATORREADER_H

#include "general.h"
#include "OmafReader.h"
#include "MediaPacket.h"
#include "OmafMediaSource.h"
#include "OmafDashSource.h"

VCD_OMAF_BEGIN

typedef std::list<MediaPacket*> PacketQueue;

struct SampleIndex
{
    SampleIndex()
        : mCurrentAddSegment(0)
        , mCurrentReadSegment(1)
        , mSegmentSampleIndex(0)
        , mGlobalStartSegment(0)
        , mGlobalSampleIndex(0)
    {}

    uint32_t mCurrentAddSegment;
    uint32_t mCurrentReadSegment;
    uint32_t mSegmentSampleIndex;
    uint32_t mGlobalStartSegment;
    uint32_t mGlobalSampleIndex;
};

typedef struct SegStatus{
    SampleIndex         sampleIndex;
    std::list<int>      depTrackIDs;
    std::map<int, int>  segStatus;     //<! segment ID & the count of Read depend segment
                                       //<! assuming segment ID is increased synchronized
    std::list<int>      listActiveSeg;
}SegStatus;

class OmafReaderManager : public Threadable{
public:
    OmafReaderManager();
    virtual ~OmafReaderManager();
public:
    //!  \brief initialize the reader with MediaSource
    //!
    int Initialize( OmafMediaSource* pSource );

    //!  \brief close the reader
    //!
    int Close();

    //!  \brief add init Segment for reading after it is downloaded
    //!
    int AddInitSegment( OmafSegment* pInitSeg, uint32_t& nInitSegID );

    //!  \brief add Segment for reading after it is downloaded
    //!
    int AddSegment( OmafSegment* pSeg, uint32_t nInitSegID, uint32_t& nSegID);

    int ParseSegment(uint32_t nSegID, uint32_t nInitSegID);

    //!  \brief Get Next packet from packet queue. each track has a packet queue
    //!
    int GetNextFrame( int trackID, MediaPacket*& pPacket, bool needParams );

    //!  \brief Get initial segments parse status.
    //!
    bool isAllInitSegParsed()
    {
        bool isParsed = false;
        mLock.lock();
        isParsed = mInitSegParsed;
        mLock.unlock();
        return isParsed;
    };

public:
    //!  \brief call when seeking
    //!
    int Seek( );

    void RemoveTrackFromPacketQueue(list<int>& trackIDs);

public:
    //!  \brief the thread routine to read packet for each active track
    //!
    virtual void Run();

private:
    //!  \brief read packet for trackID
    //!
    int  ReadNextSegment(
        int trackID,
        uint16_t initSegID,
        bool isExtractor,
        std::vector<TrackInformation*> readTrackInfos,
        bool& segmentChanged );

    //!  \brief Setup Track information for each stream and relative adaptation set
    //!
    void UpdateSourceTrackID();

    //!  \brief Update SegmentStatus based on stream. if there is extractor in the stream, need
    //!         to considering the segments for each referenced segment by the extractor. Extractor
    //!         can work only all referenced segment are ready.
    void UpdateSegmentStatus(uint32_t nInitSegID, uint32_t nSegID, int64_t segCnt);

    //!  \brief setup the structure to track the IDs and status of all segment
    //!
    void SetupStatusMap();

    //!  \brief release all use Segment
    //!
    void releaseAllSegments( );

    //!  \brief remove segment for reader based on initSegmentID & SegmentID
    //!
    uint32_t removeSegment(uint32_t initSegmentId, uint32_t segmentId);

    //!  \brief release all packets in the packet queues
    //!
    void releasePacketQueue();

    //!  \brief release all use Segment
    //!
    void setNextSampleId(int trackID, uint32_t id, bool& segmentChanged);

    void RemoveReadSegmentFromMap();

private:
    OmafReader*                     mReader;          //<! the Reader implementation
    std::map<int, PacketQueue>      mPacketQueues;    //<! <trackID, PacketQueue>
    std::vector<TrackInformation*>   mTrackInfos;      //<! track information of the opened media
    std::map<uint32_t, std::vector<TrackInformation*>> mSegTrackInfos; //<! seg id and its corresponding track infos
    int                             mCurTrkCnt;       //<! ID base for Init Segment
    OmafMediaSource*                mSource;          //<! reference to the source
    std::map<int, int>              mMapSegCnt;       //<! ID base for segment based on each InitSeg
    std::map<int, SegStatus>        mMapSegStatus;    //<! Segment status for each track
    std::map<int, int>              mMapInitTrk;      //<! ID pair for InitSegID to TrackID;
    ThreadLock                      mLock;            //<! for synchronization
    ThreadLock                      mReaderLock;      //<! lock for reader synchronization
    ThreadLock                      mPacketLock;      //<! lock for packet queue synchronization
    bool                            mEOS;             //<! flag for end of stream
    int                             mStatus;          //<! thread status: 0: runing; 1: stopping, 2. stopped;
    bool                            mReadSync;        //<! need to read  the frame at the bound of I frame (GOP boundary)
    bool                            mInitSegParsed;   //<! flag for noting all initial segments have been parsed
    uint8_t                         mVPS[256];        //<! VPS data
    uint8_t                         mVPSLen;          //<! VPS size
    uint8_t                         mSPS[256];        //<! SPS data
    uint8_t                         mSPSLen;          //<! SPS size
    uint8_t                         mPPS[256];        //<! PPS data
    uint8_t                         mPPSLen;          //<! PPS size
    uint32_t                        mWidth;           //<! sample width
    uint32_t                        mHeight;          //<! sample height
    std::map<uint32_t, std::map<uint32_t, OmafSegment*>> m_readSegMap; //<! map of <segId, std::map<initSegId, Segment>>
};

typedef Singleton<OmafReaderManager> READERMANAGER;

VCD_OMAF_END;

#endif /* OMAFEXTRATORREADER_H */

