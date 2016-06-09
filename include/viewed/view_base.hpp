#pragma once
#include <vector>
#include <ext/algorithm.hpp>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/signals2/connection.hpp>

namespace viewed
{
	/// This class provides base for building views,
	/// but not required to be base class, as long as you correctly handle signals - you can do anything.
	/// it provides base for other views:
	///  * vector of pointers,
	///  * stl compatible interface, indirected, not pointers
	///  * it connects signals from container to handlers(which are virtual and can be overridden)
	///    basic implementation does almost nothing, just synchronizes view with data owning container
	///    derived classes can seal themselves with final on those methods
	///
	/// Main container expected to live as long as view does. View holds only non owning pointers to data
	/// 
	/// container must meet following conditions: 
	/// * stl compatible interface
	/// * at least forward iterator category, but pointers/references must be stable, iterators can be unstable
	/// * have member type:
	///     signal_range_type, which is a random access range of valid pointers(at least at moment of call) to value_type
	///                        sorted by pointer value
	///                        
	/// * on_upsert member function which connects given functor with boost::signals2 signal and returns connection.
	///             slot signature is: void (signal_range_type sorted_updated, signal_range_type inserted).
	///             signal is emitted after new data is upserted into container, with 2 ranges of pointers. 
	///             1st points to elements that were updated, 2nd to newly inserted.
	///             
	/// * on_erase member function which connects given functor with boost::signals2 signal and returns connection.
	///             slot signature is: void (signal_range_type sorted_recsptr_toremove).
	///             signal is emitted before data is erased from container, with range of pointers to elements to erase.
	///             
	/// * on_clear member function which connects given functor with boost::signals2 signal and returns connection.
	///             slot signature is: void ().
	///             signal is emitted before container is cleared.
	/// 
	/// 
	/// this class is intended to be inherited and extended to provide more functionality
	/// sorting, filtering, and may be more
	/// 
	/// derived classes should implement:
	///  * merge_newdata
	///  * reinit_view
	///  * clear
	///  * erase_records
	///  * initialization in constructor,
	///    this class does not connects signals nor initializes store, but provides methods
	/// (see method declaration for more description)
	///
	/// @Param Container class to which this view will connect and listen updates
	///                  Container must have on_upsert, on_erase, on_clear signal member functions
	///                  on_upsert, on_erase provide signal_range_type range, 
	///                  random access of pointers to affected elements, 
	///                  sorted by pointer value
	template <class Container>
	class view_base
	{
		//typedef view_base<Container> self_type;
		typedef view_base self_type;

	public:
		typedef Container container_type;
		/// container interface typedefs
		typedef typename container_type::value_type      value_type;
		typedef const value_type *                       const_pointer;
		typedef const value_type &                       const_reference;
		typedef const_reference                          reference;
		typedef const_pointer                            pointer;

	protected:
		typedef std::vector<const value_type *> store_type;
		typedef typename container_type::signal_range_type signal_range_type;

	public:
		typedef typename store_type::size_type           size_type;
		typedef typename store_type::difference_type     difference_type;

	protected:
		container_type * m_owner = nullptr; // pointer to owning container, can be used by derived classes
		store_type m_store; // view store, can be used by derived classes

		/// raii connections
		boost::signals2::scoped_connection m_clear_con;
		boost::signals2::scoped_connection m_upsert_con;
		boost::signals2::scoped_connection m_erase_con;

	public:
		/// container interface
		typedef boost::indirect_iterator <
			typename store_type::const_iterator
		> const_iterator;
		typedef boost::indirect_iterator <
			typename store_type::const_reverse_iterator
		> const_reverse_iterator;

		typedef const_iterator iterator;
		typedef const_reverse_iterator reverse_iterator;

		const_iterator begin() const noexcept { return m_store.begin(); }
		const_iterator end() const noexcept { return m_store.end(); }
		const_iterator cbegin() const noexcept { return m_store.cbegin(); }
		const_iterator cend() const noexcept { return m_store.cend(); }

		const_reverse_iterator rbegin() const noexcept { return m_store.rbegin(); }
		const_reverse_iterator rend() const noexcept { return m_store.rend(); }
		const_reverse_iterator crbegin() const noexcept { return m_store.crbegin(); }
		const_reverse_iterator crend() const noexcept { return m_store.crend(); }

		const_reference at(size_type idx) const { return *m_store.at(idx); }
		const_reference operator [](size_type idx) const { return *m_store.operator[](idx); }
		const_reference front() const { return *m_store.front(); }
		const_reference back() const { return *m_store.back(); }

		size_type size() const noexcept { return m_store.size(); }
		bool empty() const noexcept { return m_store.empty(); }

	public:
		/// returns pointer to owning container
		container_type * get_owner() const noexcept { return m_owner; }

		/// reinitializes view
		/// default implementation just copies from owner
		virtual void reinit_view();

	protected:
		/// container event handlers, those are called on container signals, 
		/// you could reimplement them to provide proper handling of your view
		
		/// called when new data is upserted in owning container
		/// view have to synchronize itself.
		/// @Param sorted_newrecs range of pointers to updated records, sorted by pointer value
		/// 
		/// default implementation, appends inserted new records, and does nothing with sorted_updated
		virtual void merge_newdata(const signal_range_type & sorted_updated, const signal_range_type & inserted);

		/// called when some records are erased from container
		/// view have to synchronize itself.
		/// @Param sorted_newrecs range of pointers to updated records, sorted by pointer value
		/// 
		/// default implementation, erases those records from main store
		virtual void erase_records(const signal_range_type & sorted_erased);

		virtual void clear_view();
		virtual void connect_signals();
		
	protected:
		/// helper method to make a pointer
		static const_pointer make_pointer(const value_type & val) { return &val; }

		/// removes from m_store records from recs
		/// complexity N log2 M, 
		/// where N = m_store.size(), M = recs.size()
		void sorted_erase_records(const signal_range_type & recs);

	protected:
		view_base(container_type * owner) : m_owner(owner) { }
		virtual ~view_base() = default;

		view_base(const view_base &) = delete;
		view_base& operator =(const view_base &) = delete;

		// we have signals move constructor and operator cannot be used
		view_base(view_base &&) = delete;
		view_base& operator =(view_base &&) = delete;
		
	}; //class view_base

	template <class Container>
	void view_base<Container>::reinit_view()
	{
		m_store.clear();
		boost::push_back(m_store, *m_owner | boost::adaptors::transformed(make_pointer));
	}

	template <class Container>
	void view_base<Container>::merge_newdata(const signal_range_type & sorted_updated, const signal_range_type & inserted)
	{
		boost::push_back(m_store, inserted);
	}

	template <class Container>
	void view_base<Container>::erase_records(const signal_range_type & sorted_erased)
	{
		sorted_erase_records(sorted_erased);
	}

	template <class Container>
	void view_base<Container>::clear_view()
	{
		m_store.clear();
	}

	template <class Container>
	void view_base<Container>::connect_signals()
	{
		m_clear_con = m_owner->on_clear([this] { clear_view(); });
		m_upsert_con = m_owner->on_upsert([this](const signal_range_type & u, const signal_range_type & i) { merge_newdata(u, i); });
		m_erase_con = m_owner->on_erase([this](const signal_range_type & r) { erase_records(r); });
	}

	template <class Container>
	void view_base<Container>::sorted_erase_records(const signal_range_type & recs)
	{
		auto todel = [&recs](const_pointer rec) {
			return boost::binary_search(recs, rec);
		};

		boost::remove_erase_if(m_store, todel);
	}
}
