# Immersive Video Delivery Getting Started Guide

   * [Immersive Video Delivery Getting Started Guide](#immersive-video-delivery-getting-started-guide)
   * [Introduction](#introduction)
   * [Prerequisites](#prerequisites)
   * [Build](#build)
      * [Build Server Components](#build-server-preresuisties)
      * [Build Client Components](#build-client-preresuisties)
   * [Quick Run](#quick-run)

This document describes how to build OMAF-Compliant Immsersive Video Delivery components and how to run sample tests.

Intel Immersive Video Delivery solution provides basic components for OMAF-Compliant Tiled 360 Video Delivery, including MCTS-based HEVC transcoding, OMAF-compliant streaming packing, client OMAF dash access library; and FFMPEG plugins for quick trial for these components to setup E2E OMAF-Compliant 360 video streaming. Both VOD and Live streaming can be supported with the solution. The solution only supports one video processing mode which is not in ISO/IEC 23090-2 Annex D; and it provides an common interface for developer to create new video processing mode.

# Introduction
Intel VCD Immersive Video Delivery solution provides basic components for OMAF-Compliant Tiled 360 Video Delivery, including MCTS-based HEVC transcoding, OMAF-compliant streaming packing, client OMAF dash access library; and FFMPEG plugins for quick trial for these components to setup E2E OMAF-Compliant 360 video streaming.
<IMG src="img/OMAF_Compliant-Video-Delivery-Introduction.png" height="450">

please refer to [wiki]() for more information.

# Prerequistes
To build the whole solution, there are some prerequistes must be ready.
```bash
gcc >= 6.3.1
g++ >= 6.3.1
cmake > 3.12.4
```

You can use the following command to install relative dependency in server/client side:
```bash
sudo yum install pcre-devel openssl openssl-devel
sudo yum install devtoolset-6-gcc devtoolset-6-gcc-c++
```

# Build
## Build Server Components
```bash
git clone https://github.com/OpenVisualCloud/ImmersiveVideo
cd ImmersiveVideo/src
mkdir build
cd build 
cmake .. -DSERVER
```

## Build Client Components
```bash
- git clone https://github.com/OpenVisualCloud/ImmersiveVideo
- cd ImmersiveVideo/src
- mkdir build
- cd build 
- cmake .. -DCLIENT
```

# Quick Run
To run sample test, Ngnix server should be installed in server side. please refer to [ngnix setup](ngnix_setup.md)

## Server Side 

Set up RSA if HTTPS is used
```bash
sudo ssh-keygen -t rsa
sudo ssh-copy-id root@<server ip>
```

### Live Streaming
- Test Command for 4K Video:

```bash
  ffmpeg -i [rtmp://localhost/demo/1.flv] -input_type 1 -c:v:0 distributed_encoder -s:0 3840x1920 -tile_row:0 6 -tile_column:0 10 -config_file:0 config_high.txt -g:0 15 -b:0 30M -map 0:v -c:v:1 distributed_encoder -s:1 1024x640 -tile_row:1 2 -tile_column:1 4 -config_file:1 config_low.txt -g:1 15 -b:1 5M -map 0:v -f omaf_packing -is_live 1 -split_tile 1 -seg_duration 1 -window_size 20 -extra_window_size 30 -base_url http://[ServerIP]/OMAFLive_4k/ -out_name Test /usr/local/nginx/html/OMAFLive_4k/
```

- Test Command for 8K Video:
```bash
  numactl -c 1 ffmpeg -re -i [rtmp://192.168.1.10:1935/live/video] -input_type 1 -rc 1 -c:v:0 distributed_encoder -s:0 7680x3840 -g:0 25 -tile_row:0 6 -tile_column:0 12 -la_depth:0 0 -config_file:0 config_high.txt -b:0 50M -map 0:v -c:v:1 distributed_encoder -s:1 1280x1280 -sws_flags neighbor -g:1 25 -tile_row:1 2 -tile_column:1 2 -la_depth:1 0 -config_file:1 config_low.txt -b:1 2M -map 0:v -f omaf_packing -is_live 1 -split_tile 1 -seg_duration 1 -extractors_per_thread 4 -base_url http://[ServerIP]/OMAFLive_8k/ -out_name Test /usr/local/nginx/html/OMAFLive_8k/ 
```

### Tiled Content Generation for VOD

- Test Command for 4K Video:
```bash  
  ffmpeg -i [file] -input_type 1 -c:v:0 distributed_encoder -s:0 3840x1920 -tile_row:0 6 -tile_column:0 10 -config_file:0 config_high.txt -g:0 15 -b:0 30M -map 0:v -c:v:1 distributed_encoder -s:1 1024x640 -tile_row:1 2 -tile_column:1 4 -config_file:1 config_low.txt -g:1 15 -b:1 5M -map 0:v -f omaf_packing -is_live 0 -split_tile 1 -seg_duration 1 -window_size 20 -extra_window_size 30 -base_url http://[server ip]]/OMAFStatic_4k/ -out_name Test /usr/local/nginx/html/OMAFStatic_4k/
```

- Test Command for 8K Video:
```bash
  numactl -c 1 ffmpeg -re -i [rtmp://192.168.1.10:1935/live/video] -input_type 1 -rc 1 -c:v:0 distributed_encoder -s:0 7680x3840 -g:0 25 -tile_row:0 6 -tile_column:0 12 -la_depth:0 0 -config_file:0 config_high.txt -b:0 50M -map 0:v -c:v:1 distributed_encoder -s:1 1280x1280 -sws_flags neighbor -g:1 25 -tile_row:1 2 -tile_column:1 2 -la_depth:1 0 -config_file:1 config_low.txt -b:1 2M -map 0:v -f omaf_packing -is_live 1 -split_tile 1 -seg_duration 1 -extractors_per_thread 4 -base_url http://[ServerIP]/OMAFStatic_8k/ -out_name Test /usr/local/nginx/html/OMAFStatic_8k/ 
```

## Client Side

- modify the config.xml; please refer to [Reference Player Configuration](Immersive_Video_Delivery_RefPlayer.md) for detail information

```bash
- run ./render
- type 's' to start playing.
```
