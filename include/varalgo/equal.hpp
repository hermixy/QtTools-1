#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class InputIterator1, class InputIterator2, class Pred>
	inline bool equal(InputIterator1 first1, InputIterator1 last1,
		              InputIterator2 first2, InputIterator2 last2,
		              Pred && pred)
	{
		auto alg = [&first1, &last1, &first2, &last2](auto && pred)
		{
			return std::equal(first1, last1, first2, last2, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	template <class InputIterator1, class InputIterator2, class Pred>
	inline bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, Pred && pred)
	{
		auto alg = [&first1, &last1, &first2] (auto && pred)
		{
			return std::equal(first1, last1, first2, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class SinglePassRange1, class SinglePassRange2, class Pred>
	inline bool equal(const SinglePassRange1 & rng1, const SinglePassRange2 & rng2, Pred && pred)
	{
		return varalgo::equal(
			boost::begin(rng1), boost::end(rng1),
		    boost::begin(rng2), boost::end(rng2),
			std::forward<Pred>(pred));
	}
}
