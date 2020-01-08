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
 * File:   OmafExtractorSelector.cpp
 * Author: media
 *
 * Created on May 28, 2019, 1:19 PM
 */

#include "OmafExtractorSelector.h"
#include "OmafMediaStream.h"
#include "OmafReaderManager.h"
#include <cfloat>
#include <math.h>
#include <chrono>
#include <cstdint>

VCD_OMAF_BEGIN

OmafExtractorSelector::OmafExtractorSelector( int size )
{
    pthread_mutex_init(&mMutex, NULL);
    mSize = size;
    m360ViewPortHandle = nullptr;
    mParamViewport = nullptr;
    mCurrentExtractor = nullptr;
    mPose = nullptr;
    mUsePrediction = false;
}

OmafExtractorSelector::~OmafExtractorSelector()
{
    pthread_mutex_destroy( &mMutex );

    if(m360ViewPortHandle)
    {
        genViewport_unInit(m360ViewPortHandle);
        m360ViewPortHandle = nullptr;
    }

    if(mParamViewport)
    {
        SAFE_DELETE(mParamViewport->m_pUpLeft);
        SAFE_DELETE(mParamViewport->m_pDownRight);
    }
    SAFE_DELETE(mParamViewport);

    if(mPoseHistory.size())
    {
        for(auto &p:mPoseHistory)
        {
            SAFE_DELETE(p.pose);
        }
    }

    mUsePrediction = false;
}

int OmafExtractorSelector::SelectExtractors(OmafMediaStream* pStream)
{
    OmafExtractor* pSelectedExtrator = GetExtractorByPose( pStream );

    if(NULL == pSelectedExtrator && !mCurrentExtractor)
        return ERROR_NULL_PTR;

    mCurrentExtractor = pSelectedExtrator ? pSelectedExtrator : mCurrentExtractor;

    ListExtractor extractors;

    if(mUsePrediction)
    {
        extractors = GetExtractorByPosePrediction( pStream );
    }

    extractors.push_front(mCurrentExtractor);

    if(pSelectedExtrator || extractors.size() > 1)
    {
        list<int> trackIDs;
        for(auto &it: extractors)
        {
            trackIDs.push_back(it->GetTrackNumber());
        }
        READERMANAGER::GetInstance()->RemoveTrackFromPacketQueue(trackIDs);
    }

    int ret = pStream->UpdateEnabledExtractors(extractors);

    return ret;
}

int OmafExtractorSelector::UpdateViewport(HeadPose* pose)
{
    if (!pose)
        return ERROR_NULL_PTR;

    pthread_mutex_lock(&mMutex);

    PoseInfo pi;
    pi.pose = new HeadPose;
    memcpy(pi.pose, pose, sizeof(HeadPose));
    std::chrono::high_resolution_clock clock;
    pi.time = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    mPoseHistory.push_front(pi);
    if( mPoseHistory.size() > (uint32_t)(this->mSize) )
    {
        auto pit = mPoseHistory.back();
        SAFE_DELETE(pit.pose);
        mPoseHistory.pop_back();
    }

    pthread_mutex_unlock(&mMutex);
    return ERROR_NONE;
}

int OmafExtractorSelector::SetInitialViewport( std::vector<Viewport*>& pView, HeadSetInfo* headSetInfo, OmafMediaStream* pStream)
{
    if(!headSetInfo || !headSetInfo->viewPort_hFOV || !headSetInfo->viewPort_vFOV
        || !headSetInfo->viewPort_Width || !headSetInfo->viewPort_Height)
    {
        return ERROR_INVALID;
    }

    mParamViewport = new generateViewPortParam;
    mParamViewport->m_iViewportWidth = headSetInfo->viewPort_Width;
    mParamViewport->m_iViewportHeight = headSetInfo->viewPort_Height;
    mParamViewport->m_viewPort_fPitch = headSetInfo->pose->pitch;
    mParamViewport->m_viewPort_fYaw = headSetInfo->pose->yaw;
    mParamViewport->m_viewPort_hFOV = headSetInfo->viewPort_hFOV;
    mParamViewport->m_viewPort_vFOV = headSetInfo->viewPort_vFOV;
    mParamViewport->m_output_geoType = headSetInfo->output_geoType;
    mParamViewport->m_input_geoType = headSetInfo->input_geoType;

    mParamViewport->m_iInputWidth = pStream->GetStreamWidth();
    mParamViewport->m_iInputHeight = pStream->GetStreamHeight();

    mParamViewport->m_tileNumRow = pStream->GetRowSize();
    mParamViewport->m_tileNumCol = pStream->GetColSize();
    mParamViewport->m_pUpLeft = new point[6];
    mParamViewport->m_pDownRight = new point[6];

    m360ViewPortHandle = genViewport_Init(mParamViewport);
    if(!m360ViewPortHandle)
        return ERROR_NULL_PTR;

    //set current Pose;
    mPose = new HeadPose;
    memcpy(mPose, headSetInfo->pose, sizeof(HeadPose));

    return UpdateViewport(mPose);
}

bool OmafExtractorSelector::IsDifferentPose(HeadPose* pose1, HeadPose* pose2)
{
    // return false if two pose is same
    if(pose1->yaw == pose2->yaw && pose1->pitch == pose2->pitch)
        return false;

    return true;
}

OmafExtractor* OmafExtractorSelector::GetExtractorByPose( OmafMediaStream* pStream )
{
    pthread_mutex_lock(&mMutex);
    if(mPoseHistory.size() == 0)
    {
        pthread_mutex_unlock(&mMutex);
        return NULL;
    }

    HeadPose* previousPose = mPose;
    int64_t historySize = 0;

    mPose = mPoseHistory.front().pose;
    mPoseHistory.pop_front();

    if(!mPose)
    {
        pthread_mutex_unlock(&mMutex);
        return nullptr;
    }

    historySize = mPoseHistory.size();

    pthread_mutex_unlock(&mMutex);

    // won't get viewport if pose hasn't changed
    if( previousPose && mPose && !IsDifferentPose( previousPose, mPose ) && historySize > 1)
    {
        LOG(INFO)<<"pose hasn't changed!"<<endl;
        return NULL;
    }

    // to select extractor;
    OmafExtractor *selectedExtractor = SelectExtractor(pStream, mPose);
    if(selectedExtractor && previousPose)
        LOG(INFO)<<"pose has changed from ("<<previousPose->yaw<<","<<previousPose->pitch<<") to ("<<mPose->yaw<<","<<mPose->pitch<<") ! extractor id is: "<<selectedExtractor->GetID()<<endl;

    if(previousPose != mPose)
        SAFE_DELETE(previousPose);

    return selectedExtractor;
}

OmafExtractor* OmafExtractorSelector::SelectExtractor(OmafMediaStream* pStream, HeadPose* pose)
{
    // to select extractor;
    int ret = genViewport_setViewPort(m360ViewPortHandle, pose->yaw, pose->pitch);
    if(ret != 0)
        return NULL;
    ret = genViewport_process(mParamViewport, m360ViewPortHandle);
    if(ret != 0)
        return NULL;

    // get Content Coverage from 360SCVP library
    CCDef* outCC = new CCDef;
    ret = genViewport_getContentCoverage(m360ViewPortHandle, outCC);
    if(ret != 0)
        return NULL;

    // get the extractor with largest intersection
    OmafExtractor *selectedExtractor = GetNearestExtractor(pStream, outCC);

    if(outCC)
    {
        delete outCC;
        outCC = nullptr;
    }

    return selectedExtractor;
}

OmafExtractor* OmafExtractorSelector::GetNearestExtractor(OmafMediaStream* pStream, CCDef* outCC)
{
    // calculate which extractor should be chosen
    OmafExtractor *selectedExtractor = nullptr;
    float leastDistance = FLT_MAX;
    std::map<int, OmafExtractor*> extractors = pStream->GetExtractors();
    for(auto &ie: extractors)
    {
        ContentCoverage* cc = ie.second->GetContentCoverage();
        if(!cc)
            continue;

        int32_t ca = cc->coverage_infos[0].centre_azimuth;
        int32_t ce = cc->coverage_infos[0].centre_elevation;

        // for now, every extractor has the same azimuth_range and elevation_range
        // , so we just calculate least Euclidean distance between centres to find the
        // extractor with largest intersection
        float distance = sqrt( pow((ca - outCC->centreAzimuth), 2) + pow((ce - outCC->centreElevation), 2) );
        if(distance < leastDistance)
        {
            leastDistance = distance;
            selectedExtractor =  ie.second;
        }
    }

    return selectedExtractor;
}

ListExtractor OmafExtractorSelector::GetExtractorByPosePrediction( OmafMediaStream* pStream )
{
    ListExtractor extractors;
    pthread_mutex_lock(&mMutex);
    if(mPoseHistory.size() <= 1)
    {
        pthread_mutex_unlock(&mMutex);
        return extractors;
    }

    PoseInfo pf, pb;
    pf = mPoseHistory.front();
    pb = mPoseHistory.back();
    pthread_mutex_unlock(&mMutex);

    std::chrono::high_resolution_clock clock;
    uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();

    // simple prediction, assuming the move is uniform
    float yaw = (pb.pose->yaw - pf.pose->yaw)/(pb.time - pf.time) * (time - pb.time + 1000) + pb.pose->yaw;
    float pitch = (pb.pose->pitch - pf.pose->pitch)/(pb.time - pf.time) * (time - pb.time + 1000) + pb.pose->pitch;
    HeadPose* pose = new HeadPose;
    pose->yaw = yaw;
    pose->pitch = pitch;
    // to select extractor;
    OmafExtractor *selectedExtractor = SelectExtractor(pStream, pose);
    if(selectedExtractor)
        extractors.push_back(selectedExtractor);
    SAFE_DELETE(pose);
    return extractors;
}

VCD_OMAF_END
