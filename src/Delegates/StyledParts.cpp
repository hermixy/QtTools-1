#include <QtTools/Delegates/StyledParts.hpp>
#include <QtCore/QString>
#include <QtGui/QPainter>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QTableView>

namespace QtTools {
namespace Delegates
{
#ifdef Q_OS_WIN
	static const std::type_info * GetKnownStyleType(const char * name)
	{
		auto keys = QStyleFactory::keys();
		int idx = keys.indexOf(name);
		if (idx < 0) return nullptr;

		auto * style = QStyleFactory::create(keys[idx]);
		if (!style) return nullptr;
		return &typeid(*style);
	}

	static const std::type_info * windowsVistaStyleTypeId = GetKnownStyleType("WindowsVista");

	static bool IsWindowsVistaStyle(QStyle * style)
	{
		return style && windowsVistaStyleTypeId && typeid(*style) == *windowsVistaStyleTypeId;
	}
#endif

	void FixStyleOptionViewItem(QStyleOptionViewItem & opt)
	{
#ifdef Q_OS_WIN

		// taken from qwindowsvistastyle.cpp: 1443 qt 5.3
		if (!IsWindowsVistaStyle(AccquireStyle(opt)))
			return;

		const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(opt.widget);
		bool newStyle = true;

		if (qobject_cast<const QTableView *>(opt.widget))
			newStyle = false;

		if (newStyle && view) {
			/*
			// We cannot currently get the correct selection color for "explorer style" views
			COLORREF cref = 0;
			XPThemeData theme(d->treeViewHelper(), 0, QLatin1String("LISTVIEW"), 0, 0);
			unsigned int res = pGetThemeColor(theme.handle(), LVP_LISTITEM, LISS_SELECTED, TMT_TEXTCOLOR, &cref);
			QColor textColor(GetRValue(cref), GetGValue(cref), GetBValue(cref));
			*/
			opt.palette.setColor(QPalette::All, QPalette::HighlightedText,
			                     opt.palette.color(QPalette::Active, QPalette::Text));
			// Note that setting a saturated color here results in ugly XOR colors in the focus rect
			opt.palette.setColor(QPalette::All, QPalette::Highlight, opt.palette.base().color().darker(108));
			
			// We hide the  focusrect in singleselection as it is not required
			bool nofocus = view->selectionMode() == QAbstractItemView::SingleSelection
				&& !(opt.state & QStyle::State_KeyboardFocusChange);
			if (nofocus)
				opt.state &= ~QStyle::State_HasFocus;
		}

#endif // Q_OS_WIN
	}

	void DrawBackground(QPainter * painter, const QStyleOptionViewItem & opt)
	{
		// code taken from: qcommonstyle.cpp:2156 qt 5.3
		auto * style = AccquireStyle(opt);
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
	}

	void DrawCheckmark(QPainter * painter, const QRect & checkRect, const QStyleOptionViewItem & opt)
	{
		// code taken from: qcommonstyle.cpp:2159 qt 5.3
		QStyleOptionViewItem option = opt;
		option.rect = checkRect;
		option.state = option.state & ~QStyle::State_HasFocus;

		switch (opt.checkState) {
		case Qt::Unchecked:
			option.state |= QStyle::State_Off;
			break;
		case Qt::PartiallyChecked:
			option.state |= QStyle::State_NoChange;
			break;
		case Qt::Checked:
			option.state |= QStyle::State_On;
			break;
		}

		auto * style = AccquireStyle(opt);
		style->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &option, painter, opt.widget);
	}

	void DrawDecoration(QPainter * painter, const QRect & decorationRect, const QStyleOptionViewItem & opt)
	{
		// code taken from: qcommonstyle.cpp:2179 qt 5.3
		QIcon::Mode mode = QIcon::Normal;
		if (!(opt.state & QStyle::State_Enabled))
			mode = QIcon::Disabled;
		else if (opt.state & QStyle::State_Selected)
			mode = QIcon::Selected;

		QIcon::State state = opt.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
		opt.icon.paint(painter, decorationRect, opt.decorationAlignment, mode, state);
	}

	void DrawFocusFrame(QPainter * painter, const QRect & focusRect, const QStyleOptionViewItem & opt)
	{
		// code taken from: qcommonstyle.cpp:2209 qt 5.3
		auto * style = AccquireStyle(opt);

		QStyleOptionFocusRect focusOpt;
		focusOpt.QStyleOption::operator =(opt);
		focusOpt.rect = focusRect;

		focusOpt.state |= QStyle::State_KeyboardFocusChange;
		focusOpt.state |= QStyle::State_Item;

		auto cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
		auto cr = opt.state & QStyle::State_Selected ? QPalette::Highlight : QPalette::Window;
		focusOpt.backgroundColor = opt.palette.color(cg, cr);

		style->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpt, painter, opt.widget);
	}
}}
