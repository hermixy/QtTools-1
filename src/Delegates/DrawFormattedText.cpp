#include <QtTools/Delegates/DrawFormattedText.hpp>
#include <QtGui/QPainter>
#include <algorithm> // for replace_if

namespace QtTools {
namespace Delegates
{
	namespace TextLayout
	{
		QList<QTextLayout::FormatRange> ElideFormats(const QList<QTextLayout::FormatRange> & formats, int elidePoint)
		{
			QList<QTextLayout::FormatRange> slicedFormats;
			for (auto & format : formats) {
				if (format.start + format.length > elidePoint)
				{
					auto fmt = format;
					if (format.start >= elidePoint)
						fmt.start -= elidePoint;
					else
						fmt.start = elidePoint;
					slicedFormats.push_back(fmt);
				}
			}
			return slicedFormats;
		}

		QString ElideText(const QFontMetrics & fm, QString lineToElide, Qt::TextElideMode mode, int width)
		{
			// заменяем все пробельные символы на пробелы, а именно \r, \n, \u2028, прочие
			auto finder = [](QChar ch) { return ch.isSpace(); };
			std::replace_if(lineToElide.begin(), lineToElide.end(), finder, ' ');
			return fm.elidedText(lineToElide, mode, width);
		}
	
		int DoLayout(QTextLayout & layout, const QRect & textRect)
		{
			int width = textRect.width();
			int height = textRect.height();
			qreal cury = 0;
			int elideIndex = 0;

			layout.beginLayout();
			for (;;) {
				auto line = layout.createLine();
				if (!line.isValid()) {
					break;
				}
				
				line.setLineWidth(width);
				line.setPosition({0, cury});
				cury += line.height();

				// последняя строка вылезла за границу по высоте
				if (cury > height) {
					elideIndex = qMax(0, elideIndex - 1);
					break;
				}

				// очень длинное слово, не вмещается на линию
				if (line.naturalTextWidth() > width) {
					break;
				}

				++elideIndex;
			};
			layout.endLayout();

			return elideIndex;
		}

		QRectF BoundingRect(QTextLayout & layout, int elideIndex)
		{
			auto lc = layout.lineCount();
			int diff = lc - elideIndex;
			switch (diff)
			{
				case 0: // не было усечения
				case 1: // усечение по последней строке
					return layout.boundingRect();
				default: // по факту усечение по предпоследней строке
				{
					auto br = layout.boundingRect();
					auto line = layout.lineAt(lc - 2);
					br.adjust(0, 0, 0, -line.height());
					return br;

					//QRectF br = layout.boundingRect();
					//for (int backStep = diff - 1; backStep > 0; --backStep)
					//{
					//	auto line = layout.lineAt(lc - backStep);
					//	br.adjust(0, 0, 0, -line.height());
					//}

					//return br;
				}
			}
		}

		QRect ElideLineRect(QTextLayout & layout, int elideIndex, const QRect & boundingRectangle)
		{
			auto line = layout.lineAt(elideIndex);
			return boundingRectangle.adjusted(0, boundingRectangle.height() - line.height(), 0, 0);
		}

		void DrawLayout(QPainter * painter, const QRect & drawRect, const QTextLayout & layout, int elideIndex)
		{
			auto drawPos = drawRect.topLeft();
			for (int i = 0; i < elideIndex; ++i)
			{
				auto line = layout.lineAt(i);
				line.draw(painter, drawPos);
			}
		}

		QTextOption PrepareTextOption(const QStyleOptionViewItem & opt)
		{
			QTextOption textOption;

			const Qt::Alignment align = QStyle::visualAlignment(opt.direction, opt.displayAlignment);
			textOption.setWrapMode(opt.features & QStyleOptionViewItem::WrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
			textOption.setTextDirection(opt.direction);
			textOption.setAlignment(align);

			return textOption;
		}
	} // namespace TextLayout

	static void DoDrawText(QPainter * painter, QRect textRect, const QStyleOptionViewItem & opt,
	                       const QList<QTextLayout::FormatRange> & additionalFormats)
	{
		using namespace TextLayout;
		// could be interesting: qcommonstyle.cpp:861  qt 5.3 (viewItemDrawText)
		auto * style = AccquireStyle(opt);
		auto & text = opt.text;

		RemoveTextMargin(style, textRect);
		auto textOption = PrepareTextOption(opt);

		QTextLayout textLayout(text, opt.font, painter->device());
		textLayout.setTextOption(textOption);
		textLayout.setAdditionalFormats(additionalFormats);
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

			QTextLayout elideLayout(elidedText, opt.font, painter->device());
			elideLayout.setTextOption(textOption);
			elideLayout.setAdditionalFormats(ElideFormats(additionalFormats, elidePoint));
			elideLayout.setCacheEnabled(true);
			DoLayout(elideLayout, drawRect);
			// обманываем, нам нужно что бы он нарисовал одну единственную линию
			DrawLayout(painter, drawRect, elideLayout, 1);
		}
	}

	void PreparePainter(QPainter * painter, const QRect & textRect, const QStyleOptionViewItem & opt)
	{
		// code taken from: qcommonstyle.cpp:2187 qt 5.3 
		// and reworked
		auto & palette = opt.palette;

		QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
		if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
			cg = QPalette::Inactive;

		painter->setFont(opt.font);
		painter->setPen(palette.color(cg, opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
		//painter->setBackground(palette.color(cg, opt.state & QStyle::State_Selected ? QPalette::Highlight : QPalette::Window));

		if (opt.state & QStyle::State_Editing) {
			painter->setPen(palette.color(cg, QPalette::Text));
			painter->drawRect(textRect.adjusted(0, 0, -1, -1));
		}
	}

	void DrawFormattedText(QPainter * painter, const QRect & textRect, const QStyleOptionViewItem & opt,
	                       const QList<QTextLayout::FormatRange> & additionalFormats /* = */ )
	{
		PreparePainter(painter, textRect, opt);
		DoDrawText(painter, textRect, opt, additionalFormats);
	}
}}