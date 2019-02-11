#pragma once
#include <tuple>
#include <string>
#include <string_view>

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringRef>

namespace viewed
{
	struct std_string_sftree_traits_base
	{
		using path_type = std::string;
		using pathview_type = std::string_view;

		using path_equal_to_type = std::equal_to<>;
		using path_less_type     = std::less<>;
		using path_hash_type     = std::hash<pathview_type>;

		static pathview_type get_name(pathview_type path);

		auto parse_path(const pathview_type & context, const pathview_type & path) const
		    -> std::tuple<std::uintptr_t, pathview_type, pathview_type>;
		bool is_child(const pathview_type & path, const pathview_type & context, const pathview_type & node_name) const;

		// set_name, get_name(leaf/node_type),
		// sort_pred_type, filter_pred_type
		// get_path should be defined in derived class
	};

	namespace detail
	{
		struct qqstring_hash
		{
			std::size_t operator()(const QString & str) const noexcept
			{
				return qHash(str);
			}

			std::size_t operator()(const QStringRef & str) const noexcept
			{
				return qHash(str);
			}
		};
	}

	struct qstring_sftree_traits_type
	{
		using path_type = QString;
		using pathview_type = QStringRef;

		using path_equal_to_type = std::equal_to<>;
		using path_less_type     = std::less<>;
		using path_hash_type     = detail::qqstring_hash;

		static pathview_type get_name(pathview_type path);

		auto parse_path(const pathview_type & context, const pathview_type & path) const
		    -> std::tuple<std::uintptr_t, pathview_type, pathview_type>;
		bool is_child(const pathview_type & path, const pathview_type & context, const pathview_type & node_name) const;

		// set_name, get_name(leaf/node_type),
		// sort_pred_type, filter_pred_type
		// get_path should be defined in derived class
	};
}
