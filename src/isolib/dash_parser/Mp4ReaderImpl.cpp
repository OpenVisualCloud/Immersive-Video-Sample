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
//! \file:   Mp4ReaderImpl.cpp
//! \brief:  Mp4Reader class implementation
//!

#include "Mp4ReaderImpl.h"
#include "Mp4DataTypes.h"
#include "../atoms/AudSampEntryAtom.h"
#include "../atoms/AvcConfigAtom.h"
#include "../atoms/AvcSampEntry.h"
#include "../atoms/CleanApertureAtom.h"
#include "../atoms/FormAllocator.h"
#include "../atoms/HevcSampEntry.h"
#include "../atoms/InitViewOrientationSampEntry.h"
#include "../atoms/MediaDataAtom.h"
#include "../atoms/MetaAtom.h"
#include "../atoms/MetaDataSampEntryAtom.h"
#include "../atoms/MovieAtom.h"
#include "../atoms/MovieFragAtom.h"
#include "../atoms/Mp4AudSampEntryAtom.h"
#include "Mp4ReaderUtil.h"
#include "../atoms/SegIndexAtom.h"
#include "../atoms/TypeAtom.h"
#include "../atoms/UriMetaSampEntryAtom.h"

#include <limits.h>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstring>
#include <fstream>

#if defined(__GNUC__) && defined(__DEPRECATED)
#undef __DEPRECATED
#endif
#include <strstream>

using namespace std;

VCD_MP4_BEGIN

using PrestTS = DecodePts::PresentTimeTS;
const char* ident = "$Id: MP4VR version " MP4_BUILD_VERSION " $";

Mp4Reader* Mp4Reader::Create()
{
    return (new Mp4Reader());
}

void Mp4Reader::Destroy(Mp4Reader* mp4Reader)
{
    if (NULL != mp4Reader)
    {
        delete mp4Reader;
    }
}

int32_t Mp4Reader::Initialize(StreamIO* stream)
{
    InitSegmentId initSegId = 0;
    SegmentId segIndex         = 0;

    UniquePtr<StreamIOInternal> internalStream(new StreamIOInternal(stream));

    if (!internalStream->IsStreamGood())
    {
        return OMAF_FILE_OPEN_ERROR;
    }

    auto& io  = m_initSegProps[initSegId].segPropMap[segIndex].io;
    io.strIO = move(internalStream);
    io.size   = io.strIO->GetStreamSize();

    try
    {
        int32_t error = ReadStream(initSegId, segIndex);
        if (error)
        {
            return error;
        }
    }
    catch (const Exception& exc)
    {
        ISO_LOG(LOG_ERROR, "Error: %s\n", exc.what());
		return OMAF_FILE_READ_ERROR;
    }
    catch (const exception& e)
    {
        ISO_LOG(LOG_ERROR, "Error: %s\n", e.what());
		return OMAF_FILE_READ_ERROR;
    }
    return ERROR_NONE;
}

Mp4Reader::Mp4Reader()
    : m_readerSte(ReaderState::UNINITIALIZED)
{
}

const TrackBasicInfo& Mp4Reader::GetTrackBasicInfo(InitSegmentTrackId trackIdPair) const
{
    return m_initSegProps.at(trackIdPair.first).basicTrackInfos.at(trackIdPair.second);
}

TrackBasicInfo& Mp4Reader::GetTrackBasicInfo(InitSegmentTrackId trackIdPair)
{
    return m_initSegProps.at(trackIdPair.first).basicTrackInfos.at(trackIdPair.second);
}

bool Mp4Reader::CanFindTrackDecInfo(InitSegmentId initSegId, SegmentTrackId segTrackId) const
{
    const auto& segProps =
        m_initSegProps.at(initSegId).segPropMap.find(segTrackId.first);
    if (segProps != m_initSegProps.at(initSegId).segPropMap.end())
    {
        const auto& trackDecInfo = segProps->second.trackDecInfos.find(segTrackId.second);
        return trackDecInfo != segProps->second.trackDecInfos.end();
    }
    else
    {
        return false;
    }
}

const TrackDecInfo& Mp4Reader::GetTrackDecInfo(InitSegmentId initSegId, SegmentTrackId segTrackId) const
{
    return m_initSegProps.at(initSegId)
        .segPropMap.at(segTrackId.first)
        .trackDecInfos.at(segTrackId.second);
}

TrackDecInfo& Mp4Reader::GetTrackDecInfo(InitSegmentId initSegId, SegmentTrackId segTrackId)
{
    return m_initSegProps.at(initSegId)
        .segPropMap.at(segTrackId.first)
        .trackDecInfos.at(segTrackId.second);
}

bool Mp4Reader::FoundPrevSeg(InitSegmentId initSegId,
                                              SegmentId curSegmentId,
                                              SegmentId& prevSegId) const
{
    auto& segProps = m_initSegProps.at(initSegId).segPropMap.at(curSegmentId);
    auto& seqToSeg = m_initSegProps.at(segProps.initSegmentId).seqToSeg;
    auto iterator  = segProps.sequences.empty()
                        ? seqToSeg.end()
                        : seqToSeg.find(*segProps.sequences.begin());

    if (iterator == seqToSeg.end())
    {
        return false;
    }
    else
    {
        if (iterator != seqToSeg.begin())
        {
            --iterator;
            prevSegId = iterator->second;
            return true;
        }
        else
        {
            return false;
        }
    }
}

const TrackDecInfo* Mp4Reader::GetPrevTrackDecInfo(InitSegmentId initSegId,
                                             SegmentTrackId segTrackId) const
{
    SegmentId curSegId = segTrackId.first;
    ContextId ctxId    = segTrackId.second;
    SegmentId prevSegId;
    const TrackDecInfo* trackDecInfo = NULL;
    while (FoundPrevSeg(initSegId, curSegId, prevSegId))
    {
        auto& segProps =
            m_initSegProps.at(initSegId).segPropMap.at(prevSegId);

        if (segProps.trackDecInfos.count(ctxId))
        {
            trackDecInfo = &segProps.trackDecInfos.at(ctxId);
        }

        if (trackDecInfo)
        {
            return trackDecInfo;
        }

        curSegId = prevSegId;
    }

    return NULL;
}

const SampleInfoVector& Mp4Reader::GetSampInfos(
    InitSegmentId initSegId,
    SegmentTrackId segTrackId,
    ItemId& sampId) const
{
    auto& segProps = m_initSegProps.at(initSegId).segPropMap.at(segTrackId.first);
    sampId         = GetTrackDecInfo(initSegId, segTrackId).itemIdBase;
    auto& sampInfoVec = segProps.trackDecInfos.at(segTrackId.second).samples;
    return sampInfoVec;
}

const ParameterSetMap* Mp4Reader::GetParameterSetMap(InitSegTrackIdPair indexPair) const
{
    auto ctxIndex           = indexPair.first.second;
    InitSegmentId initSegId = indexPair.first.first;
    SegmentId segIndex;
    int result = GetSegIndex(indexPair, segIndex);
    if (result)
    {
        return NULL;
    }
    const auto& segProps = m_initSegProps.at(initSegId).segPropMap.at(segIndex);
    const auto& initSegProps = m_initSegProps.at(segProps.initSegmentId);
    SegmentTrackId segTrackId         = SegmentTrackId(segIndex, ctxIndex);
    const auto& idIter     = segProps.itemToParameterSetMap.find(
        make_pair(indexPair.first, indexPair.second - GetTrackDecInfo(initSegId, segTrackId).itemIdBase));
    if (idIter == segProps.itemToParameterSetMap.end())
    {
        return NULL;
    }

    int foundCnt = initSegProps.basicTrackInfos.at(indexPair.first.second)
                       .parameterSetMaps.count(idIter->second);
    if (foundCnt)
    {
        return &initSegProps.basicTrackInfos.at(indexPair.first.second)
                    .parameterSetMaps.at(idIter->second);
    }
    else
    {
        return NULL;
    }
}

void Mp4Reader::Close()
{
    m_readerSte = ReaderState::UNINITIALIZED;
    while (!m_initSegProps.empty())
    {
        DisableInitSeg(m_initSegProps.begin()->first.GetIndex());
    }
}

void Mp4Reader::CfgSegSidxFallback(InitSegmentId initSegId, SegmentTrackId segTrackId)
{
    SegmentId segIndex     = segTrackId.first;
    ContextId ctxId       = segTrackId.second;
    auto& segProps = m_initSegProps.at(initSegId).segPropMap.at(segIndex);

    PrestTS segDurTS = 0;
    for (auto& trackDecInfo : segProps.trackDecInfos)
    {
        segDurTS = max(segDurTS, trackDecInfo.second.durationTS);
    }

    SegmentId prevSegId;
    PrestTS curSegStartTS = 0;
    const TrackDecInfo* trackDecInfo = GetPrevTrackDecInfo(initSegId, segTrackId);
    if (trackDecInfo)
    {
        curSegStartTS = trackDecInfo->noSidxFallbackPTSTS;
    }
    PrestTS followingTS = curSegStartTS + segDurTS;

    if (followingTS > segProps.trackDecInfos.at(ctxId).noSidxFallbackPTSTS)
    {
        segProps.trackDecInfos.at(ctxId).noSidxFallbackPTSTS = followingTS;
    }
}

void Mp4Reader::RefreshCompTimes(InitSegmentId initSegId, SegmentId segIndex)
{
    using PMapIt = DecodePts::PMap::iterator;
    using PMapTSIt = DecodePts::PMapTS::iterator;
    using PMapTSRevIt = DecodePts::PMapTS::reverse_iterator;

    std::map<ContextId, TrackDecInfo>::iterator iter;
    for (iter = m_initSegProps.at(initSegId).segPropMap.at(segIndex).trackDecInfos.begin();
        iter != m_initSegProps.at(initSegId).segPropMap.at(segIndex).trackDecInfos.end();
        iter++)
    {
        auto& trackDecInfo = iter->second;
        if (trackDecInfo.pMap.size())
        {
            PMapIt iter1 = trackDecInfo.pMap.begin();
            for ( ; iter1 != trackDecInfo.pMap.end(); iter1++)
            {
                trackDecInfo.samples.at(iter1->second).compositionTimes.push_back(uint64_t(iter1->first));
            }

            PMapTSIt iter2 = trackDecInfo.pMapTS.begin();
            for ( ; iter2 != trackDecInfo.pMapTS.end(); iter2++)
            {
                trackDecInfo.samples.at(iter2->second).compositionTimesTS.push_back(uint64_t(iter2->first));
            }

            if (trackDecInfo.hasEditList)
            {
                PMapTSRevIt iter3 = trackDecInfo.pMapTS.rbegin();
                if (iter3 == trackDecInfo.pMapTS.rend())
                {
                    ISO_LOG(LOG_ERROR, "Failed to get TimeStamp !\n");
					throw exception();
                }
                trackDecInfo.samples.at(iter3->second).sampleDuration =
                    min(trackDecInfo.samples.at(iter3->second).sampleDuration,
                             static_cast<uint32_t>(trackDecInfo.durationTS - iter3->first));
            }
        }
    }
}

DashSegGroup Mp4Reader::CreateDashSegs(InitSegmentId initSegId)
{
    return DashSegGroup(*this, initSegId);
}

ConstDashSegGroup Mp4Reader::CreateDashSegs(InitSegmentId initSegId) const
{
    return ConstDashSegGroup(*this, initSegId);
}

int32_t Mp4Reader::ParseInitSeg(StreamIO* strIO, uint32_t initSegId)
{
    InitSegmentId initSegmentId = initSegId;
    SegmentId segIndex = 0;

    if (m_initSegProps.count(initSegmentId))
    {
        return ERROR_NONE;
    }

    ReaderState prevState = m_readerSte;
    m_readerSte          = ReaderState::INITIALIZING;

    SegmentProperties& segProps = m_initSegProps[initSegmentId].segPropMap[segIndex];
    SegmentIO& io                        = segProps.io;
    io.strIO.reset(new StreamIOInternal(strIO));
    if (io.strIO->PeekEOS())
    {
        m_readerSte = prevState;
        io.strIO.reset();
        ISO_LOG(LOG_ERROR, "Peek to EOS\n");
        return OMAF_FILE_READ_ERROR;
    }
    io.size = strIO->GetStreamSize();

    segProps.initSegmentId = initSegmentId;
    segProps.segmentId     = segIndex;

    bool ftypFound       = false;
    bool moovFound       = false;
    bool earliestPTSRead = false;

    int32_t error = ERROR_NONE;
    if (io.strIO->PeekEOS())
    {
        error = OMAF_INVALID_FILE_HEADER;
    }

    try
    {
        while (!error && !io.strIO->PeekEOS())
        {
            std::string boxType;
            int64_t boxSize = 0;
            Stream bitstream;
            error = ReadAtomParams(io, boxType, boxSize);
            if (!error)
            {
                ISO_LOG(LOG_INFO, "boxType is %s\n", boxType.c_str());
                if (boxType == "ftyp")
                {
                    if (ftypFound == true)
                    {
                        ISO_LOG(LOG_ERROR, "boxType is ftyp and is True!!!\n");
                        error = OMAF_FILE_READ_ERROR;
                        break;
                    }
                    ftypFound = true;

                    error = ReadAtom(io, bitstream);
                    if (!error)
                    {
                        FileTypeAtom ftyp;
                        ftyp.FromStream(bitstream);

                        std::set<std::string> supportedBrands;

                        if (ftyp.CheckCompatibleBrand("nvr1"))
                        {
                            supportedBrands.insert("[nvr1] ");
                        }

                        ISO_LOG(LOG_INFO, "Compatible brands found\n");

                        m_initSegProps[initSegmentId].ftyp = ftyp;
                    }
                }
                else if (boxType == "sidx")
                {
                    error = ReadAtom(io, bitstream);
                    if (!error)
                    {
                        SegmentIndexAtom sidx;
                        sidx.FromStream(bitstream);

                        if (!earliestPTSRead)
                        {
                            earliestPTSRead = true;
                        }
                        GenSegInAtom(sidx, m_initSegProps[initSegmentId].segmentIndex,
                                         io.strIO->TellOffset());
                    }
                }
                else if (boxType == "moov")
                {
                    if (moovFound == true)
                    {
                        ISO_LOG(LOG_ERROR, "boxType is moov and is Found!!!\n");
                        error = OMAF_FILE_READ_ERROR;
                        break;
                    }
                    moovFound = true;

                    error = ReadAtom(io, bitstream);
                    if (!error)
                    {
                        MovieAtom moov;
                        moov.FromStream(bitstream);
                        m_initSegProps[initSegmentId].moovProperties = ExtractMoovProps(moov);
                        m_initSegProps[initSegmentId].trackProperties =
                            FillTrackProps(initSegmentId, segIndex, moov);
                        m_initSegProps[initSegmentId].movieTScale =
                            moov.GetMovieHeaderAtom().GetTimeScale();
                        m_matrix = moov.GetMovieHeaderAtom().GetMatrix();
                    }
                }
                else if (boxType == "moof")
                {
                    ISO_LOG(LOG_WARNING, "Skipping root level 'moof' box - not allowed in Initialization Segment\n");
                    error = SkipAtom(io);
                }
                else if (boxType == "mdat")
                {
                    error = SkipAtom(io);
                }
                else
                {
                    ISO_LOG(LOG_WARNING, "Skipping root level box of unknown type '%s'\n", boxType.c_str());
					error = SkipAtom(io);
                }
            }
        }
    }
    catch (Exception& exc)
    {
        ISO_LOG(LOG_ERROR, "ParseInitSegment Exception Error: %s\n", exc.what());
        error = OMAF_FILE_READ_ERROR;
    }
    catch (exception& e)
    {
        ISO_LOG(LOG_ERROR, "ParseInitSegment exception Error:: %s\n", e.what());
        error = OMAF_FILE_READ_ERROR;
    }

    if (!error && (!ftypFound || !moovFound))
    {
        error = OMAF_INVALID_FILE_HEADER;
    }

    if (!error)
    {
        for (auto& trackDecInfo : segProps.trackDecInfos)
        {
            CfgSegSidxFallback(initSegmentId, make_pair(segIndex, trackDecInfo.first));
        }

        RefreshCompTimes(initSegmentId, segIndex);

        if ((!io.strIO->IsStreamGood()) && (!io.strIO->IsReachEOS()))
        {
            ISO_LOG(LOG_ERROR, "Stream is Good? %d\n", int32_t(io.strIO->IsStreamGood()));
            ISO_LOG(LOG_ERROR, "Reach EOS? %d\n", int32_t(io.strIO->IsReachEOS()));
            return OMAF_FILE_READ_ERROR;
        }
        io.strIO->ClearStatus();

        m_initSegProps[initSegmentId].fileProperty = GetFileProps();

        m_readerSte = ReaderState::READY;
    }
    else
    {
        DisableInitSeg(initSegmentId.GetIndex());
    }
    return error;
}

int32_t Mp4Reader::DisableInitSeg(uint32_t initSegId)
{
    bool isInitSegment = !!m_initSegProps.count(initSegId);
    if (isInitSegment)
    {
        for (auto& basicTrackInfo : m_initSegProps.at(initSegId).basicTrackInfos)
        {
            m_ctxInfoMap.erase(InitSegmentTrackId(initSegId, basicTrackInfo.first));
        }
        m_initSegProps.erase(initSegId);
    }
    return (isInitSegment) ? ERROR_NONE : OMAF_INVALID_SEGMENT;
}

int32_t Mp4Reader::ParseSeg(StreamIO* strIO,
                                          uint32_t initSegId,
                                          uint32_t segIndex,
                                          uint64_t earliestPTSinTS)
{
    if (m_initSegProps.count(initSegId) &&
        m_initSegProps.at(initSegId).segPropMap.count(segIndex))
    {
        return ERROR_NONE;
    }
    if (!m_initSegProps.count(initSegId))
    {
        return OMAF_INVALID_SEGMENT;
    }

    ReaderState prevState = m_readerSte;
    m_readerSte          = ReaderState::INITIALIZING;

    SegmentProperties& segProps =
        m_initSegProps.at(initSegId).segPropMap[segIndex];
    SegmentIO& io = segProps.io;
    io.strIO.reset(new StreamIOInternal(strIO));
    if (io.strIO->PeekEOS())
    {
        m_readerSte = prevState;
        io.strIO.reset();
        ISO_LOG(LOG_ERROR, "Peek to EOS!!!\n");
        return OMAF_FILE_READ_ERROR;
    }
    io.size = strIO->GetStreamSize();

    segProps.initSegmentId = initSegId;
    segProps.segmentId     = segIndex;

    bool stypFound       = false;
    bool earliestPTSRead = false;
    std::map<ContextId, PrestTS> earliestPTSTS;

    int32_t error = ERROR_NONE;
    try
    {
        while (!error && !io.strIO->PeekEOS())
        {
            std::string boxType;
            int64_t boxSize = 0;
            Stream bitstream;
            error = ReadAtomParams(io, boxType, boxSize);
            if (!error)
            {
                ISO_LOG(LOG_INFO, "boxType is %s\n", boxType.c_str());
                if (boxType == "styp")
                {
                    error = ReadAtom(io, bitstream);
                    if (!error)
                    {
                        SegmentTypeAtom styp;
                        styp.FromStream(bitstream);

                        if (stypFound == false)
                        {
                            segProps.styp = styp;
                            stypFound              = true;
                        }
                    }
                }
                else if (boxType == "sidx")
                {
                    error = ReadAtom(io, bitstream);
                    if (!error)
                    {
                        SegmentIndexAtom sidx;
                        sidx.FromStream(bitstream);

                        if (!earliestPTSRead)
                        {
                            earliestPTSRead = true;

                            for (auto& basicTrackInfo : m_initSegProps.at(initSegId).basicTrackInfos)
                            {
                                ContextId ctxId = basicTrackInfo.first;
                                earliestPTSTS[ctxId] =
                                    PrestTS(sidx.GetEarliestPresentationTime());
                            }
                        }
                    }
                }
                else if (boxType == "moof")
                {
                    const StreamIO::offset_t moofFirstByte = io.strIO->TellOffset();

                    error = ReadAtom(io, bitstream);
                    if (!error)
                    {
                        MovieFragmentAtom moof(
                            m_initSegProps.at(initSegId).moovProperties.fragmentSampleDefaults);
                        moof.SetMoofFirstByteOffset(static_cast<uint64_t>(moofFirstByte));
                        moof.FromStream(bitstream);

                        if (!earliestPTSRead)
                        {
                            for (auto& basicTrackInfo : m_initSegProps.at(initSegId).basicTrackInfos)
                            {
                                ContextId ctxId = basicTrackInfo.first;
                                if (earliestPTSinTS != UINT64_MAX)
                                {
                                    earliestPTSTS[ctxId] = PrestTS(earliestPTSinTS);
                                }
                                else if (const TrackDecInfo* precTrackDecInfo = GetPrevTrackDecInfo(
                                             initSegId, SegmentTrackId(segIndex, ctxId)))
                                {
                                    if (precTrackDecInfo)
                                    {
                                        earliestPTSTS[ctxId] = precTrackDecInfo->noSidxFallbackPTSTS;
                                    }
                                }
                                else
                                {
                                    earliestPTSTS[ctxId] = 0;
                                }
                            }

                            earliestPTSRead = true;
                        }

                        CtxIdPresentTSMap earliestPTSTSForTrack;
                        for (auto& trackFragmentAtom : moof.GetTrackFragmentAtoms())
                        {
                            auto ctxId = ContextId(trackFragmentAtom->GetTrackFragmentHeaderAtom().GetTrackId());
                            earliestPTSTSForTrack.insert(make_pair(ctxId, earliestPTSTS.at(ctxId)));
                        }

                        AddTrackProps(initSegId, segIndex, moof, earliestPTSTSForTrack);
                    }
                }
                else if (boxType == "mdat")
                {
                    error = SkipAtom(io);
                }
                else
                {
                    ISO_LOG(LOG_WARNING, "Skipping root level box of unknown type '%s'\n", boxType.c_str());
                    error = SkipAtom(io);
                }
            }
        }
    }
    catch (Exception& exc)
    {
        ISO_LOG(LOG_ERROR, "parseSegment Exception Error: %s\n", exc.what());
        error = OMAF_FILE_READ_ERROR;
    }
    catch (exception& e)
    {
        ISO_LOG(LOG_ERROR, "parseSegment exception Error: %s\n", e.what());
        error = OMAF_FILE_READ_ERROR;
    }

    if (!error)
    {
        for (auto& trackDecInfo : segProps.trackDecInfos)
        {
            CfgSegSidxFallback(initSegId, make_pair(segIndex, trackDecInfo.first));
        }

        RefreshCompTimes(initSegId, segIndex);

        if ((!io.strIO->IsStreamGood()) && (!io.strIO->IsReachEOS()))
        {
            ISO_LOG(LOG_ERROR, "Stream is Good? %d\n", int32_t(io.strIO->IsStreamGood()));
            ISO_LOG(LOG_ERROR, "Reach to EOS? %d\n", int32_t(io.strIO->IsReachEOS()));
            return OMAF_FILE_READ_ERROR;
        }
        io.strIO->ClearStatus();
        m_readerSte = ReaderState::READY;
    }
    else
    {
        DisableSeg(initSegId, segIndex);
    }
    return error;
}

int32_t Mp4Reader::DisableSeg(uint32_t initSegId, uint32_t segIndex)
{
    if (!m_initSegProps.count(initSegId))
    {
        return OMAF_INVALID_SEGMENT;
    }

    bool isSegment = !!m_initSegProps.at(initSegId).segPropMap.count(segIndex);
    if (isSegment)
    {
        auto& segProps = m_initSegProps.at(initSegId).segPropMap.at(segIndex);

        bool hasValidInitSeg = !!m_initSegProps.count(segProps.initSegmentId);
        if (hasValidInitSeg)
        {
            auto& seqToSeg =
                m_initSegProps.at(segProps.initSegmentId).seqToSeg;
            for (auto& sequence : segProps.sequences)
            {
                seqToSeg.erase(sequence);
            }
            SegmentProperties& segProps = m_initSegProps.at(initSegId).segPropMap[segIndex];
            SegmentIO& io = segProps.io;
            io.strIO.reset(NULL);
            m_initSegProps.at(initSegId).segPropMap.erase(segIndex);
        }
        else
        {
            return OMAF_INVALID_SEGMENT;
        }
    }

    return (isSegment) ? ERROR_NONE : OMAF_INVALID_SEGMENT;
}

int32_t Mp4Reader::GetSegIndex(uint32_t initSegId, VarLenArray<SegInfo>& segIndex)
{
    bool isInitSegment = !!m_initSegProps.count(initSegId);

    if (isInitSegment)
    {
        segIndex = m_initSegProps.at(initSegId).segmentIndex;
    }
    else
    {
        return OMAF_INVALID_SEGMENT;
    }
    return ERROR_NONE;
}


int32_t Mp4Reader::ParseSegIndex(StreamIO* strIO,
                                               VarLenArray<SegInfo>& segIndex)
{
    SegmentIO io;
    io.strIO.reset(new StreamIOInternal(strIO));
    if (io.strIO->PeekEOS())
    {
        io.strIO.reset();
        ISO_LOG(LOG_ERROR, "PeekEOS is true!!!\n");
        return OMAF_FILE_READ_ERROR;
    }
    io.size = strIO->GetStreamSize();

    int32_t error          = ERROR_NONE;
    bool segIndexFound = false;
    try
    {
        while (!error && !io.strIO->PeekEOS())
        {
            std::string boxType;
            int64_t boxSize = 0;
            Stream bitstream;
            error = ReadAtomParams(io, boxType, boxSize);
            if (!error)
            {
                if (boxType == "sidx")
                {
                    error = ReadAtom(io, bitstream);
                    if (!error)
                    {
                        SegmentIndexAtom sidx;
                        sidx.FromStream(bitstream);
                        GenSegInAtom(sidx, segIndex, io.strIO->TellOffset());
                        segIndexFound = true;
                        break;
                    }
                }
                else if (boxType == "styp" || boxType == "moof" || boxType == "mdat")
                {
                    error = SkipAtom(io);
                }
                else
                {
                    ISO_LOG(LOG_WARNING, "Skipping root level box of unknown type '%s\n'", boxType.c_str());
					error = SkipAtom(io);
                }
            }
        }
    }
    catch (Exception& exc)
    {
        ISO_LOG(LOG_ERROR, "ParseSegmentIndex Exception Error: %s\n", exc.what());
		error = OMAF_FILE_READ_ERROR;
    }
    catch (exception& e)
    {
        ISO_LOG(LOG_ERROR, "ParseSegmentIndex exception Error: %s\n", e.what());
		error = OMAF_FILE_READ_ERROR;
    }

    if (!segIndexFound)
    {
        ISO_LOG(LOG_ERROR, "ParseSegmentIndex couldn't find sidx box!\n");
		error = OMAF_INVALID_SEGMENT;
    }

    if (!error)
    {
        if ((!io.strIO->IsStreamGood()) && (!io.strIO->IsReachEOS()))
        {
            return OMAF_FILE_READ_ERROR;
        }
        io.strIO->ClearStatus();
    }
    return error;
}

void Mp4Reader::IsInited() const
{
    if (!(m_readerSte == ReaderState::INITIALIZING || m_readerSte == ReaderState::READY))
    {
        ISO_LOG(LOG_ERROR, "Mp4Reader is not initialized !\n");
		throw exception();
    }
}

int Mp4Reader::IsInitErr() const
{
    if (!(m_readerSte == ReaderState::INITIALIZING || m_readerSte == ReaderState::READY))
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }
    return ERROR_NONE;
}

Mp4Reader::CtxType Mp4Reader::GetCtxType(const InitSegmentTrackId trackIdPair) const
{
    const auto contextInfo = m_ctxInfoMap.find(trackIdPair);
    if (contextInfo == m_ctxInfoMap.end())
    {
        ISO_LOG(LOG_ERROR, "Context ID is invalide !\n");
		throw exception();
    }

    return contextInfo->second.ctxType;
}

int Mp4Reader::GetCtxTypeError(const InitSegmentTrackId trackIdPair,
                                             Mp4Reader::CtxType& ctxType) const
{
    const auto contextInfo = m_ctxInfoMap.find(trackIdPair);
    if (contextInfo == m_ctxInfoMap.end())
    {
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }

    ctxType = contextInfo->second.ctxType;
    return ERROR_NONE;
}

int32_t Mp4Reader::ReadStream(InitSegmentId initSegId, SegmentId segIndex)
{
    m_readerSte = ReaderState::INITIALIZING;

    SegmentIO& io = m_initSegProps[initSegId].segPropMap[segIndex].io;
    m_initSegProps.at(initSegId).segPropMap.at(segIndex).initSegmentId = initSegId;
    m_initSegProps.at(initSegId).segPropMap.at(segIndex).segmentId     = segIndex;

    bool ftypFound = false;
    bool moovFound = false;

    int32_t error = ERROR_NONE;
    if (io.strIO->PeekEOS())
    {
        error = OMAF_INVALID_FILE_HEADER;
    }

    while (!error && !io.strIO->PeekEOS())
    {
        std::string boxType;
        int64_t boxSize = 0;
        Stream bitstream;
        error = ReadAtomParams(io, boxType, boxSize);
        if (!error)
        {
            ISO_LOG(LOG_INFO, "boxType is %s\n", boxType.c_str());
            if (boxType == "ftyp")
            {
                if (ftypFound == true)
                {
                    return OMAF_FILE_READ_ERROR;
                }
                ftypFound = true;

                error = ReadAtom(io, bitstream);
                if (!error)
                {
                    FileTypeAtom ftyp;
                    ftyp.FromStream(bitstream);

                    std::set<std::string> supportedBrands;

                    if (ftyp.CheckCompatibleBrand("nvr1"))
                    {
                        supportedBrands.insert("[nvr1] ");
                    }

                    ISO_LOG(LOG_INFO, "Compatible brands found\n");

                    m_initSegProps[initSegId].ftyp = ftyp;
                }
            }
            else if (boxType == "styp")
            {
                error = ReadAtom(io, bitstream);
                if (!error)
                {
                    SegmentTypeAtom styp;
                    styp.FromStream(bitstream);

                    std::set<std::string> supportedBrands;

                    ISO_LOG(LOG_INFO, "Compatible brands found\n");
                }
            }
            else if (boxType == "sidx")
            {
                error = ReadAtom(io, bitstream);
                if (!error)
                {
                    SegmentIndexAtom sidx;
                    sidx.FromStream(bitstream);
                }
            }
            else if (boxType == "moov")
            {
                if (moovFound == true)
                {
                    ISO_LOG(LOG_ERROR, "boxType is moov and is True!!!\n");
                    return OMAF_FILE_READ_ERROR;
                }
                moovFound = true;

                error = ReadAtom(io, bitstream);
                if (!error)
                {
                    MovieAtom moov;
                    moov.FromStream(bitstream);
                    m_initSegProps[initSegId].moovProperties = ExtractMoovProps(moov);
                    m_initSegProps[initSegId].trackProperties =
                        FillTrackProps(initSegId, segIndex, moov);
                    m_initSegProps[initSegId].movieTScale =
                        moov.GetMovieHeaderAtom().GetTimeScale();
                    AddSegSeq(initSegId, segIndex, 0);
                    m_matrix = moov.GetMovieHeaderAtom().GetMatrix();
                }
            }
            else if (boxType == "moof")
            {
                const StreamIO::offset_t moofFirstByte = io.strIO->TellOffset();

                error = ReadAtom(io, bitstream);
                if (!error)
                {
                    MovieFragmentAtom moof(
                        m_initSegProps.at(initSegId).moovProperties.fragmentSampleDefaults);
                    moof.SetMoofFirstByteOffset(static_cast<uint64_t>(moofFirstByte));
                    moof.FromStream(bitstream);

                    CtxIdPresentTSMap earliestPTSTSForTrack;
                    AddTrackProps(initSegId, segIndex, moof, earliestPTSTSForTrack);

                    SegmentProperties& segProps =
                        m_initSegProps[initSegId].segPropMap[segIndex];
                    for (auto& trackFragmentAtom : moof.GetTrackFragmentAtoms())
                    {
                        auto ctxId = ContextId(trackFragmentAtom->GetTrackFragmentHeaderAtom().GetTrackId());
                        TrackDecInfo& trackDecInfo = segProps.trackDecInfos[ctxId];

                        if (trackDecInfo.samples.size())
                        {
                            int64_t sampleDataEndOffset = static_cast<int64_t>(
                            trackDecInfo.samples.rbegin()->dataOffset + trackDecInfo.samples.rbegin()->dataLength);
                            if (sampleDataEndOffset > io.size || sampleDataEndOffset < 0)
                            {
                                ISO_LOG(LOG_ERROR, "Sample data offset exceeds movie fragment !\n");
                                throw exception();
                            }
                        }
                    }
                }
            }
            else if (boxType == "mdat")
            {
                error = SkipAtom(io);
            }
            else
            {
                ISO_LOG(LOG_WARNING, "Skipping root level box of unknown type '%s'\n", boxType.c_str());
                error = SkipAtom(io);
            }
        }
    }

    if (!error && (!ftypFound || !moovFound))
    {
        error = OMAF_INVALID_FILE_HEADER;
    }

    if (!error)
    {
        RefreshCompTimes(initSegId, segIndex);

        if ((!io.strIO->IsStreamGood()) && (!io.strIO->IsReachEOS()))
        {
	    ISO_LOG(LOG_ERROR, "Stream is Good? %d\n", int32_t(io.strIO->IsStreamGood()));
	    ISO_LOG(LOG_ERROR, "Reach to EOS? %d\n", int32_t(io.strIO->IsReachEOS()));
            return OMAF_FILE_READ_ERROR;
        }
        io.strIO->ClearStatus();
        m_initSegProps[initSegId].fileProperty = GetFileProps();
        m_readerSte                                       = ReaderState::READY;
    }
    else
    {
        DisableInitSeg(initSegId.GetIndex());
    }
    return error;
}


Mp4Reader::ItemInfoMap Mp4Reader::ExtractItemInfoMap(const MetaAtom& metaAtom) const
{
    ItemInfoMap itemInfoMap;

    const uint32_t countNumberOfItems = metaAtom.GetItemInfoAtom().GetEntryCount();
    for (uint32_t i = 0; i < countNumberOfItems; ++i)
    {
        const ItemInfoEntry& item  = metaAtom.GetItemInfoAtom().GetItemInfoEntry(i);
        FourCCInt type             = item.GetItemType();
        const uint32_t itemId = item.GetItemID();
        if (!IsImageType(type))
        {
            ItemInfo itemInfo;
            itemInfo.type = type.GetString();
            itemInfoMap.insert({itemId, itemInfo});
        }
    }

    return itemInfoMap;
}

FileProperty Mp4Reader::GetFileProps() const
{
    FileProperty fileProperty;

    for (const auto& initSegment : m_initSegProps)
    {
        for (const auto& trackProps : initSegment.second.trackProperties)
        {
            if (trackProps.second.trackProperty.HasProperty(FeatureOfTrack::HasAlternatives))
            {
                fileProperty.SetProperty(FeatureOfFile::CONTAIN_ALT_TRACKS);
            }
        }
    }

    return fileProperty;
}

int32_t Mp4Reader::ConvertStrBytesToInt(SegmentIO& io, const unsigned int count, int64_t& result)
{
    int64_t value = 0;
    for (unsigned int i = 0; i < count; ++i)
    {
        value = (value << 8) | static_cast<int64_t>(io.strIO->GetOneByte());
        if (!io.strIO->IsStreamGood())
        {
            return OMAF_FILE_READ_ERROR;
        }
    }

    result = value;
    return ERROR_NONE;
}

int32_t Mp4Reader::ReadAtomParams(SegmentIO& io, std::string& boxType, int64_t& boxSize)
{
    const int64_t startLocation = io.strIO->TellOffset();

    int32_t error = ConvertStrBytesToInt(io, 4, boxSize);
    if (error)
    {
        return error;
    }

    static const size_t TYPE_LENGTH = 4;
    boxType.resize(TYPE_LENGTH);
    io.strIO->ReadStream(&boxType[0], TYPE_LENGTH);
    if (!io.strIO->IsStreamGood())
    {
        return OMAF_FILE_READ_ERROR;
    }

    if (boxSize == 1)
    {
        error = ConvertStrBytesToInt(io, 8, boxSize);
        if (error)
        {
            return error;
        }
    }

    int64_t boxEndOffset = startLocation + boxSize;
    if (boxSize < 8 || (boxEndOffset < 8) || ((io.size > 0) && (boxEndOffset > io.size)))
    {
        return OMAF_FILE_READ_ERROR;
    }

    LocateToOffset(io, startLocation);
    if (!io.strIO->IsStreamGood())
    {
        return OMAF_FILE_READ_ERROR;
    }
    return ERROR_NONE;
}

int32_t Mp4Reader::ReadAtom(SegmentIO& io, Stream& bitstream)
{
    std::string boxType;
    int64_t boxSize = 0;

    int32_t error = ReadAtomParams(io, boxType, boxSize);
    if (error)
    {
        return error;
    }

    std::vector<uint8_t> data((uint64_t) boxSize);
    io.strIO->ReadStream(reinterpret_cast<char*>(data.data()), boxSize);
    if (!io.strIO->IsStreamGood())
    {
        return OMAF_FILE_READ_ERROR;
    }
    bitstream.Clear();
    bitstream.Reset();
    bitstream.WriteArray(data, uint64_t(boxSize));
    return ERROR_NONE;
}

int32_t Mp4Reader::SkipAtom(SegmentIO& io)
{
    const int64_t startLocation = io.strIO->TellOffset();

    std::string boxType;
    int64_t boxSize = 0;
    int32_t error        = ReadAtomParams(io, boxType, boxSize);
    if (error)
    {
        return error;
    }

    LocateToOffset(io, startLocation + boxSize);
    if (!io.strIO->IsStreamGood())
    {
        return OMAF_FILE_READ_ERROR;
    }
    return ERROR_NONE;
}

int Mp4Reader::GetImgDims(unsigned int trackId,
                                            const uint32_t itemId,
                                            uint32_t& imgW,
                                            uint32_t& imgH) const
{
    CtxType ctxType;
    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    ItemId internalId(itemId);
    SegmentId segIndex;
    int result = GetSegIndex(trackIdPair, internalId, segIndex);
    if (result != ERROR_NONE)
    {
        return result;
    }
    SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);

    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }

    switch (ctxType)
    {
    case CtxType::META:
        imgH = m_metaInfo.at(segTrackId).imageInfoMap.at(internalId).height;
        imgW  = m_metaInfo.at(segTrackId).imageInfoMap.at(internalId).width;
        break;

    case CtxType::TRACK:
    {
        ItemId baseId;
        auto& sampInfos        = GetSampInfos(initSegId, segTrackId, baseId);
        auto& sampInfo         = sampInfos.at((internalId - baseId).GetIndex());
        imgH                   = sampInfo.height;
        imgW                   = sampInfo.width;
        break;
    }

    default:
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }
    return ERROR_NONE;
}

int32_t Mp4Reader::GetCtxItems(InitSegmentTrackId trackIdPair, IdVector& ctxItms) const
{
    ctxItms.clear();
    auto trackId = trackIdPair.second;
    for (const auto& segment : CreateDashSegs(trackIdPair.first))
    {
        SegmentId segIndex       = segment.segmentId;
        SegmentTrackId segTrackId = make_pair(segIndex, trackId);
        switch(GetCtxType(trackIdPair))
        {
        case CtxType::META:
        {
            ctxItms.reserve(m_metaInfo.at(segTrackId).imageInfoMap.size());
            for (const auto& imageInfo : m_metaInfo.at(segTrackId).imageInfoMap)
            {
                ctxItms.push_back(imageInfo.first.GetIndex());
            }
            break;
        }
        case CtxType::TRACK:
        {
            if (CanFindTrackDecInfo(trackIdPair.first, segTrackId))
            {
                ctxItms.reserve(GetTrackDecInfo(trackIdPair.first, segTrackId).samples.size());
                for (const auto& infoOfSamp : GetTrackDecInfo(trackIdPair.first, segTrackId).samples)
                {
                    ctxItms.push_back(infoOfSamp.sampleId);
                }
            }
            break;
        }
        default:
            return OMAF_INVALID_MP4READER_CONTEXTID;
        }
    }
    return ERROR_NONE;
}

void Mp4Reader::GetAvcSpecData(const DataVector& rawItemData, DataVector& itemData)
{
    Stream bitstream(rawItemData);
    while (bitstream.BytesRemain() > 0)
    {
        const unsigned int nalLength = bitstream.Read32();
        const uint8_t firstByte      = bitstream.Read8();

        itemData.push_back(0);
        itemData.push_back(0);
        itemData.push_back(0);
        itemData.push_back(1);

        itemData.push_back(firstByte);
        bitstream.ReadArray(itemData, nalLength - 1);
    }
}

void Mp4Reader::GetHevcSpecData(const DataVector& rawItemData, DataVector& itemData)
{
    Stream bitstream(rawItemData);
    while (bitstream.BytesRemain() > 0)
    {
        const unsigned int nalLength = bitstream.Read32();
        const uint8_t firstByte      = bitstream.Read8();
        HevcNalDefs naluType     = HevcNalDefs((firstByte >> 1) & 0x3f);

        if (itemData.size() == 0 || naluType == HevcNalDefs::VPS || naluType == HevcNalDefs::SPS ||
            naluType == HevcNalDefs::PPS)
        {
            itemData.push_back(0);
        }
        itemData.push_back(0);
        itemData.push_back(0);
        itemData.push_back(1);

        itemData.push_back(firstByte);
        bitstream.ReadArray(itemData, nalLength - 1);
    }
}

int32_t Mp4Reader::ParseAvcData(
    char* buf,
    uint32_t& bufSize)
{
    uint32_t outputOffset = 0;
    uint32_t byteOffset   = 0;
    uint32_t nalLength    = 0;

    while (outputOffset < bufSize)
    {
        nalLength                               = (uint8_t) buf[outputOffset + byteOffset];
        buf[outputOffset + byteOffset] = 0;
        byteOffset++;
        nalLength = (nalLength << 8) | (uint8_t) buf[outputOffset + byteOffset];
        buf[outputOffset + byteOffset] = 0;
        byteOffset++;
        nalLength = (nalLength << 8) | (uint8_t) buf[outputOffset + byteOffset];
        buf[outputOffset + byteOffset] = 0;
        byteOffset++;
        nalLength = (nalLength << 8) | (uint8_t) buf[outputOffset + byteOffset];
        buf[outputOffset + byteOffset] = 1;
        byteOffset++;
        outputOffset += nalLength + 4;
        byteOffset = 0;
    }
    return ERROR_NONE;
}

int32_t Mp4Reader::ParseHevcData(
    char* buf,
    uint32_t& bufSize)
{
    uint32_t outputOffset = 0;
    uint32_t byteOffset   = 0;
    uint32_t nalLength    = 0;

    while (outputOffset < bufSize)
    {
        nalLength                               = (uint8_t) buf[outputOffset + byteOffset];
        buf[outputOffset + byteOffset] = 0;
        byteOffset++;
        nalLength = (nalLength << 8) | (uint8_t) buf[outputOffset + byteOffset];
        buf[outputOffset + byteOffset] = 0;
        byteOffset++;
        nalLength = (nalLength << 8) | (uint8_t) buf[outputOffset + byteOffset];
        buf[outputOffset + byteOffset] = 0;
        byteOffset++;
        nalLength = (nalLength << 8) | (uint8_t) buf[outputOffset + byteOffset];
        buf[outputOffset + byteOffset] = 1;
        byteOffset++;
        outputOffset += nalLength + 4;
        byteOffset = 0;
    }
    return ERROR_NONE;
}

void Mp4Reader::ProcessDecoderConfigProperties(const InitSegmentTrackId /*trackIdPair*/)
{
}

void Mp4Reader::RefreshDecCodeType(InitSegmentId initSegIndex,
                                                   SegmentTrackId segTrackId,
                                                   const SampleInfoVector& sampleInfo,
                                                   size_t prevSampInfoSize)
{
    auto& decCodeType = m_initSegProps.at(initSegIndex)
                                   .segPropMap.at(segTrackId.first)
                                   .trackDecInfos.at(segTrackId.second)
                                   .decoderCodeTypeMap;
    for (size_t sampId = prevSampInfoSize; sampId < sampleInfo.size(); ++sampId)
    {
        auto& info = sampleInfo[sampId];
        decCodeType.insert(
            make_pair(info.sampleId, info.sampleEntryType));
    }
}

void Mp4Reader::RefreshItemToParamSet(ItemToParameterSetMap& itemToParameterSetMap,
                                                       InitSegmentTrackId trackIdPair,
                                                       const SampleInfoVector& sampleInfo,
                                                       size_t prevSampInfoSize)
{
    for (size_t sampId = prevSampInfoSize; sampId < sampleInfo.size(); ++sampId)
    {
        auto itemId            = InitSegTrackIdPair(trackIdPair, (uint32_t) sampId);
        auto parameterSetMapId = sampleInfo[sampId].sampleDescriptionIndex;
        itemToParameterSetMap.insert(make_pair(itemId, parameterSetMapId));
    }
}

void Mp4Reader::AddSegSeq(InitSegmentId initSegIndex,
                                             SegmentId segIndex,
                                             Sequence sequence)
{
    SegmentProperties& segProps =
        m_initSegProps.at(initSegIndex).segPropMap[segIndex];
    segProps.sequences.insert(sequence);
    auto& seqToSeg = m_initSegProps.at(initSegIndex).seqToSeg;
    seqToSeg.insert(make_pair(sequence, segIndex));
}

TrackPropertiesMap Mp4Reader::FillTrackProps(InitSegmentId initSegId,
                                                            SegmentId segIndex,
                                                            MovieAtom& moovAtom)
{
    TrackPropertiesMap trackPropsMap;

    std::vector<TrackAtom*> trackAtoms = moovAtom.GetTrackAtoms();
    for (auto trackAtom : trackAtoms)
    {
        TrackProperties trackProps;
        pair<TrackBasicInfo, TrackDecInfo> initAndTrackDecInfo =
            ExtractTrackDecInfo(trackAtom, moovAtom.GetMovieHeaderAtom().GetTimeScale());
        TrackBasicInfo& basicTrackInfo = initAndTrackDecInfo.first;
        TrackDecInfo& trackDecInfo         = initAndTrackDecInfo.second;

        if (basicTrackInfo.sampleEntryType == "hvc1" || basicTrackInfo.sampleEntryType == "hev1" ||
            basicTrackInfo.sampleEntryType == "hvc2" || basicTrackInfo.sampleEntryType == "avc1" ||
            basicTrackInfo.sampleEntryType == "avc3" || basicTrackInfo.sampleEntryType == "mp4a" ||
            basicTrackInfo.sampleEntryType == "urim" || basicTrackInfo.sampleEntryType == "mp4v" ||
            basicTrackInfo.sampleEntryType == "invo")
        {
            ContextId ctxIndex               = ContextId(trackAtom->GetTrackHeaderAtom().GetTrackID());
            InitSegmentTrackId trackIdPair   = make_pair(initSegId, ctxIndex);
            SegmentTrackId segTrackId         = make_pair(segIndex, ctxIndex);

            trackDecInfo.samples           = GenSampInfo(trackAtom);
            auto& initSegProps = m_initSegProps.at(initSegId);
            RefreshItemToParamSet(
                initSegProps.segPropMap[segIndex].itemToParameterSetMap, trackIdPair,
                trackDecInfo.samples);

            auto& storedTrackInfo =
                initSegProps.segPropMap[segIndex].trackDecInfos[trackIdPair.second];
            storedTrackInfo = move(trackDecInfo);
            m_initSegProps[initSegId].basicTrackInfos[trackIdPair.second] =
                move(basicTrackInfo);

            FillSampEntryMap(trackAtom, initSegId);
            RefreshDecCodeType(initSegId, segTrackId, storedTrackInfo.samples);

            trackProps.trackProperty     = GetTrackProps(trackAtom);
            trackProps.referenceTrackIds = GetRefTrackIds(trackAtom);
            trackProps.trackGroupIds     = GetTrackGroupIds(trackAtom);
            trackProps.alternateTrackIds = GetAlternateTrackIds(trackAtom, moovAtom);
            trackProps.alternateGroupId  = trackAtom->GetTrackHeaderAtom().GetAlternateGroup();
            if (trackAtom->GetEditAtom().get() && trackAtom->GetEditAtom()->GetEditListAtom())
            {
                trackProps.editBox = trackAtom->GetEditAtom();
            }

            CtxInfo contextInfo;
            contextInfo.ctxType         = CtxType::TRACK;
            m_ctxInfoMap[trackIdPair] = contextInfo;

            trackPropsMap.insert(make_pair(trackIdPair.second, move(trackProps)));
        }
    }

    for (auto& trackProps : trackPropsMap)
    {
        if (trackProps.second.trackProperty.HasProperty(FeatureOfTrack::IsVideo))
        {
            for (auto& associatedTrack : trackProps.second.referenceTrackIds["vdep"])
            {
                if (trackPropsMap.count(associatedTrack))
                {
                    trackPropsMap[associatedTrack].trackProperty.SetProperty(
                        FeatureOfTrack::HasAssociatedDepthTrack);
                }

            }

            if (trackProps.second.referenceTrackIds["vdep"].empty())
            {
                m_initSegProps.at(initSegId).corresTrackId = trackProps.first;
            }

            if (!(trackProps.second.referenceTrackIds["scal"].empty()))
            {
                m_initSegProps.at(initSegId).corresTrackId = trackProps.first;
            }
        }
    }

    return trackPropsMap;
}

ItemId Mp4Reader::GetSuccedentItmId(InitSegmentId initSegIndex,
                                               SegmentTrackId segTrackId) const
{
    SegmentId segIndex = segTrackId.first;
    ContextId ctxId   = segTrackId.second;
    ItemId nextItemIdBase;
    if (m_initSegProps.count(initSegIndex) &&
        m_initSegProps.at(initSegIndex).segPropMap.count(segIndex))
    {
        auto& segProps =
            m_initSegProps.at(initSegIndex).segPropMap.at(segIndex);
        if (segProps.trackDecInfos.count(ctxId))
        {
            auto& trackDecInfo = segProps.trackDecInfos.at(ctxId);
            if (trackDecInfo.samples.size())
            {
                nextItemIdBase = trackDecInfo.samples.rbegin()->sampleId + 1;
            }
            else
            {
                nextItemIdBase = trackDecInfo.itemIdBase;
            }
        }
    }
    return nextItemIdBase;
}

ItemId Mp4Reader::GetPrevItemId(InitSegmentId initSegIndex,
                                               SegmentTrackId segTrackId) const
{
    SegmentId prevSegId;
    ContextId ctxId = segTrackId.second;
    ItemId itemId;
    SegmentId curSegmentId = segTrackId.first;

    while (FoundPrevSeg(initSegIndex, curSegmentId, prevSegId) && itemId == ItemId(0))
    {
        if (m_initSegProps.at(initSegIndex)
                .segPropMap.at(prevSegId)
                .trackDecInfos.count(ctxId))
        {
            itemId = GetSuccedentItmId(initSegIndex, SegmentTrackId(prevSegId, ctxId));
        }
        curSegmentId = prevSegId;
    }
    return itemId;
}

void Mp4Reader::AddTrackProps(
    InitSegmentId initSegId,
    SegmentId segIndex,
    MovieFragmentAtom& moofAtom,
    const CtxIdPresentTSMap& earliestPTSTS)
{
    InitSegmentProperties& initSegProps = m_initSegProps[initSegId];
    SegmentProperties& segProps         = initSegProps.segPropMap[segIndex];

    AddSegSeq(initSegId, segIndex, m_nextSeq);
    m_nextSeq = m_nextSeq.GetIndex() + 1;

    uint64_t sampDataOffset = 0;
    bool firstTrackFragment                     = true;

    std::vector<TrackFragmentAtom*> trackFragmentAtoms = moofAtom.GetTrackFragmentAtoms();
    for (auto& trackFragmentAtom : trackFragmentAtoms)
    {
        auto ctxId = ContextId(trackFragmentAtom->GetTrackFragmentHeaderAtom().GetTrackId());
        SegmentId prevSegId;

        TrackDecInfo& trackDecInfo      = segProps.trackDecInfos[ctxId];
        size_t prevSampInfoSize = trackDecInfo.samples.size();
        bool hasSamps           = prevSampInfoSize > 0;
        if (auto* timeAtom = trackFragmentAtom->GetTrackFragmentBaseMediaDecodeTimeAtom())
        {
            trackDecInfo.nextPTSTS = PrestTS(timeAtom->GetBaseMediaDecodeTime());
        }
        else if (!hasSamps)
        {
            auto it = earliestPTSTS.find(ctxId);
            if (it != earliestPTSTS.end())
            {
                trackDecInfo.nextPTSTS = it->second;
            }
            else
            {
                trackDecInfo.nextPTSTS = 0;
            }
        }
        ItemId segmentItemIdBase =
            hasSamps ? trackDecInfo.itemIdBase
                       : GetPrevItemId(initSegId, SegmentTrackId(segIndex, ContextId(ctxId)));
        InitSegmentTrackId trackIdPair  = make_pair(initSegId, ctxId);
        const TrackBasicInfo& basicTrackInfo = GetTrackBasicInfo(trackIdPair);
        uint32_t sampDescId = trackFragmentAtom->GetTrackFragmentHeaderAtom().GetSampleDescrIndex();

        std::vector<TrackRunAtom*> trackRunAtoms = trackFragmentAtom->GetTrackRunAtoms();
        for (const auto trackRunAtom : trackRunAtoms)
        {
            ItemId trackrunItemIdBase =
                trackDecInfo.samples.size() > 0 ? trackDecInfo.samples.back().sampleId + 1 : segmentItemIdBase;
            uint64_t baseDataOffset = 0;
            if ((trackFragmentAtom->GetTrackFragmentHeaderAtom().GetFlags() &
                 TrackFragmentHeaderAtom::pDataOffset) != 0)
            {
                baseDataOffset = trackFragmentAtom->GetTrackFragmentHeaderAtom().GetBaseDataOffset();
            }
            else if ((trackFragmentAtom->GetTrackFragmentHeaderAtom().GetFlags() &
                      TrackFragmentHeaderAtom::IsBaseMoof) != 0)
            {
                baseDataOffset = moofAtom.GetMoofFirstByteOffset();
            }
            else
            {
                if (firstTrackFragment)
                {
                    baseDataOffset = moofAtom.GetMoofFirstByteOffset();
                }
                else
                {
                    baseDataOffset = sampDataOffset;
                }
            }
            if ((trackRunAtom->GetFlags() & TrackRunAtom::pDataOffset) != 0)
            {
                baseDataOffset += uint32_t(trackRunAtom->GetDataOffset());
            }

            AddSampsToTrackDecInfo(trackDecInfo, initSegProps, basicTrackInfo,
                                  initSegProps.trackProperties.at(ctxId), baseDataOffset,
                                  sampDescId, segmentItemIdBase, trackrunItemIdBase, trackRunAtom);
        }
        trackDecInfo.itemIdBase      = segmentItemIdBase;
        SegmentTrackId segTrackId = make_pair(segIndex, ctxId);
        RefreshDecCodeType(initSegId, segTrackId, trackDecInfo.samples, prevSampInfoSize);
        RefreshItemToParamSet(
            m_initSegProps[initSegId].segPropMap[segIndex].itemToParameterSetMap,
            trackIdPair, trackDecInfo.samples, prevSampInfoSize);

        if (trackDecInfo.samples.size())
        {
            sampDataOffset =
                trackDecInfo.samples.rbegin()->dataOffset + trackDecInfo.samples.rbegin()->dataLength;
        }
        firstTrackFragment = false;
    }
}

void Mp4Reader::AddSampsToTrackDecInfo(TrackDecInfo& trackDecInfo,
                                                const InitSegmentProperties& initSegProps,
                                                const TrackBasicInfo& basicTrackInfo,
                                                const TrackProperties& trackProps,
                                                const uint64_t baseDataOffset,
                                                const uint32_t sampDescId,
                                                ItemId itemIdBase,
                                                ItemId trackrunItemIdBase,
                                                const TrackRunAtom* trackRunAtom)
{
    using PMapWrap = DecodePts::PMap;
    using PMapTSWrap = DecodePts::PMapTS;
    using PMapIt = DecodePts::PMap::iterator;
    using PMapTSIt = DecodePts::PMapTS::iterator;
    const std::vector<TrackRunAtom::SampleDetails>& samples = trackRunAtom->GetSampleDetails();
    const uint32_t sampCnt                        = static_cast<uint32_t>(samples.size());
    const DecodePts::SampleIndex itemIdOffset         = trackrunItemIdBase.GetIndex() - itemIdBase.GetIndex();

    DecodePts ptsInfo;
    if (trackProps.editBox)
    {
        trackDecInfo.hasEditList = true;
        ptsInfo.SetAtom(trackProps.editBox->GetEditListAtom(), initSegProps.movieTScale,
                          basicTrackInfo.timeScale);
    }
    ptsInfo.SetAtom(trackRunAtom);
    ptsInfo.UnravelTrackRun();
    PMapWrap sampPrestTMap;
    PMapTSWrap sampPrestTMapTS;
    ptsInfo.SetLocalTime(static_cast<uint64_t>(trackDecInfo.nextPTSTS));
    ptsInfo.GetTimeTrackRun(basicTrackInfo.timeScale, sampPrestTMap);
    ptsInfo.GetTimeTrackRunTS(sampPrestTMapTS);
    PMapIt iter1 = sampPrestTMap.begin();
    for ( ; iter1 != sampPrestTMap.end(); iter1++)
    {
        trackDecInfo.pMap.insert(make_pair(iter1->first, iter1->second + itemIdOffset));
    }

    PMapTSIt iter2 = sampPrestTMapTS.begin();
    for ( ; iter2 != sampPrestTMapTS.end(); iter2++)
    {
        trackDecInfo.pMapTS.insert(make_pair(iter2->first, iter2->second + itemIdOffset));
    }

    int64_t durTS        = 0;
    uint64_t sampDataOffset = baseDataOffset;
    for (uint32_t sampId = 0; sampId < sampCnt; ++sampId)
    {
        SampleInfo infoOfSamp{};
        infoOfSamp.sampleId               = (trackrunItemIdBase + ItemId(sampId)).GetIndex();
        infoOfSamp.sampleEntryType        = basicTrackInfo.sampleEntryType;
        infoOfSamp.sampleDescriptionIndex = sampDescId;

        infoOfSamp.dataOffset = sampDataOffset;
        infoOfSamp.dataLength = samples.at(sampId).version0.pSize;
        sampDataOffset += infoOfSamp.dataLength;
        if (basicTrackInfo.sampleRes.count(sampDescId))
        {
            infoOfSamp.width  = basicTrackInfo.sampleRes.at(sampDescId).width;
            infoOfSamp.height = basicTrackInfo.sampleRes.at(sampDescId).height;
        }
        else
        {
            infoOfSamp.width  = 0;
            infoOfSamp.height = 0;
        }
        infoOfSamp.sampleDuration = samples.at(sampId).version0.pDuration;
        infoOfSamp.sampleType = samples.at(sampId).version0.pFlags.flags.sample_is_non_sync_sample == 0
                                    ? OUTPUT_REF_FRAME
                                    : OUTPUT_NONREF_FRAME;
        infoOfSamp.sampleFlags.flagsAsUInt = samples.at(sampId).version0.pFlags.flagsAsUInt;
        durTS += samples.at(sampId).version0.pDuration;
        trackDecInfo.samples.push_back(infoOfSamp);
    }
    trackDecInfo.durationTS += durTS;
    trackDecInfo.nextPTSTS += durTS;
}

IdVector Mp4Reader::GetAlternateTrackIds(TrackAtom* trackAtom, MovieAtom& moovAtom) const
{
    IdVector trackIds;
    const uint16_t groupId = trackAtom->GetTrackHeaderAtom().GetAlternateGroup();

    if (groupId == 0)
    {
        return trackIds;
    }

    unsigned int trackId         = trackAtom->GetTrackHeaderAtom().GetTrackID();
    std::vector<TrackAtom*> trackAtomVec = moovAtom.GetTrackAtoms();
    std::vector<TrackAtom*>::iterator iter = trackAtomVec.begin();
    for ( ; iter != trackAtomVec.end(); iter++)
    {
        const uint32_t foundTrackId = (*iter)->GetTrackHeaderAtom().GetTrackID();
        if ((trackId != foundTrackId) &&
            (groupId == (*iter)->GetTrackHeaderAtom().GetAlternateGroup()))
        {
            trackIds.push_back(foundTrackId);
        }
    }

    return trackIds;
}

TrackProperty Mp4Reader::GetTrackProps(TrackAtom* trackAtom) const
{
    TrackProperty trackProperty;

    TrackHeaderAtom tkhdAtom        = trackAtom->GetTrackHeaderAtom();
    HandlerAtom& handlerAtom        = trackAtom->GetMediaAtom().GetHandlerAtom();
    SampleTableAtom& stbl       = trackAtom->GetMediaAtom().GetMediaInformationAtom().GetSampleTableAtom();
    SampleDescriptionAtom& stsd = stbl.GetSampleDescriptionAtom();

    if (tkhdAtom.GetAlternateGroup() != 0)
    {
        trackProperty.SetProperty(FeatureOfTrack::HasAlternatives);
    }

    if (stbl.GetSampleToGroupAtoms().size() != 0)
    {
        trackProperty.SetProperty(FeatureOfTrack::HasSampleGroups);
    }

    if (handlerAtom.GetHandlerType() == "vide")
    {
        trackProperty.SetProperty(FeatureOfTrack::IsVideo);

        const std::vector<VisualSampleEntryAtom*> sampleEntries = stsd.GetSampleEntries<VisualSampleEntryAtom>();
        for (const auto& sampleEntry : sampleEntries)
        {
            if (sampleEntry->IsStereoscopic3DAtomPresent())
            {
                trackProperty.SetImmersiveProperty(ImmersiveProperty::HasVRStereoscopic3D);
            }
            if (sampleEntry->IsSphericalVideoV2AtomAtomPresent())
            {
                trackProperty.SetImmersiveProperty(ImmersiveProperty::HasVRV2SpericalVideo);
            }
        }

        if (trackAtom->GetHasSphericalVideoV1Atom())
        {
            trackProperty.SetImmersiveProperty(ImmersiveProperty::HasVRV1SpericalVideo);
            if (trackAtom->GetSphericalVideoV1Atom().GetGeneralMetaData().stereoType !=
                SphericalVideoV1Atom::StereoTypeV1 ::UNDEFINED)
            {
                trackProperty.SetImmersiveProperty(ImmersiveProperty::HasVRStereoscopic3D);
            }
        }
    }

    if (handlerAtom.GetHandlerType() == "soun")
    {
        trackProperty.SetProperty(FeatureOfTrack::IsAudio);

        const std::vector<MP4AudioSampleEntryAtom*> sampleEntries = stsd.GetSampleEntries<MP4AudioSampleEntryAtom>();
        for (const auto& sampleEntry : sampleEntries)
        {
            if (sampleEntry->GetType() == "mp4a")
            {
                if (sampleEntry->HasChannelLayoutAtom())
                {
                    if (sampleEntry->GetChannelLayoutAtom().GetStreamStructure() == 1)  // = channelStructured
                    {
                        trackProperty.SetImmersiveProperty(ImmersiveProperty::IsAudioLSpeakerChnlStructTrack);
                    }
                }
                if (sampleEntry->HasSpatialAudioAtom())
                {
                    trackProperty.SetImmersiveProperty(ImmersiveProperty::IsVRSpatialAudioTrack);
                }
                if (sampleEntry->HasNonDiegeticAudioAtom())
                {
                    trackProperty.SetImmersiveProperty(ImmersiveProperty::IsVRNonDiegeticAudioTrack);
                }
                break;
            }
        }
    }

    if (handlerAtom.GetHandlerType() == "meta")
    {
        trackProperty.SetProperty(FeatureOfTrack::IsMetadata);

        const std::vector<MetaDataSampleEntryAtom*> sampleEntries = stsd.GetSampleEntries<MetaDataSampleEntryAtom>();
        for (const auto& sampleEntry : sampleEntries)
        {
            if (sampleEntry->GetType() == "urim")
            {
                UriMetaSampleEntryAtom* uriMetaSampEntry  = (UriMetaSampleEntryAtom*) sampleEntry;
                UriMetaSampleEntryAtom::VRTMDType vrTMDType = uriMetaSampEntry->GetVRTMDType();

                switch (vrTMDType)
                {
                case UriMetaSampleEntryAtom::VRTMDType::UNKNOWN:
                default:
                    break;
                }
                break;
            }
        }
    }
    return trackProperty;
}

TypeToCtxIdsMap Mp4Reader::GetRefTrackIds(TrackAtom* trackAtom) const
{
    TypeToCtxIdsMap trackReferenceMap;

    if (trackAtom->GetHasTrackReferences())
    {
        const std::vector<TrackReferenceTypeAtom>& trackReferenceTypeAtoms =
            trackAtom->GetTrackReferenceAtom().GetTypeAtoms();
        for (const auto& trackReferenceTypeAtom : trackReferenceTypeAtoms)
        {
            trackReferenceMap[trackReferenceTypeAtom.GetType()] = map<std::uint32_t, VectorT, VectorT, ContextId>(
                trackReferenceTypeAtom.GetTrackIds(), [](std::uint32_t x) {
                    if ((x >> 16) != 0)
                    {
                        ISO_LOG(LOG_ERROR, "Context ID is invalid !\n");
						throw exception();
                    }
                    return ContextId(x);
                });
        }
    }

    return trackReferenceMap;
}

TypeToIdsMap Mp4Reader::GetTrackGroupIds(TrackAtom* trackAtom) const
{
    std::map<FourCCInt, IdVector> trackGroupMap;

    if (trackAtom->GetHasTrackGroup())
    {
        const std::vector<TrackGroupTypeAtom>& trackGroupTypeAtoms =
            trackAtom->GetTrackGroupAtom().GetTrackGroupTypeAtoms();
        for (unsigned int i = 0; i < trackGroupTypeAtoms.size(); i++)
        {
            IdVector trackGroupID;
            trackGroupID.push_back(trackGroupTypeAtoms.at(i).GetTrackGroupId());
            trackGroupMap[trackGroupTypeAtoms.at(i).GetType()] = trackGroupID;
        }
    }

    return trackGroupMap;
}

TypeToIdsMap Mp4Reader::GetSampGroupIds(TrackAtom* trackAtom) const
{
    std::map<FourCCInt, IdVector> sampleGroupIdsMap;

    SampleTableAtom& stbl = trackAtom->GetMediaAtom().GetMediaInformationAtom().GetSampleTableAtom();
    const std::vector<SampleToGroupAtom> sampleToGroupAtomes = stbl.GetSampleToGroupAtoms();
    for (const auto& sampleToGroupAtom : sampleToGroupAtomes)
    {
        const unsigned int numberOfSamps = sampleToGroupAtom.GetNumberOfSamples();
        IdVector sampleIds(numberOfSamps);
        for (unsigned int i = 0; i < numberOfSamps; ++i)
        {
            if (sampleToGroupAtom.GetSampleGroupDescriptionIndex(i) != 0)
            {
                sampleIds.push_back(i);
            }
        }
        sampleGroupIdsMap[sampleToGroupAtom.GetGroupingType()] = sampleIds;
    }

    return sampleGroupIdsMap;
}

pair<TrackBasicInfo, TrackDecInfo> Mp4Reader::ExtractTrackDecInfo(TrackAtom* trackAtom,
                                                                          uint32_t movieTimescale) const
{
    TrackBasicInfo basicTrackInfo;
    TrackDecInfo trackDecInfo;

    MediaHeaderAtom& mdhdAtom                = trackAtom->GetMediaAtom().GetMediaHeaderAtom();
    SampleTableAtom& stbl                = trackAtom->GetMediaAtom().GetMediaInformationAtom().GetSampleTableAtom();
    TrackHeaderAtom& trackHeaderAtom         = trackAtom->GetTrackHeaderAtom();
    const TimeToSampleAtom& timeToSampAtom = stbl.GetTimeToSampleAtom();
    shared_ptr<const CompositionOffsetAtom> compositionOffsetAtom = stbl.GetCompositionOffsetAtom();

    basicTrackInfo.width  = trackHeaderAtom.GetWidth();
    basicTrackInfo.height = trackHeaderAtom.GetHeight();

    SampleDescriptionAtom& stsd = stbl.GetSampleDescriptionAtom();
    FourCCInt handlerType         = trackAtom->GetMediaAtom().GetHandlerAtom().GetHandlerType();

    if (handlerType == "vide")
    {
        VisualSampleEntryAtom* sampleEntry =
            stsd.GetSampleEntry<VisualSampleEntryAtom>(1);
        if (sampleEntry)
        {
            basicTrackInfo.sampleEntryType = sampleEntry->GetType();
        }
    }
    else if (handlerType == "soun")
    {
        AudioSampleEntryAtom* sampleEntry =
            stsd.GetSampleEntry<AudioSampleEntryAtom>(1);
        if (sampleEntry)
        {
            basicTrackInfo.sampleEntryType = sampleEntry->GetType();
        }
    }
    else if (handlerType == "meta")
    {
        MetaDataSampleEntryAtom* sampleEntry =
            stsd.GetSampleEntry<MetaDataSampleEntryAtom>(1);
        if (sampleEntry)
        {
            basicTrackInfo.sampleEntryType = sampleEntry->GetType();
        }
    }

    basicTrackInfo.timeScale = mdhdAtom.GetTimeScale();

    shared_ptr<const EditAtom> editAtom = trackAtom->GetEditAtom();
    DecodePts ptsInfo;
    ptsInfo.SetAtom(&timeToSampAtom);
    ptsInfo.SetAtom(compositionOffsetAtom.get());
    if (editAtom)
    {
        trackDecInfo.hasEditList          = true;
        const EditListAtom* editListAtom = editAtom->GetEditListAtom();
        ptsInfo.SetAtom(editListAtom, movieTimescale, mdhdAtom.GetTimeScale());
    }
    ptsInfo.Unravel();

    trackDecInfo.durationTS = PrestTS(ptsInfo.GetSpan());
    trackDecInfo.pMap       = ptsInfo.GetTime(basicTrackInfo.timeScale);
    trackDecInfo.pMapTS     = ptsInfo.GetTimeTS();

    if (trackAtom->GetHasTrackTypeAtom())
    {
        trackDecInfo.hasTtyp = true;
        trackDecInfo.ttyp    = trackAtom->GetTrackTypeAtom();
    }

    return make_pair(basicTrackInfo, trackDecInfo);
}

SampleInfoVector Mp4Reader::GenSampInfo(TrackAtom* trackAtom) const
{
    SampleInfoVector sampInfos;

    SampleTableAtom& stbl       = trackAtom->GetMediaAtom().GetMediaInformationAtom().GetSampleTableAtom();
    SampleDescriptionAtom& stsd = stbl.GetSampleDescriptionAtom();
    SampleToChunkAtom& stsc     = stbl.GetSampleToChunkAtom();
    ChunkOffsetAtom& stco       = stbl.GetChunkOffsetAtom();
    SampleSizeAtom& stsz        = stbl.GetSampleSizeAtom();
    TimeToSampleAtom& stts      = stbl.GetTimeToSampleAtom();
    const FourCCInt handlerType   = trackAtom->GetMediaAtom().GetHandlerAtom().GetHandlerType();

    const std::vector<uint32_t> entrySize = stsz.GetEntrySize();
    const std::vector<uint64_t> chunkOffsets      = stco.GetChunkOffsets();
    const std::vector<uint32_t> sampleDeltas      = stts.GetSampleDeltas();

    const unsigned int sampCnt = stsz.GetSampleNum();

    if (sampCnt > entrySize.size() || sampCnt > sampleDeltas.size())
    {
        ISO_LOG(LOG_ERROR, "Segment file header is not correct !\n");
		throw exception();
    }

    sampInfos.reserve(sampCnt);

    uint32_t prevChunkId = 0;
    for (uint32_t sampId = 0; sampId < sampCnt; ++sampId)
    {
        SampleInfo infoOfSamp{};
        uint32_t sampDescId = 0;
        if (!stsc.GetSampleDescrIndex(sampId, sampDescId))
        {
            ISO_LOG(LOG_ERROR, "Segment file header is not correct !\n");
			throw exception();
        }

        infoOfSamp.sampleId               = sampId;
        infoOfSamp.dataLength             = entrySize.at(sampId);
        infoOfSamp.sampleDescriptionIndex = sampDescId;

        uint32_t chunkIndex = 0;
        if (!stsc.GetSampleChunkIndex(sampId, chunkIndex))
        {
            ISO_LOG(LOG_ERROR, "Segment file header is not correct !\n");
            throw exception();
        }

        if (chunkIndex != prevChunkId)
        {
            infoOfSamp.dataOffset = chunkOffsets.at(chunkIndex - 1);
            prevChunkId    = chunkIndex;
        }
        else
        {
            infoOfSamp.dataOffset = sampInfos.back().dataOffset + sampInfos.back().dataLength;
        }

        infoOfSamp.sampleDuration = sampleDeltas.at(sampId);

        if (handlerType == "vide")
        {
            if (const VisualSampleEntryAtom* sampleEntry =
                    stsd.GetSampleEntry<VisualSampleEntryAtom>(sampDescId))
            {
                infoOfSamp.width           = sampleEntry->GetWidth();
                infoOfSamp.height          = sampleEntry->GetHeight();
                infoOfSamp.sampleEntryType = sampleEntry->GetType();

                infoOfSamp.sampleType                                  = OUTPUT_NONREF_FRAME;
                infoOfSamp.sampleFlags.flags.sample_is_non_sync_sample = 1;
            }
        }
        else if (handlerType == "soun")
        {
            if (const AudioSampleEntryAtom* sampleEntry =
                    stsd.GetSampleEntry<AudioSampleEntryAtom>(infoOfSamp.sampleDescriptionIndex.GetIndex()))
            {
                infoOfSamp.sampleEntryType = sampleEntry->GetType();
                infoOfSamp.sampleType = OUTPUT_REF_FRAME;
            }
        }
        else if (handlerType == "meta")
        {
            if (const MetaDataSampleEntryAtom* sampleEntry =
                    stsd.GetSampleEntry<MetaDataSampleEntryAtom>(infoOfSamp.sampleDescriptionIndex.GetIndex()))
            {
                infoOfSamp.sampleEntryType = sampleEntry->GetType();
                infoOfSamp.sampleType = OUTPUT_REF_FRAME;
            }
        }

        sampInfos.push_back(infoOfSamp);
    }

    if (stbl.HasSyncSampleAtom() && (handlerType == "vide"))
    {
        const std::vector<uint32_t> syncSamps = stbl.GetSyncSampleAtom().get()->GetSyncSampleIds();
        for (unsigned int i = 0; i < syncSamps.size(); ++i)
        {
            uint32_t syncSamp                               = syncSamps.at(i) - 1;
            auto& infoOfSamp                                       = sampInfos.at(syncSamp);
            infoOfSamp.sampleType                                  = OUTPUT_REF_FRAME;
            infoOfSamp.sampleFlags.flags.sample_is_non_sync_sample = 0;
        }
    }

    return sampInfos;
}

MoovProperties Mp4Reader::ExtractMoovProps(const MovieAtom& moovAtom) const
{
    MoovProperties moovProperties;
    moovProperties.fragmentDuration = 0;

    if (moovAtom.IsMovieExtendsAtomPresent())
    {
        const MovieExtendsAtom* mvexAtom = moovAtom.GetMovieExtendsAtom();
        if (mvexAtom->IsMovieExtendsHeaderAtomPresent())
        {
            const MovieExtendsHeaderAtom& mehdAtom = mvexAtom->GetMovieExtendsHeaderAtom();
            moovProperties.fragmentDuration      = mehdAtom.GetFragmentDuration();
        }

        moovProperties.fragmentSampleDefaults.clear();
        for (const auto& track : mvexAtom->GetTrackExtendsAtoms())
        {
            moovProperties.fragmentSampleDefaults.push_back(track->GetFragmentSampleDefaults());
        }
    }

    return moovProperties;
}

void Mp4Reader::FillSampEntryMap(TrackAtom* trackAtom, InitSegmentId initSegId)
{
    InitSegmentTrackId trackIdPair =
        make_pair(initSegId, ContextId(trackAtom->GetTrackHeaderAtom().GetTrackID()));
    SampleDescriptionAtom& stsd =
        trackAtom->GetMediaAtom().GetMediaInformationAtom().GetSampleTableAtom().GetSampleDescriptionAtom();
    auto& parameterSetMaps =
        m_initSegProps[trackIdPair.first].basicTrackInfos[trackIdPair.second].parameterSetMaps;

    {
        const std::vector<HevcSampleEntry*> sampleEntries = stsd.GetSampleEntries<HevcSampleEntry>();
        unsigned int index                           = 1;
        for (auto& entry : sampleEntries)
        {
            ParameterSetMap parameterSetMap =
                GenDecoderParameterSetMap(entry->GetHevcConfigurationAtom().GetConfiguration());
            parameterSetMaps.insert(std::make_pair(SmpDesIndex(index), parameterSetMap));

            const auto* clapAtom = entry->GetClap();
            if (clapAtom != NULL)
            {
                ISO_LOG(LOG_INFO, "CleanApertureAtom reading not implemented\n");
            }

            SampleRes size = {entry->GetWidth(), entry->GetHeight()};
            GetTrackBasicInfo(trackIdPair).sampleRes.insert(make_pair(index, size));

            if (entry->IsStereoscopic3DAtomPresent())
            {
                GetTrackBasicInfo(trackIdPair)
                    .st3dProperties.insert(make_pair(index, Genst3d(entry->GetStereoscopic3DAtom())));
            }
            else if (trackAtom->GetHasSphericalVideoV1Atom())
            {
                GetTrackBasicInfo(trackIdPair)
                    .st3dProperties.insert(make_pair(index, Genst3d(trackAtom->GetSphericalVideoV1Atom())));
            }

            if (entry->IsSphericalVideoV2AtomAtomPresent())
            {
                GetTrackBasicInfo(trackIdPair)
                    .sv3dProperties.insert(make_pair(index, Gensv3d(entry->GetSphericalVideoV2Atom())));
            }
            else if (trackAtom->GetHasSphericalVideoV1Atom())
            {
                GetTrackBasicInfo(trackIdPair)
                    .sphericalV1Properties.insert(
                        make_pair(index, GenSphericalVideoV1Property(trackAtom->GetSphericalVideoV1Atom())));
            }

            FillRinfAtomInfo(GetTrackBasicInfo(trackIdPair), index, *entry);
            GetTrackBasicInfo(trackIdPair)
                .nalLengthSizeMinus1.insert(make_pair(
                    index, entry->GetHevcConfigurationAtom().GetConfiguration().GetLengthSizeMinus1()));

            ++index;
        }
    }

    {
        const std::vector<AvcSampleEntry*> sampleEntries = stsd.GetSampleEntries<AvcSampleEntry>();
        unsigned int index                          = 1;
        for (auto& entry : sampleEntries)
        {
            ParameterSetMap parameterSetMap =
                GenDecoderParameterSetMap(entry->GetAvcConfigurationAtom().GetConfiguration());
            parameterSetMaps.insert(std::make_pair(index, parameterSetMap));

            const auto* clapAtom = entry->GetClap();
            if (clapAtom != NULL)
            {
                ISO_LOG(LOG_INFO, "CleanApertureAtom reading not implemented\n");
            }

            SampleRes size = {entry->GetWidth(), entry->GetHeight()};
            GetTrackBasicInfo(trackIdPair).sampleRes.insert(make_pair(index, size));

            if (entry->IsStereoscopic3DAtomPresent())
            {
                GetTrackBasicInfo(trackIdPair)
                    .st3dProperties.insert(make_pair(index, Genst3d(entry->GetStereoscopic3DAtom())));
            }
            else if (trackAtom->GetHasSphericalVideoV1Atom())
            {
                GetTrackBasicInfo(trackIdPair)
                    .st3dProperties.insert(make_pair(index, Genst3d(trackAtom->GetSphericalVideoV1Atom())));
            }
            if (entry->IsSphericalVideoV2AtomAtomPresent())
            {
                GetTrackBasicInfo(trackIdPair)
                    .sv3dProperties.insert(make_pair(index, Gensv3d(entry->GetSphericalVideoV2Atom())));
            }
            else if (trackAtom->GetHasSphericalVideoV1Atom())
            {
                GetTrackBasicInfo(trackIdPair)
                    .sphericalV1Properties.insert(
                        make_pair(index, GenSphericalVideoV1Property(trackAtom->GetSphericalVideoV1Atom())));
            }

            FillRinfAtomInfo(GetTrackBasicInfo(trackIdPair), index, *entry);
            GetTrackBasicInfo(trackIdPair)
                .nalLengthSizeMinus1.insert(make_pair(
                    index, entry->GetAvcConfigurationAtom().GetConfiguration().GetLengthSizeMinus1()));

            ++index;
        }
    }

    {
        const std::vector<MP4AudioSampleEntryAtom*> sampleEntries = stsd.GetSampleEntries<MP4AudioSampleEntryAtom>();
        unsigned int index                                  = 1;
        for (auto& entry : sampleEntries)
        {
            ParameterSetMap parameterSetMap = GenDecoderParameterSetMap(entry->GetESDAtom());
            parameterSetMaps.insert(std::make_pair(index, parameterSetMap));

            if (entry->HasChannelLayoutAtom())
            {
                GetTrackBasicInfo(trackIdPair)
                    .chnlProperties.insert(make_pair(index, GenChnl(entry->GetChannelLayoutAtom())));
            }
            if (entry->HasSpatialAudioAtom())
            {
                GetTrackBasicInfo(trackIdPair)
                    .sa3dProperties.insert(make_pair(index, GenSA3D(entry->GetSpatialAudioAtom())));
            }
            ++index;
        }
    }
}

void Mp4Reader::FillRinfAtomInfo(TrackBasicInfo& basicTrackInfo,
                                                        unsigned int index,
                                                        const SampleEntryAtom& entry)
{
    auto rinfAtom = entry.GetRestrictedSchemeInfoAtom();
    if (rinfAtom)
    {
        if (rinfAtom->GetSchemeType() == "podv")
        {
            auto& povdAtom = rinfAtom->GetProjectedOmniVideoAtom();

            ProjFormat format = {
                (OmniProjFormat) povdAtom.GetProjectionFormatAtom().GetProjectFormat()};
            basicTrackInfo.pfrmProperties.insert(make_pair(index, format));

            if (povdAtom.HasRegionWisePackingAtom())
            {
                basicTrackInfo.rwpkProperties.insert(make_pair(index, Genrwpk(povdAtom.GetRegionWisePackingAtom())));
            }

            if (povdAtom.HasCoverageInformationAtom())
            {
                basicTrackInfo.coviProperties.insert(
                    make_pair(index, Gencovi(povdAtom.GetCoverageInformationAtom())));
            }

            if (povdAtom.HasRotationAtom())
            {
                auto rotation = povdAtom.GetRotationAtom().GetRotation();
                basicTrackInfo.rotnProperties.insert(
                    make_pair(index, Rotation{rotation.yaw, rotation.pitch, rotation.roll}));
            }
            else
            {
                basicTrackInfo.rotnProperties.insert(make_pair(index, Rotation{0, 0, 0}));
            }
        }

        if (rinfAtom->HasSchemeTypeAtom())
        {
            SchemeTypesPropertyInternal schemeTypes{};
            schemeTypes.mainScheme = rinfAtom->GetSchemeTypeAtom();
            for (auto compatSchemeType : rinfAtom->GetCompatibleSchemeTypes())
            {
                schemeTypes.compatibleSchemes.push_back(*compatSchemeType);
            }
            basicTrackInfo.schemeTypesProperties.insert(make_pair(index, schemeTypes));
        }

        if (rinfAtom->HasStereoVideoAtom())
        {
            VideoFramePackingType stviArrangement;
            stviArrangement = (VideoFramePackingType) rinfAtom->GetStereoVideoAtom()
                                  .GetStereoIndicationType()
                                  .typePOVD.compositionType;
            basicTrackInfo.stviProperties.insert(make_pair(index, stviArrangement));
        }
        else if (rinfAtom->GetSchemeType() == "podv")
        {
            basicTrackInfo.stviProperties.insert(make_pair(index, VideoFramePackingType::OMNI_MONOSCOPIC));
        }
    }
}

void Mp4Reader::GenSegInAtom(const SegmentIndexAtom& sidx,
                             SegmentIndex& segIndex,
                             int64_t dataOffset)
{
    uint64_t offset   = static_cast<uint64_t>(dataOffset) + sidx.GetFirstOffset();
    uint64_t totalDur = sidx.GetEarliestPresentationTime();

    std::vector<SegmentIndexAtom::Reference> segRefs = sidx.GetReferences();
    segIndex                                  = VarLenArray<SegInfo>(segRefs.size());

    uint32_t segId = 0;
    SegInfo *segInfo = NULL;
    for ( ; segId < segRefs.size(); segId++)
    {
        segInfo = &(segIndex[segId]);

        segInfo->segmentId       = segId + 1;
        segInfo->referenceId     = sidx.GetReferenceId();
        segInfo->timescale       = sidx.GetTimescale();
        segInfo->referenceType   = segRefs.at(segId).referenceType;
        segInfo->earliestPTSinTS = totalDur;
        totalDur += segRefs.at(segId).subsegmentDuration;
        segInfo->durationInTS    = segRefs.at(segId).subsegmentDuration;
        segInfo->startDataOffset = offset;
        segInfo->dataSize        = segRefs.at(segId).referencedSize;
        offset += segIndex[segId].dataSize;
        segInfo->startsWithSAP = segRefs.at(segId).startsWithSAP;
        segInfo->SAPType       = segRefs.at(segId).sapType;

    }
}

unsigned Mp4Reader::GenTrackId(InitSegmentTrackId idPair) const
{
    InitSegmentId initSegId = idPair.first;
    ContextId ctxIndex      = idPair.second;

    if ((ctxIndex.GetIndex() >> 16) != 0)
    {
        ISO_LOG(LOG_ERROR, "Segment file header is not correct !\n");
        throw exception();
    }

    unsigned int actualTrackId = (initSegId.GetIndex() << 16) | ctxIndex.GetIndex();
    return actualTrackId;
}

InitSegmentTrackId Mp4Reader::MakeIdPair(unsigned id) const
{
    unsigned int combinedId = id;

    InitSegmentId initSegId = InitSegmentId((combinedId >> 16) & 0xffff);
    ContextId ctxIndex = ContextId(combinedId & 0xffff);

    InitSegmentTrackId newIdPair = make_pair(initSegId, ctxIndex);
    return newIdPair;
}

uint64_t Mp4Reader::ParseNalLen(char* buffer) const
{
    uint64_t nalLen = 0;
    size_t id = 0;
    nalLen |= ((uint64_t)((uint8_t)(buffer[id]))) << 24;
    id++;
    nalLen |= (((uint64_t)((uint8_t)(buffer[id]))) << 16) & 0x0000000000ff0000;
    id++;
    nalLen |= (((uint64_t)((uint8_t)(buffer[id]))) << 8) & 0x000000000000ff00;
    id++;
    nalLen |= ((uint64_t)((uint8_t)(buffer[id]))) & 0x00000000000000ff;
    id++;
    return nalLen;
}

void Mp4Reader::WriteNalLen(uint64_t length, char* buffer) const
{
    if (!buffer)
    {
        ISO_LOG(LOG_ERROR, "The buffer is NULL for Nal Length !\n");
        throw exception();
    }

    buffer[0] = (char)((0xff000000 & length) >> 24);
    buffer[1] = (char)((0x00ff0000 & length) >> 16);
    buffer[2] = (char)((0x0000ff00 & length) >> 8);
    buffer[3] = (char)((uint8_t)length);
}

template <typename T, typename CustomCont>
VarLenArray<T> makeVarLenArray(const CustomCont& customCont)
{
    VarLenArray<T> varLenArray(customCont.size());
    for (typename CustomCont::const_iterator contIter = customCont.begin(); contIter != customCont.end(); ++contIter)
    {
        varLenArray.arrayElets[contIter - customCont.begin()] = T(*contIter);
    }
    return varLenArray;
}

template <typename CustomCont, typename CustomMap>
auto GenVarLenArrayMap(const CustomCont& customCont, CustomMap customMap) -> VarLenArray<decltype(customMap(*customCont.begin()))>
{
    VarLenArray<decltype(customMap(*customCont.begin()))> varLenArray(customCont.size());
    typename CustomCont::const_iterator contIter = customCont.begin();
    for ( ; contIter != customCont.end(); ++contIter)
    {
        varLenArray.arrayElets[contIter - customCont.begin()] = customMap(*contIter);
    }
    return varLenArray;
}

template <typename T, typename CustomCont>
VarLenArray<VarLenArray<T>> GenVarLenArray2d(const CustomCont& customCont)
{
    VarLenArray<VarLenArray<T>> varLenArray(customCont.size());
    typename CustomCont::const_iterator contIter = customCont.begin();
    for ( ; contIter != customCont.end(); ++contIter)
    {
        varLenArray.arrayElets[contIter - customCont.begin()] = makeVarLenArray<T>(*contIter);
    }
    return varLenArray;
}

template <typename CustomCont>
VarLenArray<FourCC> GenFourCCVarLenArray(const CustomCont& customCont)
{
    VarLenArray<FourCC> varLenArray(customCont.size());
    typename CustomCont::const_iterator contIter = customCont.begin();
    for ( ; contIter != customCont.end(); ++contIter)
    {
        varLenArray.arrayElets[contIter - customCont.begin()] = FourCC(contIter->c_str());
    }
    return varLenArray;
}

int32_t Mp4Reader::GetMajorBrand(FourCC& majorBrand,
                                           uint32_t initSegIndex,
                                           uint32_t segIndex) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    bool isInitSegment = m_initSegProps.count(initSegIndex) > 0;
    if (!isInitSegment)
    {
        return OMAF_INVALID_SEGMENT;
    }

    bool isSegment = false;
    if (segIndex != UINT32_MAX)
    {
        isSegment = m_initSegProps.at(initSegIndex).segPropMap.count(segIndex) > 0;
        if (!isSegment)
        {
            return OMAF_INVALID_SEGMENT;
        }
    }

    majorBrand = FourCC((isSegment ? m_initSegProps.at(initSegIndex)
                                         .segPropMap.at(segIndex)
                                         .styp.GetMajorBrand()
                                   : m_initSegProps.at(initSegIndex).ftyp.GetMajorBrand())
                            .c_str());
    return ERROR_NONE;
}


int32_t Mp4Reader::GetMinorVersion(uint32_t& minorVersion,
                                             uint32_t initSegIndex,
                                             uint32_t segIndex) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    bool isInitSegment = m_initSegProps.count(initSegIndex) > 0;
    if (!isInitSegment)
    {
        return OMAF_INVALID_SEGMENT;
    }

    bool isSegment = false;
    if (segIndex != UINT32_MAX)
    {
        isSegment = m_initSegProps.at(initSegIndex).segPropMap.count(segIndex) > 0;
        if (!isSegment)
        {
            return OMAF_INVALID_SEGMENT;
        }
    }

    minorVersion = (isSegment ? m_initSegProps.at(initSegIndex)
                                    .segPropMap.at(segIndex)
                                    .styp.GetMinorVersion()
                              : m_initSegProps.at(initSegIndex).ftyp.GetMinorVersion());

    return ERROR_NONE;
}


int32_t Mp4Reader::GetCompatibleBrands(VarLenArray<FourCC>& compatBrands,
                                                 uint32_t initSegIndex,
                                                 uint32_t segIndex) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    bool isInitSegment = m_initSegProps.count(initSegIndex) > 0;
    if (!isInitSegment)
    {
        return OMAF_INVALID_SEGMENT;
    }

    bool isSegment = false;
    if (segIndex != UINT32_MAX)
    {
        isSegment = m_initSegProps.at(initSegIndex).segPropMap.count(segIndex) > 0;
        if (!isSegment)
        {
            return OMAF_INVALID_SEGMENT;
        }
    }

    compatBrands = GenFourCCVarLenArray(
        isSegment ? m_initSegProps.at(initSegIndex)
                        .segPropMap.at(segIndex)
                        .styp.GetCompatibleBrands()
                  : m_initSegProps.at(initSegIndex).ftyp.GetCompatibleBrands());

    return ERROR_NONE;
}

int32_t Mp4Reader::GetTrackInformations(VarLenArray<TrackInformation>& outTrackInfos) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    size_t totalSize = 0;
    for (auto& propMap : m_initSegProps)
    {
        totalSize += propMap.second.basicTrackInfos.size();
    }

    outTrackInfos               = VarLenArray<TrackInformation>(totalSize);
    uint32_t basicTrackId       = 0;
    size_t offset               = 0;
    for (auto const& initSegment : m_initSegProps)
    {
        InitSegmentId initSegId = initSegment.first;
        uint32_t newTrackId     = basicTrackId;

        for (auto const& trackPropsKv : initSegment.second.trackProperties)
        {
            ContextId trackId                      = trackPropsKv.first;
            const TrackProperties& trackProps = trackPropsKv.second;
            outTrackInfos.arrayElets[newTrackId].initSegmentId = initSegId.GetIndex();
            outTrackInfos.arrayElets[newTrackId].trackId       = GenTrackId(make_pair(initSegment.first, trackId));
            outTrackInfos.arrayElets[newTrackId].alternateGroupId = trackProps.alternateGroupId;
            outTrackInfos.arrayElets[newTrackId].features         = trackProps.trackProperty.GetFeatureMask();
            outTrackInfos.arrayElets[newTrackId].vrFeatures       = trackProps.trackProperty.GetVRFeatureMask();
            outTrackInfos.arrayElets[newTrackId].timeScale =
                m_initSegProps.at(initSegId).basicTrackInfos.at(trackId).timeScale;
            outTrackInfos.arrayElets[newTrackId].frameRate = {};
            std::string tempURI                             = trackProps.trackURI;
            tempURI.push_back('\0');
            outTrackInfos.arrayElets[newTrackId].trackURI = makeVarLenArray<char>(tempURI);
            outTrackInfos.arrayElets[newTrackId].alternateTrackIds =
                makeVarLenArray<unsigned int>(trackProps.alternateTrackIds);

            outTrackInfos.arrayElets[newTrackId].referenceTrackIds =
                VarLenArray<TypeToTrackIDs>(trackProps.referenceTrackIds.size());
            offset = 0;
            for (auto const& reference : trackProps.referenceTrackIds)
            {
                outTrackInfos.arrayElets[newTrackId].referenceTrackIds[offset].type = FourCC(reference.first.GetUInt32());
                outTrackInfos.arrayElets[newTrackId].referenceTrackIds[offset].trackIds =
                    GenVarLenArrayMap(reference.second, [&](ContextId aContextId) {
                        return GenTrackId({initSegment.first, aContextId});
                    });
                offset++;
            }

            outTrackInfos.arrayElets[newTrackId].trackGroupIds =
                VarLenArray<TypeToTrackIDs>(trackProps.trackGroupIds.size());
            offset = 0;
            for (auto const& group : trackProps.trackGroupIds)
            {
                outTrackInfos.arrayElets[newTrackId].trackGroupIds[offset].type = FourCC(group.first.GetUInt32());
                outTrackInfos.arrayElets[newTrackId].trackGroupIds[offset].trackIds =
                    makeVarLenArray<unsigned int>(group.second);
                offset++;
            }
            newTrackId++;
        }

        std::map<uint32_t, size_t> trackSampCounts;

        std::vector<ContextId> idPairVec;
        {
            uint32_t count = 0;
            for (auto const& basicTrackInfosKv : m_initSegProps.at(initSegId).basicTrackInfos)
            {
                idPairVec.push_back(basicTrackInfosKv.first);
                trackSampCounts[count + basicTrackId] = 0u;
                ++count;
            }
        }


        for (auto const& allSegmentProperties : initSegment.second.segPropMap)
        {
            if (allSegmentProperties.second.initSegmentId == initSegId)
            {
                auto& segTrackInfos = allSegmentProperties.second.trackDecInfos;
                newTrackId         = basicTrackId;
                for (ContextId ctxId : idPairVec)
                {
                    auto trackDecInfo = segTrackInfos.find(ctxId);
                    if (trackDecInfo != segTrackInfos.end())
                    {
                        trackSampCounts[newTrackId] += trackDecInfo->second.samples.size();
                    }
                    ++newTrackId;
                }
            }
        }

        for (auto const& trackSampCount : trackSampCounts)
        {
            auto counttrackid                                  = trackSampCount.first;
            auto count                                         = trackSampCount.second;
            outTrackInfos.arrayElets[counttrackid].sampleProperties = VarLenArray<TrackSampInfo>(count);
            outTrackInfos.arrayElets[counttrackid].maxSampleSize    = 0;
        }

        std::map<uint32_t, size_t> sampOffset;
        for (auto const& segment : CreateDashSegs(initSegId))
        {
            newTrackId = basicTrackId;
            std::vector<ContextId>::iterator ctxIdIter = idPairVec.begin();
            for ( ; ctxIdIter != idPairVec.end(); ctxIdIter++)
            //for (ContextId ctxId : idPairVec)
            {
                std::map<ContextId, TrackDecInfo>::const_iterator decInfoIter;
                decInfoIter = segment.trackDecInfos.find((*ctxIdIter));
                //auto trackInfoIt = segment.trackDecInfos.find((*ctxIdIter));
                if (decInfoIter != segment.trackDecInfos.end())
                {
                    auto& trackDecInfo = decInfoIter->second;
                    offset             = sampOffset[newTrackId];

                    if (trackDecInfo.hasTtyp)
                    {
                        auto& tAtom                                          = trackDecInfo.ttyp;
                        outTrackInfos.arrayElets[newTrackId].hasTypeInformation = true;

                        outTrackInfos.arrayElets[newTrackId].type.majorBrand   = tAtom.GetMajorBrand().c_str();
                        outTrackInfos.arrayElets[newTrackId].type.minorVersion = tAtom.GetMinorVersion();

                        std::vector<FourCC> convertedCompatibleBrands;
                        for (auto& compatibleBrand : tAtom.GetCompatibleBrands())
                        {
                            convertedCompatibleBrands.push_back(FourCC(compatibleBrand.c_str()));
                        }
                        outTrackInfos.arrayElets[newTrackId].type.compatibleBrands =
                            makeVarLenArray<FourCC>(convertedCompatibleBrands);
                    }
                    else
                    {
                        outTrackInfos.arrayElets[newTrackId].hasTypeInformation = false;
                    }

                    if (trackDecInfo.samples.size() > 0)
                    {
                        uint32_t delta;
                        if (trackDecInfo.samples.size() >= 3)
                        {
                            delta = trackDecInfo.samples.at(1).sampleDuration;
                        }
                        else
                        {
                            delta = trackDecInfo.samples.at(0).sampleDuration;
                        }
                        auto timeScale =
                            m_initSegProps.at(initSegId).basicTrackInfos.at((*ctxIdIter)).timeScale;
                        outTrackInfos.arrayElets[newTrackId].frameRate = RatValue{timeScale, delta};
                    }

                    for (auto const& sample : trackDecInfo.samples)
                    {
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleId =
                            ItemId(sample.sampleId).GetIndex();
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleEntryType =
                            FourCC(sample.sampleEntryType.GetUInt32());
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleDescriptionIndex =
                            sample.sampleDescriptionIndex.GetIndex();
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleType = sample.sampleType;
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].initSegmentId =
                            segment.initSegmentId.GetIndex();
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].segmentId = segment.segmentId.GetIndex();
                        if (sample.compositionTimes.size())
                        {
                            outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].earliestTStamp =
                                sample.compositionTimes.at(0);
                            outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].earliestTStampTS =
                                sample.compositionTimesTS.at(0);
                        }
                        else
                        {
                            outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].earliestTStamp   = 0;
                            outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].earliestTStampTS = 0;
                        }
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleFlags.flagsAsUInt =
                            sample.sampleFlags.flagsAsUInt;
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleDurationTS =
                            sample.sampleDuration;

                        unsigned int sampleSize = sample.dataLength;
                        if (sampleSize > outTrackInfos.arrayElets[newTrackId].maxSampleSize)
                        {
                            outTrackInfos.arrayElets[newTrackId].maxSampleSize = sampleSize;
                        }
                        offset++;
                    }
                    sampOffset[newTrackId] = offset;
                }
                ++newTrackId;
            }
        }

        basicTrackId += static_cast<uint32_t>(initSegment.second.basicTrackInfos.size());
    }

    return ERROR_NONE;
}

int32_t Mp4Reader::GetTrackInformation(VarLenArray<TrackInformation>& outTrackInfos) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    size_t totalSize = m_initSegProps.size();
    outTrackInfos = std::move(VarLenArray<TrackInformation>(totalSize));
    uint32_t basicTrackId       = 0;
    size_t offset               = 0;
    for (auto const& initSegment : m_initSegProps)
    {
        InitSegmentId initSegId = initSegment.first;
        uint32_t newTrackId     = basicTrackId;

        auto trackPropsKv = --initSegment.second.trackProperties.end();
        ContextId trackId                      = trackPropsKv->first;
        const TrackProperties& trackProps = trackPropsKv->second;
        outTrackInfos.arrayElets[newTrackId].initSegmentId = initSegId.GetIndex();
        outTrackInfos.arrayElets[newTrackId].trackId       = GenTrackId(make_pair(initSegment.first, trackId));
        outTrackInfos.arrayElets[newTrackId].alternateGroupId = trackProps.alternateGroupId;
        outTrackInfos.arrayElets[newTrackId].features         = trackProps.trackProperty.GetFeatureMask();
        outTrackInfos.arrayElets[newTrackId].vrFeatures       = trackProps.trackProperty.GetVRFeatureMask();
        outTrackInfos.arrayElets[newTrackId].timeScale =
            m_initSegProps.at(initSegId).basicTrackInfos.at(trackId).timeScale;
        outTrackInfos.arrayElets[newTrackId].frameRate = {};
        std::string tempURI                             = trackProps.trackURI;
        tempURI.push_back('\0');
        outTrackInfos.arrayElets[newTrackId].trackURI = makeVarLenArray<char>(tempURI);
        outTrackInfos.arrayElets[newTrackId].alternateTrackIds =
            makeVarLenArray<unsigned int>(trackProps.alternateTrackIds);
        outTrackInfos.arrayElets[newTrackId].referenceTrackIds =
                VarLenArray<TypeToTrackIDs>(trackProps.referenceTrackIds.size());
        offset = 0;
        for (auto const& reference : trackProps.referenceTrackIds)
        {
            outTrackInfos.arrayElets[newTrackId].referenceTrackIds[offset].type = FourCC(reference.first.GetUInt32());
            outTrackInfos.arrayElets[newTrackId].referenceTrackIds[offset].trackIds =
                GenVarLenArrayMap(reference.second, [&](ContextId aContextId) {
                    return GenTrackId({initSegment.first, aContextId});
                });
            offset++;
        }

        outTrackInfos.arrayElets[newTrackId].trackGroupIds =
            VarLenArray<TypeToTrackIDs>(trackProps.trackGroupIds.size());
        offset = 0;
        for (auto const& group : trackProps.trackGroupIds)
        {
            outTrackInfos.arrayElets[newTrackId].trackGroupIds[offset].type = FourCC(group.first.GetUInt32());
            outTrackInfos.arrayElets[newTrackId].trackGroupIds[offset].trackIds =
                makeVarLenArray<unsigned int>(group.second);
            offset++;
        }

        std::map<uint32_t, size_t> trackSampCounts;

        std::vector<ContextId> idPairVec;
        auto basicTrackInfosKv = --m_initSegProps.at(initSegId).basicTrackInfos.end();
        idPairVec.push_back(basicTrackInfosKv->first);
        trackSampCounts[basicTrackId] = 0u;

        for (auto const& allSegmentProperties : initSegment.second.segPropMap)
        {
            if (allSegmentProperties.second.initSegmentId == initSegId)
            {
                auto& segTrackInfos = allSegmentProperties.second.trackDecInfos;
                newTrackId         = basicTrackId;
                auto ctxId = --idPairVec.end();
                auto trackDecInfo = segTrackInfos.find(*ctxId);
                if (trackDecInfo != segTrackInfos.end())
                {
                    trackSampCounts[newTrackId] += trackDecInfo->second.samples.size();
                }
            }
        }

        auto trackSampCount = --trackSampCounts.end();
        auto counttrackid                                  = trackSampCount->first;
        auto count                                         = trackSampCount->second;
        outTrackInfos.arrayElets[counttrackid].sampleProperties = VarLenArray<TrackSampInfo>(count);
        outTrackInfos.arrayElets[counttrackid].maxSampleSize    = 0;

        std::map<uint32_t, size_t> sampOffset;
        for (auto const& segment : CreateDashSegs(initSegId))
        {
            newTrackId = basicTrackId;
            std::vector<ContextId>::iterator ctxIdIter = idPairVec.begin();
            ctxIdIter = --idPairVec.end();
            std::map<ContextId, TrackDecInfo>::const_iterator decInfoIter;
            decInfoIter = segment.trackDecInfos.find((*ctxIdIter));
            if (decInfoIter != segment.trackDecInfos.end())
            {
                auto& trackDecInfo = decInfoIter->second;
                offset             = sampOffset[newTrackId];

                if (trackDecInfo.hasTtyp)
                {
                    auto& tAtom                                          = trackDecInfo.ttyp;
                    outTrackInfos.arrayElets[newTrackId].hasTypeInformation = true;

                    outTrackInfos.arrayElets[newTrackId].type.majorBrand   = tAtom.GetMajorBrand().c_str();
                    outTrackInfos.arrayElets[newTrackId].type.minorVersion = tAtom.GetMinorVersion();

                    std::vector<FourCC> convertedCompatibleBrands;
                    for (auto& compatibleBrand : tAtom.GetCompatibleBrands())
                    {
                        convertedCompatibleBrands.push_back(FourCC(compatibleBrand.c_str()));
                    }
                    outTrackInfos.arrayElets[newTrackId].type.compatibleBrands =
                        makeVarLenArray<FourCC>(convertedCompatibleBrands);
                }
                else
                {
                    outTrackInfos.arrayElets[newTrackId].hasTypeInformation = false;
                }

                if (trackDecInfo.samples.size() > 0)
                {
                    uint32_t delta;
                    if (trackDecInfo.samples.size() >= 3)
                    {
                        delta = trackDecInfo.samples.at(1).sampleDuration;
                    }
                    else
                    {
                        delta = trackDecInfo.samples.at(0).sampleDuration;
                    }
                    auto timeScale =
                        m_initSegProps.at(initSegId).basicTrackInfos.at((*ctxIdIter)).timeScale;
                    outTrackInfos.arrayElets[newTrackId].frameRate = RatValue{timeScale, delta};
                }

                for (auto const& sample : trackDecInfo.samples)
                {
                    outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleId =
                        ItemId(sample.sampleId).GetIndex();
                    outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleEntryType =
                        FourCC(sample.sampleEntryType.GetUInt32());
                    outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleDescriptionIndex =
                        sample.sampleDescriptionIndex.GetIndex();
                    outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleType = sample.sampleType;
                    outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].initSegmentId =
                        segment.initSegmentId.GetIndex();
                    outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].segmentId = segment.segmentId.GetIndex();
                    if (sample.compositionTimes.size())
                    {
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].earliestTStamp =
                            sample.compositionTimes.at(0);
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].earliestTStampTS =
                            sample.compositionTimesTS.at(0);
                    }
                    else
                    {
                        outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].earliestTStamp   = 0;
                            outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].earliestTStampTS = 0;
                    }
                    outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleFlags.flagsAsUInt =
                        sample.sampleFlags.flagsAsUInt;
                    outTrackInfos.arrayElets[newTrackId].sampleProperties[offset].sampleDurationTS =
                        sample.sampleDuration;

                    unsigned int sampleSize = sample.dataLength;
                    if (sampleSize > outTrackInfos.arrayElets[newTrackId].maxSampleSize)
                    {
                        outTrackInfos.arrayElets[newTrackId].maxSampleSize = sampleSize;
                    }
                    offset++;
                }
                sampOffset[newTrackId] = offset;
            }
        }
        basicTrackId++;
    }
    return ERROR_NONE;
}

int32_t Mp4Reader::GetDisplayWidth(uint32_t trackId, uint32_t& displayPicW) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    CtxType ctxType;
    int error = GetCtxTypeError(MakeIdPair(trackId), ctxType);
    if (error)
    {
        return error;
    }

    if (ctxType == CtxType::TRACK)
    {
        displayPicW = GetTrackBasicInfo(MakeIdPair(trackId)).width >> 16;
        return ERROR_NONE;
    }
    else
    {
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }
}


int32_t Mp4Reader::GetDisplayHeight(uint32_t trackId, uint32_t& displayPicH) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    CtxType ctxType;
    int error = GetCtxTypeError(MakeIdPair(trackId), ctxType);
    if (error)
    {
        return error;
    }

    if (ctxType == CtxType::TRACK)
    {
        displayPicH = GetTrackBasicInfo(MakeIdPair(trackId)).height >> 16;
        return ERROR_NONE;
    }
    else
    {
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }
}

int32_t Mp4Reader::GetDisplayWidthFP(uint32_t trackId, uint32_t& displayPicW) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    CtxType ctxType;
    int error = GetCtxTypeError(MakeIdPair(trackId), ctxType);
    if (error)
    {
        return error;
    }

    if (ctxType == CtxType::TRACK)
    {
        displayPicW = GetTrackBasicInfo(MakeIdPair(trackId)).width;
        return ERROR_NONE;
    }
    else
    {
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }
}

int32_t Mp4Reader::GetDisplayHeightFP(uint32_t trackId, uint32_t& displayPicH) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    CtxType ctxType;
    int error = GetCtxTypeError(MakeIdPair(trackId), ctxType);
    if (error)
    {
        return error;
    }

    if (ctxType == CtxType::TRACK)
    {
        displayPicH = GetTrackBasicInfo(MakeIdPair(trackId)).height;
        return ERROR_NONE;
    }
    else
    {
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }
}

int32_t Mp4Reader::GetWidth(uint32_t trackId, uint32_t itemId, uint32_t& imgW) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    uint32_t tempheight = 0;
    return GetImgDims(trackId, itemId, imgW, tempheight);
}


int32_t Mp4Reader::GetHeight(uint32_t trackId, uint32_t itemId, uint32_t& imgH) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    uint32_t tempwidth = 0;
    return GetImgDims(trackId, itemId, tempwidth, imgH);
}

int32_t Mp4Reader::GetDims(uint32_t trackId, uint32_t itemId, uint32_t& imgW, uint32_t& imgH) const {
  if (IsInitErr()) {
    return OMAF_MP4READER_NOT_INITIALIZED;
  }

  return GetImgDims(trackId, itemId, imgW, imgH);
}

int32_t Mp4Reader::GetPlaybackDurationInSecs(uint32_t trackId, double& durInSecs) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    auto trackIdPair = MakeIdPair(trackId);
    double playDur   = 0.0;
    CtxType ctxType;
    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }

    InitSegmentId initSegId = trackIdPair.first;
    ContextId initTrackId   = trackIdPair.second;

    switch (ctxType)
    {
    case CtxType::TRACK:
    {
        int64_t maxTInUS = 0;
        uint32_t timescale =
            m_initSegProps.at(initSegId).basicTrackInfos.at(initTrackId).timeScale;
        std::map<SegmentId, SegmentProperties>::const_iterator iter = m_initSegProps.at(initSegId).segPropMap.begin();
        for ( ; iter != m_initSegProps.at(initSegId).segPropMap.end(); iter++)
        {
            if (iter->second.initSegmentId == trackIdPair.first)
            {
                std::map<ContextId, TrackDecInfo>::const_iterator iter1;
                iter1 = iter->second.trackDecInfos.find(trackIdPair.second);
                if (iter1 != iter->second.trackDecInfos.end())
                {
                    if (iter1->second.samples.size())
                    {
                        for (const auto& compTS : iter1->second.samples.back().compositionTimesTS)
                        {
                            maxTInUS = max(
                                maxTInUS,
                                int64_t((compTS + iter1->second.samples.back().sampleDuration) *
                                        1000000 / timescale));
                        }
                    }
                }
            }
        }
        playDur = maxTInUS / 1000000.0;
        break;
    }

    default:
        std::map<SegmentId, SegmentProperties>::const_iterator iter = m_initSegProps.at(initSegId).segPropMap.begin();
        for ( ; iter != m_initSegProps.at(initSegId).segPropMap.end(); iter++)
        {
            double timeSlot = 0.0;
            if (iter->second.initSegmentId == trackIdPair.first)
            {
                SegmentId segIndex       = iter->first;
                SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);
                switch (ctxType)
                {
                case CtxType::META:
                    if (m_metaInfo.at(segTrackId).forceFPSSet)
                    {
                        timeSlot = m_metaInfo.at(segTrackId).dispMasterImgs /
                                      m_metaInfo.at(segTrackId).assignedFPS;
                    }
                    else
                    {
                        ISO_LOG(LOG_WARNING, "GetPlaybackDurationInSecs() called for meta context, but forced FPS was not set\n");
                    }
                    break;
                case CtxType::FILE:
                    for (const auto& contextInfo : m_ctxInfoMap)
                    {
                        double ctxDur;
                        error = GetPlaybackDurationInSecs(GenTrackId(contextInfo.first), ctxDur);
                        if (error)
                        {
                            return error;
                        }
                        if (ctxDur > timeSlot)
                        {
                            timeSlot = ctxDur;
                        }
                    }
                    break;
                default:
                    return OMAF_INVALID_MP4READER_CONTEXTID;
                }
            }
            playDur += timeSlot;
        }
    }

    durInSecs = playDur;
    return ERROR_NONE;
}

int32_t Mp4Reader::GetSampListByType(uint32_t trackId,
                                                      SampleFrameType itemType,
                                                      VarLenArray<uint32_t>& itemIds) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    IdVector allItems;
    int32_t error = GetCtxItems(trackIdPair, allItems);
    if (error)
    {
        return error;
    }

    std::vector<uint32_t> matches;
    CtxType ctxType;
    error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }

    bool foundAny = false;

    for (const auto& segment : CreateDashSegs(trackIdPair.first))
    {
        foundAny                  = true;
        SegmentTrackId segTrackId = make_pair(segment.segmentId, trackIdPair.second);
        if (ctxType == CtxType::TRACK)
        {
            if (itemType == SampleFrameType::OUT_REF_PIC)
            {
                if (CanFindTrackDecInfo(initSegId, segTrackId))
                {
                    ItemId sampId;
                    const SampleInfoVector& infoOfSamp = GetSampInfos(initSegId, segTrackId, sampId);
                    for (uint32_t index = 0; index < infoOfSamp.size(); ++index)
                    {
                        if (infoOfSamp[index].sampleType == FrameCodecType::OUTPUT_REF_FRAME)
                        {
                            matches.push_back((ItemId(index) + sampId).GetIndex());
                        }
                    }
                }
            }
            else if (itemType == SampleFrameType::NON_OUT_REFPIC)
            {
                if (CanFindTrackDecInfo(initSegId, segTrackId))
                {
                    ItemId sampId;
                    const SampleInfoVector& infoOfSamp = GetSampInfos(initSegId, segTrackId, sampId);
                    for (uint32_t index = 0; index < infoOfSamp.size(); ++index)
                    {
                        if (infoOfSamp[index].sampleType == FrameCodecType::NON_OUTPUT_REF_FRAME)
                        {
                            matches.push_back((ItemId(index) + sampId).GetIndex());
                        }
                    }
                }
            }
            else if (itemType == SampleFrameType::OUT_NONREF_PIC)
            {
                if (CanFindTrackDecInfo(initSegId, segTrackId))
                {
                    ItemId sampId;
                    const SampleInfoVector& infoOfSamp = GetSampInfos(initSegId, segTrackId, sampId);
                    for (uint32_t index = 0; index < infoOfSamp.size(); ++index)
                    {
                        if (infoOfSamp[index].sampleType == FrameCodecType::OUTPUT_NONREF_FRAME)
                        {
                            matches.push_back((ItemId(index) + sampId).GetIndex());
                        }
                    }
                }
            }
            else if (itemType == SampleFrameType::DISPLAY_PIC)
            {
                if (CanFindTrackDecInfo(initSegId, segTrackId))
                {
                    IdVector sampleIds;
                    ItemId sampId;
                    const SampleInfoVector& infoOfSamp = GetSampInfos(initSegId, segTrackId, sampId);
                    for (uint32_t index = 0; index < infoOfSamp.size(); ++index)
                    {
                        if (infoOfSamp[index].sampleType == FrameCodecType::OUTPUT_NONREF_FRAME ||
                            infoOfSamp[index].sampleType == FrameCodecType::OUTPUT_REF_FRAME)
                        {
                            sampleIds.push_back((ItemId(index) + sampId).GetIndex());
                        }
                    }

                    std::vector<ItemIdTStampPair> samplePresentationTimes;
                    for (auto sampleId : sampleIds)
                    {
                        auto& trackDecInfo = GetTrackDecInfo(initSegId, segTrackId);
                        const auto singleSampPresentationTimes =
                            trackDecInfo.samples.at((ItemId(sampleId) - trackDecInfo.itemIdBase).GetIndex()).compositionTimes;
                        for (auto sampleTime : singleSampPresentationTimes)
                        {
                            samplePresentationTimes.push_back(make_pair(sampleId, sampleTime));
                        }
                    }

                    sort(samplePresentationTimes.begin(), samplePresentationTimes.end(),
                              [&](ItemIdTStampPair a, ItemIdTStampPair b) { return a.second < b.second; });

                    for (auto pair : samplePresentationTimes)
                    {
                        matches.push_back(pair.first.GetIndex());
                    }
                }
            }
            else if (itemType == SampleFrameType::SAMPLES_PIC)
            {
                matches = allItems;
            }
            //else
            //{
            //    return OMAF_ERROR_BAD_PARAM;
            //}
        }
    }

    if (!foundAny)
    {
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }

    itemIds = makeVarLenArray<uint32_t>(matches);
    return ERROR_NONE;
}


int32_t Mp4Reader::GetSampType(uint32_t trackId, uint32_t itemId, FourCC& trackItemType) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    SegmentId segIndex;
    int32_t result = GetSegIndex(trackIdPair, itemId, segIndex);
    if (result != ERROR_NONE)
    {
        return result;
    }
    SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);
    CtxType ctxType;
    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }

    FourCCInt boxtype;
    switch (ctxType)
    {
    case CtxType::TRACK:
    {
        ItemId sampId;
        auto& infoOfSamp = GetSampInfos(initSegId, segTrackId, sampId);
        boxtype          = infoOfSamp.at((ItemId(itemId) - sampId).GetIndex()).sampleEntryType;
        trackItemType    = FourCC(boxtype.GetUInt32());
        return ERROR_NONE;
    }

    default:
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }
}

int32_t Mp4Reader::GetSegIndex(InitSegmentTrackId trackIdPair,
                                         ItemId itemId,
                                         SegmentId& segIndex) const
{
    bool wentPast = false;

    if (m_initSegProps.empty() || !m_initSegProps.count(trackIdPair.first) ||
        m_initSegProps.at(trackIdPair.first).segPropMap.empty())
    {
        return OMAF_INVALID_SEGMENT;
    }

    const auto& segs = CreateDashSegs(trackIdPair.first);
    for (auto segmentIt = segs.begin(); !wentPast && segmentIt != segs.end(); ++segmentIt)
    {
        if (segmentIt->initSegmentId == trackIdPair.first)
        {
            auto track = segmentIt->trackDecInfos.find(trackIdPair.second);
            if (track != segmentIt->trackDecInfos.end())
            {
                if (track->second.itemIdBase > ItemId(itemId))
                {
                    wentPast = true;
                }
                else
                {
                    segIndex = segmentIt->segmentId;
                }
            }
        }
    }

    int ret      = OMAF_INVALID_ITEM_ID;
    auto segment = m_initSegProps.at(trackIdPair.first).segPropMap.find(segIndex);
    if (segment != m_initSegProps.at(trackIdPair.first).segPropMap.end())
    {
        auto track = segment->second.trackDecInfos.find(trackIdPair.second);
        if (track != segment->second.trackDecInfos.end())
        {
            if (itemId - track->second.itemIdBase < ItemId(uint32_t(track->second.samples.size())))
            {
                ret = ERROR_NONE;
            }
        }
    }
    return ret;
}

int32_t Mp4Reader::GetSegIndex(InitSegTrackIdPair id, SegmentId& segIndex) const
{
    return GetSegIndex(id.first, id.second, segIndex);
}

int32_t Mp4Reader::GetSampDataInfo(uint32_t ctxId,
                                               uint32_t itemIndex,
                                               const InitSegmentId& initSegId,
                                               uint64_t& refSampLength,
                                               uint64_t& refDataOffset)
{
    auto trackCtxId = ContextId(ctxId);
    if (!(m_initSegProps[initSegId].trackProperties[trackCtxId].referenceTrackIds["scal"].empty()))
    {
        return OMAF_INVALID_PROPERTY_INDEX;
    }

    InitSegmentTrackId neededInitSegTrackId = make_pair(initSegId, trackCtxId);

    SegmentId segIndex;
    int32_t result = GetSegIndex(neededInitSegTrackId, itemIndex, segIndex);
    if (result != ERROR_NONE)
    {
        return result;
    }
    SegmentTrackId segTrackId = make_pair(segIndex, neededInitSegTrackId.second);
    ItemId itemId             = ItemId(itemIndex) - GetTrackDecInfo(initSegId, segTrackId).itemIdBase;

    refDataOffset   = GetTrackDecInfo(initSegId, segTrackId).samples.at(itemId.GetIndex()).dataOffset;
    refSampLength = GetTrackDecInfo(initSegId, segTrackId).samples.at(itemId.GetIndex()).dataLength;

    return ERROR_NONE;
}

int32_t Mp4Reader::GetDepedentSampInfo(uint32_t trackId,
                                                  uint32_t itemIndex,
                                                  const InitSegmentId& initSegId,
                                                  uint8_t trackReference,
                                                  uint64_t& refSampLength,
                                                  uint64_t& refDataOffset)
{
    auto trackCtxId = MakeIdPair(trackId).second;
    if (m_initSegProps[initSegId].trackProperties[trackCtxId].referenceTrackIds["scal"].empty())
    {
        return OMAF_INVALID_PROPERTY_INDEX;
    }
    auto refTrackCtxId =
        m_initSegProps[initSegId].trackProperties[trackCtxId].referenceTrackIds["scal"].at(
            trackReference);

    InitSegmentTrackId refInitSegTrackId = make_pair(initSegId, refTrackCtxId);

    SegmentId refSegmentId;
    int32_t result = GetSegIndex(refInitSegTrackId, itemIndex, refSegmentId);
    if (result != ERROR_NONE)
    {
        return result;
    }
    SegmentTrackId refSegTrackId = make_pair(refSegmentId, refInitSegTrackId.second);
    ItemId refItemId             = ItemId(itemIndex) - GetTrackDecInfo(initSegId, refSegTrackId).itemIdBase;

    refDataOffset   = GetTrackDecInfo(initSegId, refSegTrackId).samples.at(refItemId.GetIndex()).dataOffset;
    refSampLength = GetTrackDecInfo(initSegId, refSegTrackId).samples.at(refItemId.GetIndex()).dataLength;

    return ERROR_NONE;
}

bool ParseExtractorNal(const DataVector& NalData,
                       ExtSample& extSamp,
                       uint8_t lenSizeMinus1,
                       uint64_t& extSize)
{
    Stream nalus(NalData);
    ExtNalHdr extNalHdr;
    uint32_t extractors = 0;
    uint64_t inlineSizes = 0;
    size_t order_idx = 1;

    while (nalus.BytesRemain() > 0)
    {
        size_t readCnt = 0;
        if (lenSizeMinus1 == 0)
        {
            readCnt = nalus.Read8();
        }
        else if (lenSizeMinus1 == 1)
        {
            readCnt = nalus.Read16();
        }
        else if (lenSizeMinus1 == 3)
        {
            readCnt = nalus.Read32();
        }
        else
        {
            ISO_LOG(LOG_ERROR, "Length field size is not correct !\n");
            throw exception();
        }

        extNalHdr.forbidden_zero_bit    = (uint8_t) nalus.Read1(1);
        extNalHdr.nal_unit_type         = (uint8_t) nalus.Read1(6);
        extNalHdr.nuh_layer_id          = (uint8_t) nalus.Read1(6);
        extNalHdr.nuh_temporal_id_plus1 = (uint8_t) nalus.Read1(3);

        readCnt = readCnt - 2;
        if (extNalHdr.nal_unit_type == 49)
        {
            ExtSample::Extractor extractor;
            for (; readCnt > 0; readCnt--)
            {
                uint8_t constType = (uint8_t) nalus.Read1(8);

                if (constType == 0)
                {
                    ExtSample::SampleConstruct sampConst;
                    sampConst.order_idx        = (uint8_t) order_idx;
                    sampConst.constructor_type = (uint8_t) constType;
                    sampConst.track_ref_index  = (uint8_t) nalus.Read1(8);
                    readCnt--;
                    sampConst.track_ref_index = sampConst.track_ref_index - 1;
                    sampConst.sample_offset   = (int8_t) nalus.Read1(8);
                    readCnt--;
                    sampConst.data_offset = nalus.Read1((lenSizeMinus1 + 1) * 8);
                    readCnt -= (lenSizeMinus1 + 1);
                    sampConst.data_length = nalus.Read1((lenSizeMinus1 + 1) * 8);
                    readCnt -= (lenSizeMinus1 + 1);
                    if (sampConst.data_length < UINT32_MAX)
                    {
                        extSize += sampConst.data_length;
                    }
                    extractor.sampleConstruct.push_back(sampConst);
                    order_idx = order_idx + 1;
                }
                else if (constType == 2)
                {
                    ExtSample::InlineConstruct inlinConst;
                    inlinConst.order_idx        = (uint8_t) order_idx;
                    inlinConst.constructor_type = (uint8_t) constType;
                    inlinConst.data_length      = (uint8_t) nalus.Read1(8);
                    for (uint8_t i = 0; i < inlinConst.data_length; i++)
                    {
                        inlinConst.inline_data.push_back((uint8_t) nalus.Read1(8));
                    }
                    inlineSizes += inlinConst.data_length;
                    extractor.inlineConstruct.push_back(inlinConst);
                    readCnt   = readCnt - inlinConst.data_length - 1;
                    order_idx = order_idx + 1;
                }
            }
            extSamp.extractors.push_back(extractor);
            extractors++;
        }
        else
        {
            nalus.SkipBytes(readCnt);
        }
    }
    if (extractors)
    {
        if (extSize > 0)
        {
            extSize += inlineSizes;
        }
        return true;
    }
    else
    {
        return false;
    }
}

int32_t Mp4Reader::GetExtractorTrackSampData(uint32_t trackId,
                                                         uint32_t itemIndex,
                                                         char* buf,
                                                         uint32_t& bufSize,
                                                         bool strHrd)
{
    uint32_t spaceAvailable = bufSize;
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    SegmentId segIndex;
    int32_t result = GetSegIndex(trackIdPair, itemIndex, segIndex);
    if (result != ERROR_NONE)
    {
        return result;
    }
    SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);
    ItemId itemId             = ItemId(itemIndex) - GetTrackDecInfo(initSegId, segTrackId).itemIdBase;

    SegmentIO& io = m_initSegProps.at(initSegId).segPropMap.at(segIndex).io;
    CtxType ctxType;
    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }
    switch (ctxType)
    {
    case CtxType::TRACK:
    {
        if (itemId.GetIndex() >= GetTrackDecInfo(initSegId, segTrackId).samples.size())
        {
            return OMAF_INVALID_ITEM_ID;
        }

        const uint32_t sampLen = GetTrackDecInfo(initSegId, segTrackId).samples.at(itemId.GetIndex()).dataLength;
        if (bufSize < sampLen)
        {
            bufSize = sampLen;
            return OMAF_MEMORY_TOO_SMALL_BUFFER;
        }

        int64_t neededDataOffset = (int64_t) GetTrackDecInfo(initSegId, segTrackId).samples.at(itemId.GetIndex()).dataOffset;
        LocateToOffset(io, neededDataOffset);
        io.strIO->ReadStream(buf, sampLen);
        bufSize = sampLen;

        if (!io.strIO->IsStreamGood())
        {
            return OMAF_FILE_READ_ERROR;
        }
        break;
    }
    default:
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }

    FourCC codeType;
    error = GetDecoderCodeType(GenTrackId(trackIdPair), itemIndex, codeType);
    if (error)
    {
        return error;
    }

    if (codeType == "avc1" || codeType == "avc3")
    {
        if (strHrd)
        {
            return ParseAvcData(buf, bufSize);
        }
        else
        {
            return ERROR_NONE;
        }
    }
    else if (codeType == "hvc1" || codeType == "hev1")
    {
        if (strHrd)
        {
            return ParseHevcData(buf, bufSize);
        }
        else
        {
            return ERROR_NONE;
        }
    }

    else if (codeType == "hvc2")
    {
        std::vector<uint8_t> extSampBuffer(buf, buf + bufSize);

        uint8_t nalLengthSizeMinus1 = 3;
        ItemId sampId;
        auto& infoOfSamp             = GetSampInfos(initSegId, segTrackId, sampId);
        SmpDesIndex index = infoOfSamp.at((ItemId(itemIndex) - sampId).GetIndex()).sampleDescriptionIndex;
        if (GetTrackBasicInfo(trackIdPair).nalLengthSizeMinus1.count(index.GetIndex()) != 0)
        {
            nalLengthSizeMinus1 = GetTrackBasicInfo(trackIdPair).nalLengthSizeMinus1.at(index);
            assert(nalLengthSizeMinus1 == 3);
        }

        ExtSample extSamp;
        uint64_t extSize = 0;
        uint64_t tolerance      = 0;

        if (ParseExtractorNal(extSampBuffer, extSamp, nalLengthSizeMinus1, extSize))
        {
            if (extSize == 0)
            {
                extSize = 0;
                for (auto& extractor : extSamp.extractors)
                {
                    for (auto& sampConstr : extractor.sampleConstruct)
                    {
                        uint64_t refSampLength = 0;
                        uint64_t refDataOffset = 0;
                        result =
                            GetDepedentSampInfo(trackId, itemIndex, initSegId, sampConstr.track_ref_index,
                                refSampLength, refDataOffset);
                        if (result != ERROR_NONE)
                        {
                            return result;
                        }
                        extSize += refSampLength;
                    }
                }
                tolerance = (uint64_t)(extSize / 10);
                extSize += tolerance;
            }
            if (extSize > (uint64_t) spaceAvailable)
            {
                bufSize = (uint32_t)(extSize + tolerance);
                return OMAF_MEMORY_TOO_SMALL_BUFFER;
            }

            uint32_t extractedBytes            = 0;
            char* buffer                       = buf;
            char* inlineNalLengthPlaceHolder = NULL;
            size_t inlineLength                = 0;
            vector<ExtSample::SampleConstruct>::iterator sampConst;
            vector<ExtSample::InlineConstruct>::iterator inlinConst;
            uint64_t refSampLength = 0;
            uint64_t refSampOffset = 0;
            uint8_t trackRefIndex = UINT8_MAX;

            for (auto& extractor : extSamp.extractors)
            {
                for (sampConst = extractor.sampleConstruct.begin(),
                    inlinConst = extractor.inlineConstruct.begin();
                    sampConst != extractor.sampleConstruct.end() ||
                    inlinConst != extractor.inlineConstruct.end();)
                {
                    if (inlinConst != extractor.inlineConstruct.end() &&
                        (sampConst == extractor.sampleConstruct.end() ||
                        (*inlinConst).order_idx < (*sampConst).order_idx))
                    {
                        inlineNalLengthPlaceHolder = buffer;

                        memcpy(buffer, (*inlinConst).inline_data.data(), (*inlinConst).inline_data.size());
                        inlineLength = (*inlinConst).inline_data.size() - (nalLengthSizeMinus1 + 1);   // exclude the length
                        buffer += (*inlinConst).data_length;
                        extractedBytes += (*inlinConst).data_length;
                        ++inlinConst;
                    }
                    else if (sampConst != extractor.sampleConstruct.end())
                    {
                        auto referredTrack = ContextId((*sampConst).track_ref_index + 1);
                        InitSegmentId ref_initSegmentId;
                        for (const auto& loopInitSegment : m_initSegProps)
                        {
                            if (loopInitSegment.second.corresTrackId == referredTrack)
                            {
                                ref_initSegmentId = loopInitSegment.first;
                                break;
                            }
                        }
                        InitSegmentTrackId ref_trackIdPair = make_pair(ref_initSegmentId, referredTrack);
                        SegmentId ref_segmentId;
                        int32_t result = GetSegIndex(ref_trackIdPair, itemIndex, ref_segmentId);
                        SegmentIO& ref_io = m_initSegProps.at(ref_initSegmentId).segPropMap.at(ref_segmentId).io;

                        if ((*sampConst).track_ref_index != trackRefIndex || trackRefIndex == UINT8_MAX)
                        {
                            result =
                                GetSampDataInfo(((*sampConst).track_ref_index + 1), itemIndex, ref_initSegmentId,
                                    refSampLength, refSampOffset);
                            if (result != ERROR_NONE)
                            {
                                return result;
                            }
                            trackRefIndex = (*sampConst).track_ref_index;
                            LocateToOffset(ref_io, refSampOffset);
                        }
                        ref_io.strIO->ReadStream(buffer, (nalLengthSizeMinus1 + 1));
                        uint64_t refNalLength = ParseNalLen(buffer);

                        uint64_t inputReadOffset = refSampOffset + (*sampConst).data_offset;

                        uint64_t bytesToCopy = refNalLength;
                        if ((*sampConst).data_length == 0)
                        {
                            bytesToCopy = refNalLength;
                            refSampLength = 0;
                        }
                        else
                        {
                            if ((uint64_t)((*sampConst).data_offset) + (uint64_t)((*sampConst).data_length) > refSampLength)
                            {
                                if ((*sampConst).data_offset > refSampLength)
                                {
                                    return OMAF_INVALID_SEGMENT;
                                }
                                bytesToCopy = refSampLength - (*sampConst).data_offset;
                            }
                            else
                            {
                                bytesToCopy = (*sampConst).data_length;
                            }

                            if (inlineNalLengthPlaceHolder != NULL)
                            {
                                uint64_t actualNalLength = bytesToCopy + inlineLength;
                                WriteNalLen(actualNalLength, inlineNalLengthPlaceHolder);
                                inlineNalLengthPlaceHolder = NULL;
                            }
                            else
                            {
                                inputReadOffset += (nalLengthSizeMinus1 + 1);
                                if (bytesToCopy == refSampLength - (*sampConst).data_offset)
                                {
                                    bytesToCopy -= (nalLengthSizeMinus1 + 1);
                                }
                                buffer += (nalLengthSizeMinus1 + 1);
                                extractedBytes += (nalLengthSizeMinus1 + 1);
                            }
                        }

                        if (extractedBytes + (uint32_t)bytesToCopy > spaceAvailable)
                        {
                            bufSize = extractedBytes + (uint32_t)bytesToCopy;
                            return OMAF_MEMORY_TOO_SMALL_BUFFER;
                        }
                        if (inputReadOffset > 0)
                        {
                            LocateToOffset(ref_io, (int64_t) inputReadOffset);
                        }
                        ref_io.strIO->ReadStream(buffer, bytesToCopy);
                        buffer += bytesToCopy;
                        extractedBytes += (uint32_t)bytesToCopy;
                        ++sampConst;
                        inlineNalLengthPlaceHolder = NULL;
                        inlineLength = 0;

                        refSampLength -= (refNalLength + (nalLengthSizeMinus1 + 1));
                    }
                }
            }
            bufSize = extractedBytes;
            if (strHrd)
            {
                return ParseHevcData(buf, bufSize);
            }

            return ERROR_NONE;
        }
        return OMAF_UNSUPPORTED_DASH_CODECS_TYPE;  // hvc2 but unknown extractor?
    }
    else if ((codeType == "mp4a") || (codeType == "invo") || (codeType == "urim") || (codeType == "mp4v"))
    {
        return ERROR_NONE;
    }
    else
    {
        return OMAF_UNSUPPORTED_DASH_CODECS_TYPE;
    }
}

int32_t Mp4Reader::GetSampData(uint32_t trackId,
                                                uint32_t itemIndex,
                                                char* buf,
                                                uint32_t& bufSize,
                                                bool strHrd)
{
    uint32_t spaceAvailable = bufSize;
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    SegmentId segIndex;
    int32_t result = GetSegIndex(trackIdPair, itemIndex, segIndex);
    if (result != ERROR_NONE)
    {
        return result;
    }
    SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);
    ItemId itemId             = ItemId(itemIndex) - GetTrackDecInfo(initSegId, segTrackId).itemIdBase;

    SegmentIO& io = m_initSegProps.at(initSegId).segPropMap.at(segIndex).io;
    CtxType ctxType;
    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }
    switch (ctxType)
    {
    case CtxType::TRACK:
    {
        if (itemId.GetIndex() >= GetTrackDecInfo(initSegId, segTrackId).samples.size())
        {
            return OMAF_INVALID_ITEM_ID;
        }

        const uint32_t sampLen = GetTrackDecInfo(initSegId, segTrackId).samples.at(itemId.GetIndex()).dataLength;
        if (bufSize < sampLen)
        {
            bufSize = sampLen;
            return OMAF_MEMORY_TOO_SMALL_BUFFER;
        }

        LocateToOffset(io, (int64_t) GetTrackDecInfo(initSegId, segTrackId).samples.at(itemId.GetIndex()).dataOffset);
        io.strIO->ReadStream(buf, sampLen);
        bufSize = sampLen;

        if (!io.strIO->IsStreamGood())
        {
            return OMAF_FILE_READ_ERROR;
        }
        break;
    }
    default:
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }

    FourCC codeType;
    error = GetDecoderCodeType(GenTrackId(trackIdPair), itemIndex, codeType);
    if (error)
    {
        return error;
    }

    if (codeType == "avc1" || codeType == "avc3")
    {
        if (strHrd)
        {
            return ParseAvcData(buf, bufSize);
        }
        else
        {
            return ERROR_NONE;
        }
    }
    else if (codeType == "hvc1" || codeType == "hev1")
    {
        if (strHrd)
        {
            return ParseHevcData(buf, bufSize);
        }
        else
        {
            return ERROR_NONE;
        }
    }
    else if (codeType == "hvc2")
    {
        std::vector<uint8_t> extSampBuffer(buf, buf + bufSize);

        uint8_t nalLengthSizeMinus1 = 3;
        ItemId sampId;
        auto& infoOfSamp             = GetSampInfos(initSegId, segTrackId, sampId);
        SmpDesIndex index = infoOfSamp.at((ItemId(itemIndex) - sampId).GetIndex()).sampleDescriptionIndex;
        if (GetTrackBasicInfo(trackIdPair).nalLengthSizeMinus1.count(index.GetIndex()) != 0)
        {
            nalLengthSizeMinus1 = GetTrackBasicInfo(trackIdPair).nalLengthSizeMinus1.at(index);
            assert(nalLengthSizeMinus1 == 3);
        }

        ExtSample extSamp;
        uint64_t extSize = 0;
        uint64_t tolerance      = 0;

        if (ParseExtractorNal(extSampBuffer, extSamp, nalLengthSizeMinus1, extSize))
        {
            if (extSize == 0)
            {
                extSize = 0;
                for (auto& extractor : extSamp.extractors)
                {
                    for (auto& sampConstr : extractor.sampleConstruct)
                    {
                        uint64_t refSampLength = 0;
                        uint64_t refDataOffset = 0;
                        result =
                            GetDepedentSampInfo(trackId, itemIndex, initSegId, sampConstr.track_ref_index,
                                refSampLength, refDataOffset);
                        if (result != ERROR_NONE)
                        {
                            return result;
                        }
                        extSize += refSampLength;
                    }
                }
                tolerance = (uint64_t)(extSize / 10);
                extSize += tolerance;
            }
            if (extSize > (uint64_t) spaceAvailable)
            {
                bufSize = (uint32_t)(extSize + tolerance);
                return OMAF_MEMORY_TOO_SMALL_BUFFER;
            }

            uint32_t extractedBytes            = 0;
            char* buffer                       = buf;
            char* inlineNalLengthPlaceHolder = NULL;
            size_t inlineLength                = 0;
            vector<ExtSample::SampleConstruct>::iterator sampConst;
            vector<ExtSample::InlineConstruct>::iterator inlinConst;
            uint64_t refSampLength = 0;
            uint64_t refSampOffset = 0;
            uint8_t trackRefIndex = UINT8_MAX;

            for (auto& extractor : extSamp.extractors)
            {
                for (sampConst = extractor.sampleConstruct.begin(),
                    inlinConst = extractor.inlineConstruct.begin();
                    sampConst != extractor.sampleConstruct.end() ||
                    inlinConst != extractor.inlineConstruct.end();)
                {
                    if (inlinConst != extractor.inlineConstruct.end() &&
                        (sampConst == extractor.sampleConstruct.end() ||
                        (*inlinConst).order_idx < (*sampConst).order_idx))
                    {
                        inlineNalLengthPlaceHolder = buffer;
                        memcpy(buffer, (*inlinConst).inline_data.data(), (*inlinConst).inline_data.size());
                        inlineLength = (*inlinConst).inline_data.size() - (nalLengthSizeMinus1 + 1);
                        buffer += (*inlinConst).data_length;
                        extractedBytes += (*inlinConst).data_length;
                        ++inlinConst;
                    }
                    else if (sampConst != extractor.sampleConstruct.end())
                    {
                        if ((*sampConst).track_ref_index != trackRefIndex || trackRefIndex == UINT8_MAX)
                        {
                            result =
                                GetDepedentSampInfo(trackId, itemIndex, initSegId, (*sampConst).track_ref_index,
                                    refSampLength, refSampOffset);
                            if (result != ERROR_NONE)
                            {
                                return result;
                            }
                            trackRefIndex = (*sampConst).track_ref_index;
                            LocateToOffset(io, refSampOffset);
                        }
                        io.strIO->ReadStream(buffer, (nalLengthSizeMinus1 + 1));
                        uint64_t refNalLength = ParseNalLen(buffer);

                        uint64_t inputReadOffset = refSampOffset + (*sampConst).data_offset;

                        uint64_t bytesToCopy = refNalLength;
                        if ((*sampConst).data_length == 0)
                        {
                            bytesToCopy = refNalLength;
                            refSampLength = 0;
                        }
                        else
                        {
                            if ((uint64_t)((*sampConst).data_offset) + (uint64_t)((*sampConst).data_length) > refSampLength)
                            {
                                if ((*sampConst).data_offset > refSampLength)
                                {
                                    return OMAF_INVALID_SEGMENT;
                                }
                                bytesToCopy = refSampLength - (*sampConst).data_offset;
                            }
                            else
                            {
                                bytesToCopy = (*sampConst).data_length;
                            }

                            if (inlineNalLengthPlaceHolder != NULL)
                            {
                                uint64_t actualNalLength = bytesToCopy + inlineLength;
                                WriteNalLen(actualNalLength, inlineNalLengthPlaceHolder);
                                inlineNalLengthPlaceHolder = NULL;
                            }
                            else
                            {
                                inputReadOffset += (nalLengthSizeMinus1 + 1);
                                if (bytesToCopy == refSampLength - (*sampConst).data_offset)
                                {
                                    bytesToCopy -= (nalLengthSizeMinus1 + 1);
                                }
                                buffer += (nalLengthSizeMinus1 + 1);
                                extractedBytes += (nalLengthSizeMinus1 + 1);
                            }
                        }

                        if (extractedBytes + (uint32_t)bytesToCopy > spaceAvailable)
                        {
                            bufSize = extractedBytes + (uint32_t)bytesToCopy;
                            return OMAF_MEMORY_TOO_SMALL_BUFFER;
                        }
                        if (inputReadOffset > 0)
                        {
                            LocateToOffset(io, (int64_t) inputReadOffset);
                        }
                        io.strIO->ReadStream(buffer, bytesToCopy);
                        buffer += bytesToCopy;
                        extractedBytes += (uint32_t)bytesToCopy;
                        ++sampConst;
                        inlineNalLengthPlaceHolder = NULL;
                        inlineLength = 0;

                        refSampLength -= (refNalLength + (nalLengthSizeMinus1 + 1));
                    }
                }
            }
            bufSize = extractedBytes;
            if (strHrd)
            {
                return ParseHevcData(buf, bufSize);
            }

            return ERROR_NONE;
        }
        return OMAF_UNSUPPORTED_DASH_CODECS_TYPE;  // hvc2 but unknown extractor?
    }
    else if ((codeType == "mp4a") || (codeType == "invo") || (codeType == "urim") || (codeType == "mp4v"))
    {
        return ERROR_NONE;
    }
    else
    {
        return OMAF_UNSUPPORTED_DASH_CODECS_TYPE;
    }
}

int32_t Mp4Reader::GetSampOffset(uint32_t trackId,
                                                  uint32_t itemIndex,
                                                  uint64_t& sampOffset,
                                                  uint32_t& sampLen)
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    SegmentId segIndex;
    int32_t result = GetSegIndex(trackIdPair, itemIndex, segIndex);
    if (result != ERROR_NONE)
    {
        return result;
    }
    SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);
    ItemId itemId             = ItemId(itemIndex) - GetTrackDecInfo(initSegId, segTrackId).itemIdBase;

    CtxType ctxType;
    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }
    if (ctxType == CtxType::TRACK)
    {
        if (itemId.GetIndex() >= GetTrackDecInfo(initSegId, segTrackId).samples.size())
        {
            return OMAF_INVALID_ITEM_ID;
        }
        sampLen = GetTrackDecInfo(initSegId, segTrackId).samples.at(itemId.GetIndex()).dataLength;
        sampOffset = GetTrackDecInfo(initSegId, segTrackId).samples.at(itemId.GetIndex()).dataOffset;
    }
    else
    {
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }
    return ERROR_NONE;
}


int32_t Mp4Reader::GetCodecSpecInfo(uint32_t trackId,
                                                     uint32_t itemId,
                                                     VarLenArray<MediaCodecSpecInfo>& codecSpecInfos) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);

    if (auto* parameterSetMap = GetParameterSetMap(InitSegTrackIdPair(trackIdPair, itemId)))
    {
        codecSpecInfos = VarLenArray<MediaCodecSpecInfo>(parameterSetMap->size());

        int i = 0;
        for (auto const& entry : *parameterSetMap)
        {
            MediaCodecSpecInfo decSpecInfo;
            decSpecInfo.codecSpecInfoType = entry.first;
            decSpecInfo.codecSpecInfoBits = makeVarLenArray<unsigned char>(entry.second);
            codecSpecInfos.arrayElets[i++]  = decSpecInfo;
        }
        return ERROR_NONE;
    }
    return OMAF_INVALID_ITEM_ID;  // or invalid context...?
}


int32_t Mp4Reader::GetTrackTStamps(uint32_t trackId, VarLenArray<TStampID>& timeStamps) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    std::map<TStamp, ItemId> timestampMap;

    CtxType ctxType;
    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }

    for (const auto& segment : m_initSegProps.at(initSegId).segPropMap)
    {
        if (segment.second.initSegmentId == trackIdPair.first)
        {
            SegmentId segIndex       = segment.first;
            SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);
            switch (ctxType)
            {
            case CtxType::TRACK:
            {
                if (CanFindTrackDecInfo(initSegId, segTrackId))
                {
                    for (const auto& infoOfSamp : GetTrackDecInfo(initSegId, segTrackId).samples)
                    {
                        for (auto compositionTime : infoOfSamp.compositionTimes)
                        {
                            timestampMap.insert(make_pair(compositionTime, infoOfSamp.sampleId));
                        }
                    }
                }
                break;
            }

            case CtxType::META:
            {
                if (m_metaInfo.at(segTrackId).forceFPSSet == true)
                {
                    for (const auto& imageInfo : m_metaInfo.at(segTrackId).imageInfoMap)
                    {
                        if (imageInfo.second.type == "master")
                        {
                            timestampMap.insert(make_pair(static_cast<TStamp>(imageInfo.second.displayTime),
                                                               imageInfo.first));
                        }
                    }
                }
                else
                {
                    return OMAF_INVALID_MP4READER_CONTEXTID;
                }
                break;
            }

            default:
                return OMAF_INVALID_MP4READER_CONTEXTID;
            }
        }
    }

    timeStamps = VarLenArray<TStampID>(timestampMap.size());
    uint32_t i = 0;
    for (auto const& entry : timestampMap)
    {
        TStampID pair;
        pair.timeStamp           = entry.first;
        pair.itemId              = entry.second.GetIndex();
        timeStamps.arrayElets[i++] = pair;
    }

    return ERROR_NONE;
}


int32_t Mp4Reader::GetSampTStamps(
    uint32_t trackId,
    uint32_t itemIndex,
    VarLenArray<uint64_t>& timeStamps) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    SegmentId segIndex;
    int32_t result = GetSegIndex(trackIdPair, itemIndex, segIndex);
    if (result != ERROR_NONE)
    {
        return result;
    }
    ItemId itemId = ItemId(itemIndex) -
                    GetTrackDecInfo(initSegId, SegmentTrackId(segIndex, trackIdPair.second)).itemIdBase;
    SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);

    CtxType ctxType;
    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }

    switch (ctxType)
    {
    case CtxType::TRACK:
    {
        const auto& displayTimes =
            GetTrackDecInfo(initSegId, segTrackId).samples.at(itemId.GetIndex()).compositionTimes;
        timeStamps = makeVarLenArray<uint64_t>(displayTimes);
        break;
    }

    case CtxType::META:
    {
        if (m_metaInfo.at(segTrackId).forceFPSSet == true)
        {
            timeStamps = VarLenArray<uint64_t>(1);
            timeStamps.arrayElets[0] =
                static_cast<TStamp>(m_metaInfo.at(segTrackId).imageInfoMap.at(itemId.GetIndex()).displayTime);
        }
        else
        {
            return OMAF_INVALID_MP4READER_CONTEXTID;
        }
        break;
    }

    default:
    {
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }
    }
    return ERROR_NONE;
}


int32_t Mp4Reader::GetSampInDecSeq(
    uint32_t trackId,
    VarLenArray<TStampID>& sampItems) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    DecodingOrderVector sampItemsVector;
    sampItemsVector.clear();

    CtxType ctxType;
    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }

    for (const auto& segment : m_initSegProps.at(initSegId).segPropMap)
    {
        if (segment.second.initSegmentId == trackIdPair.first)
        {
            SegmentId segIndex       = segment.first;
            SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);
            switch (ctxType)
            {
            case CtxType::TRACK:
            {
                if (CanFindTrackDecInfo(initSegId, segTrackId))
                {
                    const auto& samples = GetTrackDecInfo(initSegId, segTrackId).samples;
                    sampItemsVector.reserve(samples.size());
                    for (const auto& sample : samples)
                    {
                        for (const auto compositionTime : sample.compositionTimes)
                        {
                            sampItemsVector.push_back(make_pair(sample.sampleId, compositionTime));
                        }
                    }

                    sort(sampItemsVector.begin(), sampItemsVector.end(),
                              [&](ItemIdTStampPair a, ItemIdTStampPair b) { return a.second < b.second; });
                }
                break;
            }
            case CtxType::META:
            {
//#ifndef DISABLE_UNCOVERED_CODE
                if (m_metaInfo.at(segTrackId).forceFPSSet == true)
                {
                    sampItemsVector.reserve(m_metaInfo.at(segTrackId).imageInfoMap.size());
                    for (const auto& image : m_metaInfo.at(segTrackId).imageInfoMap)
                    {
                        sampItemsVector.push_back(pair<uint32_t, TStamp>(
                            image.first.GetIndex(), static_cast<TStamp>(image.second.displayTime)));
                    }
                }
                else
                {
                    return OMAF_INVALID_MP4READER_CONTEXTID;
                }
                break;
//#endif  // DISABLE_UNCOVERED_CODE
            }
            default:
            {
                return OMAF_INVALID_MP4READER_CONTEXTID;
            }
            }
        }
    }

    sampItems = VarLenArray<TStampID>(sampItemsVector.size());
    uint32_t i        = 0;
    for (auto const& entry : sampItemsVector)
    {
        TStampID pair;
        pair.timeStamp                  = entry.second;
        pair.itemId                     = entry.first.GetIndex();
        sampItems.arrayElets[i++] = pair;
    }
    return ERROR_NONE;
}

int32_t Mp4Reader::GetDecoderCodeType(uint32_t trackId, uint32_t itemId, FourCC& decoderCodeType) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    SegmentId segIndex;
    int32_t result = GetSegIndex(trackIdPair, itemId, segIndex);
    if (result != ERROR_NONE)
    {
        return result;
    }
    SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);
    const auto segPropsIt =
        m_initSegProps.at(initSegId).segPropMap.find(segIndex);
    if (segPropsIt == m_initSegProps.at(initSegId).segPropMap.end())
    {
        return OMAF_INVALID_ITEM_ID;
    }
    const auto& segProps = segPropsIt->second;
    const auto decInfoIter        = segProps.trackDecInfos.find(segTrackId.second);
    if (decInfoIter == segProps.trackDecInfos.end())
    {
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }
    const auto& trackDecInfo          = decInfoIter->second;
    const auto& decCodeType = trackDecInfo.decoderCodeTypeMap;

    auto iter = decCodeType.find(ItemId(itemId));
    if (iter != decCodeType.end())
    {
        decoderCodeType = iter->second.GetUInt32();
        return ERROR_NONE;
    }

    const auto parameterSetIdIt = segProps.itemToParameterSetMap.find(
        InitSegTrackIdPair(trackIdPair, ItemId(itemId) - GetTrackDecInfo(initSegId, segTrackId).itemIdBase));
    if (parameterSetIdIt == segProps.itemToParameterSetMap.end())
    {
        return OMAF_INVALID_ITEM_ID;
    }
    const SmpDesIndex parameterSetId = parameterSetIdIt->second;
    iter = decCodeType.find(ItemId(parameterSetId.GetIndex()));
    if (iter != decCodeType.end())
    {
        decoderCodeType = iter->second.GetUInt32();
        return ERROR_NONE;
    }
    return OMAF_INVALID_ITEM_ID;  // or invalid context...?
}


int32_t Mp4Reader::GetDurOfSamp(uint32_t trackId, uint32_t sampleId, uint32_t& sampDur) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    SegmentId segIndex;
    int32_t result = GetSegIndex(trackIdPair, sampleId, segIndex);
    if (result != ERROR_NONE)
    {
        return result;
    }
    SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);

    CtxType ctxType;
    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }

    switch (ctxType)
    {
    case CtxType::TRACK:
    {
        break;
    }
    case CtxType::META:
    default:
    {
        return OMAF_INVALID_MP4READER_CONTEXTID;
    }
    }

    ItemId itemId = ItemId(sampleId) - GetTrackDecInfo(initSegId, segTrackId).itemIdBase;

    sampDur =
        (uint32_t)(uint64_t(GetTrackDecInfo(initSegId, segTrackId).samples.at(itemId.GetIndex()).sampleDuration) *
                   1000) /
        GetTrackBasicInfo(trackIdPair).timeScale;
    return ERROR_NONE;
}

int32_t Mp4Reader::GetAudioChnlProp(uint32_t trackId, uint32_t sampleId, ChnlProperty& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.chnlProperties, index, result);

    if (result == ERROR_NONE)
    {
        outProp.strStruct   = propIn.strStruct;
        outProp.layOut      = propIn.layOut;
        outProp.omitChnlMap = propIn.omitChnlMap;
        outProp.objCnt      = propIn.objCnt;
        outProp.chnlCnt     = propIn.chnlCnt;
        outProp.chnlLayOuts = makeVarLenArray<ChannelLayout>(propIn.chnlLayOuts);
    }

    return result;
}

int32_t Mp4Reader::GetSpatAudioProp(uint32_t trackId,
                                                     uint32_t sampleId,
                                                     SpatialAudioProperty& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.sa3dProperties, index, result);

    if (result == ERROR_NONE)
    {
        outProp.version                  = propIn.version;
        outProp.ambisonicType            = propIn.ambisonicType;
        outProp.ambisonicOrder           = propIn.ambisonicOrder;
        outProp.ambisonicChnlSeq         = propIn.ambisonicChnlSeq;
        outProp.ambisonicNorml           = propIn.ambisonicNorml;
        outProp.chnlMap                  = makeVarLenArray<uint32_t>(propIn.chnlMap);
    }

    return result;
}

int32_t Mp4Reader::GetSteScop3DProp(uint32_t trackId,
                                                       uint32_t sampleId,
                                                       OmniStereoScopic3D& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.st3dProperties, index, result);

    if (result == ERROR_NONE)
    {
        outProp = propIn;
    }

    return result;
}

int32_t Mp4Reader::GetSpheV1Prop(uint32_t trackId,
                                                         uint32_t sampleId,
                                                         SphericalVideoV1Property& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.sphericalV1Properties, index, result);

    if (result == ERROR_NONE)
    {
        outProp = propIn;
    }

    return result;
}

int32_t Mp4Reader::GetSpheV2Prop(uint32_t trackId,
                                                         uint32_t sampleId,
                                                         SphericalVideoV2Property& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.sv3dProperties, index, result);

    if (result == ERROR_NONE)
    {
        outProp = propIn;
    }

    return result;
}

int32_t Mp4Reader::GetRWPKProp(uint32_t trackId,
                                                          uint32_t sampleId,
                                                          RWPKProperty& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.rwpkProperties, index, result);

    if (result == ERROR_NONE)
    {
        outProp.constituentPictureMatching = propIn.constituentPicMatching;
        outProp.packedPicHeight            = propIn.packedPicHeight;
        outProp.packedPicWidth             = propIn.packedPicWidth;
        outProp.projPicHeight              = propIn.projPicHeight;
        outProp.projPicWidth               = propIn.projPicWidth;
        outProp.regions                    = makeVarLenArray<RWPKRegion>(propIn.regions);
    }

    return result;
}

int32_t Mp4Reader::GetCOVIInfoProp(uint32_t trackId,
                                                            uint32_t sampleId,
                                                            COVIInformation& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.coviProperties, index, result);

    if (result == ERROR_NONE)
    {
        outProp.coviShapeType       = propIn.coviShapeType;
        outProp.defaultViewIdc      = (OmniViewIdc) propIn.defaultViewIdc;
        outProp.viewIdcPresenceFlag = propIn.viewIdcPresenceFlag;
        outProp.sphereRegions       = makeVarLenArray<COVIRegion>(propIn.sphereRegions);
    }

    return result;
}

int32_t Mp4Reader::GetProjFrmtProp(uint32_t trackId,
                                                         uint32_t sampleId,
                                                         ProjFormat& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.pfrmProperties, index, result);

    if (result == ERROR_NONE)
    {
        outProp = propIn;
    }

    return result;
}

int32_t Mp4Reader::GetStereVideoProp(uint32_t trackId,
                                                                 uint32_t sampleId,
                                                                 VideoFramePackingType& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.stviProperties, index, result);

    if (result == ERROR_NONE)
    {
        outProp = propIn;
    }

    return result;
}

int32_t Mp4Reader::GetRotateProp(uint32_t trackId, uint32_t sampleId, Rotation& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.rotnProperties, index, result);

    if (result == ERROR_NONE)
    {
        outProp = propIn;
    }

    return result;
}


int32_t Mp4Reader::GetScheTypesProp(uint32_t trackId,
                                          uint32_t sampleId,
                                          SchemeTypesProperty& outProp) const
{
    TrackBasicInfo basicTrackInfo;
    SmpDesIndex index;
    uint32_t result = SearchInfoForTrack(trackId, sampleId, basicTrackInfo, index);
    auto propIn     = GetSampProp(basicTrackInfo.schemeTypesProperties, index, result);

    if (result == ERROR_NONE)
    {
        outProp.mainScheme.type    = propIn.mainScheme.GetSchemeType().GetUInt32();
        outProp.mainScheme.version = propIn.mainScheme.GetSchemeVersion();
        outProp.mainScheme.uri     = makeVarLenArray<char>(propIn.mainScheme.GetSchemeUri());

        std::vector<SchemeType> compatibleTypes;
        for (auto& compatibleScheme : propIn.compatibleSchemes)
        {
            SchemeType compType{};
            compType.type    = compatibleScheme.GetSchemeType().GetUInt32();
            compType.version = compatibleScheme.GetSchemeVersion();
            compType.uri     = makeVarLenArray<char>(compatibleScheme.GetSchemeUri());
            compatibleTypes.push_back(compType);
        }
        outProp.compatibleSchemeTypes = makeVarLenArray<SchemeType>(compatibleTypes);
    }

    return result;
}

uint32_t Mp4Reader::SearchInfoForTrack(uint32_t trackId,
                                              uint32_t sampleId,
                                              TrackBasicInfo& basicTrackInfo,
                                              SmpDesIndex& index) const
{
    if (IsInitErr())
    {
        return OMAF_MP4READER_NOT_INITIALIZED;
    }

    InitSegmentTrackId trackIdPair = MakeIdPair(trackId);
    InitSegmentId initSegId       = trackIdPair.first;
    SegmentId segIndex;
    int32_t result = GetSegIndex(trackIdPair, sampleId, segIndex);
    if (result != ERROR_NONE)
    {
        return result;
    }
    SegmentTrackId segTrackId = make_pair(segIndex, trackIdPair.second);

    CtxType ctxType;
    int error = GetCtxTypeError(trackIdPair, ctxType);
    if (error)
    {
        return error;
    }

    if (ctxType == CtxType::TRACK)
    {
        ItemId sampId;
        auto& infoOfSamp = GetSampInfos(initSegId, segTrackId, sampId);
        index            = infoOfSamp.at((ItemId(sampleId) - sampId).GetIndex()).sampleDescriptionIndex;
        basicTrackInfo    = GetTrackBasicInfo(trackIdPair);
        return ERROR_NONE;
    }

    return OMAF_INVALID_MP4READER_CONTEXTID;
}

VCD_MP4_END
