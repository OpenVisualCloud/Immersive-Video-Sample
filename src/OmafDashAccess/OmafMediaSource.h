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
//! \file:   OmafMediaSource.h
//! \brief:  the root class of Dash Access Lib Media Source
//! \detail: this is the root class to provide overall OMAF Dash Access
//!          methods
//!
//! Created on May 22, 2019, 11:14 AM
//!

#ifndef _MEDIASOURCE_H
#define _MEDIASOURCE_H

#include "OmafMediaStream.h"
#include "OmafTypes.h"
#include "general.h"

VCD_OMAF_BEGIN

typedef enum {
  STATUS_CREATED = 0,
  STATUS_READY,
  STATUS_RUNNING,
  STATUS_EXITING,
  STATUS_STOPPED,
  STATUS_UNKNOWN,
} DASH_STATUS;

class OmafMediaSource {
 public:
  //!
  //! \brief  construct
  //!
  OmafMediaSource() {
    mViewPortChanged = false;
    memset(&mHeadSetInfo, 0, sizeof(mHeadSetInfo));
    memset(&mPose, 0, sizeof(mPose));
    mLoop = false;
    mEOS = false;
  };

  //!
  //! \brief  de-construct
  //!
  virtual ~OmafMediaSource(){};

 public:
  //!
  //! \brief  Open Media from special url. it's pure interface
  //!
  //! \param  [in] url
  //!         the location of the mpd to be opened
  //! \param  [in] cacheDir
  //!         path to cache files it can be "" for no cache needed
  //!
  //! \return
  //!         ERROR_NONE if success, else fail reason
  //!
  virtual int OpenMedia(std::string url, std::string cacheDir, void* externalLog, PluginDef i360scvp_plugin, bool enableExtractor, bool enablePredictor = false,
                        std::string predictPluginName = "", std::string dllPath = "") = 0;

  virtual int StartStreaming() = 0;
  //!
  //! \brief  Close the media. it's pure interface
  //!
  //! \return
  //!         loop status
  //!
  virtual int CloseMedia() = 0;

  //!
  //! \brief  Open Media from special url. it's pure interface
  //!
  //! \param  [in] streamID
  //!         the ID of the stream to be operated
  //! \param  [out] pkt
  //!         Packet to hold the media stream
  //! \param  [out] clearBuf
  //!
  //! \return
  //!         ERROR_NONE if success, else fail reason
  //!
  virtual int GetPacket(int streamID, std::list<MediaPacket*>* pkts, bool needParams, bool clearBuf) = 0;

  //!
  //! \brief  Open Media from special url. it's pure interface
  //!
  //! \param  [in] media_info
  //!
  //!
  //! \return
  //!         ERROR_NONE if success, else fail reason
  //!
  virtual int GetMediaInfo(DashMediaInfo* media_info) = 0;

  //!
  //! \brief  Open Media from special url.
  //!
  //! \param  [in] streamID
  //!
  //!
  //! \return
  //!         MediaStream* the pointer to the stream
  //!
  virtual OmafMediaStream* GetStream(int streamID) {
    if ((uint32_t)(streamID) >= mMapStream.size() || streamID < 0) return nullptr;
    std::map<int, OmafMediaStream*>::iterator it = mMapStream.find(streamID);
    if (it != mMapStream.end()) {
      return it->second;
    }
    return nullptr;
  };

  //!
  //! \brief  Get the stream number of the media
  //!
  //! \return
  //!         the total count of the stream in the media
  //!
  virtual int GetStreamCount() { return mMapStream.size(); };

  //!
  //! \brief  set initial viewport for the stream. it's pure interface
  //!
  //! \param  [in] clientInfo
  //!
  //!
  //! \return
  //!         ERROR_NONE if success, else fail reason
  //!
  virtual int SetupHeadSetInfo(HeadSetInfo* clientInfo) = 0;

  //!
  //! \brief  Change the viewport for the media. it's pure interface
  //!
  //! \param  [in] pose
  //!
  //!
  //! \return
  //!         ERROR_NONE if success, else fail reason
  //!
  virtual int ChangeViewport(HeadPose* pose) = 0;

  //!
  //! \brief  Get statistic information relative to the media. it's pure interface
  //! \param  [out] info
  //!         the information of statistic, such as bandwidth
  //! \return
  //!         the total count of the stream in the media
  //!
  virtual int GetStatistic(DashStatisticInfo* dsInfo) = 0;

  //!
  //! \brief  seek to special position of the media in VOD mode
  //!
  //! \return
  //!         ERROR_NONE if success, else fail reason
  //!
  virtual int SeekTo(int64_t time)  // need to implement in later version
  {
    if (time < 0) {
      return ERROR_INVALID;
    }
    return ERROR_NONE;
  };

  //!
  //! \brief  Get total track count of the media
  //!
  //! \return
  //!         EOF status
  //!
  virtual int GetTrackCount() = 0;

  //!
  //! \brief  access the media with a loop or non-loop mode in VOD mode
  //!
  //! \return
  //!         ERROR_NONE if success, else fail reason
  //!
  virtual int SetLoop(bool bLoop) {
    mLoop = bLoop;
    return 0;
  }

  //!
  //! \brief  Check whether it is loop mode or not
  //!
  //! \return
  //!         loop status
  //!
  virtual bool IsLoop() { return mLoop; }

  //!
  //! \brief  Check whether it is End of stream
  //!
  //! \return
  //!         EOF status
  //!
  bool isEOS() { return mEOS; };

  virtual int SelectSpecialSegments(int extractorTrackIdx) = 0;

 public:
  void SetOmafDashParams(OmafDashParams params) { omaf_dash_params_ = params; };
  OmafDashParams GetOmafParams() { return omaf_dash_params_; };

 protected:
  OmafDashParams omaf_dash_params_;
  std::string mUrl;                            //!< the url of the media
  std::string mCacheDir;                       //!< the path for cached files
  std::map<int, OmafMediaStream*> mMapStream;  //!< map for streams in the media
  bool mLoop;                                  //!< loop status
  bool mEOS;                                   //!< EOS status
  std::vector<Viewport*> mViewPorts;           //!<
  HeadSetInfo mHeadSetInfo;                    //!<
  HeadPose mPose;                              //!<
  bool mViewPortChanged;                       //!<
};

VCD_OMAF_END;

#endif /* MEDIASOURCE_H */
