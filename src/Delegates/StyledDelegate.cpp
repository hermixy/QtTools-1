#include <QtTools/Delegates/StyledDelegate.hpp>
#include <QtTools/Delegates/StyledParts.hpp>
#include <QtTools/Delegates/DrawFormattedText.hpp>
#include <QtGui/QPainter>

namespace QtTools {
namespace Delegates
{
	void StyledDelegate::InitStyle(QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		initStyleOption(&option, index);
	}

	void StyledDelegate::DrawBackground(QPainter * painter, QStyleOptionViewItem & option) const
	{
		Delegates::DrawBackground(painter, option);
	}

	void StyledDelegate::DrawCheckmark(QPainter * painter, QStyleOptionViewItem & option) const
	{
		Delegates::DrawCheckmark(painter, Delegates::CheckmarkSubrect(option), option);
	}

	void StyledDelegate::DrawDecoration(QPainter * painter, QStyleOptionViewItem & option) const
	{
		Delegates::DrawDecoration(painter, Delegates::DecorationSubrect(option), option);
	}

	void StyledDelegate::DrawText(QPainter * painter, QStyleOptionViewItem & option) const
	{
		using namespace Delegates;
		
		auto rect = TextSubrect(option);
		RemoveTextMargin(option, rect);

		PreparePainter(painter, option);
		DrawEditingFrame(painter, rect, option);
		DrawFormattedText(painter, option.text, rect, option);
	}

	void StyledDelegate::DrawFocusFrame(QPainter * painter, QStyleOptionViewItem & option) const
	{
		Delegates::DrawFocusFrame(painter, Delegates::FocusFrameSubrect(option), option);
	}

	void StyledDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		auto opt = option;
		InitStyle(opt, index);
		FixStyleOptionViewItem(opt);

		painter->save();
		// painter->setClipRect(opt.rect);
		painter->translate(opt.rect.topLeft());
		opt.rect.moveTopLeft({0, 0});

		DrawBackground(painter, opt);
		if (HasCheckmark(opt))  DrawCheckmark(painter, opt);
		if (HasDecoration(opt)) DrawDecoration(painter, opt);
		if (HasText(opt))       DrawText(painter, opt);
		if (HasFocusFrame(opt)) DrawFocusFrame(painter, opt);

		painter->restore();
	}
}}
