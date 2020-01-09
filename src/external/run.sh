#!/bin/bash -x

RES=$1
TYPE=$2
IP=$3

parameters_usage(){
    echo 'Usage: 1. <resolution>:          [ 4k , 8k ]'
    echo '       2. <type>:                [ live , static ]'
    echo '       3. <IP>:                  IP of this machine.'
}

if [ "${RES}" = "-h" ] || [ $# != 3 ] ; then
    parameters_usage
    exit 0
fi
if [ "${RES}" != "4k" ] && [ "${RES}" != "8k" ] ; then
    parameters_usage
    exit 0
fi
if [ "${TYPE}" != "live" ] && [ "${TYPE}" != "static" ] ; then
    parameters_usage
    exit 0
fi

if [ ! -d "../build/server/ffmpeg" ]
    echo 'Build server first'
    exit 0
else
    cd ../build/server/ffmpeg
fi

ffmpeg_4k_live(){
    ./ffmpeg -re -stream_loop -1 -i $1 -input_type 1 -rc 1 -c:v:0 distributed_encoder -s:0 3840x1920 -tile_row:0 6 -tile_column:0 10 -config_file:0 config_high.txt  -la_depth:0 0 -r:0 30 -g:0 15 -b:0 30M -map 0:v -c:v:1 distributed_encoder -s:1 1024x640 -tile_row:1 2 -tile_column:1 4 -config_file:1 config_low.txt  -la_depth:1 0 -r:1 30 -g:1 15 -b:1 5M -map 0:v -vframes 10000 -f omaf_packing -is_live 1 -split_tile 1 -seg_duration 1 -window_size 20 -extra_window_size 30 -base_url https://$2:443/live4k/ -out_name Test /usr/local/nginx/html/live4k/
}

ffmpeg_4k_static(){
    numactl -c 1 ./ffmpeg -stream_loop -1 -i $1 -input_type 1 -rc 1 -r 30 -c:v:0 distributed_encoder -s:0 3840x1920 -g:0 15 -tile_row:0 6 -tile_column:0 10 -la_depth:0 0 -config_file:0 config_high.txt -b:0 30M -map 0:v -c:v:1 distributed_encoder -s:1 1024x640 -sws_flags neighbor -g:1 15 -tile_row:1 1 -tile_column:1 4 -la_depth:1 0 -config_file:1 config_low.txt -b:1 2M -map 0:v -vframes 500 -f omaf_packing -is_live 0 -split_tile 1 -seg_duration 1 -base_url https://$2:443/static4k/ -out_name Test /usr/local/nginx/html/static4k/
}

ffmpeg_8k_live(){
    numactl -c 1 ./ffmpeg -re -stream_loop -1 -i $1 -input_type 1 -rc 1 -c:v:0 distributed_encoder -s:0 7680x3840 -g:0 25 -tile_row:0 6 -tile_column:0 12 -config_file:0 config_high.txt -b:0 50M -map 0:v -c:v:1 distributed_encoder -s:1 1280x1280 -sws_flags neighbor -g:1 25 -tile_row:1 2 -tile_column:1 2 -config_file:1 config_low.txt -b:1 2M -map 0:v -vframes 1000000 -f omaf_packing -is_live 1 -split_tile 1 -seg_duration 1 -extractors_per_thread 5 -base_url https://$2:443/live8k/ -out_name Test /usr/local/nginx/html/live8k/
}

ffmpeg_8k_static(){
    ./ffmpeg -re -stream_loop -1 -f rawvideo -video_size 3840x1920 -i $1 -input_type 1 -rc 1 -vframes 1000 -c:v:0 distributed_encoder -s:0 3840x1920 -tile_row:0 6 -tile_column:0 10 -config_file:0 config_high.txt -la_depth:0 0 -r:0 30 -g:0 15 -b:0 30M -map 0:v -c:v:1 distributed_encoder -s:1 1024x640 -tile_row:1 2 -tile_column:1 4 -config_file:1 config_low.txt -la_depth:1 0 -r:1 30 -g:1 15 -b:1 0.5M -map 0:v -f omaf_packing -is_live 0 -split_tile 1 -seg_duration 1 -base_url https://$2:443/static8k/ -out_name Test /usr/local/nginx/html/static8k/
}

export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64:$LD_LIBRARY_PATH

if [ "${RES}" = "4k" ] ; then

    VIDEO="../../../Sample-Videos/test1_h265_3840x2048_30fps_30M_200frames.mp4"
    echo "ip 127.0.0.1 port 9089" > config_low.txt
    echo "ip 127.0.0.1 port 9090" > config_high.txt

    if [ "${TYPE}" = "live" ] ; then
        ffmpeg_4k_live ${VIDEO} ${IP}
    else
        ffmpeg_4k_static ${VIDEO} ${IP}
    fi
else

    VIDEO="../../../Sample-Videos/test1_h265_8k_25fps_60M_100frames.mp4"
    echo "ip 127.0.0.1 port 9089 numa 1" > config_low.txt
    echo "ip 127.0.0.1 port 9090 numa 2" > config_high.txt

    if [ "${TYPE}" = "live" ] ; then
        ffmpeg_8k_live ${VIDEO} ${IP}
    else
        ffmpeg_8k_static ${VIDEO} ${IP}
    fi
fi
