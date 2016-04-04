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
 * @file        socket.cpp
 * @author      Kyungwook Tak (k.tak@samsung.com)
 * @version     1.0
 * @brief
 */
#include "common/socket.h"

#include <exception>
#include <system_error>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include <systemd/sd-daemon.h>

#include <cchecker/log.h>

namespace {

int createSystemdSocket(const std::string &path)
{
	int n = ::sd_listen_fds(-1);

	if (n < 0)
		throw std::system_error(std::error_code(), "failed to sd_listen_fds");

	for (int fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + n; ++fd) {
		if (::sd_is_socket_unix(fd, SOCK_STREAM, 1, path.c_str(), 0) > 0) {
			LogInfo("service's systemd socket found with fd: " << fd);
			return fd;
		}
	}

	throw std::system_error(std::error_code(), "get systemd socket failed!");
}

} // namespace anonymous

namespace CCHECKER {

Socket::Socket(int fd) : m_fd(fd)
{
	if (m_fd < 0)
		throw std::logic_error("Socket fd from constructor is invalid!!");
}

Socket::Socket(const std::string &path) : m_fd(createSystemdSocket(path))
{
}

Socket::Socket(Socket &&other)
{
	if (other.m_fd < 0)
		throw std::logic_error("Socket fd from move constructor is invalid!!");

	m_fd = other.m_fd;
	other.m_fd = 0;
}

Socket &Socket::operator=(Socket &&other)
{
	if (this == &other)
		return *this;

	if (other.m_fd < 0)
		throw std::logic_error("Socket fd from move assignment is invalid!!");

	m_fd = other.m_fd;
	other.m_fd = 0;

	return *this;
}

Socket::~Socket()
{
	if (m_fd == 0)
		return;

	LogInfo("Close socket of fd: " << m_fd);
	::close(m_fd);
}

Socket Socket::accept() const
{
	int fd = ::accept(m_fd, nullptr, nullptr);

	if (fd < 0)
		throw std::system_error(
			std::error_code(errno, std::generic_category()),
			"socket accept failed!");

	LogInfo("Accept client success with fd: " << fd);

	return Socket(fd);
}

Socket Socket::connect(const std::string &path)
{
	if (path.size() >= sizeof(sockaddr_un::sun_path))
		throw std::invalid_argument("socket path size too long!");

	int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);

	if (fd < 0)
		throw std::system_error(
			std::error_code(errno, std::generic_category()),
			"socket create failed!");

	sockaddr_un addr;
	addr.sun_family = AF_UNIX;

	strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path));

	if (::connect(fd, reinterpret_cast<sockaddr *>(&addr),
				  sizeof(sockaddr_un)) == -1)
		throw std::system_error(
			std::error_code(errno, std::generic_category()),
			"socket connect failed!");

	LogInfo("Connect to CSR server success with fd:" << fd);

	return Socket(fd);
}

int Socket::getFd() const
{
	return m_fd;
}

RawBuffer Socket::read(void) const
{
	size_t total = 0;

	RawBuffer data(1024, 0);
	auto buf = reinterpret_cast<char *>(data.data());
	auto size = data.size();

	LogDebug("Read data from stream on socket fd[" << m_fd << "]");

	while (total < size) {
		int bytes = ::read(m_fd, buf + total, size - total);

		if (bytes < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				continue;
			else
				throw std::system_error(
					std::error_code(errno, std::generic_category()),
					"socket read failed!");
		}

		/* TODO: handle the case of more bytes in stream to read than buffer size */
		total += bytes;
		break;
	}

	data.resize(total);

	LogDebug("Read data of size[" << total
		  << "] from stream on socket fd[" << m_fd << "] done.");

	return data;
}

void Socket::write(const RawBuffer &data) const
{
	size_t total = 0;

	auto buf = reinterpret_cast<const char *>(data.data());
	auto size = data.size();

	LogDebug("Write data to stream on socket fd[" << m_fd << "]");

	while (total < size) {
		int bytes = ::write(m_fd, buf + total, size - total);

		if (bytes < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				continue;
			else
				throw std::system_error(
					std::error_code(errno, std::generic_category()),
					"socket write failed!");
		}

		total += bytes;
	}

	LogDebug("Write data of size[" << total <<
		  "] to stream on socket fd[" << m_fd << "] done.");
}

} // namespace CCHECKER
