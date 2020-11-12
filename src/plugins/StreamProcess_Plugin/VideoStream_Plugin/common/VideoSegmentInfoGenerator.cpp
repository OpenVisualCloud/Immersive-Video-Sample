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
//! \file:   VideoSegmentInfoGenerator.cpp
//! \brief:  Video segment information generator class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <utility>
#include "error.h"
#include "VideoSegmentInfoGenerator.h"

VideoSegmentInfoGenerator::VideoSegmentInfoGenerator()
{
    m_bsBuffer = NULL;
    m_segmentationInfo = NULL;
    m_videoSegInfo = NULL;
    m_streamIdx = 0;
}

VideoSegmentInfoGenerator::VideoSegmentInfoGenerator(
    BSBuffer *bs,
    InitialInfo *initInfo,
    uint8_t streamIdx,
    uint16_t videoWidth,
    uint16_t videoHeight,
    uint8_t tileInRow,
    uint8_t tileInCol)
{
    m_bsBuffer = bs;
    m_segmentationInfo = initInfo->segmentationInfo;
    m_streamIdx = streamIdx;

    m_videoSegInfo = new VideoSegmentInfo;
    if (!m_videoSegInfo)
        return;

    m_videoSegInfo->maxWidth = videoWidth;
    m_videoSegInfo->maxHeight = videoHeight;
    m_videoSegInfo->tileInRow = tileInRow;
    m_videoSegInfo->tileInCol = tileInCol;
    m_videoSegInfo->tilesNum = tileInRow * tileInCol;

    memset_s(m_videoSegInfo->tracksSegInfo, 1024 * sizeof(TrackSegmentInfo*), 0);
    for (uint16_t trackIdx = 0; trackIdx <= m_videoSegInfo->tilesNum; trackIdx++)
    {
        m_videoSegInfo->tracksSegInfo[trackIdx] = new TrackSegmentInfo;
        if (!(m_videoSegInfo->tracksSegInfo[trackIdx]))
            return;
    }
}

VideoSegmentInfoGenerator::VideoSegmentInfoGenerator(const VideoSegmentInfoGenerator& src)
{
    m_bsBuffer = std::move(src.m_bsBuffer);
    m_segmentationInfo = std::move(src.m_segmentationInfo);
    m_streamIdx = src.m_streamIdx;
    m_videoSegInfo = std::move(src.m_videoSegInfo);
}

VideoSegmentInfoGenerator::~VideoSegmentInfoGenerator()
{
    if (m_videoSegInfo)
    {
        for (uint16_t trackIdx = 0; trackIdx <= m_videoSegInfo->tilesNum; trackIdx++)
        {
            DELETE_MEMORY(m_videoSegInfo->tracksSegInfo[trackIdx]);
        }

        delete m_videoSegInfo;
        m_videoSegInfo = NULL;
    }
}

void VideoSegmentInfoGenerator::InitFirstTrackInfo(TrackSegmentInfo *trackSegInfo)
{
    trackSegInfo->isoTrackId = 1;
    trackSegInfo->nbSegments = 0;
    trackSegInfo->segmentIndex = 0;
    trackSegInfo->useSourceTiming = 0;
    trackSegInfo->sampleCount = 0;
    trackSegInfo->isoCreated = 0;

    //snprintf(trackSegInfo->repId, sizeof(trackSegInfo->repId), "%d", m_streamIdx);
    //snprintf(trackSegInfo->segInitName, sizeof(trackSegInfo->segInitName), "%s_set1_init.mp4", m_videoSegInfo->outName);
    //snprintf(trackSegInfo->segMediaNameTmpl, sizeof(trackSegInfo->segMediaNameTmpl), "%s_track1_$Number$.m4s", m_videoSegInfo->outName);
    //snprintf(trackSegInfo->segMediaName, sizeof(trackSegInfo->segMediaName), "%s%s_track1_%d.m4s", m_videoSegInfo->dirName, m_videoSegInfo->outName, trackSegInfo->segmentIndex + 1);

}

void VideoSegmentInfoGenerator::InitTileTrackSegInfo(
    TrackSegmentInfo *trackSegInfo,
    TileInfo *tileInfo)
{
    trackSegInfo->dashTrackId = trackSegInfo->tileIdx + 2;
    trackSegInfo->isoTrackId = 1;
    trackSegInfo->nbSegments = 0;
    trackSegInfo->segmentIndex = 0;
    trackSegInfo->useSourceTiming = 0;
    trackSegInfo->sampleCount = 0;
    trackSegInfo->isoCreated = 0;
    trackSegInfo->bitRate = m_videoSegInfo->bitRate / m_videoSegInfo->tilesNum;

    trackSegInfo->tile.horizontalPos = tileInfo->horizontalPos;
    trackSegInfo->tile.verticalPos = tileInfo->verticalPos;
    trackSegInfo->tile.tileWidth = tileInfo->tileWidth;
    trackSegInfo->tile.tileHeight = tileInfo->tileHeight;

    trackSegInfo->dependencyId = m_streamIdx;

    //snprintf(trackSegInfo->repId, sizeof(trackSegInfo->repId),
    //        "%d_%d", m_streamIdx, trackSegInfo->tileIdx+1);
    //snprintf(trackSegInfo->segInitName, sizeof(trackSegInfo->segInitName),
    //        "%s_track%d_init.mp4", m_videoSegInfo->outName, trackSegInfo->dashTrackId);
    //snprintf(trackSegInfo->segMediaNameTmpl, sizeof(trackSegInfo->segMediaNameTmpl),
    //        "%s_track%d_$Number$.m4s", m_videoSegInfo->outName, trackSegInfo->dashTrackId);
    //snprintf(trackSegInfo->segMediaName, sizeof(trackSegInfo->segMediaName),
    //        "%s%s_track%d_%d.m4s", m_videoSegInfo->dirName,
    //        m_videoSegInfo->outName, trackSegInfo->dashTrackId,
    //        trackSegInfo->segmentIndex + 1);
}

int32_t VideoSegmentInfoGenerator::Initialize(TileInfo *tilesInfo)
{
    if (!tilesInfo)
        return OMAF_ERROR_NULL_PTR;

    m_videoSegInfo->totalFrames = 0;
    m_videoSegInfo->nbFrames = 0;
    m_videoSegInfo->bitRate = m_bsBuffer->bitRate;
    Rational inputFrameRate = m_bsBuffer->frameRate;
    m_videoSegInfo->frameRate = inputFrameRate.num / inputFrameRate.den;
    m_videoSegInfo->frameDur = (uint64_t)(1000 / m_videoSegInfo->frameRate); //millisecond
    m_videoSegInfo->firstDtsInFragment = 0;
    m_videoSegInfo->segDur = m_segmentationInfo->segDuration; //second
    m_videoSegInfo->fragDur = m_segmentationInfo->segDuration; //second
    m_videoSegInfo->framePerFragment = m_videoSegInfo->fragDur / m_videoSegInfo->frameDur;
    m_videoSegInfo->framePerSegment = m_videoSegInfo->segDur / m_videoSegInfo->frameDur;

    m_videoSegInfo->fragmentStarted = false;
    m_videoSegInfo->segmentStarted  = false;

    snprintf(m_videoSegInfo->outName, sizeof(m_videoSegInfo->outName),
            "%s%d", m_segmentationInfo->outName, m_streamIdx);
    snprintf(m_videoSegInfo->dirName, sizeof(m_videoSegInfo->dirName),
            "%s", m_segmentationInfo->dirName);

    InitFirstTrackInfo(m_videoSegInfo->tracksSegInfo[0]);

    for (uint8_t tileIdx = 1; tileIdx <= m_videoSegInfo->tilesNum; tileIdx++)
    {
        (m_videoSegInfo->tracksSegInfo[tileIdx])->tileIdx = tileIdx - 1;
        InitTileTrackSegInfo(m_videoSegInfo->tracksSegInfo[tileIdx], &(tilesInfo[tileIdx-1]));
    }

    return ERROR_NONE;
}

void VideoSegmentInfoGenerator::UpdateTrackSegInfo(TrackSegmentInfo *trackSegInfo)
{
    return;
}

void VideoSegmentInfoGenerator::Update()
{
    return;
}
