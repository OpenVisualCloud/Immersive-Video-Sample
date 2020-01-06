# Immersive Video OMAF Sample

## Introduction
   The Immersive Video Delivery OMAF Sample provides a quick trial to setup E2E OMAF-Compliant 360 video streaming. OMAF 360 Video streaming Samples can support both VOD and Live streaming for 8K and 4K contents. 

## Hardware Requirements

## Build

 - From source
   
   - server :
    ```bash
    cd src/external && ./build_server.sh
    ```
   - client :
    ```bash
    cd src/external && ./build_client.sh
    ```
 - deployment :

   - server :
    ```bash
    cd OMAF-Sample/server && ./deploy.sh
    docker image ls # Created an image. [REPOSITORY:immersive_server, TAG:v0.1]
    ```
   - client :
    ```bash
    cd OMAF-Sample/client && ./deploy.sh
    cd ../package # Created packages.
    dpkg -i immersive-client_1-1.0.0-1.el7_amd64.deb
    ```

## How to run (https)

 - From source
   
   - server :
    ```bash
    cd src/external && sudo ./run_server.sh
    ```
   - client :
    ```bash
    cd build/client/plyer
    vim config.xml(Set up configuration)
    ./render
    ```

 - deployment
   - server
    ```bash
    cd OMAF-Sample/server && ./deploy.sh
    docker run -p 5000:443 -p 5001:8080 -it immersive_server:v0.1 /bin/bash # Map the port
    cd Sample-Videos && ./run.sh
    ```
   - client
    ```bash
    cd /usr/bin/immersive/
    sudo vim config.xml(Set up configuration) :
    sudo ./render
    ```
    
