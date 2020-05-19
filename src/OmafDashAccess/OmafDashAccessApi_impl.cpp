/*
 * Copyright (c) 2018, Intel Corporation
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


/*
 * File:   VRDashStreamingAPI.h
 * Author: Zhang, Andrew
 *
 * Created on January 15, 2019, 1:11 PM
 */

#include "OmafDashAccessApi.h"

#include <cstdlib>
#include "general.h"
#include "OmafMediaSource.h"
#include "OmafDashSource.h"
#include "../utils/GlogWrapper.h"

using namespace std;

VCD_USE_VROMAF;
VCD_USE_VRVIDEO;

Handler OmafAccess_Init( DashStreamingClient* pCtx)
{
    OmafMediaSource* pSource = new OmafDashSource();

    return (Handler)((long)pSource);
}

int OmafAccess_OpenMedia( Handler hdl, DashStreamingClient* pCtx, bool enablePredictor, char *predictPluginName, char *libPath)
{
    OmafMediaSource* pSource = (OmafMediaSource*)hdl;
    pSource->SetLoop(false);
    int ret = pSource->OpenMedia(pCtx->media_url, pCtx->cache_path,
        pCtx->enable_extractor, enablePredictor,
        predictPluginName, libPath);

    return ret;
}

int OmafAccess_CloseMedia( Handler hdl )
{
    OmafMediaSource* pSource = (OmafMediaSource*)hdl;

    return pSource->CloseMedia();
}

int OmafAccess_SeekMedia( Handler hdl, uint64_t time )
{
    OmafMediaSource* pSource = (OmafMediaSource*)hdl;

    pSource->SeekTo(time);

    return 0;
    //return pSource->SeekTo(time);
}

int OmafAccess_GetMediaInfo( Handler hdl, DashMediaInfo* info )
{
    OmafMediaSource* pSource = (OmafMediaSource*)hdl;
    pSource->GetMediaInfo(info);
    return ERROR_NONE;
}

int OmafAccess_GetPacket(
    Handler hdl,
    int stream_id,
    DashPacket* packet,
    int* size,
    uint64_t* pts,
    bool needParams,
    bool clearBuf )
{
    OmafMediaSource* pSource = (OmafMediaSource*)hdl;
    std::list<MediaPacket*> pkts;
    pSource->GetPacket(stream_id, &pkts, needParams, clearBuf);

    if( 0 == pkts.size()) {
        return ERROR_NULL_PACKET;
    }

    *size = pkts.size();

    int i = 0;
    for(auto it=pkts.begin(); it!=pkts.end(); it++){
        MediaPacket* pPkt = (MediaPacket*)(*it);
        if(!pPkt)
        {
            *size -= 1;
            continue;
        }
        int outSize = pPkt->Size();
        char* buf = (char*)malloc(outSize * sizeof(char));
        memcpy(buf, pPkt->Payload(), outSize);
        RegionWisePacking *newRwpk = new RegionWisePacking;
        RegionWisePacking *pRwpk = pPkt->GetRwpk();
        *newRwpk = *pRwpk;
        newRwpk->rectRegionPacking = new RectangularRegionWisePacking[newRwpk->numRegions];
        memcpy(newRwpk->rectRegionPacking, pRwpk->rectRegionPacking, pRwpk->numRegions * sizeof(RectangularRegionWisePacking));
        packet[i].rwpk = newRwpk;
        packet[i].buf  = buf;
        packet[i].size = outSize;
        packet[i].segID = pPkt->GetSegID();
        i++;

        delete pPkt;
        pPkt = NULL;
    }

    return ERROR_NONE;
}

int OmafAccess_SetupHeadSetInfo( Handler hdl, HeadSetInfo* clientInfo)
{
    OmafMediaSource* pSource = (OmafMediaSource*)hdl;

    return pSource->SetupHeadSetInfo(clientInfo);
}

int OmafAccess_ChangeViewport( Handler hdl, HeadPose* pose)
{
    OmafMediaSource* pSource = (OmafMediaSource*)hdl;

    return pSource->ChangeViewport(pose);
}

int OmafAccess_Statistic( Handler hdl, DashStatisticInfo* info )
{
    OmafMediaSource* pSource = (OmafMediaSource*)hdl;

    return pSource->GetStatistic(info);
}

int OmafAccess_Close( Handler hdl )
{
    OmafMediaSource* pSource = (OmafMediaSource*)hdl;
    delete pSource;
    return ERROR_NONE;
}


