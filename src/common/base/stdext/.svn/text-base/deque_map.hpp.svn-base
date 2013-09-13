#ifndef DEQUE_MAP_HPP_INCLUDED
#define DEQUE_MAP_HPP_INCLUDED

#include "common/base/stdext/associated_array.hpp"
#include <deque>

template <
	typename KeyType,
	typename MappedType,
	typename Pred = std::less<KeyType>,
	typename Allocator = std::allocator<std::pair<const KeyType, MappedType> >
	>
class deque_map :
	public sorted_array<std::pair<KeyType, MappedType>,
	pair_compare_1st<Pred>,
	Allocator,
	std::deque<std::pair<KeyType, MappedType>, Allocator>
	>
{
private:
	typedef sorted_array<
				std::pair<KeyType, MappedType>,
				pair_compare_1st<Pred>,
				Allocator,
				std::deque<std::pair<KeyType, MappedType>, Allocator>
			> base_type;
	typedef deque_map<KeyType, MappedType, Pred, Allocator> this_type;
public:
	typedef KeyType key_type;
	typedef MappedType mapped_type;
public:
	deque_map(bool auto_sort = true, bool stable = true) : base_type(auto_sort, stable)
	{
	}
	mapped_type& operator[](const key_type& key)
	{
		typename base_type::iterator it = this->find(key);
		if (it == this->end())
		{
			this->insert(value_type(key, mapped_type()));
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
void swap(deque_map<KeyType, MappedType, Pred, Allocator>& lhs, deque_map<KeyType, MappedType, Pred, Allocator>& rhs)
{
	lhs.swap(rhs);
}

}

#endif//DEQUE_MAP_HPP_INCLUDED
