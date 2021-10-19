CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(VROmafPacking)

OPTION(USE_TRACE
       "Use trace"
       OFF
)

AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(../utils/ UTIL_SRC)

ADD_DEFINITIONS("-g -c -fPIC -lglog -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0
                 -z noexecstack -z relro -z now -fstack-protector-strong
                 -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat
                 -Wformat-security -Wl,-S -Wall -Werror")

IF(USE_SAFE_MEM)
ADD_DEFINITIONS("-D_SAFE_MEM_")
ENDIF()

IF(USE_TRACE)
  ADD_DEFINITIONS("-D_USE_TRACE_")
  INCLUDE_DIRECTORIES(/usr/local/include ../utils ../360SCVP ../isolib ../trace ../plugins/StreamProcess_Plugin)
  LINK_DIRECTORIES(/usr/local/lib ../isolib/dash_writer ../trace)
ELSE()
  INCLUDE_DIRECTORIES(/usr/local/include ../utils ../360SCVP ../isolib ../plugins/StreamProcess_Plugin)
  LINK_DIRECTORIES(/usr/local/lib ../isolib/dash_writer)
ENDIF()

SET(DIR_SRC
    ${DIR_SRC}
    ${UTIL_SRC}
)

ADD_LIBRARY(VROmafPacking SHARED ${DIR_SRC})

TARGET_LINK_LIBRARIES(VROmafPacking dashwriter)
TARGET_LINK_LIBRARIES(VROmafPacking glog)
TARGET_LINK_LIBRARIES(VROmafPacking dl)
IF(USE_SAFE_MEM)
TARGET_LINK_LIBRARIES(VROmafPacking safestring_shared)
ENDIF()
IF(USE_TRACE)
  TARGET_LINK_LIBRARIES(VROmafPacking trace)
  TARGET_LINK_LIBRARIES(VROmafPacking lttng-ust)
ENDIF()

INSTALL(TARGETS VROmafPacking
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/error.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/common_data.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/Log.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/VROmafPacking_data.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/VROmafPackingAPI.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/VROmafPacking.pc DESTINATION lib/pkgconfig)
