// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 06/22/11
// Description: optimized bitmap

#ifndef COMMON_COLLECTION_BITMAP_HPP
#define COMMON_COLLECTION_BITMAP_HPP
#pragma once

#include <stddef.h>
#include <climits>
#include <cstring>
#include <vector>
#include "common/base/stdint.h"

bool BitmapGet(const void* p, uint32_t offset);
void BitmapSet(const void* p, uint32_t offset, bool value);
bool BitmapTestAndSet(const void* p, uint32_t offset, bool value);
bool BitmapTestAndSet(const void* p, uint32_t offset, bool value);

template <typename CocreteType, typename IndexType>
class BitmapBase
{
public:
	bool Get(IndexType index) const
	{
		size_t offset = index / CHAR_BIT;
		unsigned int mask = 1U << (index % CHAR_BIT);
		return (static_cast<const CocreteType*>(this)->GetBits()[offset] & mask) != 0;
	}
	void Clear(IndexType index)
	{
		size_t offset = index / CHAR_BIT;
		unsigned int mask = ~(1U << (index % CHAR_BIT));
		static_cast<CocreteType*>(this)->GetBits()[offset] &= mask;
	}
	void Set(IndexType index)
	{
		size_t offset = index / CHAR_BIT;
		unsigned int mask = 1U << (index % CHAR_BIT);
		static_cast<CocreteType*>(this)->GetBits()[offset] |= mask;
	}
	void Set(IndexType index, bool value)
	{
		if (value)
			Set(index);
		else
			Clear(index);
	}
};

// using size_t as default bitmap index type
template <bool UseLongLong>
struct SelectLargeIndexType
{
	typedef size_t Type;
};

template <>
struct SelectLargeIndexType<true>
{
	typedef unsigned long long Type;
};

template <unsigned long long Size>
struct SelectIndexTypeBySize
{
	static const bool UseLargeIndexType = sizeof(size_t) != sizeof(unsigned long long) && Size >= 0xFFFFFFFF;
	typedef typename SelectLargeIndexType<UseLargeIndexType>::Type Type;
};

template <unsigned long long Size, typename IndexType = typename SelectIndexTypeBySize<Size>::Type >
class FixedBitmap : public BitmapBase<FixedBitmap<Size, IndexType>, IndexType>
{
	friend class BitmapBase<FixedBitmap<Size, IndexType>, IndexType>;
public:
	FixedBitmap()
	{
		memset(m_bits, 0, sizeof(m_bits));
	}
private:
	const unsigned char* GetBits() const
	{
		return m_bits;
	}
	unsigned char* GetBits()
	{
		return m_bits;
	}
private:
	unsigned char m_bits[(Size + CHAR_BIT - 1) / CHAR_BIT];
};

/// dynamic bitmap
template <typename IndexType = size_t>
class Bitmap : public BitmapBase<Bitmap<IndexType>, IndexType>
{
	friend class BitmapBase<Bitmap<IndexType>, IndexType>;
public:
	Bitmap(IndexType size) : m_bits((size + CHAR_BIT - 1) / CHAR_BIT)
	{
	}
	void Resize(IndexType size)
	{
		m_bits.resize((size + CHAR_BIT - 1) / CHAR_BIT);
	}
private:
	const unsigned char* GetBits() const
	{
		return &m_bits[0];
	}
	unsigned char* GetBits()
	{
		return &m_bits[0];
	}
private:
	std::vector<unsigned char> m_bits;
};

typedef Bitmap<unsigned long long> LargeBitmap;

#endif//BITMAP_HPP_INCLUDED
