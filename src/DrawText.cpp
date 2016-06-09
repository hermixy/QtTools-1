#include <QtTools/DrawText.hpp>

namespace QtTools
{
	void DrawText(QPainter & painter, QPoint pos, int flags, const QString & text, QRect * boundingRect /* = nullptr */)
	{
		int size = 32767;
		QPoint corner {pos.x(), pos.y() - size};
		switch (flags & Qt::AlignHorizontal_Mask)
		{
			case Qt::AlignLeft:                               break;
			case Qt::AlignRight:     corner.rx() -= size;     break;
			case Qt::AlignHCenter:   corner.rx() -= size / 2; break;
		}

		switch (flags & Qt::AlignVertical_Mask)
		{
			case Qt::AlignTop:       corner.ry() += size;     break;
			case Qt::AlignBottom:                             break;
			case Qt::AlignVCenter:   corner.ry() += size / 2; break;
		}

		QRect rect(corner, QSize(size, size));
		painter.drawText(rect, flags, text, boundingRect);
	}

	void DrawText(QPainter & painter, QPointF pos , int flags, const QString & text, QRectF * boundingRect /* = nullptr */)
	{
		qreal size = 32767;
		QPointF corner {pos.x(), pos.y() - size};
		switch (flags & Qt::AlignHorizontal_Mask)
		{
			case Qt::AlignLeft:                               break;
			case Qt::AlignRight:     corner.rx() -= size;     break;
			case Qt::AlignHCenter:   corner.rx() -= size / 2; break;
		}

		switch (flags & Qt::AlignVertical_Mask)
		{
			case Qt::AlignTop:       corner.ry() += size;     break;
			case Qt::AlignBottom:                             break;
			case Qt::AlignVCenter:   corner.ry() += size / 2; break;
		}

		QRectF rect(corner, QSize(size, size));
		painter.drawText(rect, flags, text, boundingRect);
	}
}
