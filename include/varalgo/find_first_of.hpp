#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class InputIterator1, class ForwardIterator2, class Pred>
	inline InputIterator1
		find_first_of(InputIterator1 first1, InputIterator1 last1,
		              ForwardIterator2 first2, ForwardIterator2 last2,
		              Pred && pred)
	{
		auto alg = [&first1, &last1, &first2, &last2](auto && pred)
		{
			return std::find_first_of(first1, last1, first2, last2, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class SinglePassRange1, class ForwardRange2, class Pred>
	inline auto find_first_of(const SinglePassRange1 & rng1, const ForwardRange2 & rng2, Pred && pred)
	{
		return varalgo::find_first_of(
			boost::begin(rng1), boost::end(rng1),
		    boost::begin(rng2), boost::end(rng2),
			std::forward<Pred>(pred));
	}

	template <class SinglePassRange1, class ForwardRange2, class Pred>
	inline auto find_first_of(SinglePassRange1 & rng1, const ForwardRange2 & rng2, Pred && pred)
	{
		return varalgo::find_first_of(
			boost::begin(rng1), boost::end(rng1),
		    boost::begin(rng2), boost::end(rng2),
			std::forward<Pred>(pred));
	}

	template <boost::range_return_value re, class SinglePassRange1, class ForwardRange2, class Pred>
	inline auto find_first_of(const SinglePassRange1 & rng1, const ForwardRange2 & rng2, Pred && pred)
		-> typename boost::range_return<const SinglePassRange1, re>::type
	{
		return boost::range_return<SinglePassRange1, re>::pack(
			varalgo::find_first_of(rng1, rng2, std::forward<Pred>(pred)),
		    rng1);
	}

	template <boost::range_return_value re, class SinglePassRange1, class ForwardRange2, class Pred>
	inline auto find_first_of(SinglePassRange1 & rng1, const ForwardRange2 & rng2, Pred && pred)
		-> typename boost::range_return<SinglePassRange1, re>::type
	{
		return boost::range_return<SinglePassRange1, re>::pack(
			varalgo::find_first_of(rng1, rng2, std::forward<Pred>(pred)),
		    rng1);
	}
}
