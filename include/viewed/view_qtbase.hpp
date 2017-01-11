#pragma once
#include <vector>
#include <ext/range/range_traits.hpp>
#include <viewed/qt_model.hpp>
#include <viewed/view_base.hpp>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/signals2/connection.hpp>

namespace viewed
{
	/// This class provides base for building views based on viewed containers.	
	/// It also has knowledge of Qt models and it's signals(beginInsertRows, layoutUpdated, etc),
	/// see also view_base.
	/// 
	/// It knows about qt and emits/calls appropriate methods where needed,
	/// like layoutUpdated, beginInsertRows, etc
	///
	/// Class itself does not inherit QAbstractItemModel, but acquires it via virtual method get_model - 
	/// default implementation uses dynamic_cast.
	template <class Container>
	class view_qtbase : public view_base<Container>
	{
		typedef view_qtbase            self_type;
		typedef view_base<Container>   base_type;

	public:
		using typename base_type::container_type;
		using typename base_type::const_pointer;

	protected:
		using typename base_type::store_type;
		using typename base_type::signal_range_type;

		using base_type::m_store;
		using base_type::m_owner;
		using base_type::make_pointer;

		typedef std::vector<int> int_vector;
		typedef viewed::AbstractItemModel model_type;

	public:
		/// reinitializes view
		/// default implementation just copies from owner
		/// calls qt beginResetModel/endResetModel
		virtual void reinit_view();

	protected:
		/// acquires pointer to qt model, normally you would inherit both QAbstractItemModel and this class.
		/// default implementation uses dynamic_cast
		virtual model_type * get_model();
		/// emits qt signal model->dataChanged about changed rows. Changred rows are defined by [first; last)
		/// default implantation just calls get_model->dataChanged(index(row, 0), inex(row, model->columnCount)
		virtual void emit_changed(int_vector::const_iterator first, int_vector::const_iterator last);
		/// changes persistent indexes via get_model->changePersistentIndex.
		/// [first; last) - range where range[oldIdx] => newIdx.
		/// if newIdx < 0 - index should be removed(changed on invalid, qt supports it)
		virtual void change_indexes(int_vector::const_iterator first, int_vector::const_iterator last, int offset);

	protected:
		/// container event handlers, those are called on container signals, 
		/// you could reimplement them to provide proper handling of your view
		
		/// called when new data is upserted in owning container
		/// view have to synchronize itself.
		/// @Param sorted_newrecs range of pointers to updated records, sorted by pointer value
		/// 
		/// default implementation, appends inserted new records, and does nothing with sorted_updated
		/// calls qt beginInsertRows/endInsertRows
		virtual void merge_newdata(const signal_range_type & sorted_updated, const signal_range_type & inserted) override;

		/// called when some records are erased from container
		/// view have to synchronize itself.
		/// @Param sorted_newrecs range of pointers to updated records, sorted by pointer value
		/// 
		/// default implementation, erases those records from main store
		/// calls qt layoutAboutToBeChanged/layoutChanged
		virtual void erase_records(const signal_range_type & sorted_erased) override;

		/// called when container is cleared, clears m_store.
		/// calls qt beginResetModel/endResetModel
		virtual void clear_view() override;
		
	protected:
		view_qtbase(container_type * owner) : base_type(owner) { }
		virtual ~view_qtbase() = default;

		view_qtbase(const view_qtbase &) = delete;
		view_qtbase& operator =(const view_qtbase &) = delete;

		// we have signals move constructor and operator cannot be used
		view_qtbase(view_qtbase &&) = delete;
		view_qtbase& operator =(view_qtbase &&) = delete;
		
	}; //class view_qtbase

	template <class Container>
	auto view_qtbase<Container>::get_model() -> model_type *
	{
		auto * model = dynamic_cast<QAbstractItemModel *>(this);
		assert(model);
		return static_cast<model_type *>(model);
	}

	template <class Container>
	void view_qtbase<Container>::emit_changed(int_vector::const_iterator first, int_vector::const_iterator last)
	{
		if (first == last) return;

		auto * model = get_model();
		int ncols = model->columnCount(model_type::invalid_index);

		for (; first != last; ++first)
		{
			int row = *first;

			auto top_left = model->index(row, 0, model_type::invalid_index);
			auto bottom_right = model->index(row, ncols - 1, model_type::invalid_index);
			model->dataChanged(top_left, bottom_right, model_type::all_roles);
		}
	}

	template <class Container>
	void view_qtbase<Container>::change_indexes(int_vector::const_iterator first, int_vector::const_iterator last, int offset)
	{
		auto * model = get_model();
		auto size = last - first;

		for (const auto & idx : model->persistentIndexList())
		{
			if (!idx.isValid()) continue;

			auto row = idx.row();
			auto col = idx.column();

			if (row < offset) continue;

			assert(row < size); (void)size;
			model->changePersistentIndex(idx, model->index(first[row - offset], col));
		}
	}

	template <class Container>
	void view_qtbase<Container>::reinit_view()
	{
		auto * model = get_model();
		model->beginResetModel();
		base_type::reinit_view();
		model->endResetModel();
	}

	template <class Container>
	void view_qtbase<Container>::merge_newdata(const signal_range_type & sorted_updated, const signal_range_type & inserted)
	{
		if (inserted.empty()) return;

		int first = static_cast<int>(m_store.size());
		int last  = static_cast<int>(m_store.size() + inserted.size() - 1);
		
		auto * model = get_model();
		model->beginInsertRows(model_type::invalid_index, first, last);
		base_type::merge_newdata(sorted_updated, inserted);
		model->endInsertRows();
	}

	template <class Container>
	void view_qtbase<Container>::erase_records(const signal_range_type & sorted_erased)
	{
		if (sorted_erased.empty()) return;

		auto test = [&sorted_erased](const_pointer ptr)
		{
			return boost::binary_search(sorted_erased, ptr);
		};

		std::vector<int> affected_indexes(sorted_erased.size());
		auto erased_first = affected_indexes.begin();
		auto erased_last = erased_first;
		auto first = m_store.begin();
		auto last = m_store.end();

		for (auto it = std::find_if(first, last, test); it != last; it = std::find_if(it, last, test))
			*erased_last++ = static_cast<int>(it - first);

		auto * model = get_model();
		Q_EMIT model->layoutAboutToBeChanged(model_type::empty_model_list, model->VerticalSortHint);

		auto index_map = viewed::build_relloc_map(erased_first, erased_last, m_store.size());
		viewed::inverse_index_array(index_map.begin(), index_map.end());
		change_indexes(index_map.begin(), index_map.end(), 0);

		last = viewed::remove_indexes(first, last, erased_first, erased_last);
		m_store.resize(last - first);
		
		Q_EMIT model->layoutChanged(model_type::empty_model_list, model->VerticalSortHint);
	}

	template <class Container>
	void view_qtbase<Container>::clear_view()
	{
		auto * model = get_model();
		model->beginResetModel();
		base_type::clear_view();
		model->endResetModel();
	}
}
