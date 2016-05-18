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
 * @file        timer.cpp
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       The implementation of timer 
 */
#include "common/timer.h"

namespace CCHECKER {

Timer::Timer() : m_isStop(false)
{
}

Timer::~Timer()
{
	timerStop();
}

void Timer::timerStart(int interval)
{
	LogDebug("Timer start!!");
	m_isStop = false;

	std::thread([this, interval]() {
		while(1) {
			std::lock_guard<std::mutex> lock(m_mutex);
			LogDebug("[timer] running interval : " << interval);

			if (m_isStop)
				break;

			job();

			std::this_thread::sleep_for(
				std::chrono::milliseconds(interval));
		}
	}).detach();
}

void Timer::timerStop()
{
	LogDebug("Timer stop!!");
	std::lock_guard<std::mutex> lock(m_mutex);
	m_isStop = true;
}

} // namespace CCHECKER
