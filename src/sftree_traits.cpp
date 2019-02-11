#include <viewed/sftree_traits.hpp>
#include <viewed/sftree_facade_qtbase.hpp>

namespace viewed
{
	inline static int find_first_of(std::string_view str, std::string_view needle)
	{
		return str.find_first_of(needle);
	}

	static int rfind_fist_of(std::string_view str, std::string_view needle)
	{
		auto first = str.begin();
		auto last  = str.end();

		using std::make_reverse_iterator;
		auto it = std::find_first_of(make_reverse_iterator(first), make_reverse_iterator(last), needle.begin(), needle.end()).base();
		return it == last ? -1 : it - first;
	}

	auto std_string_sftree_traits_base::get_name(pathview_type path) -> pathview_type
	{
		int pos = find_first_of(path, "/\\") + 1;
		return path.substr(pos);
	}

	auto std_string_sftree_traits_base::parse_path(const pathview_type & path, const pathview_type & context) const
	    -> std::tuple<std::uintptr_t, pathview_type, pathview_type>
	{
		std::string_view seps = "/\\";
		//[first, last) - next segment in leaf_path,
		auto first = path.begin() + context.size();
		auto last = path.end();
		auto it = std::find_first_of(first, last, seps.begin(), seps.end());

		if (it == last)
		{
			pathview_type name = path.substr(context.size());
			return std::make_tuple(viewed::LEAF, context, std::move(name));
		}
		else
		{
			pathview_type name = path.substr(context.size(), it - first);
			it = std::find_if_not(it, last, [](auto ch) { return ch == '/'; });
			pathview_type newpath = path.substr(0, it - path.begin());
			return std::make_tuple(viewed::PAGE, std::move(newpath), std::move(name));
		}
	}

	bool std_string_sftree_traits_base::is_child(const pathview_type & path, const pathview_type & context, const pathview_type & node_name) const
	{
		auto ref = path.substr(node_name.size(), context.size());
		return ref == node_name;
	}


	auto qstring_sftree_traits_type::get_name(pathview_type path) -> pathview_type
	{
		int pos = path.lastIndexOf('/') + 1;
		return path.mid(pos);
	}

	auto qstring_sftree_traits_type::parse_path(const pathview_type & path, const pathview_type & context) const
	    -> std::tuple<std::uintptr_t, pathview_type, pathview_type>
	{
		//[first, last) - next segment in leaf_path,
		auto first = path.begin() + context.size();
		auto last = path.end();
		auto it = std::find(first, last, '/');

		if (it == last)
		{
			pathview_type name = path.mid(context.size()); // = QString::null;
			return std::make_tuple(viewed::LEAF, context, std::move(name));
		}
		else
		{
			pathview_type name = path.mid(context.size(), it - first);
			it = std::find_if_not(it, last, [](auto ch) { return ch == '/'; });
			pathview_type newpath = path.mid(0, it - path.begin());
			return std::make_tuple(viewed::PAGE, std::move(newpath), std::move(name));
		}
	}

	bool qstring_sftree_traits_type::is_child(const pathview_type & path, const pathview_type & context, const pathview_type & node_name) const
	{
		auto ref = path.mid(node_name.size(), context.size());
		return ref == node_name;
	}
}
