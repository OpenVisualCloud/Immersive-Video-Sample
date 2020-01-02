#!/bin/sh

#example:
# better run as ROOT user at main encoder machine!

# press [enter] key three times if you don’t want to store key in a specific folder or need a passphrase
ssh-keygen -t rsa

# copy ssh key to worker with ip:
ssh-copy-id root@10.67.xxx.xxx

