#ifndef ASSOCIATED_ARRAY_HPP
#define ASSOCIATED_ARRAY_HPP

#include <memory>
#include <vector>
#include <deque>
#include <functional>
#include <algorithm>

template <typename Pred>
struct pair_compare_1st
{
	template <typename First, typename Second>
	bool operator()(const std::pair<First, Second>& a, const std::pair<First, Second>& b) const
	{
		return Pred()(a.first, b.first);
	}
	template <typename First, typename Second>
	bool operator()(const std::pair<First, Second>& a, const First& b) const
	{
		return Pred()(a.first, b);
	}
	template <typename First, typename Second>
	bool operator()(const First& a, const std::pair<First, Second>& b) const
	{
		return Pred()(a, b.first);
	}
};


template <
	typename Type,
	typename Pred = std::less<Type>,
	typename Allocator = std::allocator<Type>,
	typename Container = std::vector<Type, Allocator>
>
class sorted_array : private Container
{
	template <typename T>
	struct EqualPred
	{
		EqualPred(const T& value):m_value(value)
		{
		}
		template <typename U>
		bool operator()(const U& u) const
		{
			Pred pred;
			return !pred(m_value, u) && !pred(u, m_value);
		}
		const T& m_value;
	};
	typedef sorted_array<Type, Pred, Allocator> this_type;
	typedef Container base_type;
public:
	using base_type::value_type;
	using base_type::size_type;
	using base_type::difference_type;
	using base_type::iterator;
	using base_type::const_iterator;
	using base_type::pointer;
	using base_type::reference;
public:
	sorted_array(bool auto_sort = true, bool stable = true)
		: m_auto_sort(auto_sort), m_stable_sort(stable), m_sorted(true)
	{
	}
	void sort()
	{
		if (!m_sorted)
			do_sort();
	}
	void stable_sort()
	{
		if (!m_sorted)
		{
			std::stable_sort(begin(), end());
			m_sorted = true;
		}
	}

	bool auto_sort_enabled() const
	{
		return m_sorted;
	}

	void enable_auto_sort(bool option = true)
	{
		m_auto_sort = option;
	}
	bool is_sorted() const
	{
		return m_sorted;
	}

	using base_type::begin;
	using base_type::end;
	using base_type::rbegin;
	using base_type::rend;
	using base_type::size;
	using base_type::empty;
	using base_type::clear;

	template <typename T>
	typename base_type::iterator lower_bound(const T& key)
	{
		if (m_sorted)
			return std::lower_bound(begin(), end(), key, m_pred);
		return this->end();
	}

	template <typename T>
	typename base_type::const_iterator lower_bound(const T& key) const
	{
		if (m_sorted)
			return std::lower_bound(begin(), end(), key, m_pred);
		return this->end();
	}

	template <typename T>
	typename base_type::iterator upper_bound(const T& key)
	{
		if (m_sorted)
			return std::upper_bound(begin(), end(), key, m_pred);
		return this->end();
	}

	template <typename T>
	typename base_type::const_iterator upper_bound(const T& key) const
	{
		if (m_sorted)
			return std::upper_bound(begin(), end(), key, m_pred);
		return this->end();
	}

	template <typename T>
	typename base_type::iterator find(const T& key)
	{
		if (m_sorted)
		{
			typename base_type::iterator it = std::lower_bound(begin(), end(), key, m_pred);
			if (it != end() && !m_pred(key, *it))
				return it;
			else
				return end();
		}
		else
		{
			EqualPred<T> pred(key);
			return std::find_if(begin(), end(), pred);
		}
	}

	template <typename T>
	typename base_type::const_iterator find(const T& key) const
	{
		if (m_sorted)
		{
			typename base_type::const_iterator it = std::lower_bound(begin(), end(), key, m_pred);
			if (it != end() && !m_pred(key, *it))
				return it;
			else
				return end();
		}
		else
		{
			EqualPred<T> pred(key);
			return std::find_if(begin(), end(), pred);
		}
	}

	void insert(const typename base_type::value_type& value)
	{
		if (m_auto_sort)
		{
			if (m_sorted)
			{
				typename base_type::iterator it = std::upper_bound(begin(), end(), value, m_pred);
				base_type::insert(it, value);
			}
			else
			{
				this->push_back(value);
				do_sort();
			}
		}
		else
		{
			this->push_back(value);
			m_sorted = false;
		}
	}

	void swap(this_type& rhs)
	{
		base_type::swap(rhs);
		std::swap(m_auto_sort, rhs.m_auto_sort);
		std::swap(m_stable_sort, rhs.m_stable_sort);
		std::swap(m_sorted, rhs.m_sorted);
		std::swap(m_pred, rhs.m_pred);
	}

	// make unique
	void unique()
	{
		sort();
		typename base_type::iterator it = std::unique(begin(), end());
		if (it != end())
		{
			base_type::erase(it);
		}
	}
private:
	void do_sort()
	{
		if (m_stable_sort)
			std::stable_sort(begin(), end(), m_pred);
		else
			std::sort(begin(), end(), m_pred);
		m_sorted = true;
	}
private:
	bool m_auto_sort;
	bool m_stable_sort;
	bool m_sorted;
	Pred m_pred;
};

namespace std
{

template <
	typename Type,
	typename Pred,
	typename Allocator
>
void swap(sorted_array<Type, Pred, Allocator>& lhs, sorted_array<Type, Pred, Allocator>& rhs)
{
	lhs.swap(rhs);
}

}

#endif//ASSOCIATED_ARRAY_HPP
