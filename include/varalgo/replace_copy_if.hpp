#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class InputIterator, class OutputIterator, class Type, class Pred>
	inline OutputIterator replace_copy_if(
		InputIterator first, InputIterator last,
		OutputIterator dest,
		Pred && pred, const Type & new_val)
	{
		auto alg = [&first, &last, &dest, &new_val](auto && pred)
		{
			return std::replace_copy_if(first, last, dest, std::forward<decltype(pred)>(pred), new_val);
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
	
	/// range overloads
	template <class SinglePassRange, class OutputIterator, class Type, class Pred>
	inline OutputIterator replace_copy_if(const SinglePassRange & rng, OutputIterator dest, Pred && pred, const Type & new_val)
	{
		return varalgo::replace_copy_if(boost::begin(rng), boost::end(rng), std::move(dest), std::forward<Pred>(pred), new_val);
	}
}