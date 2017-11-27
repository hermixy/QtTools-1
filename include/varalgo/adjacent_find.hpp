#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class ForwardIterator, class Pred>
	inline ForwardIterator adjacent_find(ForwardIterator first, ForwardIterator last, Pred && pred)
	{
		auto finder = [&first, &last](auto && pred)
		{
			return std::adjacent_find(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(finder), std::forward<Pred>(pred));
	}	

	/// range overloads
	template <class ForwardRange, class Pred>
	inline auto adjacent_find(const ForwardRange & rng, Pred && pred)
	{
		return varalgo::adjacent_find(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <class ForwardRange, class Pred>
	inline auto adjacent_find(ForwardRange & rng, Pred && pred)
	{
		return varalgo::adjacent_find(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline auto adjacent_find(const ForwardRange & rng, Pred && pred)
		-> typename boost::range_return<const ForwardRange, re>::type
	{
		return boost::range_return<const ForwardRange, re>::pack(
			varalgo::adjacent_find(rng, std::forward<Pred>(pred)),
			rng);
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline auto adjacent_find(ForwardRange & rng, Pred && pred)
		-> typename boost::range_return<ForwardRange, re>::type
	{
		return boost::range_return<ForwardRange, re>::pack(
			varalgo::adjacent_find(rng, std::forward<Pred>(pred)),
			rng);
	}
}