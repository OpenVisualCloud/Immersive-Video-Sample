#!/bin/bash -x
OS=$(awk -F= '/^NAME/{print $2}' /etc/os-release)
TARGET=$1
EX_PATH=${PWD}

if [ $# != 1 ] ; then
    echo "Please choose server or client you want to build on."
    exit
fi

if [ "${TARGET}" != "server" ] && [ "${TARGET}" != "client" ] ; then
    echo "Please choose server, client or test you want to build on."
    exit
fi

program_exists() {
    local RET='0'
    command -v $1  >/dev/null 2>&1 || { local RET='1'; }
    # fail on non-zero return value
    if [ ${RET} -ne 0 ]; then
        return 1
    fi

    return 0
}

install_tools() {
    program_exists gcc
    if [ $? != 0 ];then
        if [ $(gcc -dumpversion | awk -F'.' '{print $1}') -ge 6 ];then
    	    if [ "${OS}" == \""Ubuntu"\" ];then
    	    	sudo apt-get install -y software-properties-common
    	    	sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    	    	sudo apt update
    	    	sudo apt install g++-7 -y
    	    	sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 --slave /usr/bin/g++ g++ /usr/bin/g++-7
    	    	sudo update-alternatives --config gcc
    	    elif [ "${OS}" == \""CentOS Linux"\" ];then
                sudo yum install centos-release-scl
                sudo yum install scl-utils
                sudo yum install devtoolset-7-gcc*
    	    	source /opt/rh/devtoolset-7/enable
                scl enable devtoolset-7 bash
    	    fi
        fi
    fi

    if [ "$os" == \""Ubuntu"\" ];then
        sudo apt-get install libgmp-dev libmpfr-dev mpc libmpc-dev patch autoconf libtool automake libssl-dev libevent-dev libcurl4-openssl-dev -y
    elif [ "$os" == \""CentOS Linux"\" ];then
	sudo yum install gmp gmp-devel mpfr mpfr-devel libmpc libmpc-devel patch autoconf libtool automake libssl-devel libevent-devel.x86_64 openssl-devel libxml2-devel -y
    fi

    mkdir -p ../build/external && cd ../build/external
    if [ ! -f "./zlib-1.2.11.tar.gz" ];then
        wget -q http://zlib.net/zlib-1.2.11.tar.gz
    fi
    tar xf zlib-1.2.11.tar.gz
    cd zlib-1.2.11 && ./configure && make -j $(nproc) && sudo make install && cd ..

    program_exists yasm
    if [ $? != 0 ];then
        if [ ! -f "./yasm-1.3.0.tar.gz" ];then
            wget -q http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
        fi
        tar zxf yasm-1.3.0.tar.gz
        cd yasm-1.3.0
        ./configure && make -j $(nproc) && sudo make install && cd ..
    fi

    program_exists cmake
    if [ $? != 0 ];then
        if [ ! -f "./cmake-3.12.4.tar.gz" ];then
            wget -q https://cmake.org/files/v3.12/cmake-3.12.4.tar.gz
        fi
        tar xf cmake-3.12.4.tar.gz
        cd cmake-3.12.4
        ./bootstrap --prefix=/usr && make -j $(nproc) && sudo make install && cd ..
    fi

    if [ ! -f "./curl-7.66.0.tar.xz" ];then
        wget -q https://curl.haxx.se/download/curl-7.66.0.tar.xz
    fi
    tar xf curl-7.66.0.tar.xz
    cd curl-7.66.0 && ./configure --with-darwinssl && make -j $(nproc) && sudo make install
}

install_dependencies() {
    cd ${EX_PATH}
    if [ $1 == "server" ] ; then
        ./build_glog.sh
        ./build_Nokia_omaf.sh
        ./install_openHEVC.sh
        ./install_SVT.sh
        ./install_thrift.sh
        ./install_FFmpeg.sh server
    elif [ $1 == "client" ] ; then
        ./build_glog.sh
        ./build_Nokia_omaf.sh
        ./prebuild_player.sh
        ./install_FFmpeg.sh client
    elif [ $1 == "android" ] ; then
        ./prebuild_android.sh
    fi
}

install_tools
install_dependencies ${TARGET}
