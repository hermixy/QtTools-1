#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class InputIterator, class RandomAccessIterator, class Pred>
	inline RandomAccessIterator partial_sort_copy(
		InputIterator first, InputIterator last,
	    RandomAccessIterator d_first, RandomAccessIterator d_last,
	    Pred && pred)
	{
		auto alg = [&first, &last, &d_first, &d_last](auto && pred)
		{
			return std::partial_sort_copy(first, last, d_first, d_last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
	
	/// range overloads
	template <class SinglePassRange, class RandomAccessRange, class Pred>
	inline auto partial_sort_copy(const SinglePassRange & input, const RandomAccessRange & output, Pred && pred)
	{
		return varalgo::partial_sort_copy(
			boost::begin(input), boost::end(input),
			boost::begin(output), boost::end(output),
			std::forward<Pred>(pred));
	}

	template <class SinglePassRange, class RandomAccessRange, class Pred>
	inline auto partial_sort_copy(const SinglePassRange & input, RandomAccessRange & output, Pred && pred)
	{
		return varalgo::partial_sort_copy(
			boost::begin(input), boost::end(input),
			boost::begin(output), boost::end(output),
			std::forward<Pred>(pred));
	}
}
