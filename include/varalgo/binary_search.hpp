#pragma once

#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class ForwardIterator, class Type, class Pred>
	inline bool binary_search(ForwardIterator first, ForwardIterator last, const Type & val, Pred && pred)
	{
		auto alg = [&first, &last, &val](auto && pred)
		{
			return std::binary_search(first, last, val, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class ForwardRange, class Type, class Pred>
	inline bool binary_search(const ForwardRange & rng, const Type & val, Pred && pred)
	{
		return varalgo::binary_search(boost::begin(rng), boost::end(rng), val, std::forward<Pred>(pred));
	}
}
