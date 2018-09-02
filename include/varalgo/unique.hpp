#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class ForwardIterator, class Pred>
	inline ForwardIterator unique(ForwardIterator first, ForwardIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::unique(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class ForwardRange, class Pred>
	inline auto unique(const ForwardRange & rng, Pred && pred)
	{
		return varalgo::unique(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <class ForwardRange, class Pred>
	inline auto unique(ForwardRange & rng, Pred && pred)
	{
		return varalgo::unique(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline auto unique(const ForwardRange & rng, Pred && pred)
		-> typename boost::range_return<const ForwardRange, re>::type
	{
		return boost::range_return<const ForwardRange, re>::pack(
			varalgo::unique(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred)),
			rng);
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline auto unique(ForwardRange & rng, Pred && pred)
		-> typename boost::range_return<ForwardRange, re>::type
	{
		return boost::range_return<ForwardRange, re>::pack(
			varalgo::unique(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred)),
			rng);
	}
}
