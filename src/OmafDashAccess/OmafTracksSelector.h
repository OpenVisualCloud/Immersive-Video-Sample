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
//! \file:   OmafTracksSelector.h
//! \brief:  Tracks selector base class definition
//! \detail: Define the operation of tracks selector base class based on viewport
//! Created on May 28, 2019, 1:19 PM
//!

#ifndef OMAFTRACKSSELECTOR_H
#define OMAFTRACKSSELECTOR_H

#include "general.h"
#include "OmafMediaStream.h"
#include "360SCVPViewportAPI.h"
#include "OmafViewportPredict/ViewportPredictPlugin.h"

using namespace VCD::OMAF;

VCD_OMAF_BEGIN

#define POSE_SIZE 10

typedef struct POSEINFO{
    HeadPose  *pose;
    uint64_t  time;
}PoseInfo;

class OmafTracksSelector {
public:
    //!
    //! \brief  construct
    //!
    OmafTracksSelector(int size = POSE_SIZE);

    //!
    //! \brief  de-construct
    //!
    virtual ~OmafTracksSelector();

    //!
    //! \brief  Select tracks for the stream based on the latest pose. each time
    //!         the selector will select tracks based on the latest pose. the
    //!         information stored in mPoseHistory can be used for prediction for
    //!         further movement
    //!
    virtual int SelectTracks(OmafMediaStream *pStream) = 0;

    //!
    //! \brief  Set Init viewport
    //!
    int SetInitialViewport(
        std::vector<Viewport*>& pView,
        HeadSetInfo* headSetInfo,
        OmafMediaStream* pStream);

    //!
    //! \brief  Update Viewport; each time pose update will be recorded, but only
    //!         the latest will be used when SelectTracks is called.
    //!
    int UpdateViewport(HeadPose* pose);

    //!
    //! \brief  Load viewport prediction plugin
    //!
    int EnablePosePrediction(std::string predictPluginName, std::string libPath);

    //!
    //! \brief  Get the priority of the segment
    //!
    //virtual int GetSegmentPriority(OmafSegment *segment) = 0;

private:

    //!
    //! \brief  Initialize viewport prediction plugin
    //!
    int InitializePredictPlugins();

protected:
    std::list<PoseInfo>               mPoseHistory;
    int                               mSize;
    pthread_mutex_t                   mMutex;
    HeadPose                          *mPose;
    void                              *m360ViewPortHandle;
    param_360SCVP                     *mParamViewport;
    bool                              mUsePrediction;
    std::string                       mPredictPluginName;
    std::string                       mLibPath;
    std::map<std::string, ViewportPredictPlugin*> mPredictPluginMap;
};

VCD_OMAF_END;

#endif /* OMAFTRACKSSELECTOR_H */
