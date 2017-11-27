#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class InputIterator, class Pred>
	inline bool all_of(InputIterator first, InputIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::all_of(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class SinglePassRange, class Pred>
	inline bool all_of(const SinglePassRange & rng, Pred && pred)
	{
		return varalgo::all_of(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}
}