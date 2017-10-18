#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOption>
#include <QtWidgets/QStyledItemDelegate>
#include <QtTools/Delegates/AccqireStyle.hpp>

namespace QtTools {
namespace Delegates
{
	/// "подправляет" opt. Необходимость данного метода обусловлена реализацией
	/// QStyle::drawControl(CE_ItemViewItem, ...)
	/// qwindowsvistastyle.cpp:1443 qt 5.3
	/// 
	/// на момент qt 5.3 только QWindowsVistaStyle ведет себя так,
	/// вообще не плохо бы придумать более общее решение, если возможно
	/// 
	/// QWindowsVistaStyle делает правки палитры что бы выглядеть корректно для QListView, QTreeView
	/// перед тем как передать opt для дальнейшего рисования в QCommonStyle, мы должны так же, увы
	void FixStyleOptionViewItem(QStyleOptionViewItem & opt);

	/************************************************************************/
	/*             HasMethods                                               */
	/************************************************************************/
	inline bool HasCheckmark(const QStyleOptionViewItem & opt)
	{
		return opt.features & QStyleOptionViewItem::HasCheckIndicator;
	}

	inline bool HasDecoration(const QStyleOptionViewItem & opt)
	{
		return opt.features & QStyleOptionViewItem::HasDecoration;
	}

	inline bool HasText(const QStyleOptionViewItem & opt)
	{
		return opt.features & QStyleOptionViewItem::HasDisplay;
	}

	inline bool HasFocusFrame(const QStyleOptionViewItem & opt)
	{
		return opt.state & QStyle::State_HasFocus;
	}

	/************************************************************************/
	/*            Subrect methods                                           */
	/************************************************************************/
	/// получает подобласть checkmark
	/// @Param opt как есть из метода paint, с обработкой по желанию
	inline QRect CheckmarkSubrect(const QStyleOptionViewItem & opt)
	{
		auto * style = AccquireStyle(opt);
		return style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, opt.widget);
	}

	/// получает подобласть decoration
	/// @Param opt как есть из метода paint, с обработкой по желанию
	inline QRect DecorationSubrect(const QStyleOptionViewItem & opt)
	{
		auto * style = AccquireStyle(opt);
		return style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
	}

	/// получает подобласть text
	/// @Param opt как есть из метода paint, с обработкой по желанию
	inline QRect TextSubrect(const QStyleOptionViewItem & opt)
	{
		auto * style = AccquireStyle(opt);
		return style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
	}

	/// получает область рамки фокуса
	/// @Param opt как есть из метода paint, с обработкой по желанию
	inline QRect FocusFrameSubrect(const QStyleOptionViewItem & opt)
	{
		auto * style = AccquireStyle(opt);
		return style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt, opt.widget);
	}


	/// возвращает horizontal text margin.
	/// Qt вычисляет область текста с отступами справа и слева
	/// смотри: qcommonstyle.cpp:861  qt 5.3 (viewItemDrawText)
	///         viewItemSize.cpp:846  qt 5.3 (viewItemDrawText)
	inline int TextMargin(QStyle * style)
	{
		return style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
	}

	/// возвращает horizontal text margin.
	/// Qt вычисляет область текста с отступами справа и слева
	/// смотри: qcommonstyle.cpp:861  qt 5.3 (viewItemDrawText)
	///         viewItemSize.cpp:846  qt 5.3 (viewItemDrawText)
	inline int TextMargin(const QStyleOptionViewItem & opt)
	{
		auto * style = AccquireStyle(opt);
		return TextMargin(opt);
	}

	/// убирает text margin из области полученной с помощью QtTools::Delegates::TextSubrect
	/// смотри: qcommonstyle.cpp:861  qt 5.3 (viewItemDrawText)
	inline void RemoveTextMargin(QStyle * style, QRect & textRect)
	{
		int padding = TextMargin(style);
		textRect.adjust(padding, 0, -padding, 0);
	}

	/// убирает text margin из области полученной с помощью QtTools::Delegates::TextSubrect
	/// смотри: qcommonstyle.cpp:861  qt 5.3 (viewItemDrawText)
	inline void RemoveTextMargin(const QStyleOptionViewItem & opt, QRect & textRect)
	{
		auto * style = AccquireStyle(opt);
		return RemoveTextMargin(style, textRect);
	}

	/************************************************************************/
	/*            DrawMethods                                               */
	/************************************************************************/
	/// рисует background
	/// @Param opt как есть из метода paint, с обработкой по желанию
	void DrawBackground(QPainter * painter, const QStyleOptionViewItem & opt);
	/// рисует checkmark
	/// @Param opt как есть из метода paint, с обработкой по желанию
	/// @Param checkRect область в которой должен быть нарисован checkMark
	void DrawCheckmark(QPainter * painter, const QRect & checkRect, const QStyleOptionViewItem & opt);
	/// рисует decoration
	/// @Param opt как есть из метода paint, с обработкой по желанию
	/// @Param decorationRecet область в которой должен быть нарисован decaration
	void DrawDecoration(QPainter * painter, const QRect & decorationRect, const QStyleOptionViewItem & opt);
	/// рисует рамку фокуса
	/// @Param opt как есть из метода paint, с обработкой по желанию
	void DrawFocusFrame(QPainter * painter, const QRect & focusRect, const QStyleOptionViewItem & opt);
}}
