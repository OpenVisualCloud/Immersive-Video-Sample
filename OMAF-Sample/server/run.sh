#!/bin/bash -x

/usr/local/nginx/sbin/nginx

echo "ip 127.0.0.1 port 9089" > config_low.txt
echo "ip 127.0.0.1 port 9090" > config_high.txt
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64:$LD_LIBRARY_PATH

./ffmpeg -re -stream_loop -1 -i ./test1_h265_3840x2048_30fps_30M_200frames.mp4 -input_type 1 -rc 1 -c:v:0 distributed_encoder -s:0 3840x1920 -tile_row:0 6 -tile_column:0 10 -config_file:0 config_high.txt  -la_depth:0 0 -r:0 30 -g:0 15 -b:0 30M -map 0:v -c:v:1 distributed_encoder -s:1 1024x640 -tile_row:1 2 -tile_column:1 4 -config_file:1 config_low.txt  -la_depth:1 0 -r:1 30 -g:1 15 -b:1 5M -map 0:v -vframes 10000 -f omaf_packing -is_live 1 -split_tile 1 -seg_duration 1 -window_size 20 -extra_window_size 30 -base_url https://10.67.119.113:443/live4k/ -out_name Test /usr/local/nginx/html/live4k/  

