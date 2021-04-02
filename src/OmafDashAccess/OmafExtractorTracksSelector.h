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
//! \file:   OmafExtractorTracksSelector.h
//! \brief:  Extractor tracks selector class definition
//! \detail: Define the data and the operation of extractor tracks selector
//!          based on viewport
//!
//! Created on May 28, 2019, 1:19 PM
//!

#ifndef OMAFEXTRACTORTRACKSSELECTOR_H
#define OMAFEXTRACTORTRACKSSELECTOR_H

#include "OmafExtractor.h"
#include "OmafTracksSelector.h"

using namespace VCD::OMAF;

VCD_OMAF_BEGIN

typedef std::list<OmafExtractor*> ListExtractor;

class OmafExtractorTracksSelector : public OmafTracksSelector
{
public:
    //!
    //! \brief  construct
    //!
    OmafExtractorTracksSelector(int size = POSE_SIZE) : OmafTracksSelector(size)
    {
        mCurrentExtractor = nullptr;
    };

    //!
    //! \brief  de-construct
    //!
    virtual ~OmafExtractorTracksSelector();

public:

    //!
    //! \brief  SelectTracks for the stream which has extractor tracks. each time
    //!         the selector will select extractor track based on the latest pose. the
    //!         information stored in mPoseHistory can be used for prediction for
    //!         further movement
    //!
    virtual int SelectTracks(OmafMediaStream* pStream, bool isTimed);

    //!
    //! \brief  Enable extractor track and its tile tracks in adaptation set.
    //!
    virtual int UpdateEnabledTracks(OmafMediaStream* pStream);

    //!
    //! \brief  Get current tile tracks map after select tracks
    //!
    virtual map<int, OmafAdaptationSet*> GetCurrentTracksMap()
    {
        return mCurrentExtractor->GetCurrentTracksMap();
    }

    //!
    //! \brief  update Viewport; each time pose update will be recorded, but only
    //!         the latest will be used when SelectTracks is called.
    //!
    int UpdateViewport(HeadPose* pose);

    //!
    //! \brief  Set Init viewport
    //!
    int SetInitialViewport(
        std::vector<Viewport*>& pView,
        HeadSetInfo* headSetInfo,
        OmafMediaStream* pStream);

    //!
    //! \brief  Load viewport prediction plugin
    //!
    int EnablePosePrediction(std::string predictPluginName, std::string libPath);

    //!
    //! \brief  Get the priority of the segment
    //!
    //virtual int GetSegmentPriority(OmafSegment *segment);

private:
    //!
    //! \brief  Get Extractor Track based on latest Pose
    //!
    OmafExtractor* GetExtractorByPose( OmafMediaStream* pStream );

    //!
    //! \brief  predict Extractor Tracks based history Poses
    //!
    ListExtractor GetExtractorByPosePrediction( OmafMediaStream* pStream );

    bool IsDifferentPose(HeadPose* pose1, HeadPose* pose2);

    OmafExtractor* GetNearestExtractor(OmafMediaStream* pStream, CCDef* outCC);

    OmafExtractor* SelectExtractor(OmafMediaStream* pStream, HeadPose* pose);

private:
    OmafExtractor                     *mCurrentExtractor;
    ListExtractor                      mCurrentExtractors;
};

VCD_OMAF_END;

#endif /* OMAFEXTRACTORTRACKSSELECTOR_H */
