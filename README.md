

# Open Visual Cloud Immersive Video Samples
[![Travis Build Status](https://travis-ci.com/OpenVisualCloud/Immersive-Video-Sample.svg?branch=master)](https://travis-ci.com/OpenVisualCloud/Immersive-Video-Sample)
[![Stable release](https://img.shields.io/badge/latest_release-v1.4.0-green.svg)](https://github.com/OpenVisualCloud/Immersive-Video-Sample/releases/latest)
[![Contributions](https://img.shields.io/badge/contributions-welcome-blue.svg)](https://github.com/OpenVisualCloud/Immersive-Video-Sample/wiki)

This Immersive Video project includes 2 samples which are based on different streaming frameworks.  
OMAF sample is based on OMAF standard, uses MPEG DASH as the protocol to deliver tiled 360 video stream, and it can support both VOD and live streaming mode.  
WebRTC sample enables tiled 360 video streaming based on WebRTC protocol and Open WebRTC Toolkit media server framework for low-latency streaming.  
Both of the samples use SVT-HEVC with MCTS supported to do 360 video tiled encoding and achieve real-time performance for 4K and 8K contents.  

# What's in this project
The Immersive Video project contains below components:
-  **Source Code** of the components which is necessary to build the samples. Please refer to [src](src/README.md) for details.
-  **OMAF Sample**: Immersive 360 video streaming sample based on OMAF standard which uses MPEG-DASH to delivery tiled 360 video. Please refer to [OMAF-Sample](OMAF-Sample/README.md) for details.
-  **WebRTC Sample**: WebRTC streaming protocol and Open WebRTC Toolkit media server framework based low-latency live streaming for immersive 360 video. Please refer to [WebRTC-Sample](WebRTC-Sample/README.md) for details.

# System requirements
## Operating system
The Immersive Video samples may run on Linux* 64 bit operating systems. The list below represents the operating systems that the samples were tested and validated on:
- **Client**: CentOS 7.6 or Ubuntu* 18.04 Server LTS
- **Server**: CentOS 7.6

# License
OMAF sample is licensed under the BSD 3-Clause "New" or "Revised" License, except that "FFMPEG Plugins" is under the LGPLv2.0 license, see [OMAF-Sample LICENSE](src/LICENSE).

WebRTC sample is licensed under Apache License 2.0, see [WebRTC-Sample LICENSE](WebRTC-Sample/owt-server/LICENSE). 

# How to contribute
See [CONTRIBUTING](CONTRIBUTING.md) for details. Thank you!

# How to report bugs
Use the [Issues](https://github.com/OpenVisualCloud/Immersive-Video-Sample/issues) tab on GitHub.
