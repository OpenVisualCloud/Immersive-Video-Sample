#!/bin/bash
/usr/local/nginx/sbin/nginx
cd ../build/server/ffmpeg
echo "ip 127.0.0.1 port 9089" > config_low.txt
echo "ip 127.0.0.1 port 9090" > config_high.txt
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64:$LD_LIBRARY_PATH
