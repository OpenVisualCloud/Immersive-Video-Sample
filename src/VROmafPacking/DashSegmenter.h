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
//! \file:   DashSegmenter.h
//! \brief:  DashInitSegmenter class and DashSegmenter class definition
//! \detail: Define the operation and needed data for DashInitSegmenter
//!          and DashSegmenter classes.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _DASHSEGMENTER_H_
#define _DASHSEGMENTER_H_

#include <fstream>
#include <map>
#include <set>
#include <dlfcn.h>

#include "MediaData.h"
#include "DashSegmentWriterPluginAPI.h"
#include "AcquireTrackData.h"
#include "DataItem.h"

#include "VROmafPacking_def.h"
#include "OmafPackingCommon.h"
#include "MediaStream.h"
#include "ExtractorTrack.h"

VCD_NS_BEGIN

#define AVC_STARTCODES_LEN 4
#define AVC_NALUTYPE_LEN   5

#define DEFAULT_HEVC_TEMPORALIDPLUS1 1

#define DEFAULT_EXTRACTORTRACK_TRACKIDBASE 1000
#define DEFAULT_AUDIOTRACK_TRACKIDBASE     2000

#define DEFAULT_QUALITY_RANK 1
#define MAINSTREAM_QUALITY_RANK 1
#define SECONDRES_STREAM_QUALITY_RANK 2

class TrackSegmentCtx;
class DashInitSegmenter;
class DashSegmenter;

typedef uint32_t StreamId;

//!
//! \enum:   OperatingMode
//! \brief:  define the operation mode, whether OMAF or not
//!
enum OperatingMode
{
    None,
    OMAF
};

//!
//! \enum:   DataInputFormat
//! \brief:  define the format of input data
//!
enum DataInputFormat
{
    VideoMono,
    VideoFramePacked,	// generic, not to be used with OMAF
    VideoTopBottom,
    VideoSideBySide,
    VideoLeft,
    VideoRight,
    Audio,
    VideoExtractor
};

//!
//! \struct: TrackConfig
//! \brief:  define the configuration of the track
//!
struct TrackConfig
{
    VCD::MP4::TrackMeta meta;
    std::map<std::string, std::set<VCD::MP4::TrackId>> trackReferences;
    DataInputFormat pipelineOutput;
};

//!
//! \struct: InitSegConfig
//! \brief:  define the configuration of the initial
//!          segment for one track
//!
struct InitSegConfig
{
    std::map<VCD::MP4::TrackId, TrackConfig> tracks;

    bool fragmented = true;

    bool writeToBitstream = true;

    bool packedSubPictures = false;

    OperatingMode mode = OperatingMode::OMAF;

    std::list<StreamId> streamIds;

    char initSegName[1024];
};

//!
//! \struct: GeneralSegConfig
//! \brief:  define the configuration of the general
//!          data segment for one track
//!
struct GeneralSegConfig
{
    VCD::MP4::FractU64 sgtDuration;

    VCD::MP4::DataItem<VCD::MP4::FractU64> subsgtDuration;

    bool needCheckIDR = true;

    std::map<VCD::MP4::TrackId, VCD::MP4::TrackMeta> tracks;

    VCD::MP4::SequenceId baseSequenceIdx;

    bool useSeparatedSidx;

    std::list<uint32_t> streamsIdx;

    //std::shared_ptr<Log> log;

    char trackSegBaseName[1024];

    bool cmafEnabled = false;

    E_ChunkInfoType chunkInfoType = E_ChunkInfoType::E_NO_CHUNKINFO;
};

//!
//! \struct: TrackInfo
//! \brief:  define the information of one track which
//!          will be changed dynamically during the segmentation
//!
struct TrackInfo
{
    VCD::MP4::FrameTime nextFramePresTime; // used for constructing frame presentation time

    bool isFirstFrame = true;

    int64_t lastPresIndex = 0;

    VCD::MP4::FrameTime nextCodingTime;

    size_t numConsecutiveFrames = 0;

    bool endOfStream = false;
};

//!
//! \class DashInitSegmenter
//! \brief Define the operation and needed data for generating
//!        the initial segment for one track
//!

class DashInitSegmenter
{
public:

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] aConfig
    //!         pointer to the configuration of initial segment
    //!
    DashInitSegmenter(InitSegConfig *aConfig);

    //!
    //! \brief  Destructor
    //!
    ~DashInitSegmenter();

    //!
    //! \brief  Generate the initial segment
    //!
    //! \param  [in] trackSegCtx
    //!         the pointer to the segmentation context
    //!         of the track
    //! \param  [in] tileTrackSegCtxs
    //!         map of tile track and its segmentation context
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateInitSegment(
        TrackSegmentCtx *trackSegCtx,
        std::map<VCD::MP4::TrackId, TrackSegmentCtx*> trackSegCtxs);

    //!
    //! \brief  Get initial segment size
    //!
    //! \return uint64_t
    //!         initial segment size
    //!
    uint64_t GetInitSegmentSize() { return m_initSegSize; };

    void SetSegmentWriter(VCD::MP4::SegmentWriterBase *segWriter)
    {
        m_segWriter = segWriter;
    };

private:
    std::set<VCD::MP4::TrackId>                   m_firstFrameRemaining;         //!< the remaining track index for which initial segment is not generated for the first frame

    const InitSegConfig                           m_config;                      //!< the configuration for the initial segment

    uint64_t                                      m_initSegSize = 0;

    VCD::MP4::SegmentWriterBase                   *m_segWriter = NULL;

private:

    //!
    //! \brief  Add the specified track of AVC coded data into sample
    //!         entry of the initial segment
    //!
    //! \param  [in]aTrackId
    //!         the index of the specified track
    //! \param  [in] aMeta
    //!         meta data of the coded data of the specified track
    //!
    //! \return void
    //!
    //void AddH264VideoTrack(VCD::MP4::TrackId aTrackId, CodedMeta& aMeta);

    //!
    //! \brief  Add the specified track of HEVC coded data into sample
    //!         entry of the initial segment
    //!
    //! \param  [in]aTrackId
    //!         the index of the specified track
    //! \param  [in] aMeta
    //!         meta data of the coded data of the specified track
    //!
    //! \return void
    //!
    //void AddH265VideoTrack(VCD::MP4::TrackId aTrackId, CodedMeta& aMeta);

    //!
    //! \brief  Add the specified track of HEVC extractor track data
    //!         into sample entry of the initial segment
    //!
    //! \param  [in]aTrackId
    //!         the index of the specified track
    //! \param  [in] aMeta
    //!         meta data of the coded data of the specified track
    //!
    //! \return void
    //!
    //void AddH265ExtractorTrack(VCD::MP4::TrackId aTrackId, CodedMeta& aMeta);

    //void AddAACTrack(VCD::MP4::TrackId aTrackId, CodedMeta& aMeta);
    //!
    //! \brief  Make initial segment for the track
    //!
    //! \param  [in]aFragmented
    //!         indicate whether the initial segment is fragmented
    //!
    //! \return InitialSegment
    //!         the initial segment
    //!
    //VCD::MP4::InitialSegment MakeInitSegment(bool aFragmented);

    //!
    //! \brief  Fill the OMAF compliant sample entry
    //!
    //! \param  [in]aTrackId
    //!         the index of the track
    //! \param  [in] aMeta
    //!         meta data of track coded data
    //! \param  [out] aSampleEntry
    //!         OMAF compliant HEVC video sample entry
    //!         which will be written into initial segment
    //! \param  [out] aTrackMeta
    //!         track meta data
    //!
    //! \return void
    //!
    //void FillOmafStructures(
    //    VCD::MP4::TrackId aTrackId,
    //    CodedMeta& aMeta,
    //    VCD::MP4::HevcVideoSampleEntry& aSampleEntry,
    //    VCD::MP4::TrackMeta& aTrackMeta);
};

//!
//! \class DashSegmenter
//! \brief Define the operation of generating data segments for one track
//!

class DashSegmenter
{
public:

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] config
    //!         pointer to the configuration of general data segment
    //! \param  [in] needCreateWriter
    //!         indicate whether to create segment writer
    //!
    DashSegmenter(GeneralSegConfig *config, bool needCreateWriter = true);

    //!
    //! \brief  Destructor
    //!
    ~DashSegmenter();

    //!
    //! \brief  Write the coded data into segment
    //!
    //! \param  [in]trackSegCtx
    //!         the pointer to the segmentation context
    //!         of the track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t SegmentData(TrackSegmentCtx *trackSegCtx);

    //!
    //! \brief  Get totally generated segments number
    //!
    //! \return uint64_t
    //!         totally generated segments number
    //!
    uint64_t GetSegmentsNum() { return m_segNum; };

    //!
    //! \brief  Get current segment size
    //!
    //! \return uint64_t
    //!         current segment size
    //!
    uint64_t GetSegmentSize() { return m_segSize; };

    void SetSegmentWriter(VCD::MP4::SegmentWriterBase *segWriter);

protected:

    //!
    //! \brief  Write the coded data into the segment
    //!
    //! \param  [in]dataNalu
    //!         the pointer to the nalu information of
    //!         the coded data
    //! \param  [in] codedMeta
    //!         meta data of the coded data
    //! \param  [in] outBaseName
    //!         segment base name
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t SegmentOneTrack(
        Nalu *dataNalu,
        VCD::MP4::CodedMeta codedMeta,
        char *outBaseName);

    //!
    //! \brief  Feed the coded data into the segmenter
    //!
    //! \param  [in] trackId
    //!         the index of the track
    //! \param  [in] codedFrameMeta
    //!         meta data of the coded data
    //! \param  [in]dataNalu
    //!         the pointer to the nalu information of
    //!         the coded data
    //! \param  [in] compositionTime
    //!         presentation time of the coded data
    //!
    //! \return void
    //!
    //void Feed(
    //    VCD::MP4::TrackId trackId,
    //    VCD::MP4::CodedMeta codedFrameMeta,
    //    Nalu *dataNalu,
    //    VCD::MP4::FrameCts compositionTime);

    std::map<VCD::MP4::TrackId, TrackInfo>   m_trackInfo;                   //!< track information of all tracks

    const GeneralSegConfig         m_config;                      //!< the configuration of data segment of the track

    bool                           m_createWriter = false;

    VCD::MP4::SegmentWriterBase    *m_segWriter = NULL;

    bool                           m_waitForInitSegment = false;  //!< whether to wait for initial segment

    //!
    //! \brief  Detect the coded data whether comes from reference frame
    //!
    //! \param  [in] frameData
    //!         the pointer to the coded data
    //!
    //! \return bool
    //!         whether the coded data comes from reference frame
    //!
    //bool DetectNonRefFrame(uint8_t *frameData);

    //!
    //! \brief  Write the segment
    //!
    //! \param  [in] aSegments
    //!         the segments
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason 
    //!
    int32_t WriteSegment(char *baseName);

    //!
    //! \brief  Pack all extractors data into bitstream
    //!
    //! \param  [in] extractorsMap
    //!         the pointer to the all extractors map belong to the
    //!         extractor track
    //! \param  [in] refTrackIdxs
    //!         list of reference track index for all extractors
    //! \param  [out] extractorsNalu
    //!         pointer to the nalu information of packed extractors
    //!         bitstream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t PackExtractors(
        std::map<uint8_t, VCD::MP4::Extractor*>* extractorsMap,
        std::list<VCD::MP4::TrackId> refTrackIdxs,
        Nalu *extractorsNalu);

private:

    uint64_t                                                          m_segNum = 0;            //!< current segments number
    uint64_t                                                          m_subSegNum = 0;
    std::ostringstream                                                m_frameStream;
    FILE                                                              *m_file = NULL;          //!< file pointer to write segments
    char                                                              m_segName[1024];           //!< segment file name string
    uint64_t                                                          m_segSize = 0;
    uint64_t                                                          m_prevSegSize = 0;
};

//!
//! \struct: TrackSegmentCtx
//! \brief:  define the context of segmentation for
//!          one track, which will be updated dynamically
//!          during the segmentation
//!
struct TrackSegmentCtx
{
    bool              isAudio;
    bool              isExtractorTrack;

    TileInfo          *tileInfo;
    uint16_t          tileIdx;

    uint16_t          extractorTrackIdx;
    std::map<uint8_t, VCD::MP4::Extractor*>* extractors;
    Nalu              extractorTrackNalu;
    std::list<VCD::MP4::TrackId> refTrackIdxs;

    Nalu              videoNalu; //!< nalu for non-tiled video
    Nalu              audioNalu;

    VCD::MP4::TrackId trackIdx;

    VCD::MP4::SegmentWriterBase *segWriter;
    void              *segWriterPluginHdl;

    InitSegConfig     dashInitCfg;
    GeneralSegConfig  dashCfg;
    DashInitSegmenter *initSegmenter;
    DashSegmenter     *dashSegmenter;

    VCD::MP4::CodedMeta         codedMeta;

    uint8_t           qualityRanking;

    bool              isEOS;

    TrackSegmentCtx()
    {
        isAudio = false;
        isExtractorTrack = false;

        tileInfo = NULL;
        tileIdx = 0;

        extractorTrackIdx = 0;
        extractors = NULL;
        memset_s(&extractorTrackNalu, sizeof(Nalu), 0);

        memset_s(&videoNalu, sizeof(Nalu), 0);
        memset_s(&audioNalu, sizeof(Nalu), 0);

        segWriter = NULL;
        segWriterPluginHdl = NULL;

        initSegmenter = NULL;
        dashSegmenter = NULL;
        qualityRanking = 0;
        isEOS = false;
    }

    ~TrackSegmentCtx()
    {
        if (segWriterPluginHdl)
        {
            if (segWriter)
            {
                VCD::MP4::DestroySegmentWriter* destroySegWriter = NULL;
                destroySegWriter = (VCD::MP4::DestroySegmentWriter*)dlsym(segWriterPluginHdl, "Destroy");
                const char *dlsymErr = dlerror();
                if (dlsymErr)
                {
                    OMAF_LOG(LOG_ERROR, "Failed to load symbol segment writer Destroy !\n");
                    OMAF_LOG(LOG_ERROR, "And get error message %s \n", dlsymErr);
                    return;
                }

                if (!destroySegWriter)
                {
                    OMAF_LOG(LOG_ERROR, "NULL segment writer destructor !\n");
                    return;
                }

                destroySegWriter(segWriter);
            }
        }
    }

    VCD::MP4::SegmentWriterCfg MakeSegmentWriterConfig(
        GeneralSegConfig *dashConfig)
    {
        VCD::MP4::SegmentWriterCfg config {};
        config.segmentDuration = dashConfig->sgtDuration;
        config.subsegmentDuration = dashConfig->subsgtDuration;
        config.checkIDR = dashConfig->needCheckIDR;
        config.useSeparatedSidx = dashConfig->useSeparatedSidx;
        config.chunkInfoType = (VCD::MP4::ChunkInfoType)(dashConfig->chunkInfoType);
        return config;
    }

    int32_t CreateDashSegmentWriter()
    {
        if (!segWriterPluginHdl)
        {
            OMAF_LOG(LOG_ERROR, "NULL segment writer plugin handle !\n");
            return OMAF_ERROR_NULL_PTR;
        }

        VCD::MP4::CreateSegmentWriter* createSegWriter = NULL;
        createSegWriter = (VCD::MP4::CreateSegmentWriter*)dlsym(segWriterPluginHdl, "Create");
        const char *dlsymErr2 = NULL;
        dlsymErr2 = dlerror();
        if (dlsymErr2)
        {
            OMAF_LOG(LOG_ERROR, "Failed to load symbol Create: %s\n", dlsymErr2);
            return OMAF_ERROR_DLSYM;
        }

        if (!createSegWriter)
        {
            OMAF_LOG(LOG_ERROR, "NULL segment writer creator !\n");
            return OMAF_ERROR_NULL_PTR;
        }

        segWriter = createSegWriter(MakeSegmentWriterConfig(&(dashCfg)));
        if (!segWriter)
        {
            OMAF_LOG(LOG_ERROR, "Failed to create segment writer !\n");
            return OMAF_ERROR_NULL_PTR;
        }

        return ERROR_NONE;
    }
};

VCD_NS_END;
#endif /* _DASHSEGMENTER_H_ */
