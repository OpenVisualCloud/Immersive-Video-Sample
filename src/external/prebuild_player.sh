#!/bin/bash
path=${PWD}
os=$(awk -F= '/^NAME/{print $2}' /etc/os-release)
if [ "$os" == \""Ubuntu"\" ];then
	sudo apt-get install lsb-core libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libgles2-mesa-dev libglm-dev libegl1-mesa-dev -y
elif [ "$os" == \""CentOS Linux"\" ];then
	sudo yum install -y redhat-lsb libXrandr libXrandr-devel libXinerama libXinerama-devel libXcursor libXcursor-devel libXi libXi-devel mesa-libGL mesa-libGL-devel mesa-libGLU mesa-libGLU-devel mesa-libGLES-devel glm-devel mesa-libEGL-devel SDL2 SDL2-devel
fi
check_mkdir()
{
	if [ ! -d "$1" ];then
		mkdir -p $1 
	fi
	cd $1
}
###INSTALL LIBVA
intel_curl() {
    curl -O -k -x '' -H 'X-JFrog-Art-Api: AKCp5dL3Kxmp2PhDfYhT2oFk4SDxJji5H8S38oAqmMSkiD46Ho8uCA282aJJhM9ZqCKLb64bw' "$@"
}
check_mkdir $path/../build/external/
intel_curl  https://ubit-artifactory-sh.intel.com/artifactory/DCG_Media_Driver-local/PV5-Build-After-Branch-Out/build_prod_mss_kblg/10013/MediaServerStudioEssentialsKBL2019R1HF1_10010.tar.gz
tar xzf MediaServerStudioEssentialsKBL2019R1HF1_10010.tar.gz
cd MediaServerStudioEssentialsKBL2019R1HF1_10010
tar xzf intel-linux-media-kbl-10010.tar.gz
cd intel-linux-media-kbl-10010 
if [ "$os" == \""CentOS Linux"\" ];then
    sed -i '108s/lib\/x86_64-linux-gnu/usr\/lib64/' ./install_media.sh
fi 
( echo 'n' ) | sudo ./install_media.sh
cd opt/intel/mediasdk/opensource/libva/2.3.0-10010
tar xjf libva-2.3.0.tar.bz2
cd libva-2.3.0
./autogen.sh
make -j
sudo make install

###INSTALL GLFW
check_mkdir $path/../build/external/
if [ ! -d "./glfw" ];then
        git clone https://github.com/glfw/glfw
fi
cd ./glfw 
if [ ! -d "./build" ];then
	mkdir build
fi  
cd build && cmake .. && make -j8 && sudo make install

cd ${path} && ./install_FFmpeg.sh client

if [ "$os" == \""CentOS Linux"\" ];then
    sed -i '15s/v2/v2\ lzma/' ${path}/../player/CMakeLists.txt
fi 
