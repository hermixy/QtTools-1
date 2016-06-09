#pragma once
#include <QtWidgets/QAbstractItemDelegate>

namespace QtTools {
namespace Delegates
{
	/// делегат, который рисует checkbox подобно стандартному делегату 
	/// и вообще пытается вести подобно стандартному,
	/// но не рисует текст, чекбокс позиционируется по центру, чекбокс рисуется всегда
	/// так же на данный момент игнорирует роли FontRole, Background, подобное, но использует палитру, 
	/// с помощью которой его можно кастомизировать из вне
	class CheckBoxDelegate : public QAbstractItemDelegate
	{
	public:
		void paint(QPainter * painter, 
		           const QStyleOptionViewItem & option,
		           const QModelIndex & index) const override;
	
		QSize sizeHint(const QStyleOptionViewItem & option,
		               const QModelIndex & index) const override;

		bool editorEvent(QEvent * event,
		                 QAbstractItemModel * model,
		                 const QStyleOptionViewItem & option,
		                 const QModelIndex & index) override;

		CheckBoxDelegate(QObject * parent = nullptr)
			: QAbstractItemDelegate(parent) {}
	};
}}
