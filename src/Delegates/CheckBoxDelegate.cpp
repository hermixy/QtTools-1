#include <QtTools/Delegates/CheckBoxDelegate.hpp>
#include <QtTools/Delegates/StyledParts.hpp>
#include <QtWidgets/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>

namespace QtTools {
namespace Delegates
{
	static QRect CheckmarkRect(const QStyleOptionViewItem & opt)
	{
		auto * style = AccquireStyle(opt);
		QRect rect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, opt.widget);
		return style->alignedRect(opt.direction, opt.displayAlignment, rect.size(), opt.rect);
	}

	void CheckBoxDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		// мы пытаемся нарисовать checkbox,
		// при этом мы хотим что бы он выглядел как тот, который рисуется QStyledItemDelegate по умолчанию,
		// но с центрированием и полным отбрасыванием текста

		// CE_ItemViewItem рисуется следующим образом(подсмотрено из qcommonstyle.cpp:2147 версии qt 5.3)
		// 1. background, PE_PanelItemViewItem
		// 2. checkmark   PE_IndicatorViewItemCheck
		// 3. draw Icon   icon.paint
		// 4. draw text
		// 5. draw focus rect, PE_FrameFocusRect
		// 
		// мы хотим только PE_PanelItemViewItem, PE_IndicatorViewItemCheck, PE_FrameFocusRect
		auto opt = option;
		opt.displayAlignment = Qt::AlignCenter;
		opt.features |= QStyleOptionViewItem::HasCheckIndicator;
		opt.checkState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());

		painter->save();
		painter->setClipRect(opt.rect);

		DrawBackground(painter, opt);
		DrawCheckmark(painter, CheckmarkRect(opt), opt);
		if (HasFocusFrame(opt))
			DrawFocusFrame(painter, opt.rect, opt);

		painter->restore();

	}

	QSize CheckBoxDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		auto * style = AccquireStyle(option);
		return style->sizeFromContents(QStyle::CT_ItemViewItem, &option, QSize {}, option.widget);
	}

	bool CheckBoxDelegate::editorEvent(QEvent * event, QAbstractItemModel * model,
	                                   const QStyleOptionViewItem & option, const QModelIndex & index)
	{
		// код взят из qstyleditemdelegate.cpp:642 версии qt 5.3 и немного переработан, в основном стилистически

		// make sure that the item is checkable
		Qt::ItemFlags flags = model->flags(index);
		bool checkable = flags & Qt::ItemIsUserCheckable &&
			option.state & QStyle::State_Enabled &&
			flags & Qt::ItemIsEnabled;

		if (!checkable)
			return false;

		// make sure that we have a check state
		QVariant value = index.data(Qt::CheckStateRole);
		if (!value.isValid())
			return false;

		auto evType = event->type();
		bool mouseButtonEvent = evType == QEvent::MouseButtonRelease ||
			evType == QEvent::MouseButtonDblClick ||
			evType == QEvent::MouseButtonPress;

		if (mouseButtonEvent)
		{
			auto * me = static_cast<QMouseEvent*>(event);
			if (me->button() != Qt::LeftButton)
				return false;

			auto opt = option;
			opt.displayAlignment = Qt::AlignCenter;
			opt.features |= QStyleOptionViewItem::HasCheckIndicator;

			//  проверяем что пользователь щелкнул именно по чекбоксу,
			//  тут можно подправить, если хочется что бы реагировало на любые нажатия
			QRect checkRect = CheckmarkRect(opt);
			if (!checkRect.contains(me->pos()))
				return false;

			if (evType == QEvent::MouseButtonPress || evType == QEvent::MouseButtonDblClick)
				return true;
		}
		else if (evType == QEvent::KeyPress)
		{
			auto * ke = static_cast<QKeyEvent*>(event);
			auto key = ke->key();
			if (key != Qt::Key_Space && key != Qt::Key_Select)
				return false;
		}
		else {
			return false;
		}

		auto state = static_cast<Qt::CheckState>(value.toInt());
		if (flags & Qt::ItemIsTristate)
			state = ((Qt::CheckState)((state + 1) % 3));
		else
			state = (state == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
		return model->setData(index, state, Qt::CheckStateRole);
	}

}}
