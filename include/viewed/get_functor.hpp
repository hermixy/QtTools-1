#pragma once
#include <type_traits> // for result_of
#include <functional>  // for reference_wrapprer
#include <utility>     // for std::get
#include <tuple>       // for std::get

#include <boost/variant.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>

namespace viewed
{
	/// functor which transforms it's arguments via get<Index> passes them to wrapped functor
	template <std::size_t Index, class Functor>
	struct get_functor
	{
		Functor func;

		template <class ... Args>
		auto operator()(Args && ... args) const
		{
			using std::get;
			return func(get<Index>(std::forward<Args>(args))...);
		}

		get_functor() = default;
		get_functor(Functor func) : func(std::move(func)) {}
	};

	/// transforms Functor to get_functor<Functor>
	template <std::size_t Index, class Functor>
	struct make_get_functor_type
	{
		typedef typename std::conditional<
			std::is_copy_constructible<Functor>::value,
			get_functor<Index, Functor>,
			get_functor<Index, std::reference_wrapper<const Functor>>
		>::type type;
	};

	/// transforms boost::varaint<Types...> to boost::variant<get_functor<Types>...>
	template <std::size_t Index, class... VariantTypes>
	struct make_get_functor_type<Index, boost::variant<VariantTypes...>>
	{
		template <class Type>
		struct helper
		{
			typedef typename make_get_functor_type<Index, Type>::type type;
		};

		using type = boost::mp11::mp_transform<helper, boost::variant<VariantTypes...>>;
	};

	template <std::size_t Index, class Functor>
	inline auto make_get_functor(const Functor & func)		
	{
		using result_type = typename make_get_functor_type<Index, Functor>::type;
		return result_type {func};
	}
}
