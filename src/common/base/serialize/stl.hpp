// Copyright (c) 2008, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef SERIALIZE_STL_HPP_INCLUDED
#define SERIALIZE_STL_HPP_INCLUDED

// Add serialization support for STL contailers

#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>

#ifdef __GNUC__

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= 40403
 #pragma push_macro("__DEPRECATED")
#else
 #ifdef __DEPRECATED
  #define __DEPRECATED_DEFINED
 #endif
#endif

#undef __DEPRECATED

#include <ext/hash_map>
#include <ext/hash_set>

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= 40403
 #pragma pop_macro("__DEPRECATED")
#else
 #ifdef __DEPRECATED_DEFINED
  #define __DEPRECATED
 #endif
#endif

#endif

#ifdef _MSC_VER
#include <hash_set>
#include <hash_map>
#endif

#include <common/base/serialize/base.hpp>

namespace Serialize
{

// overload for container
template <typename EncoderType, typename Container>
void EncodeContainer(EncoderType& encoder, const Container& value, const char* name)
{
    size_t size = value.size();
    encoder.EncodeListBegin(size, name);
    for (typename Container::const_iterator i = value.begin(); i != value.end(); ++i)
        Encode(encoder, *i);
    encoder.EncodeListEnd();
}

template <typename DecoderType, typename Container>
bool DecodeContainer(DecoderType& decoder, Container& value, const char* name)
{
    typename DecoderType::Result result(decoder);

    value.clear();

    size_t size = 0;
    if (!decoder.DecodeListBegin(size, name))
        return false;

    typename Container::value_type element;
    for (size_t i = 0; i < size; ++i)
    {
        if (!Decode(decoder, element))
            return false;
        value.push_back(element);
    }

    result = decoder.DecodeListEnd();

    return result;
}

template <typename EncoderType, typename Type, typename Allocator>
void Encode(EncoderType& encoder, const std::vector<Type, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

// overload for vector
template <typename DecoderType, typename Type, typename Allocator>
bool Decode(DecoderType& decoder, std::vector<Type, Allocator>& value, const char* name)
{
    typename DecoderType::Result result(decoder);

    value.clear();

    size_t size = 0;
    if (!decoder.DecodeListBegin(size, name))
        return false;

    value.resize(size);
    for (size_t i = 0; i < size; ++i)
    {
        if (!Decode(decoder, value[i]))
            return false;
    }

    result = decoder.DecodeListEnd();

    return result;
}

template <typename EncoderType, typename Type, typename Allocator>
void Encode(EncoderType& encoder, const std::deque<Type, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Type, typename Allocator>
bool Decode(DecoderType& decoder, std::deque<Type, Allocator>& value, const char* name)
{
    return DecodeContainer(decoder, value, name);
}

template <typename EncoderType, typename Type, typename Allocator>
void Encode(EncoderType& encoder, const std::list<Type, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Type, typename Allocator>
bool Decode(DecoderType& decoder, std::list<Type, Allocator>& value, const char* name)
{
    return DecodeContainer(decoder, value, name);
}

template <typename DecoderType, typename Container>
bool DecodeSet(DecoderType& decoder, Container& value, const char* name)
{
    typename DecoderType::Result result(decoder);

    value.clear();
    size_t size = 0;
    if (!decoder.DecodeListBegin(size, name))
        return false;

    typename Container::value_type element;
    for (size_t i = 0; i < size; ++i)
    {
        if (!Decode(decoder, element))
            return false;
        value.insert(element);
    }

    result = decoder.DecodeListEnd();

    return result;
}


// overload for set and multiset
template <typename EncoderType, typename Type, typename Traits, typename Allocator>
void Encode(EncoderType& encoder, const std::set<Type, Traits, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename EncoderType, typename Type, typename Traits, typename Allocator>
void Encode(EncoderType& encoder, const std::multiset<Type, Traits, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Type, typename Traits, typename Allocator>
bool Decode(DecoderType& decoder, std::set<Type, Traits, Allocator>& value, const char* name)
{
    return DecodeSet(decoder, value, name);
}

template <typename DecoderType, typename Type, typename Traits, typename Allocator>
bool Decode(DecoderType& decoder, std::multiset<Type, Traits, Allocator>& value, const char* name)
{
    return DecodeSet(decoder, value, name);
}

// overload for map

// serialization for pair
template <typename EncoderType, typename First, typename Second>
void Encode(EncoderType& encoder, const std::pair<First, Second>& value, const char* name)
{
    encoder.EncodeRecordBegin("pair", name);
    Encode(encoder, value.first, "first");
    Encode(encoder, value.second, "second");
    encoder.EncodeRecordEnd("pair");
}

template <typename DecoderType, typename First, typename Second>
bool Decode(DecoderType& decoder, std::pair<First, Second>& value, const char* name)
{
    typename DecoderType::Result result(decoder);
    result =
        decoder.DecodeRecordBegin("pair", name) &&
        Decode(decoder, value.first, "first") &&
        Decode(decoder, value.second, "second") &&
        decoder.DecodeRecordEnd("pair");
    return result;
}

template <typename EncoderType, typename Key, typename Value, typename Traits, typename Allocator>
void Encode(EncoderType& encoder, const std::map<Key, Value, Traits, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename EncoderType, typename Key, typename Value, typename Traits, typename Allocator>
void Encode(EncoderType& encoder, const std::multimap<Key, Value, Traits, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Container>
bool DecodeMap(DecoderType& decoder, Container& value, const char* name)
{
    typename DecoderType::Result result(decoder);

    value.clear();
    size_t size = 0;
    if (!decoder.DecodeListBegin(size, name))
        return false;

    std::pair<typename Container::key_type, typename Container::mapped_type> element;
    for (size_t i = 0; i < size; ++i)
    {
        if (!Decode(decoder, element))
            return false;
        value.insert(element);
    }

    result = decoder.DecodeListEnd();

    return result;
}

// std::map & std::hash_multimap
template <typename DecoderType, typename Key, typename Value, typename Traits, typename Allocator>
bool Decode(DecoderType& decoder, std::map<Key, Value, Traits, Allocator>& value, const char* name)
{
    return DecodeMap(decoder, value, name);
}

template <typename DecoderType, typename Key, typename Value, typename Traits, typename Allocator>
bool Decode(DecoderType& decoder, std::multimap<Key, Value, Traits, Allocator>& value, const char* name)
{
    return DecodeMap(decoder, value, name);
}

// for hash containers
#ifdef __GNUC__

// hash_set
template <typename EncoderType, typename Type, typename Hash, typename EqualKey, typename Allocator>
void Encode(EncoderType& encoder, const __gnu_cxx::hash_set<Type, Hash, EqualKey, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Type, typename Hash, typename EqualKey, typename Allocator>
bool Decode(DecoderType& decoder, __gnu_cxx::hash_set<Type, Hash, EqualKey, Allocator>& value, const char* name)
{
    return DecodeSet(decoder, value, name);
}


// hash_multiset
template <typename EncoderType, typename Type, typename Hash, typename EqualKey, typename Allocator>
void Encode(EncoderType& encoder, const __gnu_cxx::hash_multiset<Type, Hash, EqualKey, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Type, typename Hash, typename EqualKey, typename Allocator>
bool Decode(DecoderType& decoder, __gnu_cxx::hash_multiset<Type, Hash, EqualKey, Allocator>& value, const char* name)
{
    return DecodeSet(decoder, value, name);
}

// hash_map
template <typename EncoderType, typename Key, typename Value, typename Hash, typename EqualKey, typename Allocator>
void Encode(EncoderType& encoder, const __gnu_cxx::hash_map<Key, Value, Hash, EqualKey, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Key, typename Value, typename Hash, typename EqualKey, typename Allocator>
bool Decode(DecoderType& decoder, __gnu_cxx::hash_map<Key, Value, Hash, EqualKey, Allocator>& value, const char* name)
{
    return DecodeMap(decoder, value, name);
}

// hash_multimap
template <typename EncoderType, typename Key, typename Value, typename Hash, typename EqualKey, typename Allocator>
void Encode(EncoderType& encoder, const __gnu_cxx::hash_multimap<Key, Value, Hash, EqualKey, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Key, typename Value, typename Hash, typename EqualKey, typename Allocator>
bool Decode(DecoderType& decoder, __gnu_cxx::hash_multimap<Key, Value, Hash, EqualKey, Allocator>& value, const char* name)
{
    return DecodeMap(decoder, value, name);
}
#endif

#ifdef _MSC_VER

// hash_set
template <typename EncoderType, typename Type, typename Traits, typename Allocator>
void Encode(EncoderType& encoder, const stdext::hash_set<Type, Traits, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Type, typename Traits, typename Allocator>
bool Decode(DecoderType& decoder, stdext::hash_set<Type, Traits, Allocator>& value, const char* name)
{
    return DecodeSet(decoder, value, name);
}


// hash_multiset
template <typename EncoderType, typename Type, typename Traits, typename Allocator>
void Encode(EncoderType& encoder, const stdext::hash_multiset<Type, Traits, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Type, typename Traits, typename Allocator>
bool Decode(DecoderType& decoder, stdext::hash_multiset<Type, Traits, Allocator>& value, const char* name)
{
    return DecodeSet(decoder, value, name);
}

// hash_map
template <typename EncoderType, typename Key, typename Value, typename Traits, typename Allocator>
void Encode(EncoderType& encoder, const stdext::hash_map<Key, Value, Traits, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Key, typename Value, typename Traits, typename Allocator>
bool Decode(DecoderType& decoder, stdext::hash_map<Key, Value, Traits, Allocator>& value, const char* name)
{
    return DecodeMap(decoder, value, name);
}

// hash_multimap
template <typename EncoderType, typename Key, typename Value, typename Traits, typename Allocator>
void Encode(EncoderType& encoder, const stdext::hash_multimap<Key, Value, Traits, Allocator>& value, const char* name)
{
    EncodeContainer(encoder, value, name);
}

template <typename DecoderType, typename Key, typename Value, typename Traits, typename Allocator>
bool Decode(DecoderType& decoder, stdext::hash_multimap<Key, Value, Traits, Allocator>& value, const char* name)
{
    return DecodeMap(decoder, value, name);
}
#endif

} //end namespace Serialize

#endif//SERIALIZE_STL_HPP_INCLUDED
