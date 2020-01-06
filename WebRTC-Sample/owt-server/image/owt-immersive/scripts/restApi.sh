#!/bin/bash
SCRIPT=`pwd`/$0

if [ ! -f ${SCRIPT} ]; then
    SCRIPT=$0
fi

ROOT=`dirname $SCRIPT`
DB_URL='localhost/owtdb'

UPDATE=false
ADD=false
CLEARUP=false

WIDTH=0
HEIGHT=0
STREAM_URL=""

usage() {
    echo \
"Usage: $0 [OPTION]
Set server video encoding resolution, publish a video stream, remove video streams.

Options:
    -s [4k|8k]      Set server video encoding resolution 3840x2048 or 7680x3840, default is 3840x2048
    -c url          Connect a FILE|RTSP|RTMP stream to server
    -d              Disconnect all streams from server"

    exit 0
}

parse_arguments(){
    if [ $# == 0 ]; then
        usage $*
    fi

    case $1 in
        "-s")
            if [ $# == 1 ];then
                WIDTH=3840
                HEIGHT=2048
            elif [ $2 == '4k' ];then
                WIDTH=3840
                HEIGHT=2048
            elif [ $2 == '8k' ];then
                WIDTH=7680
                HEIGHT=3840
            else
                echo "Invalid parameter $2, must be 4k or 8k."
                exit 0
            fi

            UPDATE=true
            FILE="updateRoom.js"
            ;;
        "-c")
            if [ $# == 1 ];then
                echo "Stream url is required."
                exit 0
            fi

            echo "Connecting to $2 ..."

            ADD=true
            STREAM_URL=$2
            FILE="add_stream.js"
            ;;
        "-d")
            CLEARUP=true
            FILE="clear_streams.js"
            ;;
        *)
            usage $*
            ;;
    esac
}

install_config() {
    export DB_URL
    option=$1
    [[ -s ${ROOT}/scripts/initdb.js ]] && node ${ROOT}/scripts/initdb.js "${ROOT}/scripts/${option}" || return 1
}

update_room() {
    sed s/width:.*,height:.*/width:${WIDTH},height:${HEIGHT}/g -i ${ROOT}/scripts/config.js
    node ${ROOT}/scripts/updateRoom.js
}

add_stream() {
    sed s$\'url\':\'.*\'$\'url\':\'${STREAM_URL}\'$ -i ${ROOT}/scripts/config.js
    node ${ROOT}/scripts/add_stream.js
}

clear_streams() {
    node ${ROOT}/scripts/clear_streams.js
}

echo "$0 Enter"

parse_arguments $*

install_config ${FILE}

${UPDATE} && update_room
${CLEARUP} && clear_streams
${ADD} && add_stream

echo "$0 Exit"
