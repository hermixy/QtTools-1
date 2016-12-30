#pragma once
#include <vector>
#include <algorithm>
#include <numeric>

#include <viewed/view_qtbase.hpp>
#include <viewed/indirect_functor.hpp>
#include <viewed/get_functor.hpp>
#include <viewed/algorithm.hpp>

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <ext/algorithm.hpp>
#include <ext/iterator/zip_iterator.hpp>

#include <ext/varalgo/sorting_algo.hpp>
#include <ext/varalgo/on_sorted_algo.hpp>

namespace viewed
{
	/// see also view_qtbase description for more information
	/// 
	/// sfview_qtbase is sorted and filtered based on provided SortPred, FilterPred.
	/// Those can be: simple predicate or boost::variant of different predicates.
	///
	/// In derived class you can provide methods like sort_by/filter_by,
	/// which will configure those predicates
	/// on construction this class initializes internal store
	/// 
	/// @Param Container - class to which this view will connect and listen updates, see view_qtbase for more description
	/// @Param SortPred - sort predicate or boost::variant of predicates,
	///                   example: std::less<Type>, boost::varaint<std::less<Type>, std::greater<Type>
	/// @Param FilterPred - filter predicate or boost::variant of predicates,
	///                     NOTE: boost::variant is not supported yet for filter predicate
	template <
		class Container,
		class SortPred,
		class FilterPred
	>
	class sfview_qtbase : public view_qtbase<Container>
	{
		typedef view_qtbase<Container> base_type;
		typedef sfview_qtbase<Container, SortPred, FilterPred> self_type;

	public:
		using typename base_type::container_type;
		using typename base_type::const_pointer;

		typedef SortPred sort_pred_type;
		typedef FilterPred filter_pred_type;

	protected:
		using typename base_type::store_type;
		using typename base_type::signal_range_type;
		using typename base_type::model_type;
		using typename base_type::int_vector;

		using base_type::m_owner;
		using base_type::m_store;
		using base_type::get_model;
		using base_type::make_pointer;
		using base_type::change_indexes;
		using base_type::emit_changed;


		typedef typename store_type::iterator       store_iterator;
		typedef typename store_type::const_iterator store_const_iterator;

		typedef std::pair <
			store_const_iterator,
			store_const_iterator
		> search_hint_type;

	protected:
		filter_pred_type m_filter_pred;
		sort_pred_type m_sort_pred;

	public:
		sort_pred_type sort_pred() const { return m_sort_pred; }
		filter_pred_type filter_pred() const { return m_filter_pred; }

	protected:
		static void mark_pointer(const_pointer & ptr) { reinterpret_cast<std::uintptr_t &>(ptr) |= 1; }
		static bool is_marked(const_pointer ptr) { return reinterpret_cast<std::uintptr_t>(ptr) & 1; }

	public:
		/// reinitializes view from owner
		virtual void reinit_view() override;

	protected:
		/// merges new and updated records into view, preserving filter/sort order. stable
		/// emits appropriate qt signals, uses merge_newdata(iter..., iter..., ...) to calculate index permutations.
		virtual void merge_newdata(const signal_range_type & sorted_updated, const signal_range_type & inserted) override;
		
	protected:
		/// merges m_store's [middle, last) into [first, last) according to m_sort_pred. stable.
		/// first, middle, last - is are one range, as in std::inplace_merge
		/// if resort_old is true it also resorts [first, middle), otherwise it's assumed it's sorted
		virtual void merge_newdata(
			store_iterator first, store_iterator middle, store_iterator last, 
			bool resort_old = true);

		/// merges m_store's [middle, last) into [first, last) according to m_sort_pred. stable.
		/// first, middle, last - is are one range, as in std::inplace_merge
		/// if resort_old is true it also resorts [first, middle), otherwise it's assumed it's sorted
		/// 
		/// range [ifirst, imiddle, ilast) must be permuted the same way as range [first, middle, last)
		virtual void merge_newdata(
			store_iterator first, store_iterator middle, store_iterator last,
			int_vector::iterator ifirst, int_vector::iterator imiddle, int_vector::iterator ilast,
			bool resort_old = true);

		/// sorts m_store's [first; last) with m_sort_pred, stable sort		
		virtual void sort(store_iterator first, store_iterator last);
		/// sorts m_store's [first; last) with m_sort_pred, stable sort
		/// range [ifirst; ilast) must be permuted the same way as range [first; last)
		virtual void sort(store_iterator first, store_iterator last,
		                  int_vector::iterator ifirst, int_vector::iterator ilast);

		/// sorts m_store's [first; last) with m_sort_pred, stable sort
		/// emits qt layoutAboutToBeChanged(..., VerticalSortHint), layoutUpdated(..., VerticalSortHint)
		virtual void sort_and_notify(store_iterator first, store_iterator last);

		/// get pair of iterators that hints where to search element
		virtual search_hint_type search_hint(const_pointer ptr) const;

		/// removes from cont elements that do not pass m_filter_pred
		virtual void remove_filtered(store_type & cont);

		/// appends elements from data to m_store that satisfy m_filter_pred
		virtual void copy_filtered(const signal_range_type & data);
		virtual void copy_filtered(const container_type & data);

	public:
		sfview_qtbase(container_type * owner,
		            sort_pred_type sortPred = {},
		            filter_pred_type filterPred = {})
			: base_type(owner),
			  m_sort_pred(std::move(sortPred)),
			  m_filter_pred(std::move(filterPred))
		{ }

		virtual ~sfview_qtbase() = default;

		sfview_qtbase(const sfview_qtbase &) = delete;
		sfview_qtbase & operator =(const sfview_qtbase &) = delete;

		sfview_qtbase(sfview_qtbase &&) = delete;
		sfview_qtbase & operator =(sfview_qtbase &&) = delete;

	}; //class sfview_qtbase

	template <class Container, class SortPred, class FilterPred>
	void sfview_qtbase<Container, SortPred, FilterPred>::reinit_view()
	{
		auto * model = this->get_model();
		model->beginResetModel();

		m_store.clear();
		copy_filtered(*m_owner);
		sort(m_store.begin(), m_store.end());

		model->endResetModel();
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_qtbase<Container, SortPred, FilterPred>::
		merge_newdata(const signal_range_type & sorted_updated, const signal_range_type & inserted)
	{
		int_vector index_array, affected_indexes;
		int_vector::iterator removed_first, removed_last, changed_first, changed_last;
		store_iterator first, middle, last;

		auto middle_sz = m_store.size();
		removed_first = removed_last = changed_first = changed_last = affected_indexes.begin();
		bool order_changed = false;

		if (!sorted_updated.empty())
		{
			affected_indexes.resize(sorted_updated.size());
			removed_first = affected_indexes.begin();
			removed_last = removed_first;
			changed_first = affected_indexes.end();
			changed_last = changed_first;

			boost::push_back(m_store, sorted_updated);

			first = m_store.begin();
			middle = first + middle_sz;
			last = m_store.end();

			auto test_and_mark = [new_first = middle, new_last = last](const_pointer ptr)
			{
				auto it = std::lower_bound(new_first, new_last, ptr);
				if (it == new_last) return false; 
				if (ptr != *it)     return false;

				mark_pointer(*it);
				return true;
			};

			for (auto it = std::find_if(first, middle, test_and_mark); it != middle; it = std::find_if(it, middle, test_and_mark))
			{
				int row = static_cast<int>(it - first);
				bool passes = !m_filter_pred || m_filter_pred(**it);

				if (passes)   *--changed_first = row;
				else          *removed_last++  = row;
			}

			//emit_changed(changed_first, changed_last);
			order_changed = changed_first != changed_last;

			middle = viewed::remove_indexes(first, middle, removed_first, removed_last);
			last = std::copy_if(first + middle_sz, last, middle, [](auto ptr) { return !is_marked(ptr); });
			
			middle_sz = middle - first;
			m_store.resize(last - first);
		}

		auto * model = get_model();
		Q_EMIT model->layoutAboutToBeChanged(model_type::empty_model_list, model->VerticalSortHint);

		copy_filtered(inserted);

		first = m_store.begin();
		middle = first + middle_sz;
		last = m_store.end();

		constexpr int offset = 0;
		index_array.resize(m_store.size() + (removed_last - removed_first));
		std::iota(index_array.begin(), index_array.end(), offset);

		for (auto it = removed_first; it != removed_last; ++it)
			index_array[*it] = -*it;

		std::stable_partition(index_array.begin(), index_array.end(), [](int v) { return v >= 0; });

		auto ifirst = index_array.begin();
		auto imiddle = ifirst + middle_sz;
		auto ilast = ifirst + m_store.size();

		merge_newdata(
			first, middle, last, 
			ifirst, imiddle, ilast,
			order_changed
		);

		change_indexes(ifirst, ilast, offset);

		Q_EMIT model->layoutChanged(model_type::empty_model_list, model->VerticalSortHint);
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_qtbase<Container, SortPred, FilterPred>::
		merge_newdata(store_iterator first, store_iterator middle, store_iterator last, bool resort_old /*= true*/)
	{
		auto comp = make_indirect_fun(m_sort_pred);

		if (resort_old) ext::varalgo::stable_sort(first, middle, comp);
		ext::varalgo::sort(middle, last, comp);
		ext::varalgo::inplace_merge(first, middle, last, comp);
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_qtbase<Container, SortPred, FilterPred>::
		merge_newdata(store_iterator first, store_iterator middle, store_iterator last,
		              int_vector::iterator ifirst, int_vector::iterator imiddle, int_vector::iterator ilast,
		              bool resort_old /* = true */)
	{
		assert(last - first == ilast - ifirst);
		assert(middle - first == imiddle - ifirst);

		auto comp = make_get_functor<0>(make_indirect_fun(m_sort_pred));

		auto zfirst  = ext::make_zip_iterator(first, ifirst);
		auto zmiddle = ext::make_zip_iterator(middle, imiddle);
		auto zlast   = ext::make_zip_iterator(last, ilast);

		if (resort_old) ext::varalgo::stable_sort(zfirst, zmiddle, comp);
		ext::varalgo::sort(zmiddle, zlast, comp);
		ext::varalgo::inplace_merge(zfirst, zmiddle, zlast, comp);
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_qtbase<Container, SortPred, FilterPred>::sort(store_iterator first, store_iterator last)
	{
		auto comp = make_indirect_fun(m_sort_pred);
		ext::varalgo::stable_sort(first, last, comp);
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_qtbase<Container, SortPred, FilterPred>::
		sort(store_iterator first, store_iterator last,
		     int_vector::iterator ifirst, int_vector::iterator ilast)
	{
		auto comp = make_get_functor<0>(make_indirect_fun(m_sort_pred));

		auto zfirst = ext::make_zip_iterator(first, ifirst);
		auto zlast = ext::make_zip_iterator(last, ilast);
		ext::varalgo::stable_sort(zfirst, zlast, comp);
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_qtbase<Container, SortPred, FilterPred>::sort_and_notify(store_iterator first, store_iterator last)
	{
		auto * model = get_model();
		Q_EMIT model->layoutAboutToBeChanged(model_type::empty_model_list, model->VerticalSortHint);

		int offset = first - m_store.begin();
		int_vector indexes(last - first);
		auto ifirst = indexes.begin();
		auto ilast = indexes.end();
		std::iota(ifirst, ilast, offset);

		sort(first, last, ifirst, ilast);
		change_indexes(ifirst, ilast, offset);

		Q_EMIT model->layoutChanged(model_type::empty_model_list, model->VerticalSortHint);
	}

	template <class Container, class SortPred, class FilterPred>
	auto sfview_qtbase<Container, SortPred, FilterPred>::search_hint(const_pointer ptr) const -> search_hint_type
	{
		auto comp = make_indirect_fun(m_sort_pred);
		return ext::varalgo::equal_range(m_store, ptr, comp);
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_qtbase<Container, SortPred, FilterPred>::remove_filtered(store_type & cont)
	{
		if (m_filter_pred)
		{
			auto pred = [this](const_pointer ptr) { return !m_filter_pred(*ptr); };
			boost::remove_erase_if(cont, pred);
		}
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_qtbase<Container, SortPred, FilterPred>::copy_filtered(const signal_range_type & data)
	{
		if (!m_filter_pred)
		{
			boost::push_back(m_store, data);
			return;
		}

		auto pred = make_indirect_fun(m_filter_pred);
		std::copy_if(data.begin(), data.end(), std::back_inserter(m_store), pred);
	}

	template <class Container, class SortPred, class FilterPred>
	void sfview_qtbase<Container, SortPred, FilterPred>::copy_filtered(const container_type & data)
	{
		auto range = data | boost::adaptors::transformed(make_pointer);
		if (!m_filter_pred)
		{
			boost::push_back(m_store, range);
			return;
		}

		auto pred = make_indirect_fun(m_filter_pred);
		std::copy_if(range.begin(), range.end(), std::back_inserter(m_store), pred);
	}
}
