#!/bin/bash +ex

export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64/:$LD_LIBRARY_PATH

kill -9 $(pidof lttng-sessiond)
ps -aux | grep lttng-sessiond

lttng-sessiond --daemonize
lttng create E2E_latency_trace
if [ $1 == "server" ] ; then
    lttng enable-event --userspace E2E_latency_tp_provider:pre_de_info
    lttng enable-event --userspace E2E_latency_tp_provider:post_de_info
    lttng enable-event --userspace E2E_latency_tp_provider:pre_op_info
    lttng enable-event --userspace E2E_latency_tp_provider:post_op_info
    lttng start

    VFRAMES=5000000000
    numactl -c 1 ./ffmpeg -re -stream_loop -1 -thread_count 16 -thread_type 4 -c:v libopenhevc \
        -i /path/to/xxxx.mp4 \
        -input_type 1 -rc 1 -vframes ${VFRAMES} \
        -c:v:0 distributed_encoder -s:0 7680x3840 -tile_row:0 6 -tile_column:0 10 \
        -config_file:0 config_high.txt -la_depth:0 30 -g:0 15 -b:0 30M -map 0:v \
        -c:v:1 distributed_encoder -s:1 640x640 -sws_flags neighbor -tile_row:1 2 -tile_column:1 2 \
        -config_file:1 config_low.txt -la_depth:1 30 -g:1 15 -b:1 1M -map 0:v \
        -f omaf_packing -is_live 1 -split_tile 1 -seg_duration 1 -has_extractor 0 \
        -window_size 20 -extra_window_size 30 -need_external_log 1 -min_log_level 0 \
        -base_url http://0.0.0.0:8080/temp/ -out_name Test /usr/local/nginx/html/temp/ || stty sane
elif [ $1 == "client" ] ; then
    lttng enable-event --userspace E2E_latency_tp_provider:da_ssi_info
    lttng enable-event --userspace E2E_latency_tp_provider:pre_da_info
    lttng enable-event --userspace E2E_latency_tp_provider:post_da_info
    lttng enable-event --userspace E2E_latency_tp_provider:pre_rd_info
    lttng enable-event --userspace E2E_latency_tp_provider:post_rd_info
    lttng start

    ./render
else
    echo "Choose server or client to start trace"
    exit 1
fi

echo "------------------------------"

lttng destroy

INPUT_PATH=`ls -td /root/lttng-traces/* | head -1`
OUTPUT_NAME=`echo "${INPUT_PATH}" | cut -d/ -f 4 | cut -d- -f 3`
if [ -d ${INPUT_PATH}/ust ] ; then
    sleep 1s && babeltrace2 ${INPUT_PATH}/ust > ${OUTPUT_NAME}_trace.log
else
    echo -e "\n\nNo trace log found !!"
fi
