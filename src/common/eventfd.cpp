/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * @file        eventfd.cpp
 * @author      Jaemin Ryu (jm77.ryu@samsung.com)
 * @version     1.0
 * @brief
 */

#include <sys/types.h>
#include <unistd.h>
#include <exception>
#include <system_error>
#include <cstdint>

#include "eventfd.h"

namespace CCHECKER {

EventFD::EventFD(unsigned int initval, int flags)
{
	fd = ::eventfd(initval, flags);

	if (fd == -1) {
		throw std::logic_error("EventFd from constructor is failed!!");
	}
}

EventFD::~EventFD()
{
	if (fd != -1) {
		::close(fd);
	}
}

void EventFD::send()
{
	const std::uint64_t val = 1;

	if (::write(fd, &val, sizeof(val)) == -1) {
		throw std::logic_error("EventFd send to fd is failed!!");
	}
}

void EventFD::receive()
{
	std::uint64_t val;

	if (::read(fd, &val, sizeof(val)) == -1) {
		throw std::logic_error("EventFd read to fd is failed!!");
	}
}

} // namespace CCHECKER
