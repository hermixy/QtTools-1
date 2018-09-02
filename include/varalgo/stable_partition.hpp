#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class BidirectionalIterator, class Pred>
	inline BidirectionalIterator stable_partition(BidirectionalIterator first, BidirectionalIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::stable_partition(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overload
	template <class BidirectionalRange, class Pred>
	inline auto stable_partition(const BidirectionalRange & rng, Pred && pred)
	{
		return varalgo::stable_partition(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <class BidirectionalRange, class Pred>
	inline auto stable_partition(BidirectionalRange & rng, Pred && pred)
	{
		return varalgo::stable_partition(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	/// range overload
	template <boost::range_return_value re, class BidirectionalRange, class Pred>
	inline auto stable_partition(const BidirectionalRange & rng, Pred && pred)
		-> typename boost::range_return<const BidirectionalRange, re>::type
	{
		return boost::range_return<const BidirectionalRange, re>::pack(
			varalgo::stable_partition(rng, std::forward<Pred>(pred)),
			rng);
	}

	template <boost::range_return_value re, class BidirectionalRange, class Pred>
	inline auto stable_partition(BidirectionalRange & rng, Pred && pred)
		-> typename boost::range_return<BidirectionalRange, re>::type
	{
		return boost::range_return<BidirectionalRange, re>::pack(
			varalgo::stable_partition(rng, std::forward<Pred>(pred)),
			rng);
	}
}
