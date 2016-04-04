/*
 *  Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License
 */
/*
 * @file        socket.h
 * @author      Kyungwook Tak (k.tak@samsung.com)
 * @version     1.0
 * @brief
 */
#pragma once

#include <string>

#include "common/types.h"

namespace CCHECKER {

class Socket {
public:
	Socket(int fd = 0);
	Socket(const std::string &path); /* Construct with systemd socket from path */

	Socket(const Socket &) = delete;
	Socket &operator=(const Socket &) = delete;

	Socket(Socket &&);
	Socket &operator=(Socket &&);

	virtual ~Socket();

	Socket accept(void) const;
	int getFd(void) const;

	RawBuffer read(void) const;
	void write(const RawBuffer &data) const;

	/* TODO: can it be constructor? */
	static Socket connect(const std::string &path);

private:
	int m_fd;
};

}
