#ifndef VECTOR_SET_HPP_INCLUDED
#define VECTOR_SET_HPP_INCLUDED

#include "common/base/stdext/associated_array.hpp"

template <typename Type, typename Pred = std::less<Type>, typename Allocator = std::allocator<Type> >
class vector_set : public sorted_array<Type, Pred, Allocator>
{
private:
	typedef sorted_array<Type, Pred, Allocator> base_type;
public:
	//using base_type::capacity;
	//using base_type::reserve;
	vector_set(bool auto_sort = true, bool stable = true) : base_type(auto_sort, stable)
	{
	}
};

namespace std
{

template <typename Type, typename Pred, typename Allocator>
void swap(vector_set<Type, Pred, Allocator>& lhs, vector_map<Type, Pred, Allocator>& rhs)
{
	lhs.swap(rhs);
}

}

#endif//VECTOR_MAP_HPP_INCLUDED
