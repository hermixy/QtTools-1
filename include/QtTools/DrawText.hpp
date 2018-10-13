#pragma once
#include <QtGui/QPainter>

namespace QtTools
{
	/// реализация drawText с поддержкой flags для QPoint,
	/// в painter, уже есть реализация, но для QRect
	void DrawText(QPainter & painter, QPointF pos, int flags, const QString & text, QRect * boundingRect = nullptr);
	void DrawText(QPainter & painter, QPoint pos, int flags, const QString & text, QRect * boundingRect = nullptr);

	inline void DrawText(QPainter & painter, qreal x, qreal y, int flags, const QString & text, QRect * boundingRect = nullptr)
	{ DrawText(painter, QPointF(x, y), flags, text, boundingRect); }
	
	inline void DrawText(QPainter & painter, int x, int y, int flags, const QString & text, QRect * boundingRect = nullptr)
	{ DrawText(painter, QPointF(x, y), flags, text, boundingRect); }
}

