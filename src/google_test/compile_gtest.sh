#!/bin/bash

git clone https://github.com/google/googletest.git
cd googletest/
git checkout -b v1.8.x origin/v1.8.x
cd googletest/
mkdir build
cd build/
cmake -DBUILD_SHARED_LIBS=ON ..
make
g++ -I../include/ -I.. -c ../src/gtest-all.cc -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../include/ -I.. -c ../src/gtest_main.cc -D_GLIBCXX_USE_CXX11_ABI=0
ar -rv libgtest.a gtest-all.o gtest_main.o
cp libgtest.a ../../../
cp -r ../include ../../../
cd ../../..
