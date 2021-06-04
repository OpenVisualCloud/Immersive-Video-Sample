# **Changelog**
---
## [1.6.0] - 2021-6-4
**Features & bug fix:**
- OMAF Packing Library
   + Support GOP size output in DASH MPD file

- OMAF Dash Access Library
   + Support motion halting possibility output with viewport prediction
   + Support in-time viewport update strategy to reduce M2HQ latency
   + bug fix : incorrect pts, framerate calculation

- 360SCVP (Stream Concatenation and Viewport Processing) Library
   + Support HEVC B frame in bitstream parsing and stitching
   + Optimize viewport related tiles selectin for both equirectangular and cube-map projections
   + Support webrtc under cube-map projection
   + Optimize memory copy in GenerateSliceHdr

- Reference OMAF Player
   + Android Player: Support in-time viewport update strategy to reduce M2HQ latency
   + Android Player: Support user input configuration
   + Linux Player: Support in-time viewport update strategy to reduce M2HQ latency
   + Linux Player: Support performance data log and frame sequences log to trace M2HQ latency

- FFmpeg Plugins & Encoder Library
   + Encoder Library:add HEVC B frame support and optimize config file parsing
   + FFmpeg Plugins: add option for HEVC B frame support

---
## [1.4.0] - 2021-1-14
**Features & bug fix:**
- OMAF Packing Library
   + Support packing for cube-map projection in extractor track mode
   + Support both fixed and dynamic sub-picture resolutions in extractor track mode
   + Support packing for AAC audio stream
   + Support packing for planar projection in late-binding mode
   + Plugin mode to support customized media stream process
   + Support external log callback
   + bug fix: memory leak, hang / crash in some condition

- OMAF Dash Access Library
   + Support cube-map projection in extractor track mode
   + Support maximum decodable picture width and height limitation in late-binding mode
   + Support DASH de-packing for AAC audio stream segments
   + Support planar projection in late-binding mode
   + bug fix: memory leak, time out, tiles stitching disorder in some condition

- 360SCVP (Stream Concatenation and Viewport Processing) Library
   + code refactor: add plugin definition for tile selection
   + optimization for tile selection to improve performance, accuracy and efficiency
   + Support external log callback

- Reference OMAF Player
   + Android Player: with ERP and Cube-map support
   + Android platform: extend DashAccess JNI library with MediaCodec decoder integrated.
   + Linux Player: Support WebRTC source with multiple video stream decoding, rendering; and RTCP FOV feedback
   + Linux Player: Support Planar Video
   + Code refactor

- FFmpeg Plugins & Encoder Library
   + Encoder Library: Bug fix for memory leak
   + FFmpeg Plugins: add option for external log callback and log level set
   + FFmpeg Plugins: add option for fixed/dynamic sub-picture resolution for extractor track mode
   + FFmpeg Plugins: add audio stream input process
   + FFmpeg Plugins: add option for planar projection support
   + FFmpeg Plugins: add option for customized packing plugin and media stream process plugin set

---
## [1.2.0] - 2020-8-14
**Features & bug fix:**
- OMAF Packing Library
   + Support late-binding mode, option for extractor track generation
   + Support packing for cube-map projection in late-binding mode
   + Optimize tile partition for extractor generation
   + Plugin mode to support customized packing method
   + bug fix: memory leak, hang / crash in some condition

- OMAF Dash Access Library
   + Support late-binding mode: tile-selection and bit-stream rewriting in client side
   + Support cube-map projection in late-binding mode
   + Enable NDK cross-compiling, Add JNI support for Dash Access API
   + Optimization of downloading and segmentation parsing for fault-tolerance and stability
   + Plugin mode to support customized FOV sequence operation

- 360SCVP (Stream Concatenation and Viewport Processing) Library
   + Support Cube-map projection: tile processing and viewport processing
   + optimization for tile selection to improve performance and accuracy

- Reference OMAF Player
   + Support Cube-map projection
   + Support multiple video streams decoding and tile rendering
   + Code refactor

- FFmpeg Plugins & Encoder Library
   + Encoder Library: Enable local session support to improve performance
   + Encoder Library: Bug fix for memory leak, resource release, share memory usage, call mechanism, etc.
   + FFmpeg Plugins: add option for Cube-map projection support
   + FFmpeg Plugins: add option for late-binding support

---
## [1.0.0] - 2020-01-13   
**Features:** 
- OMAF Packing Library
   + Generate OMAF compliant DASH MPD and tiled-based mp4 segmentation.
   + Support OMAF viewport-dependent baseline presentation profile and HEVC-based viewport-dependent OMAF video profile.
   + Support viewport-dependent extractor track generation and relative meta data generation
   + Support tiled video processing with multi-resolution content
   + Support modes: VOD and Live Streaming

-  OMAF Dash Access Library
   + Support OMAF-Compliant MPD file parser; 
   + Support Tile-based MP4 segmentation (media track and extractor track) downloading and parsing;
   + Support Viewport-dependent extractor track selection and reading;
   + Support OMAF-compliant metadata parsing;
   + Support HTTPS/HTTP;

-  360SCVP (Stream Concatenation and Viewport Processing) Library
   + Provide a unify interface to process tile-based HEVC bitstream processing and viewport-based Content Processing;
   + Support HEVC bitstream processing: VPS/SPS/PPS parsing and generating, 360Video-relative SEI Generating and parsing, HEVC tile-based bitstream aggregation;
   + Support Viewport generation, viewport-based tile selection and extractor selection based on content coverage;

-  Reference OMAF Player
   + Support Linux platform with OpenGL rendering
   + Support OMAF-Compliant Dash source and WebRTC-based tile source;
   + Support ERP-based 360Video Rendering;
   + Support Tile-based 360Video Rendering;
   + Support Intel Gen-GPU Hardware acceleration with GPU is presented in the system; 

-  FFMPEG Plugins
   + Demux plugin with OMAF Dash accessing support;
   + Multiplexing plugin with OMAF Packing library support;
   + SVT encoder support

**Know Issues:**
-  Audio segmentation hasn't been support yet;
---
