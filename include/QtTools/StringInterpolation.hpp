#pragma once
#include <ext/string_interpolation.hpp>
#include <QtTools/ToolsBase.hpp>

namespace QtTools
{
	template <class Dictionary>
	inline void interpolate(const QString & text, const Dictionary & dict, QString & res)
	{
		auto input = boost::make_iterator_range_n(text.utf16(), text.length());
		ext::interpolate_internal::interpolate_into_container(input.begin(), input.end(), res, dict);
	}

	template <class Dictionary>
	inline QString interpolate(const QString & text, const Dictionary & dict)
	{
		QString result;
		interpolate(text, dict, result);
		return result;
	}
}