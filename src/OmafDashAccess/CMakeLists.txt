cmake_minimum_required(VERSION 2.8)

project(OmafDashAccess)

AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(../utils UTIL_SRC)
AUX_SOURCE_DIRECTORY(OmafDashDownload/. DOWN_SRC)
AUX_SOURCE_DIRECTORY(OmafDashParser/. PARSE_SRC)

ADD_DEFINITIONS("-g -c -fPIC -lstdc++fs -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -z noexecstack -z relro -z now -fstack-protector-strong -fPIE -fPIC -pie -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-S -Wall -Werror")

INCLUDE_DIRECTORIES(/usr/local/include ../utils mp4lib/api/reader)
LINK_DIRECTORIES(/usr/local/lib)

set(DIR_SRC
    ${DIR_SRC}
    ${UTIL_SRC}
    ${DOWN_SRC}
    ${PARSE_SRC}
    )

ADD_LIBRARY(OmafDashAccess SHARED  ${DIR_SRC})

TARGET_LINK_LIBRARIES(OmafDashAccess streamsegmenter_static_fpic)
TARGET_LINK_LIBRARIES(OmafDashAccess streamsegmenter_static)
TARGET_LINK_LIBRARIES(OmafDashAccess mp4vr_static_fpic)
TARGET_LINK_LIBRARIES(OmafDashAccess mp4vr_static)
TARGET_LINK_LIBRARIES(OmafDashAccess glog)
TARGET_LINK_LIBRARIES(OmafDashAccess curl)
TARGET_LINK_LIBRARIES(OmafDashAccess 360SCVP)

install(TARGETS OmafDashAccess
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

install(FILES ${PROJECT_SOURCE_DIR}/../utils/error.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../utils/data_type.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../utils/ns_def.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/../utils/OmafStructure.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/OmafDashAccessApi.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/OmafDashAccess.pc DESTINATION lib/pkgconfig)

