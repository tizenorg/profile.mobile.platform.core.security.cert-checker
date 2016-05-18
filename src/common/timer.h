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
 * @file        timer.h
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       The implementation of timer
 */
#pragma once

#include <functional>
#include <chrono>
#include <future>
#include <cstdio>

#include "common/log.h"

namespace CCHECKER {

class Timer
{
public:
	Timer(void);
	virtual ~Timer(void);

	Timer(const Timer &) = delete;
	Timer &operator=(const Timer &) = delete;
	Timer(Timer &&) = delete;
	Timer &operator=(Timer &&) = delete;

protected:
	void timerStart(int interval);
	template <class Callable, class ...Args>
	void timerStart(int interval, Callable&& f, Args&&... args);
	void timerStop(void);

	// This is for derived class member function.
	virtual void job(void){}

	std::mutex m_mutex;
	bool m_isStop;
};

// 'f' function is for non-member function.
template <class Callable, class ...Args>
void Timer::timerStart(int interval, Callable&& f, Args&&... args)
{
	LogDebug("Timer start!!");
	m_isStop = false;

	std::function<typename std::result_of<Callable(Args...)>::type()>
		task(
			std::bind(std::forward<Callable>(f),
			std::forward<Args>(args)...)
		);

	std::thread([this, interval, task]() {
		while(1) {
			std::lock_guard<std::mutex> lock(m_mutex);
			LogDebug("[timer] running interval : " << interval);

			if (m_isStop)
				break;

			task();

			std::this_thread::sleep_for(
				std::chrono::milliseconds(interval));
		}
	}).detach();
}

} // namespace CCHECKER
