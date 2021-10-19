CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

OPTION(USE_OMAF
       "Use omaf"
       OFF
)

OPTION(USE_WEBRTC
       "Use webrtc"
       OFF
)

OPTION(USE_TRACE
       "Use trace"
       OFF
)

OPTION(LINUX_OS
       "Use linux os"
       OFF
)

OPTION(ANDROID_OS
       "Use android os"
       OFF
)

PROJECT(player_lib)

ADD_DEFINITIONS("-g -c -fPIC -lglog -std=c++11 -fpermissive")

IF(USE_SAFE_MEM)
ADD_DEFINITIONS("-D_SAFE_MEM_")
ENDIF()

IF(USE_OMAF)
  ADD_DEFINITIONS("-D_ENABLE_DASH_SOURCE_")
ENDIF()

IF(USE_WEBRTC)
  IF(NOT DEFINED WEBRTC_LINUX_SDK)
    message(SEND_ERROR "WEBRTC_LINUX_SDK is required")
  ENDIF()

  ADD_DEFINITIONS("-D_ENABLE_WEBRTC_SOURCE_ -DWEBRTC_LINUX -DWEBRTC_POSIX")
  INCLUDE_DIRECTORIES("${WEBRTC_LINUX_SDK}/include")
  LINK_DIRECTORIES("${WEBRTC_LINUX_SDK}/lib")
ENDIF()

IF(LINUX_OS)
  ADD_DEFINITIONS("-D_LINUX_OS_")
ENDIF()

IF(ANDROID_OS)
  ADD_DEFINITIONS("-D_ANDROID_OS_")
ENDIF()

IF(USE_TRACE)
  ADD_DEFINITIONS("-D_USE_TRACE_")
  INCLUDE_DIRECTORIES(/usr/local/include ../../utils ../../trace)
  LINK_DIRECTORIES(/usr/local/lib ../../trace)
ELSE()
  INCLUDE_DIRECTORIES(/usr/local/include ../../utils)
  LINK_DIRECTORIES(/usr/local/lib)
ENDIF()

AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(./Decoder DEC_SRC)
AUX_SOURCE_DIRECTORY(./MediaSource MEDSRC_SRC)
AUX_SOURCE_DIRECTORY(./Render RNDR_SRC)
AUX_SOURCE_DIRECTORY(./Mesh MESH_SRC)
AUX_SOURCE_DIRECTORY(./Api API_SRC)
AUX_SOURCE_DIRECTORY(./Common COMMON_SRC)

set(DIR_SRC
    ${DIR_SRC}
    ${DEC_SRC}
    ${MEDSRC_SRC}
    ${RNDR_SRC}
    ${MESH_SRC}
    ${API_SRC}
    ${COMMON_SRC}
    )

ADD_LIBRARY(MediaPlayer SHARED ${DIR_SRC} ../../utils/Log.cpp ../../utils/tinyxml2.cpp)

IF(USE_OMAF)
  SET(LINK_LIB ${LINK_LIB} OmafDashAccess)
ENDIF()

IF(USE_WEBRTC)
  SET(LINK_LIB ${LINK_LIB}
      owt sioclient_tls ssl crypto dl)
ENDIF()

IF(USE_TRACE)
  SET(LINK_LIB ${LINK_LIB} trace lttng-ust)
ENDIF()

IF(LINUX_OS)
SET(LINK_LIB ${LINK_LIB}
    avfilter avformat avcodec avdevice avutil swscale swresample
    360SCVP GL GLU X11 pthread lzma z glog EGL GLESv2 dl)
IF(USE_SAFE_MEM)
SET(LINK_LIB ${LINK_LIB} safestring_shared)
ENDIF()
ENDIF()

IF(ANDROID_OS)
SET(LINK_LIB ${LINK_LIB}
    360SCVP glog android mediandk OpenMAXAL log GLESv2 GLESv3)
IF(USE_SAFE_MEM)
SET(LINK_LIB ${LINK_LIB} safestring_shared)
ENDIF()
ENDIF()

TARGET_LINK_LIBRARIES(MediaPlayer ${LINK_LIB})

INSTALL(TARGETS MediaPlayer
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

INSTALL(FILES ${PROJECT_SOURCE_DIR}/Api/MediaPlayerAPI.h DESTINATION include)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/Api/MediaPlayer.pc DESTINATION lib/pkgconfig)
