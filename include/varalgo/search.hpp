#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class ForwardIterator1, class ForwardIterator2, class Pred>
	inline ForwardIterator1
		search(ForwardIterator1 first1, ForwardIterator1 last1,
		       ForwardIterator2 first2, ForwardIterator2 last2,
		       Pred && pred)
	{
		auto alg = [&first1, &last1, &first2, &last2](auto && pred)
		{
			return std::search(first1, last1, first2, last2, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class ForwardRange1, class ForwardRange2, class Pred>
	inline auto search(const ForwardRange1 & rng1, const ForwardRange2 & rng2, Pred && pred)
	{
		return varalgo::search(
			boost::begin(rng1), boost::end(rng1),
		    boost::begin(rng2), boost::end(rng2),
			std::forward<Pred>(pred));
	}

	template <class ForwardRange1, class ForwardRange2, class Pred>
	inline auto search(ForwardRange1 & rng1, const ForwardRange2 & rng2, Pred && pred)
	{
		return varalgo::search(
			boost::begin(rng1), boost::end(rng1),
		    boost::begin(rng2), boost::end(rng2),
			std::forward<Pred>(pred));
	}

	template <boost::range_return_value re, class ForwardRange1, class ForwardRange2, class Pred>
	inline auto search(const ForwardRange1 & rng1, const ForwardRange2 & rng2, Pred && pred)
		-> typename boost::range_return<const ForwardRange1, re>::type
	{
		return boost::range_return<ForwardRange1, re>::pack(
			varalgo::search(rng1, rng2, std::forward<Pred>(pred)),
		    rng1);
	}

	template <boost::range_return_value re, class ForwardRange1, class ForwardRange2, class Pred>
	inline auto search(ForwardRange1 & rng1, const ForwardRange2 & rng2, Pred && pred)
		-> typename boost::range_return<ForwardRange1, re>::type
	{
		return boost::range_return<ForwardRange1, re>::pack(
			varalgo::search(rng1, rng2, std::forward<Pred>(pred)),
		    rng1);
	}
}
