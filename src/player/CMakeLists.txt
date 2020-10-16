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

PROJECT(player)

ADD_DEFINITIONS("-g -c -fPIC -lglog -std=c++11 -fpermissive")

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

IF(USE_TRACE)
  ADD_DEFINITIONS("-D_USE_TRACE_")
  INCLUDE_DIRECTORIES(/usr/local/include ../utils ../trace)
  LINK_DIRECTORIES(/usr/local/lib ../trace)
ELSE()
  INCLUDE_DIRECTORIES(/usr/local/include ../utils)
  LINK_DIRECTORIES(/usr/local/lib)
ENDIF()

AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(../utils/ UTIL_SRC)
AUX_SOURCE_DIRECTORY(./Decoder DEC_SRC)
AUX_SOURCE_DIRECTORY(./MediaSource MEDSRC_SRC)
AUX_SOURCE_DIRECTORY(./Render RNDR_SRC)
AUX_SOURCE_DIRECTORY(./Mesh MESH_SRC)

set(DIR_SRC
    ${DIR_SRC}
    ${DEC_SRC}
    ${MEDSRC_SRC}
    ${RNDR_SRC}
    ${MESH_SRC}
    ${UTIL_SRC}
    )

ADD_EXECUTABLE(render ${DIR_SRC} ${DEC_SRC} ${MEDSRC_SRC} ${RNDR_SRC} ${MESH_SRC})

IF(USE_OMAF)
  SET(LINK_LIB ${LINK_LIB} OmafDashAccess)
ENDIF()

IF(USE_WEBRTC)
  SET(LINK_LIB ${LINK_LIB}
      owt sioclient_tls ssl crypto boost_system dl)
ENDIF()

IF(USE_TRACE)
  SET(LINK_LIB ${LINK_LIB} trace lttng-ust)
ENDIF()

SET(LINK_LIB ${LINK_LIB}
    avfilter avformat avcodec avdevice avutil swscale swresample
    360SCVP glfw GL GLU X11 pthread lzma z glog EGL GLESv2 dl safestring_shared)

TARGET_LINK_LIBRARIES(render ${LINK_LIB})
