#pragma once
#include <QtCore/QVector>
#include <QtGUi/QTextLayout>
#include <QtWidgets/QStyleOption>
#include <QtTools/Delegates/Utils.hpp>

namespace QtTools {
namespace Delegates
{
	/// вспомогательные функции для разметки/форматирования/рисования текста для ItemView делегатов
	namespace TextLayout
	{
		/// Что бы корректно отобразить текст c помощью QTextLayout, максимально приближенно к стандартному поведению Qt
		/// (Qt внутри так же использует QTextLayout, но не предоставляет публичных вспомогательных функций/классов, а так же содержит некоторые баги)
		/// нужно делать разметку в 2 этапа:
		///  * основной текст, который может поместится без усечения
		///  * оставшийся текст, который нужно усекать
		/// при этом нужно учитывать длинные слова.
		/// Как только мы встречаем длинное слово, целиком не помещающееся по ширине -
		/// это линию обрабатываем как последнюю с усечением
		/// так же нужно учитывать настройки выравнивания элементов


		/// срезает и возвращает форматы, которые находятся после elidePoint.
		/// индексы форматов корректируются по elidePoint, он считается новым началом
		/// полезно при форматировании elided текста
		QVector<QTextLayout::FormatRange> ElideFormats(const QVector<QTextLayout::FormatRange> & formats, int elidePoint);

		/// выполняет усечение текста. На данный момент усечение выполняется с помощью QFontMetrics::edlidedText,
		/// как следствие дополнительные форматы не учитываются, в дальнейшем нужно бы написать функцию с учетом
		/// @Param lineToElide линия которую нужно усечь, в линии так же все пробельные символы будут заменены на пробелы(0x20)
		/// @Param mode способ усечения
		/// @Param width необходимая ширина
		QString ElideText(const QFontMetrics & fm, QString lineToElide, Qt::TextElideMode mode, int width);


		/// Выполняет разметку основного текста,
		/// если находит одно из 2х условий усечения - прекращает работу:
		///   * если текущая строка вылазит по высоте - предыдущая должна усекаться
		///   * если текущая строка вылазит по ширине - текущая строка должна усекаться
		///     (например состоит из одного очень длинного слова, которое не влазит)
		/// 
		/// возвращает индекс усеченной строки (далее elidedIndex)
		/// если по факту усечение не было - возвращает lineCount
		/// assert(lineCount - elideIndex <= 2)
		/// 
		/// после выполнения данной функции для layout.boundingRectangle() (далее LBR) возможны 3 случая:
		/// switch(auto diff = layout.lineCount() - elidedIndex):
		///  0. Усечение не было обнаружено и прямоугольник содержит область корректной разметки
		///  1. Было обнаружено усечение по длине линии - прямоугольник содержит область вместе усеченной строкой
		///  2. Было обнаружено усечение по высоте области - при этом прямоугольник
		///     содержит область вместе с усеченной линией, и дополнительно содержит область следующей строки, после усеченной
		///  
		///  в случае 0 нужно просто нарисовать все строки в LBR.
		///  в случае 1 нужно нарисовать все строки до elidedIndex в области LBR, и потом отдельно elided строку в области elided строки
		///  в случае 2 нужно нарисовать все строки до elidedIndex в области LBR, из которой следует исключить дополнительные строки после elidedIndex
		///  
		///  Смотри также вспомогательные функции ниже
		int DoLayout(QTextLayout & layout, const QRectF & textRect);

		/// вычисляет bounding rectangle в layout после DoLayout(layout, ...),
		/// учитывая результат выполнения последнего(смотри описание DoLayout)
		QRectF BoundingRect(QTextLayout & layout, int elideIndex);
		/// вычисляет natural bounding rectangle в layout после DoLayout(layout, ...),
		/// учитывая результат выполнения последнего(смотри описание DoLayout)
		QRectF NaturalBoundingRect(QTextLayout & layout, int elideIndex);

		/// вычисляет прямоугольник для elide линии(смотри описание DoLayout)
		QRectF ElideLineRect(QTextLayout & layout, int elideIndex, const QRectF & boundingRectangle);

		/// выравнивает прямоугольник к заданным размерам учитывая alignment, direction элемента
		inline QRect AlignedRect(QStyle * style, const QStyleOptionViewItem & opt, const QSize & size, const QRect & rect)
		{
			return style->alignedRect(opt.direction, opt.displayAlignment, size, rect);
		}
		
		/// возвращает инициализированный QTextOption оп opt
		/// setWrapMode, setDirection, setAlignment
		QTextOption PrepareTextOption(const QStyleOptionViewItem & opt);

		/// рисует линии из layout до elideIndex в drawRect(смотри описание DoLayout)
		void DrawLayout(QPainter * painter, const QPointF & drawPos, const QTextLayout & layout, int elideIndex);
		
		inline void DrawLayout(QPainter * painter, const QPointF & drawPos, const QTextLayout & layout)
		{ return DrawLayout(painter, drawPos, layout, layout.lineCount()); }

	}

	/// Подготавливает painter для дальнейшего рисования
	/// выставляет шрифт, перо, фон.
	void PreparePainter(QPainter * painter, const QStyleOptionViewItem & opt);

	/// Рисует рамку редактируемого тектса(если opt.state содержит QStyle::State_Editing).
	/// Стандартный ItemView делегат рисует ее на этапе рисования текста.
	/// textRect следует получать с помощью QtTools::Delegates::TextSubrect(opt), это базовый способ QStyledItemDelegate
	void DrawEditingFrame(QPainter * painter, const QRect & textRect, const QStyleOptionViewItem & opt);

	/// Рисует текст text в textRect painter'а с помощью QTextLayout, учитывая параметры из opt.
	/// Данный метод не вызывает PreparePainter, RemoveTextMargin.
	/// additionalFormats - дополнительные форматы для opt.text
	void DrawFormattedText(QPainter * painter, const QString & text, const QRect & textRect, const QStyleOptionViewItem & opt,
	                       const QVector<QTextLayout::FormatRange> & additionalFormats = {});
}}
