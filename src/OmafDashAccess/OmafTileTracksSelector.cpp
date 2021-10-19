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
 * File:   OmafTileTracksSelector.cpp
 * Author: media
 *
 * Created on May 28, 2019, 1:19 PM
 */

#include "OmafTileTracksSelector.h"
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

VCD_OMAF_BEGIN

OmafTileTracksSelector::~OmafTileTracksSelector()
{
    if (m_currentTracks.size())
        m_currentTracks.clear();
}

bool IsSelectionChanged(TracksMap selection1, TracksMap selection2)
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

int OmafTileTracksSelector::SelectTracks(OmafMediaStream* pStream, bool isTimed)
{
    DashStreamInfo *streamInfo = pStream->GetStreamInfo();
    int ret = ERROR_NONE;
    if (streamInfo->stream_type == MediaType_Video)
    {
        TracksMap selectedTracks;
        uint32_t rowSize = pStream->GetRowSize();
        uint32_t colSize = pStream->GetColSize();
        if (mUsePrediction && mPoseHistory.size() >= POSE_SIZE) // using prediction
        {
            std::vector<std::pair<ViewportPriority, TracksMap>>  predictedTracksArray = GetTileTracksByPosePrediction(pStream);
            if (predictedTracksArray.empty()) // Prediction error occurs
            {
                selectedTracks = GetTileTracksByPose(pStream);
            }
            else
            {
                // fetch union set of predictedTracksArray ordered by ViewportPriority
                std::sort(predictedTracksArray.begin(), predictedTracksArray.end(), \
                    [&](std::pair<ViewportPriority, TracksMap> track1, std::pair<ViewportPriority, TracksMap> track2) { return track1.first < track2.first;});
                for (uint32_t i = 0; i < predictedTracksArray.size(); i++)
                {
                    TracksMap oneTracks = predictedTracksArray[i].second;
                    TracksMap::iterator iter = oneTracks.begin();
                    for ( ; iter != oneTracks.end(); iter++)
                    {
                        // ignore when key is identical and have tracks selection limitation.
                        if (selectedTracks.size() <= rowSize * colSize / 2 || selectedTracks.size() < oneTracks.size())
                        {
                            selectedTracks.insert(*iter);
                        }
                        else break;
                    }
                }
            }
            // clear predictedTracksArray
            if (predictedTracksArray.size())
            {
                for (uint32_t i = 0; i < predictedTracksArray.size(); i++)
                {
                    predictedTracksArray[i].second.clear();
                }
                predictedTracksArray.clear();
            }
        }
        else // not using prediction
        {
            selectedTracks = GetTileTracksByPose(pStream);
        }
        {
        std::lock_guard<std::mutex> lock(mCurrentMutex);
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
            int32_t stream_frame_rate = streamInfo->framerate_num / streamInfo->framerate_den;
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
        }
        selectedTracks.clear();
    }

    return ret;
}

int OmafTileTracksSelector::UpdateEnabledTracks(OmafMediaStream* pStream)
{
    DashStreamInfo *streamInfo = pStream->GetStreamInfo();
    int ret = ERROR_NONE;
    if (streamInfo->stream_type == MediaType_Video)
    {
        ret = pStream->UpdateEnabledTileTracks(m_currentTracks);
    }
    else if (streamInfo->stream_type == MediaType_Audio)
    {
        ret = pStream->EnableAllAudioTracks();
    }
    return ret;
}

bool IsPoseChanged(HeadPose* pose1, HeadPose* pose2)
{
    // return false if two pose is same
    if(abs(pose1->yaw - pose2->yaw)<1e-3 && abs(pose1->pitch - pose2->pitch)<1e-3)
    {
        OMAF_LOG(LOG_INFO,"pose has not changed!\n");
        return false;
    }
    return true;
}

TracksMap OmafTileTracksSelector::GetTileTracksByPose(OmafMediaStream* pStream)
{
    TracksMap selectedTracks;
    int64_t historySize = 0;
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

        historySize = mPoseHistory.size();
    }

    // won't get viewport if pose hasn't changed
    if( previousPose && mPose && !IsPoseChanged( previousPose, mPose ) && historySize > 1 && mPose->pts > 0 && !mUsePrediction)
    {
        OMAF_LOG(LOG_INFO,"pose hasn't changed!\n");
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        //trace
        tracepoint(mthq_tp_provider, T2_detect_pose_change, 0);
#endif
#endif
        return selectedTracks;
    }

    // to select tile tracks;
    OMAF_LOG(LOG_INFO, "Start to select tile tracks!\n");
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        // trace
        tracepoint(mthq_tp_provider, T1_select_tracks, "tiletracks");
#endif
#endif

    selectedTracks = SelectTileTracks(pStream, mPose);
    if (selectedTracks.size() && previousPose)
    {
        OMAF_LOG(LOG_INFO,"pose has changed from yaw %f, pitch %f\n", previousPose->yaw, previousPose->pitch);
        OMAF_LOG(LOG_INFO,"to yaw %f, pitch %f\n", mPose->yaw, mPose->pitch);
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        // trace
        tracepoint(mthq_tp_provider, T2_detect_pose_change, 1);
#endif
#endif
    }

    // if (previousPose != mPose)
    //     SAFE_DELETE(previousPose);

    return selectedTracks;
}

TracksMap OmafTileTracksSelector::SelectTileTracks(
    OmafMediaStream* pStream,
    HeadPose* pose)
{
    TracksMap selectedTracks;

    // to select tile tracks
    int ret = I360SCVP_setViewPortEx(m360ViewPortHandle, pose);
    if (ret)
        return selectedTracks;

    ret = I360SCVP_process(mParamViewport, m360ViewPortHandle);
    if (ret)
        return selectedTracks;
    TileDef *tilesInViewport = new TileDef[1024];
    if (!tilesInViewport)
        return selectedTracks;

    Param_ViewportOutput paramViewportOutput;
    int32_t selectedTilesNum = I360SCVP_getTilesInViewport(
            tilesInViewport, &paramViewportOutput, m360ViewPortHandle);

    // in planar projection format
    if (abs(pose->zoomFactor) < 1e-3 && mProjFmt == ProjectionFormat::PF_PLANAR)
    {
        return selectedTracks;
    }

    if (selectedTilesNum <= 0 || selectedTilesNum > 1024)
    {
        OMAF_LOG(LOG_ERROR, "Failed to get tiles information in viewport !\n");
        DELETE_ARRAY(tilesInViewport);
        return selectedTracks;
    }

    std::map<int, OmafAdaptationSet*> asMap = pStream->GetMediaAdaptationSet();
    std::map<int, OmafAdaptationSet*>::iterator itAS;

    // insert all tile tracks in viewport into selected tile tracks map
    uint32_t sqrtedSize = (uint32_t)sqrt(selectedTilesNum);
    while(sqrtedSize && selectedTilesNum%sqrtedSize) { sqrtedSize--; }
    bool needAddtionalTile = false;
    if ((selectedTilesNum > 4) && (sqrtedSize == 1)) // selectedTilesNum is prime number
    {
        OMAF_LOG(LOG_INFO,"need additional tile is true! original selected tile num of high quality is %d\n", selectedTilesNum);
        needAddtionalTile = true;
    }
    if (mProjFmt == ProjectionFormat::PF_ERP)
    {
        for (int32_t index = 0; index < selectedTilesNum; index++)
        {
            int32_t left = tilesInViewport[index].x;
            int32_t top  = tilesInViewport[index].y;

            for (itAS = asMap.begin(); itAS != asMap.end(); itAS++)
            {
                OmafAdaptationSet *adaptationSet = itAS->second;
                OmafSrd *srd = adaptationSet->GetSRD();
                int32_t tileLeft = srd->get_X();
                int32_t tileTop  = srd->get_Y();
                uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();

                if ((qualityRanking == HIGHEST_QUALITY_RANKING) && (tileLeft == left) && (tileTop == top))
                {
                    int trackID = adaptationSet->GetID();
                    selectedTracks.insert(make_pair(trackID, adaptationSet));
                    break;
                }
            }
        }
    }
    else if (mProjFmt == ProjectionFormat::PF_CUBEMAP)
    {
        for (int32_t index = 0; index < selectedTilesNum; index++)
        {
            int32_t left = tilesInViewport[index].x;
            int32_t top  = tilesInViewport[index].y;
            int32_t faceId = tilesInViewport[index].faceId;
            for (itAS = asMap.begin(); itAS != asMap.end(); itAS++)
            {
                OmafAdaptationSet *adaptationSet = itAS->second;
                uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();

                if (qualityRanking == HIGHEST_QUALITY_RANKING)
                {
                    TileDef *tileInfo = adaptationSet->GetTileInfo();
                    if (!tileInfo)
                    {
                        OMAF_LOG(LOG_ERROR, "NULL tile information for Cubemap !\n");
                        DELETE_ARRAY(tilesInViewport);
                        return selectedTracks;
                    }
                    int32_t tileLeft = tileInfo->x;
                    int32_t tileTop  = tileInfo->y;
                    int32_t tileFaceId = tileInfo->faceId;
                    if ((tileLeft == left) && (tileTop == top) && (tileFaceId == faceId))
                    {
                        int trackID = adaptationSet->GetID();
                        selectedTracks.insert(make_pair(trackID, adaptationSet));
                        break;
                    }
                }
            }
        }
    }
    else if (mProjFmt == ProjectionFormat::PF_PLANAR)
    {
        uint32_t corresQualityRanking = 0;
        for (int32_t index = 0; index < selectedTilesNum; index++)
        {
            int32_t left = tilesInViewport[index].x;
            int32_t top  = tilesInViewport[index].y;
            int32_t strId = tilesInViewport[index].streamId;
            map<int32_t, int32_t>::iterator itStrQua;
            itStrQua = mTwoDStreamQualityMap.find(strId);
            if (itStrQua == mTwoDStreamQualityMap.end())
            {
                OMAF_LOG(LOG_ERROR, "Can't find corresponding quality ranking for stream index %d !\n", strId);
                DELETE_ARRAY(tilesInViewport);
                return selectedTracks;
            }

            corresQualityRanking = itStrQua->second;
            OMAF_LOG(LOG_INFO, "Selected tile from stream %d with quality ranking %d\n", strId, corresQualityRanking);

            for (itAS = asMap.begin(); itAS != asMap.end(); itAS++)
            {
                OmafAdaptationSet *adaptationSet = itAS->second;
                OmafSrd *srd = adaptationSet->GetSRD();
                int32_t tileLeft = srd->get_X();
                int32_t tileTop  = srd->get_Y();
                uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();

                if ((qualityRanking == (uint32_t)(corresQualityRanking)) && (tileLeft == left) && (tileTop == top))
                {
                    int trackID = adaptationSet->GetID();
                    OMAF_LOG(LOG_INFO, "Selected track %d\n", trackID);
                    selectedTracks.insert(make_pair(trackID, adaptationSet));
                    break;
                }
            }
        }

        if (needAddtionalTile)
        {
            for (itAS = asMap.begin(); itAS != asMap.end(); itAS++)
            {
                OmafAdaptationSet *adaptationSet = itAS->second;
                uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();
                int trackID = adaptationSet->GetID();
                if (selectedTracks.find(trackID) == selectedTracks.end() && qualityRanking == corresQualityRanking)
                {
                    selectedTracks.insert(make_pair(trackID, adaptationSet));
                    break;
                }
            }
        }
    }

    if (needAddtionalTile && (mProjFmt != ProjectionFormat::PF_PLANAR))
    {
        for (itAS = asMap.begin(); itAS != asMap.end(); itAS++)
        {
            OmafAdaptationSet *adaptationSet = itAS->second;
            uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();
            int trackID = adaptationSet->GetID();
            if (selectedTracks.find(trackID) == selectedTracks.end() && qualityRanking == HIGHEST_QUALITY_RANKING)
            {
                selectedTracks.insert(make_pair(trackID, adaptationSet));
                break;
            }
        }
    }
    // insert all tile tracks from low qulity video into selected tile tracks map when projection type is not PF_PLANAR
    if (mProjFmt != ProjectionFormat::PF_PLANAR)
    {
        for (itAS = asMap.begin(); itAS != asMap.end(); itAS++)
        {
            OmafAdaptationSet *adaptationSet = itAS->second;
            uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();
            if (qualityRanking > HIGHEST_QUALITY_RANKING)
            {
                int trackID = adaptationSet->GetID();
                selectedTracks.insert(make_pair(trackID, adaptationSet));
            }
        }
    }

    DELETE_ARRAY(tilesInViewport);

    return selectedTracks;
}

std::vector<std::pair<ViewportPriority, TracksMap>> OmafTileTracksSelector::GetTileTracksByPosePrediction(
    OmafMediaStream *pStream)
{
    std::vector<std::pair<ViewportPriority, TracksMap>> predictedTracks;
    int64_t historySize = 0;
    HeadPose* previousPose = NULL;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if(mPoseHistory.size() == 0)
        {
            return predictedTracks;
        }

        previousPose = mPose;

        mPose = mPoseHistory.front();
        // mPoseHistory.pop_front();

        if(!mPose)
        {
            return predictedTracks;
        }

        historySize = mPoseHistory.size();

    }
    // won't get viewport if pose hasn't changed
    if( previousPose && mPose && !IsPoseChanged( previousPose, mPose ) && historySize > 1 && mPose->pts > 0)
    {
        OMAF_LOG(LOG_INFO,"pose hasn't changed!\n");
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        //trace
        tracepoint(mthq_tp_provider, T2_detect_pose_change, 0);
#endif
#endif
        // SAFE_DELETE(previousPose);
        return predictedTracks;
    }
    // if viewport changed, then predict viewport using pose history.
    if (mPredictPluginMap.size() == 0)
    {
        OMAF_LOG(LOG_ERROR,"predict plugin map is empty!\n");
        return predictedTracks;
    }
    // 1. figure out the pts of predicted angle
    uint32_t current_segment_num = pStream->GetSegmentNumber();

    DashStreamInfo *stream_info = pStream->GetStreamInfo();
    if (stream_info == nullptr) return predictedTracks;

    int32_t stream_frame_rate = stream_info->framerate_num / stream_info->framerate_den;
    uint64_t first_predict_pts = current_segment_num > 0 ? (current_segment_num - 1) * stream_frame_rate : 0;
    // 2. predict process
    ViewportPredictPlugin *plugin = mPredictPluginMap.at(mPredictPluginName);
    std::map<uint64_t, ViewportAngle*> predict_angles;
    float possibilityOfHalting = 0;
    OMAF_LOG(LOG_INFO, "first_predict_pts %ld\n", first_predict_pts);
    plugin->Predict(first_predict_pts, predict_angles, &possibilityOfHalting);
    // possibility of halting
    OMAF_LOG(LOG_INFO, "Possibility of halting is %f\n", possibilityOfHalting);
    if (predict_angles.empty())
    {
        OMAF_LOG(LOG_INFO,"predictPose_func return an invalid value!\n");
        return predictedTracks;
    }
    // candicate nums to select tile tracks
    uint32_t poseCandicateNum = predict_angles.size();
    HeadPose *predictPose = new HeadPose[poseCandicateNum];
    if (!predictPose)
        return predictedTracks;
    for (uint32_t i = 0; i < poseCandicateNum; i++)
    {
        predictPose[i].yaw = predict_angles[ptsInterval[i] + first_predict_pts]->yaw;
        predictPose[i].pitch = predict_angles[ptsInterval[i] + first_predict_pts]->pitch;
        OMAF_LOG(LOG_INFO, "Start to select tile tracks!\n");
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        // trace
        tracepoint(mthq_tp_provider, T1_select_tracks, "tiletracks");
#endif
#endif
        TracksMap selectedTracks = SelectTileTracks(pStream, &predictPose[i]);
        if (selectedTracks.size() && previousPose)
        {
            predictedTracks.push_back(make_pair(predict_angles[ptsInterval[i] + first_predict_pts]->priority, selectedTracks));
            OMAF_LOG(LOG_INFO,"pose has changed from yaw %f, pitch %f\n", previousPose->yaw, previousPose->pitch);
            OMAF_LOG(LOG_INFO,"to yaw %f, pitch %f\n", mPose->yaw, mPose->pitch);

#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
            //trace
            tracepoint(mthq_tp_provider, T2_detect_pose_change, 1);
#endif
#endif
        }
    }
    // SAFE_DELETE(previousPose);
    SAFE_DELARRAY(predictPose);
    predict_angles.clear();
    return predictedTracks;
}

VCD_OMAF_END
