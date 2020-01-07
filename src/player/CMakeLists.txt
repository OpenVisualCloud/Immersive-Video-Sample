CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

option(USE_OMAF
	"Use omaf"
	OFF
)

option(USE_WEBRTC
	"Use webrtc"
	OFF
)

PROJECT(player)

if(USE_OMAF)
ADD_DEFINITIONS("-g -c -fPIC -lglog -std=c++11 -fpermissive")
endif()

if(USE_WEBRTC)
ADD_DEFINITIONS("-g -c -fPIC -lglog -std=c++11 -fpermissive -DLOW_LATENCY_USAGE")
endif()

INCLUDE_DIRECTORIES(/usr/local/include ../utils)

LINK_DIRECTORIES(/usr/local/lib)

if(USE_WEBRTC)
    if(NOT DEFINED WEBRTC_LINUX_SDK)
        message(SEND_ERROR "WEBRTC_LINUX_SDK is required")
    endif()

    ADD_DEFINITIONS("-D_ENABLE_WEBRTC_SOURCE_ -DWEBRTC_LINUX -DWEBRTC_POSIX")
    INCLUDE_DIRECTORIES("${WEBRTC_LINUX_SDK}/include")
    LINK_DIRECTORIES("${WEBRTC_LINUX_SDK}/lib")
endif()

AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(../utils/ UTIL_SRC)

set(DIR_SRC
    ${DIR_SRC}
    ${UTIL_SRC}
    )

ADD_EXECUTABLE(render ${DIR_SRC})

if(USE_OMAF)
	set(LINK_LIB ${LINK_LIB} OmafDashAccess)
endif()

if(USE_WEBRTC)
	set(LINK_LIB ${LINK_LIB} owt sioclient_tls ssl crypto boost_system dl)
endif()

set(LINK_LIB ${LINK_LIB} avfilter avformat avcodec avdevice avutil swscale swresample va va-drm va-x11 360SCVP glfw GL GLU X11 pthread lzma z glog EGL GLESv2 dl)

TARGET_LINK_LIBRARIES( render ${LINK_LIB} )
