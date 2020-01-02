cmake_minimum_required(VERSION 2.8)

project(360SCVP)

AUX_SOURCE_DIRECTORY(. DIR_SRC)

ADD_DEFINITIONS("-z noexecstack -z relro -z now -fstack-protector-strong -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-S -Wall -Werror -g -c -fPIC -std=c++11")

INCLUDE_DIRECTORIES(/usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)

ADD_LIBRARY(360SCVP SHARED  ${DIR_SRC})

install(TARGETS 360SCVP
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

install(FILES ${PROJECT_SOURCE_DIR}/360SCVPAPI.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/360SCVPViewportAPI.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/360SCVP.pc DESTINATION lib/pkgconfig)

