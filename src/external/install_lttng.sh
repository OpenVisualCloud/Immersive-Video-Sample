#!/bin/bash -ex

export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64/:/usr/lib64:$LD_LIBRARY_PATH

mkdir -p ../build/external/lttng
cd ../build/external/lttng
OS=$(awk -F= '/^NAME/{print $2}' /etc/os-release)

# Install liburcu library
if [ ! -f "./userspace-rcu-latest-0.11.tar.bz2" ];then
    wget -c https://lttng.org/files/urcu/userspace-rcu-latest-0.11.tar.bz2
    tar -xjf userspace-rcu-latest-0.11.tar.bz2
fi
cd userspace-rcu-0.11.*
./configure
make -j $(nproc)
sudo make install
sudo ldconfig
cd ../

# Install uuid, popt and other dependencies
if [ "${OS}" == \""Ubuntu"\" ];then
    echo "Ubuntu OS"
    sudo apt-get install uuid-dev -y
    sudo apt-get install libpopt-dev -y
    sudo apt-get install libxml2-dev -y
    sudo apt-get install libdw-dev -y
elif [ "${OS}" == \""CentOS Linux"\" ];then
    echo "CentOS OS"
    sudo yum install uuid.x86_64 -y
    sudo yum install libuuid -y
    sudo yum install libuuid-devel -y
    sudo yum install uuid-devel.x86_64 -y
    sudo yum install popt-devel.x86_64 -y
    sudo yum install glib2-devel -y
    sudo yum install elfutils-devel -y
fi

# Install numactl
if [ "${OS}" == \""Ubuntu"\" ];then
    sudo apt-get install numactl -y
    sudo apt-get install libnuma-dev -y
elif [ "${OS}" == \""CentOS Linux"\" ];then
    sudo yum install numactl.x86_64 -y
    sudo yum install numactl-devel.x86_64 -y
    sudo yum install numactl-libs.x86_64 -y
fi

# Install lttng-ust
if [ ! -f "./lttng-ust-latest-2.11.tar.bz2" ];then
    wget -c http://lttng.org/files/lttng-ust/lttng-ust-latest-2.11.tar.bz2
    tar -xjf lttng-ust-latest-2.11.tar.bz2
fi
cd lttng-ust-2.11.*
./configure --disable-man-pages
make -j $(nproc)
sudo make install
sudo ldconfig
cd ../

# Install lttng-tools
if [ ! -f "./lttng-tools-latest-2.11.tar.bz2" ];then
    wget -c http://lttng.org/files/lttng-tools/lttng-tools-latest-2.11.tar.bz2
    tar -xjf lttng-tools-latest-2.11.tar.bz2
fi
cd lttng-tools-2.11.*
./configure
make -j $(nproc)
sudo make install
sudo ldconfig
cd ../

# Install babeltrace2
if [ ! -f "./babeltrace-2.0.0.tar.bz2" ];then
    wget -c https://www.efficios.com/files/babeltrace/babeltrace-2.0.0.tar.bz2
    tar -xjf babeltrace-2.0.0.tar.bz2
fi
cd babeltrace-2.0.0
./configure
make -j $(nproc)
sudo make install
cd ../
