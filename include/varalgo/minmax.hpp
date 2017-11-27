#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class Type, class Pred>
	inline const Type & min(const Type & a, const Type & b, Pred && pred)
	{
		auto alg = [&a, &b](auto && pred)
		{
			return std::min(a, b, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	template <class Type, class Pred>
	inline const Type & max(const Type & a, const Type & b, Pred && pred)
	{
		auto alg = [&a, &b](auto && pred)
		{
			return std::max(a, b, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	template <class Type, class Pred>
	inline std::pair<const Type &, const Type &>
		minmax(const Type & a, const Type & b, Pred && pred)
	{
		auto alg = [&a, &b](auto && pred)
		{
			return std::minmax(a, b, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}
}
