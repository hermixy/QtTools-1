#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class ForwardIterator, class Pred>
	inline bool is_partitioned(ForwardIterator first, ForwardIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::is_partitioned(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	template <class ForwardRange, class Pred>
	inline bool is_partitioned(const ForwardRange & rng, Pred && pred)
	{
		return varalgo::is_partitioned(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}
}
