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
//! \file:   Mp4ReaderImpl.h
//! \brief:  Mp4Reader class definition
//! \detail: Define detailed file reader operation class
//!

#ifndef _MP4READERIMPL_H_
#define _MP4READERIMPL_H_

#include "Mp4DataTypes.h"
#include "../atoms/ChannelLayoutAtom.h"
#include "../atoms/FormAllocator.h"
#include "../atoms/DecPts.h"
#include "../atoms/ElemStreamDescAtom.h"
#include "../atoms/TypeAtom.h"
#include "../atoms/BasicAudAtom.h"
#include "../atoms/BasicVideoAtom.h"
#include "../atoms/MetaAtom.h"
#include "../atoms/MovieAtom.h"
#include "../atoms/MovieFragAtom.h"
#include "Mp4Segment.h"
#include "Mp4StreamIO.h"
#include "../atoms/SegIndexAtom.h"

#include <fstream>
#include <functional>
#include <istream>
#include <memory>

using namespace std;

VCD_MP4_BEGIN

class CleanAperture;
class AvcDecoderConfigurationRecord;
class HevcDecoderConfigurationRecord;

//!
//! \class Mp4Reader
//! \brief Define the operation and needed data for mp4 segment files reading
//!

class Mp4Reader
{
public:
    //!
    //! \brief  Constructor
    //!
    Mp4Reader();

    //!
    //! \brief  Destructor
    //!
    virtual ~Mp4Reader() = default;

    //!
    //! \brief  Create one instance of Mp4Reader class
    //!
    //! \return Mp4Reader*
    //!         the pointer to the created instance
    //!
    static Mp4Reader* Create();

    //!
    //! \brief  Destroy the specified instance of Mp4Reader class
    //!
    //! \param  [in] mp4Reader
    //!         pointer to the specified instance
    //!
    //! \return void
    //!
    static void Destroy(Mp4Reader* mp4Reader);

public:

    //!
    //! \brief  Initialize Mp4Reader according to specified
    //!         stream operation interface
    //!
    //! \param  [in] stream
    //!         pointer to the specified stream operation
    //!         interface
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize(StreamIO* stream);

    //!
    //! \brief  Free resource of Mp4Reader
    //!
    //! \return void
    //!
    void Close();

    //!
    //! \brief  Get the major brand from specified segment
    //!         of specified track
    //!
    //! \param  [out] majorBrand
    //!         output major brand
    //! \param  [in] initSegIndex
    //!         initial segment index corresponding to
    //!         specified track
    //! \param  [in] segIndex
    //!         index of specified segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetMajorBrand(FourCC& majorBrand,
                          uint32_t initSegIndex = 0,
                          uint32_t segIndex               = UINT32_MAX) const;

    //!
    //! \brief  Get the minor version from specified segment
    //!         of specified track
    //!
    //! \param  [out] minorVersion
    //!         output minor version
    //! \param  [in] initSegIndex
    //!         initial segment index corresponding to
    //!         specified track
    //! \param  [in] segIndex
    //!         index of specified segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetMinorVersion(uint32_t& minorVersion,
                            uint32_t initSegIndex = 0,
                            uint32_t segIndex               = UINT32_MAX) const;

    //!
    //! \brief  Get the compatible brands from specified segment
    //!         of specified track
    //!
    //! \param  [out] compatBrands
    //!         output compatible brands
    //! \param  [in] initSegIndex
    //!         initial segment index corresponding to
    //!         specified track
    //! \param  [in] segIndex
    //!         index of specified segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetCompatibleBrands(VarLenArray<FourCC>& compatBrands,
                                uint32_t initSegIndex = 0,
                                uint32_t segIndex               = UINT32_MAX) const;

    //!
    //! \brief  Get the track information for all tracks
    //!         after one segment file is parsed and Mp4Reader
    //!         is initialized
    //!
    //! \param  [out] trackInfos
    //!         track information for all tracks
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetTrackInformations(VarLenArray<TrackInformation>& trackInfos) const;

    //!
    //! \brief  Get the track information for basic tracks
    //!         after one segment file is parsed and Mp4Reader
    //!         is initialized
    //!
    //! \param  [out] trackInfos
    //!         track information for basic tracks
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetTrackInformation(VarLenArray<TrackInformation>& trackInfos) const;

    //!
    //! \brief  Get picture display width
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] displayPicW
    //!         picture display width
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetDisplayWidth(uint32_t trackId, uint32_t& displayPicW) const;

    //!
    //! \brief  Get picture display height
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] displayPicH
    //!         picture display height
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetDisplayHeight(uint32_t trackId, uint32_t& displayPicH) const;

    //!
    //! \brief  Get fixed point picture display width
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] displayPicW
    //!         fixed point picture display width
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetDisplayWidthFP(uint32_t trackId, uint32_t& displayPicW) const;

    //!
    //! \brief  Get fixed point picture display height
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] displayPicH
    //!         fixed point picture display height
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetDisplayHeightFP(uint32_t trackId, uint32_t& displayPicH) const;

    //!
    //! \brief  Get width of specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  itemId
    //!         index of specified sample
    //! \param  [out] width
    //!         width of specified sample in pixels
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetWidth(uint32_t trackId, uint32_t itemId, uint32_t& width) const;

    //!
    //! \brief  Get height of specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  itemId
    //!         index of specified sample
    //! \param  [out] height
    //!         height of specified sample in pixels
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetHeight(uint32_t trackId, uint32_t itemId, uint32_t& height) const;


    int32_t GetDims(uint32_t trackId, uint32_t itemId,uint32_t& width, uint32_t& height) const;
    //!
    //! \brief  Get playback duration of specified track,
    //!         and unit is second
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] druInSecs
    //!         playback duration in second
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetPlaybackDurationInSecs(uint32_t trackId, double& durInSecs) const;

    //!
    //! \brief  Get samples list of specified sample frame type from
    //!         the specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  itemType
    //!         specified sample frame type, like reference frame type
    //! \param  [out] itemIds
    //!         samples list
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSampListByType(uint32_t trackId, SampleFrameType itemType, VarLenArray<uint32_t>& itemIds) const;

    //!
    //! \brief  Get sample type of specified sample in
    //!         the specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  itemId
    //!         index of specified sample
    //! \param  [out] trackItemType
    //!         detailed sample type
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSampType(uint32_t trackId, uint32_t itemId, FourCC& trackItemType) const;

    //!
    //! \brief  Get complete data for specified sample in
    //!         the specified extractor track
    //!
    //! \param  [in]  trackId
    //!         index of specific extractor track
    //! \param  [in]  itemIndex
    //!         index of specified sample
    //! \param  [out] buf
    //!         pointer to the allocated memory to store
    //!         the sample data
    //! \param  [out] bufSize
    //!         size of sample data
    //! \param  [in]  strHrd
    //!         whether to insert NAL unit start codes into
    //!         sample data
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetExtractorTrackSampData(uint32_t trackId,
                                        uint32_t itemIndex,
                                        char* buf,
                                        uint32_t& bufSize,
                                        bool strHrd = true);

    //!
    //! \brief  Get complete data for specified sample in
    //!         the specified normal track
    //!
    //! \param  [in]  trackId
    //!         index of specific normal track
    //! \param  [in]  itemId
    //!         index of specified sample
    //! \param  [out] buf
    //!         pointer to the allocated memory to store
    //!         the sample data
    //! \param  [out] bufSize
    //!         size of sample data
    //! \param  [in]  strHrd
    //!         whether to insert NAL unit start codes into
    //!         sample data
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSampData(uint32_t trackId,
                               uint32_t itemId,
                               char* buf,
                               uint32_t& bufSize,
                               bool strHrd = true);

    //!
    //! \brief  Get track sample data offset and length for
    //!         the specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  itemIndex
    //!         index of specified sample
    //! \param  [out] sampOffset
    //!         output sample offset
    //! \param  [out] sampLen
    //!         output sample length
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSampOffset(uint32_t trackId,
                                 uint32_t itemIndex,
                                 uint64_t& sampOffset,
                                 uint32_t& sampLen);

    //!
    //! \brief  Get media codec related specific information,
    //!         like SPS, PPS and so on, for specified sample in
    //!         specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  itemId
    //!         index of specified sample
    //! \param  [out] codecSpecInfos
    //!         output media codec related specific information
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetCodecSpecInfo(uint32_t trackId,
                                    uint32_t itemId,
                                    VarLenArray<MediaCodecSpecInfo>& codecSpecInfos) const;

    //!
    //! \brief  Get display time stamp of each sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] timeStamps
    //!         output time stamps for each sample
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetTrackTStamps(uint32_t trackId,
                               VarLenArray<TStampID>& timeStamps) const;

    //!
    //! \brief  Get display time stamp of the specified sample
    //!         in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in] itemId
    //!         index of the specified sample
    //! \param  [out] timeStamps
    //!         output time stamps for the sample
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSampTStamps(uint32_t trackId,
                                  uint32_t itemId,
                                  VarLenArray<uint64_t>& timeStamps) const;

    //!
    //! \brief  Get samples in decoding sequence in specified track,
    //!         gotten samples are presented by TStampID structure,
    //!         that is <timestamp, sampleId> pair
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] sampItems
    //!         output samples in decoding sequence
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSampInDecSeq(uint32_t trackId,
                                      VarLenArray<TStampID>& sampItems) const;

    //!
    //! \brief  Get decoder code type for specified sample
    //!         in specified track, like "hvc1" and so on.
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  itemId
    //!         index of specified sample
    //! \param  [out] decoderCodeType
    //!         output decoder code type
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetDecoderCodeType(uint32_t trackId,
                               uint32_t itemId,
                               FourCC& decoderCodeType) const;

    //!
    //! \brief  Get duration for specified sample
    //!         in specified track, in milliseconds.
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] sampDur
    //!         output sample duration
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetDurOfSamp(uint32_t trackId,
                              uint32_t sampleId,
                              uint32_t& sampDur) const;

    //!
    //! \brief  Get audio channel layout box information for
    //!         specified sample in specified 'chnl' track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output channel property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetAudioChnlProp(uint32_t trackId,
                            uint32_t sampleId,
                            ChnlProperty& outProp) const;

    //!
    //! \brief  Get spatial audio box information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output spatial audio property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSpatAudioProp(uint32_t trackId,
                                    uint32_t sampleId,
                                    SpatialAudioProperty& outProp) const;

    //!
    //! \brief  Get stereo scopic 3D information for spherical video for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output stereo scopic 3D property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSteScop3DProp(uint32_t trackId,
                                      uint32_t sampleId,
                                      OmniStereoScopic3D& outProp) const;

    //!
    //! \brief  Get spherical video V1 information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output spherical video V1 property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSpheV1Prop(uint32_t trackId,
                                        uint32_t sampleId,
                                        SphericalVideoV1Property& outProp) const;

    //!
    //! \brief  Get spherical video V2 information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output spherical video V2 property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSpheV2Prop(uint32_t trackId,
                                        uint32_t sampleId,
                                        SphericalVideoV2Property& outProp) const;

    //!
    //! \brief  Get region wise packing information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output region wise packing property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetRWPKProp(uint32_t trackId,
                                         uint32_t sampleId,
                                         RWPKProperty& outProp) const;
    //!
    //! \brief  Get content coverage information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output content coverage property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetCOVIInfoProp(uint32_t trackId,
                                           uint32_t sampleId,
                                           COVIInformation& outProp) const;
    //!
    //! \brief  Get projection format information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output projection format property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetProjFrmtProp(uint32_t trackId,
                                        uint32_t sampleId,
                                        ProjFormat& outProp) const;
    //!
    //! \brief  Get scheme type information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output scheme type property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetScheTypesProp(uint32_t trackId,
                                   uint32_t sampleId,
                                   SchemeTypesProperty& outProp) const;
    //!
    //! \brief  Get stereo video information for
    //!         specified sample in specified track,
    //!         only podv scheme is supported
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output stereo video property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetStereVideoProp(uint32_t trackId,
                                                uint32_t sampleId,
                                                VideoFramePackingType& outProp) const;
    //!
    //! \brief  Get rotation information for
    //!         specified sample in specified track,
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] outProp
    //!         output podv rotation property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetRotateProp(uint32_t trackId,
                                uint32_t sampleId,
                                Rotation& outProp) const;

private:
    uint32_t SearchInfoForTrack(uint32_t trackId,
                             uint32_t sampleId,
                             TrackBasicInfo& basicTrackInfo,
                             SmpDesIndex& index) const;

    template <typename Tkey, typename Tval>
    Tval GetSampProp(std::map<Tkey, Tval>& propsMap,
                         SmpDesIndex& index,
                         uint32_t& result) const;

public:

    //!
    //! \brief  Parse specified initial segment
    //!
    //! \param  [in]  strIO
    //!         pointer to specified initial segment handler
    //! \param  [in]  initSegId
    //!         index of specified initial segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ParseInitSeg(StreamIO* strIO, uint32_t initSegId);

    //!
    //! \brief  Disable specified initial segment
    //!         Invalidate the data buffer pointer to the
    //!         specified initial segment, then the data
    //!         from the segment can not be accessed any longer
    //!
    //! \param  [in]  initSegId
    //!         index of specified initial segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t DisableInitSeg(uint32_t initSegId);

    //!
    //! \brief  Parse specified segment for specified track
    //!
    //! \param  [in]  strIO
    //!         pointer to specified segment handler
    //! \param  [in]  initSegId
    //!         index of specified initial segment, this index
    //!         is corresponding to track index
    //! \param  [in]  segIndex
    //!         index of specified segment
    //! \param  [in]  earliestPTSinTS
    //!         the earliest presentation time in timescale for
    //!         the specified sample
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ParseSeg(StreamIO* strIO,
                         uint32_t initSegId,
                         uint32_t segIndex,
                         uint64_t earliestPTSinTS = UINT64_MAX);

    //!
    //! \brief  Disable specified segment for specified track
    //!         Disable the data buffer pointer to the specified
    //!         segment, then the data from the segment can not
    //!         be accessed any longer
    //!
    //! \param  [in]  initSegId
    //!         index of specified initial segment, which is
    //!         corresponding to track index
    //! \param  [in]  segIndex
    //!         index of specified segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t DisableSeg(uint32_t initSegId,
                              uint32_t segIndex);

    //!
    //! \brief  Get segment index information for specified
    //!         track
    //!
    //! \param  [in]  initSegId
    //!         index of specified initial segment, which is
    //!         corresponding to track index
    //! \param  [out] segIndex
    //!         array of segmentation information structure
    //!         which hold segment index information
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetSegIndex(uint32_t initSegId,
                            VarLenArray<SegInfo>& segIndex);

    //!
    //! \brief  Parse segment index information for specified
    //!         stream
    //!
    //! \param  [in]  strIO
    //!         pointer to the specified stream handler
    //! \param  [out] segIndex
    //!         array of segmentation information structure
    //!         which hold parsed segment index information
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ParseSegIndex(StreamIO* strIO,
                              VarLenArray<SegInfo>& segIndex);

private:
    std::map<InitSegmentId, InitSegmentProperties> m_initSegProps;
    UniquePtr<StreamIO> m_fileStr;

    Sequence m_nextSeq = {};

    enum class ReaderState
    {
        UNINITIALIZED,
        INITIALIZING,
        READY
    };
    ReaderState m_readerSte;

    enum class CtxType
    {
        META,
        TRACK,
        FILE,
        NOT_SET
    };

    struct CtxInfo
    {
        CtxType ctxType                 = CtxType::NOT_SET;
        bool isCoverImg                  = false;
        ItemId coverImgId                = 0;
        bool enableLoopPlay              = false;
    };
    std::map<InitSegmentTrackId, CtxInfo> m_ctxInfoMap;

    friend class DashSegGroup;
    friend class ConstDashSegGroup;

    DashSegGroup CreateDashSegs(InitSegmentId initSegId);
    ConstDashSegGroup CreateDashSegs(InitSegmentId initSegId) const;

    void IsInited() const;

    int IsInitErr() const;

    CtxType GetCtxType(InitSegmentTrackId id) const;

    int GetCtxTypeError(const InitSegmentTrackId id, CtxType& ctxType) const;

    int32_t ReadStream(InitSegmentId initSegId, SegmentId segIndex);

    FileProperty GetFileProps() const;

    int32_t ReadAtomParams(SegmentIO& io, std::string& boxType, int64_t& boxSize);
    int32_t ReadAtom(SegmentIO& io, Stream& bitstream);
    int32_t SkipAtom(SegmentIO& io);

    int GetImgDims(uint32_t trackId, uint32_t itemId, uint32_t& width, uint32_t& height) const;

    int32_t GetCtxItems(InitSegmentTrackId segTrackId, IdVector& items) const;

    bool IsProtected(uint32_t trackId, uint32_t itemId) const;

    void GetAvcSpecData(const DataVector& rawData, DataVector& specData);

    void GetHevcSpecData(const DataVector& rawData, DataVector& specData);

    int32_t ParseAvcData(char* buf, uint32_t& bufSize);

    int32_t ParseHevcData(char* buf, uint32_t& bufSize);

    std::map<SegmentTrackId, MetaAtom> m_metaMap;

    struct ImgInfo
    {
        std::string type = "invalid";
        uint32_t width  = 0;
        uint32_t height = 0;
        double displayTime   = 0;
    };
    typedef std::map<ItemId, ImgInfo> ImgInfoMap;

    struct ItemInfo
    {
        std::string type;
    };
    typedef std::map<ItemId, ItemInfo> ItemInfoMap;

    struct MetaAtomInfo
    {
        uint32_t dispMasterImgs = 0;
        bool enableLoopPlay     = false;
        bool forceFPSSet        = false;
        float assignedFPS       = 0.0;
        ImgInfoMap imageInfoMap;
        ItemInfoMap itemInfoMap;
    };
    std::map<SegmentTrackId, MetaAtomInfo> m_metaInfo;

    TrackBasicInfo& GetTrackBasicInfo(InitSegmentTrackId initSegTrackId);
    const TrackBasicInfo& GetTrackBasicInfo(InitSegmentTrackId initSegTrackId) const;

    TrackDecInfo& GetTrackDecInfo(InitSegmentId initSegId,
                            SegmentTrackId segTrackId);
    const TrackDecInfo& GetTrackDecInfo(InitSegmentId initSegId,
                                  SegmentTrackId segTrackId) const;
    bool CanFindTrackDecInfo(InitSegmentId initSegId,
                      SegmentTrackId segTrackId) const;

    const SampleInfoVector& GetSampInfos(InitSegmentId initSegId,
                                          SegmentTrackId segTrackId,
                                          ItemId& itemIdBase) const;

    const ParameterSetMap* GetParameterSetMap(InitSegTrackIdPair id) const;  //< returns null if not found

    bool FoundPrevSeg(InitSegmentId initSegId,
                             SegmentId curSegmentId,
                             SegmentId& precedingSegmentId) const;

    const TrackDecInfo* GetPrevTrackDecInfo(InitSegmentId initSegId,
                                           SegmentTrackId segTrackId) const;

    void CfgSegSidxFallback(InitSegmentId initSegId,
                                  SegmentTrackId segTrackId);

    void RefreshCompTimes(InitSegmentId initSegId,
                                SegmentId segIndex);

    ItemInfoMap ExtractItemInfoMap(const MetaAtom& metaAtom) const;

    void ProcessDecoderConfigProperties(const InitSegmentTrackId segTrackId);

    std::vector<int32_t> m_matrix;

    void RefreshDecCodeType(InitSegmentId initSegIndex,
                                  SegmentTrackId segTrackId,
                                  const SampleInfoVector& sampleInfo,
                                  size_t prevSampInfoSize = 0);

    void RefreshItemToParamSet(ItemToParameterSetMap& itemToParameterSetMap,
                                      InitSegmentTrackId initSegTrackId,
                                      const SampleInfoVector& sampleInfo,
                                      size_t prevSampInfoSize = 0);

    void AddSegSeq(InitSegmentId initSegIndex,
                            SegmentId segIndex,
                            Sequence sequence);

    TrackPropertiesMap FillTrackProps(InitSegmentId initSegIndex,
                                           SegmentId segIndex,
                                           MovieAtom& moovAtom);

    ItemId GetSuccedentItmId(InitSegmentId initSegIndex,
                              SegmentTrackId segTrackId) const;

    ItemId GetPrevItemId(InitSegmentId initSegIndex,
                              SegmentTrackId segTrackId) const;

    typedef std::map<ContextId, DecodePts::PresentTimeTS> CtxIdPresentTSMap;

    void AddTrackProps(InitSegmentId initSegId,
                              SegmentId segIndex,
                              MovieFragmentAtom& moofAtom,
                              const CtxIdPresentTSMap& earliestPTSTS);

    void AddSampsToTrackDecInfo(TrackDecInfo& trackInfo,
                               const InitSegmentProperties& initSegProps,
                               const TrackBasicInfo& basicTrackInfo,
                               const TrackProperties& trackProps,
                               const uint64_t baseDataOffset,
                               const uint32_t sampDescId,
                               ItemId itemIdBase,
                               ItemId trackrunItemIdBase,
                               const TrackRunAtom* trackRunAtom);

    MoovProperties ExtractMoovProps(const MovieAtom& moovAtom) const;

    void FillSampEntryMap(TrackAtom* trackAtom,
                            InitSegmentId initSegId);

    void FillRinfAtomInfo(TrackBasicInfo& trackInfo,
                                       unsigned int index,
                                       const SampleEntryAtom& entry);

    TrackProperty GetTrackProps(TrackAtom* trackAtom) const;

    TypeToCtxIdsMap GetRefTrackIds(TrackAtom* trackAtom) const;

    TypeToIdsMap GetTrackGroupIds(TrackAtom* trackAtom) const;

    TypeToIdsMap GetSampGroupIds(TrackAtom* trackAtom) const;

    IdVector GetAlternateTrackIds(TrackAtom* trackAtom,
                                  MovieAtom& moovAtom) const;

    pair<TrackBasicInfo, TrackDecInfo> ExtractTrackDecInfo(TrackAtom* trackAtom,
                                                         uint32_t movieTimescale) const;

    SampleInfoVector GenSampInfo(TrackAtom* trackAtom) const;

    int AddDecReferences(InitSegmentId initSegId,
                                SegmentTrackId segmentTrackId,
                                const DecodingOrderVector& sampItems,
                                DecodingOrderVector& output) const;

    void GenSegInAtom(const SegmentIndexAtom& sidxAtom,
                          SegmentIndex& segIndex,
                          int64_t dataOffsetAnchor);

    int32_t ConvertStrBytesToInt(SegmentIO& io,
                      uint32_t count,
                      int64_t& result);

    void LocateToOffset(SegmentIO& io, int64_t pos) const
    {
        if (io.strIO->TellOffset() != pos)
        {
            io.strIO->SeekOffset(pos);
        }
    }

    unsigned GenTrackId(InitSegmentTrackId id) const;

    InitSegmentTrackId MakeIdPair(unsigned id) const;

    int32_t GetSegIndex(InitSegmentTrackId initSegTrackId,
                        ItemId itemId, SegmentId& segIndex) const;
    int32_t GetSegIndex(InitSegTrackIdPair id, SegmentId& segIndex) const;


    int32_t GetSampDataInfo(uint32_t trackId,
                              uint32_t itemIndex,
                              const InitSegmentId& initSegId,
                              uint64_t& refSampLength,
                              uint64_t& refDataOffset);

    int32_t GetDepedentSampInfo(uint32_t trackId,
                                 uint32_t itemIndex,
                                 const InitSegmentId& initSegId,
                                 uint8_t trackReference,
                                 uint64_t& refSampLength,
                                 uint64_t& refDataOffset);

    uint64_t ParseNalLen(char* buffer) const;
    void WriteNalLen(uint64_t length, char* buffer) const;
};

template <typename Tkey, typename Tval>
inline Tval Mp4Reader::GetSampProp(std::map<Tkey, Tval>& propsMap,
                                                 SmpDesIndex& index,
                                                 uint32_t& result) const
{
    if (result != ERROR_NONE)
    {
        return Tval();
    }

    if (propsMap.count(index.GetIndex()) == 0)
    {
        result = OMAF_INVALID_SAMPLEDESCRIPTION_INDEX;
        return Tval();
    }

    return propsMap.at(index);
}

VCD_MP4_END;
#endif /* _MP4READERIMPL_H_ */
