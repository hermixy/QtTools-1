#include <QtTools/StandardItemModel.hqt>
#include <iterator>
#include <algorithm>
#include <ext/algorithm.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/assert.hpp>

namespace QtTools
{
	namespace
	{
		/// ссылка на StandardItemModel Item по номеру строки,
		/// умеет обменивать строки, фактически адаптер для стандартных алгоритмов
		struct RowItemReference
		{
			StandardItemModel * owner;
			int row;
		};

		void swap(RowItemReference op1, RowItemReference op2)
		{
			BOOST_ASSERT(op1.owner == op2.owner);
			op1.owner->SwapRows(op1.row, op2.row);
		}

		struct RowIterator :
			public boost::iterator_facade<
				RowIterator,
				RowItemReference,
				std::random_access_iterator_tag,
				RowItemReference
			>
		{
			StandardItemModel * owner;
			int row;

			RowIterator(StandardItemModel * owner, int row) : owner(owner), row(row) {}

			RowItemReference dereference() const { return RowItemReference{ owner, row }; }
			bool equal(const RowIterator & other) const { return row == other.row; }
			void increment() { ++row; }
			void decrement() { --row; }
			void advance(std::ptrdiff_t step) { row += step; }
			std::ptrdiff_t distance_to(const RowIterator & other) const { return other.row - row; }
		};
	}

	void StandardItemModel::SwapRows(int row1, int row2)
	{
		for (int i = 0, n = columnCount(); i < n; ++i)
		{
			auto idx1 = index(row1, i);
			auto idx2 = index(row2, i);
			auto itd1 = itemData(idx1);
			auto itd2 = itemData(idx2);

			setItemData(idx1, itd2);
			setItemData(idx2, itd1);
		}
	}

	bool StandardItemModel::moveRows(const QModelIndex & sourceParent, int sourceRow, int count,
									 const QModelIndex & destinationParent, int destinationChild)
	{
		if (sourceParent != destinationParent)
			return false;

		bool allowed = beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
		if (!allowed) return false;

		RowIterator first {this, sourceRow};
		RowIterator last {this, sourceRow + count};
		RowIterator new_pos {this, destinationChild};
		ext::slide(first, last, new_pos);

		endMoveRows();
		return true;
	}

	/*
	 *just to remember reverse_iterator usage
	bool StandardItemModel::moveRows(const QModelIndex & sourceParent, int sourceRow, int count,
									 const QModelIndex & destinationParent, int destinationChild)
	{
		if (sourceParent != destinationParent)
			return false;

		bool allowed = beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
		if (!allowed) return false;

		// используем std::rotate, нужно перевести координаты в соотвествующие итераторы
		if (destinationChild < sourceRow) // move up: right rotate
		{
			// right rotate делается реверсными итераторами
			typedef std::reverse_iterator<RowIterator> Iterator;
			// let src = 5, count = 2, dest = 2
			//  rend should point at 1
			//  rbeg should point at 6
			//  rnewBeg should point at 4
			// ------------------------------------------
			//        rend      rnewBeg  rbeg   
			//  it(dst)|    it(src)|       | it(src+n)
			//        / \         / \     / \
			// | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
			// ------------------------------------------
			Iterator rbeg{RowIterator(this, sourceRow + count)};
			Iterator rnewBeg{RowIterator(this, sourceRow)};
			Iterator rend{RowIterator(this, destinationChild)};
		
			std::rotate(rbeg, rnewBeg, rend);
		}
		else // move down: left rotate 
		{
			// left rotate делается прямыми итераторами
			RowIterator beg{this, sourceRow};
			RowIterator newBeg{this, sourceRow + count}; // элемент который должен стать первым, это первый элемент, который не перемещаем
			RowIterator end{this, destinationChild};

			std::rotate(beg, newBeg, end);
		}

		endMoveRows();
		return true;
	}
	}*/
}
