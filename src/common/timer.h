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

#include <condition_variable>
#include <chrono>
#include <future>

#include "common/log.h"

namespace CCHECKER {

class Timer {
public:
	Timer(void);
	virtual ~Timer(void);

	Timer(const Timer &) = delete;
	Timer &operator=(const Timer &) = delete;
	Timer(Timer &&) = delete;
	Timer &operator=(Timer &&) = delete;

	void timerStart(int interval);
	void timerStop(void);

protected:
	// This is for derived class member function.
	virtual void job(void) {}

	void run();

private:
	std::mutex m_mutex;
	std::condition_variable m_cv;
	std::thread m_thread;

	bool m_isStop;
	bool m_isRunning;
	int m_interval;
};

} // namespace CCHECKER
