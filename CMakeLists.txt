# Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved

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
# @file        CMakeLists.txt
# @author      Janusz Kozerski <j.kozerski@samsung.com>
# @author      Sangwan Kwon <sangwan.kwon@samsung.com>
# @brief
#

############################# Check minimum CMake version #####################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(${SERVICE_NAME})

############################# cmake packages ##################################

INCLUDE(FindPkgConfig)

#############################     defines    ##################################

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(RESDIR "${PREFIX}/res")
SET(LOCALEDIR "${RESDIR}/locale")

############################# compiler flags ##################################

SET(CMAKE_CXX_FLAGS_PROFILING  "-g -std=c++0x -O0 -pg -Wp,-U_FORTIFY_SOURCE")
SET(CMAKE_C_FLAGS_DEBUG        "-g -O0 -ggdb -Wp,-U_FORTIFY_SOURCE")
SET(CMAKE_CXX_FLAGS_DEBUG      "-g -std=c++0x -O0 -ggdb -Wp,-U_FORTIFY_SOURCE")
SET(CMAKE_C_FLAGS_RELEASE      "-g -O2")
SET(CMAKE_CXX_FLAGS_RELEASE    "-g -std=c++0x -O2")
SET(CMAKE_CXX_FLAGS_CCOV       "-g -std=c++0x -O2 --coverage")

# Set compiler warning flags
ADD_DEFINITIONS("-Werror")                      # Make all warnings into errors.
ADD_DEFINITIONS("-Wall")                        # Generate all warnings
ADD_DEFINITIONS("-Wextra")                      # Generate even more extra warnings
ADD_DEFINITIONS("-fvisibility=hidden -fPIE")    # Hide symbols by default

# Set library flag
ADD_DEFINITIONS("-fPIC")

# Set linker flag
SET(CMAKE_SHARED_LINKER_FLAGS  "-Wl,--as-needed")
SET(CMAKE_EXE_LINKER_FLAGS     "-Wl,--as-needed")

# Pass macro to sources
ADD_DEFINITIONS("-DSERVICE_NAME=\"${SERVICE_NAME}\"")
ADD_DEFINITIONS("-DSERVICE_STREAM=\"${SERVICE_STREAM}\"")
ADD_DEFINITIONS("-DSERVICE_USER=\"${SERVICE_USER}\"")
ADD_DEFINITIONS("-DSERVICE_GROUP=\"${SERVICE_GROUP}\"")

ADD_DEFINITIONS("-DPOPUP_STREAM=\"${POPUP_STREAM}\"")

ADD_DEFINITIONS("-DINCLUDE_INSTALL_DIR=\"${INCLUDE_INSTALL_DIR}\"")

STRING(REGEX MATCH "([^.]*)" API_VERSION "${VERSION}")

IF (CMAKE_BUILD_TYPE MATCHES "DEBUG")
    ADD_DEFINITIONS("-DBUILD_TYPE_DEBUG")
ENDIF (CMAKE_BUILD_TYPE MATCHES "DEBUG")

# Enable popup
ADD_DEFINITIONS("-DPOPUP=1")
ADD_DEFINITIONS("-DDB_INSTALL_DIR=\"${DB_INSTALL_DIR}\"")

# Define global macro about TARGET
SET(TARGET_CERT_CHECKER "${SERVICE_NAME}")
SET(TARGET_CERT_CHECKER_COMMON "${SERVICE_NAME}-common")
SET(TARGET_CERT_CHECKER_CLIENT "${SERVICE_NAME}-client")
SET(TARGET_CERT_CHECKER_POPUP "${SERVICE_NAME}-popup")

# Tests
SET(TARGET_CERT_CHECKER_TESTS "${SERVICE_NAME}-tests")
SET(TARGET_CERT_CHECKER_TESTS_LOGIC "${SERVICE_NAME}-tests-logic")
SET(TARGET_CERT_CHECKER_TESTS_CLIENT "${SERVICE_NAME}-tests-client")
SET(TARGET_CERT_CHECKER_POPUP_TEST "${SERVICE_NAME}-popup-test")

# Define global macro about PATH
SET(CERT_CHECKER_SRC_PATH ${PROJECT_SOURCE_DIR}/src)
SET(CERT_CHECKER_SERVICE_PATH ${CERT_CHECKER_SRC_PATH}/service)
SET(CERT_CHECKER_UI_PATH ${CERT_CHECKER_SRC_PATH}/ui)
SET(CERT_CHECKER_DB_PATH ${CERT_CHECKER_SRC_PATH}/db)

SET(DPL_CORE_PATH ${CERT_CHECKER_SRC_PATH}/dpl/core)
SET(DPL_CORE_SRC_PATH ${DPL_CORE_PATH}/src)
SET(DPL_DB_PATH ${CERT_CHECKER_SRC_PATH}/dpl/db)
SET(DPL_DB_SRC_PATH ${DPL_DB_PATH}/src)

CONFIGURE_FILE(
	packaging/${SERVICE_NAME}.manifest.in
	${SERVICE_NAME}.manifest @ONLY)
CONFIGURE_FILE(
	packaging/${SERVICE_NAME}-common.manifest.in
	${SERVICE_NAME}-common.manifest @ONLY)
CONFIGURE_FILE(
	packaging/${SERVICE_NAME}-client.manifest.in
	${SERVICE_NAME}-client.manifest @ONLY)
CONFIGURE_FILE(
	packaging/${SERVICE_NAME}-tests.manifest.in
	${SERVICE_NAME}-tests.manifest @ONLY)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(db)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(systemd)
ADD_SUBDIRECTORY(pkgconfig)
