# Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
# @author      Sangwan Kwon (sangwan.kwon@samsung.com)
# @brief       Make client library for ocsp-service.
#
PKG_CHECK_MODULES(${TARGET_CERT_CHECKER_CLIENT}_DEP
	REQUIRED
	libsystemd-journal
)

SET(${TARGET_CERT_CHECKER_CLIENT}_SRCS
	${CERT_CHECKER_SRC_PATH}/client/ocsp-client.cpp
)

INCLUDE_DIRECTORIES(SYSTEM
	${${TARGET_CERT_CHECKER_CLIENT}_DEP_INCLUDE_DIRS}
	${CERT_CHECKER_SRC_PATH}/
	${CERT_CHECKER_SRC_PATH}/include/
	${DPL_CORE_PATH}/include/
	${DPL_DB_PATH}/include/
)

ADD_LIBRARY(${TARGET_CERT_CHECKER_CLIENT}
	SHARED ${${TARGET_CERT_CHECKER_CLIENT}_SRCS}
)

SET_TARGET_PROPERTIES(${TARGET_CERT_CHECKER_CLIENT}
	PROPERTIES
		COMPILE_FLAGS "-D_GNU_SOURCE -fvisibility=default"
		LINKER_LANGUAGE CXX
		SOVERSION ${API_VERSION}
		VERSION ${VERSION}
)

TARGET_LINK_LIBRARIES(${TARGET_CERT_CHECKER_CLIENT}
	${${TARGET_CERT_CHECKER_CLIENT}_DEP_LIBRARIES}
	${TARGET_CERT_CHECKER_COMMON}
)

INSTALL(TARGETS ${TARGET_CERT_CHECKER_CLIENT} DESTINATION ${LIB_INSTALL_DIR})
INSTALL(
	FILES
		ocsp-client.h
		error.h
	DESTINATION
		${INCLUDE_INSTALL_DIR}/cchecker
)
