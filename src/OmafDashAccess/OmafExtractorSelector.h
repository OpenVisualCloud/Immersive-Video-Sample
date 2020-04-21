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
//! \file:   OmafExtractorSelector.h
//! \brief:
//! \detail:
//! Created on May 28, 2019, 1:19 PM
//!

#ifndef OMAFEXTRACTORSELECTOR_H
#define OMAFEXTRACTORSELECTOR_H

#include "general.h"
#include "OmafExtractor.h"
#include "OmafMediaStream.h"
#include "360SCVPViewportAPI.h"
#include "OmafViewportPredict/ViewportPredictPlugin.h"

using namespace VCD::OMAF;

VCD_OMAF_BEGIN

#define POSE_SIZE 20

typedef std::list<OmafExtractor*> ListExtractor;

typedef struct POSEINFO{
    HeadPose  *pose;
    uint64_t  time;
}PoseInfo;

class OmafExtractorSelector {
public:
    //!
    //! \brief  construct
    //!
    OmafExtractorSelector(int size = POSE_SIZE);

    //!
    //! \brief  de-construct
    //!
    virtual ~OmafExtractorSelector();

public:

    //!
    //! \brief  SelectExtractor for the stream which has extractors. each time
    //!         the selector will select extractor based on the latest pose. the
    //!         information stored in mPoseHistory can be used for prediction for
    //!         further movement
    //!
    int SelectExtractors(OmafMediaStream* pStream);

    //!
    //! \brief  update Viewport; each time pose update will be recorded, but only
    //!         the latest will be used when SelectExtractors is called.
    //!
    int UpdateViewport(HeadPose* pose);

    //!
    //! \brief  Set Init viewport
    //!
    int SetInitialViewport( std::vector<Viewport*>& pView, HeadSetInfo* headSetInfo, OmafMediaStream* pStream);

    void EnablePosePrediction(std::string predictPluginName, std::string libPath)
    {
        mUsePrediction = true;
        mPredictPluginName.assign(predictPluginName);
        mLibPath.assign(libPath);
        InitializePredictPlugins();
    };

private:
    //!
    //! \brief  Get Extractor based on latest Pose
    //!
    OmafExtractor* GetExtractorByPose( OmafMediaStream* pStream );

    //!
    //! \brief  predict Extractor based history Poses
    //!
    ListExtractor GetExtractorByPosePrediction( OmafMediaStream* pStream );

    bool IsDifferentPose(HeadPose* pose1, HeadPose* pose2);

    OmafExtractor* GetNearestExtractor(OmafMediaStream* pStream, CCDef* outCC);

    OmafExtractor* SelectExtractor(OmafMediaStream* pStream, HeadPose* pose);

    int InitializePredictPlugins();

private:
    std::list<PoseInfo>               mPoseHistory;               //<!
    int                               mSize;
    pthread_mutex_t                   mMutex;                     //<! for synchronization
    HeadPose                          *mPose;
    OmafExtractor                     *mCurrentExtractor;
    void                              *m360ViewPortHandle;
    generateViewPortParam             *mParamViewport;
    bool                              mUsePrediction;
    std::string                       mPredictPluginName;
    std::string                       mLibPath;
    std::map<std::string, ViewportPredictPlugin*> mPredictPluginMap;
};

VCD_OMAF_END;

#endif /* OMAFEXTRACTORSELECTOR_H */

