#!/bin/sh -e

cd ../build/external

mkdir lttng
cd lttng

#install liburcu library
wget -c https://lttng.org/files/urcu/userspace-rcu-latest-0.11.tar.bz2
tar -xjf userspace-rcu-latest-0.11.tar.bz2
cd userspace-rcu-0.11.*
./configure
make
sudo make install
sudo ldconfig
cd ../

#install uuid and popt libraries
os=$(awk -F= '/^NAME/{print $2}' /etc/os-release)
if [ "$os" == \""Ubuntu"\" ];then
    echo "Ubuntu OS"
    sudo apt-get install uuid-dev -y
    sudo apt-get install libpopt-dev -y
elif [ "$os" == \""CentOS Linux"\" ];then
    echo "CentOS OS"
    sudo yum install uuid.x86_64 -y
    sudo yum install uuid-devel.x86_64 -y
    sudo yum install popt-devel.x86_64 -y
fi

#install numactl
if [ "$os" == \""Ubuntu"\" ];then
    wget -c http://www.rpmfind.net/linux/fedora/linux/releases/30/Everything/x86_64/os/Packages/n/numactl-devel-2.0.12-2.fc30.x86_64.rpm
    wget -c http://www.rpmfind.net/linux/fedora/linux/development/rawhide/Everything/x86_64/os/Packages/n/numactl-libs-2.0.12-4.fc32.x86_64.rpm
    sudo apt-get install alien -y
    sudo alien -i numactl-devel-2.0.12-2.fc30.x86_64.rpm
    sudo alien -i numactl-libs-2.0.12-4.fc32.x86_64.rpm
elif [ "$os" == \""CentOS Linux"\" ];then
    sudo yum install numactl.x86_64 -y
    sudo yum install numactl-devel.x86_64 -y
    sudo yum install numactl-libs.x86_64 -y
fi

#install lttng-ust
wget -c http://lttng.org/files/lttng-ust/lttng-ust-latest-2.11.tar.bz2
tar -xjf lttng-ust-latest-2.11.tar.bz2
cd lttng-ust-2.11.*
./configure --disable-man-pages
make
sudo make install
sudo ldconfig
cd ../

cd ../../../external
