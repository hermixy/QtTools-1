#pragma once
#include <QtCore/Qt>
#include <QtCore/QSize>

QT_BEGIN_NAMESPACE
class QWidget;
class QHeaderView;
class QTableView;
class QTreeView;
class QLayout;
QT_END_NAMESPACE

namespace QtTools
{
	/// вычисляет высоту строки для заданного view,
	/// из view берется шрифт, и другие необходимые параметры.
	/// при этом модель не используется.
	/// 
	/// по-умолчанию высота равна 30 пикселей, что многовато.
	/// NOTE: текущая реализация просто возвращает 21,
	///       нужно реализовать корректное вычисление
	int CalculateDefaultRowHeight(const QTableView * view);

	/// вычисляет размер header по его полям
	/// по факту суммирует размер секций
	int HeaderWidth(const QHeaderView * header);

	/// вычисляет ширину tableView:
	/// * + суммирует ширину колонок
	/// * + frameWidth
	/// * + verticalHeader()->width() if visible
	/// * + verticalScrollBar width if withScrollBar
	int TableWidthHint(const QTableView * view, bool withScrollBar);

	/// вычисляет высоту tableView:
	/// * + суммирует высоту строк колонок
	/// * + frameWidth
	/// * + horizontalHeader()->width() if visible
	/// * + horizontalScrollBar width if withScrollBar
	int TableHeightHint(const QTableView * view, bool withScrollBar);

	/// вычисляет желаемые размеры tableView:
	/// но не превышает нижнего/верхнего пределов
	/// * + суммирует высоту/ширину строк колонок
	/// * + frameWidth
	/// * + h/v*header()->width() if visible
	/// * + h/v*scrollBar width if needed
	QSize TableSizeHint(const QTableView * view, const QSize & minimum, const QSize & maximum,
	                    bool forceSB = false);

	/// вычисляет дополнительное место занимаемое layout'ом.
	/// на данный момент это contentsMargins
	QSize LayoutAdditionalSize(const QLayout * layout);

	/// вычисляет и меняет размер колонок под содержимое
	void ResizeColumnsToContents(QTableView * tableView);
	void ResizeColumnsToContents(QTreeView * treeView);
}
