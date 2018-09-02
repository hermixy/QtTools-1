#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class InputIterator1, class InputIterator2, class OutputIterator, class Pred>
	inline OutputIterator merge(InputIterator1 first1, InputIterator1 last1,
	                            InputIterator2 first2, InputIterator2 last2,
	                            OutputIterator d_first, Pred && pred)
	{
		auto alg = [&first1, &last1, &first2, &last2, &d_first](auto && pred)
		{
			return std::merge(first1, last1, first2, last2, d_first, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class SinglePassRange1, class SinglePassRange2, class OutputIterator, class Pred>
	inline auto merge(const SinglePassRange1 & rng1, const SinglePassRange2 & rng2,
	                  OutputIterator d_first, Pred && pred)
	{
		return varalgo::merge(
			boost::begin(rng1), boost::end(rng1),
		    boost::begin(rng2), boost::end(rng2),
			std::move(d_first), std::forward<Pred>(pred));
	}
}
