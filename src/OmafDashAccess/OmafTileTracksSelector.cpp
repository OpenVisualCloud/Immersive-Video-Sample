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

    if (m_predictedTracks.size())
    {
        std::map<int, TracksMap>::iterator it;
        for (it = m_predictedTracks.begin(); it != m_predictedTracks.end(); it++)
        {
            if ((it->second).size())
            {
                (it->second).clear();
            }
        }

        m_predictedTracks.clear();
    }
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

int OmafTileTracksSelector::SelectTracks(OmafMediaStream* pStream)
{
    TracksMap selectedTracks;
    if (mUsePrediction)
    {
        std::map<int, TracksMap> predictedTracks = GetTileTracksByPosePrediction(pStream);
        if (predictedTracks.empty())
        {
            if (mPoseHistory.size() < POSE_SIZE)
            {
                selectedTracks = GetTileTracksByPose(pStream);
            }
        }
        else
        {
            std::map<int, TracksMap>::iterator it;
            it = predictedTracks.begin();
            selectedTracks = it->second;
        }

        if (predictedTracks.size())
        {
            std::map<int, TracksMap>::iterator it;
            for (it = predictedTracks.begin(); it != predictedTracks.end(); it++)
            {
                if ((it->second).size())
                {
                    (it->second).clear();
                }
            }

            predictedTracks.clear();
        }
    }
    else
    {
        selectedTracks = GetTileTracksByPose(pStream);
    }

    if (selectedTracks.empty() && m_currentTracks.empty())
        return ERROR_INVALID;

    bool isPoseChanged = IsSelectionChanged(m_currentTracks, selectedTracks);

    if (m_currentTracks.empty() || isPoseChanged)
    {
        if (m_currentTracks.size())
        {
            m_currentTracks.clear();
        }
        //LOG(INFO)<<"************Will update tile tracks selection *****"<<endl;
        m_currentTracks = selectedTracks;
    }
    selectedTracks.clear();

    // add implementation later for not used packet remove
    //std::list<TracksMap> enabledTracks;
    //enabledTracks.push_front(m_currentTracks);

    //if (isPoseChanged || enabledTracks.size() > 1)

    int ret = pStream->UpdateEnabledTileTracks(m_currentTracks);
    return ret;
}

bool IsPoseChanged(HeadPose* pose1, HeadPose* pose2)
{
    // return false if two pose is same
    if(abs(pose1->yaw - pose2->yaw)<1e-3 && abs(pose1->pitch - pose2->pitch)<1e-3)
    {
        LOG(INFO)<<"pose has not changed!"<<std::endl;
        return false;
    }
    return true;
}

TracksMap OmafTileTracksSelector::GetTileTracksByPose(OmafMediaStream* pStream)
{
    TracksMap selectedTracks;

    pthread_mutex_lock(&mMutex);
    if(mPoseHistory.size() == 0)
    {
        pthread_mutex_unlock(&mMutex);
        return selectedTracks;
    }

    HeadPose* previousPose = mPose;
    int64_t historySize = 0;

    mPose = mPoseHistory.front().pose;
    mPoseHistory.pop_front();

    if(!mPose)
    {
        pthread_mutex_unlock(&mMutex);
        return selectedTracks;
    }

    historySize = mPoseHistory.size();

    pthread_mutex_unlock(&mMutex);

    // won't get viewport if pose hasn't changed
    if( previousPose && mPose && !IsPoseChanged( previousPose, mPose ) && historySize > 1)
    {
        LOG(INFO)<<"pose hasn't changed!"<<endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        //trace
        tracepoint(mthq_tp_provider, T2_detect_pose_change, 0);
#endif
#endif
        return selectedTracks;
    }

    // to select tile tracks;
    selectedTracks = SelectTileTracks(pStream, mPose);
    if (selectedTracks.size() && previousPose)
    {
        LOG(INFO)<<"pose has changed from ("<<previousPose->yaw<<","<<previousPose->pitch<<") to ("<<mPose->yaw<<","<<mPose->pitch<<") !"<<endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        // trace
        tracepoint(mthq_tp_provider, T2_detect_pose_change, 1);
#endif
#endif
    }

    if (previousPose != mPose)
        SAFE_DELETE(previousPose);

    return selectedTracks;
}

TracksMap OmafTileTracksSelector::SelectTileTracks(
    OmafMediaStream* pStream,
    HeadPose* pose)
{
    TracksMap selectedTracks;

    // to select tile tracks
    int ret = I360SCVP_setViewPort(m360ViewPortHandle, pose->yaw, pose->pitch);
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
    if (selectedTilesNum <= 0 || selectedTilesNum > 1024)
    {
        LOG(ERROR) << "Failed to get tiles information in viewport !" << endl;
        return selectedTracks;
    }

    std::map<int, OmafAdaptationSet*> asMap = pStream->GetMediaAdaptationSet();
    std::map<int, OmafAdaptationSet*>::iterator itAS;

    // insert all tile tracks in viewport into selected tile tracks map
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

    // insert all tile tracks from low qulity video into selected tile tracks map
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
    return selectedTracks;
}

std::map<int, TracksMap> OmafTileTracksSelector::GetTileTracksByPosePrediction(
    OmafMediaStream *pStream)
{
    std::map<int, TracksMap> predictedTracks;
    pthread_mutex_lock(&mMutex);
    if(mPoseHistory.size() <= 1)
    {
        pthread_mutex_unlock(&mMutex);
        return predictedTracks;
    }

    pthread_mutex_unlock(&mMutex);
    if (mPredictPluginMap.size() == 0)
    {
        LOG(ERROR)<<"predict plugin map is empty!"<<endl;
        return predictedTracks;
    }

    ViewportPredictPlugin *plugin = mPredictPluginMap.at(mPredictPluginName);
    uint32_t pose_interval = POSE_INTERVAL;
    uint32_t pre_pose_count = PREDICTION_POSE_COUNT;
    uint32_t predict_interval = PREDICTION_INTERVAL;
    plugin->Intialize(pose_interval, pre_pose_count, predict_interval);
    std::list<ViewportAngle> pose_history;
    pthread_mutex_lock(&mMutex);
    for (auto it=mPoseHistory.begin(); it!=mPoseHistory.end(); it++)
    {
        ViewportAngle angle;
        angle.yaw = it->pose->yaw;
        angle.pitch = it->pose->pitch;
        pose_history.push_front(angle);
    }
    pthread_mutex_unlock(&mMutex);
    ViewportAngle* predict_angle = plugin->Predict(pose_history);
    if (predict_angle == NULL)
    {
        LOG(ERROR)<<"predictPose_func return an invalid value!"<<endl;
        return predictedTracks;
    }

    HeadPose* previousPose = mPose;
    pthread_mutex_lock(&mMutex);
    mPose = mPoseHistory.front().pose;
    mPoseHistory.pop_front();
    if(!mPose)
    {
        pthread_mutex_unlock(&mMutex);
        return predictedTracks;
    }
    pthread_mutex_unlock(&mMutex);
    // won't get viewport if pose hasn't changed
    if( previousPose && mPose && !IsPoseChanged( previousPose, mPose ) && pose_history.size() > 1)
    {
        LOG(INFO)<<"pose hasn't changed!"<<endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        //trace
        tracepoint(mthq_tp_provider, T2_detect_pose_change, 0);
#endif
#endif
        SAFE_DELETE(previousPose);
        SAFE_DELETE(predict_angle);
        return predictedTracks;
    }

    // to select tile tracks;
    HeadPose *predictPose = new HeadPose;
    if (!predictPose)
        return predictedTracks;

    predictPose->yaw = predict_angle->yaw;
    predictPose->pitch = predict_angle->pitch;
    TracksMap selectedTracks = SelectTileTracks(pStream, predictPose);
    if (selectedTracks.size() && previousPose)
    {
        predictedTracks.insert(make_pair(1, selectedTracks));
        LOG(INFO)<<"pose has changed from ("<<previousPose->yaw<<","<<previousPose->pitch<<") to ("<<mPose->yaw<<","<<mPose->pitch<<") !"<<endl;

#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        //trace
        tracepoint(mthq_tp_provider, T2_detect_pose_change, 1);
#endif
#endif
    }
    SAFE_DELETE(previousPose);
    SAFE_DELETE(predictPose);
    SAFE_DELETE(predict_angle);
    return predictedTracks;
}

VCD_OMAF_END
