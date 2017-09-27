#pragma once

#include <algorithm>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>

namespace varalgo{

	template <class ForwardIterator1, class ForwardIterator2>
	struct search_visitor :
		boost::static_visitor<ForwardIterator1>
	{
		ForwardIterator1 first1, last1;
		ForwardIterator2 first2, last2;

		search_visitor(ForwardIterator1 first1, ForwardIterator1 last1,
		               ForwardIterator2 first2, ForwardIterator2 last2)
			: first1(first1), last1(last1), first2(first2), last2(last2) {}

		template <class Pred>
		inline ForwardIterator1 operator()(Pred pred) const
		{
			return std::search(first1, last1, first2, last2, pred);
		}
	};

	template <class ForwardIterator1, class ForwardIterator2, class... VaraintTypes>
	inline ForwardIterator1
		search(ForwardIterator1 first1, ForwardIterator1 last1,
		       ForwardIterator2 first2, ForwardIterator2 last2,
		       const boost::variant<VaraintTypes...> & pred)
	{
		return boost::apply_visitor(
			search_visitor<ForwardIterator1, ForwardIterator2> {first1, last1, first2, last2},
			pred);
	}

	template <class ForwardIterator1, class ForwardIterator2, class Pred>
	inline ForwardIterator1
		search(ForwardIterator1 first1, ForwardIterator1 last1,
		       ForwardIterator2 first2, ForwardIterator2 last2,
		       Pred pred)
	{
		return std::search(first1, last1, first2, last2, pred);
	}

	/// range overloads
	template <class ForwardRange1, class ForwardRange2, class Pred>
	inline typename boost::range_iterator<const ForwardRange1>::type
		search(const ForwardRange1 & rng1, const ForwardRange2 & rng2,
		       const Pred & pred)
	{
		return varalgo::
			search(boost::begin(rng1), boost::end(rng1),
		           boost::begin(rng2), boost::end(rng2),
		           pred);
	}

	template <class ForwardRange1, class ForwardRange2, class Pred>
	inline typename boost::range_iterator<ForwardRange1>::type
		search(ForwardRange1 & rng1, const ForwardRange2 & rng2,
		         const Pred & pred)
	{
		return varalgo::
			search(boost::begin(rng1), boost::end(rng1),
		           boost::begin(rng2), boost::end(rng2),
		           pred);
	}

	template <boost::range_return_value re, class ForwardRange1, class ForwardRange2, class Pred>
	inline typename boost::range_return<const ForwardRange1, re>::type
		search(const ForwardRange1 & rng1, const ForwardRange2 & rng2,
		       const Pred & pred)
	{
		return boost::range_return<ForwardRange1, re>::pack(
			varalgo::search(rng1, rng2, pred),
		    rng1);
	}

	template <boost::range_return_value re, class ForwardRange1, class ForwardRange2, class Pred>
	inline typename boost::range_return<ForwardRange1, re>::type
		search(ForwardRange1 & rng1, const ForwardRange2 & rng2,
		       const Pred & pred)
	{
		return boost::range_return<ForwardRange1, re>::pack(
		    varalgo::search(rng1, rng2, pred),
		    rng1);
	}
}