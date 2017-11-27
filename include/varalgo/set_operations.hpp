#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	/************************************************************************/
	/*                     includes                                         */
	/************************************************************************/
	template <class InputIterator1, class InputIterator2, class Pred>
	inline bool includes(InputIterator1 first1, InputIterator1 last1,
	                     InputIterator2 first2, InputIterator2 last2,
	                     Pred && pred)
	{
		auto alg = [&first1, &last1, &first2, &last2](auto && pred)
		{
			return std::includes(first1, last1, first2, last2, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
	
	/// range overloads
	template <class SinglePassRange1, class SinglePassRange2, class Pred>
	inline bool includes(const SinglePassRange1 & rng1, const SinglePassRange2 & rng2, Pred && pred)
	{
		return varalgo::includes(
			boost::begin(rng1), boost::end(rng1),
			boost::begin(rng2), boost::end(rng2),
			std::forward<Pred>(pred));
	}

	/************************************************************************/
	/*                       set_difference                                 */
	/************************************************************************/
	template <class InputIterator1, class InputIterator2, class OutputIterator, class Pred>
	inline OutputIterator set_difference(InputIterator1 first1, InputIterator1 last1,
	                                     InputIterator2 first2, InputIterator2 last2,
	                                     OutputIterator out, Pred && pred)
	{
		auto alg = [&first1, &last1, &first2, &last2, &out](auto && pred)
		{
			return std::set_difference(first1, last1, first2, last2, out, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class SinglePassRange1, class SinglePassRange2, class OutputIterator, class Pred>
	inline bool set_difference(const SinglePassRange1 & rng1,
	                           const SinglePassRange2 & rng2,
	                           OutputIterator out, Pred && pred)
	{
		return varalgo::set_difference(
			boost::begin(rng1), boost::end(rng1),
			boost::begin(rng2), boost::end(rng2),
			out, std::forward<Pred>(pred));
	}

	/************************************************************************/
	/*                       set_intersection                               */
	/************************************************************************/
	template <class InputIterator1, class InputIterator2, class OutputIterator, class Pred>
	inline OutputIterator set_intersection(InputIterator1 first1, InputIterator1 last1,
	                                       InputIterator2 first2, InputIterator2 last2,
	                                       OutputIterator out, Pred && pred)
	{
		auto alg = [&first1, &last1, &first2, &last2, &out](auto && pred)
		{
			return std::set_intersection(first1, last1, first2, last2, out, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
	
	/// range overloads
	template <class SinglePassRange1, class SinglePassRange2, class OutputIterator, class Pred>
	inline bool set_intersection(const SinglePassRange1 & rng1,
	                             const SinglePassRange2 & rng2,
	                             OutputIterator out, Pred && pred)
	{
		return varalgo::set_intersection(
			boost::begin(rng1), boost::end(rng1),
			boost::begin(rng2), boost::end(rng2),
			out, std::forward<Pred>(pred));
	}

	/************************************************************************/
	/*                       set_symmetric_difference                       */
	/************************************************************************/
	template <class InputIterator1, class InputIterator2, class OutputIterator, class Pred>
	inline OutputIterator set_symmetric_difference(InputIterator1 first1, InputIterator1 last1,
	                                               InputIterator2 first2, InputIterator2 last2,
	                                               OutputIterator out, Pred && pred)
	{
		auto alg = [&first1, &last1, &first2, &last2, &out](auto && pred)
		{
			return std::set_symmetric_difference(first1, last1, first2, last2, out, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
	
	/// range overloads
	template <class SinglePassRange1, class SinglePassRange2, class OutputIterator, class Pred>
	inline bool set_symmetric_difference(const SinglePassRange1 & rng1,
	                                     const SinglePassRange2 & rng2,
	                                     OutputIterator out, Pred && pred)
	{
		return varalgo::set_symmetric_difference(
			boost::begin(rng1), boost::end(rng1),
			boost::begin(rng2), boost::end(rng2),
			out, std::forward<Pred>(pred));
	}

	/************************************************************************/
	/*                       set_union                                      */
	/************************************************************************/
	template <class InputIterator1, class InputIterator2, class OutputIterator, class Pred>
	inline OutputIterator set_union(InputIterator1 first1, InputIterator1 last1,
	                                InputIterator2 first2, InputIterator2 last2,
	                                OutputIterator out, Pred && pred)
	{
		auto alg = [&first1, &last1, &first2, &last2, &out](auto && pred)
		{
			return std::set_union(first1, last1, first2, last2, out, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
	
	/// range overloads
	template <class SinglePassRange1, class SinglePassRange2, class OutputIterator, class Pred>
	inline bool set_union(const SinglePassRange1 & rng1,
	                      const SinglePassRange2 & rng2,
	                      OutputIterator out, Pred && pred)
	{
		return varalgo::set_union(
			boost::begin(rng1), boost::end(rng1),
			boost::begin(rng2), boost::end(rng2),
			out, std::forward<Pred>(pred));
	}

}
