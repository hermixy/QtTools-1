#include "TestTreeModel.hqt"
#include <QtTools/ToolsBase.hpp>

auto test_entity_sftree_traits::parse_path(const pathview_type & path, const pathview_type & context) const
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

bool test_entity_sftree_traits::is_child(const pathview_type & path, const pathview_type & context, const pathview_type & node_name) const
{
	auto ref = path.mid(context.size(), node_name.size());
	return ref == node_name;
}

void TestTreeModelBase::recalculate_page(page_type & page)
{
	page.int_value = 0;
	for (auto & child : page.children)
	{
		page.int_value += viewed::visit([](auto * item) { return item->int_value; }, child);
	}
}

void TestTreeModelBase::SortBy(int section, Qt::SortOrder order)
{
	section = ViewToMetaIndex(section);
	base_type::sort_by(section, order);
}

void TestTreeModelBase::FilterBy(QString expr)
{
	base_type::filter_by(expr);
}

struct any_from_element
{
	auto operator()(const test_tree_entity * ptr) const { return QVariant::fromValue(ptr); }
	auto operator()(const test_tree_node   * ptr) const { return QVariant::fromValue(ptr); }
};

QVariant TestTreeModelBase::GetEntity(const QModelIndex & index) const
{
	if (not index.isValid()) return QVariant();

	const auto & val = get_element_ptr(index);
	return viewed::visit(any_from_element(), val);
}

QVariant TestTreeModelBase::GetItem(const QModelIndex & index) const
{
	if (not index.isValid()) return QVariant();

	const auto & val = get_element_ptr(index);
	const auto meta_index = ViewToMetaIndex(index.column());

	auto visitor = [meta_index](auto * ptr) -> QVariant
	{
		switch (meta_index)
		{
			case 0:  return QVariant::fromValue(::get_name(&ptr->filename).toString());
			case 1:  return QVariant::fromValue(ptr->sometext);
			case 2:  return QVariant::fromValue(ptr->int_value);
			default: return {};
		}
	};

	return viewed::visit(visitor, val);
}

int TestTreeModelBase::FullRowCount(const QModelIndex & index) const
{
	if (not index.isValid()) return 0;

	const auto * page = get_page(index);
	return page->children.size();
}
