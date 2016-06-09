#pragma once
#include <QtCore/QString>
#include <QtTools/Delegates/StyledDelegate.hpp>
#include <QtTools/Delegates/DrawFormattedText.hpp>

namespace QtTools {
namespace Delegates
{
	/// рисует текст opt.text, в textRect painter'а с помощью QTextLayout, учитывая opt.
	/// некоторые части визуально выделены для отображения найденных частей(задается selectionFormats).
	/// при обрезании текста, если за обрезанием остается форматирование - оно переносится на "..."
	void DrawSearchFormatedText(QPainter * painter, QRect textRect, const QStyleOptionViewItem & opt,
	                            const QList<QTextLayout::FormatRange> & selectionFormats);

	/// "раскрашивает" text в соответствии с filterWord
	/// слово ищется в text, совпадения раскрашивается в заданный формат
	void FormatSearchText(const QString & text, const QString & filterWord, 
	                      const QTextCharFormat & format, QList<QTextLayout::FormatRange> & formats);
	
	/// делегат, отображающий текст подобно QStyledItemDelegate.
	/// дополнительно ищет заданный текст в отображаемой строке, и подсвечивает найденные участки
	class SearchDelegate : public StyledDelegate
	{
	protected:
		QString m_searchText;
		QTextCharFormat m_format;

	protected:
		/// форматирует текст text в соответствии с m_searchText в formats
		/// реализация по-умолчанию вызывает FormatSearchText
		virtual void FormatText(const QString & text, QList<QTextLayout::FormatRange> & formats) const;
		void DrawText(QPainter * painter, QStyleOptionViewItem & opt) const override;

	public:
		QString GetFilterText() const { return m_searchText; }
		void    SetFilterText(const QString text) { m_searchText = text; }

		const QTextCharFormat & GetFormat() const                         { return m_format; }
		void                    SetFormat(const QTextCharFormat & format) { m_format = format; }

	public:
		Q_DISABLE_COPY(SearchDelegate);

		SearchDelegate(QObject * parent = nullptr) : StyledDelegate(parent)
		{
			m_format.setForeground(Qt::GlobalColor::red);
			m_format.setBackground(Qt::GlobalColor::green);
		}
		
	};
}}
