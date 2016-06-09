#pragma once
#include <QtCore/Qt>
#include <QtCore/QSize>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE
class QHeaderView;
class QTableView;
class QLayout;
QT_END_NAMESPACE

namespace QtTools
{
	/// находит предка с типом Type
	/// если такого предка нет - вернет nullptr
	/// например, может быть полезно для нахождения QMdiArea
	template <class Type>
	Type * FindAncestor(QWidget * widget)
	{
		while (widget)
		{
			if (auto * w = qobject_cast<Type *>(widget))
				return w;

			widget = widget->parentWidget();
		}

		return nullptr;
	}

	/// находит предка с типом Type
	/// если такого предка нет - вернет nullptr
	/// например, может быть полезно для нахождения QMdiArea
	template <class Type>
	inline const Type * FindAncestor(const QWidget * widget)
	{
		return FindAncestor<Type>(const_cast<QWidget *>(widget));
	}

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

	/// вычисляет дпополнительно место занимаемое layout'ом.
	/// на данный момент это contentsMargins
	QSize LayoutAdditionalSize(const QLayout * layout);
}
