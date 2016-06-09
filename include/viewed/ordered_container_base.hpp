#pragma once
#include <vector>
#include <viewed/associative_conatiner_base.hpp>
#include <viewed/signal_types.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ordered_index.hpp>

namespace viewed
{
	template <class Type, class Compare>
	struct ordered_container_traits
	{
		typedef Type value_type;

		/// container class that stores value_type,
		/// main_store_type should provide stable pointers/references,
		/// iterators allowed to be invalidated on modify operations.
		typedef boost::multi_index_container <
			value_type,
			boost::multi_index::indexed_by<
				boost::multi_index::ordered_unique<
					boost::multi_index::identity<Type>, Compare
				>
			>
		> main_store_type;

		/// container type used for storing raw pointers for views notifications
		typedef std::vector<const value_type *> signal_store_type;

		/************************************************************************/
		/*                   traits functions/functors                          */
		/************************************************************************/
		/// function interface can be static function members or static functors members.
		/// if overloading isn't needed static function members  - will be ok,
		/// but if you want provide several overloads - use static functors members
		
		static main_store_type make_store(Compare comp) 
		{ 
			typedef typename main_store_type::ctor_args ctor_args;
			return main_store_type(
				ctor_args(boost::multi_index::identity<Type>(), std::move(comp))
			);
		}

		/// get_pointer returns pointer to internal_value_type, it used only for filling signal_store_type
		static const value_type * get_pointer(const value_type & val) { return &val; }

		/// update takes current internal_value_type rvalue as first argument and some generic type as second.
		/// It updates current value with new data
		/// usually second type is some reference of value_type
		struct update_type;
		static const update_type update;
	};

	template <class Type, class Compare>
	struct ordered_container_traits<Type, Compare>::update_type
	{
		typedef void result_type;
		result_type operator()(value_type & val, const value_type & newval) const
		{
			val = newval;
		}

		result_type operator()(value_type & val, value_type && newval) const
		{
			val = std::move(newval);
		}
	};

	template <class Type, class Compare>
	const typename ordered_container_traits<Type, Compare>::update_type
		ordered_container_traits<Type, Compare>::update = {};



	/// ordered_container_base class. You are expected to inherit it and add more functional.
	/// Generic container with stl compatible interface
	///
	/// It store data in ordered store(something similar to std::set) and does not allow duplicates, new records will replace already existing
	/// It provides forward bidirectional iterators, iterators, pointers and references - are stable
	/// Iterators are read-only. use upsert to add new data
	///
	/// Emits signals when elements added or erased
	/// Can be used to build views on this container, see viewed::view_base
	/// 
	/// @Param Type element type
	/// @Param Compare functor used to test elements for equivalence
	/// @Param Traits traits class describes various aspects of ordered container
	template <
		class Type,
		class Compare = std::less<>,
		class Traits = ordered_container_traits<Type, Compare>
	>
	class ordered_container_base :
		public associative_conatiner_base<Type, Traits>
	{
		typedef associative_conatiner_base<Type, Traits> base_type;
		
	protected:
		typedef typename base_type::traits_type traits_type;

	public:
		typedef Compare key_compare;
		using typename base_type::const_reference;
		using typename base_type::const_iterator;
		using typename base_type::size_type;

	public:
		key_compare key_comp() const { return base_type::m_store.key_comp(); }
		
		template <class CompatibleKey>
		const_iterator lower_bound(const CompatibleKey & key) const { return base_type::m_store.lower_bound(key); }

		template <class CompatibleKey>
		const_iterator upper_bound(const CompatibleKey & key) const { return base_type::m_store.upper_bound(key); }

		template <class CompatibleKey>
		std::pair<const_iterator, const_iterator> equal_range(const CompatibleKey & key) const
		{ return base_type::m_store.equal_range(key); }

		// bring base overloads
		using base_type::erase;

		template <class CompatibleKey>
		size_type erase(const CompatibleKey & key) { return base_type::m_store.erase(key); }

	public:
		ordered_container_base(key_compare comp = {})
			: base_type(base_type::traits_type(), std::move(comp)) {}

		ordered_container_base(traits_type traits, key_compare comp)
			: base_type(std::move(traits), std::move(comp)) {}
		
		ordered_container_base(const ordered_container_base & val) = delete;
		ordered_container_base & operator =(const ordered_container_base & val) = delete;

		ordered_container_base(ordered_container_base && val) = default;
		ordered_container_base & operator =(ordered_container_base && val) = default;
	};
}
