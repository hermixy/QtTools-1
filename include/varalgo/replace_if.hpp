#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class ForwardIterator, class Type, class Pred>
	inline void replace_if(ForwardIterator first, ForwardIterator last, Pred && pred, const Type & new_val)
	{
		auto alg = [&first, &last, &new_val](auto && pred)
		{
			return std::replace_if(first, last, std::forward<decltype(pred)>(pred), new_val);
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
	
	/// range overloads
	template <class ForwardRange, class Type, class Pred>
	inline const ForwardRange & replace_if(const ForwardRange & rng, Pred && pred, const Type & new_val)
	{
		varalgo::replace_if(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred), new_val);
		return rng;
	}

	template <class ForwardRange, class Type, class Pred>
	inline ForwardRange & replace_if(ForwardRange & rng, Pred && pred, const Type & new_val)
	{
		varalgo::replace_if(boost::begin(rng), boost::end(rng), std::forward<Pred>(pred), new_val);
		return rng;
	}
}