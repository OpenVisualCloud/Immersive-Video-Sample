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
#include "../isolib/dash_parser/Mp4ReaderImpl.h"
#include "../isolib/dash_parser/Mp4StreamIO.h"
#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>

VCD_OMAF_BEGIN

class SegmentStream : public VCD::MP4::StreamIO {
public:
    SegmentStream(){
        mSegment = NULL;
    };
    SegmentStream(OmafSegment* seg){
        mSegment = seg;
        mFileStream.open( seg->GetSegmentCacheFile().c_str(), ios_base::binary | ios_base::in );
    };
    ~SegmentStream(){
        mSegment = NULL;
        mFileStream.close();
    };
public:
    /** Returns the number of bytes read. The value of 0 indicates end
        of file.
        @param [buffer] The buffer to write the data into
        @param [size]   The number of bytes to read from the stream
        @returns The number of bytes read, or 0 on EOF.
    */
    virtual offset_t ReadStream(char* buffer, offset_t size){
        if(NULL == mSegment) return -1;

        mFileStream.read(buffer, size);
        std::streamsize readCnt = mFileStream.gcount();
        return (offset_t)readCnt;
    };

    /** Seeks to the given offset. Should the offset be erronous we'll
        find it out also by the next read that will signal EOF.

        Seeking to the point after the last input byte is permissable;
        so seeking to offset 0 of an empty file should be OK as well
        as seeking to offset 1 of a 1-byte file. The next read would
        indicate EOF, though.

        @param [offset] Offset to seek into
        @returns true if the seek was successful
     */
    virtual bool SeekAbsoluteOffset(offset_t offset){
        if(NULL == mSegment) return false;
        if (mFileStream.tellg() == -1)
        {
            mFileStream.clear();
            mFileStream.seekg(0, ios_base::beg);
        }

        mFileStream.seekg( offset );

        return true;
    };

    /** Retrieve the current offset of the file.
        @returns The current offset of the file.
     */
    virtual offset_t TellOffset(){

        if(NULL == mSegment) return -1;
        offset_t offset1 = mFileStream.tellg();
        return offset1;
    };

    /** Retrieve the size of the current file.

        @returns The current size of the file. Return
        StreamIO::IndeterminateSize if the file size cannot be determined.
     */
    virtual offset_t GetStreamSize(){
        mFileStream.seekg(0, ios_base::end);
        int64_t size = mFileStream.tellg();
        mFileStream.seekg(0, ios_base::beg);
        return size;
    };

private:
    OmafSegment*   mSegment;
    std::ifstream  mFileStream;
};

OmafMP4VRReader::OmafMP4VRReader()
{
    mMP4ReaderImpl = (void*) VCD::MP4::Mp4Reader::Create();
}

OmafMP4VRReader::OmafMP4VRReader(OmafMP4VRReader&& other)
{
    mMP4ReaderImpl = std::move(other.mMP4ReaderImpl);
}

OmafMP4VRReader::~OmafMP4VRReader()
{
    if(mMP4ReaderImpl)
    {
        VCD::MP4::Mp4Reader::Destroy((VCD::MP4::Mp4Reader*)mMP4ReaderImpl);
        mMP4ReaderImpl = NULL;
    }
}

int32_t OmafMP4VRReader::initialize( OmafSegment* pSeg)
{
    if(NULL == mMP4ReaderImpl){
        mMP4ReaderImpl = (void*) VCD::MP4::Mp4Reader::Create();
    }

    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->Initialize( new SegmentStream(pSeg) );
}

void OmafMP4VRReader::close()
{
    if(NULL == mMP4ReaderImpl){
        LOG(ERROR) << "OmafMP4VRReader::close NULL != mMP4ReaderImpl" << endl;
        return ;
    }
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    pReader->Close();
}

int32_t OmafMP4VRReader::getMajorBrand(FourCC& majorBrand,
                                       uint32_t initializationSegmentId,
                                       uint32_t segmentId ) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;
    VCD::MP4::FourCC brand;
    int ret = pReader->GetMajorBrand(brand, initializationSegmentId, segmentId);

    if( 0!=ret ) return ERROR_INVALID;

    majorBrand = brand;

    return ERROR_NONE;
}

int32_t OmafMP4VRReader::getMinorVersion(uint32_t& minorVersion,
                                       uint32_t initializationSegmentId,
                                       uint32_t segmentId ) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->GetMinorVersion(minorVersion, initializationSegmentId, segmentId);
}

int32_t OmafMP4VRReader::getCompatibleBrands(std::vector<VCD::OMAF::FourCC*>& compatibleBrands,
                                             uint32_t initializationSegmentId,
                                             uint32_t segmentId ) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::VarLenArray<VCD::MP4::FourCC> brands;

    int32_t ret = pReader->GetCompatibleBrands(brands, initializationSegmentId, segmentId);

    for(uint32_t i=0; i<brands.size; i++){
        VCD::OMAF::FourCC* cc = new VCD::OMAF::FourCC(brands[i].item);
        compatibleBrands.push_back(cc);
    }

    return ret;
}

void OmafMP4VRReader::SelectedTrackInfos(std::vector<VCD::OMAF::TrackInformation*>& trackInfos, std::vector<VCD::OMAF::TrackInformation*> middleTrackInfos) const
{
    std::map<int,int> mapInitTrack = getMapInitTrk();
    if (mapInitTrack.size() != 0)
    {
        vector<pair<int,int>> needIDs;
        for (auto mapId = mapInitTrack.begin(); mapId != mapInitTrack.end(); mapId++)
        {
            uint32_t currentInitSegId = mapId->first;
            uint32_t currentTrackId   = mapId->second;
            for (auto itTrack = middleTrackInfos.begin(); itTrack != middleTrackInfos.end(); itTrack++)
            {
                TrackInformation *track = *itTrack;
                if (currentInitSegId == track->initSegmentId && currentTrackId == (track->trackId & 0xffff))
                {
                    trackInfos.push_back(track);
                    needIDs.push_back(make_pair(currentInitSegId, currentTrackId));
                    break;
                }
            }
        }
        for (auto itTrack = middleTrackInfos.begin(); itTrack != middleTrackInfos.end(); itTrack++)
        {
            TrackInformation *track = *itTrack;
            pair<int,int> tmpID = make_pair(track->initSegmentId, track->trackId & 0xffff);
            if (find(needIDs.begin(), needIDs.end(), tmpID) == needIDs.end())
            {
                SAFE_DELETE(track);
            }
        }
    }
    else
    {
        for (auto itTrack = middleTrackInfos.begin(); itTrack != middleTrackInfos.end(); itTrack++)
        {
            TrackInformation *track = *itTrack;
            if(!track) continue;
            auto itRefTrack = track->referenceTrackIds[0];
            TypeToTrackIDs *refTrackIds = &(itRefTrack);

            if (refTrackIds->trackIds.size != 0)
            {
                trackInfos.push_back(track);
                continue;
            }
            else
            {
                uint32_t initSegIndex = track->initSegmentId;
                uint32_t combinedTrackId = track->trackId;
                auto itTrack2 = middleTrackInfos.begin();
                for ( ; itTrack2 != middleTrackInfos.end(); itTrack2++)
                {
                    if(!(*itTrack2)) continue;
                    if (((*itTrack2)->initSegmentId == initSegIndex) &&
                        ((*itTrack2)->trackId != combinedTrackId))
                    {
                        break;
                    }
                }

                if (itTrack2 != middleTrackInfos.end())
                {
                    SAFE_DELETE(track);
                    continue;
                }
                else
                {
                    trackInfos.push_back(track);
                }
            }
        }
    }
}

int32_t OmafMP4VRReader::getTrackInformations(std::vector<VCD::OMAF::TrackInformation*>& trackInfos) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    if(trackInfos.size() != 0) trackInfos.clear();

    std::vector<VCD::OMAF::TrackInformation*> middleTrackInfos;

    VCD::MP4::VarLenArray<VCD::MP4::TrackInformation> *Infos = new VCD::MP4::VarLenArray<VCD::MP4::TrackInformation>;

    pReader->GetTrackInformations(*Infos);

    for( uint32_t i=0; i<(*Infos).size; i++){
        TrackInformation *trackInfo = new TrackInformation;
        *trackInfo = (*Infos)[i];
        middleTrackInfos.push_back(trackInfo);
    }
    SelectedTrackInfos(trackInfos, middleTrackInfos);
    middleTrackInfos.clear();

    delete Infos;
    return ERROR_NONE;
}

int32_t OmafMP4VRReader::getDisplayWidth(uint32_t trackId, uint32_t& displayWidth) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->GetDisplayWidth(trackId, displayWidth);
}

int32_t OmafMP4VRReader::getDisplayHeight(uint32_t trackId, uint32_t& displayHeight) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->GetDisplayHeight(trackId, displayHeight);
}

int32_t OmafMP4VRReader::getDisplayWidthFP(uint32_t trackId, uint32_t& displayWidth) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->GetDisplayWidthFP(trackId, displayWidth);
}

int32_t OmafMP4VRReader::getDisplayHeightFP(uint32_t trackId, uint32_t& displayHeight) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->GetDisplayHeightFP(trackId, displayHeight);
}

int32_t OmafMP4VRReader::getWidth(uint32_t trackId, uint32_t sampleId, uint32_t& width) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->GetWidth(trackId, sampleId, width);
}

int32_t OmafMP4VRReader::getHeight(uint32_t trackId, uint32_t sampleId, uint32_t& height) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->GetHeight(trackId, sampleId, height);
}

int32_t OmafMP4VRReader::getPlaybackDurationInSecs(uint32_t trackId, double& durationInSecs) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->GetPlaybackDurationInSecs(trackId, durationInSecs);
}

int32_t OmafMP4VRReader::getTrackSampleListByType(uint32_t trackId,
                                                  VCD::OMAF::TrackSampleType sampleType,
                                                  std::vector<uint32_t>& sampleIds) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::SampleFrameType type;

    type = sampleType;

    VCD::MP4::VarLenArray<uint32_t> ids;

    int32_t ret = pReader->GetSampListByType(trackId, type, ids);

    for(uint32_t idx=0; idx<ids.size; idx++){
        sampleIds.push_back(ids[idx]);
    }

    return ret;

}

int32_t OmafMP4VRReader::getTrackSampleType(uint32_t trackId, uint32_t sampleId, VCD::OMAF::FourCC& trackSampleBoxType) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::FourCC cc;

    if (NULL == pReader)
    {
        return ERROR_NULL_PTR;
    }

    int32_t ret = pReader->GetSampType(trackId, sampleId, cc);

    trackSampleBoxType = cc;

    return ret;
}

int32_t OmafMP4VRReader::getExtractorTrackSampleData(uint32_t trackId,
                                   uint32_t sampleId,
                                   char* memoryBuffer,
                                   uint32_t& memoryBufferSize,
                                   bool videoByteStreamHeaders)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;
    int ret = pReader->GetExtractorTrackSampData( trackId, sampleId, memoryBuffer, memoryBufferSize, videoByteStreamHeaders);

    return ret;
}

int32_t OmafMP4VRReader::getTrackSampleData(uint32_t trackId,
                                   uint32_t sampleId,
                                   char* memoryBuffer,
                                   uint32_t& memoryBufferSize,
                                   bool videoByteStreamHeaders)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;
    int ret = pReader->GetSampData( trackId, sampleId, memoryBuffer, memoryBufferSize, videoByteStreamHeaders);

    return ret;
}

int32_t OmafMP4VRReader::getTrackSampleOffset(uint32_t trackId, uint32_t sampleId, uint64_t& sampleOffset, uint32_t& sampleLength)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->GetSampOffset(trackId, sampleId, sampleOffset, sampleLength);
}

int32_t OmafMP4VRReader::getDecoderConfiguration(uint32_t trackId, uint32_t sampleId, std::vector<VCD::OMAF::DecoderSpecificInfo>& decoderInfos) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::VarLenArray<VCD::MP4::MediaCodecSpecInfo> *Infos = new VCD::MP4::VarLenArray<VCD::MP4::MediaCodecSpecInfo>;

    int32_t ret = pReader->GetCodecSpecInfo( trackId, sampleId, *Infos );

    for(uint32_t i=0; i<(*Infos).size; i++){
        DecoderSpecificInfo info;
        info = (*Infos)[i];

        decoderInfos.push_back(info);
    }

    delete Infos;
    return ret;
}

int32_t OmafMP4VRReader::getTrackTimestamps(uint32_t trackId, std::vector<VCD::OMAF::TimestampIDPair>& timestamps) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::VarLenArray<VCD::MP4::TStampID> id_pairs;

    int32_t ret = pReader->GetTrackTStamps( trackId, id_pairs );

    for(uint32_t i=0; i<id_pairs.size; i++){
        TimestampIDPair IdPair;

        IdPair.itemId    = id_pairs[i].itemId;
        IdPair.timeStamp = id_pairs[i].timeStamp;

        timestamps.push_back(IdPair);
    }

    return ret;
}

int32_t OmafMP4VRReader::getTimestampsOfSample(uint32_t trackId, uint32_t sampleId, std::vector<uint64_t>& timestamps) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::VarLenArray<uint64_t> tms;

    int32_t ret = pReader->GetSampTStamps( trackId, sampleId, tms );

    for(uint32_t i=0; i<tms.size; i++){
        timestamps.push_back(tms[i]);
    }

    return ret;
}

int32_t OmafMP4VRReader::getSamplesInDecodingOrder(uint32_t trackId, std::vector<VCD::OMAF::TimestampIDPair>& sampleDecodingOrder) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::VarLenArray<VCD::MP4::TStampID> id_pairs;

    int32_t ret = pReader->GetSampInDecSeq( trackId, id_pairs );

    for(uint32_t i=0; i<id_pairs.size; i++){
        TimestampIDPair IdPair;

        IdPair.itemId    = id_pairs[i].itemId;
        IdPair.timeStamp = id_pairs[i].timeStamp;

        sampleDecodingOrder.push_back(IdPair);
    }

    return ret;
}

int32_t OmafMP4VRReader::getDecoderCodeType(uint32_t trackId, uint32_t sampleId, VCD::OMAF::FourCC& decoderCodeType) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::FourCC cc;

    int32_t ret = pReader->GetDecoderCodeType(trackId, sampleId, cc);

    decoderCodeType = cc;

    return ret;
}

int32_t OmafMP4VRReader::getSampleDuration(uint32_t trackId, uint32_t sampleId, uint32_t& sampleDuration) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->GetDurOfSamp(trackId, sampleId, sampleDuration);
}

int32_t OmafMP4VRReader::getPropertyChnl(uint32_t trackId, uint32_t sampleId, VCD::OMAF::chnlProperty& chProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::ChnlProperty chProp;

    int32_t ret = pReader->GetAudioChnlProp(trackId, sampleId, chProp);

    chProperty = chProp;

    return ret;
}

int32_t OmafMP4VRReader::getPropertySpatialAudio(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SpatialAudioProperty& spatialaudioproperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::SpatialAudioProperty spProp;

    int32_t ret = pReader->GetSpatAudioProp(trackId, sampleId, spProp);

    spatialaudioproperty = spProp;

    return ret;
}

int32_t OmafMP4VRReader::getPropertyStereoScopic3D(uint32_t trackId, uint32_t sampleId, VCD::OMAF::StereoScopic3DProperty& stereoscopicproperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::OmniStereoScopic3D ssProp;

    int32_t ret = pReader->GetSteScop3DProp(trackId, sampleId, ssProp);
    stereoscopicproperty = ssProp;

    return ret;
}

int32_t OmafMP4VRReader::getPropertySphericalVideoV1(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SphericalVideoV1Property& sphericalproperty) const
{
    return 0;
}

int32_t OmafMP4VRReader::getPropertySphericalVideoV2(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SphericalVideoV2Property& sphericalproperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::SphericalVideoV2Property sv2Prop;

    int32_t ret = pReader->GetSpheV2Prop(trackId, sampleId, sv2Prop);

    sphericalproperty = sv2Prop;

    return ret;
}

int32_t OmafMP4VRReader::getPropertyRegionWisePacking(uint32_t trackId, uint32_t sampleId, RegionWisePacking *rwpk) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::RWPKProperty rwpkProp;

    int32_t ret = pReader->GetRWPKProp(trackId, sampleId, rwpkProp);

    rwpk->constituentPicMatching = rwpkProp.constituentPictureMatching;
    rwpk->packedPicHeight        = rwpkProp.packedPicHeight;
    rwpk->packedPicWidth         = rwpkProp.packedPicWidth;
    rwpk->projPicHeight          = rwpkProp.projPicHeight;
    rwpk->projPicWidth           = rwpkProp.projPicWidth;

    rwpk->numRegions = rwpkProp.regions.size;

    rwpk->rectRegionPacking = new RectangularRegionWisePacking[rwpk->numRegions];

    for(uint32_t i = 0; i < rwpkProp.regions.size; i++){

        rwpk->rectRegionPacking[i].guardBandFlag         = rwpkProp.regions[i].guardBandFlag;
        rwpk->rectRegionPacking[i].transformType         = (uint8_t)RegionWisePackingType::OMNI_RECTANGULAR;
        rwpk->rectRegionPacking[i].bottomGbHeight        = rwpkProp.regions[i].region.rectReg.bottomGbHeight;
        rwpk->rectRegionPacking[i].gbNotUsedForPredFlag  = rwpkProp.regions[i].region.rectReg.gbNotUsedForPredFlag;
        rwpk->rectRegionPacking[i].gbType0               = rwpkProp.regions[i].region.rectReg.gbType0;
        rwpk->rectRegionPacking[i].gbType1               = rwpkProp.regions[i].region.rectReg.gbType1;
        rwpk->rectRegionPacking[i].gbType2               = rwpkProp.regions[i].region.rectReg.gbType2;
        rwpk->rectRegionPacking[i].gbType3               = rwpkProp.regions[i].region.rectReg.gbType3;
        rwpk->rectRegionPacking[i].leftGbWidth           = rwpkProp.regions[i].region.rectReg.leftGbWidth;
        rwpk->rectRegionPacking[i].packedRegHeight       = rwpkProp.regions[i].region.rectReg.packedRegHeight;
        rwpk->rectRegionPacking[i].packedRegLeft         = rwpkProp.regions[i].region.rectReg.packedRegLeft;
        rwpk->rectRegionPacking[i].packedRegTop          = rwpkProp.regions[i].region.rectReg.packedRegTop;
        rwpk->rectRegionPacking[i].packedRegWidth        = rwpkProp.regions[i].region.rectReg.packedRegWidth;
        rwpk->rectRegionPacking[i].projRegHeight         = rwpkProp.regions[i].region.rectReg.projRegHeight;
        rwpk->rectRegionPacking[i].projRegLeft           = rwpkProp.regions[i].region.rectReg.projRegLeft;
        rwpk->rectRegionPacking[i].projRegTop            = rwpkProp.regions[i].region.rectReg.projRegTop;
        rwpk->rectRegionPacking[i].projRegWidth          = rwpkProp.regions[i].region.rectReg.projRegWidth;
        rwpk->rectRegionPacking[i].rightGbWidth          = rwpkProp.regions[i].region.rectReg.rightGbWidth;
        rwpk->rectRegionPacking[i].topGbHeight           = rwpkProp.regions[i].region.rectReg.topGbHeight;
    }

    return ret;
}
int32_t OmafMP4VRReader::getPropertyCoverageInformation(uint32_t trackId, uint32_t sampleId, VCD::OMAF::CoverageInformationProperty& coviProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::COVIInformation ccProp;

    int32_t ret = pReader->GetCOVIInfoProp(trackId, sampleId, ccProp);

    coviProperty = ccProp;

    return ret;
}

int32_t OmafMP4VRReader::getPropertyProjectionFormat(uint32_t trackId, uint32_t sampleId, VCD::OMAF::ProjectionFormatProperty& projectionFormatProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::ProjFormat pfProp;

    int32_t ret = pReader->GetProjFrmtProp(trackId, sampleId, pfProp);

    projectionFormatProperty.format = pfProp.format;

    return ret;
}

int32_t OmafMP4VRReader::getPropertySchemeTypes(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SchemeTypesProperty& schemeTypesProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::SchemeTypesProperty stProp;

    int32_t ret = pReader->GetScheTypesProp(trackId, sampleId, stProp);

    schemeTypesProperty = stProp;

    return ret;
}

int32_t OmafMP4VRReader::getPropertyStereoVideoConfiguration(uint32_t trackId, uint32_t sampleId, VCD::OMAF::PodvStereoVideoConfiguration& stereoVideoProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::VideoFramePackingType psConf;

    int32_t ret = pReader->GetStereVideoProp(trackId, sampleId, psConf);

    stereoVideoProperty = psConf;

    return ret;
}

int32_t OmafMP4VRReader::getPropertyRotation(uint32_t trackId, uint32_t sampleId, VCD::OMAF::Rotation& rotationProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    VCD::MP4::Rotation rot;

    int32_t ret = pReader->GetRotateProp(trackId, sampleId, rot);

    rotationProperty = rot;

    return ret;
}


int32_t OmafMP4VRReader::parseInitializationSegment(OmafSegment* streamInterface, uint32_t initSegmentId)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    SegmentStream *segment = new SegmentStream(streamInterface);
    if (NULL == segment) return ERROR_NULL_PTR;

    return pReader->ParseInitSeg(segment, initSegmentId);
}

int32_t OmafMP4VRReader::invalidateInitializationSegment(uint32_t initSegmentId)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    return pReader->DisableInitSeg(initSegmentId);
}

int32_t OmafMP4VRReader::parseSegment( OmafSegment* streamInterface,
                                       uint32_t initSegmentId,
                                       uint32_t segmentId,
                                       uint64_t earliestPTSinTS )
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;

    SegmentStream *segment = new SegmentStream(streamInterface);
    if (NULL == segment) return ERROR_NULL_PTR;

    return pReader->ParseSeg(segment, initSegmentId, segmentId, earliestPTSinTS);
}

int32_t OmafMP4VRReader::invalidateSegment(uint32_t initSegmentId, uint32_t segmentId)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    VCD::MP4::Mp4Reader* pReader = (VCD::MP4::Mp4Reader*)mMP4ReaderImpl;
    return pReader->DisableSeg(initSegmentId, segmentId);
}

VCD_OMAF_END
