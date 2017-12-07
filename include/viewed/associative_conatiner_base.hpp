#pragma once
#include <algorithm>
#include <iterator> // for back_inserter
#include <viewed/signal_traits.hpp>
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

	/*
	/// signal_traits describes types used for communication between stores and views
	struct signal_traits
	{
		/// random access range of valid pointers(at least at moment of call) to value_type
		typedef implementation-defined signal_range_type;
		
		/// static functor/method for creating signal_range_type from some signal_store_type
		static signal_range_type make_range(const Type ** first, const Type ** last);

		/// signals should be boost::signals or compatible:
		/// * connect call, connection, scoped_connection
		/// * emission via call like: signal(args...)
		
		typedef implementation-defined connection;
		typedef implementation-defined scoped_connection;

		/// signal is emitted in process of updating data in container(after update/insert, before erase) with 3 ranges of pointers.
		///  * 1st to erased elements, sorted by pointer value
		///  * 2nd points to elements that were updated, sorted by pointer value
		///  * 3rd to newly inserted, order is unspecified
		///  signature: void(signal_range_type sorted_erased, signal_range_type sorted_updated, signal_range_type inserted)
		typedef implementation-defined update_signal_type;

		/// signal is emitted before data is erased from container,
		/// with range of pointers to elements to erase, sorted by pointer value
		/// signature: void(signal_range_type erased),
		typedef implementation-defined erase_signal_type;

		/// signal is emitted before container is cleared.
		/// signature: void(),
		typedef implementation-defined clear_signal_type;
	}
	*/

	/// Base container class build on top of some defined by traits associative container
	/// You are expected to inherit it and add more functional.
	/// Generic container with STL compatible interface
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
	/// @Param SignalTraits traits class describes various aspects of signaling,
	///        see description above, also see default_signal_traits
	template <
		class Type,
		class Traits,
		class SignalTraits = default_signal_traits<Type>
	>
	class associative_conatiner_base : protected Traits
	{
		typedef associative_conatiner_base<Type, Traits, SignalTraits> self_type;
	
	protected:
		typedef Traits        traits_type;
		typedef SignalTraits  signal_traits;
	
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
		typedef typename signal_traits::connection          connection;
		typedef typename signal_traits::scoped_connection   scoped_connection;

		typedef typename signal_traits::signal_range_type   signal_range_type;
		typedef typename signal_traits::update_signal_type  update_signal_type;
		typedef typename signal_traits::erase_signal_type   erase_signal_type;
		typedef typename signal_traits::clear_signal_type   clear_signal_type;

	protected:
		main_store_type m_store;

		update_signal_type m_update_signal;
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
		template <class... Args> connection on_erase(Args && ... args)  { return m_erase_signal.connect(std::forward<Args>(args)...); }
		template <class... Args> connection on_update(Args && ... args) { return m_update_signal.connect(std::forward<Args>(args)...); }
		template <class... Args> connection on_clear(Args && ... args)  { return m_clear_signal.connect(std::forward<Args>(args)...); }

	protected:
		static void mark_pointer(const_pointer & ptr) { reinterpret_cast<std::uintptr_t &>(ptr) |= 1; }
		static bool is_marked(const_pointer ptr) { return reinterpret_cast<std::uintptr_t>(ptr) & 1; }

	protected:
		/// finds and updates or appends elements from [first; last) into internal store m_store
		/// those elements also placed into upserted_recs for further notifications of views
		template <class SinglePassIterator>
		void upsert_newrecs(SinglePassIterator first, SinglePassIterator last);

		template <class SinglePassIterator>
		void assign_newrecs(SinglePassIterator first, SinglePassIterator last);

		/// erases elements [first, last) from attached views
		void erase_from_views(const_iterator first, const_iterator last);

		/// notifies views about update
		void notify_views(signal_store_type & erased, signal_store_type & updated, signal_store_type & inserted);

	public:
		/// erases all elements
		void clear();
		/// erases elements [first, last) from internal store and views
		/// [first, last) must be a valid range
		const_iterator erase(const_iterator first, const_iterator last);
		/// erase element pointed by it
		const_iterator erase(const_iterator it) { return erase(it, std::next(it)); }
		/// erase element by key
		template <class CompatibleKey>
		size_type erase(const CompatibleKey & key);

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

	template <class Type, class Traits, class SignalTraits>
	template <class SinglePassIterator>
	void associative_conatiner_base<Type, Traits, SignalTraits>::upsert_newrecs
		(SinglePassIterator first, SinglePassIterator last)
	{
		signal_store_type erased, updated, inserted;
		ext::try_reserve(updated, first, last);
		ext::try_reserve(inserted, first, last);

		for (; first != last; ++first)
		{
			auto && val = *first;

			typename main_store_type::const_iterator where;
			bool inserted_into_store;
			std::tie(where, inserted_into_store) = m_store.insert(std::forward<decltype(val)>(val));

			auto * ptr = traits_type::get_pointer(*where);
			if (inserted_into_store)
			{
				inserted.push_back(ptr);
			}
			else
			{
				traits_type::update(const_cast<value_type &>(*where), std::forward<decltype(val)>(val));
				updated.push_back(ptr);
			}
		}

		std::sort(updated.begin(), updated.end());
		notify_views(erased, updated, inserted);
	}

	template <class Type, class Traits, class SignalTraits>
	template <class SinglePassIterator>
	void associative_conatiner_base<Type, Traits, SignalTraits>::assign_newrecs
		(SinglePassIterator first, SinglePassIterator last)
	{
		signal_store_type erased, updated, inserted;
		ext::try_reserve(updated, first, last);
		ext::try_reserve(inserted, first, last);

		erased.resize(m_store.size());

		auto erased_first = erased.begin();
		auto erased_last = erased.end();
		std::transform(m_store.begin(), m_store.end(), erased_first, traits_type::get_pointer);
		std::sort(erased_first, erased_last);

		for (; first != last; ++first)
		{
			auto && val = *first;

			typename main_store_type::const_iterator where;
			bool inserted_into_store;
			std::tie(where, inserted_into_store) = m_store.insert(std::forward<decltype(val)>(val));

			auto * ptr = traits_type::get_pointer(*where);
			if (inserted_into_store)
			{
				inserted.push_back(ptr);
			}
			else
			{
				traits_type::update(const_cast<value_type &>(*where), std::forward<decltype(val)>(val));
				updated.push_back(ptr);

				// remove found item from erase list
				auto it = std::lower_bound(erased_first, erased_last, ptr);
				if (it != erased_last and *it == ptr) mark_pointer(*it);
			}
		}

		erased_last = std::remove_if(erased_first, erased_last, is_marked);
		erased.erase(erased_last, erased.end());
		std::sort(updated.begin(), updated.end());
		notify_views(erased, updated, inserted);

		for (auto * ptr : erased) m_store.erase(*ptr);
	}

	template <class Type, class Traits, class SignalTraits>
	void associative_conatiner_base<Type, Traits, SignalTraits>::erase_from_views(const_iterator first, const_iterator last)
	{
		signal_store_type todel;
		std::transform(first, last, std::back_inserter(todel), traits_type::get_pointer);
		std::sort(todel.begin(), todel.end());

		auto rawRange = signal_traits::make_range(todel.data(), todel.data() + todel.size());
		m_erase_signal(rawRange);
	}

	template <class Type, class Traits, class SignalTraits>
	void associative_conatiner_base<Type, Traits, SignalTraits>::clear()
	{
		m_clear_signal();
		m_store.clear();
	}

	template <class Type, class Traits, class SignalTraits>
	void associative_conatiner_base<Type, Traits, SignalTraits>::notify_views
		(signal_store_type & erased, signal_store_type & updated, signal_store_type & inserted)
	{
		auto urr = signal_traits::make_range(updated.data(), updated.data() + updated.size());
		auto irr = signal_traits::make_range(inserted.data(), inserted.data() + inserted.size());
		auto err = signal_traits::make_range(erased.data(), erased.data() + erased.size());
		m_update_signal(err, urr, irr);
	}


	template <class Type, class Traits, class SignalTraits>
	auto associative_conatiner_base<Type, Traits, SignalTraits>::erase(const_iterator first, const_iterator last) -> const_iterator
	{
		erase_from_views(first, last);
		return m_store.erase(first, last);
	}

	template <class Type, class Traits, class SignalTraits>
	template <class CompatibleKey>
	auto associative_conatiner_base<Type, Traits, SignalTraits>::erase(const CompatibleKey & key) -> size_type
	{
		const_iterator first, last;
		std::tie(first, last) = m_store.equal_range(key);
		auto count = std::distance(first, last);
		erase(first, last);
		return count;
	}

	template <class Type, class Traits, class SignalTraits>
	template <class SinglePassIterator>
	inline void associative_conatiner_base<Type, Traits, SignalTraits>::upsert(SinglePassIterator first, SinglePassIterator last)
	{
		return upsert_newrecs(first, last);
	}

	template <class Type, class Traits, class SignalTraits>
	template <class SinglePassIterator>
	inline void associative_conatiner_base<Type, Traits, SignalTraits>::assign(SinglePassIterator first, SinglePassIterator last)
	{
		return assign_newrecs(first, last);
	}
}
