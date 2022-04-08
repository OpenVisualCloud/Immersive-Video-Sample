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

/*
 * File:   OmafViewTracksSelector.cpp
 * Author: media
 *
 * Created on May 28, 2019, 1:19 PM
 */

#include "OmafViewTracksSelector.h"
#include "OmafMediaStream.h"
#include <cfloat>
#include <math.h>
#include <chrono>
#include <cstdint>
#include "general.h"
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
#include "../trace/MtHQ_tp.h"
#endif
#endif

#define MAX_SELECT_VIEW_NUM 6

VCD_OMAF_BEGIN

int OmafViewTracksSelector::SetInitialViewport(std::vector<Viewport*>& pView, HeadSetInfo* headSetInfo, OmafMediaStream* pStream)
{
    // set current Pose;
    mPose = new HeadPose;
    if (!mPose) return ERROR_NULL_PTR;

    memcpy_s(mPose, sizeof(HeadPose), headSetInfo->pose, sizeof(HeadPose));

    return UpdateViewport(mPose);
}

int OmafViewTracksSelector::UpdateViewport(HeadPose* pose)
{
    if (!pose) return ERROR_NULL_PTR;

    std::lock_guard<std::mutex> lock(mMutex);
    HeadPose* input_pose = new HeadPose;

    if (!(input_pose)) return ERROR_NULL_PTR;
    memcpy_s(input_pose, sizeof(HeadPose), pose, sizeof(HeadPose));
    mPoseHistory.push_front(input_pose);
    if (mPoseHistory.size() > (uint32_t)(this->mSize)) {
        auto pit = mPoseHistory.back();
        SAFE_DELETE(pit);
        mPoseHistory.pop_back();
    }
    return ERROR_NONE;
}

bool OmafViewTracksSelector::IsSelectionChanged(TracksMap selection1, TracksMap selection2)
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

int OmafViewTracksSelector::SelectTracks(OmafMediaStream* pStream, bool isTimed)
{
    DashStreamInfo *streamInfo = pStream->GetStreamInfo();
    int ret = ERROR_NONE;
    if (streamInfo->stream_type == MediaType_Video)
    {
        std::lock_guard<std::mutex> lock(mCurrentMutex);
        TracksMap selectedTracks;

        selectedTracks = SelectViewTracks(pStream, mUseAutoModeForFreeView);

        if (selectedTracks.empty() && m_currentTracks.empty())
            return ERROR_INVALID;

        bool isPoseChanged = IsSelectionChanged(m_currentTracks, selectedTracks);

        if (m_currentTracks.empty() || isPoseChanged)
        {
            if (m_currentTracks.size())
            {
                m_currentTracks.clear();
            }
            m_currentTracks = selectedTracks;
        }
        if (isTimed)
        {
            int32_t stream_frame_rate = round(float(streamInfo->framerate_num) / streamInfo->framerate_den);
            uint32_t sampleNumPerSeg = pStream->GetSegmentDuration() * stream_frame_rate;
            uint64_t curr_pts = (static_cast<uint64_t>(pStream->GetSegmentNumber()) - 1) * sampleNumPerSeg;
            // if existed, replace it
            if (m_prevTimedTracksMap.find(curr_pts) != m_prevTimedTracksMap.end())
            {
                m_prevTimedTracksMap[curr_pts] = m_currentTracks;
            }
            else
            {
                m_prevTimedTracksMap.insert(make_pair(curr_pts, m_currentTracks));
            }

            OMAF_LOG(LOG_INFO, "Insert segid %lld tracks into previous map!\n", curr_pts);
        }
        selectedTracks.clear();
    }

    return ret;
}

int OmafViewTracksSelector::UpdateEnabledTracks(OmafMediaStream* pStream)
{
    DashStreamInfo *streamInfo = pStream->GetStreamInfo();
    int ret = ERROR_NONE;
    if (streamInfo->stream_type == MediaType_Video)
    {
        std::lock_guard<std::mutex> lock(mCurrentMutex);
        ret = pStream->UpdateEnabledTileTracks(m_currentTracks);
    }
    else if (streamInfo->stream_type == MediaType_Audio)
    {
        ret = pStream->EnableAllAudioTracks();
    }
    return ret;
}

bool OmafViewTracksSelector::IsPoseChanged(HeadPose* pose1, HeadPose* pose2)
{
    // return false if two pose is same
    if (pose1->hViewId == pose2->hViewId && pose1->vViewId == pose2->vViewId)
    {
        OMAF_LOG(LOG_INFO, "Pose have not changed!\n");
        return false;
    }
    else
    {
        return true;
    }
}

TracksMap OmafViewTracksSelector::SelectViewTracks(OmafMediaStream* pStream, bool useAutoMode)
{
    TracksMap selectedTracks;
    HeadPose* previousPose = NULL;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if(mPoseHistory.size() == 0)
        {
            return selectedTracks;
        }

        previousPose = mPose;

        mPose = mPoseHistory.front();
        // mPoseHistory.pop_front();

        if(!mPose)
        {
            return selectedTracks;
        }
    }

    if (useAutoMode) selectedTracks = GetViewTracksInAutoMode(pStream, mPose);
    else selectedTracks = GetViewTracksInManualMode(pStream, mPose);

    if (selectedTracks.size() && previousPose)
    {
        OMAF_LOG(LOG_INFO,"pose has changed from horizontal id %d, vertical id %d\n", previousPose->hViewId, previousPose->vViewId);
        OMAF_LOG(LOG_INFO,"to horizontal id %d, vertical id %d\n", mPose->hViewId, mPose->vViewId);
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        // trace
        tracepoint(mthq_tp_provider, T2_detect_pose_change, 1);
#endif
#endif
    }

    return selectedTracks;
}

TracksMap OmafViewTracksSelector::GetViewTracksInManualMode(OmafMediaStream* pStream, HeadPose* pose)
{
    TracksMap selectedTracks;
    vector<pair<int32_t, int32_t>> selectedViewIds;
    // 1. select view id according to current pose
    int32_t currHViewId = pose->hViewId;
    int32_t currVViewId = pose->vViewId;
    // (TBD: configurable motion mode)
    // 1.1 select left views
    uint32_t selectedViewNum = 0;
    for (int32_t h = currHViewId; h >= 0 && selectedViewNum <= MAX_SELECT_VIEW_NUM / 2; h--)
    {
        selectedViewIds.push_back(std::make_pair(h, currVViewId));
        selectedViewNum++;
    }
    // 1.2 select right views
    for (int32_t h = currHViewId + 1; h < MAX_CAMERA_H_NUM && selectedViewNum < MAX_SELECT_VIEW_NUM; h++)
    {
        selectedViewIds.push_back(std::make_pair(h, currVViewId));
        selectedViewNum++;
    }
    // 1.3 supplement left views
    std::sort(selectedViewIds.begin(), selectedViewIds.end(), \
                    [&](std::pair<int32_t, int32_t> view1, std::pair<int32_t, int32_t> view2) { return view1.first < view2.first;});
    while (selectedViewNum < MAX_SELECT_VIEW_NUM) {
        if (selectedViewIds[0].first > 0) {
            selectedViewIds.insert(selectedViewIds.begin(), make_pair(selectedViewIds[0].first - 1, 0));
            selectedViewNum++;
        }
    }
    // 2. insert corresponding viewid tracks into selectedTracks.
    mASMap = pStream->GetMediaAdaptationSet();
    for (auto iter = mASMap.begin(); iter != mASMap.end(); iter++)
    {
        OmafAdaptationSet *adaptationSet = iter->second;
        pair<int32_t, int32_t> tmpViewId = adaptationSet->GetViewID();
        if (find(selectedViewIds.begin(), selectedViewIds.end(), tmpViewId) != selectedViewIds.end())
        {
            int trackID = adaptationSet->GetID();
            selectedTracks.insert(std::make_pair(trackID, adaptationSet));
        }
    }
    return selectedTracks;
}


TracksMap OmafViewTracksSelector::GetViewTracksInAutoMode(OmafMediaStream* pStream, HeadPose* pose)
{
    TracksMap selectedTracks;
    vector<pair<int32_t, int32_t>> selectedViewIds;
    // 1. select view id according to current pose
    static int selectCnt = 0;
    uint32_t view_num = MAX_SELECT_VIEW_NUM;
    uint32_t group_num = 2;
    m_autoSelector->SetSelectorParams(view_num, group_num);
    selectedViewIds = m_autoSelector->GetSelectedViewIds(selectCnt);
    selectCnt++;
    // 2. insert corresponding viewid tracks into selectedTracks.
    mASMap = pStream->GetMediaAdaptationSet();
    for (auto iter = mASMap.begin(); iter != mASMap.end(); iter++)
    {
        OmafAdaptationSet *adaptationSet = iter->second;
        pair<int32_t, int32_t> tmpViewId = adaptationSet->GetViewID();
        if (find(selectedViewIds.begin(), selectedViewIds.end(), tmpViewId) != selectedViewIds.end())
        {
            int trackID = adaptationSet->GetID();
            selectedTracks.insert(std::make_pair(trackID, adaptationSet));
        }
    }
    return selectedTracks;
}

VCD_OMAF_END
