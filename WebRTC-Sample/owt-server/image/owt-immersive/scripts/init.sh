#!/bin/bash -e

mongod --config /etc/mongod.conf &
rabbitmq-server &
while ! mongo --quiet --eval "db.adminCommand('listDatabases')"
do
  sleep 1
done
echo mongodb connected successfully
cd /home/owt
./management_api/init.sh
