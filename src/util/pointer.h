// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

#pragma once

#include "irrlichttypes.h"
#include "debug.h" // For assert()
#include <cstring>
#include <cstdlib>
#include <memory> // std::shared_ptr
#include <string_view>


template<typename T> class ConstSharedPtr {
public:
	ConstSharedPtr(T *ptr) : ptr(ptr) {}
	ConstSharedPtr(const std::shared_ptr<T> &ptr) : ptr(ptr) {}

	const T* get() const noexcept { return ptr.get(); }
	const T& operator*() const noexcept { return *ptr.get(); }
	const T* operator->() const noexcept { return ptr.get(); }

private:
	std::shared_ptr<T> ptr;
};

template <typename T>
class Buffer
{
public:
	Buffer()
	{
		m_size = 0;
		data = nullptr;
	}
	Buffer(u32 size)
	{
		m_size = size;
		if (size != 0) {
			data = new T[size];
		} else {
			data = nullptr;
		}
	}

	// Disable class copy
	Buffer(const Buffer &) = delete;
	Buffer &operator=(const Buffer &) = delete;

	Buffer(Buffer &&buffer)
	{
		m_size = buffer.m_size;
		if (m_size != 0) {
			data = buffer.data;
			buffer.data = nullptr;
			buffer.m_size = 0;
		} else {
			data = nullptr;
		}
	}
	// Copies whole buffer
	Buffer(const T *t, u32 size)
	{
		m_size = size;
		if (size != 0) {
			data = new T[size];
			memcpy(data, t, sizeof(T) * size);
		} else {
			data = nullptr;
		}
	}

	~Buffer()
	{
		drop();
	}

	Buffer& operator=(Buffer &&buffer)
	{
		if (this == &buffer) {
			return *this;
		}
		drop();
		m_size = buffer.m_size;
		if (m_size != 0) {
			data = buffer.data;
			buffer.data = nullptr;
			buffer.m_size = 0;
		} else {
			data = nullptr;
		}
		return *this;
	}

	void copyTo(Buffer &buffer) const
	{
		buffer.drop();
		buffer.m_size = m_size;
		if (m_size != 0) {
			buffer.data = new T[m_size];
			memcpy(buffer.data, data, sizeof(T) * m_size);
		} else {
			buffer.data = nullptr;
		}
	}

	T & operator[](u32 i) const
	{
		return data[i];
	}
	T * operator*() const
	{
		return data;
	}

	u32 getSize() const
	{
		return m_size;
	}

	operator std::string_view() const
	{
		if (!data) {
			return std::string_view();
		}
		return std::string_view(reinterpret_cast<char*>(data), m_size);
	}

private:
	void drop()
	{
		delete[] data;
	}
	T *data;
	u32 m_size;
};

/************************************************
 *           !!!  W A R N I N G  !!!            *
 *                                              *
 * This smart pointer class is NOT thread safe. *
 * ONLY use in a single-threaded context!       *
 *                                              *
 ************************************************/
template <typename T>
class SharedBuffer
{
public:
	SharedBuffer()
	{
		m_size = 0;
		m_mem = nullptr;
	}

	SharedBuffer(u32 size)
	{
		m_size = size;
		m_mem = (Mem *)calloc(1, sizeof(*m_mem) + sizeof(T) * m_size);
		m_mem->refcount = 1;
	}

	SharedBuffer(const SharedBuffer &buffer)
	{
		m_size = buffer.m_size;
		m_mem = buffer.m_mem;
		++m_mem->refcount;
	}

	SharedBuffer & operator=(const SharedBuffer & buffer)
	{
		if (this == &buffer) {
			return *this;
		}
		drop();
		m_size = buffer.m_size;
		m_mem = buffer.m_mem;
		++m_mem->refcount;
		return *this;
	}

	//! Copies whole buffer
	SharedBuffer(const T *t, u32 size)
	{
		if(t) {
			m_size = size;
			m_mem = (Mem *)malloc(sizeof(*m_mem) + sizeof(T) * m_size);
			m_mem->refcount = 1;
			memcpy(m_mem->data, t, sizeof(T) * m_size);
		} else {
			m_size = 0;
			m_mem = nullptr;
		}
	}

	//! Copies whole buffer
	SharedBuffer(const Buffer<T> &buffer) : SharedBuffer(*buffer, buffer.getSize())
	{
	}

	~SharedBuffer()
	{
		drop();
	}

	T & operator[](u32 i) const
	{
		assert(m_mem && i < m_size);
		return m_mem->data[i];
	}

	T * operator*() const
	{
		assert(m_mem);
		return m_mem->data;
	}

	u32 getSize() const
	{
		return m_size;
	}

	operator Buffer<T>() const
	{
		return Buffer<T>(m_mem->data, m_size);
	}

	private:
	void drop()
	{
		if(!m_mem) {
			return;
		}
		assert(m_mem->refcount > 0);
		--m_mem->refcount;
		if (m_mem->refcount == 0) {
			free(m_mem);
			m_mem = nullptr;
			m_size = 0;
		}
	}

	struct Mem {
		u32 refcount;
		T data[];
	} *m_mem;

	u32 m_size;
};

// This class is not thread-safe!
class IntrusiveReferenceCounted {
public:
	IntrusiveReferenceCounted() = default;
	virtual ~IntrusiveReferenceCounted() = default;
	void grab() noexcept { ++m_refcount; }
	void drop() noexcept { if (--m_refcount == 0) delete this; }

	DISABLE_CLASS_COPY(IntrusiveReferenceCounted)
private:
	u32 m_refcount = 1;
};
