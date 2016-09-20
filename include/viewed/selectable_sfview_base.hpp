#pragma once
#include <cassert>
#include <viewed/sfview_base.hpp>
#include <boost/container/flat_set.hpp>
#include <ext/algorithm/slide.hpp>

namespace viewed
{

	/// see also view_base description for more information
	/// 
	/// selectable_sfview_base is sorted and filtered based on provided SortPred, FilterPred.
	/// Those can be: simple predicate or boost::variant of different predicates.
	///
	/// In derived class you can provide methods like sort_by/filter_by,
	/// which will configure those predicates
	/// 
	/// it also provides an ability to select some elements(mark them as selected)
	/// and then partition on those elements
	/// 
	/// @Param Container - class to which this view will connect and listen updates, see view_base for more description
	/// @Param SortPred - sort predicate or boost::variant of predicates,
	///                   example: std::less<Type>, boost::varaint<std::less<Type>, std::greater<Type>
	/// @Param FilterPred - filter predicate or boost::variant of predicates,
	///                     NOTE: boost::variant is'n supported yet for filter predicate
	template <
		class Container,
		class SortPred,
		class FilterPred
	>
	class selectable_sfview_base : public sfview_base<Container, SortPred, FilterPred>
	{
		typedef sfview_base<Container, SortPred, FilterPred> base_type;
		typedef selectable_sfview_base<Container, SortPred, FilterPred> self_type;
	
	public:
		using typename base_type::container_type;
		using typename base_type::sort_pred_type;
		using typename base_type::filter_pred_type;

		using typename base_type::value_type;
		using typename base_type::const_pointer;
		using typename base_type::const_iterator;
		using typename base_type::iterator;

	protected:
		using typename base_type::store_type;
		using typename base_type::signal_range_type;
		using typename base_type::search_hint_type;
		using typename base_type::store_iterator;

		using base_type::make_pointer;
		using base_type::m_owner;
		using base_type::m_store;

	protected:
		typedef boost::container::flat_set<const value_type *> selection_set_type;
		selection_set_type m_selection_set;

		bool m_partition_by_selection = false;
		bool m_partition_by_selection_asc = true;

	public:
		bool is_partiotioned_by_selection() const { return m_partition_by_selection; }
		bool is_partiotioned_by_selection_asc() const { return m_partition_by_selection_asc; }

		void select(iterator it)                    { set_selected(it, true); }
		void deselect(iterator it)                  { set_selected(it, false); }
		void toggle_selected(iterator it)           { set_selected(it, !is_selected(it)); }

		/// returns a const range of selected elements, 
		/// order of elements is undefined
		const selection_set_type & seleted_elements() const { return m_selection_set; }

		/// sets element by it to selected state.
		/// adjusts order if we are partitioned by selection.
		/// returns iterator after adjustion.
		virtual iterator set_selected(iterator it, bool selected);
		virtual bool is_selected(const_iterator it) const   { return m_selection_set.count(*it.base()); }

		virtual void clear_selection();

	protected:
		/// rotates m_store so, that:
		///   if {ext_it} was part of partition it moved just after partition
		///   if {ext_it} was not part of partition it moved at the end of partition
		/// this method must be called only when m_partition_by_selection == true
		/// returns new position of element pointer by ext_it
		virtual iterator adjust_selection_partition(iterator ext_it);

		virtual void erase_records(const signal_range_type & sorted_erased) override;
		virtual void clear_view() override;
		
		virtual void partition_by_selection(store_iterator first, store_iterator last);

		/// sorts [first; last) with m_sort_pred, stable sorts		
		virtual void sort(store_iterator first, store_iterator last) override;

		/// merges [middle, last) into [first, last) according to m_sort_pred. stable.
		/// first, middle, last - is are one range, as in std::inplace_merge
		/// if resort_old is true it also resorts [first, middle), otherwise it's assumed it's sorted
		virtual void sort_and_merge(store_iterator first, store_iterator middle, store_iterator last, bool resort_old = true) override;

		/// get pair of iterators that hints where to search element
		virtual search_hint_type search_hint(const_pointer ptr) const override;

	protected:
		selectable_sfview_base(container_type * owner,
		                       sort_pred_type sortPred = {},
		                       filter_pred_type filterPred = {})
			: base_type(owner, std::move(sortPred), std::move(filterPred))
		{}

		virtual ~selectable_sfview_base() = default;
		selectable_sfview_base(const selectable_sfview_base &) = delete;
		selectable_sfview_base & operator =(const selectable_sfview_base &) = delete;
	};



	template <class Container, class SortPred, class FilterPred>
	auto selectable_sfview_base<Container, SortPred, FilterPred>::set_selected(iterator it, bool selected) -> iterator
	{
		auto ptr = *it.base();

		if (!m_partition_by_selection)
		{
			if (selected)
				m_selection_set.insert(ptr);
			else
				m_selection_set.erase(ptr);
			return it;
		}
		else
		{
			auto sel_it = m_selection_set.lower_bound(ptr);
			bool found = sel_it != m_selection_set.end() && *sel_it == ptr;

			// selected | found    action
			//    0     |   0      do nothing
			//    0     |   1      insert and adjust
			//    1     |   0      erase  and adjust
			//    1     |   1      do nothing
			if (selected ^ found)
			{
				it = adjust_selection_partition(it);
				if (selected)
					m_selection_set.insert(sel_it, ptr);
				else
					m_selection_set.erase(sel_it);
			}

			return it;
		}
	}

	template <class Container, class SortPred, class FilterPred>
	void selectable_sfview_base<Container, SortPred, FilterPred>::clear_selection()
	{
		/// we clearing all selection, if we are partitioned by selection
		/// cleared elements must be rotated out of partitioned part,
		/// but because we are clearing - that part will become empty, 
		/// so we can leave it as is
		m_selection_set.clear();
	}

	template <class Container, class SortPred, class FilterPred>
	auto selectable_sfview_base<Container, SortPred, FilterPred>::adjust_selection_partition(iterator ext_it) -> iterator
	{
		assert(m_partition_by_selection);
		auto pos = ext_it - begin(); // make iterator from const_iterator
		auto it = m_store.begin() + pos;
		decltype(it) pp;

		if (m_partition_by_selection_asc) {
			auto pred = [this](pointer ptr) { return m_selection_set.count(ptr) != 0; };
			pp = std::partition_point(m_store.begin(), m_store.end(), pred);
		}
		else {
			auto pred = [this](pointer ptr) { return m_selection_set.count(ptr) == 0; };
			pp = std::partition_point(m_store.begin(), m_store.end(), pred);
		}

		return ext::slide(it, it + 1, pp).first;
	}

	template <class Container, class SortPred, class FilterPred>
	void selectable_sfview_base<Container, SortPred, FilterPred>::erase_records(const signal_range_type & sorted_erased)
	{
		for (auto ptr : sorted_erased)
			m_selection_set.erase(ptr);

		base_type::erase_records(sorted_erased);
	}

	template <class Container, class SortPred, class FilterPred>
	void selectable_sfview_base<Container, SortPred, FilterPred>::clear_view()
	{
		m_selection_set.clear();
		base_type::clear_view();
	}

	template <class Container, class SortPred, class FilterPred>
	void selectable_sfview_base<Container, SortPred, FilterPred>::partition_by_selection(store_iterator first, store_iterator last)
	{
		typedef const value_type * pointer;
		if (m_partition_by_selection_asc) 
		{
			auto pred = [this](pointer ptr) { return m_selection_set.count(ptr) != 0; };
			std::stable_partition(first, last, pred);
		}
		else
		{
			auto pred = [this](pointer ptr) { return m_selection_set.count(ptr) == 0; };
			std::stable_partition(first, last, pred);
		}
	}

	template <class Container, class SortPred, class FilterPred>
	void selectable_sfview_base<Container, SortPred, FilterPred>::sort(store_iterator first, store_iterator last)
	{
		if (m_partition_by_selection)
			partition_by_selection(first, last);
		else
			base_type::sort(first, last);
	}

	template <class Container, class SortPred, class FilterPred>
	void selectable_sfview_base<Container, SortPred, FilterPred>::sort_and_merge(store_iterator first, store_iterator middle, store_iterator last, bool resort_old /*= true*/)
	{
		if (m_partition_by_selection)
			partition_by_selection(first, last);
		else
			base_type::sort_and_merge(first, middle, last, resort_old);
	}

	template <class Container, class SortPred, class FilterPred>
	auto selectable_sfview_base<Container, SortPred, FilterPred>::search_hint(const_pointer ptr) const -> search_hint_type
	{
		if (!m_partition_by_selection)
			return base_type::search_hint(ptr);

		auto begIt = m_store.begin();
		auto endIt = m_store.end();
		
		bool asc = m_partition_by_selection_asc;
		decltype(begIt) pp;
		if (asc) 
		{
			auto pred = [this](const_pointer ptr) { return m_selection_set.count(ptr) != 0; };
			pp = std::partition_point(begIt, endIt, pred);
		}
		else
		{
			auto pred = [this](const_pointer ptr) { return m_selection_set.count(ptr) == 0; };
			pp = std::partition_point(begIt, endIt, pred);
		}

		// asc | sel -> action
		//  0     0  -> search [begIt; pp)
		//  0     1  -> search [pp; endIt)
		//  1     0  -> search [pp; endIt)
		//  1     1  -> search [begIt; pp)
		if ((m_selection_set.count(ptr) != 0) ^ asc)
			begIt = pp;
		else
			endIt = pp;

		return {begIt, endIt};
	}
}
