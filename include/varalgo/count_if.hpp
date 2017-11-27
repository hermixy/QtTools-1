#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class InputIterator, class Pred>
	inline auto count_if(InputIterator first, InputIterator last, Pred && pred)
		-> typename boost::iterator_difference<InputIterator>::type
	{
		auto alg = [&first, &last](auto && pred)
		{
			return std::count_if(first, last, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
	
	/// range overload
	template <class SinglePassRange, class Pred>
	inline auto count_if(const SinglePassRange & rng, Pred && pred)
	{
		return varalgo::count_if(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred));
	}
}
