#pragma once
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOption>

namespace QtTools {
namespace Delegates
{
	/// получает стиль либо из opt.widget->style()
	inline QStyle * AccquireStyle(const QStyleOptionViewItem & opt)
	{
		return opt.widget->style();
	}

	/// возвращает color group по QStyleOption
	inline QPalette::ColorGroup ColorGroup(const QStyleOption & opt) noexcept
	{
		return not (opt.state & QStyle::State_Enabled) ? QPalette::Disabled :
		       not (opt.state & QStyle::State_Active)  ? QPalette::Inactive
		                                               : QPalette::Normal;
	}
}}
