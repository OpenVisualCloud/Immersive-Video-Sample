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
//! \file:   OmafDashSource.h
//! \brief:
//! \detail:
//! Created on May 22, 2019, 3:18 PM
//!

#ifndef OMAFDASHSOURCE_H
#define OMAFDASHSOURCE_H

#include "general.h"
#include "OmafMediaSource.h"
#include "OmafMediaStream.h"
#include "MediaPacket.h"
#include "OmafMPDParser.h"
#include "DownloadManager.h"
#include "OmafTracksSelector.h"
#include "OmafTilesStitch.h"
#include <mutex>

using namespace VCD::OMAF;

VCD_OMAF_BEGIN

typedef std::list<OmafMediaStream*> ListStream;

class OmafDashSource : public OmafMediaSource, Threadable {
 public:
  //!
  //! \brief construct
  //!
  OmafDashSource();

  //!
  //! \brief de-construct
  //!
  virtual ~OmafDashSource();

 public:
  //!
  //! \brief Interface implementation from base class: OmafMediaSource
  //!
  virtual int OpenMedia(std::string url, std::string cacheDir, void* externalLog, PluginDef i360scvp_plugin, bool enableExtractor = true,
                        bool enablePredictor = false, std::string predictPluginName = "", std::string libPath = "");
  virtual int StartStreaming();
  virtual int CloseMedia();
  virtual int GetPacket(int streamID, std::list<MediaPacket*>* pkts, bool needParams, bool clearBuf);
  virtual int GetStatistic(DashStatisticInfo* dsInfo);
  virtual int SetupHeadSetInfo(HeadSetInfo* clientInfo);
  virtual int ChangeViewport(HeadPose* pose);
  virtual int GetMediaInfo(DashMediaInfo* media_info);
  virtual int GetTrackCount();
  virtual int SelectSpecialSegments(int extractorTrackIdx);
  //!
  //! \brief Interface implementation from base class: Threadable
  //!
  virtual void Run();

 private:
  //!
  //! \brief Select extractors or adaptation set for streams
  //!
  int SelectSegements(bool isTimed);

  //!
  //! \brief
  //!
  void StopThread();

  //!
  //! \brief update mpd in dynamic mode
  //!
  int UpdateMPD();

  //!
  //! \brief Download Segment in dynamic/static mode
  //!
  int DownloadSegments(bool bFirst);

  //!
  //! \brief Download Assigned addtional Segment in dynamic/static mode
  //!
  int DownloadAssignedSegments(std::map<uint32_t, TracksMap> additional_tracks);

  //!
  //! \brief  Enable tracks adaptation sets according current selected tracks
  //!
  int UpdateEnabledTracks();

  //!
  //! \brief run thread for dynamic mpd processing
  //!
  void thread_dynamic();

  //!
  //! \brief run thread for static mpd processing
  //!
  void thread_static();

  //!
  //! \brief run thread for catch-up downloading due to in-time viewport update
  //!
  static void* CatchupThreadWrapper(void*);

  std::map<uint32_t, std::map<int, OmafAdaptationSet*>> GetNewTracksFromDownloaded(std::map<uint32_t, std::map<int, OmafAdaptationSet*>> additional_tracks, std::list<pair<uint32_t, int>> downloadedCatchupTracks, map<uint32_t, uint32_t> catchupTimesInSeg);

  void thread_catchup();
  //!
  //! \brief ClearStreams
  //!
  void ClearStreams();

  //!
  //! \brief SeekToSeg
  //!
  void SeekToSeg(int seg_num);

  //!
  //! \brief SetEOS
  //!
  int SetEOS(bool eos);

  //!
  //! \brief Download init Segment
  //!
  int DownloadInitSeg();

  //!
  //! \brief GetSegmentDuration
  //!
  uint64_t GetSegmentDuration(int stream_id);

  //!
  //! \brief Get and Set current status
  //!
  DASH_STATUS GetStatus() { return mStatus; };
  void SetStatus(DASH_STATUS status) {
    std::lock_guard<std::mutex> lock(mMutex);
    mStatus = status;
  };

  //!
  //! \brief Get MPD information
  //!
  MPDInfo* GetMPDInfo() {
    if (!mMPDParser) return nullptr;

    return mMPDParser->GetMPDInfo();
  };

  int SyncTime(std::string url);

  int StartReadThread();

private:
    OmafDashSource& operator=(const OmafDashSource& other) { return *this; };
    OmafDashSource(const OmafDashSource& other) { /* do not create copies */ };

 private:
  OmafMPDParser* mMPDParser;       //<! the MPD parser
  DASH_STATUS mStatus;             //<! the status of the source
  OmafTracksSelector* m_selector;  //<! tracks selector basing on viewport
  std::mutex mMutex;               //<! for synchronization
  MPDInfo* mMPDinfo;               //<! MPD information
  int dcount;
  int mPreExtractorID;
  OmafTilesStitch* m_stitch = nullptr;
  std::shared_ptr<OmafDashSegmentClient> dash_client_;
  std::shared_ptr<OmafReaderManager> omaf_reader_mgr_;
  bool mIsLocalMedia;
  pthread_t m_catchupThread; //<! catch up thread ID
};

VCD_OMAF_END;

#endif /* OMAFSOURCE_H */
