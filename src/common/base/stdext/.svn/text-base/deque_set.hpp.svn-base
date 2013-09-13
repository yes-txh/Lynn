#ifndef DEQUE_SET_HPP_INCLUDED
#define DEQUE_SET_HPP_INCLUDED

#include "common/base/stdext/associated_array.hpp"

template <typename Type, typename Pred = std::less<Type>, typename Allocator = std::allocator<Type> >
class deque_set : public sorted_array<Type, Pred, Allocator, std::deque<Type, Allocator> >
{
private:
	typedef sorted_array<Type, Pred, Allocator, std::deque<Type, Allocator> > base_type;
public:
	deque_set(bool auto_sort = true, bool stable = true) : base_type(auto_sort, stable)
	{
	}
};

namespace std
{

template <typename Type, typename Pred, typename Allocator>
void swap(deque_set<Type, Pred, Allocator>& lhs, deque_set<Type, Pred, Allocator>& rhs)
{
	lhs.swap(rhs);
}

}

#endif//DEQUE_MAP_HPP_INCLUDED
