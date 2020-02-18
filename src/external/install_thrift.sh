#!/bin/sh -ex

mkdir -p ../build/external
cd ../build/external

# boost
if [ ! -f "./boost_1_63_0.tar.gz" ] ; then
    wget -q https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.gz
fi
tar zxf boost_1_63_0.tar.gz
cd boost_1_63_0
./bootstrap.sh --without-libraries=python
./b2 -a cxxflags="-D_GLIBCXX_USE_CXX11_ABI=0" -j8
sudo ./b2 cxxflags="-D_GLIBCXX_USE_CXX11_ABI=0" install
cd ..

# Thrift
if [ ! -f "./thrift-0.12.0.tar.gz" ] ; then
    wget -q http://apache.osuosl.org/thrift/0.12.0/thrift-0.12.0.tar.gz
fi
tar zxf thrift-0.12.0.tar.gz
cd thrift-0.12.0
patch configure ../../../external/Disable_cxx11_abi_for_thrift.patch
sed -i '21 a #  include <unistd.h>' ./lib/cpp/src/thrift/transport/PlatformSocket.h
./configure --with-boost=/usr/local --with-boost-libdir=/usr/local/lib --with-libevent=/usr --with-java=0
make -j8
sudo make install
cd ..

