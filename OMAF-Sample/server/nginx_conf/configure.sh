#!/bin/bash -e

COUNTRY_NAME=$1
STATE_OR_PROVIENCE_NAME=$2
LOCALITY_NAME=$3
DEFAULT_CITY=$4
ORGANIZATION_NAME=$5
DEFAULT_COMPANY_LTD=$6
COMMON_NAME=$7
EMAIL_ADDRESS=$8

parameters_usage(){
    echo 'Usage: 1. <COUNTRY_NAME>:              A 2 letter code. eg. [CN].'
    echo '       2. <STATE_OR_PROVIENCE_NAME>:   Full name. eg. [Shanghai].'
    echo '       3. <DEFAULT_CITY>:              eg. [Somewhere].'
    echo '       4. <ORGANIZATION_NAME>:         eg. [Somegroup].'
    echo '       5. <DEFAULT_COMPANY_LTD>:       eg. [Default.Ltd]'
    echo '       6. <COMMON_NAME>:               eg. [Nobody].'
    echo '       7. <EMAIL_ADDRESS>:             eg. [Nobody@email.com].'
}

if [ "${ITEM}" = "-h" ] || [ $# != 7 ] ; then
    parameters_usage
    exit 0
fi

cd /usr/local/nginx/conf
mkdir -p ssl
cd ssl

openssl req -x509 -nodes -days 30 -newkey rsa:4096 -keyout ./self.key -out ./self.crt << EOL
${COUNTRY_NAME}
${STATE_OR_PROVIENCE_NAME}
${LOCALITY_NAME}
${DEFAULT_CITY}
${ORGANIZATION_NAME}
${DEFAULT_COMPANY_LTD}
${COMMON_NAME}
${EMAIL_ADDRESS}
EOL

chmod 640 self.key
chmod 644 self.crt

openssl dhparam -dsaparam -out ./dhparam.pem 4096

chmod 644 dhparam.pem

openssl rsa -noout -text -in self.key
openssl x509 -noout -text -in self.crt
