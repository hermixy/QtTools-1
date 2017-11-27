#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class RandomAccessIterator, class Pred>
	inline void nth_element(RandomAccessIterator first, RandomAccessIterator nth, RandomAccessIterator last, Pred && pred)
	{
		auto alg = [&first, &nth, &last](auto && pred)
		{
			return std::nth_element(first, nth, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class RandomAccessRange, class Pred>
	inline const RandomAccessRange & nth_element(
		const RandomAccessRange & rng,
		typename boost::range_iterator<const RandomAccessRange>::type nth,
		Pred && pred)
	{
		varalgo::nth_element(boost::begin(rng), nth, boost::end(rng), std::forward<Pred>(pred));
		return rng;
	}

	template <class RandomAccessRange, class Pred>
	inline RandomAccessRange & nth_element(
		RandomAccessRange & rng,
		typename boost::range_iterator<RandomAccessRange>::type nth,
		Pred && pred)
	{
		varalgo::nth_element(boost::begin(rng), nth, boost::end(rng), std::forward<Pred>(pred));
		return rng;
	}
}
