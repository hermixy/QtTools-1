#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class InputIterator, class Pred>
	inline void for_each(InputIterator first, InputIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::for_each(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class SinglePassRange, class Pred>
	inline auto for_each(const SinglePassRange & rng, Pred && pred)
	{
		return varalgo::for_each(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <class SinglePassRange, class Pred>
	inline void for_each(SinglePassRange & rng, Pred && pred)
	{
		return varalgo::for_each(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}
}
