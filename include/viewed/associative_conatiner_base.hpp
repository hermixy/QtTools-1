#pragma once
#include <algorithm>
#include <iterator> // for back_inserter
#include <viewed/signal_types.hpp>
#include <ext/try_reserve.hpp>

namespace viewed
{
	/*
	/// container_traits describes store type, update on exist action, etc
	struct container_traits
	{
		//////////////////////////////////////////////////////////////////////////
		///                           types                                         
		//////////////////////////////////////////////////////////////////////////		
		/// container class that stores value_type,
		/// main_store_type should provide stable pointers/references,
		/// iterators allowed to be invalidated on modify operations.
		typedef implementaion_defined main_store_type;
		
		/// container type used for storing raw pointers for views notifications
		typedef implementaion_defined signal_store_type;

		//////////////////////////////////////////////////////////////////////////
		///                   traits functions/functors                          
		//////////////////////////////////////////////////////////////////////////
		/// function interface can be static function members or static functors members.
		/// if overloading isn't needed static function members  - will be ok,
		/// but if you want provide several overloads - use static functors members

		/// constructs main_store_type with provided arguments
		static main_store_type make_store(... main_store_ctor_args);

		/// get_pointer returns pointer to internal_value_type, it used only for filling signal_store_type
		static const value_type * get_pointer(const value_type & val) { return &val; }

		/// update takes current internal_value_type rvalue as first argument and some generic type as second.
		/// It updates current value with new data
		/// usually second type is some reference of value_type 
		struct update_type;
		static const update_type                  update;
	};
	*/

	/// Base container class build on top of some defined by traits associative container
	/// You are expected to inherit it and add more functional.
	/// Generic container with stl compatible interface
	///
	/// It store data in store specified by traits
	/// iterators can be unstable, pointers and references - have to be stable
	/// (this is dictated by views, which stores pointers)
	/// Iterators are read-only. use upsert to add new data
	///
	/// Emits signals when elements added or erased
	/// Can be used to build views on this container, see viewed::view_base
	/// 
	/// @Param Element type
	/// @Param Traits traits class describes various aspects of container,
	///        see description above, also see hash_container_traits and ordered_container_traits for example
	/// TODO: make signal_types a traits parameter
	template <
		class Type,
		class Traits
	>
	class associative_conatiner_base : protected Traits
	{
		typedef associative_conatiner_base<Type, Traits> self_type;
	
	protected:
		typedef Traits traits_type;
		typedef viewed::signal_types<Type> signal_types;
	
		typedef typename traits_type::main_store_type       main_store_type;
		typedef typename traits_type::signal_store_type     signal_store_type;

	public:
		//container interface
		typedef       Type      value_type;
		typedef const Type &    const_reference;
		typedef const Type *    const_pointer;

		typedef const_pointer   pointer;
		typedef const_reference reference;

		typedef typename main_store_type::size_type        size_type;
		typedef typename main_store_type::difference_type  difference_type;

		typedef typename main_store_type::const_iterator const_iterator;
		typedef                           const_iterator iterator;

	public:
		/// forward signal types
		typedef typename signal_types::signal_range_type   signal_range_type;
		typedef typename signal_types::upsert_signal_type  upsert_signal_type;
		typedef typename signal_types::erase_signal_type   erase_signal_type;
		typedef typename signal_types::clear_signal_type   clear_signal_type;

	protected:
		main_store_type m_store;

		upsert_signal_type m_upsert_signal;
		erase_signal_type  m_erase_signal;
		clear_signal_type  m_clear_signal;

	public:
		const_iterator begin()  const noexcept { return m_store.cbegin(); }
		const_iterator end()    const noexcept { return m_store.cend(); }
		const_iterator cbegin() const noexcept { return m_store.cbegin(); }
		const_iterator cend()   const noexcept { return m_store.cend(); }

		template <class CompatibleKey>
		const_iterator find(const CompatibleKey & key) const { return m_store.find(key); }
		template <class CompatibleKey>
		const_iterator count(const CompatibleKey & key) const { return m_store.count(key); }

		size_type size() const noexcept { return m_store.size(); }
		bool empty()     const noexcept { return m_store.empty(); }

		/// signals
		template <class... Args> boost::signals2::connection on_erase(Args && ... args)  { return m_erase_signal.connect(std::forward<Args>(args)...); }
		template <class... Args> boost::signals2::connection on_upsert(Args && ... args) { return m_upsert_signal.connect(std::forward<Args>(args)...); }
		template <class... Args> boost::signals2::connection on_clear(Args && ... args)  { return m_clear_signal.connect(std::forward<Args>(args)...); }

	protected:
		/// finds and updates or appends elements from [first; last) into internal store m_store
		/// those elements also placed into upserted_recs for further notifications of views
		template <class SinglePassIterator>
		void upsert_newrecs(SinglePassIterator first, SinglePassIterator last,
							signal_store_type & updated, signal_store_type & inserted);

		/// erases elements [first, last) from attached views
		void erase_from_views(const_iterator first, const_iterator last);

		/// notifies views about new data 
		void merge_newdata_into_views(signal_store_type & updated, signal_store_type & inserted);

	public:
		/// erases all elements
		void clear();
		/// erases elements [first, last) from internal store and views
		/// [first, last) must be a valid range
		const_iterator erase(const_iterator first, const_iterator last);
		/// erase element pointed by it
		const_iterator erase(const_iterator it) { return erase(it, std::next(it)); }

		/// upserts new record from [first, last)
		/// records which are already in container will be replaced with new ones
		template <class SinglePassIterator>
		void upsert(SinglePassIterator first, SinglePassIterator last);
		void upsert(std::initializer_list<value_type> ilist) { upsert(std::begin(ilist), std::end(ilist)); }

		/// clear container and assigns elements from [first, last)
		template <class SinglePassIterator>
		void assign(SinglePassIterator first, SinglePassIterator last);
		void assign(std::initializer_list<value_type> ilist) { assign(std::begin(ilist), std::end(ilist)); }

	protected:
		template <class ... StoreArgs>
		associative_conatiner_base(traits_type traits = {}, StoreArgs && ... storeArgs)
			: traits_type(std::move(traits)),
			  m_store(traits_type::make_store(std::forward<StoreArgs>(storeArgs)...))
		{};

		associative_conatiner_base(const associative_conatiner_base &) = delete;
		associative_conatiner_base & operator =(const associative_conatiner_base &) = delete;

		associative_conatiner_base(associative_conatiner_base && op) = default;
		associative_conatiner_base & operator =(associative_conatiner_base && op) = default;
	};

	template <class Type, class Traits>
	template <class SinglePassIterator>
	void associative_conatiner_base<Type, Traits>::upsert_newrecs
		(SinglePassIterator first, SinglePassIterator last,
		 signal_store_type & updated, signal_store_type & inserted)
	{
		ext::try_reserve(updated, first, last);
		ext::try_reserve(inserted, first, last);

		for (; first != last; ++first)
		{
			auto && val = *first;

			typename main_store_type::const_iterator where;
			bool inserted_into_store;
			std::tie(where, inserted_into_store) = m_store.insert(std::forward<decltype(val)>(val));

			if (inserted_into_store) {
				inserted.push_back(traits_type::get_pointer(*where));
			}
			else {
				traits_type::update(const_cast<value_type &>(*where), std::forward<decltype(val)>(val));
				updated.push_back(traits_type::get_pointer(*where));
			}
		}
	}

	template <class Type, class Traits>
	void associative_conatiner_base<Type, Traits>::erase_from_views(const_iterator first, const_iterator last)
	{
		signal_store_type todel;
		std::transform(first, last, std::back_inserter(todel), traits_type::get_pointer);
		std::sort(todel.begin(), todel.end());

		auto rawRange = boost::make_iterator_range(todel.data(), todel.data() + todel.size());
		m_erase_signal(rawRange);
	}

	template <class Type, class Traits>
	void associative_conatiner_base<Type, Traits>::clear()
	{
		m_clear_signal();
		m_store.clear();
	}

	template <class Type, class Traits>
	void associative_conatiner_base<Type, Traits>::merge_newdata_into_views(signal_store_type & updated, signal_store_type & inserted)
	{
		std::sort(updated.begin(), updated.end());
		auto urr = boost::make_iterator_range(updated.data(), updated.data() + updated.size());
		auto irr = boost::make_iterator_range(inserted.data(), inserted.data() + inserted.size());
		m_upsert_signal(urr, irr);
	}


	template <class Type, class Traits>
	auto associative_conatiner_base<Type, Traits>::erase(const_iterator first, const_iterator last) -> const_iterator
	{
		erase_from_views(first, last);
		return m_store.erase(first, last);
	}

	template <class Type, class Traits>
	template <class SinglePassIterator>
	void associative_conatiner_base<Type, Traits>::upsert(SinglePassIterator first, SinglePassIterator last)
	{
		signal_store_type updated, inserted;
		upsert_newrecs(first, last, updated, inserted);
		merge_newdata_into_views(updated, inserted);
	}

	template <class Type, class Traits>
	template <class SinglePassIterator>
	void associative_conatiner_base<Type, Traits>::assign(SinglePassIterator first, SinglePassIterator last)
	{
		clear();

		m_store.insert(first, last);

		signal_store_type to_notify, dummy;
		to_notify.resize(m_store.size());
		std::transform(m_store.begin(), m_store.end(), to_notify.begin(), traits_type::get_pointer);

		merge_newdata_into_views(dummy, to_notify);
	}
}