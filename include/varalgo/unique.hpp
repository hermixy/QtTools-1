#pragma once

#include <algorithm>

#include <boost/range.hpp>
#include <boost/range/detail/range_return.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>

namespace varalgo{

	template <class ForwardIterator>
	struct unique_visitor :
		boost::static_visitor<ForwardIterator>
	{
		ForwardIterator first, last;

		unique_visitor(ForwardIterator first, ForwardIterator last)
			: first(first), last(last) {}

		template <class Pred>
		inline ForwardIterator operator()(Pred pred) const
		{
			return std::unique(first, last, pred);
		}
	};

	template <class ForwardIterator, class... VaraintTypes>
	inline ForwardIterator
		unique(ForwardIterator first, ForwardIterator last, const boost::variant<VaraintTypes...> & pred)
	{
		return boost::apply_visitor(
			unique_visitor<ForwardIterator> {first, last},
			pred);
	}
	
	template <class ForwardIterator, class Pred>
	inline ForwardIterator
		unique(ForwardIterator first, ForwardIterator last, Pred pred)
	{
		return std::unique(first, last, pred);
	}

	/// range overloads
	template <class ForwardRange, class Pred>
	inline typename boost::range_iterator<const ForwardRange>::type
		unique(const ForwardRange & rng, const Pred & pred)
	{
		return varalgo::unique(boost::begin(rng), boost::end(rng), pred);
	}

	template <class ForwardRange, class Pred>
	inline typename boost::range_iterator<ForwardRange>::type
		unique(ForwardRange & rng, const Pred & pred)
	{
		return varalgo::unique(boost::begin(rng), boost::end(rng), pred);
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline typename boost::range_return<const ForwardRange, re>::type
		unique(const ForwardRange & rng, const Pred & pred)
	{
		return boost::range_return<const ForwardRange, re>::pack(
			varalgo::unique(boost::begin(rng), boost::end(rng), pred),
			rng);
	}

	template <boost::range_return_value re, class ForwardRange, class Pred>
	inline typename boost::range_return<ForwardRange, re>::type
		unique(ForwardRange & rng, const Pred & pred)
	{
		return boost::range_return<ForwardRange, re>::pack(
			varalgo::unique(boost::begin(rng), boost::end(rng), pred),
			rng);
	}
}