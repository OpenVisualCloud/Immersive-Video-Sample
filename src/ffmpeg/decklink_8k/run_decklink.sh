#!/bin/bash -ex

export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH
FFMPEG="./ffmpeg"
FRAME_NUM=60
OUTPUT_MP4="tmp.mp4"
OUTPUT_BIN="tmp.yuv"
QUEUE_SIZE="100000000000"
          # 100,000,000,000

# ${FFMPEG} -sources decklink
# ${FFMPEG} -f decklink -list_devices 1 -i dummy
# ${FFMPEG} -f decklink -list_formats 1 -i "DeckLink 8K Pro (1)"

if [ $1 == "null" ]; then
    OUTOPT="-c:v copy -f null /dev/null"
elif [ $1 == "copy" ]; then
    OUTOPT="-c:v copy -f ${OUTPUT_BIN}"
elif [ $1 == "rawvideo" ]; then
    OUTOPT="-c:v rawvideo ${OUTPUT_BIN}"
elif [ $1 == "264" ]; then
    OUTOPT="-c:v libx264 -b:v 20M ${OUTPUT_MP4}"
elif [ $1 == "de" ]; then
    OUTOPT="-input_type 1 -rc 1 "`
          `"-c:v distributed_encoder "`
          `"     -s 7680x3840 -sws_flags neighbor "`
          `"     -tile_row 6 -tile_column 12 "`
          `"     -config_file config.xml "`
          `"     -la_depth 0 -r 30 -g 30 -b:v 20M "`
          `"-y ${OUTPUT_MP4}"
else
    echo "please choose mode"
    exit 1
fi


${FFMPEG} -f decklink -video_input sdi \
                      -format_code bmdMode8K4320p30 \
                      -duplex_mode full \
                      -raw_format uyvy422 \
                      -queue_size ${QUEUE_SIZE} \
          -i 'DeckLink 8K Pro (1)' \
          -pix_fmt yuv420p \
          -vframes ${FRAME_NUM} \
          ${OUTOPT}
