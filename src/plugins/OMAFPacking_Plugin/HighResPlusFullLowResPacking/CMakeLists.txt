CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(HighResPlusFullLowResPacking)

AUX_SOURCE_DIRECTORY(. DIR_SRC)

ADD_DEFINITIONS("-g -c -fPIC -lglog -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -z noexecstack -z relro -z now -fstack-protector-strong -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-S -Wall -Werror")

INCLUDE_DIRECTORIES(/usr/local/include ../../../utils ../../../360SCVP ../)
LINK_DIRECTORIES(/usr/local/lib)

ADD_LIBRARY(HighResPlusFullLowResPacking SHARED ${DIR_SRC})

TARGET_LINK_LIBRARIES(HighResPlusFullLowResPacking glog)

install(TARGETS HighResPlusFullLowResPacking
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

install(FILES ${PROJECT_SOURCE_DIR}/../../../utils/error.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../../../utils/common_data.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../../../utils/Log.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../OMAFPackingPluginAPI.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/HighResPlusFullLowResPacking.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/HighResPlusFullLowResPacking.pc DESTINATION lib/pkgconfig)
