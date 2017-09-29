#pragma once
#include <vector>
#include <viewed/associative_conatiner_base.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/hashed_index.hpp>

namespace viewed
{
	template <class Type, class Hash, class Equal>
	struct hash_container_traits
	{
		typedef Type value_type;

		/// container class that stores value_type,
		/// main_store_type should provide stable pointers/references,
		/// iterators allowed to be invalidated on modify operations.
		typedef boost::multi_index_container <
			value_type,
			boost::multi_index::indexed_by<
				boost::multi_index::hashed_unique<
					boost::multi_index::identity<Type>,
					Hash, Equal
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
				
		static main_store_type make_store(Hash hash, Equal eq)
		{
			typedef typename main_store_type::ctor_args ctor_args;
			/// The first element of this tuple indicates the minimum number of buckets 
			/// set up by the index on construction time. 
			/// If the default value 0 is used, an implementation defined number is used instead.
			return main_store_type(
				ctor_args(0, boost::multi_index::identity<Type>(), std::move(hash), std::move(eq))
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

	template <class Type, class Hash, class Equal>
	struct hash_container_traits<Type, Hash, Equal>::update_type
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

	template <class Type, class Hash, class Equal>
	const typename hash_container_traits<Type, Hash, Equal>::update_type
		hash_container_traits<Type, Hash, Equal>::update = {};



	/// hash_container_base class.  You are expected to inherit it and add more functional.
	/// Generic container with stl compatible interface
	///
	/// It store data in hashed store and does not allow duplicates, new records will replace already existing
	/// It provides forward traversal iterators, while iterators not stable, pointers and references - are stable
	/// Iterators are read-only. use upsert to add new data
	///
	/// Emits signals when elements added or erased
	/// Can be used to build views on this container, see viewed::view_base
	/// 
	/// @Param Type element type
	/// @Param Hash functor used to compare elements
	/// @Param Equal functor used to compare elements
	/// @Param Traits traits class describes various aspects of hashed container
	template <
		class Type,
		class Hash = std::hash<Type>,
		class Equal = std::equal_to<Type>,
		class Traits = hash_container_traits<Type, Hash, Equal>,
		class SignalTraits = default_signal_traits<Type>
	>
	class hash_container_base:
		public associative_conatiner_base<Type, Traits, SignalTraits>
	{
		typedef associative_conatiner_base<Type, Traits, SignalTraits> base_type;

	protected:
		typedef typename base_type::traits_type traits_type;

	public:
		typedef Equal     key_equal;
		typedef Hash      hasher;

		using typename base_type::const_iterator;
		using typename base_type::const_reference;

	public:
		key_equal key_eq() const { return base_type::m_store.key_eq(); }
		hasher    hash_function() const { return base_type::m_store.hash_function(); }
		
		template <class CompatibleKey>
		std::pair<const_iterator, const_iterator> equal_range(const CompatibleKey & key) const
		{ return base_type::m_store.equal_range(key); }
		
	public:
		hash_container_base(hasher hash = {}, key_equal eq = {})
			: base_type(base_type::traits_type(), std::move(hash), std::move(eq))
		{ }

		hash_container_base(traits_type traits, hasher hash, key_equal eq)
			: base_type(std::move(traits), std::move(hash), std::move(eq))
		{ }
		
		hash_container_base(const hash_container_base & val) = delete;
		hash_container_base& operator =(const hash_container_base & val) = delete;

		hash_container_base(hash_container_base && val) = default;
		hash_container_base & operator =(hash_container_base && val) = default;
	};
}
