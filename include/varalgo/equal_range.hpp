#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class ForwardIterator, class Type, class Pred>
	inline auto equal_range(ForwardIterator first, ForwardIterator last, const Type & val, Pred && pred)
		-> std::pair<ForwardIterator, ForwardIterator>
	{
		auto alg = [&first, &last, &val](auto && pred)
		{
			return std::equal_range(first, last, val, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class ForwardRange, class Type, class Pred>
	inline auto equal_range(const ForwardRange & rng, const Type & val, Pred && pred)
	{
		return varalgo::equal_range(boost::begin(rng), boost::end(rng), val, std::forward<Pred>(pred));
	}

	template <class ForwardRange, class Type, class Pred>
	inline auto equal_range(ForwardRange & rng, const Type & val, Pred && pred)
	{
		return varalgo::equal_range(boost::begin(rng), boost::end(rng), val, std::forward<Pred>(pred));
	}
}

