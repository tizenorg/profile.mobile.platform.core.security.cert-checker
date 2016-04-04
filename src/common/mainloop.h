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
 * @file        mainloop.h
 * @author      Kyungwook Tak (k.tak@samsung.com)
 * @version     1.0
 * @brief       Manageloop with epoll
 */
#pragma once

#include <functional>
#include <mutex>
#include <unordered_map>

namespace CCHECKER {

class Mainloop {
public:
	typedef std::function<void(uint32_t event)> Callback;

	Mainloop();
	virtual ~Mainloop();

	Mainloop(const Mainloop &) = delete;
	Mainloop &operator=(const Mainloop &) = delete;
	Mainloop(Mainloop &&) = delete;
	Mainloop &operator=(Mainloop &&) = delete;

	void run(int timeout);

	void addEventSource(int fd, uint32_t event, Callback &&callback);
	void removeEventSource(int fd);

private:
	void dispatch(int timeout);

	bool m_isTimedOut;
	int m_pollfd;
	std::mutex m_mutex;
	std::unordered_map<int, Callback> m_callbacks;

	constexpr static size_t MAX_EPOLL_EVENTS = 32;
};

} // namespace CCHECKER
