#pragma once

/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/util/utility.h>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <span>

template<class T, size_t SIZE>
class StaticArrayList
{
public:
	using value_type = T;
	using size_type = size_t;
	using iterator = T*;
	using const_iterator = const T*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	constexpr StaticArrayList() = default;

	// Iterators (STL API)
	constexpr iterator begin() { return data(); }
	constexpr iterator end() { return data() + size(); }
	constexpr const_iterator begin() const { return data(); }
	constexpr const_iterator end() const { return data() + size(); }
	constexpr const_iterator cbegin() const { return begin(); }
	constexpr const_iterator cend() const { return end(); }
	constexpr reverse_iterator rbegin() { return reverse_iterator(end()); }
	constexpr reverse_iterator rend() { return reverse_iterator(begin()); }
	constexpr const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	constexpr const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
	constexpr const_reverse_iterator crbegin() const { return rbegin(); }
	constexpr const_reverse_iterator crend() const { return rend(); }

	// Capacity (STL API)
	constexpr size_t size() const { return size_; }
	constexpr bool empty() const { return !size(); };
	constexpr size_t capacity() const { return SIZE; }
	constexpr size_t max_size() const { return SIZE; }

	constexpr void resize(size_t size)
	{
		assert(size <= max_size());
		size_= size;
	}

	// Capacity
	constexpr bool isFull() const
	{
		return !freeSpace();
	}

	constexpr size_t freeSpace() const
	{
		return capacity() - size();
	}

	// Element Access (STL API)
	constexpr T &front() { return at(0);	}
	constexpr T &back() { return at(size()-1);	}

	constexpr T &at(size_t idx)
	{
		assert(idx < size());
		return (*this)[idx];
	}

	constexpr T *data() { return storage(); }
	constexpr const T *data() const { return storage(); }

	constexpr T& operator[] (int idx) { return data()[idx]; }
	constexpr const T& operator[] (int idx) const { return data()[idx]; }

	// Modifiers (STL API)
	constexpr void clear()
	{
		//logMsg("removing all array list items (%d)", size_);
		size_ = 0;
	}

	constexpr void pop_back()
	{
		assert(size_);
		size_--;
	}

	constexpr void push_back(const T &d)
	{
		assert(size_ < max_size());
		data()[size_] = d;
		size_++;
	}

	constexpr T &emplace_back(auto &&...args)
	{
		assert(size_ < max_size());
		auto newAddr = &data()[size_];
		new(newAddr) T(IG_forward(args)...);
		size_++;
		return *newAddr;
	}

	constexpr iterator insert(const_iterator position, const T& val)
	{
		assert(size_ < max_size());
		iterator p = data() + (position - begin());
		if(p == end())
		{
			push_back(val);
		}
		else
		{
			std::move_backward(p, end(), end() + 1);
			*p = val;
			size_++;
		}
		return p;
	}

	constexpr iterator erase(iterator position)
	{
		if(position + 1 != end())
			std::move(position + 1, end(), position);
		size_--;
		return position;
	}

	constexpr iterator erase(iterator first, iterator last)
	{
		if(first != last)
		{
			if(last != end())
				std::move(last, end(), first);
			size_ -= std::distance(first, last);
		}
		return first;
	}

	constexpr operator std::span<T>() const
	{
		return {data(), size()};
	}

private:
	size_t size_{};
	T arr[SIZE];

	constexpr T *storage() { return arr; }
	constexpr const T *storage() const { return arr; }
};

namespace IG
{

template<class T, size_t SIZE, class Pred>
static constexpr typename StaticArrayList<T,SIZE>::size_type
	erase_if(StaticArrayList<T,SIZE>& c, Pred pred)
{
	auto it = std::remove_if(c.begin(), c.end(), pred);
	auto r = std::distance(it, c.end());
	c.erase(it, c.end());
	return r;
}

}
