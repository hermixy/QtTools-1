#pragma once
#include <variant>

namespace varalgo
{
	template <class Pred>
	struct variant_traits
	{
		template <class Visitor, class ... Variants>
		inline static constexpr auto visit(Visitor && vis, Variants && ... vars)
		{
			return vis(std::forward<Variants>(vars)...);
		}
	};

	template <class ... Types>
	struct variant_traits<std::variant<Types...>>
	{
		template <class Visitor, class ... Variants>
		inline static constexpr auto visit(Visitor && vis, Variants && ... vars)
		{
			return std::visit(std::forward<Visitor>(vis), std::forward<Variants>(vars)...);
		}
	};
}
