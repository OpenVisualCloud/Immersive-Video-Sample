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
//! \file:   OmafViewTracksSelector.h
//! \brief:  View tracks selector class definition
//! \detail: Define the data and the operation of view tracks selector based on
//!          viewport
//!
//! Created on May 28, 2019, 1:19 PM
//!

#ifndef OMAFVIEWTRACKSSELECTOR_H
#define OMAFVIEWTRACKSSELECTOR_H

#include "OmafTracksSelector.h"

using namespace VCD::OMAF;

VCD_OMAF_BEGIN

class ViewSelector
{
public:
    ViewSelector() {
        view_num_in_group_ = 0;
        group_num_ = 0;
    }
    virtual ~ViewSelector() {
        start_h_view_ids_.clear();
    }
    void SetSelectorParams(uint32_t view_num, uint32_t group_num) {
        view_num_in_group_ = view_num;
        group_num_ = group_num;
        Initialize();
    }
    vector<pair<int32_t, int32_t>> GetSelectedViewIds(uint32_t seg_id) {
        vector<pair<int32_t, int32_t>> selected_view_ids;
        selected_view_ids.clear();

        uint32_t raw_index = (seg_id - 1) % (group_num_ * 2);
        uint32_t processed_index = raw_index < group_num_ ? raw_index : (group_num_ * 2) - (raw_index + 1);

        if (seg_id == 0) processed_index = 0;// process zero value which is error value

        for (uint32_t i = start_h_view_ids_[processed_index]; i < start_h_view_ids_[processed_index] + view_num_in_group_; i++) {
            selected_view_ids.push_back(make_pair(i, 0));
        }

        return selected_view_ids;
    }

private:

    void Initialize() {
        //1. initialize start_h_view_ids_
        for (uint32_t i = 0; i < group_num_; i++) {
            start_h_view_ids_.push_back(i * view_num_in_group_);
        }
    }

private:
    uint32_t view_num_in_group_;
    uint32_t group_num_;
    vector<int32_t> start_h_view_ids_;
};

enum class AutoMode { LEFT = 0, RIGHT = 1 };

class OmafViewTracksSelector : public OmafTracksSelector
{
public:
    //!
    //! \brief  construct
    //!
    OmafViewTracksSelector(int size = POSE_SIZE) : OmafTracksSelector(size)
    {
        m_autoSelector = new ViewSelector();
    };

    //!
    //! \brief  de-construct
    //!
    virtual ~OmafViewTracksSelector() {
        mASMap.clear();
        if (m_currentTracks.size())
            m_currentTracks.clear();
        SAFE_DELETE(m_autoSelector);
    }

public:

    //!
    //! \brief  Select view tracks for the stream based on the latest pose. each time
    //!         the selector will select view tracks based on the latest pose. the
    //!         information stored in mPoseHistory can be used for prediction for
    //!         further movement
    //!
    virtual int SelectTracks(OmafMediaStream* pStream, bool isTimed);

    //!
    //! \brief  Enable view tracks adaptation sets and update view tracks list
    //!         according current selected tracks
    //!
    virtual int UpdateEnabledTracks(OmafMediaStream* pStream);

    //!
    //! \brief  Get current view tracks map after select tracks
    //!
    virtual map<int, OmafAdaptationSet*> GetCurrentTracksMap()
    {
        std::lock_guard<std::mutex> lock(mCurrentMutex);
        return m_currentTracks;
    }

    virtual bool hasCurrentTracksMap()
    {
        std::lock_guard<std::mutex> lock(mCurrentMutex);
        return !m_currentTracks.empty();
    }
    //!
    //! \brief  update Viewport; each time pose update will be recorded, but only
    //!         the latest will be used when SelectTracks is called.
    //!
    int UpdateViewport(HeadPose* pose);

    //!
    //! \brief  Set Init viewport
    //!
    virtual int SetInitialViewport(std::vector<Viewport*>& pView, HeadSetInfo* headSetInfo, OmafMediaStream* pStream);

    TracksMap GetViewTracksInAutoMode(OmafMediaStream* pStream, HeadPose *pose);

    TracksMap GetViewTracksInManualMode(OmafMediaStream* pStream, HeadPose *pose);

private:

    TracksMap SelectViewTracks(OmafMediaStream* pStream, bool useAutoMode);

    bool IsSelectionChanged(TracksMap selection1, TracksMap selection2);

    bool IsPoseChanged(HeadPose* pose1, HeadPose* pose2);

private:
    TracksMap                 m_currentTracks;
    ViewSelector             *m_autoSelector;
};

VCD_OMAF_END;

#endif /* OMAFVIEWTRACKSSELECTOR_H */
