#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOption>
#include <QtWidgets/QStyledItemDelegate>

namespace QtTools {
namespace Delegates 
{
	/// Delegate который ведет себя подобно QStyledItemDelegate,
	/// но предоставляет перегружаемые функции рисования частей себя, подобно QItemDelegate, 
	/// при этом рисование происходит с учетом стиля.
	/// по факту вся работа делает в свободных функциях из
	///   QtTools/Delegates/StyledParts.h 
	///   QtTools/Delegates/DrawFormattedText.h 
	class StyledDelegate : public QStyledItemDelegate
	{
	protected:
		/// иницилизирует option из index, дефолтная реализация вызывает initFromOption
		virtual void InitStyle(QStyleOptionViewItem & option, const QModelIndex & index) const;

		/// функции принимает painter и option, если наследнику нужна доп информация - 
		/// то или писать свои функции, или выставлять некий временный член, или расширить option
		/// option создается в методе paint и существует пока отрабатывает paint,
		/// может быть модифицирован по желанию

		/// рисует фон, дефолтная реализация:
		/// Delegates::DrawBackground(painter, option);
		virtual void DrawBackground(QPainter * painter, QStyleOptionViewItem & option) const;
		/// рисует кряж, дефолтная реализация:
		/// Delegates::DrawCheckmark(painter, Delegates::CheckmarkSubrect(option), option);
		virtual void DrawCheckmark(QPainter * painter, QStyleOptionViewItem & option) const;
		/// рисует значок, дефолтная реализация:
		/// Delegates::DrawDecoration(painter, Delegates::DecorationSubrect(option), option);
		virtual void DrawDecoration(QPainter * painter, QStyleOptionViewItem & option) const;
		/// рисует текст, дефолтная реализация:
		/// Delegates::DrawFormattedText(painter, Delegates::TextSubrect(option), option);
		virtual void DrawText(QPainter * painter, QStyleOptionViewItem & option) const;
		/// рисует рамку фокуса, дефолтная реализация:
		/// Delegates::DrawFocusFrame(painter, Delegates::FocusFrameSubrect(option), option);
		virtual void DrawFocusFrame(QPainter * painter, QStyleOptionViewItem & option) const;

	public:
		/// реализация метода paint.
		/// 
		/// метод вызывает InitStyle, 
		/// после чего делает Delegates::FixStyleOptionViewItem(opt)
		/// 
		/// painter->save();
		/// painter->setClipRect(opt.rect);
		/// painter->translate(opt.rect.topLeft());
		/// opt.rect.moveTopLeft({0, 0});
		/// 
		/// последовательно вызывает методы рисования частей, но только если данная часть присутствует.
		/// т.е. if (HasCheckmark) DrawCheckmark и т.д.
		/// порядок рисования: DrawBackground -> DrawCheckmark -> DrawDecoration -> DrawText -> DrawFocusFrame
		void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;

	public:
		StyledDelegate(QObject * parent = nullptr)
			: QStyledItemDelegate(parent) {}
	};
}}