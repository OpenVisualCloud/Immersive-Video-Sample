# WebRTC360
It's a webrtc server for 360 video use case.<br>
Download, build and pack a official owt-server-5.0.x DIST in third-party. Then it will build a immersive video mixer node to replace VideoMixer in DIST, and a WebRTC node for tiles selection and FOV feedback.

To build and run on a bare-metal machine, following steps below.
As to deploy a docker image, go to `deploy` for details.

## Install

1. Install owt-server
```
./scripts/install_owt_server.sh
```

2. Install deps
```
./scripts/install_deps.sh
```

3. Build immersive video mixer node
```
./scripts/build.js -t all
```

4. Pack IM video DIST
```
./scripts/pack.sh
```

## Run Server

```bash
cd dist

# init owt-server, only init once after reboot
./bin/init-all.sh
# Update RabbitMQ/MongoDB Account? [No/yes]
# Press "Enter" to use default RabbitMQ/MongoDB account

# start owt-server
./bin/start-all.sh

# stop owt-server
./bin/stop-all.sh
killall -9 node

# check logs for debug
ls logs/

# restart owt-server = stop + rm logs + start
./bin/stop-all.sh
killall -9 node

rm logs/* -v

./bin/start-all.sh
```

## Start 360 video streaming

```bash
cd ../rest/
. "$HOME/.nvm/nvm.sh"
npm install mongojs
npm install .

# start owt-server
# configure owt-server, 4k or 8k
./control.js -s 4k
# restart owt-server to take into effect

# start input, 4k or 8k
# it is ready for connction from clients
./control.js -c /home/WebRTC360/test1_h265_3840x2048_30fps_30M_200frames.mp4
# check logs
# dist/logs/streaming-xxx-x.log
# dist/logs/video-xxx-x.log
# dist/logs/webrtc-xxx-x.log
```
