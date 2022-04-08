CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(MPDWriter)

AUX_SOURCE_DIRECTORY(../../../isolib/atoms DIR_ATOMS_SRCS)
AUX_SOURCE_DIRECTORY(../../../isolib/common DIR_COMMON_SRCS)
AUX_SOURCE_DIRECTORY(../../../isolib/include DIR_INCLUDE_SRCS)
AUX_SOURCE_DIRECTORY(../common DIR_PLUGIN_COMMON_SRCS)
AUX_SOURCE_DIRECTORY(. DIR_MPDWRITER_SRCS)

ADD_DEFINITIONS("-g -c -fPIC -lglog -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0
                 -z noexecstack -z relro -z now -fstack-protector-strong
                 -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat
                 -Wformat-security -Wl,-S -Wall -Werror")

INCLUDE_DIRECTORIES(../common /usr/local/include ../../../utils ../../../isolib/atoms ../../../isolib/common ../../../isolib/include)

LINK_DIRECTORIES(/usr/local/lib ../../../isolib/atoms ../../../isolib/common ../../../isolib/include ../common)

set(DIR_MPDWRITER_SRCS
    ${DIR_MPDWRITER_SRCS}
    ${DIR_ATOMS_SRCS}
    ${DIR_COMMON_SRCS}
    ${DIR_INCLUDE_SRCS}
    ${DIR_PLUGIN_COMMON_SRCS}
   )

ADD_LIBRARY(MPDWriter SHARED ${DIR_MPDWRITER_SRCS})

TARGET_LINK_LIBRARIES(MPDWriter glog)

install(TARGETS MPDWriter
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

install(FILES ${PROJECT_SOURCE_DIR}/../../../utils/error.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../../../utils/Log.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../common/MediaData.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../common/Frame.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../common/AcquireTrackData.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../common/DataItem.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../common/DataItem.icc DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../common/Fraction.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../common/Frame.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../common/FrameWrapper.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../../../isolib/include/Common.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../../../isolib/include/Index.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../../../isolib/common/ISOLog.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../DashMPDWriterPluginAPI.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/MPDWriter.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/MPDWriter.pc DESTINATION lib/pkgconfig)
