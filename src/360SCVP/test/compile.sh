#!/bin/bash -e

cp ../../google_test/libgtest.a .

g++ -I../../google_test -std=c++11 -I../util/ -g  -c testI360SCVP_common.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../../google_test -std=c++11 -I../util/ -g  -c testI360SCVP_erp.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../../google_test -std=c++11 -I../util/ -g  -c testI360SCVP_cubemap.cpp -D_GLIBCXX_USE_CXX11_ABI=0

LD_FLAGS="-I/usr/local/include/ -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib"
g++ -L/usr/local/lib testI360SCVP.o libgtest.a -o testI360SCVP_common ${LD_FLAGS}
g++ -L/usr/local/lib testI360SCVP.o libgtest.a -o testI360SCVP_erp ${LD_FLAGS}
g++ -L/usr/local/lib testI360SCVP_cubemap.o libgtest.a -o testI360SCVP_cubemap ${LD_FLAGS}

./testI360SCVP_common
./testI360SCVP_erp
./testI360SCVP_cubemap

