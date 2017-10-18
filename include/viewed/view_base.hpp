#pragma once
#include <vector>
#include <ext/range/range_traits.hpp>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace viewed
{
	/// This class provides base for building views based on viewed containers.
	/// 
	/// it provides base for other views:
	///  * vector of pointers
	///  * STL compatible interface, indirected, not pointers
	///  * it connects signals from container to handlers(which are virtual and can be overridden)
	///    basic implementation does almost nothing, just synchronizes view with data owning container
	///    derived classes can seal themselves with final on those methods
	/// 
	/// Main container expected to live as long as view does. View holds only non owning pointers to data
	/// 
	/// container must meet following conditions:
	/// * STL compatible interface(types, methods)
	/// * at least forward iterator category, but pointers/references must be stable, iterators can be unstable
	/// * have member type:
	///     signal_range_type - random access range of valid pointers(at least at moment of call) to value_type
	///                         sorted by pointer value
	///     scoped_connection - owning signal connection handle, breaks connection in destructor
	///     
	/// * on_update member function which connects given functor with signal and returns connection.
	///             slot signature is: void (signal_range_type sorted_erased, signal_range_type sorted_updated, signal_range_type inserted).
	///             signal is emitted in process of updating data in container(after update/insert, before erase) with 3 ranges of pointers.
	///             1st points to removed elements, 2nd points to elements that were updated, 3nd to newly inserted.
	///             
	/// * on_erase member function which connects given functor with signal and returns connection.
	///             slot signature is: void (signal_range_type sorted_recsptr_toremove).
	///             signal is emitted before data is erased from container, with range of pointers to elements to erase.
	///             
	/// * on_clear member function which connects given functor with signal and returns connection.
	///             slot signature is: void ().
	///             signal is emitted before container is cleared.
	/// 
	/// 
	/// this class is intended to be inherited and extended to provide more functionality
	/// sorting, filtering, and may be more
	/// 
	/// derived classes should implement:
	///  * update_data
	///  * reinit_view
	///  * clear_view
	///  * erase_records
	///  * initialization in constructor,
	///    this class does not connects signals nor initializes store, but provides methods
	/// (see method declaration for more description)
	///
	/// @Param Container class to which this view will connect and listen updates
	///                  Container must have on_update, on_erase, on_clear signal member functions
	///                  on_update, on_erase provide signal_range_type range,
	///                  random access of pointers to affected elements,
	///                  sorted by pointer value
	template <class Container>
	class view_base
	{
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
		typedef typename container_type::signal_range_type   signal_range_type;
		typedef typename container_type::scoped_connection   scoped_connection;
		typedef std::vector<const value_type *>              store_type;

	public:
		typedef typename store_type::size_type           size_type;
		typedef typename store_type::difference_type     difference_type;

	protected:
		container_type * m_owner = nullptr; // pointer to owning container, can be used by derived classes
		store_type m_store; // view store, can be used by derived classes

		/// raii connections
		scoped_connection m_clear_con;
		scoped_connection m_update_con;
		scoped_connection m_erase_con;

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

		/// normally should not be called outside of view class.
		/// Provided, when view class used directly without inheritance, to complete initialization.
		/// Calls connects signals and calls reinit_view.
		/// 
		/// Derived views probably will automatically call it constructor
		/// or directly connect_signals/reinit_view
		virtual void init();

	protected:
		/// connects container signals to appropriate handlers
		virtual void connect_signals();

		/// container event handlers, those are called on container signals,
		/// you could reimplement them to provide proper handling of your view
		
		/// called when new data is updated in owning container
		/// view have to synchronize itself.
		/// @Param sorted_newrecs range of pointers to updated records, sorted by pointer value
		/// 
		/// default implementation removes erased, appends inserted records, and does nothing with sorted_updated
		virtual void update_data(
			const signal_range_type & sorted_erased,
			const signal_range_type & sorted_updated,
			const signal_range_type & inserted);

		/// called when some records are erased from container
		/// view have to synchronize itself.
		/// @Param sorted_newrecs range of pointers to updated records, sorted by pointer value
		/// 
		/// default implementation, erases those records from main store
		virtual void erase_records(const signal_range_type & sorted_erased);

		/// called when container is cleared, clears m_store.
		virtual void clear_view();
		
	protected:
		/// helper method to make a pointer
		static const_pointer make_pointer(const value_type & val) { return &val; }

		/// removes from m_store records from recs
		/// complexity N log2 M,
		/// where N = m_store.size(), M = recs.size()
		void sorted_erase_records(const signal_range_type & sorted_erased);

	public:
		view_base(container_type * owner) : m_owner(owner) { }
		virtual ~view_base() = default;

		view_base(const view_base &) = delete;
		view_base& operator =(const view_base &) = delete;

		// we have signals move constructor and operator cannot be used
		view_base(view_base &&) = delete;
		view_base& operator =(view_base &&) = delete;
		
	}; //class view_base

	template <class Container>
	void view_base<Container>::connect_signals()
	{
		m_clear_con = m_owner->on_clear([this] { clear_view(); });
		m_update_con = m_owner->on_update([this](const signal_range_type & e, const signal_range_type & u, const signal_range_type & i) { update_data(e, u, i); });
		m_erase_con = m_owner->on_erase([this](const signal_range_type & r) { erase_records(r); });
	}

	template <class Container>
	void view_base<Container>::reinit_view()
	{
		auto rng = *m_owner | boost::adaptors::transformed(make_pointer);
		ext::assign(m_store, boost::begin(rng), boost::end(rng));
	}

	template <class Container>
	void view_base<Container>::init()
	{
		connect_signals();
		reinit_view();
	}

	template <class Container>
	void view_base<Container>::update_data(
		const signal_range_type & sorted_erased,
		const signal_range_type & sorted_updated,
		const signal_range_type & inserted)
	{
		auto first = m_store.begin();
		auto last = m_store.end();

		if (not sorted_erased.empty())
		{
			auto todel = [&sorted_erased](const_pointer rec)
			{
				return boost::binary_search(sorted_erased, rec);
			};

			last = boost::remove_if(m_store, todel);
		}

		auto old_sz = last - first;
		m_store.resize(old_sz + inserted.size());
		boost::copy(inserted, m_store.begin() + old_sz);
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
	void view_base<Container>::sorted_erase_records(const signal_range_type & sorted_erased)
	{
		if (sorted_erased.empty()) return;

		auto todel = [&sorted_erased](const_pointer rec)
		{
			return boost::binary_search(sorted_erased, rec);
		};

		boost::remove_erase_if(m_store, todel);
	}
}
