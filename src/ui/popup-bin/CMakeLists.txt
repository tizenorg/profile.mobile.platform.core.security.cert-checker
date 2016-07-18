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
    elementary
    libtzplatform-config
    REQUIRED)

set(CERT_CHECKER_POPUP_SRC_DIR ${PROJECT_SOURCE_DIR}/src/ui/popup-bin)

set(CERT_CHECKER_POPUP_SOURCES
    ${CERT_CHECKER_POPUP_SRC_DIR}/main.cpp
    ${CERT_CHECKER_POPUP_SRC_DIR}/popup.cpp
    ${CERT_CHECKER_POPUP_SRC_DIR}/popup-service.cpp
    ${CERT_CHECKER_SERVICE_PATH}/app.cpp
    # dpl
    ${DPL_CORE_SRC_PATH}/assert.cpp
    ${DPL_CORE_SRC_PATH}/exception.cpp
    ${DPL_CORE_SRC_PATH}/serialization.cpp
    ${DPL_CORE_SRC_PATH}/errno_string.cpp
    ${DPL_CORE_SRC_PATH}/noncopyable.cpp
)

ADD_DEFINITIONS(${CERT_CHECKER_POPUP_DEP_CFLAGS})

INCLUDE_DIRECTORIES(
    ${CERT_CHECKER_POPUP_SRC_DIR}
    ${CERT_CHECKER_POPUP_DEP_INCLUDE_DIRS}
    ${CERT_CHECKER_SRC_PATH}
    ${CERT_CHECKER_SRC_PATH}/include/
    ${DPL_CORE_SRC_PATH}/include/
)

ADD_EXECUTABLE(${TARGET_CERT_CHECKER_POPUP}
    ${CERT_CHECKER_POPUP_SOURCES}
)

SET_TARGET_PROPERTIES(${TARGET_CERT_CHECKER_POPUP} PROPERTIES
    COMPILE_FLAGS
    -fpie
)

TARGET_LINK_LIBRARIES(${TARGET_CERT_CHECKER_POPUP}
    ${CERT_CHECKER_POPUP_DEP_LIBRARIES}
    ${TARGET_CERT_CHECKER_COMMON}
    -pie
)

INSTALL(TARGETS ${TARGET_CERT_CHECKER_POPUP} DESTINATION ${BIN_DIR})
