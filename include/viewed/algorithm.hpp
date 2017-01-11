#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <numeric>

namespace viewed
{
	template <class RandomAccessIterator>
	void inverse_index_array(RandomAccessIterator first, RandomAccessIterator last,
	                         typename std::iterator_traits<RandomAccessIterator>::value_type offset = 0)
	{
		auto i = offset;
		std::vector<typename std::iterator_traits<RandomAccessIterator>::value_type> inverse(last - first);

		for (auto it = first; it != last; ++it, ++i)
		{
			int val = *it;
			inverse[std::abs(val) - offset] = val >= 0 ? i : -1;
		}

		std::copy(inverse.begin(), inverse.end(), first);
	}
	
	template <class Iterator>
	std::vector<int> build_relloc_map(Iterator removed_first, Iterator removed_last, std::size_t store_size)
	{
		std::vector<int> index_array(store_size);

		int v1 = 0, v2 = static_cast<int>(store_size);
		int val = 0;

		for (; removed_first != removed_last; ++removed_first)
		{
			v2 = *removed_first;

			auto first = index_array.begin() + v1;
			auto last = index_array.begin() + v2;
			std::iota(first, last, val);
			val += static_cast<int>(last - first);

			*last = -1;
			v1 = ++v2;
		}

		auto first = index_array.begin() + v2;
		auto last = index_array.end();
		std::iota(first, last, val);

		return index_array;
	}

	template <class RandomAccessIterator, class Iterator>
	RandomAccessIterator remove_indexes(RandomAccessIterator first, RandomAccessIterator last,
	                                    Iterator ifirst, Iterator ilast)
	{
		if (ifirst == ilast) return last;

		auto out = first + *ifirst;
		auto it = out + 1;
		auto next = last;

		for (++ifirst; ifirst != ilast; ++ifirst)
		{
			next = first + *ifirst;
			out = std::move(it, next, out);
			it = next + 1;
		}

		return std::move(it, last, out);
	}
}
