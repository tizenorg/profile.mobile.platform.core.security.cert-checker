/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file        binary-queue.cpp
 * @author      Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 *              Kyungwook Tak (k.tak@samsung.com)
 * @version     1.0
 * @brief
 */
#include "common/binary-queue.h"

#include <cstring>
#include <stdexcept>
#include <new>

namespace CCHECKER {

BinaryQueue::BinaryQueue() : m_size(0)
{
}

BinaryQueue::~BinaryQueue()
{
}

RawBuffer BinaryQueue::pop()
{
	RawBuffer buf(m_size);

	read(m_size, buf.data());

	return buf;
}

void BinaryQueue::push(const RawBuffer &data)
{
	write(data.size(), data.data());
}

void BinaryQueue::write(size_t size, const void *bytes)
{
	while (size > 0) {
		auto s = (size > MaxBucketSize) ? MaxBucketSize : size;
		auto b = new unsigned char[s];
		memcpy(b, bytes, s);
		m_buckets.emplace(new Bucket(b, s));
		m_size += s;
		size -= s;
	}
}

void BinaryQueue::read(size_t size, void *bytes)
{
	if (size == 0)
		return;

	if (size > m_size)
		throw std::logic_error("protocol broken. no more binary to flatten in queue");

	void *cur = bytes;

	while (size > 0) {
		if (m_buckets.empty())
			throw std::logic_error("protocol broken. no more buckets to extract");

		size_t count = std::min(size, m_buckets.front()->left);
		cur = m_buckets.front()->extractTo(cur, count);

		size -= count;
		m_size -= count;

		if (m_buckets.front()->left == 0)
			m_buckets.pop();
	}
}

BinaryQueue::Bucket::Bucket(unsigned char *_data, size_t _size) :
	data(_data),
	cur(_data),
	left(_size)
{
	if (_data == nullptr || _size == 0)
		throw std::invalid_argument("Bucket construct failed.");
}

BinaryQueue::Bucket::~Bucket()
{
	delete []data;
}

void *BinaryQueue::Bucket::extractTo(void *dest, size_t size)
{
	if (dest == nullptr || size == 0)
		throw std::logic_error("logic error. invalid input to Bucket::extractTo.");

	memcpy(dest, cur, size);

	cur += size;
	left -= size;

	return static_cast<unsigned char *>(dest) + size;
}

} // namespace CCHECKER
