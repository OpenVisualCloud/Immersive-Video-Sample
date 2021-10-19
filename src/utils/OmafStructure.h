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
//!
//! \file:   OmafStructure.h
//! \brief:  Internal structure definition for the library.
//! \detail:
//! Created on May 28, 2019, 4:58 PM
//!

#ifndef STRUCT_DEF_H
#define STRUCT_DEF_H

#include "ns_def.h"
#include <vector>
#include <list>
#include <string>

VCD_OMAF_BEGIN

#define MAX_PACKET_SIZE                         (((7680 * 3840 * 3) / 2) / 2) //<! maximum packet size (frame bitstream size)

#define HEVC_STARTCODES_LEN                     4 //<! the number of bytes for HEVC start codes
#define HEVC_NALUHEADER_LEN                     2 //<! the number of bytes for HEVC NALU Header
#define DASH_SAMPLELENFIELD_SIZE                4 //<! the number of bytes for DASH sample length field

#define HEVC_SPS_NALU_TYPE                      33

#define HEVC_PPS_NALU_TYPE                      34

#define NTP_SEC_1900_TO_1970                    2208988800ul

#define ESSENTIALPROPERTY                       "EssentialProperty"
#define SUPPLEMENTALPROPERTY                    "SupplementalProperty"
#define ADAPTATIONSET                           "AdaptationSet"
#define DASH_MPD                                "MPD"
#define PERIOD                                  "Period"
#define VIEWPORT                                "Viewport"
#define REPRESENTATION                          "Representation"
#define BASEURL                                 "BaseURL"
#define SEGMENTTEMPLATE                         "SegmentTemplate"

#define SCHEMEIDURI                             "schemeIdUri"
#define SCHEMEIDURI_VIEWPORT                    "urn:mpeg:dash:viewpoint:2011"
#define SCHEMEIDURI_RWPK                        "urn:mpeg:mpegI:omaf:2017:rwpk"
#define SCHEMEIDURI_CC                          "urn:mpeg:mpegI:omaf:2017:cc"
#define SCHEMEIDURI_PF                          "urn:mpeg:mpegI:omaf:2017:pf"
#define SCHEMEIDURI_SRQR                        "urn:mpeg:mpegI:omaf:2017:srqr"
#define SCHEMEIDURI_2DQR                        "urn:mpeg:mpegI:omaf:2017:2dqr"
#define SCHEMEIDURI_FRAMEPACKINGTYPE            "urn:mpeg:mpegB:cicp:VideoFramePackingType"
#define SCHEMEIDURI_SRD                         "urn:mpeg:dash:srd:2014"
#define SCHEMEIDURI_PRESELECTION                "urn:mpeg:dash:preselection:2016"
#define SCHEMEIDURI_AUDIO                       "urn:mpeg:dash:23003:3:audio_channel_configuration:2011"
#define OMAF_XMLNS_VALUE                        "urn:mpeg:mpegI:omaf:2017"
#define XSI_XMLNS_VALUE                         "null"
#define XMLNS_VALUE                             "urn:mpeg:dash:schema:mpd:2011"
#define XLINK_XMLNS_VALUE                       "null"
#define XSI_SCHEMALOCATION_VALUE                "urn:mpeg:dash:schema:mpd:2011"
#define PROFILE_LIVE                            "urn:mpeg:dash:profile:isoff-live:2011"
#define PROFILE_ONDEMOND                        "urn:mpeg:dash:profile:isoff-on-demand:2011"

//attribute keys
#define OMAF_PACKINGTYPE                        "omaf:packing_type"
#define COMMON_VALUE                            "value"
#define OMAF_PROJECTIONTYPE                     "omaf:projection_type"
#define OMAF_XMLNS                              "xmlns:omaf"
#define XSI_XMLNS                               "xmlns:xsi"
#define XMLNS                                   "xmlns"
#define XLINK_XMLNS                             "xmlns:xlink"
#define XSI_SCHEMALOCATION                      "xsi:schemaLocation"
#define DURATION                                "duration"
#define START                                   "start"
#define MIMETYPE                                "mimeType"
#define CODECS                                  "codecs"
#define MAXWIDTH                                "maxWidth"
#define MAXHEIGHT                               "maxHeight"
#define GOPSIZE                                 "gopSize"
#define MAXFRAMERATE                            "maxFramerate"
#define SEGMENTALIGNMENT                        "segmentAlignment"
#define SUBSEGMENTALIGNMENT                     "subsegmentAlignment"
#define BITSTREAMSWITCHING                      "bitstreamSwitching"
#define INDEX                                   "id"
#define QUALITYRANKING                          "qualityRanking"
#define DEPENDENCYID                            "dependencyId"
#define BANDWIDTH                               "bandwidth"
#define WIDTH                                   "width"
#define HEIGHT                                  "height"
#define ORIGWIDTH                               "orig_width"
#define ORIGHEIGHT                              "orig_height"
#define REGIONWIDTH                             "region_width"
#define REGIONHEIGHT                            "region_height"
#define FRAMERATE                               "frameRate"
#define STARTWITHSAP                            "startWithSAP"
#define TIMESCALE                               "timescale"
#define MEDIA                                   "media"
#define INITIALIZATION                          "initialization"
#define STARTNUMBER                             "startNumber"
#define SAR                                     "sar"
#define MINBUFFERTIME                           "minBufferTime"
#define MPDTYPE                                 "type"
#define MEDIAPRESENTATIONDURATION               "mediaPresentationDuration"
#define PROFILES                                "profiles"
#define VALUE                                   "value"

#define MAXSEGMENTDURATION                      "maxSegmentDuration"
#define AVAILABILITYSTARTTIME                   "availabilityStartTime"
#define MINIMUMUPDATEPERIOD                     "minimumUpdatePeriod"
#define TIMESHIFTBUFFERDEPTH                    "timeShiftBufferDepth"
#define PUBLISHTIME                             "publishTime"

//attribute values
#define MIMETYPE_VALUE                          "video/mp4 profiles=&apos;hevd&apos;"
#define MIMETYPE_AUDIO                          "audio/mp4"
#define CODECS_VALUE                            "resv.podv+ercm.hvc1.2.4.L90.80"
#define CODECS_AUDIO                            "mp4a.40.1"
#define CODECS_VALUE_EXTRACTORTRACK             "resv.podv+ercm.hvc2.2.4.L120.80"
#define TYPE_STATIC                             "static"
#define TYPE_LIVE                               "dynamic"

#define SEGMENT_NUMBER                          "Number"
#define SEGMENT_REPRESENTATIONID                "RepresentationID"
#define SEGMENT_TIME                            "Time"
#define SEGMENT_BANDWIDTH                       "Bandwidth"

#define AUDIOSAMPLINGRATE                       "audioSamplingRate"
#define AUDIOCHANNELCONFIGURATION               "AudioChannelConfiguration"

//CC relative
//node name
#define OMAF_CC                                 "omaf:cc"
#define OMAF_COVERAGE_INFO                      "omaf:coverageInfo"
#define OMAF_SPHREGION_QUALITY                  "omaf:sphRegionQuality"
#define OMAF_QUALITY_INFO                       "omaf:qualityInfo"
#define OMAF_TWOD_QUALITYINFO                   "omaf:twoDqualityInfo"
#define OMAF_TWOD_REGIONQUALITY                 "omaf:twoDRegionQuality"
//attribute
#define SHAPE_TYPE                              "shape_type"
#define VIEW_IDC_PRESENCE_FLAG                  "view_idc_presence_flag"
#define DEFAULT_VIEW_IDC                        "default_view_idc"

#define VIEW_IDC                                "view_idc"
#define CENTRE_AZIMUTH                          "centre_azimuth"
#define CENTRE_ELEVATION                        "centre_elevation"
#define CENTRE_TILT                             "centre_tilt"
#define AZIMUTH_RANGE                           "azimuth_range"
#define ELEVATION_RANGE                         "elevation_range"

#define REMAINING_AREA_FLAG                     "remaining_area_flag"
#define QUALITY_RANKING_LOCAL_FLAG              "quality_ranking_local_flag"
#define QUALITY_TYPE                            "quality_type"
#define QUALITY_RANKING                         "quality_ranking"
#define ORIG_WIDTH                              "orig_width"
#define ORIG_HEIGHT                             "orig_height"

#define LEFT_OFFSET                             "left_offset"
#define TOP_OFFSET                              "top_offset"
#define REGION_WIDTH                            "region_width"
#define REGION_HEIGHT                           "region_height"

//!
//! \class:  OmafSrd
//! \brief:  class for SRD information.
//! \detail:
//!
//!
//!

class OmafSrd{
public:
    OmafSrd(){
        left = 0;
        top = 0;
        width = 0;
        height = 0;
        id = -1;
    };

    ~OmafSrd(){};

public:
    void SetInfo(char* value)
    {
        int32_t id, w, h, res;
        w = h = 0;
        res = sscanf( value, "%d,%d,%d,%d,%d,%d,%d", &id, &this->left, &this->top, &this->width, &this->height, &w, &h);
        if (res != 7) {
            res = sscanf( value, "%d,%d,%d,%d,%d", &id, &this->left, &this->top, &this->width, &this->height );
            if (res != 5) res=0;
        }
    };

    int32_t get_X(){return left;};
    int32_t get_Y(){return top;};
    int32_t get_W(){return width;};
    int32_t get_H(){return height;};

private:
    int32_t top;
    int32_t left;
    int32_t width;
    int32_t height;
    int32_t id;

};

typedef struct SAMPLEDATA {
    char    *data; //only one NALU
    int64_t  data_length;
    bool     is_ready;
    OmafSrd* srd_info;
}SampleData;

typedef enum{
    PF_UNKNOWN     = -1,
    PF_ERP         =  0,
    PF_CUBEMAP     =  1,
    PF_PLANAR      =  2,
    PF_RESERVED,
}ProjectionFormat;

typedef enum{
    FP_UNKNOWN             = -1,
    FP_TOP_BOTTOM          =  3,
    FP_SIDE_BY_SIDE        =  4,
    FP_TEMPORAL_INTERLEAVE =  5,
    FP_UNSUPPORTED,
}FramePackingType;

typedef enum{
    RWPK_UNKNOWN     = -1,
    RWPK_RECTANGULAR =  0,
    RWPK_RESERVED,
}RwpkType;

typedef struct COVERAGEINFO{
    int32_t view_idc;                                    //<! 1: left view of stereoscopic content
                                                     //<! 2. right view of stereoscopic content
    int32_t centre_azimuth;                              //<! Specify the azimuth of the centre point of
                                                     //<! the sphere region in units of 2^-16 degree
                                                     //<! relative to global coordinates axes
    int32_t centre_elevation;                            //<! Specify the elevation of the centre point of
                                                     //<! the sphere region in units of 2^-16 degree
                                                     //<! relative to global coordinates axes
    int32_t centre_tilt;                                 //<! Specify tile angle ofsphere region in units
                                                     //<! of 2^-16 degree relative to global coordinates axes
    int32_t azimuth_range;                               //<! Specify the azimuth range of the sphere region
                                                     //<! through the centre point of the sphere region
                                                     //<! in units of 2^-16 degrees.
    int32_t elevation_range;                             //<! Specify the elevation range of the sphere region
                                                     //<! through the centre point of the sphere region
                                                     //<! in units of 2^-16 degrees
}CoverageInfo;

typedef struct CONTENTCOVERAGE{
    int32_t                       shape_type;            //<! Specifies the shape type of the sphere region
                                                     //<! 0: the sphere region is specified by four great circles
                                                     //<! 1: the sphere region is specified by 2 azimuth circles and 2 elevation circles
                                                     //<! other values are reserved
    int32_t                       view_idc_presence;     //<! 0: not signaled; 1: indicates the association of sphere regions with
                                                     //<! paricular (left, right, or both)views or monoscopic content.
    int32_t                       default_view_idc;      //<! 0: all sphere region are monoscopic
                                                     //<! 1: all the sphere regions are on the left view of a sterescopic content
                                                     //<! 2: all the sphere regions are on the right view of a sterescopic content
                                                     //<! -1: not present. it must be present when view_idc_presence = 1
    std::vector<CoverageInfo>     coverage_infos;
}ContentCoverage;

typedef struct SRQRQUALITYINFO{
    int32_t                       quality_ranking;       //<! ranking value of hte quality ranking sphere region
    int32_t                       view_idc;              //<! the same meaning as ContentCoverage.default_view_idc
    int32_t                       orig_width;            //<! Indicates teh width of such a monoscopic projected picture for which horRatio
                                                     //<! id equals to 1. not presented if qulity_type <> 1
    int32_t                       orig_height;           //<! Indicates teh height of such a monoscopic projected picture for which horRatio
                                                     //<! id equals to 1. not presented if qulity_type <> 1
    int32_t                       centre_azimuth;        //<! see ISO/IEC 23009-2 table-15
    int32_t                       centre_elevation;      //<! see ISO/IEC 23009-2 table-15
    int32_t                       centre_tilt;           //<! see ISO/IEC 23009-2 table-15
    int32_t                       azimuth_range;         //<! see ISO/IEC 23009-2 table-15
    int32_t                       elevation_range;       //<! see ISO/IEC 23009-2 table-15
}SrqrQualityInfo;

typedef struct SPHEREQUALITY{
    int32_t                           shape_type;             //<! see ISO/IEC 23009-2 table-15
    int32_t                           remaining_area_flag;    //<! see ISO/IEC 23009-2 table-15
    int32_t                           view_idc_presence;      //<! see ISO/IEC 23009-2 table-15
    int32_t                           quality_ranking_local;  //<! see ISO/IEC 23009-2 table-15
    int32_t                           quality_type;           //<! see ISO/IEC 23009-2 table-15
    int32_t                           default_view_idc;       //<! see ISO/IEC 23009-2 table-15
    std::vector<SrqrQualityInfo>  srqr_quality_infos;
}SphereQuality;

typedef struct TWODQUALITYINFO{
    int32_t                           quality_ranking;        //<! see ISO/IEC 23009-2 table-16
    int32_t                           view_idc;               //<! see ISO/IEC 23009-2 table-16
    int32_t                           orig_width;             //<! see ISO/IEC 23009-2 table-16
    int32_t                           orig_height;            //<! see ISO/IEC 23009-2 table-16
    int32_t                           left_offset;            //<! see ISO/IEC 23009-2 table-16
    int32_t                           top_offset;             //<! see ISO/IEC 23009-2 table-16
    int32_t                           region_width;           //<! see ISO/IEC 23009-2 table-16
    int32_t                           region_height;          //<! see ISO/IEC 23009-2 table-16
}TwoDQualityInfo;

typedef struct TWODQUALITYRANKING{
    int32_t                           remaining_area_flag;    //<! see ISO/IEC 23009-2 table-16
    int32_t                           view_idc_presence;      //<! see ISO/IEC 23009-2 table-16
    int32_t                           quality_ranking_local;  //<! see ISO/IEC 23009-2 table-16
    int32_t                           quality_type;           //<! see ISO/IEC 23009-2 table-16
    int32_t                           default_view_idc;       //<! see ISO/IEC 23009-2 table-16
    std::vector<TwoDQualityInfo>  twod_quality_infos;
}TwoDQualityRanking;


typedef struct MPDINFO{
    std::string  ID;
    std::vector<std::string>      profiles;                            /*MANDATORY*/
    std::vector<std::string>      baseURL;
    std::string                   type;
    uint64_t                      availabilityStartTime;               /* expressed in milliseconds */
                                                                       /*MANDATORY if type=dynamic*/
    uint64_t                      availabilityEndTime;                 /* expressed in milliseconds */
    uint64_t                      publishTime;                         /* expressed in milliseconds */
    uint64_t                      media_presentation_duration;         /* expressed in milliseconds */
                                                                       /*MANDATORY if type=static*/
    uint32_t                      minimum_update_period;               /* expressed in milliseconds */
    uint32_t                      min_buffer_time;                     /*MANDATORY expressed in milliseconds */

    uint32_t                      time_shift_buffer_depth;             /* expressed in milliseconds */
    uint32_t                      suggested_presentation_delay;        /* expressed in milliseconds */

    uint32_t                      max_segment_duration;                /* expressed in milliseconds */
    uint32_t                      max_subsegment_duration;             /* expressed in milliseconds */
    std::string                   mpdPathBaseUrl;
    uint32_t                      fetchTime;
}MPDInfo;

typedef struct PRESELVALUE{
    std::string                   PreselTag;               //<! the preselection ID;
    std::vector<int>              SelAsIDs;                 //<! vector to save all selected ID.
}PreselValue;

VCD_OMAF_END;

#endif /* STRUCT_DEF_H */

