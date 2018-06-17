#pragma once
#include <QtCore/QAbstractItemModel>

namespace viewed
{
	class AbstractItemModel : public QAbstractItemModel
	{
		typedef QAbstractItemModel base_type;

		// bring to public from protected
	public:
		using base_type::createIndex;
		using base_type::beginResetModel;
		using base_type::endResetModel;

		using base_type::beginInsertRows;
		using base_type::endInsertRows;

		using base_type::beginRemoveRows;
		using base_type::endRemoveRows;

		using base_type::beginMoveRows;
		using base_type::endMoveRows;

		using base_type::layoutAboutToBeChanged;
		using base_type::layoutChanged;

		using base_type::persistentIndexList;
		using base_type::changePersistentIndex;
		using base_type::changePersistentIndexList;

	public:
		static const QModelIndex                  invalid_index;
		static const QList<QPersistentModelIndex> empty_model_list;
		static const QVector<int>                 all_roles;
	};
}
