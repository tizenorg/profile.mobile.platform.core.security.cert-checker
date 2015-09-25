# Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#
# @file     CMakeLists.txt
# @author   Janusz Kozerski (j.kozerski@samsung.com)
#
# @version  1.0
#

PKG_CHECK_MODULES(CERT_CHECKER_POPUP_DEP
   # dpl-efl
    elementary
    libsystemd-journal
    REQUIRED)

set(CERT_CHECKER_POPUP_SRC_DIR ${PROJECT_SOURCE_DIR}/src/ui/popup-bin)

set(CERT_CHECKER_POPUP_SOURCES
    ${CERT_CHECKER_POPUP_SRC_DIR}/popup.cpp
    ${CERT_CHECKER_SRC_PATH}/ui/popup-runner.cpp
    ${CERT_CHECKER_SRC_PATH}/app.cpp
    # logs
    ${CERT_CHECKER_SRC_PATH}/log/log.cpp
    # dpl
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/serialization.cpp
)

ADD_DEFINITIONS(${CERT_CHECKER_POPUP_DEP_CFLAGS})

INCLUDE_DIRECTORIES(
    ${CERT_CHECKER_POPUP_SRC_DIR}
    ${CERT_CHECKER_POPUP_DEP_INCLUDE_DIRS}
    ${CERT_CHECKER_SRC_PATH}/include/
)

ADD_EXECUTABLE(${TARGET_CERT_CHECKER_POPUP}
    ${CERT_CHECKER_POPUP_SOURCES}
)

SET_TARGET_PROPERTIES(${TARGET_CERT_CHECKER_POPUP} PROPERTIES
    COMPILE_FLAGS -fPIC
)

#SET_TARGET_PROPERTIES(${TARGET_CERT_CHECKER_POPUP} PROPERTIES
#    #LINK_FLAGS "-Wl,--as-needed -Wl"
#    SOVERSION ${PROJECT_API_VERSION}
#    VERSION ${PROJECT_VERSION}
#)

target_link_libraries(${TARGET_CERT_CHECKER_POPUP}
    ${CERT_CHECKER_POPUP_DEP_LIBRARIES}
#    ${TARGET_CERT_CHECKER_POPUP_RUNNER_LIB}
#    ${CERT_CHECKER_POPUP_DEP_LDFLAGS}
)

INSTALL(TARGETS ${TARGET_CERT_CHECKER_POPUP} DESTINATION ${BINDIR})
