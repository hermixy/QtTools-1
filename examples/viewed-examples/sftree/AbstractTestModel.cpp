#include "AbstractTestModel.hqt"
#include <QtTools/ToolsBase.hpp>


template <class Entity, class Type, Type Entity::*member, class Pred>
bool test_tree_sorter::compare_entity(const Entity & e1, const Entity & e2) noexcept
{
	Pred pred;
	const auto & v1 = e1.*member;
	const auto & v2 = e2.*member;
	return pred(v1, v2);
}

void test_tree_sorter::reset(unsigned type, Qt::SortOrder order)
{
	switch (type)
	{
	default:
		m_leaf_compare = nullptr;
		m_node_compare = nullptr;
		return;

	case 0:
		m_leaf_compare = order == Qt::AscendingOrder ? &compare_entity<leaf_type, path_type, &leaf_type::filename, std::less<>> : &compare_entity<leaf_type, path_type, &leaf_type::filename, std::greater<>>;
		m_node_compare = order == Qt::AscendingOrder ? &compare_entity<node_type, path_type, &node_type::filename, std::less<>> : &compare_entity<node_type, path_type, &node_type::filename, std::greater<>>;
		return;

	case 1:
		m_leaf_compare = order == Qt::AscendingOrder ? &compare_entity<leaf_type, std::string, &leaf_type::sometext, std::less<>> : &compare_entity<leaf_type, std::string, &leaf_type::sometext, std::greater<>>;
		m_node_compare = order == Qt::AscendingOrder ? &compare_entity<node_type, std::string, &node_type::sometext, std::less<>> : &compare_entity<node_type, std::string, &node_type::sometext, std::greater<>>;

	case 2:
		m_leaf_compare = order == Qt::AscendingOrder ? &compare_entity<leaf_type, int, &leaf_type::int_value, std::less<>> : &compare_entity<leaf_type, int, &leaf_type::int_value, std::greater<>>;
		m_node_compare = order == Qt::AscendingOrder ? &compare_entity<node_type, int, &node_type::int_value, std::less<>> : &compare_entity<node_type, int, &node_type::int_value, std::greater<>>;
		return;

	}
}

void test_tree_sorter::reset(viewed::nosort_type)
{
	m_leaf_compare = nullptr;
	m_node_compare = nullptr;
}

viewed::refilter_type test_tree_filter::set_expr(QString expr)
{
	expr = expr.trimmed();

	if (expr.compare(m_filterStr, Qt::CaseInsensitive) == 0)
		return viewed::refilter_type::same;

	if (expr.startsWith(m_filterStr, Qt::CaseInsensitive))
	{
		m_filterStr = expr;
		return viewed::refilter_type::incremental;
	}
	else
	{
		m_filterStr = expr;
		return viewed::refilter_type::full;
	}
}

bool test_tree_filter::matches(const pathview_type & val) const noexcept
{
	return val.contains(m_filterStr, Qt::CaseInsensitive);
}

/************************************************************************/
/*                       AbstractTestModel                              */
/************************************************************************/
unsigned AbstractTestModel::MetaToViewIndex(unsigned meta_index) const
{
	auto first = m_columns.begin();
	auto last  = m_columns.end();
	auto it = std::find(first, last, meta_index);

	return it == last ? -1 : it - first;
}

void AbstractTestModel::SetColumns(std::vector<unsigned> columns)
{
	beginResetModel();
	m_columns = std::move(columns);
	endResetModel();
}

int AbstractTestModel::columnCount(const QModelIndex & parent /* = QModelIndex() */) const
{
	return qint(m_columns.size());
}

QString AbstractTestModel::FieldName(int section) const
{
	if (section >= qint(m_columns.size()))
		return QString::null;

	int field = ViewToMetaIndex(section);

	switch (field)
	{
		case 0:  return QStringLiteral("filename");
		case 1:  return QStringLiteral("sometext");
		case 2:  return QStringLiteral("int_value");

		default:
			return QString::null;
	}
}

QString AbstractTestModel::FieldName(const QModelIndex & index) const
{
	return FieldName(index.column());
}

QString AbstractTestModel::GetString(const QModelIndex & index) const
{
	return GetItem(index).toString();
}

QString AbstractTestModel::GetStringShort(const QModelIndex & index) const
{
	return GetItem(index).toString();
}

void AbstractTestModel::SetFilter(QString expr)
{
	m_filterStr = std::move(expr);
	FilterBy(m_filterStr);
	Q_EMIT FilterChanged(m_filterStr);
}

void AbstractTestModel::sort(int column, Qt::SortOrder order /* = Qt::AscendingOrder */)
{
	m_sortColumn = column;
	m_sortOrder = order;

	SortBy(column, order);
	Q_EMIT SortingChanged(column, order);
}


QVariant AbstractTestModel::data(const QModelIndex & index, int role /* = Qt::DisplayRole */) const
{
	switch (role)
	{
	    case Qt::DisplayRole:
	    case Qt::ToolTipRole:
	    case Qt::UserRole: return GetItem(index);
	    default:           return {};
	}
}

QVariant AbstractTestModel::headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole */) const
{
	if (orientation == Qt::Vertical)
		return base_type::headerData(section, orientation, role);

	switch (role)
	{
	    case Qt::DisplayRole:
	    case Qt::ToolTipRole:
		    return FieldName(section);

	    default: return {};
	}
}


/************************************************************************/
/*                      AbstractTableTestModel                          */
/************************************************************************/
Qt::ItemFlags AbstractTableTestModel::flags(const QModelIndex & index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	//if (index.isValid())
	flags |= Qt::ItemNeverHasChildren;
	return flags;
}

QModelIndex AbstractTableTestModel::index(int row, int column, const QModelIndex & parent /*= QModelIndex()*/) const
{
	return hasIndex(row, column, parent) ? createIndex(row, column) : QModelIndex();
}

QModelIndex AbstractTableTestModel::parent(const QModelIndex & child) const
{
	return QModelIndex();
}

QModelIndex AbstractTableTestModel::sibling(int row, int column, const QModelIndex & idx) const
{
	return index(row, column);
}

bool AbstractTableTestModel::hasChildren(const QModelIndex & parent) const
{
	//return (parent.model() == this and not parent.isValid()) and (rowCount(parent) > 0 and columnCount(parent) > 0);
	return false;
}
