#include <QtTools/Delegates/SearchDelegate.hpp>
#include <QtTools/Delegates/StyledParts.hpp>
#include <QtGui/QPainter>

namespace QtTools {
namespace Delegates
{
	void FormatSearchText(const QString & text, const QString & filterWord, const QTextCharFormat & format, QList<QTextLayout::FormatRange> & formats)
	{
		if (filterWord.isEmpty()) return;

		int index = text.indexOf(filterWord, 0, Qt::CaseInsensitive);
		while (index >= 0) {
			int length = filterWord.length();
			QTextLayout::FormatRange fmt;
			fmt.start = index;
			fmt.length = length;
			fmt.format = format;
			formats.push_back(fmt);

			index = text.indexOf(filterWord, index + length, Qt::CaseInsensitive);
		}
	}

	static void ColorifyElidePoint(QString & elidedText, QList<QTextLayout::FormatRange> & formats)
	{
		const auto elideSymbol = QChar(0x2026);
		auto pos = elidedText.lastIndexOf(elideSymbol);

		for (auto & fmt : formats)
		{
			if (fmt.start > pos)
			{
				fmt.start = pos;
				fmt.length = 1;
			}
		}

		auto comparer = [](const QTextLayout::FormatRange & f1, const QTextLayout::FormatRange & f2)
		{
			return f1.start == f2.start && f1.length == f2.length;
		};

		auto end = std::unique(formats.begin(), formats.end(), comparer);
		formats.erase(end, formats.end());
	}

	void DrawSearchFormatedText(QPainter * painter, QRect textRect, const QStyleOptionViewItem & opt,
	                            const QList<QTextLayout::FormatRange> & selectionFormats)
	{
		using namespace QtTools::Delegates;
		using namespace QtTools::Delegates::TextLayout;
		PreparePainter(painter, textRect, opt);

		// could be interesting: qcommonstyle.cpp:861  qt 5.3 (viewItemDrawText)
		auto * style = AccquireStyle(opt);
		auto & text = opt.text;

		RemoveTextMargin(style, textRect);
		auto textOption = PrepareTextOption(opt);

		QTextLayout textLayout(text, opt.font);
		textLayout.setTextOption(textOption);
		textLayout.setAdditionalFormats(selectionFormats);
		textLayout.setCacheEnabled(true);

		int elideIdx = DoLayout(textLayout, textRect);
		bool needsElide = elideIdx != textLayout.lineCount();

		auto totalRect = BoundingRect(textLayout, elideIdx).toAlignedRect();
		auto totalHeight = totalRect.height();
		auto drawRect = AlignedRect(style, opt, {textRect.width(), totalHeight}, textRect);
		DrawLayout(painter, drawRect, textLayout, elideIdx);

		if (needsElide)
		{
			auto line = textLayout.lineAt(elideIdx);
			/// same as ElideLineRect(textLayout, elideIdx), but inplace
			drawRect.adjust(0, drawRect.height() - line.height(), 0, 0);

			int elidePoint = line.textStart();
			QString elidedText = text.mid(elidePoint);
			auto fontMetrics = painter->fontMetrics();   // painter is already prepared
			elidedText = ElideText(fontMetrics, elidedText, opt.textElideMode, drawRect.width());

			auto elidedFormats = ElideFormats(selectionFormats, elidePoint);
			ColorifyElidePoint(elidedText, elidedFormats);

			QTextLayout elideLayout {elidedText, opt.font};
			elideLayout.setTextOption(textOption);
			elideLayout.setAdditionalFormats(elidedFormats);
			elideLayout.setCacheEnabled(true);

			DoLayout(elideLayout, drawRect);
			// обманываем, нам нужно что бы он нарисовал одну единственную линию
			DrawLayout(painter, drawRect, elideLayout, 1);
		}
	}

	void SearchDelegate::FormatText(const QString & text, QList<QTextLayout::FormatRange> & formats) const
	{
		FormatSearchText(text, m_searchText, m_format, formats);
	}

	void SearchDelegate::DrawText(QPainter * painter, QStyleOptionViewItem & opt) const
	{
		QList<QTextLayout::FormatRange> formats;
		FormatText(opt.text, formats);
		DrawSearchFormatedText(painter, TextSubrect(opt), opt, formats);
	}
}}

