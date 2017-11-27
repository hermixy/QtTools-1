#pragma once
#include <utility>
#include <boost/variant.hpp>
#include <varalgo/std_variant_traits.hpp>

namespace varalgo
{
	template <class ... VariantTypes>
	struct variant_traits<boost::variant<VariantTypes...>>
	{
		template <class Visitor, class ... Variants>
		inline static auto visit(Visitor && vis, Variants && ... vars)
		{
			return boost::apply_visitor(std::forward<Visitor>(vis), std::forward<Variants>(vars)...);
		}
	};
}
