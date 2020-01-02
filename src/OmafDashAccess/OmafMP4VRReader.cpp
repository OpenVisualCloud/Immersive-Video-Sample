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
#include "mp4lib/api/reader/mp4vrfilereaderinterface.h"
#include "mp4lib/api/reader/mp4vrfilestreaminterface.h"
#include <iostream>
#include <fstream>      // std::ifstream
#include <set>
#include <algorithm>

void ConvertFourCC(VCD::OMAF::FourCC& cc, MP4VR::FourCC mpcc)
{
    VCD::OMAF::FourCC* tmp = new VCD::OMAF::FourCC(mpcc.value);
    cc = *tmp;
    delete tmp;
}

void AssignSampleType(VCD::OMAF::SampleType vSamT, MP4VR::SampleType mSamT)
{
    std::map<MP4VR::SampleType, VCD::OMAF::SampleType> valueMap;

    valueMap[MP4VR::OUTPUT_NON_REFERENCE_FRAME] = VCD::OMAF::OUTPUT_NON_REF_FRAME;
    valueMap[MP4VR::OUTPUT_REFERENCE_FRAME]     = VCD::OMAF::OUTPUT_REF_FRAME;
    valueMap[MP4VR::NON_OUTPUT_REFERENCE_FRAME] = VCD::OMAF::NON_OUTPUT_REF_FRAME;

    vSamT = valueMap[mSamT];
}

void AssignTrackSampleType(VCD::OMAF::TrackSampleType vTSamT, MP4VR::TrackSampleType mTSamT)
{
    std::map<MP4VR::TrackSampleType, VCD::OMAF::TrackSampleType> valueMap;
    valueMap[MP4VR::out_ref] = VCD::OMAF::out_ref;
    valueMap[MP4VR::out_non_ref] = VCD::OMAF::out_non_ref;
    valueMap[MP4VR::non_out_ref] = VCD::OMAF::non_out_ref;
    valueMap[MP4VR::display] = VCD::OMAF::display;
    valueMap[MP4VR::samples] = VCD::OMAF::samples;

    vTSamT = valueMap[mTSamT];
}

VCD_OMAF_BEGIN

class SegmentStream : public MP4VR::StreamInterface {
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
    virtual offset_t read(char* buffer, offset_t size){
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
    virtual bool absoluteSeek(offset_t offset){
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
    virtual offset_t tell(){

        if(NULL == mSegment) return -1;
        offset_t offset1 = mFileStream.tellg();
        return offset1;
    };

    /** Retrieve the size of the current file.

        @returns The current size of the file. Return
        StreamInterface::IndeterminateSize if the file size cannot be determined.
     */
    virtual offset_t size(){
        //return MP4VR::StreamInterface::IndeterminateSize;
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
    mMP4ReaderImpl = (void*) MP4VR::MP4VRFileReaderInterface::Create();
}

OmafMP4VRReader::~OmafMP4VRReader()
{
    if(mMP4ReaderImpl)
        MP4VR::MP4VRFileReaderInterface::Destroy((MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl);
}

int32_t OmafMP4VRReader::initialize( OmafSegment* pSeg)
{
    if(NULL == mMP4ReaderImpl){
        mMP4ReaderImpl = (void*) MP4VR::MP4VRFileReaderInterface::Create();
    }

    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->initialize( new SegmentStream(pSeg) );
}

void OmafMP4VRReader::close()
{
    if(NULL == mMP4ReaderImpl){
        LOG(ERROR) << "OmafMP4VRReader::close NULL != mMP4ReaderImpl" << endl;
        return ;
    }
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    pReader->close();
}

int32_t OmafMP4VRReader::getMajorBrand(FourCC& majorBrand,
                                       uint32_t initializationSegmentId,
                                       uint32_t segmentId ) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;
    MP4VR::FourCC brand;
    int ret = pReader->getMajorBrand(brand, initializationSegmentId, segmentId);

    if( 0!=ret ) return ERROR_INVALID;

    ConvertFourCC(majorBrand, brand);

    return ERROR_NONE;
}

int32_t OmafMP4VRReader::getMinorVersion(uint32_t& minorVersion,
                                       uint32_t initializationSegmentId,
                                       uint32_t segmentId ) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->getMinorVersion(minorVersion, initializationSegmentId, segmentId);
}

int32_t OmafMP4VRReader::getCompatibleBrands(std::vector<VCD::OMAF::FourCC*>& compatibleBrands,
                                             uint32_t initializationSegmentId,
                                             uint32_t segmentId ) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::DynArray<MP4VR::FourCC> brands;

    int32_t ret = pReader->getCompatibleBrands(brands, initializationSegmentId, segmentId);

    for(uint32_t i=0; i<brands.size; i++){
        VCD::OMAF::FourCC* cc = new VCD::OMAF::FourCC(brands[i].value);
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
                if (currentInitSegId == track->initSegId && currentTrackId == (track->trackId & 0xffff))
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
            pair<int,int> tmpID = make_pair(track->initSegId, track->trackId & 0xffff);
            if (find(needIDs.begin(), needIDs.end(), tmpID) == needIDs.end())
            {
                for(auto &it : track->samplePropertyArrays)
                {
                    SAFE_DELETE(it);
                }
                track->samplePropertyArrays.clear();
                SAFE_DELETE(track);
            }
        }
    }
    else
    {
        for (auto itTrack = middleTrackInfos.begin(); itTrack != middleTrackInfos.end(); itTrack++)
        {
            TrackInformation *track = *itTrack;
            auto itRefTrack = track->referenceTrackIdArrays.begin();
            TypeToTrackIDs *refTrackIds = &(*itRefTrack);

            if (refTrackIds->trackIds.size() != 0)
            {
                trackInfos.push_back(track);
                //LOG(INFO)<<"track "<<(track->trackId& 0xffff)<<" property size = "<<track->sampleProperties.size()<<endl;
                continue;
            }
            else
            {
                uint32_t initSegIndex = track->initSegId;
                uint32_t combinedTrackId = track->trackId;
                auto itTrack2 = middleTrackInfos.begin();
                for ( ; itTrack2 != middleTrackInfos.end(); itTrack2++)
                {
                    if (((*itTrack2)->initSegId == initSegIndex) &&
                        ((*itTrack2)->trackId != combinedTrackId))
                    {
                        break;
                    }
                }

                if (itTrack2 != middleTrackInfos.end())
                {
                    for(auto &it : track->samplePropertyArrays)
                    {
                        SAFE_DELETE(it);
                    }
                    track->samplePropertyArrays.clear();
                    SAFE_DELETE(track);
                    continue;
                }
                else
                {
                    trackInfos.push_back(track);
                    //LOG(INFO)<<"track "<<(track->trackId& 0xffff)<<" property size = "<<track->sampleProperties.size()<<endl;
                }
            }
        }
    }
}

int32_t OmafMP4VRReader::getTrackInformations(std::vector<VCD::OMAF::TrackInformation*>& trackInfos) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    if(trackInfos.size() != 0) trackInfos.clear();

    std::vector<VCD::OMAF::TrackInformation*> middleTrackInfos;

    MP4VR::DynArray<MP4VR::TrackInformation> *Infos = new MP4VR::DynArray<MP4VR::TrackInformation>;

    pReader->getTrackInformations(*Infos);

    uint32_t idx = 0;

    for( uint32_t i=0; i<(*Infos).size; i++){
        TrackInformation *trackInfo = new TrackInformation;
        trackInfo->trackId            = (*Infos)[i].trackId;
        trackInfo->initSegId          = (*Infos)[i].initSegmentId;

        trackInfo->alternateGroupId   = (*Infos)[i].alternateGroupId;
        trackInfo->featureBM          = (*Infos)[i].features;
        trackInfo->vrFeatureBM        = (*Infos)[i].vrFeatures;
        trackInfo->maxSampleSize      = (*Infos)[i].maxSampleSize;
        trackInfo->timeScale          = (*Infos)[i].timeScale;
        trackInfo->hasTypeInformation = (*Infos)[i].hasTypeInformation;
        trackInfo->frameRate.den      = (*Infos)[i].frameRate.den;
        trackInfo->frameRate.num      = (*Infos)[i].frameRate.num;

        for(idx=0; idx<(*Infos)[i].trackURI.size; idx++){
            trackInfo->trackURI.push_back((*Infos)[i].trackURI[idx]);
        }

        for(idx=0; idx<(*Infos)[i].alternateTrackIds.size; idx++){
            trackInfo->alternateTrackIdArrays.push_back((*Infos)[i].alternateTrackIds[idx]);
        }

        for(idx=0; idx<(*Infos)[i].referenceTrackIds.size; idx++){
            TypeToTrackIDs referenceTrackId;
            for(uint32_t j=0; j<(*Infos)[i].referenceTrackIds[idx].trackIds.size; j++){
                referenceTrackId.trackIds.push_back((*Infos)[i].referenceTrackIds[idx].trackIds[j]);
            }
            ConvertFourCC(referenceTrackId.type, (*Infos)[i].referenceTrackIds[idx].type);
            trackInfo->referenceTrackIdArrays.push_back(referenceTrackId);
        }

        for(idx=0; idx<(*Infos)[i].trackGroupIds.size; idx++){
            TypeToTrackIDs trackGroupIds;
            for(uint32_t j=0; j<(*Infos)[i].trackGroupIds[idx].trackIds.size; j++){
                trackGroupIds.trackIds.push_back((*Infos)[i].trackGroupIds[idx].trackIds[j]);
            }
            ConvertFourCC(trackGroupIds.type, (*Infos)[i].trackGroupIds[idx].type);
            trackInfo->trackGroupIdArrays.push_back(trackGroupIds);
        }

        for(idx=0; idx<(*Infos)[i].sampleProperties.size; idx++){
            SampleInformation *info = new SampleInformation;
            info->earliestTimestamp                             = (*Infos)[i].sampleProperties[idx].earliestTimestamp;
            info->earliestTimestampTS                           = (*Infos)[i].sampleProperties[idx].earliestTimestampTS;
            info->descriptionIndex                              = (*Infos)[i].sampleProperties[idx].sampleDescriptionIndex;
            info->initSegmentId                                 = (*Infos)[i].sampleProperties[idx].initSegmentId;
            info->durationTS                                    = (*Infos)[i].sampleProperties[idx].sampleDurationTS;
            ConvertFourCC(info->entryType, (*Infos)[i].sampleProperties[idx].sampleEntryType);
            info->id                                            = (*Infos)[i].sampleProperties[idx].sampleId;
            info->segmentId                                     = (*Infos)[i].sampleProperties[idx].segmentId;
            AssignSampleType(info->type, (*Infos)[i].sampleProperties[idx].sampleType);
            info->flags.flagsAsUInt                       = (*Infos)[i].sampleProperties[idx].sampleFlags.flagsAsUInt;
            info->flags.flags.isLeading                   = (*Infos)[i].sampleProperties[idx].sampleFlags.flags.is_leading;
            info->flags.flags.reserved                    = (*Infos)[i].sampleProperties[idx].sampleFlags.flags.reserved;
            info->flags.flags.sampleDegradationPriority   = (*Infos)[i].sampleProperties[idx].sampleFlags.flags.sample_degradation_priority;
            info->flags.flags.sampleHasRedundancy         = (*Infos)[i].sampleProperties[idx].sampleFlags.flags.sample_has_redundancy;
            info->flags.flags.sampleDependsOn             = (*Infos)[i].sampleProperties[idx].sampleFlags.flags.sample_depends_on;
            info->flags.flags.sampleIsNonSyncSample       = (*Infos)[i].sampleProperties[idx].sampleFlags.flags.sample_is_non_sync_sample;
            info->flags.flags.samplePaddingValue          = (*Infos)[i].sampleProperties[idx].sampleFlags.flags.sample_padding_value;
            trackInfo->samplePropertyArrays.push_back(info);
        }

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
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->getDisplayWidth(trackId, displayWidth);
}

int32_t OmafMP4VRReader::getDisplayHeight(uint32_t trackId, uint32_t& displayHeight) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->getDisplayHeight(trackId, displayHeight);
}

int32_t OmafMP4VRReader::getDisplayWidthFP(uint32_t trackId, uint32_t& displayWidth) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->getDisplayWidthFP(trackId, displayWidth);
}

int32_t OmafMP4VRReader::getDisplayHeightFP(uint32_t trackId, uint32_t& displayHeight) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->getDisplayHeightFP(trackId, displayHeight);
}

int32_t OmafMP4VRReader::getWidth(uint32_t trackId, uint32_t sampleId, uint32_t& width) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->getWidth(trackId, sampleId, width);
}

int32_t OmafMP4VRReader::getHeight(uint32_t trackId, uint32_t sampleId, uint32_t& height) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->getHeight(trackId, sampleId, height);
}

int32_t OmafMP4VRReader::getPlaybackDurationInSecs(uint32_t trackId, double& durationInSecs) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->getPlaybackDurationInSecs(trackId, durationInSecs);
}

int32_t OmafMP4VRReader::getTrackSampleListByType(uint32_t trackId,
                                                  VCD::OMAF::TrackSampleType sampleType,
                                                  std::vector<uint32_t>& sampleIds) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::TrackSampleType type;

    std::map<VCD::OMAF::TrackSampleType, MP4VR::TrackSampleType> valueMap;
    valueMap[VCD::OMAF::out_ref]     = MP4VR::out_ref;
    valueMap[VCD::OMAF::out_non_ref] = MP4VR::out_non_ref;
    valueMap[VCD::OMAF::non_out_ref] = MP4VR::non_out_ref;
    valueMap[VCD::OMAF::display]    = MP4VR::display;
    valueMap[VCD::OMAF::samples]     = MP4VR::samples;

    type = valueMap[sampleType];

    MP4VR::DynArray<uint32_t> ids;

    int32_t ret = pReader->getTrackSampleListByType(trackId, type, ids);

    for(uint32_t idx=0; idx<ids.size; idx++){
        sampleIds.push_back(ids[idx]);
    }

    return ret;

}

int32_t OmafMP4VRReader::getTrackSampleType(uint32_t trackId, uint32_t sampleId, VCD::OMAF::FourCC& trackSampleBoxType) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::FourCC cc;

    if (NULL == pReader)
    {
        return ERROR_NULL_PTR;
    }

    int32_t ret = pReader->getTrackSampleType(trackId, sampleId, cc);

    ConvertFourCC(trackSampleBoxType, cc);

    return ret;
}

int32_t OmafMP4VRReader::getExtractorTrackSampleData(uint32_t trackId,
                                   uint32_t sampleId,
                                   char* memoryBuffer,
                                   uint32_t& memoryBufferSize,
                                   bool videoByteStreamHeaders)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;
    int ret = pReader->getExtractorTrackSampleData( trackId, sampleId, memoryBuffer, memoryBufferSize, videoByteStreamHeaders);

    if(ret == MP4VR::MP4VRFileReaderInterface::MEMORY_TOO_SMALL_BUFFER)
        ret = OMAF_MEMORY_TOO_SMALL_BUFFER;
    return ret;
}

int32_t OmafMP4VRReader::getTrackSampleData(uint32_t trackId,
                                   uint32_t sampleId,
                                   char* memoryBuffer,
                                   uint32_t& memoryBufferSize,
                                   bool videoByteStreamHeaders)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;
    int ret = pReader->getTrackSampleData( trackId, sampleId, memoryBuffer, memoryBufferSize, videoByteStreamHeaders);

    if(ret == MP4VR::MP4VRFileReaderInterface::MEMORY_TOO_SMALL_BUFFER)
        ret = OMAF_MEMORY_TOO_SMALL_BUFFER;
    return ret;
}

int32_t OmafMP4VRReader::getTrackSampleOffset(uint32_t trackId, uint32_t sampleId, uint64_t& sampleOffset, uint32_t& sampleLength)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->getTrackSampleOffset(trackId, sampleId, sampleOffset, sampleLength);
}

int32_t OmafMP4VRReader::getDecoderConfiguration(uint32_t trackId, uint32_t sampleId, std::vector<VCD::OMAF::DecoderSpecificInfo>& decoderInfos) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    std::map<MP4VR::DecSpecInfoType, VCD::OMAF::DecSpecInfoType> valueMap;
    valueMap[MP4VR::AVC_SPS]     = VCD::OMAF::AVC_SPS;
    valueMap[MP4VR::AVC_PPS]     = VCD::OMAF::AVC_PPS;
    valueMap[MP4VR::HEVC_VPS]    = VCD::OMAF::HEVC_VPS;
    valueMap[MP4VR::HEVC_SPS]    = VCD::OMAF::HEVC_SPS;
    valueMap[MP4VR::HEVC_PPS]    = VCD::OMAF::HEVC_PPS;
    valueMap[MP4VR::AudioSpecificConfig]    = VCD::OMAF::AudioSpecificConfig;

    MP4VR::DynArray<MP4VR::DecoderSpecificInfo> *Infos = new MP4VR::DynArray<MP4VR::DecoderSpecificInfo>;

    int32_t ret = pReader->getDecoderConfiguration( trackId, sampleId, *Infos );

    for(uint32_t i=0; i<(*Infos).size; i++){
        DecoderSpecificInfo info;
        info.decodeSpecInfoType = valueMap[(*Infos)[i].decSpecInfoType];
        for(uint32_t j=0; j<(*Infos)[i].decSpecInfoData.size; j++)
        {
             //info.decSpecInfoData[j] = Infos[i].decSpecInfoData[j];
             info.decodeSpecInfoData.push_back((*Infos)[i].decSpecInfoData[j]);
        }

        decoderInfos.push_back(info);
    }

    delete Infos;
    return ret;
}

int32_t OmafMP4VRReader::getTrackTimestamps(uint32_t trackId, std::vector<VCD::OMAF::TimestampIDPair>& timestamps) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::DynArray<MP4VR::TimestampIDPair> id_pairs;

    int32_t ret = pReader->getTrackTimestamps( trackId, id_pairs );

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
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::DynArray<uint64_t> tms;

    int32_t ret = pReader->getTimestampsOfSample( trackId, sampleId, tms );

    for(uint32_t i=0; i<tms.size; i++){
        timestamps.push_back(tms[i]);
    }

    return ret;
}

int32_t OmafMP4VRReader::getSamplesInDecodingOrder(uint32_t trackId, std::vector<VCD::OMAF::TimestampIDPair>& sampleDecodingOrder) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::DynArray<MP4VR::TimestampIDPair> id_pairs;

    int32_t ret = pReader->getSamplesInDecodingOrder( trackId, id_pairs );

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
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::FourCC cc;

    int32_t ret = pReader->getDecoderCodeType(trackId, sampleId, cc);

    ConvertFourCC(decoderCodeType, cc);

    return ret;
}

int32_t OmafMP4VRReader::getSampleDuration(uint32_t trackId, uint32_t sampleId, uint32_t& sampleDuration) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->getSampleDuration(trackId, sampleId, sampleDuration);
}

int32_t OmafMP4VRReader::getPropertyChnl(uint32_t trackId, uint32_t sampleId, VCD::OMAF::chnlProperty& chProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::chnlProperty chProp;

    int32_t ret = pReader->getPropertyChnl(trackId, sampleId, chProp);

    chProperty.channelNumber      = chProp.channelCount;
    chProperty.definedLayout      = chProp.definedLayout;
    chProperty.objectNumber       = chProp.objectCount;
    chProperty.omittedChannelsMap = chProp.omittedChannelsMap;
    chProperty.streamStruct       = chProp.streamStructure;

    for(uint32_t i=0; i<chProp.channelLayouts.size; i++){
        ChannelLayout cl;
        cl.azimuthDegree         = chProp.channelLayouts[i].azimuth;
        cl.elevationDegree       = chProp.channelLayouts[i].elevation;
        cl.speakerPosition       = chProp.channelLayouts[i].speakerPosition;

        chProperty.channelLayoutArrays.push_back(cl);
    }


    return ret;
}

int32_t OmafMP4VRReader::getPropertySpatialAudio(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SpatialAudioProperty& spatialaudioproperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::SpatialAudioProperty spProp;

    int32_t ret = pReader->getPropertySpatialAudio(trackId, sampleId, spProp);

    spatialaudioproperty.ambisonicChannelOrder    = spProp.ambisonicChannelOrdering;
    spatialaudioproperty.ambisonicNorm            = spProp.ambisonicNormalization;
    spatialaudioproperty.ambisonicOrder           = spProp.ambisonicOrder;
    spatialaudioproperty.ambisonicType            = spProp.ambisonicType;
    spatialaudioproperty.version                  = spProp.version;


    for(uint32_t i=0; i<spProp.channelMap.size; i++){
        spatialaudioproperty.channelMap.push_back(spProp.channelMap[i]);
    }

    return ret;
}

int32_t OmafMP4VRReader::getPropertyStereoScopic3D(uint32_t trackId, uint32_t sampleId, VCD::OMAF::StereoScopic3DProperty& stereoscopicproperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::StereoScopic3DProperty ssProp;

    std::map<MP4VR::StereoScopic3DProperty, VCD::OMAF::StereoScopic3DProperty> valueMap;
    valueMap[MP4VR::StereoScopic3DProperty::MONOSCOPIC]                  = VCD::OMAF::StereoScopic3DProperty::MONO;
    valueMap[MP4VR::StereoScopic3DProperty::STEREOSCOPIC_TOP_BOTTOM]     = VCD::OMAF::StereoScopic3DProperty::STEREO_TOP_BOTTOM;
    valueMap[MP4VR::StereoScopic3DProperty::STEREOSCOPIC_LEFT_RIGHT]     = VCD::OMAF::StereoScopic3DProperty::STEREO_LEFT_RIGHT;
    valueMap[MP4VR::StereoScopic3DProperty::STEREOSCOPIC_STEREO_CUSTOM]  = VCD::OMAF::StereoScopic3DProperty::STEREO_STEREO;

    int32_t ret = pReader->getPropertyStereoScopic3D(trackId, sampleId, ssProp);

    stereoscopicproperty = valueMap[ssProp];

    return ret;
}

int32_t OmafMP4VRReader::getPropertySphericalVideoV1(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SphericalVideoV1Property& sphericalproperty) const
{
    return 0;
}

int32_t OmafMP4VRReader::getPropertySphericalVideoV2(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SphericalVideoV2Property& sphericalproperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::SphericalVideoV2Property sv2Prop;

    int32_t ret = pReader->getPropertySphericalVideoV2(trackId, sampleId, sv2Prop);

    sphericalproperty.pose.pitchFP = sv2Prop.pose.pitchFP;
    sphericalproperty.pose.rollFP  = sv2Prop.pose.rollFP;
    sphericalproperty.pose.yawFP   = sv2Prop.pose.yawFP;

    sphericalproperty.projection.cubemap.layout = sv2Prop.projection.cubemap.layout;
    sphericalproperty.projection.cubemap.padding = sv2Prop.projection.cubemap.padding;

    sphericalproperty.projection.equirectangular.bottomFP = sv2Prop.projection.equirectangular.boundsBottomFP;
    sphericalproperty.projection.equirectangular.leftFP   = sv2Prop.projection.equirectangular.boundsLeftFP;
    sphericalproperty.projection.equirectangular.rightFP  = sv2Prop.projection.equirectangular.boundsRightFP;
    sphericalproperty.projection.equirectangular.topFP    = sv2Prop.projection.equirectangular.boundsTopFP;

    std::map<MP4VR::ProjectionType, VCD::OMAF::ProjectionType> valueMap;
    valueMap[MP4VR::ProjectionType::UNKOWN]           = VCD::OMAF::ProjectionType::UNKOWN;
    valueMap[MP4VR::ProjectionType::CUBEMAP]          = VCD::OMAF::ProjectionType::CUBEMAP;
    valueMap[MP4VR::ProjectionType::EQUIRECTANGULAR]  = VCD::OMAF::ProjectionType::EQUIRECTANGULAR;
    valueMap[MP4VR::ProjectionType::MESH]             = VCD::OMAF::ProjectionType::MESH;

    sphericalproperty.projectionType = valueMap[sv2Prop.projectionType];

    return ret;
}

int32_t OmafMP4VRReader::getPropertyRegionWisePacking(uint32_t trackId, uint32_t sampleId, RegionWisePacking *rwpk) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::RegionWisePackingProperty rwpkProp;

    int32_t ret = pReader->getPropertyRegionWisePacking(trackId, sampleId, rwpkProp);

    rwpk->constituentPicMatching = rwpkProp.constituentPictureMatchingFlag;
    rwpk->packedPicHeight        = rwpkProp.packedPictureHeight;
    rwpk->packedPicWidth         = rwpkProp.packedPictureWidth;
    rwpk->projPicHeight          = rwpkProp.projPictureHeight;
    rwpk->projPicWidth           = rwpkProp.projPictureWidth;

    rwpk->numRegions = rwpkProp.regions.size;

    rwpk->rectRegionPacking = new RectangularRegionWisePacking[rwpk->numRegions];

    for(uint32_t i = 0; i < rwpkProp.regions.size; i++){

        rwpk->rectRegionPacking[i].guardBandFlag         = rwpkProp.regions[i].guardBandFlag;
        rwpk->rectRegionPacking[i].transformType         = (uint8_t)RegionWisePackingType::RECTANGULAR;
        rwpk->rectRegionPacking[i].bottomGbHeight        = rwpkProp.regions[i].region.rectangular.bottomGbHeight;
        rwpk->rectRegionPacking[i].gbNotUsedForPredFlag  = rwpkProp.regions[i].region.rectangular.gbNotUsedForPredFlag;
        rwpk->rectRegionPacking[i].gbType0               = rwpkProp.regions[i].region.rectangular.gbType0;
        rwpk->rectRegionPacking[i].gbType1               = rwpkProp.regions[i].region.rectangular.gbType1;
        rwpk->rectRegionPacking[i].gbType2               = rwpkProp.regions[i].region.rectangular.gbType2;
        rwpk->rectRegionPacking[i].gbType3               = rwpkProp.regions[i].region.rectangular.gbType3;
        rwpk->rectRegionPacking[i].leftGbWidth           = rwpkProp.regions[i].region.rectangular.leftGbWidth;
        rwpk->rectRegionPacking[i].packedRegHeight       = rwpkProp.regions[i].region.rectangular.packedRegHeight;
        rwpk->rectRegionPacking[i].packedRegLeft         = rwpkProp.regions[i].region.rectangular.packedRegLeft;
        rwpk->rectRegionPacking[i].packedRegTop          = rwpkProp.regions[i].region.rectangular.packedRegTop;
        rwpk->rectRegionPacking[i].packedRegWidth        = rwpkProp.regions[i].region.rectangular.packedRegWidth;
        rwpk->rectRegionPacking[i].projRegHeight         = rwpkProp.regions[i].region.rectangular.projRegHeight;
        rwpk->rectRegionPacking[i].projRegLeft           = rwpkProp.regions[i].region.rectangular.projRegLeft;
        rwpk->rectRegionPacking[i].projRegTop            = rwpkProp.regions[i].region.rectangular.projRegTop;
        rwpk->rectRegionPacking[i].projRegWidth          = rwpkProp.regions[i].region.rectangular.projRegWidth;
        rwpk->rectRegionPacking[i].rightGbWidth          = rwpkProp.regions[i].region.rectangular.rightGbWidth;
        rwpk->rectRegionPacking[i].topGbHeight           = rwpkProp.regions[i].region.rectangular.topGbHeight;
    }

    return ret;
}
int32_t OmafMP4VRReader::getPropertyCoverageInformation(uint32_t trackId, uint32_t sampleId, VCD::OMAF::CoverageInformationProperty& coviProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::CoverageInformationProperty ccProp;

    int32_t ret = pReader->getPropertyCoverageInformation(trackId, sampleId, ccProp);

    std::map<MP4VR::CoverageShapeType, VCD::OMAF::CoverageShapeType> valueMap_CS;
    valueMap_CS[MP4VR::CoverageShapeType::FOUR_GREAT_CIRCLES]                    = VCD::OMAF::CoverageShapeType::FOUR_CIRCLES;
    valueMap_CS[MP4VR::CoverageShapeType::TWO_AZIMUTH_AND_TWO_ELEVATION_CIRCLES] = VCD::OMAF::CoverageShapeType::TWO_AZIMUTH_TWO_ELEVATION_CIRCLES;

    std::map<MP4VR::ViewIdc, VCD::OMAF::ViewIdc> valueMap;
    valueMap[MP4VR::ViewIdc::MONOSCOPIC]     = VCD::OMAF::ViewIdc::MONOSCOPIC;
    valueMap[MP4VR::ViewIdc::LEFT]           = VCD::OMAF::ViewIdc::LEFT;
    valueMap[MP4VR::ViewIdc::RIGHT]          = VCD::OMAF::ViewIdc::RIGHT;
    valueMap[MP4VR::ViewIdc::LEFT_AND_RIGHT] = VCD::OMAF::ViewIdc::LEFT_AND_RIGHT;
    valueMap[MP4VR::ViewIdc::INVALID]        = VCD::OMAF::ViewIdc::INVALID;

    coviProperty.covShapeType   = valueMap_CS[ccProp.coverageShapeType];
    coviProperty.viewIdc      = valueMap[ccProp.defaultViewIdc];
    coviProperty.viewIdcPresenceFlag = ccProp.viewIdcPresenceFlag;

    for(uint32_t i=0; i<ccProp.sphereRegions.size; i++){
        CoverageSphereRegion region;
        region.azimuthRange    = ccProp.sphereRegions[i].azimuthRange;
        region.azimuthCentre   = ccProp.sphereRegions[i].centreAzimuth;
        region.elevationCentre = ccProp.sphereRegions[i].centreElevation;
        region.centreTilt      = ccProp.sphereRegions[i].centreTilt;
        region.elevationRange  = ccProp.sphereRegions[i].elevationRange;
        region.interpolate     = ccProp.sphereRegions[i].interpolate;
        region.viewIdc         = valueMap[ccProp.sphereRegions[i].viewIdc];
        coviProperty.sphereRegions.push_back(region);
    }

    return ret;
}

int32_t OmafMP4VRReader::getPropertyProjectionFormat(uint32_t trackId, uint32_t sampleId, VCD::OMAF::ProjectionFormatProperty& projectionFormatProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::ProjectionFormatProperty pfProp;

    int32_t ret = pReader->getPropertyProjectionFormat(trackId, sampleId, pfProp);

    std::map<MP4VR::OmafProjectionType, VCD::OMAF::OmafProjectionType> valueMap;
    valueMap[MP4VR::EQUIRECTANGULAR] = VCD::OMAF::EQUIRECTANGULAR;
    valueMap[MP4VR::CUBEMAP]         = VCD::OMAF::CUBEMAP;

    projectionFormatProperty.format = valueMap[pfProp.format];

    return ret;
}

int32_t OmafMP4VRReader::getPropertySchemeTypes(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SchemeTypesProperty& schemeTypesProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::SchemeTypesProperty stProp;

    int32_t ret = pReader->getPropertySchemeTypes(trackId, sampleId, stProp);

    ConvertFourCC(schemeTypesProperty.mainScheme.type, stProp.mainScheme.type);
    schemeTypesProperty.mainScheme.version = stProp.mainScheme.version;
    for(uint32_t i=0; i<stProp.mainScheme.uri.size; i++){
        schemeTypesProperty.mainScheme.uri.push_back(stProp.mainScheme.uri[i]);
    }

    for(uint32_t j=0; j<stProp.compatibleSchemeTypes.size; j++){
        SchemeType type;
        ConvertFourCC(type.type, stProp.compatibleSchemeTypes[j].type);
        type.version    = stProp.compatibleSchemeTypes[j].version;
        for(uint32_t k=0; k<stProp.compatibleSchemeTypes[j].uri.size; k++){
            type.uri.push_back(stProp.compatibleSchemeTypes[j].uri[k]);
        }
        schemeTypesProperty.compatibleSchemeTypes.push_back(type);
    }

    return ret;
}

int32_t OmafMP4VRReader::getPropertyStereoVideoConfiguration(uint32_t trackId, uint32_t sampleId, VCD::OMAF::PodvStereoVideoConfiguration& stereoVideoProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::PodvStereoVideoConfiguration psConf;

    std::map<MP4VR::PodvStereoVideoConfiguration, VCD::OMAF::PodvStereoVideoConfiguration> valueMap;
    valueMap[MP4VR::TOP_BOTTOM_PACKING]    = VCD::OMAF::TOP_BOTTOM_PACKING;
    valueMap[MP4VR::SIDE_BY_SIDE_PACKING]  = VCD::OMAF::SIDE_BY_SIDE_PACKING;
    valueMap[MP4VR::TEMPORAL_INTERLEAVING] = VCD::OMAF::TEMPORAL_INTERLEAVING;
    valueMap[MP4VR::MONOSCOPIC]            = VCD::OMAF::MONOSCOPIC;

    int32_t ret = pReader->getPropertyStereoVideoConfiguration(trackId, sampleId, psConf);

    stereoVideoProperty = valueMap[psConf];

    return ret;
}

int32_t OmafMP4VRReader::getPropertyRotation(uint32_t trackId, uint32_t sampleId, VCD::OMAF::Rotation& rotationProperty) const
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    MP4VR::Rotation rot;

    int32_t ret = pReader->getPropertyRotation(trackId, sampleId, rot);

    rotationProperty.pitch = rot.pitch;
    rotationProperty.roll = rot.roll;
    rotationProperty.yaw = rot.yaw;


    return ret;
}


int32_t OmafMP4VRReader::parseInitializationSegment(OmafSegment* streamInterface, uint32_t initSegmentId)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    SegmentStream *segment = new SegmentStream(streamInterface);
    if (NULL == segment) return ERROR_NULL_PTR;

    //return pReader->parseInitializationSegment(new SegmentStream(streamInterface), initSegmentId);
    return pReader->parseInitializationSegment(segment, initSegmentId);
}

int32_t OmafMP4VRReader::invalidateInitializationSegment(uint32_t initSegmentId)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    return pReader->invalidateInitializationSegment(initSegmentId);
}

int32_t OmafMP4VRReader::parseSegment( OmafSegment* streamInterface,
                                       uint32_t initSegmentId,
                                       uint32_t segmentId,
                                       uint64_t earliestPTSinTS )
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;

    SegmentStream *segment = new SegmentStream(streamInterface);
    if (NULL == segment) return ERROR_NULL_PTR;

    return pReader->parseSegment(segment, initSegmentId, segmentId, earliestPTSinTS);
}

int32_t OmafMP4VRReader::invalidateSegment(uint32_t initSegmentId, uint32_t segmentId)
{
    if(NULL == mMP4ReaderImpl) return ERROR_NULL_PTR;
    MP4VR::MP4VRFileReaderInterface* pReader = (MP4VR::MP4VRFileReaderInterface*)mMP4ReaderImpl;
    return pReader->invalidateSegment(initSegmentId, segmentId);
}

VCD_OMAF_END
