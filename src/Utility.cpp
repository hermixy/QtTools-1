#include <QtWidgets/QLineEdit>
#include <QtWidgets/QApplication>
#include <QtTools/Utility.hpp>


namespace QtTools
{
	QIcon LoadIcon(const QString & themeIcon, QStyle::StandardPixmap fallback, const QStyle * style /*= nullptr*/)
	{
		if (QIcon::hasThemeIcon(themeIcon))
			return QIcon::fromTheme(themeIcon);

		if (style == nullptr)
			style = qApp->style();

		return style->standardIcon(fallback);
	}

	QIcon LoadIcon(const QString & themeIcon, const QString & fallback)
	{
		if (QIcon::hasThemeIcon(themeIcon))
			return QIcon::fromTheme(themeIcon);

		return QIcon(fallback);
	}

	QSize ToolBarIconSizeForLineEdit(QLineEdit * lineEdit)
	{
#ifdef Q_OS_WIN
		// on windows pixelMetric(QStyle::PM_DefaultFrameWidth) returns 1,
		// but for QLineEdit internal code actually uses 2
		constexpr auto frameWidth = 2;
#else
		const auto frameWidth = lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
#endif

		lineEdit->adjustSize();
		auto height = lineEdit->size().height();
		height -= frameWidth;

		return {height, height};
	}
}
