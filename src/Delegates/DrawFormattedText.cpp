#include <cassert>
#include <algorithm> // for replace_if

#include <QtGui/QPainter>
#include <QtTools/Delegates/DrawFormattedText.hpp>

namespace QtTools {
namespace Delegates
{
	namespace TextLayout
	{
		QVector<QTextLayout::FormatRange> ElideFormats(const QVector<QTextLayout::FormatRange> & formats, int elidePoint)
		{
			QVector<QTextLayout::FormatRange> slicedFormats;
			for (auto & format : formats)
			{
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
	
		int DoLayout(QTextLayout & layout, const QRectF & textRect)
		{
			qreal width = textRect.width();
			qreal height = textRect.height();
			qreal cury = 0;
			int elideIndex = 0;

			layout.beginLayout();
			for (;;)
			{
				auto line = layout.createLine();
				if (!line.isValid())
					break;
				
				line.setLineWidth(width);
				line.setPosition({0, cury});
				cury += line.height();

				// последняя строка вылезла за границу по высоте
				if (cury > height)
				{
					elideIndex = qMax(0, elideIndex - 1);
					break;
				}

				// очень длинное слово, не вмещается на линию
				if (line.naturalTextWidth() > width)
					break;

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

		QRectF NaturalBoundingRect(QTextLayout & layout, int elideIndex)
		{
			int lc = layout.lineCount();
			assert(elideIndex <= lc);

			QRectF rect;
			lc = std::min(elideIndex + 1, lc);

			for (int i = 0; i < lc; ++i)
			{
				QTextLine line = layout.lineAt(i);
				rect |= line.naturalTextRect();
			}

			return rect;
		}

		QRectF ElideLineRect(QTextLayout & layout, int elideIndex, const QRectF & boundingRectangle)
		{
			auto line = layout.lineAt(elideIndex);
			return boundingRectangle.adjusted(0, boundingRectangle.height() - line.height(), 0, 0);
		}

		void DrawLayout(QPainter * painter, const QRectF & drawRect, const QTextLayout & layout, int elideIndex)
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

	void PreparePainter(QPainter * painter, const QStyleOptionViewItem & opt)
	{
		// code taken from: qcommonstyle.cpp:2187 qt 5.3
		// and reworked
		auto & palette = opt.palette;
		QPalette::ColorGroup cg = ColorGroup(opt);

		painter->setFont(opt.font);
		painter->setPen(palette.color(cg, opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
		painter->setBackground(palette.color(cg, opt.state & QStyle::State_Selected ? QPalette::Highlight : QPalette::Window));
	}

	void DrawEditingFrame(QPainter * painter, const QRect & textRect, const QStyleOptionViewItem & opt)
	{
		if (opt.state & QStyle::State_Editing)
		{
			painter->setPen(opt.palette.color(ColorGroup(opt), QPalette::Text));
			painter->drawRect(textRect.adjusted(0, 0, -1, -1));
		}

	}

	void DrawFormattedText(QPainter * painter, const QString & text, const QRect & textRect, const QStyleOptionViewItem & opt,
	                       const QVector<QTextLayout::FormatRange> & additionalFormats)
	{
		using namespace TextLayout;
		// could be interesting: qcommonstyle.cpp:861  qt 5.3 (viewItemDrawText)
		auto * style = AccquireStyle(opt);
		auto textOption = PrepareTextOption(opt);

		QTextLayout textLayout(text, opt.font, painter->device());
		textLayout.setTextOption(textOption);
		textLayout.setFormats(additionalFormats);
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
			elideLayout.setFormats(ElideFormats(additionalFormats, elidePoint));
			elideLayout.setCacheEnabled(true);
			DoLayout(elideLayout, drawRect);
			// обманываем, нам нужно что бы он нарисовал одну единственную линию
			DrawLayout(painter, drawRect, elideLayout, 1);
		}
	}
}}
