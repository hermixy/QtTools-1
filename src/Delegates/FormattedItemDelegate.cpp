#include <QtTools/Delegates/FormattedItemDelegate.hpp>
#include <QtTools/Delegates/DrawFormattedText.hpp>
#include <QtTools/Delegates/StyledParts.hpp>
#include <QtWidgets/QApplication>
#include <QtGui/QPainter>

namespace QtTools {
namespace Delegates
{
	void FormattedItemDelegate::DrawText(QPainter * painter, QStyleOptionViewItem & opt) const
	{
		auto fmts = Format(opt, opt.index);
		DrawFormattedText(painter, TextSubrect(opt), opt, fmts);
	}

	QList<QTextLayout::FormatRange> 
		FormattedItemDelegate::Format(const QStyleOptionViewItem & initedOption, const QModelIndex & idx) const
	{
		if (m_formatter)
			return m_formatter(initedOption, idx);
		else
			return {};
	}
}}
