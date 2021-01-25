# Immersive Video Delivery Source
[![Travis Build Status](https://travis-ci.com/OpenVisualCloud/Immersive-Video-Sample.svg?branch=master)](https://travis-ci.com/OpenVisualCloud/ImmersiveVideo)
[![Stable release](https://img.shields.io/badge/latest_release-v1.0.0-green.svg)](CHANGELOG.md)
[![Contributions](https://img.shields.io/badge/contributions-welcome-blue.svg)](https://github.com/OpenVisualCloud/Immersive-Video-Sample/wiki)

The Immersive Video Delivery solution provides basic components for OMAF-Compliant Tiled 360 Video Delivery, including MCTS-based HEVC transcoding, OMAF-compliant streaming packing, client OMAF dash access library; and FFMPEG plugins for quick trial for these components to setup E2E OMAF-Compliant 360 video streaming. The project is a reference solution for those want to build up tiled-based 360 video delivery based on Intel Platform.

# License
The Immersive Video Delivery solution is licensed under the BSD 3-Clause "New" or "Revised" License, except that "FFMPEG Plugins" is under the LGPLv2.0 license. See [LICENSE](LICENSE) for details. 
MP4 base library is from Nokia/OMAF in OMAF Packing Library and OMAF Dash Access Library, so the license should compliant with Nokia/OMAF license.

# How to contribute
See [CONTRIBUTING](CONTRIBUTING.md) for details. Thank you!

# Documents
-  How to build and run, refer to [OMAF_Compliant Immersive Video Getting Started Guide](doc/Immersive_Video_Getting_Started_Guide.md)
-  Solution architecure, refer to [OMAF_Compliant Immersive Video Reference Architecture](doc/Immersive_Video_Delivery_Architecture.md)


# What's in this project
The Immersive Video Delivery Reference solution contains below components:
-  [OMAF Packing Library](doc/Immersive_Video_Delivery_OMAF_Packing.md)
-  [OMAF Dash Access Library](doc/Immersive_Video_Delivery_DashAccess.md)
-  [360 Video Processing Library](doc/Immersive_Video_Delivery_360SCVP.md)
-  [Reference OMAF Player](doc/Immersive_Video_Delivery_RefPlayer.md)
-  [FFMPEG Plugins](doc/Immersive_Video_Delivery_FFmpeg_usage.md)
-  Documents

# System requirements
## Operating system
The Immersive Video Delivery Reference solution may run on Linux* 64 bit operating systems. The list below represents the operating systems that the solution was tested and validated on:
- Client: CentOS7.6 or Ubuntu* 18.04 
- Server: CentOS* 7.6

## Typical Hardware Configuration
- **Server**: 
    + Intel Xeon 6xxx for 4K video; Intel Xeon 8xxx for 8K video
	+ Memory >= 128GB
- **Client**:
    + i7-6770 + 8G Memory

