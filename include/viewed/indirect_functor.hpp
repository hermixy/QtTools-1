#pragma once
#include <type_traits> // for result_of
#include <functional>  // for reference_wrapprer
#include <ext/functors/indirect_functor.hpp>
#include <boost/variant.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>

namespace viewed
{
	/// преобразует предикат в boost::indirect_fun<Pred>
	template <class Pred>
	struct make_indirect_pred_type
	{
		typedef typename std::conditional<
			std::is_copy_constructible<Pred>::value,
			ext::indirect_functor<Pred>,
			ext::indirect_functor<std::reference_wrapper<const Pred>>
		>::type type;
	};

	/// преобразует boost::variant<Types...> в boost::variant<boost::indirect_fun<Types>...>
	template <class... VariantTypes>
	struct make_indirect_pred_type<boost::variant<VariantTypes...>>
	{
		typedef boost::variant<VariantTypes...> initial_variant;
		typedef typename boost::mpl::transform <
			typename initial_variant::types,
			make_indirect_pred_type<boost::mpl::_1>
		>::type transformed;

	public:
		typedef typename boost::make_variant_over<transformed>::type type;
	};

	///аналогичен boost::make_indirect_fun,
	///но корректно обрабатывает boost::variant
	template <class Pred>
	inline
	typename make_indirect_pred_type<Pred>::type
		make_indirect_fun(const Pred & pred)
	{
		return {pred};
	}
}
