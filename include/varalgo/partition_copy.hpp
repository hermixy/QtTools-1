#pragma once
#include <algorithm>
#include <varalgo/std_variant_traits.hpp>
#include <boost/range.hpp>

namespace varalgo
{
	template <class InputIterator, class OutputIterator1, class OutputIterator2, class Pred>
	inline std::pair<OutputIterator1, OutputIterator2>
		partition_copy(InputIterator first, InputIterator last,
		               OutputIterator1 dest_true, OutputIterator2 dest_false,
		               Pred && pred)
	{
		auto alg = [&first, &last, &dest_true, &dest_false](auto && pred)
		{
			return std::partition_copy(first, last, dest_true, dest_false, std::forward<decltype(pred)>(pred));
		};

		return variant_traits<std::decay_t<Pred>>::visit(std::move(alg), std::forward<Pred>(pred));
	}

	/// range overloads
	template <class SinglePassRange, class OutputIterator1, class OutputIterator2, class Pred>
	inline auto partition_copy(const SinglePassRange & rng, OutputIterator1 dest_true, OutputIterator2 dest_false, Pred && pred)
	{
		return varalgo::partition_copy(
			boost::begin(rng), boost::end(rng),
			std::move(dest_true), std::move(dest_false),
			std::forward<Pred>(pred));
	}
}
