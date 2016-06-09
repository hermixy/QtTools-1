#pragma once
#include <viewed/view_base.hpp>
#include <viewed/indirect_functor.hpp>

#include <boost/algorithm/cxx11/copy_if.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/adaptors.hpp>

#include <ext/varalgo/sorting_algo.hpp>
#include <ext/varalgo/on_sorted_algo.hpp>

namespace viewed
{
	/// see also view_base description for more information
	/// 
	/// sfview_base is sorted and filtered based on provided SortPred, FilterPred.
	/// Those can be: simple predicate or boost::variant of different predicates.
	///
	/// In derived class you can provide methods like sort_by/filter_by,
	/// which will configure those predicates
	/// on construction this class initializes internal store
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
	class sfview_base : public view_base<Container>
	{
		typedef view_base<Container> base_type;
		typedef sfview_base<Container, SortPred, FilterPred> self_type;

	public:
		using typename base_type::container_type;
		using typename base_type::const_pointer;

		typedef SortPred sort_pred_type;
		typedef FilterPred filter_pred_type;

	protected:
		using typename base_type::store_type;
		using typename base_type::signal_range_type;
		
		using base_type::make_pointer;
		using base_type::sorted_erase_records;
		using base_type::m_owner;

		typedef std::pair <
			typename store_type::const_iterator,
			typename store_type::const_iterator
		> search_hint_type;

	protected:
		filter_pred_type m_filter_pred;
		sort_pred_type m_sort_pred;

	public:
		sort_pred_type sort_pred() const { return m_sort_pred; }
		filter_pred_type filter_pred() const { return m_filter_pred; }

		/// reinitializes view from owner
		void reinit_view() override;

	protected:
		/// merges new records into view, preserving filter/sort order. stable
		void merge_newdata(const signal_range_type & sorted_updated, const signal_range_type & inserted) override;
		/// filters elements that are given in sorted_updated
		void filter_update(const signal_range_type & sorted_updated);

		/// sorts [first; last) with m_sort_pred, stable sort
		template <class RandomAccessIterator>
		void sort(RandomAccessIterator first, RandomAccessIterator last);

		/// merges [middle, last) into [first, last) according to m_sort_pred. stable.
		/// first, middle, last - is are one range, as in std::inplace_merge
		/// if resort_old is true it also resorts [first, middle), otherwise it's assumed it's sorted
		template <class RandomAccessIterator>
		void sort_and_merge(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last, bool resort_old = true);

		/// get pair of iterators that hints where to search element
		search_hint_type search_hint(const_pointer ptr) const;

		/// removes from cont elements that do not pass m_filter_pred
		template <class Cont>
		void remove_filtered(Cont & cont);

		/// copies elements from new_data to view_store elements that satisfy m_filter_pred
		template <class Cont, class SinglePasssRange>
		void copy_filtered(Cont & cont, const SinglePasssRange & new_data);

	public:
		sfview_base(container_type * owner,
		            sort_pred_type sortPred = {},
		            filter_pred_type filterPred = {})
			: base_type(owner),
			  m_sort_pred(std::move(sortPred)),
			  m_filter_pred(std::move(filterPred))
		{ }

		virtual ~sfview_base() = default;

		sfview_base(const sfview_base &) = delete;
		sfview_base & operator =(const sfview_base &) = delete;

		sfview_base(sfview_base &&) = delete;
		sfview_base & operator =(sfview_base &&) = delete;

	}; //class sfview_base

	template <class Container, class SortPred, class FilterPred>
	void sfview_base<Container, SortPred, FilterPred>::reinit_view()
	{
		m_store.clear();
		copy_filtered(m_store, *m_owner | boost::adaptors::transformed(base_type::make_pointer));
		sort(m_store.begin(), m_store.end());
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_base<Container, SortPred, FilterPred>::merge_newdata(const signal_range_type & sorted_updated, const signal_range_type & inserted)
	{
		auto old_sz = m_store.size();
		filter_update(sorted_updated);
		copy_filtered(m_store, inserted);

		auto beg = m_store.begin();
		auto end = m_store.end();
		sort_and_merge(beg, beg + old_sz, end, !sorted_updated.empty());
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_base<Container, SortPred, FilterPred>::filter_update(const signal_range_type & sorted_updated)
	{
		if (!m_filter_pred)
			return;

		auto updates_first = sorted_updated.begin();
		auto updates_last = sorted_updated.end();

		auto todel = [this, updates_first, updates_last](const_pointer rec)
		{
			auto it = ext::binary_find(updates_first, updates_last, rec);
			if (it == updates_last) return false;

			bool passes = m_filter_pred(**it);
			return !passes;
		};

		boost::remove_erase_if(m_store, todel);
	}

	template <class Container, class SortPred, class FilterPred>
	template <class RandomAccessIterator>
	void sfview_base<Container, SortPred, FilterPred>::sort(RandomAccessIterator first, RandomAccessIterator last)
	{
		auto comp = detail::make_indirect_fun(m_sort_pred);
		ext::varalgo::stable_sort(first, last, comp);
	}

	template <class Container, class SortPred, class FilterPred>
	template <class RandomAccessIterator>
	void sfview_base<Container, SortPred, FilterPred>::sort_and_merge(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last, bool resort_old /*= true*/)
	{
		auto comp = detail::make_indirect_fun(m_sort_pred);

		if (resort_old) ext::varalgo::stable_sort(first, middle, comp);
		ext::varalgo::sort(middle, last, comp);
		ext::varalgo::inplace_merge(first, middle, last, comp);
	}

	template <class Container, class SortPred, class FilterPred>
	auto sfview_base<Container, SortPred, FilterPred>::search_hint(const_pointer ptr) const -> search_hint_type
	{
		auto comp = detail::make_indirect_fun(m_sort_pred);
		return ext::varalgo::equal_range(m_store, ptr, comp);
	}

	template <class Container, class SortPred, class FilterPred>
	template <class Cont>
	void sfview_base<Container, SortPred, FilterPred>::remove_filtered(Cont & cont)
	{
		if (m_filter_pred)
		{
			auto pred = [this](const_pointer ptr) { return !m_filter_pred(*ptr); };
			boost::remove_erase_if(cont, pred);
		}
	}

	template <class Container, class SortPred, class FilterPred>
	template <class Cont, class SinglePasssRange>
	void sfview_base<Container, SortPred, FilterPred>::copy_filtered(Cont & cont, const SinglePasssRange & new_data)
	{
		if (!m_filter_pred)
		{
			boost::push_back(cont, new_data);
			return;
		}

		auto pred = detail::make_indirect_fun(m_filter_pred);
		boost::algorithm::copy_if(new_data, std::back_inserter(cont), pred);
	}
}
