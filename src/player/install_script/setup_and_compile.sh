#!/bin/bash -ex
# ********************************************************
# @file: run.sh
# @author:
# @create time: 2019-09-17 15:47:13
# @last modified: 2019-09-17 17:25:29
# @description:
# ********************************************************

(( ${EUID} != 0 )) && exec sudo -E -- "$0" "$@"

WS=$(cd `dirname $0`;pwd)
DL_PATH=${WS}/downloads
GIT_ROOT_PATH=${WS}/../..
SSH_KEY=${WS}/.id_dsa

apt_ins_deps() {
    apt update
    apt install -y \
        gcc g++ make git curl lsb-release \
        libxml2-dev libcurl4-openssl-dev \
        yasm \
        zlib1g-dev \
        libgl1-mesa-dev libglu1-mesa-dev libglfw3-dev libx11-dev libglm-dev libgles2-mesa-dev \
        lzma liblzma-dev \
        autoconf automake libtool
}

intel_curl() {
    curl -x '' -H 'X-JFrog-Art-Api: AKCp5dL3Kxmp2PhDfYhT2oFk4SDxJji5H8S38oAqmMSkiD46Ho8uCA282aJJhM9ZqCKLb64bw' "$@"
}

clean_dir() {
    local d=${1}
    if [ -e ${d} ]; then
        rm -rf ${d}
    fi
    mkdir -p ${d}
}

ins_cmake() {
    cd ${DL_PATH}
    curl -O https://cmake.org/files/v3.12/cmake-3.12.4.tar.gz
    tar xzvf cmake-3.12.4.tar.gz
    cd cmake-3.12.4
    ./bootstrap
    make -j$(nproc)
    make install
    cd ${WS}
}

ins_glog() {
    cd ${DL_PATH}
    git clone https://github.com/google/glog
    cd glog
    ./autogen.sh
    ./configure
    make -j$(nproc)
    make install
    make install DESTDIR=${DEB_PATH}
    cd ${WS}
}

ins_ffmpeg() {
    cd ${DL_PATH}
    curl -O http://ffmpeg.org/releases/ffmpeg-4.1.3.tar.bz2
    tar xjvf ffmpeg-4.1.3.tar.bz2
    cd ffmpeg-4.1.3
    patch -p1 < ${GIT_ROOT_PATH}/external/0001-Add-avcodec_receive_frame2-for-vaapi-hardware-decodi.patch
    ./configure --prefix=/usr --libdir=/usr/local/lib --enable-static --disable-shared --enable-pthreads --enable-postproc --enable-gpl
    # ./configure --prefix=/usr --libdir=/usr/local/lib --disable-static --enable-shared --enable-pthreads --enable-postproc --enable-gpl
    make -j$(nproc)
    make install
    make install DESTDIR=${DEB_PATH}
    cd ${WS}
}

ins_libva() {
    cd ${DL_PATH}
    intel_curl -O https://ubit-artifactory-sh.intel.com/artifactory/DCG_Media_Driver-local/PV5-Build-After-Branch-Out/build_prod_mss_kblg/10013/MediaServerStudioEssentialsKBL2019R1HF1_10010.tar.gz
    tar xzvf MediaServerStudioEssentialsKBL2019R1HF1_10010.tar.gz
    cd MediaServerStudioEssentialsKBL2019R1HF1_10010
    tar xzvf intel-linux-media-kbl-10010.tar.gz
    cd intel-linux-media-kbl-10010
    ( echo 'n' ) | ./install_media.sh
    cp -a etc ${DEB_PATH}
    cp -a opt ${DEB_PATH}
    cd opt/intel/mediasdk/opensource/libva/2.3.0-10010
    tar xjvf libva-2.3.0.tar.bz2
    cd libva-2.3.0
    ./autogen.sh
    make -j$(nproc)
    make install
    make install DESTDIR=${DEB_PATH}
    cd ${WS}
}

ins_libdash() {
    cd ${DL_PATH}
    git clone https://github.com/bitmovin/libdash.git
    ln -sf ${PWD}/libdash/libdash/libdash ${GIT_ROOT_PATH}/OmafDashAccess/libdash

    cd libdash
    patch -p1 < ${GIT_ROOT_PATH}/external/libdash-cmake.diff
    cd libdash
    mkdir build
    cd build
    sed -i '2 a ADD_DEFINITIONS("-std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0")' ../CMakeLists.txt
    cmake ..
    make -j$(nproc)
    cp -dr bin/libdash.so /usr/local/lib/

    mkdir -p ${DEB_PATH}/usr/local/lib/
    cp -dr bin/libdash.so ${DEB_PATH}/usr/local/lib/

# rm ../OmafDashAccess/libdash
# ln -s ${PWD}/libdash/libdash/libdash ${PWD}/../OmafDashAccess/libdash
# cd ${PWD}/../../OmafDashAccess/libdash
# mkdir build
# cd build
# sed -i '2 a ADD_DEFINITIONS("-std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0")' ../CMakeLists.txt
# cmake ..
# make -j8
# cp -dr libdash.so /usr/local/lib/


    cd ${WS}
}

ins_isolib() {
    cd ${DL_PATH}
    git clone https://github.com/MPEGGroup/isobmff.git
# rm ../OmafDashAccess/isobmff
# ln -s ${PWD}/isobmff/IsoLib/libisomediafile ${PWD}/../OmafDashAccess/libisomediafile
    ln -sf ${PWD}/isobmff/IsoLib/libisomediafile ${GIT_ROOT_PATH}/OmafDashAccess/libisomediafile
    cd isobmff/IsoLib/libisomediafile
    clean_dir build
    cd build
    cmake ..
    make -j$(nproc)
    make install DESTDIR=${DEB_PATH}

    cd ${WS}
}

ins_nokia_omaf() {
    cd ${DL_PATH}

    git clone https://github.com/nokiatech/omaf.git
    ln -sf ${PWD}/omaf/Mp4/srcs ${GIT_ROOT_PATH}/OmafDashAccess/mp4lib
    cd omaf
    patch -p1 < ${GIT_ROOT_PATH}/external/nokia_omaf_patch_for_extrator_reader.diff
    cd Mp4/srcs
    clean_dir build
    cd build
    cmake ..
    make -j$(nproc)

    cp -r ../api/streamsegmenter ${GIT_ROOT_PATH}/VROmafPacking/
    cp lib/libstreamsegmenter_static_fpic.a /usr/local/lib/
    cp lib/libstreamsegmenter_static.a /usr/local/lib/
    cp lib/libmp4vr_static_fpic.a /usr/local/lib/
    cp lib/libmp4vr_static.a /usr/local/lib/

    cp lib/libstreamsegmenter_static_fpic.a ${DEB_PATH}/usr/local/lib/
    cp lib/libstreamsegmenter_static.a ${DEB_PATH}/usr/local/lib/
    cp lib/libmp4vr_static_fpic.a ${DEB_PATH}/usr/local/lib/
    cp lib/libmp4vr_static.a ${DEB_PATH}/usr/local/lib/
    cd ${WS}
}

ins_360scvp() {
    cd ${GIT_ROOT_PATH}/360SCVP
    clean_dir build
    cd build
    cmake ..
    make -j$(nproc)
    make install
    make install DESTDIR=${DEB_PATH}
    cd ${WS}
}

ins_omafdashaccess() {
    cd ${GIT_ROOT_PATH}/OmafDashAccess
    clean_dir build
    cd build
    cmake ..
    make -j$(nproc)
    make install
    make install DESTDIR=${DEB_PATH}
    cd ${WS}
}

ins_gpac() {
    cd ${GIT_ROOT_PATH}/gpac
    ./configure --disable-crypt --disable-ssl --disable-x11 --use-png=no --use-jpeg=no
    make -j$(nproc)

    sed -i '$d' gpac.pc
    sed -i '$d' gpac.pc
    sed -i '$a\Libs: -L${libdir} -lgpac -lm -L/usr/local/lib -lz -lpthread -ldl' gpac.pc

    sudo cp bin/gcc/libgpac_static.a /usr/local/lib/libgpac.a
    sudo cp gpac.pc /usr/local/lib/pkgconfig/
    sudo cp -r include/gpac/ /usr/local/include/
    sudo cp config.h /usr/local/include/gpac/

    cd ${WS}
}

ins_f() {
    cd ${GIT_ROOT_PATH}/genViewport
cd ../genViewport
./compile.sh
sudo cp libgenviewport.so /usr/local/lib/
sudo cp genViewportAPI.h /usr/local/include/
    cd ${WS}
}

ins_enc() {
    cd ${DL_PATH}
    intel_git clone ssh://sys_mcia@git-ccr-1.devtools.intel.com:29418/vcd_immersive-dist_enc
    cd vcd_immersive-dist_enc
    mkdir -p build
    cd build
    cmake ..
    make install
    make install DESTDIR=${DEB_PATH}
    cd ${WS}
}


ins_player() {
    cd ${GIT_ROOT_PATH}/player
    ./compile.sh
    cp render ${OUT_PATH}
cat > ${OUT_PATH}/config.xml  << EOF
<?xml version="1.0"?>
<info>
    <windowWidth>960</windowWidth>
    <windowHeight>960</windowHeight>
    <sourceType>1</sourceType>
    <decoderType>0</decoderType>
    <contextType>0</contextType>
    <useDMABuffer>0</useDMABuffer>
    <viewportHFOV>80</viewportHFOV>
    <viewportVFOV>80</viewportVFOV>
    <viewportWidth>960</viewportWidth>
    <viewportHeight>960</viewportHeight>
    <cachePath>/home/media/cache</cachePath>
    <!-- <url>http://10.67.119.41:8080/testOMAFstatic/Test.mpd</url> -->
    <url>packet_MultiBS.265</url>
</info>
EOF
    cp -a test/packet_MultiBS.265 ${OUT_PATH}
    cp -a *.vs ${OUT_PATH}
    cp -a *.fs ${OUT_PATH}
    # grep -v 'url' config.xml > ${OUT_PATH}/config.xml
    # ./render
    # echo "cd ${PWD}"
    # echo "./render"
    cd ${WS}
}

initialize() {
    mkdir -p ${DL_PATH}

    export http_proxy=http://child-prc.intel.com:913
    export https_proxy=http://child-prc.intel.com:913

    local known=~/.ssh/known_hosts
    if [ ! -e ~/.ssh ]; then
        mkdir -p ~/.ssh
        chmod 700 ~/.ssh
        touch ${known}
        chmod 644 ${known}
    fi

    # if ( ! grep -q 'NZMwgpayhxQDINEzFCrcyVxBXFU' ${known} ); then
    # cat << EOF >> ~/.ssh/known_hosts
# |1|NZMwgpayhxQDINEzFCrcyVxBXFU=|qMKrrnMwfuQI6XCiXwGgCqOLaCE= ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBJDoVurTBT1CQ3Pr/4z8nGi7BRxQergRJBs4bbb0Gcs5Q5U6bXfBlJ2fnWy7ie5sVtXsLFweDy3JxwQno6mjOLs=
# EOF
    # fi

    # if ( ! grep -q 'FMfPWCnmcj7w8FcnFfzIwSk2KL4' ${known} ); then
    # cat << EOF >> ~/.ssh/known_hosts
# |1|FMfPWCnmcj7w8FcnFfzIwSk2KL4=|yS2Fu77WridNrhmnc0VidF7giIw= ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBJDoVurTBT1CQ3Pr/4z8nGi7BRxQergRJBs4bbb0Gcs5Q5U6bXfBlJ2fnWy7ie5sVtXsLFweDy3JxwQno6mjOLs=
# EOF
    # fi

    cat << EOF > ${SSH_KEY}
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAtx6ygPf+phJCWC6m37ztT/59nPyvLc0MP+2RJZCke5wsxvch
GtqYsE6FCfKyjciraQbdodtJzJ0gX3GgsO25lUUjAX+KHrK6PSxhbaN7GUzJDbrt
tZUNtEoAIq1Z4vuwTKPKCIIDdXBspq2xRd9eHQiRtjqtACbZysynu6R76LaBZT2m
/duIT/Xwxi4/8acidF7HUhgqDxap/ZjxHWtOTSP0jCFGWzEFuFaxaewghcog0zxU
f1rhhpqpH1GtcJaOrTrycKP2gLbEFTH00h2QXiVb92bQBXGpaczh1bqMFLE6s4Xb
OeZfEgRcOlSQ630PWzxPzh+cczIv+EvzNHqizwIDAQABAoIBAGK9UK8uCcdujQuT
jDOUYboClaTys1rU98f4x40j4y0r1xEp7FXYjQB0NlRY42KEU7j8FMHTxoVoLC6K
zITainksoGWMaeOHp9iYd6hwwE2yE68tnkONLaiLRmtsYc18vWF159iSkl7e0hdG
DRKKEOtC1Z8XIHW2fTVXCRKq7I1cAgRtsnQrrWXYJxBp5SFsjBmHLbUTWpGVa6Wg
6oGkRJjQaTynz2ysgvek/AONkNITNTNaS/U8OTF6vj6YhqsiYGdxAg/Rq0/A2AGs
q0PqyilyI3nOmyWUp6M6LUkA0B+lhrIcuAMM1DnApSk/MBjuTxqFkU43lkegFr/5
Kia2ILECgYEA2l330ktg9CtZ7l3XkNGSgntbALHOQEp6aUjG9+TI4h7rCV2B7d0I
DUfNnSPB8ZXbsZMsPMQq3VBlXiv/lqzJd35Py/w/kJkUw4VrBxv4lWXKtcZsKIox
dW7u4zDdqNJv5QCoX+Eg7LQOqZYbQMgnbzd2g+wxUD17CvMAkkfTao0CgYEA1q2t
USBuqFuvHQTlYtj1Rl/U2kkPMXcu3nc123jT/I1Y95sMRVIhJjGRcQ42cCOGeCCy
1ZCeW+J+QncEVWqO1g7fuOZkDn+7yzI4DJUjj1UM4cTg8cckz4fuFn+NyTgovrrE
BjyVsoh2kVjsf7t9ya/qoLjvnNDfKNhhV656+csCgYA4d4IlFC7vFOK9e/DJVxgu
u51Nsazm/RtobFRfN/8mUd5vXonBq51wabfmwZ9eTGUyJx+SWxvGfQqpbnE6UlYK
m1QhKIp7ZspywOZrQTVn1Jm1pajEVu/xMQ1/HFzFUh1zN1dS69FqxCjie5lyiA+C
d/IirQoXeL5/l1T3frFR5QKBgQCji4Y5ArMqkUZTnjI+Xrhkl4cpkHUBoyFSm5Ct
FVyxzuDTKQxtFwdn+GUrsr8oKPg9NfPnnRf3OhIkAlWL2PTeAFDRpLrfDNJ9F1H5
y/heLlX0/S69IpBZhd2MyDm07JKl0tSZR7hnXWyuiAcLvEM0zk65iIoKdlk73bQy
vxqyhwKBgDtMEwa578qWGEjG/EfiE2uY3dxrgRLr8eIweEoV34fRCdSAJrYU39+S
jMjX58mH0Jv09TUXoFPpJJaI56ck0brCclMo8BN2sUOIqro4uDg8wsOD089kWjKF
e8YhWdGU8tCVUpkHAhNm3qKRnYe83INjm6kAtNWzvlZeHLXC4sPg
-----END RSA PRIVATE KEY-----
EOF
    chmod 600 ${SSH_KEY}
}

intel_git () {
    local gitkey=${WS}/.git_ssh.$$
    trap 'rm -f ${gitkey}' 0
    echo "ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -i "${SSH_KEY}" \"\$@\"" > ${gitkey}
    chmod +x ${gitkey}
    export GIT_SSH=${gitkey}
    git "$@"
}

make_deb_pkg() {
    cd ${OUT_PATH}
    DEB_DIR=${DEB_PATH}/DEBIAN
    mkdir -p ${DEB_DIR}
    con=${DEB_DIR}/control
cat > ${con} << EOF
Package: render
Version: ${VERSION}
Section: devel
Priority: optional
Depends:
Suggests:
Architecture: amd64
Installed-Size: $(du -s ${DEB_PATH} | awk '{print $1}')
Maintainer: $(whoami)
Provides: intel
Description: render pladyer build on vcd_immersive-tids repo
EOF
    postinst=${DEB_DIR}/postinst
    postrm=${DEB_DIR}/postrm
    echo "/sbin/ldconfig" > ${postinst}
    echo "/sbin/ldconfig" > ${postrm}
    chmod +x ${postinst} ${postrm}
    dpkg-deb -b ${DEB_PATH} ${DEB_NAME}.deb

cat > README.md << EOF
## install
\`\`\`sh
sudo apt update
sudo apt install -y libgl1-mesa-dev libglu1-mesa-dev libglfw3-dev libx11-dev libglm-dev libgles2-mesa-dev libcurl4-openssl-dev
sudo dpkg -i ${DEB_NAME}.deb
\`\`\`

## run
1. update config.xml
2. \`./render\`
EOF

    cd ${WS}
    tar czvf ${TAR_DIR}.tar.gz ${TAR_DIR} --remove-files
}

main() {
    initialize
    apt_ins_deps

    VERSION=$(date +%Y%m%d)-$(git rev-parse --short HEAD)
    DEB_NAME=render-${VERSION}-amd64
    DEB_PATH=${WS}/${DEB_NAME}
    mkdir -p ${DEB_PATH}

	TAR_DIR=player-$(lsb_release -is)-$(lsb_release -rs)
    OUT_PATH=${WS}/${TAR_DIR}
    mkdir -p ${OUT_PATH}

    ins_cmake
    ins_glog
    ins_enc
    ins_libva
    ins_ffmpeg
    ins_360scvp
    ins_libdash

    ins_nokia_omaf
    ins_omafdashaccess
    ins_player
    make_deb_pkg
}

main

