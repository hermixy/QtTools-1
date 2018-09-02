#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class ForwardIterator, class Pred>
	inline bool is_sorted(ForwardIterator first, ForwardIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::is_sorted(first, last, std::forward<decltype(pred)>(pred));
		};
	}

	template <class ForwardRange, class Pred>
	inline bool is_sorted(const ForwardRange & rng, Pred && pred)
	{
		return varalgo::is_sorted(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}
}

