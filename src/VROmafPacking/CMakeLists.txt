cmake_minimum_required(VERSION 2.8)

project(VROmafPacking)

AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(../utils/ UTIL_SRC)

ADD_DEFINITIONS("-g -c -fPIC -lglog -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -z noexecstack -z relro -z now -fstack-protector-strong -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-S -Wall -Werror")

INCLUDE_DIRECTORIES(/usr/local/include ../utils ../360SCVP)
LINK_DIRECTORIES(/usr/local/lib)

set(DIR_SRC
    ${DIR_SRC}
    ${UTIL_SRC}
    )

ADD_LIBRARY(VROmafPacking SHARED  ${DIR_SRC})

TARGET_LINK_LIBRARIES(VROmafPacking streamsegmenter_static_fpic)
TARGET_LINK_LIBRARIES(VROmafPacking streamsegmenter_static)
TARGET_LINK_LIBRARIES(VROmafPacking mp4vr_static_fpic)
TARGET_LINK_LIBRARIES(VROmafPacking mp4vr_static)
TARGET_LINK_LIBRARIES(VROmafPacking glog)

install(TARGETS VROmafPacking
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

install(FILES ${PROJECT_SOURCE_DIR}/../utils/error.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/VROmafPacking_data.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/VROmafPackingAPI.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/VROmafPacking.pc DESTINATION lib/pkgconfig)
