#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>

namespace varalgo
{
	/************************************************************************/
	/*                     min_element                                      */
	/************************************************************************/
	template <class ForwardIterator, class Pred>
	inline ForwardIterator min_element(ForwardIterator first, ForwardIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::min_element(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));		
	}

	/// range overloads
	template <class ForwardRange, class Pred>
	inline auto min_element(const ForwardRange & rng, Pred && pred)
	{
		return min_element(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <class ForwardRange, class Pred>
	inline auto min_element(ForwardRange & rng, Pred && pred)
	{
		return min_element(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline auto min_element(const ForwardRange & rng, Pred pred)
		-> typename boost::range_return<const ForwardRange, re>::type
	{
		return boost::range_return<const ForwardRange, re>::pack(
			min_element(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred)),
			rng);
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline auto min_element(ForwardRange & rng, Pred pred)
		-> typename boost::range_return<ForwardRange, re>::type
	{
		return boost::range_return<ForwardRange, re>::pack(
			min_element(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred)),
			rng);
	}

	/************************************************************************/
	/*                max_element                                           */
	/************************************************************************/
	template <class ForwardIterator, class Pred>
	inline ForwardIterator max_element(ForwardIterator first, ForwardIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::max_element(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class ForwardRange, class Pred>
	inline auto max_element(const ForwardRange & rng, Pred && pred)
	{
		return max_element(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <class ForwardRange, class Pred>
	inline auto max_element(ForwardRange & rng, Pred && pred)
	{
		return max_element(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline auto max_element(const ForwardRange & rng, Pred pred)
		-> typename boost::range_return<const ForwardRange, re>::type
	{
		return boost::range_return<const ForwardRange, re>::pack(
			max_element(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred)),
			rng);
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline auto max_element(ForwardRange & rng, Pred pred)
		-> typename boost::range_return<ForwardRange, re>::type
	{
		return boost::range_return<ForwardRange, re>::pack(
			max_element(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred)),
			rng);
	}

	/************************************************************************/
	/*                minmax_element                                        */
	/************************************************************************/
	template <class ForwardIterator, class Pred>
	inline std::pair<ForwardIterator, ForwardIterator>
		minmax_element(ForwardIterator first, ForwardIterator last, Pred && pred)
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::minmax_element(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class ForwardRange, class Pred>
	inline auto minmax_element(const ForwardRange & rng, Pred && pred)
	{
		return varalgo::minmax_element(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}

	template <class ForwardRange, class Pred>
	inline auto minmax_element(ForwardRange & rng, Pred && pred)
	{
		return varalgo::minmax_element(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}
}
