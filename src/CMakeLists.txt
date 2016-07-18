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
# @file        CMakeLists.txt
# @author      Janusz Kozerski <j.kozerski@samsung.com>
# @author      Sangwan Kwon <sangwan.kwon@samsung.com>
# @brief
#

############### CERT_CHECKER ###############
PKG_CHECK_MODULES(${TARGET_CERT_CHECKER}_DEP
    REQUIRED
    capi-appfw-application
    dbus-1
    dbus-glib-1
    db-util
    elementary
    glib-2.0
    gio-2.0
    icu-i18n
    key-manager
    libsystemd-daemon
    sqlite3
    pkgmgr
    pkgmgr-info
)

SET(MAIN_PATH ${CERT_CHECKER_SRC_PATH}/main)

SET(${TARGET_CERT_CHECKER}_SRCS
    ${MAIN_PATH}/cert-checker.cpp
    ${CERT_CHECKER_SERVICE_PATH}/app.cpp
    ${CERT_CHECKER_SERVICE_PATH}/logic.cpp
    ${CERT_CHECKER_SERVICE_PATH}/queue.cpp
    ${CERT_CHECKER_SERVICE_PATH}/certs.cpp
    ${CERT_CHECKER_SERVICE_PATH}/ocsp-service.cpp
    ${CERT_CHECKER_DB_PATH}/sql_query.cpp
    ${DPL_CORE_SRC_PATH}/assert.cpp
    ${DPL_CORE_SRC_PATH}/char_traits.cpp
    ${DPL_CORE_SRC_PATH}/errno_string.cpp
    ${DPL_CORE_SRC_PATH}/exception.cpp
    ${DPL_CORE_SRC_PATH}/noncopyable.cpp
    ${DPL_CORE_SRC_PATH}/string.cpp
    ${DPL_CORE_SRC_PATH}/serialization.cpp
    ${DPL_DB_SRC_PATH}/sql_connection.cpp
    ${DPL_DB_SRC_PATH}/naive_synchronization_object.cpp
)

INCLUDE_DIRECTORIES(SYSTEM
    ./
    ${${TARGET_CERT_CHECKER}_DEP_INCLUDE_DIRS}
    ${CERT_CHECKER_SRC_PATH}/include/
    ${DPL_CORE_PATH}/include/
    ${DPL_DB_PATH}/include/
)

SET_SOURCE_FILES_PROPERTIES(${${TARGET_CERT_CHECKER}_SRCS}
    PROPERTIES
        COMPILE_FLAGS "-D_GNU_SOURCE -fPIE"
)

ADD_EXECUTABLE(${TARGET_CERT_CHECKER} ${${TARGET_CERT_CHECKER}_SRCS})

TARGET_LINK_LIBRARIES(${TARGET_CERT_CHECKER}
    ${${TARGET_CERT_CHECKER}_DEP_LIBRARIES}
    ${TARGET_CERT_CHECKER_COMMON}
    -pie
)

INSTALL(TARGETS ${TARGET_CERT_CHECKER} DESTINATION ${BIN_DIR})

ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(main)
ADD_SUBDIRECTORY(service)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(ui)
