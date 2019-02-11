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

	/// вычисляет ширину QTableView/QListView/QTreeView:
	/// * + суммирует ширину колонок/element flow область для QListView
	/// * + frameWidth
	/// * + verticalHeader()->width() if applicable and visible
	/// * + verticalScrollBar width if withScrollBar
	int ItemViewWidthHint(const QTableView * view, bool withScrollBar);
	int ItemViewWidthHint(const QTreeView  * view, bool withScrollBar);

	/// вычисляет высоту QTableView/QListView/QTreeView:
	/// * + суммирует высоту строк колонок/element flow область для QListView/QTreeView
	/// * + frameWidth
	/// * + horizontalHeader()->width() if applicable and visible
	/// * + horizontalScrollBar width if withScrollBar
	int ItemViewHeightHint(const QTableView * view, bool withScrollBar);
	int ItemViewHeightHint(const QTreeView  * view, bool withScrollBar);

	/// вычисляет желаемые размеры QTableView/QListView/QTreeView:
	/// но не превышает нижнего/верхнего пределов
	/// * + суммирует высоту/ширину строк колонок/element flow областей
	/// * + frameWidth
	/// * + h/v*header()->width() if applicable and visible
	/// * + h/v*scrollBar width if needed
	QSize ItemViewSizeHint(const QTableView * view, const QSize & minimum, const QSize & maximum, bool forceSB = false);
	QSize ItemViewSizeHint(const QTreeView  * view, const QSize & minimum, const QSize & maximum, bool forceSB = false);

	/// вычисляет sizeHint для заданного виджета содержащего view на основе содержимого view
	QSize ItemViewSizeHint(const QWidget * us, const QTableView * view);
	QSize ItemViewSizeHint(const QWidget * us, const QTreeView  * view);

	/// вычисляет дополнительное место занимаемое layout'ом.
	/// на данный момент это contentsMargins
	QSize LayoutAdditionalSize(const QLayout * layout);

	/// вычисляет и меняет размер колонок под содержимое
	void ResizeColumnsToContents(QTableView * tableView);
	void ResizeColumnsToContents(QTreeView * treeView);
}
