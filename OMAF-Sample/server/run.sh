#!/bin/bash -x

RES=${1:-"4K"}
TYPE=${2:-"LIVE"}
PROTOCOL=${3:-"HTTP"}
IP=$(cat /etc/hosts | tail -n 1 | awk '{print $1}')

parameters_usage(){
    echo 'Usage: 1. <resolution>:          [ 4K , 8K ]'
    echo '       2. <type>:                [ LIVE , VOD ]'
    echo '       3. <protocol>:            [ HTTP , HTTPS ]'
}

pkill -f WorkerServer

if [ "${RES}" = "-h" ] ; then
    parameters_usage
    exit 0
fi
if [ "${RES}" != "4K" ] && [ "${RES}" != "8K" ] ; then
    parameters_usage
    exit 0
fi
if [ "${TYPE}" != "LIVE" ] && [ "${TYPE}" != "VOD" ] ; then
    parameters_usage
    exit 0
fi

if [ "${PROTOCOL}" = "HTTPS" ] ; then
    URLBASE="https://$2:443/LIVE4K"
elif [ "${PROTOCOL}" = "HTTP" ] ; then
    URLBASE="http://$2:8080/LIVE4K"
fi

ffmpeg_4K_LIVE(){
    ./ffmpeg -stream_loop -1 \
        -i $1 -input_type 1 -rc 1 \
        -c:v:0 distributed_encoder \
        -s:0 3840x1920 \
        -tile_row:0 6 -tile_column:0 12 \
        -config_file:0 config_high.txt \
        -la_depth:0 0 -r:0 30 -g:0 15 \
        -b:0 30M -map 0:v \
        -c:v:1 distributed_encoder \
        -s:1 1024x640 \
        -tile_row:1 2 -tile_column:1 2 \
        -config_file:1 config_low.txt \
        -la_depth:1 0 -r:1 30 -g:1 15 \
        -b:1 5M -map 0:v \
        -f omaf_packing \
        -is_live 1 -split_tile 1 -seg_duration 1 \
        -window_size 20 -extra_window_size 30 \
        -base_url ${URLBASE}/LIVE4K/ \
        -out_name Test /usr/local/nginx/html/LIVE4K/
}

ffmpeg_4K_VOD(){
    ./ffmpeg -stream_loop -1 \
        -i $1 -input_type 1 -rc 1 \
        -c:v:0 distributed_encoder \
        -s:0 3840x1920 \
        -tile_row:0 6 -tile_column:0 12 \
        -config_file:0 config_high.txt \
        -la_depth:0 0 -r:0 30 -g:0 15 \
        -b:0 30M -map 0:v \
        -c:v:1 distributed_encoder \
        -s:1 1024x640 -sws_flags neighbor \
        -tile_row:1 2 -tile_column:1 2 \
        -config_file:1 config_low.txt \
        -la_depth:1 0 -r:1 30 -g:1 15 \
        -b:1 2M -map 0:v -vframes 500 \
        -f omaf_packing \
        -is_live 0 -split_tile 1 -seg_duration 1 \
        -base_url ${URLBASE}/LIVE4K/ \
        -out_name Test /usr/local/nginx/html/VOD4K/
}

ffmpeg_8K_LIVE(){
    numactl -c 1 ./ffmpeg -stream_loop -1 \
        -i $1 -input_type 1 -rc 1 \
        -c:v:0 distributed_encoder \
        -s:0 7680x3840 \
        -tile_row:0 6 -tile_column:0 12 \
        -config_file:0 config_high.txt \
        -la_depth:0 0 -r:0 25 -g:0 25 \
        -b:0 50M -map 0:v \
        -c:v:1 distributed_encoder \
        -s:1 1280x1280 -sws_flags neighbor \
        -tile_row:1 2 -tile_column:1 2 \
        -config_file:1 config_low.txt \
        -la_depth:1 0 -r:1 25 -g:1 25 \
        -b:1 2M -map 0:v \
        -f omaf_packing \
        -is_live 1 -split_tile 1 -seg_duration 1 \
        -extractors_per_thread 4 \
        -base_url ${URLBASE}/LIVE4K/ \
        -out_name Test /usr/local/nginx/html/LIVE8K/
}

ffmpeg_8K_VOD(){
    numactl -c 1 ./ffmpeg -stream_loop -1 \
        -i $1 -input_type 1 -rc 1 \
        -c:v:0 distributed_encoder \
        -s:0 7680x3840 \
        -tile_row:0 6 -tile_column:0 12 \
        -config_file:0 config_high.txt \
        -la_depth:0 0 -r:0 25 -g:0 25 \
        -b:0 50M -map 0:v \
        -c:v:1 distributed_encoder \
        -s:1 1280x1280 -sws_flags neighbor \
        -tile_row:1 2 -tile_column:1 2 \
        -config_file:1 config_low.txt \
        -la_depth:1 0 -r:1 25 -g:1 25 \
        -b:1 2M -map 0:v -vframes 500 \
        -f omaf_packing \
        -is_live 0 -split_tile 1 -seg_duration 1 \
        -base_url ${URLBASE}/LIVE4K/ \
        -out_name Test /usr/local/nginx/html/VOD8K/
}

export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64:$LD_LIBRARY_PATH

if [ "${RES}" = "4K" ] ; then

    VIDEO="test1_h265_3840x2048_30fps_30M_200frames.mp4"
    echo "ip local port 9090" > config_high.txt
    echo "ip local port 9089" > config_low.txt

    if [ "${TYPE}" = "LIVE" ] ; then
        ffmpeg_4K_LIVE ${VIDEO} ${IP}
    else
        ffmpeg_4K_VOD ${VIDEO} ${IP}
    fi

else

    VIDEO="test1_h264_8k_25fps_30M_250frames.mp4"
    echo "ip local port 9090 numa 1" > config_high.txt
    echo "ip local port 9089 numa 2" > config_low.txt

    if [ "${TYPE}" = "LIVE" ] ; then
        ffmpeg_8K_LIVE ${VIDEO} ${IP}
    else
        ffmpeg_8K_VOD ${VIDEO} ${IP}
    fi

fi
