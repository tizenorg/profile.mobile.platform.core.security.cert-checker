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
/**
 * @file    serialization.h
 * @author  Tomasz Swierczek (t.swierczek@samsung.com)
 * @version 1.0
 * @brief   Interfaces and templates used for data serialization.
 */
#pragma once

#include <stdint.h>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <memory>
#include <sys/types.h>

#include "service/app.h"
#include "common/command-id.h"

namespace CCHECKER {

namespace Common {

// Abstract data stream buffer
class IStream {
public:
	virtual void read(size_t num, void *bytes) = 0;
	virtual void write(size_t num, const void *bytes) = 0;
	virtual ~IStream() {}
};

// Serializable interface
class ISerializable {
public:
	ISerializable() {}
	ISerializable(IStream &) {}
	virtual void Serialize(IStream &) const = 0;
	virtual ~ISerializable() {}
};

struct Serialization {
	// serialization
	// normal functions

	// ISerializable objects
	static void Serialize(IStream &stream, const ISerializable &object)
	{
		object.Serialize(stream);
	}

	static void Serialize(IStream &stream, const ISerializable *const object)
	{
		object->Serialize(stream);
	}

	// char
	static void Serialize(IStream &stream, const char value)
	{
		stream.write(sizeof(value), &value);
	}
	static void Serialize(IStream &stream, const char *const value)
	{
		stream.write(sizeof(*value), value);
	}

	// unsigned char
	static void Serialize(IStream &stream, const unsigned char value)
	{
		stream.write(sizeof(value), &value);
	}
	static void Serialize(IStream &stream, const unsigned char *const value)
	{
		stream.write(sizeof(*value), value);
	}

	// unsigned int32
	static void Serialize(IStream &stream, const uint32_t value)
	{
		stream.write(sizeof(value), &value);
	}
	static void Serialize(IStream &stream, const uint32_t *const value)
	{
		stream.write(sizeof(*value), value);
	}

	// int32
	static void Serialize(IStream &stream, const int32_t value)
	{
		stream.write(sizeof(value), &value);
	}
	static void Serialize(IStream &stream, const int32_t *const value)
	{
		stream.write(sizeof(*value), value);
	}

	// unsigned int64
	static void Serialize(IStream &stream, const uint64_t value)
	{
		stream.write(sizeof(value), &value);
	}
	static void Serialize(IStream &stream, const uint64_t *const value)
	{
		stream.write(sizeof(*value), value);
	}

	// int64
	static void Serialize(IStream &stream, const int64_t value)
	{
		stream.write(sizeof(value), &value);
	}
	static void Serialize(IStream &stream, const int64_t *const value)
	{
		stream.write(sizeof(*value), value);
	}

	// bool
	static void Serialize(IStream &stream, const bool value)
	{
		stream.write(sizeof(value), &value);
	}
	static void Serialize(IStream &stream, const bool *const value)
	{
		stream.write(sizeof(*value), value);
	}

	static void Serialize(IStream &stream, const CommandId value)
	{
		Serialize(stream, static_cast<int>(value));
	}

	// app_t
	static void Serialize(IStream &stream, const app_t app)
	{
		Serialize(stream, app.app_id);
		Serialize(stream, app.pkg_id);
		Serialize(stream, static_cast<uint32_t>(app.uid));
		Serialize(stream, app.signatures);
		Serialize(stream, static_cast<int>(app.verified));
	}
	static void Serialize(IStream &stream, const app_t *const p)
	{
		Serialize(stream, *p);
	}

	// std::string
	template <typename T, typename R, typename A>
	static void Serialize(IStream &stream, const std::basic_string<T, R, A> &str)
	{
		int length = str.size();
		stream.write(sizeof(length), &length);
		stream.write(length * sizeof(T), str.data());
	}

	template<typename T, typename R, typename A>
	static void Serialize(IStream &stream,
						  const std::basic_string<T, R, A> *const str)
	{
		int length = str->size();
		stream.write(sizeof(length), &length);
		stream.write(length * sizeof(T), str->data());
	}

	// STL templates

	// std::list
	template <typename T>
	static void Serialize(IStream &stream, const std::list<T> &list)
	{
		size_t length = list.size();
		stream.write(sizeof(length), &length);

		for (const auto &item : list)
			Serialize(stream, item);
	}
	template <typename T>
	static void Serialize(IStream &stream, const std::list<T> *const list)
	{
		Serialize(stream, *list);
	}

	template <typename T>
	static void Serialize(IStream &stream, const std::set<T> &set)
	{
		auto len = set.size();
		stream.write(sizeof(len), &len);

		for (const auto &item : set)
			Serialize(stream, item);
	}
	template <typename T>
	static void Serialize(IStream &stream, const std::set<T> *const set)
	{
		Serialize(stream, *set);
	}

	// RawBuffer
	template <typename A>
	static void Serialize(IStream &stream, const std::vector<unsigned char, A> &vec)
	{
		int length = vec.size();
		stream.write(sizeof(length), &length);
		stream.write(length, vec.data());
	}

	template <typename A>
	static void Serialize(IStream &stream,
						  const std::vector<unsigned char, A> *const vec)
	{
		Serialize(stream, *vec);
	}

	// std::vector
	template <typename T, typename A>
	static void Serialize(IStream &stream, const std::vector<T, A> &vec)
	{
		int length = vec.size();
		stream.write(sizeof(length), &length);

		for (const auto &i : vec)
			Serialize(stream, i);
	}
	template <typename T, typename A>
	static void Serialize(IStream &stream, const std::vector<T, A> *const vec)
	{
		Serialize(stream, *vec);
	}

	// std::pair
	template <typename A, typename B>
	static void Serialize(IStream &stream, const std::pair<A, B> &p)
	{
		Serialize(stream, p.first);
		Serialize(stream, p.second);
	}
	template <typename A, typename B>
	static void Serialize(IStream &stream, const std::pair<A, B> *const p)
	{
		Serialize(stream, *p);
	}

	// std::map
	template <typename K, typename T>
	static void Serialize(IStream &stream, const std::map<K, T> &map)
	{
		size_t length = map.size();
		stream.write(sizeof(length), &length);

		for (const auto &item : map) {
			Serialize(stream, item.first);
			Serialize(stream, item.second);
		}
	}
	template <typename K, typename T>
	static void Serialize(IStream &stream, const std::map<K, T> *const map)
	{
		Serialize(stream, *map);
	}

	// std::unordered_map
	template <typename K, typename T>
	static void Serialize(IStream &stream, const std::unordered_map<K, T> &map)
	{
		size_t length = map.size();
		stream.write(sizeof(length), &length);

		for (const auto &item : map) {
			Serialize(stream, item.first);
			Serialize(stream, item.second);
		}
	}
	template <typename K, typename T>
	static void Serialize(IStream &stream,
						  const std::unordered_map<K, T> *const map)
	{
		Serialize(stream, *map);
	}

	// std::unique_ptr
	template <typename T>
	static void Serialize(IStream &stream, const std::unique_ptr<T> &p)
	{
		Serialize(stream, *p);
	}

	// std::shared_ptr
	template <typename T>
	static void Serialize(IStream &stream, const std::shared_ptr<T> &p)
	{
		Serialize(stream, *p);
	}
}; // struct Serialization

struct Deserialization {
	// deserialization
	// normal functions

	// ISerializable objects
	// T instead of ISerializable is needed to call proper constructor
	template <typename T>
	static void Deserialize(IStream &stream, T &object)
	{
		object = T(stream);
	}
	template <typename T>
	static void Deserialize(IStream &stream, T *&object)
	{
		object = new T(stream);
	}

	template <typename T>
	static void Deserialize(IStream &stream, std::shared_ptr<T> &object)
	{
		object.reset(new T(stream));
	}

	// char
	static void Deserialize(IStream &stream, char &value)
	{
		stream.read(sizeof(value), &value);
	}
	static void Deserialize(IStream &stream, char *&value)
	{
		value = new char;
		stream.read(sizeof(*value), value);
	}

	// unsigned char
	static void Deserialize(IStream &stream, unsigned char &value)
	{
		stream.read(sizeof(value), &value);
	}
	static void Deserialize(IStream &stream, unsigned char *&value)
	{
		value = new unsigned char;
		stream.read(sizeof(*value), value);
	}

	// unsigned int32
	static void Deserialize(IStream &stream, uint32_t &value)
	{
		stream.read(sizeof(value), &value);
	}
	static void Deserialize(IStream &stream, uint32_t *&value)
	{
		value = new uint32_t;
		stream.read(sizeof(*value), value);
	}

	// int32
	static void Deserialize(IStream &stream, int32_t &value)
	{
		stream.read(sizeof(value), &value);
	}
	static void Deserialize(IStream &stream, int32_t *&value)
	{
		value = new int32_t;
		stream.read(sizeof(*value), value);
	}

	// unsigned int64
	static void Deserialize(IStream &stream, uint64_t &value)
	{
		stream.read(sizeof(value), &value);
	}
	static void Deserialize(IStream &stream, uint64_t *&value)
	{
		value = new uint64_t;
		stream.read(sizeof(*value), value);
	}

	// int64
	static void Deserialize(IStream &stream, int64_t &value)
	{
		stream.read(sizeof(value), &value);
	}
	static void Deserialize(IStream &stream, int64_t *&value)
	{
		value = new int64_t;
		stream.read(sizeof(*value), value);
	}

	// bool
	static void Deserialize(IStream &stream, bool &value)
	{
		stream.read(sizeof(value), &value);
	}

	static void Deserialize(IStream &stream, CommandId &value)
	{
		int val;
		Deserialize(stream, val);
		value = static_cast<CommandId>(val);
	}

	// app_t
	static void Deserialize(IStream &stream, app_t &app)
	{
		Deserialize(stream, app.app_id);
		Deserialize(stream, app.pkg_id);

		uint32_t uid;
		Deserialize(stream, uid);
		app.uid = static_cast<uid_t>(uid);

		Deserialize(stream, app.signatures);

		int val;
		Deserialize(stream, val);
		app.verified = static_cast<app_t::verified_t>(val);
	}
	static void Deserialize(IStream &stream, app_t *&p)
	{
		p = new app_t();
		Deserialize(stream, *p);
	}

	static void Deserialize(IStream &stream, bool *&value)
	{
		value = new bool;
		stream.read(sizeof(*value), value);
	}

	template <typename T, typename R, typename A>
	static void Deserialize(IStream &stream, std::basic_string<T, R, A> &str)
	{
		int length;
		stream.read(sizeof(length), &length);
		std::vector<T> buf(length);
		stream.read(length * sizeof(T), buf.data());
		str = std::basic_string<T, R, A>(buf.data(), buf.data() + length);
	}

	template <typename T, typename R, typename A>
	static void Deserialize(IStream &stream, std::basic_string<T, R, A> *&str)
	{
		int length;
		stream.read(sizeof(length), &length);
		std::vector<T> buf(length);
		stream.read(length * sizeof(T), buf.data());
		str = new std::basic_string<T, R, A>(buf.data(), buf.data() + length);
	}

	// STL templates

	// std::list
	template <typename T>
	static void Deserialize(IStream &stream, std::list<T> &list)
	{
		int length;
		stream.read(sizeof(length), &length);

		for (int i = 0; i < length; ++i) {
			T obj;
			Deserialize(stream, obj);
			list.push_back(std::move(obj));
		}
	}
	template <typename T>
	static void Deserialize(IStream &stream, std::list<T> *&list)
	{
		list = new std::list<T>;
		Deserialize(stream, *list);
	}

	template <typename T>
	static void Deserialize(IStream &stream, std::set<T> &set)
	{
		size_t len;
		stream.read(sizeof(len), &len);

		for (size_t i = 0; i < len; ++i) {
			T obj;
			Deserialize(stream, obj);
			set.insert(std::move(obj));
		}
	}
	template <typename T>
	static void Deserialize(IStream &stream, std::set<T> *&set)
	{
		set = new std::set<T>;
		Deserialize(stream, *set);
	}

	// RawBuffer
	template <typename A>
	static void Deserialize(IStream &stream, std::vector<unsigned char, A> &vec)
	{
		int length;
		stream.read(sizeof(length), &length);
		vec.resize(length);
		stream.read(length, vec.data());
	}

	template <typename A>
	static void Deserialize(IStream &stream, std::vector<unsigned char, A> *&vec)
	{
		vec = new std::vector<unsigned char, A>;
		Deserialize(stream, *vec);
	}

	// std::vector
	template <typename T, typename A>
	static void Deserialize(IStream &stream, std::vector<T, A> &vec)
	{
		int length;
		stream.read(sizeof(length), &length);

		for (int i = 0; i < length; ++i) {
			T obj;
			Deserialize(stream, obj);
			vec.push_back(std::move(obj));
		}
	}
	template <typename T, typename A>
	static void Deserialize(IStream &stream, std::vector<T, A> *&vec)
	{
		vec = new std::vector<T, A>;
		Deserialize(stream, *vec);
	}

	// std::pair
	template <typename A, typename B>
	static void Deserialize(IStream &stream, std::pair<A, B> &p)
	{
		Deserialize(stream, p.first);
		Deserialize(stream, p.second);
	}
	template <typename A, typename B>
	static void Deserialize(IStream &stream, std::pair<A, B> *&p)
	{
		p = new std::pair<A, B>;
		Deserialize(stream, *p);
	}

	// std::map
	template <typename K, typename T>
	static void Deserialize(IStream &stream, std::map<K, T> &map)
	{
		int length;
		stream.read(sizeof(length), &length);

		for (int i = 0; i < length; ++i) {
			K key;
			T obj;
			Deserialize(stream, key);
			Deserialize(stream, obj);
			map[key] = std::move(obj);
		}
	}
	template <typename K, typename T>
	static void Deserialize(IStream &stream, std::map<K, T> *&map)
	{
		map = new std::map<K, T>;
		Deserialize(stream, *map);
	}

	// std::unordered_map
	template <typename K, typename T>
	static void Deserialize(IStream &stream, std::unordered_map<K, T> &map)
	{
		size_t length;
		stream.read(sizeof(length), &length);

		for (size_t i = 0; i < length; ++i) {
			K key;
			T obj;
			Deserialize(stream, key);
			Deserialize(stream, obj);
			map[key] = std::move(obj);
		}
	}
	template <typename K, typename T>
	static void Deserialize(IStream &stream, std::unordered_map<K, T> *&map)
	{
		map = new std::map<K, T>;
		Deserialize(stream, *map);
	}
}; // struct Deserialization

// generic serialization
template <typename... Args>
struct Serializer;

template <typename First, typename... Args>
struct Serializer<First, Args...> : public Serializer<Args...> {
	static void Serialize(IStream &stream, const First &f, const Args &... args)
	{
		Serialization::Serialize(stream, f);
		Serializer<Args...>::Serialize(stream, args...);
	}
};

// end of recursion
template <>
struct Serializer<> {
	static void Serialize(IStream &)
	{
		return;
	}
};

// generic deserialization
template <typename... Args>
struct Deserializer;

template <typename First, typename... Args>
struct Deserializer<First, Args...> : public Deserializer<Args...> {
	static void Deserialize(IStream &stream, First &f, Args &... args)
	{
		Deserialization::Deserialize(stream, f);
		Deserializer<Args...>::Deserialize(stream, args...);
	}
};

// end of recursion
template <>
struct Deserializer<> {
	static void Deserialize(IStream &)
	{
		return;
	}
};

} // namespace Common
} // namespace CCHECKER
