#!/bin/bash -ex

ORIPATH=${PWD}
OS=$(awk -F= '/^NAME/{print $2}' /etc/os-release)

# INSTALL DEPENDENCIES
if [ "${OS}" == \""Ubuntu"\" ];then
    sudo apt-get install lsb-core libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libgles2-mesa-dev libglm-dev libegl1-mesa-dev pkg-config libglfw3-dev liblzma-dev -y
elif [ "${OS}" == \""CentOS Linux"\" ];then
    wget https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/e/epel-release-7-12.noarch.rpm
    rpm -Uvh epel-release*rpm || true
    sudo yum install -y redhat-lsb libXrandr libXrandr-devel libXinerama libXinerama-devel libXcursor libXcursor-devel libXi libXi-devel mesa-libGL mesa-libGL-devel mesa-libGLU mesa-libGLU-devel mesa-libGLES-devel glm-devel mesa-libEGL-devel SDL2 SDL2-devel libcurl4-openssl-dev glfw glfw-devel xz-devel pkg-config lzma
fi

check_mkdir()
{
    if [ ! -d "$1" ];then
        mkdir -p $1
    fi
    cd $1
}

# INSTALL LIBVA
check_mkdir ${ORIPATH}/../build/external/
if [ ! -d "./libva" ];then
    git clone https://github.com/intel/libva.git
fi
cd libva
./autogen.sh
make -j $(nproc)
sudo make install
