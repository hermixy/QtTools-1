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
}}
