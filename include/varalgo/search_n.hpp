#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class ForwardIterator1, class Integer, class Type, class Pred>
	inline ForwardIterator1
		search_n(ForwardIterator1 first1, ForwardIterator1 last1,
		         Integer n, const Type & val, Pred && pred)
	{
		auto alg = [&first1, &last1, &n, &val](auto && pred)
		{
			return std::search_n(first1, last1, n, val, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class ForwardRange1, class Integer, class Type, class Pred>
	inline auto search_n(const ForwardRange1 & rng1, Integer n, const Type & val, Pred && pred)
	{
		return varalgo::search_n(boost::begin(rng1), boost::end(rng1), n, val, std::forward<Pred>(pred));
	}

	template <class ForwardRange1, class Integer, class Type, class Pred>
	inline auto search_n(ForwardRange1 & rng1, Integer n, const Type & val, Pred && pred)
	{
		return varalgo::search_n(boost::begin(rng1), boost::end(rng1), n, val, std::forward<Pred>(pred));
	}

	template <boost::range_return_value re, class ForwardRange1, class Integer, class Type, class Pred>
	inline auto search_n(const ForwardRange1 & rng1, Integer n, const Type & val, Pred && pred)
		-> typename boost::range_return<const ForwardRange1, re>::type
	{
		return boost::range_return<ForwardRange1, re>::pack(
			varalgo::search_n(rng1, n, val, std::forward<Pred>(pred)),
		    rng1);
	}

	template <boost::range_return_value re, class ForwardRange1, class Integer, class Type, class Pred>
	inline auto search_n(ForwardRange1 & rng1, Integer n, const Type & val, Pred && pred)
		-> typename boost::range_return<ForwardRange1, re>::type
	{
		return boost::range_return<ForwardRange1, re>::pack(
			varalgo::search_n(rng1, n, val, std::forward<Pred>(pred)),
		    rng1);
	}
}