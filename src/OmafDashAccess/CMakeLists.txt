CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

OPTION(USE_ANDROID_NDK
	"Use android ndk"
	OFF
)

PROJECT(OmafDashAccess)

AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(OmafDashDownload/. DOWN_SRC)
AUX_SOURCE_DIRECTORY(OmafDashParser/. PARSE_SRC)
AUX_SOURCE_DIRECTORY(OmafViewportPredict/. PREDICT_SRC)

IF(USE_SAFE_MEM)
ADD_DEFINITIONS("-D_SAFE_MEM_")
ENDIF()

IF(NOT USE_ANDROID_NDK)
  ADD_DEFINITIONS("-g -c -fPIC -lstdc++fs -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0
                   -z noexecstack -z relro -z now -fstack-protector-strong
                   -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat
                   -Wformat-security -Wl,-S -Wall -Werror")
ELSE()
  ADD_DEFINITIONS("-g -c -fPIC -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0
                   -fPIE -fPIC -O2 -D_FORTIFY_SOURCE=2 -Wformat
                   -Wformat-security -Wall")
ENDIF()

IF(USE_ANDROID_NDK)
  ADD_DEFINITIONS("-D_ANDROID_NDK_OPTION_")
ENDIF()

IF(USE_TRACE)
  ADD_DEFINITIONS("-D_USE_TRACE_")
ENDIF()

IF(NOT USE_ANDROID_NDK)
  IF(USE_TRACE)
    INCLUDE_DIRECTORIES(/usr/local/include ../utils ../isolib ../trace)
    LINK_DIRECTORIES(/usr/local/lib ../isolib/dash_parser ../trace)
  ELSE()
    INCLUDE_DIRECTORIES(/usr/local/include ../utils ../isolib)
    LINK_DIRECTORIES(/usr/local/lib ../isolib/dash_parser)
  ENDIF()
ELSE()
  INCLUDE_DIRECTORIES(/usr/local/include ../utils ../isolib)
  LINK_DIRECTORIES(/usr/local/lib ../isolib/dash_parser)
ENDIF()

set(DIR_SRC
    ${DIR_SRC}
    ${DOWN_SRC}
    ${PARSE_SRC}
    ${PREDICT_SRC}
    )

ADD_LIBRARY(OmafDashAccess SHARED  ${DIR_SRC} ../utils/Log.cpp ../utils/tinyxml2.cpp)

TARGET_LINK_LIBRARIES(OmafDashAccess glog)
TARGET_LINK_LIBRARIES(OmafDashAccess curl)
TARGET_LINK_LIBRARIES(OmafDashAccess dashparser)
TARGET_LINK_LIBRARIES(OmafDashAccess 360SCVP)
IF(USE_SAFE_MEM)
TARGET_LINK_LIBRARIES(OmafDashAccess safestring_shared)
ENDIF()
IF(NOT USE_ANDROID_NDK)
  IF(USE_TRACE)
    TARGET_LINK_LIBRARIES(OmafDashAccess trace)
    TARGET_LINK_LIBRARIES(OmafDashAccess lttng-ust)
  ENDIF()
  TARGET_LINK_LIBRARIES(OmafDashAccess dl)
ENDIF()
IF(USE_ANDROID_NDK)
  TARGET_LINK_LIBRARIES(OmafDashAccess log)
ENDIF()

INSTALL(TARGETS OmafDashAccess
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/error.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/pose.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/data_type.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/ns_def.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/OmafStructure.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/common_data.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/../utils/Log.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/OmafDashAccessApi.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/OmafDashAccess.pc DESTINATION lib/pkgconfig)
