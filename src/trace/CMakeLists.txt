CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(trace)

AUX_SOURCE_DIRECTORY(. DIR_SRC)

ADD_DEFINITIONS("-g -c -fPIC")

INCLUDE_DIRECTORIES(./)

ADD_LIBRARY(trace STATIC ${DIR_SRC})
