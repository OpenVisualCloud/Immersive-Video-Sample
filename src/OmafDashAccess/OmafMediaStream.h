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
//! \file:   OmafMediaStream.h
//! \brief:
//! \detail:
//! Created on May 22, 2019, 2:22 PM
//!


#ifndef OMAFMEDIASTREAM_H
#define OMAFMEDIASTREAM_H

#include "general.h"
#include "OmafReader.h"
#include "OmafAdaptationSet.h"
#include "OmafExtractor.h"
#include "MediaPacket.h"

VCD_OMAF_BEGIN

class OmafMediaStream{
public:
    //!
    //! \brief  construct
    //!
    OmafMediaStream();

    //!
    //! \brief  de-construct
    //!
    virtual ~OmafMediaStream();

public:

    //!
    //! \brief update the start number of the segment for dynamical mode
    //! \param nAvailableStartTime used to calculate start number when accessed
    //!        mpd the first: (now - nAvailableStartTime)/segment_duration + Adaption_Strart_number
    //! \return
    int UpdateStartNumber(uint64_t nAvailableStartTime);

    //!
    //! \brief  download initialize segment for each AdaptationSet
    //!
    int DownloadInitSegment();

    //!
    //! \brief  download all segments for all AdaptationSets.
    //!
    int DownloadSegments();

    //!
    //! \brief  Add extractor Adaptation Set
    //!
    int AddExtractor(OmafExtractor* pAS);

    //!
    //! \brief  Add normal Adaptation Set
    //!
    int AddAdaptationSet(OmafAdaptationSet* pAS);

    //!
    //! \brief  Initialize the stream
    //!
    int InitStream(std::string type);

    //!
    //! \brief  SetMainAdaptationSet if there is
    //!
    void SetMainAdaptationSet(OmafAdaptationSet* as){
        mMainAdaptationSet = as;
    }

    //!
    //! \brief  SetExtratorAdaptationSet if there is
    //!
    void SetExtratorAdaptationSet(OmafAdaptationSet* as){
        mExtratorAdaptationSet = as;
    }

    //!
    //! \brief  Seek to special segment and is is valid in static mode
    //!
    int SeekTo( int seg_num );

    //!
    //! \brief  Set EOS for the stream
    //!
    int SetEOS(bool eos) { m_bEOS = eos; return 0;};

    //!
    //! \brief  get all extractors relative to this stream
    //!
    std::map<int, OmafExtractor*> GetExtractors() {
        return mExtractors;
    };

    //!
    //! \brief  get all Adaptation set relative to this stream
    //!
    std::map<int, OmafAdaptationSet*> GetMediaAdaptationSet() {
        return mMediaAdaptationSet;
    };

    //!
    //! \brief  Update selected extractor after viewport changed
    //!
    int UpdateEnabledExtractors(std::list<OmafExtractor*> extractors);

    //!
    //! \brief  Get count of tracks
    //!
    int GetTrackCount();

    //!
    //! \brief  Get/Set stream ID
    //!
    void SetStreamID(int streamID) { mStreamID = streamID; };
    int GetStreamID() { return mStreamID; };

    //!
    //! \brief  get Stream information
    //!
    DashStreamInfo* GetStreamInfo(){ return m_pStreamInfo; };

    //!
    //! \brief  get current selected extractors
    //!
    std::list<OmafExtractor*> GetEnabledExtractor()
    {
        pthread_mutex_lock(&mCurrentMutex);
        std::list<OmafExtractor*> enabledExtractor (mCurrentExtractors.begin(), mCurrentExtractors.end());
        pthread_mutex_unlock(&mCurrentMutex);
        return enabledExtractor;
    };

    int32_t GetExtractorSize() {return mCurrentExtractors.size(); };

    int32_t GetTotalExtractorSize() {return mExtractors.size(); };

    void ClearEnabledExtractors() { mCurrentExtractors.clear(); };

    OmafExtractor* AddEnabledExtractor(int extractorTrackIdx)
    {
        auto it = mExtractors.find(extractorTrackIdx);
        if (it != mExtractors.end())
        {
            mCurrentExtractors.push_back(it->second);
            return (it->second);
        }
        else
        {
            return NULL;
        }
    };

    //!
    //! \brief  Check whether extractor tracks exists
    //!
    bool HasExtractor(){ return !( 0==mExtractors.size()); };

    //!
    //! \brief  Get segment duration
    //!
    uint64_t GetSegmentDuration() { return m_pStreamInfo ? m_pStreamInfo->segmentDuration : 0; };

    uint32_t GetStreamWidth() {return m_pStreamInfo ? m_pStreamInfo->width : 0;};

    uint32_t GetStreamHeight() {return m_pStreamInfo ? m_pStreamInfo->height : 0;};

    uint32_t GetRowSize(){return m_pStreamInfo ? m_pStreamInfo->tileRowNum : 0;};

    uint32_t GetColSize(){return m_pStreamInfo ? m_pStreamInfo->tileColNum : 0;};

private:
    //!
    //! \brief  UpdateStreamInfo
    //!
    void UpdateStreamInfo( );

    //!
    //! \brief  SetupExtratorDependency
    //!
    void SetupExtratorDependency();

private:
    std::map<int, OmafAdaptationSet*> mMediaAdaptationSet;            //<! Adaptation Set list for tiles
    std::map<int, OmafExtractor*>     mExtractors;                  //<! Adaptation Set list for extractor
    std::list<OmafExtractor*>         mCurrentExtractors;           //<! the current extractors to be dealt with
    OmafAdaptationSet*                mMainAdaptationSet;           //<! the main AdaptationSet, it can be exist or not
    OmafAdaptationSet*                mExtratorAdaptationSet;       //<! the Extrator AdaptationSet
    int                               mStreamID;                    //<! stream ID
    DashStreamInfo                   *m_pStreamInfo;                //<! the information of the stream
    pthread_mutex_t                   mMutex;                       //<! for synchronization
    pthread_mutex_t                   mCurrentMutex;                //<! for synchronization of mCurrentExtractors
    bool                              m_bEOS;                       //<! flag for end of stream

};

VCD_OMAF_END;

#endif /* MEDIASTREAM_H */

