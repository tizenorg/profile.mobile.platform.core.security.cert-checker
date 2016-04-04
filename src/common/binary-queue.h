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
 * @file        binary-queue.h
 * @author      Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 *              Kyungwook Tak (k.tak@samsung.com)
 * @version     1.0
 * @brief       This file is the header file of binary queue
 */
#pragma once

#include <queue>
#include <memory>
#include <cstddef>

#include "common/serialization.h"
#include "common/types.h"

using namespace CCHECKER::Common;

namespace CCHECKER {

class BinaryQueue : public IStream {
public:
	BinaryQueue();
	virtual ~BinaryQueue();

	BinaryQueue(BinaryQueue &&) = default;
	BinaryQueue &operator=(BinaryQueue &&) = default;
	BinaryQueue(const BinaryQueue &) = delete;
	BinaryQueue &operator=(const BinaryQueue &) = delete;

	RawBuffer pop(void);
	void push(const RawBuffer &);

	template <typename ...Args>
	static BinaryQueue Serialize(const Args &...args);

	template <typename ...Args>
	void Deserialize(Args &...args);

	virtual void read(size_t num, void *bytes) override;
	virtual void write(size_t num, const void *bytes) override;

private:
	const static size_t MaxBucketSize = 1024; /* Bytes */

	struct Bucket {
		explicit Bucket(unsigned char *_data, size_t _size);
		virtual ~Bucket();

		/* extract ''size'' of bytes from bucket to dest and return updated dest */
		void *extractTo(void *dest, size_t size);

		Bucket(Bucket &&) = default;
		Bucket &operator=(Bucket &&) = default;
		Bucket(const Bucket &) = delete;
		Bucket &operator=(const Bucket &) = delete;

		unsigned char *data;
		const unsigned char *cur; // current valid position of data
		size_t left;
	};

	std::queue<std::unique_ptr<Bucket>> m_buckets;
	size_t m_size;
};

template <typename ...Args>
BinaryQueue BinaryQueue::Serialize(const Args &...args)
{
	BinaryQueue q;
	Serializer<Args...>::Serialize(q, args...);
	return q;
}

template <typename ...Args>
void BinaryQueue::Deserialize(Args &...args)
{
	Deserializer<Args...>::Deserialize(*this, args...);
}

} // namespace CCHECKER
