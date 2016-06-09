#pragma once
#include <QtCore/QString>
#include <vector>

namespace QtTools
{
	/// информация по секции QHeaderInfo
	/// подробнее смотри HeaderControlModel
	struct HeaderSectionInfo
	{
		QString code;
		bool hidden = false;
		int width = -1;

		template <class Archive>
		void serialize(Archive & ar, unsigned ver)
		{
			ar & code & hidden & width;
		}
	};

	typedef std::vector<HeaderSectionInfo> HeaderSectionInfoList;
}