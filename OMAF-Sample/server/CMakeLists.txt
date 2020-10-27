CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(${TARGET} STREQUAL "build")
    ADD_CUSTOM_TARGET(build_server ALL
                      ${CMAKE_CURRENT_SOURCE_DIR}/deploy.sh
                      ${CMAKE_CURRENT_SOURCE_DIR} ${HTTP_PROXY})
ENDIF()

IF(${TARGET} STREQUAL "start")
    ADD_CUSTOM_TARGET(build_server ALL
                      ${CMAKE_CURRENT_SOURCE_DIR}/start.sh ${NODEIP})
ENDIF()
