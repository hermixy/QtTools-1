#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class ForwardIterator, class Type, class Pred>
	inline ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last, const Type & val, Pred && pred)
	{
		auto alg = [&first, &last, &val](auto && pred)
		{
			return std::upper_bound(first, last, val, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
	
	/// range overloads
	template <class ForwardRange, class Type, class Pred>
	inline auto upper_bound(const ForwardRange & rng, const Type & val, Pred && pred)
	{
		return varalgo::upper_bound(boost::begin(rng), boost::end(rng), val, std::forward<Pred>(pred));
	}

	template <class ForwardRange, class Type, class Pred>
	inline auto upper_bound(ForwardRange & rng, const Type & val, Pred && pred)
	{
		return varalgo::upper_bound(boost::begin(rng), boost::end(rng), val, std::forward<Pred>(pred));
	}

	/// range overloads
	template <boost::range_return_value re, class ForwardRange, class Type, class Pred>
	inline auto upper_bound(const ForwardRange & rng, const Type & val, const Pred & pred)
		-> typename boost::range_return<const ForwardRange, re>::type
	{
		return boost::range_return<const ForwardRange, re>::pack(
			varalgo::upper_bound(rng, val, std::forward<Pred>(pred)),
			rng);
	}

	template <boost::range_return_value re, class ForwardRange, class Type, class Pred>
	inline auto upper_bound(ForwardRange & rng, const Type & val, const Pred & pred)
		-> typename boost::range_return<ForwardRange, re>::type
	{
		return boost::range_return<ForwardRange, re>::pack(
			varalgo::upper_bound(rng, val, std::forward<Pred>(pred)),
			rng);
	}
}
