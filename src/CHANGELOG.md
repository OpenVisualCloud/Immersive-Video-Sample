# **Changelog**

## System Requirement:
-  Typical Hardware Configuration
   Server: 
        Intel Xeon 6xxx for 4K video; Intel Xeon 8xxx for 8K video
		memory >= 128GB
   Client: i7-6770 + 8G Memory
   
-  OS Requirement:
   Server: CentOS 7.4/7.6
   Client: CentOS7.4/CentOS7.6/Ubuntu18.04
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

-  360 Video Processing Library
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

**Know Issues:**
-  Cube-map projection format hasn't been support yet;
-  Audio segmentation hasn't been support yet;
---
