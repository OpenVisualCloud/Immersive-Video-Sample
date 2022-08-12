
# Docker image deployment steps

## Build docker image

```bash
./build_image.sh webrtc360:v5.0
```

## Run docker container

```bash
docker run --privileged --network=host -it webrtc360:v5.0 /bin/bash
```

## Inside docker container:

 - Init and start owt-server

```bash
cd /root/owt-server
# init owt-server, only init once after reboot
./dist/bin/init-all.sh # just type Enter
# Update RabbitMQ/MongoDB Account? [No/yes]
# Press "Enter" to use default RabbitMQ/MongoDB account
./dist/bin/start-all.sh
```

 - Setup resolution and configuration

```bash
cd /home/WebRTC360/rest/
. "$HOME/.nvm/nvm.sh"
npm install mongojs
npm install .
./rest/control.js -s 4k
# { room_resolution: { width: 3840, height: 2048 } }
# Room : sampleRoom
# Room resolution 3840 x 2048
# Update room mediaIn, h265
# Update room mediaOut, h265
# Room 62f36771189e1302a902b991 updated
```

 - Start streaming

```bash
./rest/control.js -c /home/WebRTC360/test1_h265_3840x2048_30fps_30M_200frames.mp4
# { stream_url:
#    '/home/WebRTC360/test1_h265_3840x2048_30fps_30M_200frames.mp4' }
# Stream 614594682930454900 added
```
