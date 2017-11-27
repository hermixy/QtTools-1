#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class RandomAccessIterator, class Pred>
	inline void partial_sort(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last, Pred && pred)
	{
		auto alg = [&first, &middle, &last](auto && pred)
		{
			return std::partial_sort(first, middle, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
	
	/// range overloads
	template <class RandomAccessRange, class Pred>
	inline const RandomAccessRange &
		partial_sort(const RandomAccessRange & rng,
		             typename boost::range_iterator<const RandomAccessRange>::type middle,
		             Pred && pred)
	{
		varalgo::partial_sort(boost::begin(rng), middle, boost::end(rng), std::forward<Pred>(pred));
		return rng;
	}
	
	template <class RandomAccessRange, class Pred>
	inline RandomAccessRange &
		partial_sort(RandomAccessRange & rng,
		             typename boost::range_iterator<RandomAccessRange>::type middle,
		             Pred && pred)
	{
		varalgo::partial_sort(boost::begin(rng), middle, boost::end(rng), std::forward<Pred>(pred));
		return rng;
	}
}
