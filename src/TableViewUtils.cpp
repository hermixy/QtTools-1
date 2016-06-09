#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableView>
#include <QtWidgets/QLayout>
#include <QtWidgets/QScrollBar>
#include <QtTools/TableViewUtils.hpp>

namespace QtTools
{
	int CalculateDefaultRowHeight(const QTableView * view)
	{
		return 21;
	}

	int HeaderWidth(const QHeaderView * header)
	{
		int count = header->count();
		int width = 0;
		for (int i = 0; i < count; ++i)
			width += header->sectionSize(i);

		return width;
	}

	int TableWidthHint(const QTableView * view, bool withScrollBar)
	{
		// смотри также реализацию TableSizeHint, там есть важные пояснения
		auto * model = view->model();
		Q_ASSERT(model);
		int count = model->columnCount();
		int width = 0;
		
		for (int i = 0; i < count; ++i)
			width += view->columnWidth(i);

		width += view->frameWidth() * 2;
		auto * vhdr = view->verticalHeader();
		if (not vhdr->isHidden())
			width += vhdr->width();

		if (withScrollBar)
		{
			auto * style = view->style();
			width += style->pixelMetric(QStyle::PM_ScrollBarExtent);
		}

		return width;
	}

	int TableHeightHint(const QTableView * view, bool withScrollBar)
	{
		// смотри также реализацию TableSizeHint, там есть важные пояснения
		auto * model = view->model();
		Q_ASSERT(model);
		int count = model->rowCount();
		int height = 0;

		for (int i = 0; i < count; ++i)
			height += view->rowHeight(i);

		height += view->frameWidth() * 2;
		auto * hhdr = view->horizontalHeader();
		if (not hhdr->isHidden())
			height += hhdr->height();

		if (withScrollBar)
		{
			auto * style = view->style();
			height += style->pixelMetric(QStyle::PM_ScrollBarExtent);
		}

		return height;
	}

	/// вычисляет отображение scrollbar'а по политике размеру и максимальному размеру.
	/// например: ScrollBarVisible(view->horizontalScrollBarPolicy(), totalColumnWidth, maximumWidth)
	static bool ScrollBarVisible(Qt::ScrollBarPolicy policy, int size, int maxSize)
	{
		switch (policy)
		{
			case Qt::ScrollBarAsNeeded:
				return size > maxSize;
			case Qt::ScrollBarAlwaysOff:
				return false;
			case Qt::ScrollBarAlwaysOn:
				return true;

			default: Q_UNREACHABLE();
		}
	}

	QSize TableSizeHint(const QTableView * view, const QSize & minimum, const QSize & maximum,
	                    bool forceSB /* = false */)
	{
		const auto * model = view->model();
		Q_ASSERT(model);
		const int rc = model->rowCount();
		const int cc = model->columnCount();

		int width = 0, height = 0;
		const int minWidth = minimum.width();
		const int minHeight = minimum.height();
		const int maxWidth = std::max(minWidth, maximum.width());
		const int maxHeight = std::max(minHeight, maximum.height());

		auto * hhdr = view->horizontalHeader();
		auto * vhdr = view->verticalHeader();

		/// доп размеры
		const int frameWidth = view->frameWidth() * 2;
		
		/// isVisible возвращает фактическую видимость, как результат совокупности всех факторов(видимость родителя, прочее)
		/// isHidden всегда возвращает явную скрытость данного виджета.
		/// Именно, виджет может быть одновременно не скрытым и не видимым.
		/// на момент вызова данной функции, например, из sizeHint(),
		/// мы можем находится в промежуточном состоянии изменения видимости иерархии виджетов,
		/// мы должны использовать isHidden.
		const int hhdrHeight = not hhdr->isHidden() ? hhdr->sizeHint().height() : 0;
		const int vhdrWidth = not vhdr->isHidden() ? vhdr->sizeHint().width() : 0;

		for (int i = 0; i < cc && width < maxWidth; ++i)
			width += view->columnWidth(i);

		for (int i = 0; i < rc && height < maxHeight; ++i)
			height += view->rowHeight(i);

		// columnWidth похоже учитывает grid size.
		/*
		// похоже что на текущий момент (Qt 5.4) gridSize для QTableView всегда 1
		// смотри: qtableview.cpp:1361
		// const int gridSize = view->showGrid() ? 1 : 0;
		// width += count * gridSize;
		*/

		bool horizontalScrollBarVisisble = forceSB ||
			ScrollBarVisible(view->horizontalScrollBarPolicy(), width, maxWidth - frameWidth - vhdrWidth);
		bool vertiacalScroolBarVisible = forceSB ||
			ScrollBarVisible(view->verticalScrollBarPolicy(), height, maxHeight - frameWidth - hhdrHeight);

		const auto * style = view->style();
		const auto sbextent = style->pixelMetric(QStyle::PM_ScrollBarExtent);

		if (horizontalScrollBarVisisble)
			height += sbextent;

		if (vertiacalScroolBarVisible)
			width += sbextent;

		width += frameWidth + vhdrWidth;
		height += frameWidth + hhdrHeight;

		return {
			qBound(minWidth, width, maxWidth), 
			qBound(minHeight, height, maxHeight)
		};

	}

	QSize LayoutAdditionalSize(const QLayout * layout)
	{
		auto m = layout->contentsMargins();
		return {
			m.left() + m.right(),
			m.top() + m.bottom()
		};
	}
}
