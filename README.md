# Open Visual Cloud Immersive Video Samples
[![Travis Build Status](https://travis-ci.com/OpenVisualCloud/Immersive-Video-Sample.svg?branch=master)](https://travis-ci.com/OpenVisualCloud/Immersive-Video-Sample)
[![Stable release](https://img.shields.io/badge/latest_release-v1.0-green.svg)](https://github.com/OpenVisualCloud/Immersive-Video-Sample/releases/tag/v1.0)
[![Contributions](https://img.shields.io/badge/contributions-welcome-blue.svg)](https://github.com/OpenVisualCloud/Immersive-Video-Sample/wiki)

The Immersive Video Samples includes 2 samples which is based on different streaming framework. OMAF samples is based on OMAF standard, uses MPEG DASH as the protocol to deliver tiled 360 video stream, and it can support both VOD and live streaming mode. The low-latency sample enables 360 tiled streaming based on WebRTC framework to support low-latency. Both of the samples use SVT-HEVC with MCTS supported to do 360 video tiled encoding and achieve Real-time performance for 4K and 8K contents.  

# License
The Immersive Video low-latency Samples is licensed under the OSI-approved BSD 3-Clause license. See [Low-Latency-Sample LICENSE](low-latency-sample/LICENSE) for details. And OMAF samples is licensed under OSI-approved BSD 3-Clause license and LGPLv2.0 license for different components, see [OMAF-Sample LICENSE](src/LICENSE);

# How to contribute
See [CONTRIBUTING](CONTRIBUTING.md) for details. Thank you!

# What's in this project
The CDN Transcode Sample contains below components:
-  Source code of the components which is necessary to build the samples. Please refer to [src](src/README.md) for details.
-  OMAF Sample: Immersive 360 Video streaming sample based on OMAF standard which uses MPEG-DASH to delivery Tiled 360 video. Please refer to [OMAF Sample Readme](OMAF-Sample/README.md) for details.
-  Low-Latency Sample: use WebRTC framework to live streaming Immersive 360 Video. Please refer to [Low-Latency Sample Readme](Low-Latency-Sample/README.md) for details.

# System requirements
## Operating system
The Immersive Video samples may run on Linux* 64 bit operating systems. The list below represents the operating systems that the samples were tested and validated on:
- Client: CentOS 7.6 or Ubuntu* 18.04 Server LTS
- Server: CentOS 7.6

# How to report bugs
Use the [Issues](https://github.com/OpenVisualCloud/Immersive-Video-Sample/issues) tab on GitHub.