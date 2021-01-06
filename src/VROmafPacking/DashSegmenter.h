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

#include "../isolib/dash_writer/SegmentWriter.h"
#include "../isolib/dash_writer/AcquireTrackData.h"
#include "../isolib/dash_writer/DataItem.h"

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
//! \enum:   CodedFormat
//! \brief:  indicate the coded format of the data
//!
enum CodedFormat
{
    NoneFormat,
    H264,
    H265,
    AAC,
    TimedMetadata,
    H265Extractor
};

//!
//! \enum:   ConfigType
//! \brief:  indicate the paramter set type
//!
enum ConfigType
{
    SPS,                    // for AVC and HEVC
    PPS,                    // for AVC and HEVC
    VPS,                    // for HEVC
    AudioSpecificConfig     // for AAC
};

//!
//! \enum:   FrameType
//! \brief:  indicate the frame type, whether IDR or not
//!
enum FrameType
{
    NA,
    IDR,
    NONIDR,
    // more types directly from the H264 header
};

//!
//! \enum:   OmafProjectionType
//! \brief:  define the picture projection type
//!
enum OmafProjectionType
{
    NoneProjection,
    EQUIRECTANGULAR,
    CUBEMAP,
    PLANAR
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
//! \struct: Bitrate
//! \brief:  define the bitrate information of input data
//!
struct Bitrate
{
    uint32_t avgBitrate;
    uint32_t maxBitrate;
};

//!
//! \struct: SegmenterMeta
//! \brief:  define the meta data of the segment
//!
struct SegmenterMeta
{
    // the duration of the produced segment
    VCD::MP4::FrameDuration segmentDuration;
};

//!
//! \struct: Region
//! \brief:  define the region wise packing information
//!          for one region
//!
struct Region
{
    uint32_t projTop;
    uint32_t projLeft;
    uint32_t projWidth;
    uint32_t projHeight;
    int32_t transform;
    uint16_t packedTop;
    uint16_t packedLeft;
    uint16_t packedWidth;
    uint16_t packedHeight;
};

//!
//! \struct: RegionPacking
//! \brief:  define the region wise packing information
//!          for all regions
//!
struct RegionPacking
{
    bool constituentPictMatching;
    uint32_t projPictureWidth;
    uint32_t projPictureHeight;
    uint16_t packedPictureWidth;
    uint16_t packedPictureHeight;

    std::vector<Region> regions;
};

//!
//! \struct: Spherical
//! \brief:  define the content coverage information
//!          for one region on the sphere
//!
struct Spherical
{
    int32_t cAzimuth;
    int32_t cElevation;
    int32_t cTilt;
    uint32_t rAzimuth;
    uint32_t rElevation;
    //bool interpolate;
};

//!
//! \struct: QualityInfo
//! \brief:  define the quality rank related information
//!          for one region
//!
struct QualityInfo
{
    uint8_t qualityRank;
    uint16_t origWidth = 0;    // used only with multi-res cases
    uint16_t origHeight = 0;   // used only with multi-res cases
    VCD::MP4::DataItem<Spherical> sphere;    // not used with remaining area info
};

//!
//! \struct: Quality3d
//! \brief:  define the quality rank related information
//!          for all regions
//!
struct Quality3d
{
    uint8_t shapeType = 0;
    uint8_t qualityType = 0;
    bool remainingArea = false;
    std::vector<QualityInfo> qualityInfo;
};

//!
//! \struct: CodedMeta
//! \brief:  define the meta data for the coded data
//!
struct CodedMeta
{
    int64_t presIndex;        // presentation index (as in RawFormatMeta)
    int64_t codingIndex; // coding index
    VCD::MP4::FrameTime codingTime;
    VCD::MP4::FrameTime presTime;
    VCD::MP4::FrameDuration duration;

    VCD::MP4::TrackId trackId;

    bool inCodingOrder;

    CodedFormat format = CodedFormat::NoneFormat;

    std::map<ConfigType, std::vector<uint8_t>> decoderConfig;

    uint32_t width = 0;
    uint32_t height = 0;

    uint8_t channelCfg = 0;
    uint32_t samplingFreq = 0;
    Bitrate bitrate = {}; // bitrate information

    FrameType type = FrameType::NA;

    SegmenterMeta segmenterMeta;

    // applicable in OMAF
    OmafProjectionType projection = OmafProjectionType::EQUIRECTANGULAR;
    VCD::MP4::DataItem<RegionPacking> regionPacking;
    VCD::MP4::DataItem<Spherical> sphericalCoverage;
    VCD::MP4::DataItem<Quality3d> qualityRankCoverage;

    bool isEOS = false;

    bool isIDR() const
    {
        return type == FrameType::IDR;
    }
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
    std::map<uint8_t, Extractor*>* extractors;
    Nalu              extractorTrackNalu;
    std::list<VCD::MP4::TrackId> refTrackIdxs;

    Nalu              audioNalu;

    VCD::MP4::TrackId trackIdx;
    InitSegConfig     dashInitCfg;
    GeneralSegConfig  dashCfg;
    DashInitSegmenter *initSegmenter;
    DashSegmenter     *dashSegmenter;

    CodedMeta         codedMeta;

    uint8_t           qualityRanking;

    bool              isEOS;
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

private:
    VCD::MP4::TrackDescriptionsMap                m_trackDescriptions;           //!< track description information

    std::set<VCD::MP4::TrackId>                   m_firstFrameRemaining;         //!< the remaining track index for which initial segment is not generated for the first frame

    const InitSegConfig                           m_config;                      //!< the configuration for the initial segment

    std::string                                   m_omafVideoTrackBrand = "";    //!< video track OMAF brand information
    std::string                                   m_omafAudioTrackBrand = "";    //!< audio track OMAF brand information
    uint64_t                                      m_initSegSize = 0;

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
    void AddH264VideoTrack(VCD::MP4::TrackId aTrackId, CodedMeta& aMeta);

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
    void AddH265VideoTrack(VCD::MP4::TrackId aTrackId, CodedMeta& aMeta);

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
    void AddH265ExtractorTrack(VCD::MP4::TrackId aTrackId, CodedMeta& aMeta);

    void AddAACTrack(VCD::MP4::TrackId aTrackId, CodedMeta& aMeta);
    //!
    //! \brief  Make initial segment for the track
    //!
    //! \param  [in]aFragmented
    //!         indicate whether the initial segment is fragmented
    //!
    //! \return InitialSegment
    //!         the initial segment
    //!
    VCD::MP4::InitialSegment MakeInitSegment(bool aFragmented);

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
    void FillOmafStructures(
        VCD::MP4::TrackId aTrackId,
        CodedMeta& aMeta,
        VCD::MP4::HevcVideoSampleEntry& aSampleEntry,
        VCD::MP4::TrackMeta& aTrackMeta);
};

//!
//! \class AcquireVideoFrameData
//! \brief Define the operation of acquiring video coded data
//!

class AcquireVideoFrameData : public VCD::MP4::GetDataOfFrame
{
public:

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] data
    //!         the pointer to the nalu data
    //! \param  [in] size
    //!         size of the nalu data
    //!
    AcquireVideoFrameData(uint8_t *data, uint64_t size);

    //!
    //! \brief  Destructor
    //!
    ~AcquireVideoFrameData() override;

    //!
    //! \brief  Get the coded data
    //!
    //! \return FrameBuf
    //!         the FrameBuf which includes the coded data
    //!
    VCD::MP4::FrameBuf Get() const override;

    //!
    //! \brief  Get the size of coded data
    //!
    //! \return size_t
    //!         the size of coded data
    //!
    size_t GetDataSize() const override;

    //!
    //! \brief  Clone one AcquireVideoFrameData object
    //!
    //! \return AcquireVideoFrameData*
    //!         the pointer to the cloned AcquireVideoFrameData object
    //!
    AcquireVideoFrameData* Clone() const override;

private:
    //Nalu *m_tileNalu; //!< the pointer to the nalu information of coded data
    uint8_t  *m_data;        //!< the pointer to the nalu data
    uint64_t m_dataSize;     //!< nalu data size
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
        CodedMeta codedMeta,
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
    void Feed(
        VCD::MP4::TrackId trackId,
        CodedMeta codedFrameMeta,
        Nalu *dataNalu,
        VCD::MP4::FrameCts compositionTime);

    std::map<VCD::MP4::TrackId, TrackInfo>   m_trackInfo;                   //!< track information of all tracks

    const GeneralSegConfig         m_config;                      //!< the configuration of data segment of the track

    //VCD::MP4::AutoSegmenter m_autoSegmenter;               //!< the low level segmenter to write segment
    VCD::MP4::SegmentWriter        m_segWriter;

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
    bool DetectNonRefFrame(uint8_t *frameData);

    //!
    //! \brief  Write the segment
    //!
    //! \param  [in] aSegments
    //!         the segments
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason 
    //!
    int32_t WriteSegment(VCD::MP4::SegmentList& aSegments);

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
        std::map<uint8_t, Extractor*>* extractorsMap,
        std::list<VCD::MP4::TrackId> refTrackIdxs,
        Nalu *extractorsNalu);

private:

    uint64_t                                                          m_segNum = 0;            //!< current segments number
    FILE                                                              *m_file = NULL;          //!< file pointer to write segments
    char                                                              m_segName[1024];           //!< segment file name string
    uint64_t                                                          m_segSize = 0;
};

VCD_NS_END;
#endif /* _DASHSEGMENTER_H_ */
