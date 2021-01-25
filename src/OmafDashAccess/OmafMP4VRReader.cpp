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
 * File:   OmafMP4VRReader.cpp
 * Author: media
 *
 * Created on May 28, 2019, 1:41 PM
 */

#include "OmafMP4VRReader.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include "../isolib/dash_parser/Mp4ReaderImpl.h"
#include "../isolib/dash_parser/Mp4StreamIO.h"

VCD_OMAF_BEGIN

class SegmentStream : public VCD::MP4::StreamIO {
 public:
  SegmentStream(OmafSegment* seg) { mSegment = seg; };
  ~SegmentStream() { mSegment = nullptr; };

 public:
  //!
  //! \brief  Read the stream according to the offset
  //!
  //! \param  [in] buffer
  //!         The buffer to be written the data into
  //! \param  [in] size
  //!         The number of bytes to be read from the stream
  //!
  //! \return offset_t
  //!         the actual number of bytes read from the stream
  virtual offset_t ReadStream(char* buffer, offset_t size) {
    if (nullptr == mSegment) return -1;

    return mSegment->ReadStream(buffer, size);
  };

  //!
  //! \brief  Seek to the given offset. Seeking to offset 0 of an empty
  //!         file is OK and seeking to offset 1 of a 1-byte file is also
  //!         OK.
  //!
  //! \param  [in] offset
  //!         offset to seek into
  //!
  //! \return bool
  //!         true if the seeking is successful
  virtual bool SeekAbsoluteOffset(offset_t offset) {
    if (nullptr == mSegment) return false;

    return mSegment->SeekAbsoluteOffset(offset);
  };

  //!
  //! \brief  Get the current offset position of the read file
  //!
  //! \return offset_t
  //!         the actual current offset of the read file
  virtual offset_t TellOffset() {
    if (nullptr == mSegment) return -1;

    return mSegment->TellOffset();
  };

  //!
  //! \brief Get the size of current read file
  //!
  //! \return offset_t
  //!         the size of the file, or StreamIO::IndeterminateSize if the
  //!         size can't be determined
  virtual offset_t GetStreamSize() {
    if (nullptr == mSegment) return -1;

    return mSegment->GetStreamSize();
  };

 private:
  OmafSegment* mSegment = nullptr;
};

OmafMP4VRReader::OmafMP4VRReader() { mMP4ReaderImpl = (void*)VCD::MP4::Mp4Reader::Create(); }

OmafMP4VRReader::OmafMP4VRReader(OmafMP4VRReader&& other) { mMP4ReaderImpl = std::move(other.mMP4ReaderImpl); }

OmafMP4VRReader::~OmafMP4VRReader() {
  if (mMP4ReaderImpl) {
    VCD::MP4::Mp4Reader::Destroy((VCD::MP4::Mp4Reader*)mMP4ReaderImpl);
    mMP4ReaderImpl = nullptr;
  }
}

int32_t OmafMP4VRReader::initialize(OmafSegment* pSeg) {
  if (nullptr == mMP4ReaderImpl) {
    mMP4ReaderImpl = (void*)VCD::MP4::Mp4Reader::Create();
  }

  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->Initialize(new SegmentStream(pSeg));
}

void OmafMP4VRReader::close() {
  if (nullptr == mMP4ReaderImpl) {
    OMAF_LOG(LOG_ERROR, "OmafMP4VRReader::close nullptr != mMP4ReaderImpl\n");
    return;
  }
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  pReader->Close();
}

int32_t OmafMP4VRReader::getMajorBrand(FourCC& majorBrand, uint32_t initializationSegmentId, uint32_t segmentId) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;
  VCD::MP4::FourCC brand;
  int ret = pReader->GetMajorBrand(brand, initializationSegmentId, segmentId);

  if (0 != ret) return ERROR_INVALID;

  majorBrand = brand;

  return ERROR_NONE;
}

int32_t OmafMP4VRReader::getMinorVersion(uint32_t& minorVersion, uint32_t initializationSegmentId,
                                         uint32_t segmentId) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetMinorVersion(minorVersion, initializationSegmentId, segmentId);
}

int32_t OmafMP4VRReader::getCompatibleBrands(std::vector<VCD::OMAF::FourCC*>& compatibleBrands,
                                             uint32_t initializationSegmentId, uint32_t segmentId) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::VarLenArray<VCD::MP4::FourCC> brands;

  int32_t ret = pReader->GetCompatibleBrands(brands, initializationSegmentId, segmentId);

  for (uint32_t i = 0; i < brands.size; i++) {
    VCD::OMAF::FourCC* cc = new VCD::OMAF::FourCC(brands[i].item);
    compatibleBrands.push_back(cc);
  }

  return ret;
}

void OmafMP4VRReader::SelectedTrackInfos(std::vector<VCD::OMAF::TrackInformation*>& trackInfos,
                                         std::vector<VCD::OMAF::TrackInformation*> middleTrackInfos) const {
  std::map<int, int> mapInitTrack = getMapInitTrk();
  if (mapInitTrack.size() != 0) {
    vector<pair<int, int>> needIDs;
    for (auto mapId = mapInitTrack.begin(); mapId != mapInitTrack.end(); mapId++) {
      uint32_t currentInitSegId = mapId->first;
      uint32_t currentTrackId = mapId->second;
      for (auto itTrack = middleTrackInfos.begin(); itTrack != middleTrackInfos.end(); itTrack++) {
        TrackInformation* track = *itTrack;
        if (currentInitSegId == track->initSegmentId && currentTrackId == (track->trackId & 0xffff)) {
          trackInfos.push_back(track);
          needIDs.push_back(make_pair(currentInitSegId, currentTrackId));
          break;
        }
      }
    }
    for (auto itTrack = middleTrackInfos.begin(); itTrack != middleTrackInfos.end(); itTrack++) {
      TrackInformation* track = *itTrack;
      pair<int, int> tmpID = make_pair(track->initSegmentId, track->trackId & 0xffff);
      if (find(needIDs.begin(), needIDs.end(), tmpID) == needIDs.end()) {
        SAFE_DELETE(track);
      }
    }
  } else {
    std::vector<TrackInformation*> clearTrackInfoArr;
    for (auto itTrack = middleTrackInfos.begin(); itTrack != middleTrackInfos.end(); itTrack++) {
      TrackInformation* track = *itTrack;
      if (!track) continue;
      auto itRefTrack = track->referenceTrackIds[0];
      TypeToTrackIDs* refTrackIds = &(itRefTrack);

      if (refTrackIds->trackIds.size != 0) {
        trackInfos.push_back(track);
        continue;
      } else {
        uint32_t initSegIndex = track->initSegmentId;
        uint32_t combinedTrackId = track->trackId;
        auto itTrack2 = middleTrackInfos.begin();
        for (; itTrack2 != middleTrackInfos.end(); itTrack2++) {
          if (!(*itTrack2)) continue;
          if (((*itTrack2)->initSegmentId == initSegIndex) && ((*itTrack2)->trackId != combinedTrackId)) {
            break;
          }
        }

        if (itTrack2 != middleTrackInfos.end()) {
          clearTrackInfoArr.push_back(track);
          continue;
        } else {
          trackInfos.push_back(track);
        }
      }
    }
    for (uint32_t i = 0; i < clearTrackInfoArr.size(); i++)
    {
      SAFE_DELETE(clearTrackInfoArr[i]);
    }
    clearTrackInfoArr.clear();
  }
}

int32_t OmafMP4VRReader::getTrackInformations(std::vector<VCD::OMAF::TrackInformation*>& trackInfos) const {
    double dResult;
    clock_t lBefore = clock();
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
#if 1
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  if (trackInfos.size() != 0) trackInfos.clear();

  VCD::MP4::VarLenArray<VCD::MP4::TrackInformation>* Infos = new VCD::MP4::VarLenArray<VCD::MP4::TrackInformation>;
  pReader->GetTrackInformation(*Infos);

  for (uint32_t i = 0; i < (*Infos).size; i++) {
    TrackInformation* trackInfo = new TrackInformation;
    *trackInfo = (*Infos)[i];

    trackInfos.push_back(trackInfo);
  }

  if (Infos)
  {
      delete Infos;
      Infos = NULL;
  }

  dResult = (double)(clock() - lBefore) * 1000 / CLOCKS_PER_SEC;
  OMAF_LOG(LOG_INFO, "Total Time for OmafMP4VRReader GetTrackInformation is %f ms\n", dResult);
#else
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  if (trackInfos.size() != 0) trackInfos.clear();

  std::vector<VCD::OMAF::TrackInformation*> middleTrackInfos;

  VCD::MP4::VarLenArray<VCD::MP4::TrackInformation>* Infos = new VCD::MP4::VarLenArray<VCD::MP4::TrackInformation>;

  pReader->GetTrackInformations(*Infos);

  for (uint32_t i = 0; i < (*Infos).size; i++) {
    TrackInformation* trackInfo = new TrackInformation;
    *trackInfo = (*Infos)[i];

    middleTrackInfos.push_back(trackInfo);
  }
  SelectedTrackInfos(trackInfos, middleTrackInfos);
  middleTrackInfos.clear();

  delete Infos;
#endif

  return ERROR_NONE;
}

int32_t OmafMP4VRReader::getDisplayWidth(uint32_t trackId, uint32_t& displayWidth) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetDisplayWidth(trackId, displayWidth);
}

int32_t OmafMP4VRReader::getDisplayHeight(uint32_t trackId, uint32_t& displayHeight) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetDisplayHeight(trackId, displayHeight);
}

int32_t OmafMP4VRReader::getDisplayWidthFP(uint32_t trackId, uint32_t& displayWidth) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetDisplayWidthFP(trackId, displayWidth);
}

int32_t OmafMP4VRReader::getDisplayHeightFP(uint32_t trackId, uint32_t& displayHeight) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetDisplayHeightFP(trackId, displayHeight);
}

int32_t OmafMP4VRReader::getWidth(uint32_t trackId, uint32_t sampleId, uint32_t& width) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetWidth(trackId, sampleId, width);
}

int32_t OmafMP4VRReader::getHeight(uint32_t trackId, uint32_t sampleId, uint32_t& height) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetHeight(trackId, sampleId, height);
}

int32_t OmafMP4VRReader::getDims(uint32_t trackId, uint32_t sampleId, uint32_t& width, uint32_t& height) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetDims(trackId, sampleId, width, height);
}

int32_t OmafMP4VRReader::getPlaybackDurationInSecs(uint32_t trackId, double& durationInSecs) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetPlaybackDurationInSecs(trackId, durationInSecs);
}

int32_t OmafMP4VRReader::getTrackSampleListByType(uint32_t trackId, VCD::OMAF::TrackSampleType sampleType,
                                                  std::vector<uint32_t>& sampleIds) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::SampleFrameType type;

  type = sampleType;

  VCD::MP4::VarLenArray<uint32_t> ids;

  int32_t ret = pReader->GetSampListByType(trackId, type, ids);

  for (uint32_t idx = 0; idx < ids.size; idx++) {
    sampleIds.push_back(ids[idx]);
  }

  return ret;
}

int32_t OmafMP4VRReader::getTrackSampleType(uint32_t trackId, uint32_t sampleId,
                                            VCD::OMAF::FourCC& trackSampleBoxType) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::FourCC cc;

  if (nullptr == pReader) {
    return ERROR_NULL_PTR;
  }

  int32_t ret = pReader->GetSampType(trackId, sampleId, cc);

  trackSampleBoxType = cc;

  return ret;
}

int32_t OmafMP4VRReader::getExtractorTrackSampleData(uint32_t trackId, uint32_t sampleId, char* memoryBuffer,
                                                     uint32_t& memoryBufferSize, bool videoByteStreamHeaders) {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;
  int ret =
      pReader->GetExtractorTrackSampData(trackId, sampleId, memoryBuffer, memoryBufferSize, videoByteStreamHeaders);

  return ret;
}

int32_t OmafMP4VRReader::getTrackSampleData(uint32_t trackId, uint32_t sampleId, char* memoryBuffer,
                                            uint32_t& memoryBufferSize, bool videoByteStreamHeaders) {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;
  int ret = pReader->GetSampData(trackId, sampleId, memoryBuffer, memoryBufferSize, videoByteStreamHeaders);

  return ret;
}

int32_t OmafMP4VRReader::getTrackSampleOffset(uint32_t trackId, uint32_t sampleId, uint64_t& sampleOffset,
                                              uint32_t& sampleLength) {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetSampOffset(trackId, sampleId, sampleOffset, sampleLength);
}

int32_t OmafMP4VRReader::getDecoderConfiguration(uint32_t trackId, uint32_t sampleId,
                                                 std::vector<VCD::OMAF::DecoderSpecificInfo>& decoderInfos) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::VarLenArray<VCD::MP4::MediaCodecSpecInfo>* Infos = new VCD::MP4::VarLenArray<VCD::MP4::MediaCodecSpecInfo>;

  int32_t ret = pReader->GetCodecSpecInfo(trackId, sampleId, *Infos);

  for (uint32_t i = 0; i < (*Infos).size; i++) {
    DecoderSpecificInfo info;
    info = (*Infos)[i];

    decoderInfos.push_back(info);
  }

  delete Infos;
  return ret;
}

int32_t OmafMP4VRReader::getTrackTimestamps(uint32_t trackId,
                                            std::vector<VCD::OMAF::TimestampIDPair>& timestamps) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::VarLenArray<VCD::MP4::TStampID> id_pairs;

  int32_t ret = pReader->GetTrackTStamps(trackId, id_pairs);

  for (uint32_t i = 0; i < id_pairs.size; i++) {
    TimestampIDPair IdPair;

    IdPair.itemId = id_pairs[i].itemId;
    IdPair.timeStamp = id_pairs[i].timeStamp;

    timestamps.push_back(IdPair);
  }

  return ret;
}

int32_t OmafMP4VRReader::getTimestampsOfSample(uint32_t trackId, uint32_t sampleId,
                                               std::vector<uint64_t>& timestamps) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::VarLenArray<uint64_t> tms;

  int32_t ret = pReader->GetSampTStamps(trackId, sampleId, tms);

  for (uint32_t i = 0; i < tms.size; i++) {
    timestamps.push_back(tms[i]);
  }

  return ret;
}

int32_t OmafMP4VRReader::getSamplesInDecodingOrder(uint32_t trackId,
                                                   std::vector<VCD::OMAF::TimestampIDPair>& sampleDecodingOrder) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::VarLenArray<VCD::MP4::TStampID> id_pairs;

  int32_t ret = pReader->GetSampInDecSeq(trackId, id_pairs);

  for (uint32_t i = 0; i < id_pairs.size; i++) {
    TimestampIDPair IdPair;

    IdPair.itemId = id_pairs[i].itemId;
    IdPair.timeStamp = id_pairs[i].timeStamp;

    sampleDecodingOrder.push_back(IdPair);
  }

  return ret;
}

int32_t OmafMP4VRReader::getDecoderCodeType(uint32_t trackId, uint32_t sampleId,
                                            VCD::OMAF::FourCC& decoderCodeType) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::FourCC cc;

  int32_t ret = pReader->GetDecoderCodeType(trackId, sampleId, cc);

  decoderCodeType = cc;

  return ret;
}

int32_t OmafMP4VRReader::getSampleDuration(uint32_t trackId, uint32_t sampleId, uint32_t& sampleDuration) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->GetDurOfSamp(trackId, sampleId, sampleDuration);
}

int32_t OmafMP4VRReader::getPropertyChnl(uint32_t trackId, uint32_t sampleId,
                                         VCD::OMAF::chnlProperty& chProperty) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::ChnlProperty chProp;

  int32_t ret = pReader->GetAudioChnlProp(trackId, sampleId, chProp);

  chProperty = chProp;

  return ret;
}

int32_t OmafMP4VRReader::getPropertySpatialAudio(uint32_t trackId, uint32_t sampleId,
                                                 VCD::OMAF::SpatialAudioProperty& spatialaudioproperty) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::SpatialAudioProperty spProp;

  int32_t ret = pReader->GetSpatAudioProp(trackId, sampleId, spProp);

  spatialaudioproperty = spProp;

  return ret;
}

int32_t OmafMP4VRReader::getPropertyStereoScopic3D(uint32_t trackId, uint32_t sampleId,
                                                   VCD::OMAF::StereoScopic3DProperty& stereoscopicproperty) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::OmniStereoScopic3D ssProp;

  int32_t ret = pReader->GetSteScop3DProp(trackId, sampleId, ssProp);
  stereoscopicproperty = ssProp;

  return ret;
}

int32_t OmafMP4VRReader::getPropertySphericalVideoV1(uint32_t trackId, uint32_t sampleId,
                                                     VCD::OMAF::SphericalVideoV1Property& sphericalproperty) const {
  return 0;
}

int32_t OmafMP4VRReader::getPropertySphericalVideoV2(uint32_t trackId, uint32_t sampleId,
                                                     VCD::OMAF::SphericalVideoV2Property& sphericalproperty) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::SphericalVideoV2Property sv2Prop;

  int32_t ret = pReader->GetSpheV2Prop(trackId, sampleId, sv2Prop);

  sphericalproperty = sv2Prop;

  return ret;
}

int32_t OmafMP4VRReader::getPropertyRegionWisePacking(uint32_t trackId, uint32_t sampleId,
                                                      RegionWisePacking* rwpk) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::RWPKProperty rwpkProp;

  int32_t ret = pReader->GetRWPKProp(trackId, sampleId, rwpkProp);

  rwpk->constituentPicMatching = rwpkProp.constituentPictureMatching;
  rwpk->packedPicHeight = rwpkProp.packedPicHeight;
  rwpk->packedPicWidth = rwpkProp.packedPicWidth;
  rwpk->projPicHeight = rwpkProp.projPicHeight;
  rwpk->projPicWidth = rwpkProp.projPicWidth;

  rwpk->numRegions = rwpkProp.regions.size;

  rwpk->rectRegionPacking = new RectangularRegionWisePacking[rwpk->numRegions];

  for (uint32_t i = 0; i < rwpkProp.regions.size; i++) {
    rwpk->rectRegionPacking[i].guardBandFlag = rwpkProp.regions[i].guardBandFlag;
    rwpk->rectRegionPacking[i].transformType = rwpkProp.regions[i].region.rectReg.transformType;
    rwpk->rectRegionPacking[i].bottomGbHeight = rwpkProp.regions[i].region.rectReg.bottomGbHeight;
    rwpk->rectRegionPacking[i].gbNotUsedForPredFlag = rwpkProp.regions[i].region.rectReg.gbNotUsedForPredFlag;
    rwpk->rectRegionPacking[i].gbType0 = rwpkProp.regions[i].region.rectReg.gbType0;
    rwpk->rectRegionPacking[i].gbType1 = rwpkProp.regions[i].region.rectReg.gbType1;
    rwpk->rectRegionPacking[i].gbType2 = rwpkProp.regions[i].region.rectReg.gbType2;
    rwpk->rectRegionPacking[i].gbType3 = rwpkProp.regions[i].region.rectReg.gbType3;
    rwpk->rectRegionPacking[i].leftGbWidth = rwpkProp.regions[i].region.rectReg.leftGbWidth;
    rwpk->rectRegionPacking[i].packedRegHeight = rwpkProp.regions[i].region.rectReg.packedRegHeight;
    rwpk->rectRegionPacking[i].packedRegLeft = rwpkProp.regions[i].region.rectReg.packedRegLeft;
    rwpk->rectRegionPacking[i].packedRegTop = rwpkProp.regions[i].region.rectReg.packedRegTop;
    rwpk->rectRegionPacking[i].packedRegWidth = rwpkProp.regions[i].region.rectReg.packedRegWidth;
    rwpk->rectRegionPacking[i].projRegHeight = rwpkProp.regions[i].region.rectReg.projRegHeight;
    rwpk->rectRegionPacking[i].projRegLeft = rwpkProp.regions[i].region.rectReg.projRegLeft;
    rwpk->rectRegionPacking[i].projRegTop = rwpkProp.regions[i].region.rectReg.projRegTop;
    rwpk->rectRegionPacking[i].projRegWidth = rwpkProp.regions[i].region.rectReg.projRegWidth;
    rwpk->rectRegionPacking[i].rightGbWidth = rwpkProp.regions[i].region.rectReg.rightGbWidth;
    rwpk->rectRegionPacking[i].topGbHeight = rwpkProp.regions[i].region.rectReg.topGbHeight;
  }

  return ret;
}
int32_t OmafMP4VRReader::getPropertyCoverageInformation(uint32_t trackId, uint32_t sampleId,
                                                        VCD::OMAF::CoverageInformationProperty& coviProperty) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::COVIInformation ccProp;

  int32_t ret = pReader->GetCOVIInfoProp(trackId, sampleId, ccProp);

  coviProperty = ccProp;

  return ret;
}

int32_t OmafMP4VRReader::getPropertyProjectionFormat(
    uint32_t trackId, uint32_t sampleId, VCD::OMAF::ProjectionFormatProperty& projectionFormatProperty) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::ProjFormat pfProp;

  int32_t ret = pReader->GetProjFrmtProp(trackId, sampleId, pfProp);

  projectionFormatProperty.format = pfProp.format;

  return ret;
}

int32_t OmafMP4VRReader::getPropertySchemeTypes(uint32_t trackId, uint32_t sampleId,
                                                VCD::OMAF::SchemeTypesProperty& schemeTypesProperty) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::SchemeTypesProperty stProp;

  int32_t ret = pReader->GetScheTypesProp(trackId, sampleId, stProp);

  schemeTypesProperty = stProp;

  return ret;
}

int32_t OmafMP4VRReader::getPropertyStereoVideoConfiguration(
    uint32_t trackId, uint32_t sampleId, VCD::OMAF::PodvStereoVideoConfiguration& stereoVideoProperty) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::VideoFramePackingType psConf;

  int32_t ret = pReader->GetStereVideoProp(trackId, sampleId, psConf);

  stereoVideoProperty = psConf;

  return ret;
}

int32_t OmafMP4VRReader::getPropertyRotation(uint32_t trackId, uint32_t sampleId,
                                             VCD::OMAF::Rotation& rotationProperty) const {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  VCD::MP4::Rotation rot;

  int32_t ret = pReader->GetRotateProp(trackId, sampleId, rot);

  rotationProperty = rot;

  return ret;
}

int32_t OmafMP4VRReader::parseInitializationSegment(OmafSegment* streamInterface, uint32_t initSegmentId) {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  SegmentStream* segment = new SegmentStream(streamInterface);
  if (nullptr == segment) return ERROR_NULL_PTR;

  return pReader->ParseInitSeg(segment, initSegmentId);
}

int32_t OmafMP4VRReader::invalidateInitializationSegment(uint32_t initSegmentId) {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  return pReader->DisableInitSeg(initSegmentId);
}

int32_t OmafMP4VRReader::parseSegment(OmafSegment* streamInterface, uint32_t initSegmentId, uint32_t segmentId,
                                      uint64_t earliestPTSinTS) {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

  SegmentStream* segment = new SegmentStream(streamInterface);
  if (nullptr == segment) return ERROR_NULL_PTR;

  return pReader->ParseSeg(segment, initSegmentId, segmentId, earliestPTSinTS);
}

int32_t OmafMP4VRReader::invalidateSegment(uint32_t initSegmentId, uint32_t segmentId) {
  if (nullptr == mMP4ReaderImpl) return ERROR_NULL_PTR;
  VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;
  return pReader->DisableSeg(initSegmentId, segmentId);
}

VCD_OMAF_END
