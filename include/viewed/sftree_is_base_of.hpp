#pragma once
#include <utility>

namespace viewed
{
	namespace sftree_detail
	{
		template <template <class ...> class base, class derived>
		struct is_template_base_of_impl
		{
			using yes = char(&)[2];
			using no  = char(&)[1];

			static_assert(sizeof(yes) != sizeof(no));

			template <class ... types>
			static yes test(const base<types...> & ref);
			static no  test(...);

			using type = decltype(test(std::declval<derived>()));
			constexpr static bool value = sizeof(type) == sizeof(yes);

		};

		template <template <class ... types> class base, class derived>
		struct is_base_of : std::integral_constant<bool, is_template_base_of_impl<base, derived>::value> {};
	}
}
