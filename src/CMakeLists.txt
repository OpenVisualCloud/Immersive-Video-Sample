CMAKE_MINIMUM_REQUIRED(VERSION 2.12)
PROJECT(immersive-media)

# OPTION(TARGET "This is a default option for target." None)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()
MESSAGE("Build type: " ${CMAKE_BUILD_TYPE})

IF(NOT TARGET)
  MESSAGE(SEND_ERROR "\Set TARGET: server , client")
ENDIF()

IF(${TARGET} STREQUAL "server")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -z noexecstack -z relro -z now -fstack-protector-strong -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-S -Werror -Wall -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -g")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -z noexecstack -z relro -z now -fstack-protector-strong -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-S -Werror -Wall -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/360SCVP)
  ADD_SUBDIRECTORY(360SCVP)
  ADD_SUBDIRECTORY(distributed_encoder)
  ADD_SUBDIRECTORY(VROmafPacking)
  SET(FFMPEG_DIR "ffmpeg")
  SET(FFMPEG_FILE "ffmpeg.txt")
  SET(FFMPEG_FLAG "--prefix=/usr --libdir=/usr/lib --enable-static --disable-shared --enable-gpl --enable-nonfree --disable-optimizations --disable-vaapi --enable-libDistributedEncoder --enable-libVROmafPacking")
  ADD_CUSTOM_COMMAND(
      OUTPUT ${FFMPEG_FILE}
      COMMAND mkdir -p ${FFMPEG_DIR} && cd ${FFMPEG_DIR} && ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/configure ${FFMPEG_FLAG} 
      COMMAND cd ${FFMPEG_DIR} && make -j100
      COMMENT "Building ffmpeg..."
      VERBATIM
  )
  ADD_CUSTOM_TARGET(ffmpeg ALL
      COMMAND echo "Target ffmpeg"
      DEPENDS ${FFMPEG_FILE}
      VERBATIM
  )

ELSEIF(${TARGET} STREQUAL "client")

  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -z noexecstack -z relro -z now -fstack-protector-strong -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-S -Wall -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -g")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -z noexecstack -z relro -z now -fstack-protector-strong -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-S -Wall -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0")
  ADD_SUBDIRECTORY(360SCVP)
  ADD_SUBDIRECTORY(VROmafPacking)
  SET(UTILS_DIR "utils")
  SET(PLAYER_DIR "player")
  SET(PLAYER_FILE "player.txt")
  ADD_CUSTOM_COMMAND(
      OUTPUT ${PLAYER_FILE}
      COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${UTILS_DIR}/tinyxml2.cpp ${CMAKE_CURRENT_SOURCE_DIR}/${PLAYER_DIR}
      COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${UTILS_DIR}/tinyxml2.h ${CMAKE_CURRENT_SOURCE_DIR}/${PLAYER_DIR} 
      COMMAND mkdir ${PLAYER_DIR} && cd ${PLAYER_DIR} && cmake -DUSE_OMAF=ON -DUSE_WEBRTC=OFF ${CMAKE_CURRENT_SOURCE_DIR}/${PLAYER_DIR} && make -j
      COMMENT "Building player..."
      VERBATIM
  )
  ADD_CUSTOM_TARGET(player ALL
      COMMAND echo "Target player"
      DEPENDS ${PLAYER_FILE}
      VERBATIM
  )

ELSE()

  MESSAGE(SEND_ERROR "\Set TARGET_BUILD_TYPE : server , client")

ENDIF()
