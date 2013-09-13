#ifndef EXT_HASH_FUNCTION_HPP_INCLUDED
#define EXT_HASH_FUNCTION_HPP_INCLUDED

#ifdef __GNUC__
namespace __gnu_cxx
{
	template<>
	struct hash<long long>
	{
		size_t operator()(long long __x) const
		{
			if (sizeof(__x) == sizeof(size_t))
				return __x;
			else
				return (__x >> 32) ^ (__x & 0xFFFFFFFF);
		}
	};

	template<>
	struct hash<unsigned long long>
	{
		size_t operator()(unsigned long long __x) const
		{
			if (sizeof(__x) == sizeof(size_t))
				return __x;
			else
				return (__x >> 32) ^ (__x & 0xFFFFFFFF);
		}
	};

	template<typename Traits, typename Allocator>
	struct hash<std::basic_string<char, Traits, Allocator> >
	{
		size_t operator()(const std::basic_string<char, Traits, Allocator>& __s) const
		{
			return __stl_hash_string(__s.c_str());
		}
	};

}
#endif

#endif//EXT_HASH_FUNCTION_HPP_INCLUDED

