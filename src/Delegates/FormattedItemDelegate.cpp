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
		auto rect = TextSubrect(opt);
		RemoveTextMargin(opt, rect);

		PreparePainter(painter, opt);
		DrawEditingFrame(painter, rect, opt);
		DrawFormattedText(painter, opt.text, rect, opt, fmts);
	}

	QVector<QTextLayout::FormatRange> FormattedItemDelegate::Format(const QStyleOptionViewItem & initedOption, const QModelIndex & idx) const
	{
		if (m_formatter)
			return m_formatter(initedOption, idx);
		else
			return {};
	}
}}
