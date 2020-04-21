cmake_minimum_required(VERSION 2.8)

project(predict_LR)

AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(../predict_Base BASE_SRC)

SET (DIR_SRC
    ${DIR_SRC}
    ${BASE_SRC}
)
ADD_DEFINITIONS("-g -c -fPIC -lstdc++fs -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -z noexecstack -z relro -z now -fstack-protector-strong -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-S -Wall -Werror")

INCLUDE_DIRECTORIES(/usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)

ADD_LIBRARY(ViewportPredict_LR SHARED ${DIR_SRC})