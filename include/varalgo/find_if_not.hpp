#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class InputIterator, class Pred>
	inline InputIterator find_if_not(InputIterator first, InputIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::find_if_not(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class SinglePassRange, class Pred>
	inline auto find_if_not(const SinglePassRange & rng, Pred && pred)
	{
		return varalgo::find_if_not(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <class SinglePassRange, class Pred>
	inline auto find_if_not(SinglePassRange & rng, Pred && pred)
	{
		return varalgo::find_if_not(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <boost::range_return_value re, class SinglePassRange, class Pred>
	inline auto find_if_not(const SinglePassRange & rng, Pred && pred)
		-> typename boost::range_return<const SinglePassRange, re>::type
	{
		return boost::range_return<const SinglePassRange, re>::pack(
			varalgo::find_if_not(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred)),
			rng);
	}

	template <boost::range_return_value re, class SinglePassRange, class Pred>
	inline auto find_if_not(SinglePassRange & rng, Pred && pred)
		-> typename boost::range_return<SinglePassRange, re>::type
	{
		return boost::range_return<SinglePassRange, re>::pack(
			varalgo::find_if_not(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred)),
			rng);
	}
}