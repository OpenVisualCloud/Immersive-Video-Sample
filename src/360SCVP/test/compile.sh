#!/bin/bash -e

cp ../../google_test/libgtest.a .

g++ -I../../google_test -std=c++11 -I../util/ -g  -c testI360SCVP.cpp -D_GLIBCXX_USE_CXX11_ABI=0
LD_FLAGS="-I/usr/local/include/ -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib"
g++ -L/usr/local/lib testI360SCVP.o libgtest.a -o testI360SCVP ${LD_FLAGS}
./testI360SCVP

