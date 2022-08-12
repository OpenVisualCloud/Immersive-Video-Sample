#!/bin/bash -e
SCRIPT=`pwd`/$0
PATHNAME=`dirname $SCRIPT`
ROOT=$PATHNAME/..
OWT_DIST=$ROOT/third_party/owt-server/dist
DIST=$ROOT/dist

if [ ! -d $DIST ]
then
  cp -rfv $OWT_DIST $DIST
fi

# video node
cp -v $ROOT/source/im_video_mixer_sw/src/build/Release/IMVideoMixer-sw.node $DIST/video_agent/videoMixer_sw/build/Release/videoMixer-sw.node
cp -v $ROOT/build/libdeps/build/lib/libSvtHevcEnc.so* $DIST/video_agent/lib
cp -v $ROOT/build/libdeps/build/lib/libyaml-cpp.so* $DIST/video_agent/lib
cp -v $ROOT/source/im_video_mixer_sw/src/mcts_encoder.yaml $DIST/video_agent
cp -v $ROOT/source/im_video_mixer_sw/src/4k_encoder.yaml $DIST/video_agent
cp -v $ROOT/source/im_video_mixer_sw/src/8k_encoder.yaml $DIST/video_agent

# webrtc node
cp -v $ROOT/source/rtc_frame/src/build/Release/rtcFrame.node $DIST/webrtc_agent/rtcFrame/build/Release/rtcFrame.node
cp -v $ROOT/source/rtc_frame/src/build/Release/rtcadapter.so $DIST/webrtc_agent/rtcFrame/build/Release/rtcadapter.so
cp -v $ROOT/build/libdeps/build/lib/lib360SCVP.so* $DIST/webrtc_agent/lib
cp -v $ROOT/build/libdeps/build/lib/libsafestring_shared.so* $DIST/webrtc_agent/lib

