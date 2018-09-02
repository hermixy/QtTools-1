#pragma once
#include <algorithm>
#include <ext/algorithm/binary_find.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	template <class ForwardIterator, class Type, class Pred>
	inline ForwardIterator binary_find(ForwardIterator first, ForwardIterator last,
	                                   const Type & val, Pred && pred)
	{
		auto alg = [&first, &last, &val](auto && pred)
		{
			return ext::binary_find(first, last, val, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class ForwardRange, class Type, class Pred>
	inline auto binary_find(const ForwardRange & rng, const Type & val, Pred && pred)
	{
		return varalgo::binary_find(boost::begin(rng), boost::end(rng), val, std::forward<Pred>(pred));
	}

	template <class ForwardRange, class Type, class Pred>
	inline auto binary_find(ForwardRange & rng, const Type & val, Pred && pred)
	{
		return varalgo::binary_find(boost::begin(rng), boost::end(rng), val, std::forward<Pred>(pred));
	}

	template <boost::range_return_value re, class ForwardRange, class Type, class Pred>
	inline auto binary_find(const ForwardRange & rng, const Type & val, Pred && pred)
		-> typename boost::range_return<const ForwardRange, re>::type
	{
		return boost::range_return<const ForwardRange, re>::pack(
			varalgo::binary_find(rng, val, std::forward<Pred>(pred)),
			rng);
	}

	template <boost::range_return_value re, class ForwardRange, class Type, class Pred>
	inline auto binary_find(ForwardRange & rng, const Type & val, Pred && pred)
		-> typename boost::range_return<ForwardRange, re>::type
	{
		return boost::range_return<ForwardRange, re>::pack(
			varalgo::binary_find(rng, val, std::forward<Pred>(pred)),
			rng);
	}
}
