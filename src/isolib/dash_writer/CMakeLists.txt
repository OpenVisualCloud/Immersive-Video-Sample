CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(dashwriter)

AUX_SOURCE_DIRECTORY(../atoms DIR_DASHWRITER_SRCS)
AUX_SOURCE_DIRECTORY(. DIR_DASHWRITER_SRCS)

ADD_DEFINITIONS("-g -c -fPIC -lglog -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0
                 -z noexecstack -z relro -z now -fstack-protector-strong
                 -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat
                 -Wformat-security -Wl,-S -Wall -Werror")

INCLUDE_DIRECTORIES(/usr/local/include ../include ../atoms)

LINK_DIRECTORIES(/usr/local/lib ../atoms)

ADD_LIBRARY(dashwriter STATIC ${DIR_DASHWRITER_SRCS})

TARGET_LINK_LIBRARIES(dashwriter glog)
