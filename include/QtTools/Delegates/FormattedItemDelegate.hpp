#pragma once
#include <functional>
#include <QtCore/QList>
#include <QtGui/QTextLayout>
#include <QtTools/Delegates/StyledDelegate.hpp>

namespace QtTools {
namespace Delegates
{
	/// делегат который умеет выводить расцвеченный цвет и в целом все, что поддерживается QTextCharFormat
	/// работает на основе QTextLayout
	/// вы устанавливаете функцию, которая будет генерировать дополнительные форматы
	class FormattedItemDelegate : public StyledDelegate
	{
	public:
		/// тип функции устанавливаемой пользователем
		/// принимает QStyleOptionViewItem инициализированную с помощью initStyleOption, и индекс 
		typedef std::function<
			QList<QTextLayout::FormatRange>(const QStyleOptionViewItem & initedOption, const QModelIndex & idx)
		> FormatterFunctor;

	private:
		FormatterFunctor m_formatter;

	private:
		QList<QTextLayout::FormatRange> Format(const QStyleOptionViewItem & initedOption, const QModelIndex & idx) const;
		void DrawText(QPainter * painter, QStyleOptionViewItem & opt) const override;

	public:
		void SetFormater(const FormatterFunctor & formatter) { this->m_formatter = formatter; }

		FormattedItemDelegate(QObject * parent = nullptr)
			: StyledDelegate(parent) {}
		FormattedItemDelegate(const FormatterFunctor & formatter, QObject * parent = nullptr)
			: m_formatter(formatter) {}
	};
}}
