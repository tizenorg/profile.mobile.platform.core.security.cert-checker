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
 * @file        connection.cpp
 * @author      Kyungwook Tak (k.tak@samsung.com)
 * @version     1.0
 * @brief
 */
#include "common/connection.h"

#include <utility>

namespace CCHECKER {

Connection::Connection(Socket &&socket) : m_socket(std::move(socket))
{
}

Connection::~Connection()
{
}

Connection::Connection(Connection &&other) : m_socket(std::move(other.m_socket))
{
}

Connection &Connection::operator=(Connection &&other)
{
	if (this == &other)
		return *this;

	m_socket = std::move(other.m_socket);
	return *this;
}

void Connection::send(const RawBuffer &buf) const
{
	std::lock_guard<std::mutex> lock(m_mSend);
	m_socket.write(buf);
}

RawBuffer Connection::receive() const
{
	std::lock_guard<std::mutex> lock(m_mRecv);
	return m_socket.read();
}

int Connection::getFd() const
{
	return m_socket.getFd();
}

}
