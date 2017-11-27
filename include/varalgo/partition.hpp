#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class ForwardIterator, class Pred>
	inline ForwardIterator partition(ForwardIterator first, ForwardIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::partition(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overload
	template <class ForwardRange, class Pred>
	inline auto partition(const ForwardRange & rng, Pred && pred)
	{
		return varalgo::partition(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <class ForwardRange, class Pred>
	inline auto partition(ForwardRange & rng, Pred && pred)
	{
		return varalgo::partition(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	/// range overload
	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline auto partition(const ForwardRange & rng, Pred && pred)
		-> typename boost::range_return<const ForwardRange, re>::type
	{
		return boost::range_return<const ForwardRange, re>::pack(
			varalgo::partition(rng, std::forward<Pred>(pred)),
			rng);
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline auto partition(ForwardRange & rng, Pred && pred)
		-> typename boost::range_return<ForwardRange, re>::type
	{
		return boost::range_return<ForwardRange, re>::pack(
			varalgo::partition(rng, std::forward<Pred>(pred)),
			rng);
	}
}
