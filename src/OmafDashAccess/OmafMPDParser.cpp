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

#include "OmafMPDParser.h"
#include "OmafExtractor.h"
#include <typeinfo>

VCD_OMAF_BEGIN

OmafMPDParser::OmafMPDParser()
{
    mParser = nullptr;
    this->mMpd = NULL;
    this->mMPDURL = "";
    this->mCacheDir = "";
    this->mLock = new ThreadLock();
    mMPDInfo = nullptr;
    mPF = PF_UNKNOWN;
    mExtractorEnabled = true;
}

OmafMPDParser::~OmafMPDParser()
{
    SAFE_DELETE(mParser);
    //SAFE_DELETE(mMpd);
    SAFE_DELETE(mLock);
}

int OmafMPDParser::ParseMPD( std::string mpd_file, OMAFSTREAMS& listStream )
{
    int ret = ERROR_NONE;

    if(nullptr == mParser)
        mParser = new OmafXMLParser();

    mLock->lock();

    mMPDURL = mpd_file;

    ODStatus st = mParser->Generate(const_cast<char *>(mMPDURL.c_str()), mCacheDir);
    if(st != OD_STATUS_SUCCESS)
    {
        mLock->unlock();
        LOG(INFO)<<"failed to parse MPD file."<<endl;
        return st;
    }

    mMpd = mParser->GetGeneratedMPD();

    if(NULL == mMpd){
        mLock->unlock();
        return ERROR_PARSE;
    }

    ret = ParseMPDInfo();

    if(ret != ERROR_NONE) {
        mLock->unlock();
        return ret;
    }

    ret = ParseStreams(listStream);

    if(ret != ERROR_NONE) {
        mLock->unlock();
        return ret;
    }

    mLock->unlock();

    return ret;
}

int OmafMPDParser::ParseMPDInfo()
{
    mMPDInfo = new MPDInfo;
    if(!mMPDInfo)
        return ERROR_NULL_PTR;

    auto baseUrl = mMpd->GetBaseUrls().back();
    mMPDInfo->mpdPathBaseUrl               = baseUrl->GetPath();
    mMPDInfo->profiles                     = mMpd->GetProfiles();
    mMPDInfo->type                         = mMpd->GetType();

    mMPDInfo->media_presentation_duration  = parse_duration( mMpd->GetMediaPresentationDuration().c_str()    );
    mMPDInfo->availabilityStartTime        = parse_date    ( mMpd->GetAvailabilityStartTime().c_str()        );
    mMPDInfo->availabilityEndTime          = parse_date    ( mMpd->GetAvailabilityEndTime().c_str()          );
    mMPDInfo->max_segment_duration         = parse_duration     ( mMpd->GetMaxSegmentDuration().c_str()           );
    mMPDInfo->min_buffer_time              = parse_duration     ( mMpd->GetMinBufferTime().c_str()                );
    mMPDInfo->minimum_update_period        = parse_duration     ( mMpd->GetMinimumUpdatePeriod().c_str()          );
    mMPDInfo->suggested_presentation_delay = parse_int     ( mMpd->GetSuggestedPresentationDelay().c_str()   );
    mMPDInfo->time_shift_buffer_depth      = parse_duration     ( mMpd->GetTimeShiftBufferDepth().c_str()         );

    mBaseUrls = mMpd->GetBaseUrls();
    // Get all base urls except the last one
    for(uint32_t i = 0; i < mBaseUrls.size() - 1 ; i++)
    {
        mMPDInfo->baseURL.push_back(mBaseUrls[i]->GetPath());
    }

    mPF = mMpd->GetProjectionFormat();

    return ERROR_NONE;
}

int OmafMPDParser::UpdateMPD(OMAFSTREAMS& listStream)
{
    return ParseMPD(this->mMPDURL, listStream);
}

MPDInfo* OmafMPDParser::GetMPDInfo()
{
    return this->mMPDInfo;
}

//!
//! \brief construct media streams.
//!
int OmafMPDParser::ParseStreams( OMAFSTREAMS& listStream )
{
    int ret = ERROR_NONE;

    std::vector<PeriodElement *> Periods = mMpd->GetPeriods();
    if(Periods.size() == 0)
        return ERROR_NO_VALUE;

    //processing only the first period;
    PeriodElement *pPeroid = Periods[0];

    TYPE_OMAFADAPTATIONSETS adapt_sets;

    ret = GroupAdaptationSet( pPeroid, adapt_sets );

    ret = BuildStreams( adapt_sets, listStream );

    return ret;
}

int OmafMPDParser::GroupAdaptationSet(PeriodElement* pPeriod, TYPE_OMAFADAPTATIONSETS& mapAdaptationSets )
{
    ADAPTATIONSETS AdaptationSets = pPeriod->GetAdaptationSets();

    /// so far, we supposed that there will be only one viewpoint in the mpd,
    /// so all Adaptation sets are belong to the same audio-visual content.
    /// FIXIT, if there are multiple viewpoints.
    for(auto it = AdaptationSets.begin(); it != AdaptationSets.end(); it++ ){
        AdaptationSetElement *pAS = (AdaptationSetElement*) (*it);
        OmafAdaptationSet* pOmafAS = CreateAdaptationSet(pAS);

        /// catalog the Adaptation according to the media type: video, audio, etc
        std::string type   = GetSubstr(pOmafAS->GetMimeType(), '/', true);

        mapAdaptationSets[type].push_back(pOmafAS);
    }

    return ERROR_NONE;
}

int OmafMPDParser::BuildStreams( TYPE_OMAFADAPTATIONSETS mapAdaptationSets, OMAFSTREAMS& listStream )
{
    int ret = ERROR_NONE;
    uint32_t allExtractorCnt = 0;
    std::map<std::string, OmafMediaStream*> streamsMap;
    for(auto it = mapAdaptationSets.begin(); it != mapAdaptationSets.end(); it++){
        OMAFADAPTATIONSETS ASs = it->second;
        std::string type = it->first;

        OmafMediaStream* pStream = new OmafMediaStream();
        auto mainASit = ASs.begin();

        for(auto as_it = ASs.begin(); as_it != ASs.end(); as_it++){
            OmafAdaptationSet* pOmafAs = (OmafAdaptationSet*)(*as_it);
            pOmafAs->SetBaseURL(mBaseUrls);
	    if ( typeid(*pOmafAs) == typeid( OmafExtractor ) ){
                if (mExtractorEnabled)
                {
                    OmafExtractor *tmpOmafAs = (OmafExtractor*)pOmafAs;
                    pStream->AddExtractor(tmpOmafAs);
                    pStream->SetExtratorAdaptationSet(tmpOmafAs);
                }
            }else{
                pStream->AddAdaptationSet(pOmafAs);
                if(pOmafAs->IsMain())
                {
                    pOmafAs->SetProjectionFormat(mPF);
                    pStream->SetMainAdaptationSet(pOmafAs);
                    mainASit = as_it;
                }
            }
        }

        std::map<int, OmafExtractor*> extractors = pStream->GetExtractors();
        if (extractors.size())
        {
            allExtractorCnt++;
        }

        // remove main AS from AdaptationSets for it has no real data
        ASs.erase(mainASit);

        streamsMap.insert(std::make_pair(type, pStream));
    }
    LOG(INFO)<<"allExtractorCnt"<<allExtractorCnt<<endl;
    if (allExtractorCnt < mapAdaptationSets.size())
    {
        if (mExtractorEnabled)
        {
            LOG(INFO) << "There isn't extractor track from MPD parsing, extractor track enablement should be false !" << std::endl;
            mExtractorEnabled = false;
            ret = OMAF_INVALID_EXTRACTOR_ENABLEMENT;
        }
    }
    std::map<std::string, OmafMediaStream*>::iterator itStream;
    for (itStream = streamsMap.begin(); itStream != streamsMap.end(); itStream++)
    {
        std::string type = itStream->first;
        OmafMediaStream *stream = itStream->second;
        stream->SetEnabledExtractor(mExtractorEnabled);
        stream->InitStream(type);
        listStream.push_back(stream);
    }
    return ret;
}

OmafAdaptationSet* OmafMPDParser::CreateAdaptationSet(AdaptationSetElement* pAS)
{
    if( ExtractorJudgement(pAS) ){
        return new OmafExtractor(pAS);
    }
    return new OmafAdaptationSet(pAS);
}

bool OmafMPDParser::ExtractorJudgement(AdaptationSetElement* pAS)
{
    PreselValue *sel = pAS->GetPreselection();
    if(sel)
        return true;

    ///FIXME, if @DependencyID has multiple dependency ID, then set it as extractor.
    std::vector<std::string> depIDs = pAS->GetRepresentations()[0]->GetDependencyIDs();
    if( depIDs.size() > 0 )
    {
        return true;
    }

    return false;

}

VCD_OMAF_END

