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
# @brief       Make common library for both of server and client.
#
PKG_CHECK_MODULES(${TARGET_CERT_CHECKER_COMMON}_DEP
	REQUIRED
	libsystemd-journal
	libsystemd-daemon
)

SET(COMMON_PATH ${CERT_CHECKER_SRC_PATH}/common)

SET(${TARGET_CERT_CHECKER_COMMON}_SRCS
	${COMMON_PATH}/binary-queue.cpp
	${COMMON_PATH}/connection.cpp
	${COMMON_PATH}/dispatcher.cpp
	${COMMON_PATH}/eventfd.cpp
	${COMMON_PATH}/mainloop.cpp
	${COMMON_PATH}/service.cpp
	${COMMON_PATH}/socket.cpp
	${COMMON_PATH}/timer.cpp
	${COMMON_PATH}/log.cpp
)

INCLUDE_DIRECTORIES(SYSTEM
	${${TARGET_CERT_CHECKER_COMMON}_DEP_INCLUDE_DIRS}
	${CERT_CHECKER_SRC_PATH}/include/
	${DPL_CORE_PATH}/include/
	${DPL_DB_PATH}/include/
)

ADD_LIBRARY(${TARGET_CERT_CHECKER_COMMON}
	SHARED ${${TARGET_CERT_CHECKER_COMMON}_SRCS}
)

SET_TARGET_PROPERTIES(${TARGET_CERT_CHECKER_COMMON}
	PROPERTIES
		COMPILE_FLAGS "-D_GNU_SOURCE -fvisibility=default" # TODO: visibility hidden
		LINKER_LANGUAGE CXX
		SOVERSION ${API_VERSION}
		VERSION ${VERSION}
)

TARGET_LINK_LIBRARIES(${TARGET_CERT_CHECKER_COMMON}
	${${TARGET_CERT_CHECKER_COMMON}_DEP_LIBRARIES}
)

INSTALL(TARGETS ${TARGET_CERT_CHECKER_COMMON} DESTINATION ${LIB_INSTALL_DIR})
