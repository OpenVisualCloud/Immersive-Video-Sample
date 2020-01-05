#!/bin/bash -e

IMAGE="ovc_self_certificate"
DIR=$(dirname $(readlink -f "$0"))

case "$(cat /proc/1/sched | head -n 1)" in
*self-sign*)
    openssl req -x509 -nodes -days 30 -newkey rsa:4096 -keyout /home/self.key -out /home/self.crt << EOL
CN
SH
Shanghai
Zizhu
Data Center Group
Intel Corporation
$1
nobody@intel.com
EOL
    chmod 640 "/home/self.key"
    chmod 644 "/home/self.crt"
    openssl dhparam -dsaparam -out /home/dhparam.pem 4096
    chmod 644 "/home/dhparam.pem"
    ;;
*)
    OPTIONS=("--volume=${DIR}:/home:rw")
    . "$DIR/../script/shell.sh" /home/self-sign.sh $(hostname -f)
    ;;
esac
