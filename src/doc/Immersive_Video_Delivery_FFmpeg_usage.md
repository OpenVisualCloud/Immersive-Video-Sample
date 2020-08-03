# Immersive Video Delivery FFmpeg Plugins

## Introduction
- There are 3 FFmpeg plugins provides in the project: OMAF Packing, Distribute Encoder, and OMAF Dash Access. The Plugin implementation provides a quick way to build tiled-based 360 Video delivery pipeline from server transcoding to client playback based on the libraries built from the projects. At the same time, the plugins are also the sample code how to use relative library.
- To build a E2E 360 Video delivery pipeline, the FFmpeg plugins provided key functions and relative build-in or external modules of FFmpeg are needed too to get the best performance, for example, HEVC/H264 decoder, scaling filter, demuxer, and so on. To achieve the best performance, parameter tuning of each components is must.
- The FFmpeg patches based on specific code base are provided in the project to enable these plugins.
- The sample command for "decoder + encoder + packing":
```bash
  ffmpeg -i [file] -input_type 1 -c:v:0 distributed_encoder -s:0 3840x1920 -tile_row:0 6 -tile_column:0 10 -config_file:0 config_high.txt -g:0 15 -b:0 30M -map 0:v -c:v:1 distributed_encoder -s:1 1024x640 -tile_row:1 2 -tile_column:1 4 -config_file:1 config_low.txt -g:1 15 -b:1 5M -map 0:v -f omaf_packing -is_live 0 -split_tile 1 -seg_duration 1 -window_size 20 -extra_window_size 30 -base_url http://[server ip]]/OMAFStatic_4k/ -out_name Test /usr/local/nginx/html/OMAFStatic_4k/
```

## OMAF Packing Plugin
OMAF Packing Plugin is a multiplexer in the pipeline to use OMAF packing library to packing input bitstream to generate OMAF-compliant DASH content. Plugin name is "omaf_packing". The options are available for this plugins are listed as belows.

| **Parameters** | **Descriptions** | **Type** | **Default Value** | **Range** | **Must-Have** |
| --- | --- | --- | --- | --- | --- |
| packing_proj_type | input source projection type | string | "ERP" | "ERP" or "Cubemap" | NO |
| cubemap_face_file | configure input cubemap face relation to face layout defined in OMAF for cube-3x2 | string | N/A | N/A | NO |
| viewport_w | set viewport width | int | 1024 | N/A | NO |
| viewport_h | set viewport height | int | 1024 | N/A | NO |
| viewport_yaw | set viewport yaw angle, which is the angle around y axis | float | 90 | [0.0, 180.0] | NO |
| viewport_pitch | set viewport pitch angle, which is the angle around x axis | float | 0 | [0.0, 100.0] | NO |
| viewport_fov_hor | set horizontal angle of field of view (FOV) | float | 80 | [0.0, 180.0] | NO |
| viewport_fov_ver | set vertical angle of field of view (FOV) | float | 90 | [0.0, 100.0] | NO |
| window_size | number of segments kept in the manifest | int | 5 | N/A | NO |
| extra_window_size | "number of segments kept outside of the manifest before removing from disk" | int | 15 | N/A | NO |
| split_tile | need split the stream to tiles if input is tile-based hevc stream | int | 0 | 0,1 | NO |
| seg_duration | segment duration (in u seconds, fractional value can be set) | int | 5000000 | N/A | NO |
| remove_at_exit | "remove all segments when finished", OFFSET(remove_at_exit) | int | 0 | 0,1 | NO |
| use_template | Use SegmentTemplate instead of SegmentList", OFFSET(use_template) | bool | 0 | 0,1 | NO |
| use_timeline | Use SegmentTimeline in SegmentTemplate", OFFSET(use_timeline) | int | 0 | 0,1 | NO |
| utc_timing_url | URL of the page that will return the UTC timestamp in ISO format" | string | N/A | N/A | NO |
| is_live | Enable/Disable streaming mode of output. Each frame will be moof fragment" | bool | 0 | 0,1 | YES |
| base_url | MPD BaseURL, it can be the the url of generated segmentatio and MPD files | string | N/A | N/A | YES |
| out_name | name prefix for all dash output files" | string | N/A | "dash-stream" | NO |
| need_buffered_frames | needed buffered frames number before packing starts | int | 15 | N/A | NO |
| extractors_per_thread | extractor tracks per segmentation thread | int | 0 | N/A | NO |
| has_extractor | Enable/Disable OMAF extractor tracks| int | 1 | 0, 1 | NO |
| plugin_path | OMAF Packing plugin path | string | N/A | "/usr/local/lib" | NO |
| plugin_name | OMAF Packing plugin name | string | N/A | "HighResPlusFullLowResPacking" | NO |

## Distribute Encoder Plugin
Distribute Encoder Plugin is using DistributeEncoder library to do SVT-based HEVC Encoding. Plugin name is "distributed_encoder", The options are available for this plugins are listed as belows.
| **Parameters** | **Descriptions** | **Type** | **Default Value** | **Range** | **Must-Have** |
| --- | --- | --- | --- | --- | --- |
| config_file | configure file path for workers information | string | N/A | N/A | YES |
| proj_type | input source projection type, ERP or Cubemap | string | "ERP" | "ERP" or "Cubemap" | NO |
| input_type | input stream type, 0 - encoded, 1 - raw, default is 0" | int | 0 | 0,1 | NO |
| input_codec | input bitstream type, only work when input type is 0-encoded, 0 - HEVC, 1 - AVC, default is 0" | int | 0 | 0,1 | NO |
| vui | Enable vui info | int | 0 | 0,1 | NO |
| aud | Include AUD", OFFSET(aud) | int | 0 | 0,1 | NO |
| hielevel | Hierarchical prediction levels setting, 0:flat, 1:2level, 2:3level, 3:4level | int | 3 | 0,1,2,3 | NO |
| la_depth | Look ahead distance [0, 256] | int | -1 | [-1,256] | NO |
| preset | Encoding preset [0, 12] (e,g, for subjective quality tuning mode and >=4k resolution), [0, 10] (for >= 1080p resolution), [0, 9] (for all resolution and modes) | int | 9 | [0,12] | NO |
| profile | Profile setting, Main Still Picture Profile not supported | int | 2 | 1,2,3,4 | NO |
| tier | Set tier (general_tier_flag), 0: main, 1: high| int | 0 | 0,1 | NO |
| level | Set level (level_idc) | int | 0 | [0,255] | NO |
| rc | "Bit rate control mode, 0:cqp, 1:vbr | int | 0 | 0,1 | NO |
| qp | QP value for intra frames | int | 32 | 0,51 | NO |
| sc_detection | Scene change detection | int | 0 | 0,1 | NO |
| tune | Quality tuning mode, 0: sq, 1:oq, 2:vmaf | int | 1 | 0,1,2 | NO |
| bl_mode | Random Access Prediction Structure type setting | int | 0 | 0,1 | NO |
| hdr | High dynamic range input | int | 0 | 0,1 | NO |
| asm_type | Assembly instruction set type [0: C Only, 1: Auto] | int | 0 | 0,1 | NO |
| tile_column | tile column count number, default is 1| int | 1 | [1,256] | NO |
| tile_row | tile row count number, default is 1| int | 1 | [1,256] | NO |
| in_parallel | Multiple encoders running in parallel [0: Off, 1: On] | int | 0 | 0,1 | NO |

## OMAF Dash Access Plugin
OMAF Dash Access Plugin is an FFmpeg demux used to access OMAF Dash Content by DashAccessLibrary. The demux name is "tiled_dash_demuxer". The plugin is used to playback the content for test purpose. The options are available for this plugins are listed as belows.
| **Parameters** | **Descriptions** | **Type** | **Default Value** | **Range** | **Must-Have** |
| --- | --- | --- | --- | --- | --- |
| allowed_extensions | List of file extensions that dash is allowed to access | string | "mpd" | N/A | YES |
| cache_path | the specific path of cache folder, default is /home | string | "/home" | N/A | YES |
| enable_extractor | whether to enable extractor track in OMAF Dash Access engine  | int | 1 | 0,1 | YES |

Sample Command:
```bash
  ffplay -allowed_extensions mpd -enable_extractor 0 [mpd url]
```