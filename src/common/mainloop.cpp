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
 * @file        mainloop.cpp
 * @author      Kyungwook Tak (k.tak@samsung.com)
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       Mainloop with epoll
 */
#include "common/mainloop.h"

#include <exception>
#include <stdexcept>
#include <system_error>
#include <sys/epoll.h>
#include <unistd.h>

#include "common/log.h"

namespace CCHECKER {

Mainloop::Mainloop() :
	m_isTimedOut(false),
	m_pollfd(::epoll_create1(EPOLL_CLOEXEC)),
	m_stopped(false)
{
	if (m_pollfd == -1)
		throw std::system_error(
			std::error_code(errno, std::generic_category()),
			"Failed to epoll_create1");
}

Mainloop::~Mainloop()
{
	if (!m_stopped && !m_isTimedOut && !m_callbacks.empty())
		throw std::logic_error("mainloop registered callbacks should be empty "
							"except timed out case");

	::close(m_pollfd);
}

void Mainloop::stop()
{
	m_stopped = true;
	wakeupSignal.send();
}

void Mainloop::prepare()
{
	auto wakeupMainloop = [&](uint32_t) {
		wakeupSignal.receive();
		removeEventSource(wakeupSignal.getFd());
	};

	addEventSource(wakeupSignal.getFd(), EPOLLIN, std::move(wakeupMainloop));
}

void Mainloop::run(int timeout)
{
	prepare();
	m_isTimedOut = false;

	while (!m_isTimedOut && !m_stopped) {
		dispatch(timeout);
	}

	LogDebug("Mainloop run stopped");
}

void Mainloop::addEventSource(int fd, uint32_t event, Callback &&callback)
{
	/* TODO: use scoped-lock to thread safe on member variables */
	if (m_callbacks.count(fd) != 0)
		throw std::logic_error("event source already added!");

	LogDebug("Add event[" << event << "] source on fd[" << fd << "]");

	epoll_event e;

	e.events = event;
	e.data.fd = fd;

	if (::epoll_ctl(m_pollfd, EPOLL_CTL_ADD, fd, &e) == -1)
		throw std::system_error(
			std::error_code(errno, std::generic_category()),
			"epoll_ctl failed to EPOLL_CTL_ADD.");

	m_callbacks[fd] = std::move(callback);
}

void Mainloop::removeEventSource(int fd)
{
	/* TODO: use scoped-lock to thread safe on member variables */
	if (m_callbacks.count(fd) == 0)
		throw std::logic_error("event source isn't added at all");

	LogDebug("Remove event source on fd[" << fd << "]");

	do {
		m_callbacks.erase(fd);

		if (::epoll_ctl(m_pollfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
			if (errno == ENOENT)
				throw std::logic_error("Tried to delete epoll item which wasn't added");
			else
				throw std::system_error(
					std::error_code(errno, std::generic_category()),
					"epoll_ctl failed to EPOLL_CTL_DEL.");
		}
	} while (false);
}

void Mainloop::dispatch(int timeout)
{
	int nfds = -1;
	epoll_event event[MAX_EPOLL_EVENTS];

	LogDebug("Mainloop dispatched with timeout: " << timeout);

	do {
		nfds = ::epoll_wait(m_pollfd, event, MAX_EPOLL_EVENTS, timeout);
	} while ((nfds == -1) && (errno == EINTR));

	if (nfds < 0)
		throw std::system_error(
			std::error_code(errno, std::generic_category()),
			"epoll_wait failed!");

	if (nfds == 0) {
		LogDebug("Mainloop timed out!");
		m_isTimedOut = true;
		return;
	}

	for (int i = 0; i < nfds; i++) {
		/* TODO: use scoped-lock to thread safe on member variables */
		int fd = event[i].data.fd;

		if (m_callbacks.count(fd) == 0)
			throw std::logic_error("event in, but associated callback isn't exist!");

		if (event[i].events & (EPOLLHUP | EPOLLRDHUP)) {
			LogInfo("peer connection closed on fd[" << fd << "]");
			event[i].events &= ~EPOLLIN;
		}

		LogDebug("event[" << event[i].events << "] polled on fd[" << fd << "]");

		m_callbacks[fd](event[i].events);
	}
}

} // namespace CCHECKER
