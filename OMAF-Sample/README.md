# Immersive Video OMAF Sample

## Introduction
   The Immersive Video Delivery OMAF Sample provides a quick trial to setup E2E OMAF-Compliant 360 video streaming. OMAF 360 Video streaming sample can support both VOD and Live streaming for 4K and 8K contents.

   OMAF sample can be deployed with [Kubernetes](#kubernetes-deployment-steps) or directly with [Docker image](#docker-image-deployment-steps).

## Software Requirements

 - Server OS : CentOS Linux release 7.6.1810 (Core)
 - Client OS : Ubuntu 18.04 LTS
 - Docker version : 19.03.12

## Test Environment Hardware
| Platform | Server | Client |
|:----:|:----:|:----:|
| CPU SKU | Intel® Xeon® Platinum<br>8280M CPU @ 2.70GHz | Intel® Core™ i7-6770HQ<br>CPU @ 2.60GHz x 8 |
| Memory | 128G | 16G |

---

## Docker image deployment steps

### Installation

- [Install docker engine in server](https://docs.docker.com/install)

- Server :
```bash
    cd path_to/Immersive-Video-Sample/OMAF-Sample/server
    mkdir build && cd build
    cmake .. -DHTTP_PROXY=<proxy> # proxy is optional
    make build -j $(nproc)
    docker image ls        # [REPOSITORY:immersive_server, TAG:v1.4]
```

- Client :
```bash
    cd OMAF-Sample/client && ./deploy.sh
```

### How To Run (HTTPS)

- Server :
```bash
    docker run --privileged -p 30001:443 -p 30002:8080 -it immersive_server:v1.4 bash  # Map the port.
    cd /usr/local/nginx/conf/
    ./configure.sh CN Shanghai A B C D E@F.com                                     # './configure.sh -h' for details.
    /usr/local/nginx/sbin/nginx                                                    # Start nginx.
    cd /home/immersive/Sample-Videos && ./run.sh <RES> <TYPE>                      # <RES>:[4K,8K] <TYPE>:[LIVE,VOD]
```

For details in FFmpeg plugins' parameters, refer to the [FFmpeg usage doc](../src/doc/Immersive_Video_Delivery_FFmpeg_usage.md).

- Client :
```bash
    sudo su
    cd path_to/Immersive-Video-Sample/src/build/client/player
    export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH
    vim config.xml  # Set up configuration, details in table at bottom.
    ./render        # Press 'q' button to quit.
```

---

## Kubernetes deployment steps

### Installation

 - Node set up steps same as [docker image deployment](#docker-image-deployment-steps).

 - Master set up steps:
    1. Follow the [instructions](https://kubernetes.io/docs/setup) to setup your Kubernetes cluster.
    2. All cluster nodes must have the same user (uid) and group (gid).
    3. Setup password-less access from the Kubernetes controller to each worker node:
    ```bash
        ssh-keygen
        ssh-copy-id <worker-node>
    ```


### How To Run
 - Master start/stop services as follows:
```
    cd path_to/Immersive_Video_Sample/OMAF-Sample/server
    mkdir build && cd build
    cmake ..
    make start -j $(nproc) # choose to start or stop
```

- Client :
```bash
    sudo su
    cd path_to/Immersive-Video-Sample/src/build/client/player
    export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH
    vim config.xml  # Set up configuration, details in table at bottom.
    ./render        # Press 'q' button to quit.
```

---

**Config.xml**

| **Parameters** | **Descriptions** | **examples** |
| --- | --- | --- |
| windowWidth | The width of render window | 960 for 4k, 1920 for 8k |
| windowHeight | The height of render window  | 960 for 4k, 1920 for 8k  |
| url | The resource URL path | Remote URL |
| sourceType | Source type | 0 is for Dash Source |
| enableExtractor | extractor track path or later binding path | 1 is for extractor track and 0 is for later binding |
| StreamDumpedOption | dump packet streams or not | 0 for false, 1 for true |
| viewportHFOV | Viewport horizon FOV degree | 80 |
| viewportVFOV | Viewport vertical FOV degree | 80 |
| viewportWidth | Viewport width | 960 for 4k, 1920 for 8k |
| viewportHeight | Viewport height | 960 for 4k, 1920 for 8k |
| cachePath | Cache path | /tmp/cache |
| minLogLevel | min log level | INFO / WARNING / ERROR / FATAL |
| maxVideoDecodeWidth | max video decoded width | decoded width that is supported |
| maxVideoDecodeHeight | max video decoded height | decoded height that is supported |
| predict | viewport prediction plugin | 0 is disable and 1 is enable |
| PathOf360SCVPPlugins | path of 360SCVP plugins | needed for planar format rendering |

   - **Note** : So far, some parameters settings are limited. URL need to be a remote dash source URL, choose `./run.sh 8K LIVE` for example : `https://xxx.xxx.xxx.xxx:30001/LIVE8K/Test.mpd`. The parameter `sourceType` must set to 0, which represents dash source. The parameter `decoderType` must set to 0, which stands for FFmpeg software decoder. The parameter `contextType` need to be 0, which represents glfw context. And `useDMABuffer` flag should be set to 0.
