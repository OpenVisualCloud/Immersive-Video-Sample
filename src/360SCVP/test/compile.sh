#!/bin/bash -e

cp ../../google_test/libgtest.a .

g++ -I../../google_test -std=c++11 -I../util/ -g  -c testI360SCVP_common.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../../google_test -std=c++11 -I../util/ -g  -c testI360SCVP_erp.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../../google_test -std=c++11 -I../util/ -g  -c testI360SCVP_cubemap.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../../google_test -std=c++11 -I../util/ -g  -c testI360SCVP_novelview.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../../google_test -std=c++11 -I../util/ -g  -c testI360SCVP_rotationConvert.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../../google_test -std=c++11 -I../util/ -g  -c testI360SCVP_xmlParsing.cpp -D_GLIBCXX_USE_CXX11_ABI=0

LD_FLAGS="-I/usr/local/include/ -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib -D_GLIBCXX_DEBUG=1"
g++ -L/usr/local/lib testI360SCVP_common.o libgtest.a -o testI360SCVP_common ${LD_FLAGS}
g++ -L/usr/local/lib testI360SCVP_erp.o libgtest.a -o testI360SCVP_erp ${LD_FLAGS}
g++ -L/usr/local/lib testI360SCVP_cubemap.o libgtest.a -o testI360SCVP_cubemap ${LD_FLAGS}
g++ -L/usr/local/lib testI360SCVP_novelview.o libgtest.a -o testI360SCVP_novelview ${LD_FLAGS}
g++ -L/usr/local/lib testI360SCVP_rotationConvert.o libgtest.a -o testI360SCVP_rotationConvert ${LD_FLAGS}
g++ -L/usr/local/lib testI360SCVP_xmlParsing.o libgtest.a -o testI360SCVP_xmlParsing ${LD_FLAGS}

./testI360SCVP_common
./testI360SCVP_erp
./testI360SCVP_cubemap
./testI360SCVP_novelview
./testI360SCVP_rotationConvert
./testI360SCVP_xmlParsing
