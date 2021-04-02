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
//! \file:   OmafTileTracksSelector.h
//! \brief:  Tile tracks selector class definition
//! \detail: Define the data and the operation of tile tracks selector based on
//!          viewport
//!
//! Created on May 28, 2019, 1:19 PM
//!

#ifndef OMAFTILETRACKSSELECTOR_H
#define OMAFTILETRACKSSELECTOR_H

#include "OmafTracksSelector.h"

using namespace VCD::OMAF;

VCD_OMAF_BEGIN

class OmafTileTracksSelector : public OmafTracksSelector
{
public:
    //!
    //! \brief  construct
    //!
    OmafTileTracksSelector(int size = POSE_SIZE) : OmafTracksSelector(size)
    {
    };

    //!
    //! \brief  de-construct
    //!
    virtual ~OmafTileTracksSelector();

public:

    //!
    //! \brief  Select tile tracks for the stream based on the latest pose. each time
    //!         the selector will select tile tracks based on the latest pose. the
    //!         information stored in mPoseHistory can be used for prediction for
    //!         further movement
    //!
    virtual int SelectTracks(OmafMediaStream* pStream, bool isTimed);

    //!
    //! \brief  Enable tile tracks adaptation sets and update tile tracks list
    //!         according current selected tracks
    //!
    virtual int UpdateEnabledTracks(OmafMediaStream* pStream);

    //!
    //! \brief  Get current tile tracks map after select tracks
    //!
    virtual map<int, OmafAdaptationSet*> GetCurrentTracksMap()
    {
        std::lock_guard<std::mutex> lock(mCurrentMutex);
        return m_currentTracks;
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

    TracksMap GetTileTracksByPose(OmafMediaStream* pStream);

    std::vector<std::pair<ViewportPriority, TracksMap>> GetTileTracksByPosePrediction(OmafMediaStream* pStream);

    //TracksMap GetCoveredTileTracks(OmafMediaStream* pStream, CCDef* outCC);

    TracksMap SelectTileTracks(OmafMediaStream* pStream, HeadPose* pose);

private:
    TracksMap                 m_currentTracks;
};

VCD_OMAF_END;

#endif /* OMAFTILETRACKSSELECTOR_H */
