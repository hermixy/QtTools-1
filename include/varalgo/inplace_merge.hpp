#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class BidirectionalIterator, class Pred>
	inline void inplace_merge(BidirectionalIterator first, BidirectionalIterator middle, BidirectionalIterator last, Pred && pred)
	{
		auto alg = [&first, &middle, &last](auto && pred)
		{
			return std::inplace_merge(first, middle, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	template <class BidirectionalRange, class Pred>
	inline const BidirectionalRange & inplace_merge(
		const BidirectionalRange & rng,
	    typename boost::range_const_iterator<BidirectionalRange>::type middle,
	    Pred && pred)
	{
		varalgo::inplace_merge(boost::begin(rng), middle, boost::end(rng), std::forward<Pred>(pred));
		return rng;
	}

	template <class BidirectionalRange, class Pred>
	inline BidirectionalRange & inplace_merge(
		BidirectionalRange & rng,
	    typename boost::range_iterator<BidirectionalRange>::type middle,
	    Pred && pred)
	{
		varalgo::inplace_merge(boost::begin(rng), middle, boost::end(rng), std::forward<Pred>(pred));
		return rng;
	}
}