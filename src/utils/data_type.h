/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   data_type.h
 * Author: media
 *
 * Created on January 15, 2019, 1:48 PM
 */

#ifndef _DATA_TYPE_H__
#define _DATA_TYPE_H__

#include <stdint.h>
#include "360SCVPAPI.h"
#include "ns_def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  VideoCodec_NONE = 0,
  VideoCodec_AVC,
  VideoCodec_HEVC,
  VideoCodec_AV1,

  AudioCodec_NONE = 100,
  AudioCodec_AAC,
  AudioCodec_AV3,
  AudioCodec_MP3,
} Codec_Type;

typedef enum {
  MediaType_NONE = 0,
  MediaType_Video,
  MediaType_Audio,
} MediaType;

typedef enum {
  MODE_DEFAULT = 0,
  MODE_TILE_MultiRes,
  MODE_TILE_MultiRate,
  MODE_NONE,
} MPD_MODE;

typedef struct HEADPOSE {
  float yaw;
  float pitch;
  uint64_t pts;
} HeadPose;

typedef struct HEADSETINFO {
  HeadPose* pose;
  float viewPort_hFOV;
  float viewPort_vFOV;
  int32_t viewPort_Width;
  int32_t viewPort_Height;
} HeadSetInfo;

typedef struct Viewport {
  int32_t x;
  int32_t y;
  int32_t height;
  int32_t width;
  int32_t faceId;
} Viewport;

const int MAX_QUALITY_NUM = 2;

typedef enum {
  HIGHEST_QUALITY_RANKING = 1,
  NORMAL_QUALITY_RANKING = 2,
  INVALID_QUALITY_RANKING,
} QualityRank;

typedef struct SOURCERESOLUTION {
  QualityRank qualityRanking;  // the quality ranking value
  uint32_t top;
  uint32_t left;    // the pos of the quality stream relative to whole source
  uint32_t width;   // the width of the quality stream in source
  uint32_t height;  // the height of the quality stream in source
} SourceResolution;

/*
 * avg_bandwidth : average bandwidth since the begin of downloading
 * immediate_bandwidth: immediate bandwidth at the moment
 */
typedef struct DASHSTATISTICINFO {
  int32_t avg_bandwidth;
  int32_t immediate_bandwidth;
} DashStatisticInfo;

/*
 * stream_type : Video or Audio stream
 * height : the height of original video
 * width : the width of original video
 * framerate_num : frame rate
 * framerate_den : frame rate
 * channels : for Audio
 * sample_rate : for Audio
 *
 */
typedef struct DASHSTREAMINFO {
  MediaType stream_type;
  Codec_Type codec_type;
  int32_t height;
  int32_t width;
  uint32_t tileRowNum;
  uint32_t tileColNum;
  int32_t framerate_num;
  int32_t framerate_den;
  uint64_t segmentDuration;
  int32_t bit_rate;
  int32_t channels;
  int32_t sample_rate;
  int32_t channel_bytes;
  int32_t mProjFormat;
  int32_t mFpt;
  const char* mime_type;
  const char* codec;
  int32_t source_number;
  SourceResolution* source_resolution;
} DashStreamInfo;

/*
 * duration : the duration of the media. it is meaningless if streaming_type is dynamic
 * streaming_type: the DASH stream type: 1. static or 2. dynamic
 * stream_count: stream count of this media
 * stream_info: the array to store the information of each streams
 */
typedef enum DASHSTREAMTYPE {
  DASH_STREAM_STATIC = 1,
  DASH_STREAM_DYNMIC = 2,
} DashStreamType;

typedef struct DASHMEDIAINFO {
  uint64_t duration;
  DashStreamType streaming_type;
  int32_t stream_count;
  DashStreamInfo stream_info[16];
} DashMediaInfo;

typedef struct DASHPACKET {
  uint32_t videoID;
  Codec_Type video_codec;
  uint32_t pts;
  uint64_t size;
  char* buf;
  RegionWisePacking* rwpk;
  int segID;
  int32_t height;
  int32_t width;
  int32_t numQuality;               //! number of source from stream with different qualities
  SourceResolution* qtyResolution;  //! the resolution value of each stream int source
  uint32_t tileRowNum;              //! til row after aggregation
  uint32_t tileColNum;              //! til row after aggregation
  bool bEOS;
} DashPacket;

typedef enum {
  HIGH = 0,
  LOW = 1,
  END = 2,
}ViewportPriority;

typedef enum {
  SingleViewpoint = 0,
  MultiViewpoints = 1,
  UNKNOWN,
}PredictionMode;

typedef struct PREDICTOPTION {
  PredictionMode mode;
  bool usingFeedbackAngleAdjust;
}PredictOption;

typedef struct VIEWPORTANGLE {
  // Euler angle
  float yaw;
  float pitch;
  float roll;
  uint64_t pts;
  ViewportPriority priority;
} ViewportAngle;

#ifdef __cplusplus
}
#endif

#endif /* DATA_TYPE_H */
