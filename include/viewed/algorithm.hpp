#pragma once
#include <cstdint>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ext/type_traits.hpp>

namespace viewed
{
	namespace detail
	{
		template <class Pred> static std::enable_if_t<    ext::static_castable_v<Pred, bool>, bool> active_visitor(const Pred & pred) { return static_cast<bool>(pred); }
		template <class Pred> static std::enable_if_t<not ext::static_castable_v<Pred, bool>, bool> active_visitor(const Pred & pred) { return true; }
	}

	template <class Pred> static bool active(Pred && pred)
	{
		auto vis = [](const auto & pred) { return detail::active_visitor(pred); };
		return varalgo::variant_traits<std::decay_t<Pred>>::visit(vis, std::forward<Pred>(pred));
	}

	// defined in <viewed/qt_model.hqt>
	class AbstractItemModel;

	/// inverses index array in following way:
	/// inverse[arr[i] - offset] = i for first..last.
	/// This is for when you have array of arr[new_index] => old_index,
	/// but need arr[old_index] => new_idx for qt changePersistentIndex
	template <class RandomAccessIterator>
	void inverse_index_array(RandomAccessIterator first, RandomAccessIterator last,
	                         typename std::iterator_traits<RandomAccessIterator>::value_type offset = 0)
	{
		typedef typename std::iterator_traits<RandomAccessIterator>::value_type value_type;
		static_assert(std::is_signed<value_type>::value, "index type must be signed");
		
		std::vector<value_type> inverse(last - first);
		auto i = static_cast<value_type>(offset);

		for (auto it = first; it != last; ++it, ++i)
		{
			value_type val = *it;
			inverse[std::abs(val) - offset] = val >= 0 ? i : -1;
		}

		std::copy(inverse.begin(), inverse.end(), first);
	}
	
	/// relloc map describes where elements were moved while removing elements.
	/// it's index array, where index itself - old index, and element - new_index: arr[old_index] => new_index,
	/// it is what view_qtbase::change_indexes expects with offset 0
	/// 
	/// [removed_first; removed_last) index range of removed elements, for example:
	/// [0, 5, 7] elements by indexes 0, 5, 7 were removed as if by std::remove_if algorithm.
	template <class Iterator>
	auto build_relloc_map(Iterator removed_first, Iterator removed_last, std::size_t store_size)
		-> std::vector<typename std::iterator_traits<Iterator>::value_type>
	{
		typedef typename std::iterator_traits<Iterator>::value_type value_type;
		static_assert(std::is_signed<value_type>::value, "index type must be signed");
		
		std::vector<value_type> index_array(store_size);
		value_type v1 = 0, v2 = static_cast<value_type>(store_size);
		value_type val = 0;

		for (; removed_first != removed_last; ++removed_first)
		{
			v2 = *removed_first;

			auto first = index_array.begin() + v1;
			auto last = index_array.begin() + v2;
			std::iota(first, last, val);
			val += static_cast<value_type>(last - first);

			*last = -1;
			v1 = ++v2;
		}

		auto first = index_array.begin() + v2;
		auto last = index_array.end();
		std::iota(first, last, val);

		return index_array;
	}

	/// removes elements from [first, last) by indexes given in [ifirst, ilast)
	template <class RandomAccessIterator, class Iterator>
	RandomAccessIterator remove_indexes(RandomAccessIterator first, RandomAccessIterator last,
	                                    Iterator ifirst, Iterator ilast)
	{
		if (ifirst == ilast) return last;

		auto out = first + *ifirst;
		auto it = out + 1;
		auto next = last;

		for (++ifirst; ifirst != ilast; ++ifirst)
		{
			next = first + *ifirst;
			out = std::move(it, next, out);
			it = next + 1;
		}

		return std::move(it, last, out);
	}


	template <class RandomAccessIterator>
	void emit_changed(AbstractItemModel * model, RandomAccessIterator first, RandomAccessIterator last)
	{
		if (first == last) return;

		int ncols = model->columnCount(AbstractItemModel::invalid_index);
		for (; first != last; ++first)
		{
			// lower index on top, higher on bottom
			int top, bottom;
			top = bottom = *first;

			// try to find the sequences with step of 1, for example: ..., 4, 5, 6, ...
			for (++first; first != last and *first - bottom == 1; ++first, ++bottom)
				continue;

			--first;

			auto top_left = model->index(top, 0, AbstractItemModel::invalid_index);
			auto bottom_right = model->index(bottom, ncols - 1, AbstractItemModel::invalid_index);
			model->dataChanged(top_left, bottom_right, AbstractItemModel::all_roles);
		}
	}

	template <class RandomAccessIterator>
	void change_indexes(AbstractItemModel * model, RandomAccessIterator first, RandomAccessIterator last, int offset)
	{
		auto size = last - first;
		auto list = model->persistentIndexList();

		for (const auto & idx : list)
		{
			if (!idx.isValid()) continue;

			auto row = idx.row();
			auto col = idx.column();

			if (row < offset) continue;

			assert(row < size); (void)size;
			auto newIdx = model->index(first[row - offset], col);
			model->changePersistentIndex(idx, newIdx);
		}
	}
}
