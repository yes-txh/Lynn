#ifndef VECTOR_MAP_HPP_INCLUDED
#define VECTOR_MAP_HPP_INCLUDED

#include "common/base/stdext/associated_array.hpp"

template <
	typename KeyType,
	typename MappedType,
	typename Pred = std::less<KeyType>,
	typename Allocator = std::allocator<std::pair<const KeyType, MappedType> >
	>
class vector_map : public sorted_array<std::pair<KeyType, MappedType>, pair_compare_1st<Pred>, Allocator>
{
private:
	typedef sorted_array<std::pair<KeyType, MappedType>, pair_compare_1st<Pred>, Allocator> base_type;
	typedef vector_map<KeyType, MappedType, Pred, Allocator> this_type;
public:
	typedef KeyType key_type;
	typedef MappedType mapped_type;
public:
	vector_map(bool auto_sort = true, bool stable = true) : base_type(auto_sort, stable)
	{
	}
	mapped_type& operator[](const key_type& key)
	{
		typename base_type::iterator it = this->find(key);
		if (it == this->end())
		{
			this->insert(typename base_type::value_type(key, mapped_type()));
			it = this->find(key);
			return it->second;
		}
		else
		{
			return it->second;
		}
	}
	void swap(this_type& rhs)
	{
		base_type::swap(rhs);
	}
};

namespace std
{

template <
	typename KeyType,
	typename MappedType,
	typename Pred,
	typename Allocator
	>
void swap(vector_map<KeyType, MappedType, Pred, Allocator>& lhs, vector_map<KeyType, MappedType, Pred, Allocator>& rhs)
{
	lhs.swap(rhs);
}

}

#endif//VECTOR_MAP_HPP_INCLUDED
