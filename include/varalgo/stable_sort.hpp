#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class RandomAccessIterator, class Pred>
	inline void stable_sort(RandomAccessIterator first, RandomAccessIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::stable_sort(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class RandomAccessRange, class Pred>
	inline const RandomAccessRange & stable_sort(const RandomAccessRange & rng, Pred && pred)
	{
		varalgo::stable_sort(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
		return rng;
	}

	template <class RandomAccessRange, class Pred>
	inline RandomAccessRange & stable_sort(RandomAccessRange & rng, Pred && pred)
	{
		varalgo::stable_sort(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
		return rng;
	}
}
