#include <QtTools/BasicHeaderControlModelHelper.hqt>

const QString BasicHeaderControlModelHelper::CodeListMime::MimeCodeFormat =
	QStringLiteral("application/x-BasicHeaderControlModel-ColumnCodes");

const QStringList BasicHeaderControlModelHelper::CodeListMime::MimeFormats =
	QStringList(BasicHeaderControlModelHelper::CodeListMime::MimeCodeFormat);

bool BasicHeaderControlModelHelper::CodeListMime::hasFormat(const QString & mimetype) const
{
	return mimetype == MimeCodeFormat;
}

QStringList BasicHeaderControlModelHelper::CodeListMime::formats() const
{
	return MimeFormats;
}

