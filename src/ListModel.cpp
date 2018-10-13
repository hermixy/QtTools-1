#include <vector>
#include <QtTools/ListModel.hqt>

namespace QtTools
{
	const QString      ListModel_MimeData::MimeFormat = QStringLiteral("application/x-ListModel-indexes");
	const QStringList  ListModel_MimeData::MimeFormats = {MimeFormat};


	QStringList ListModel_MimeData::formats() const
	{
		return MimeFormats;
	}

	bool ListModel_MimeData::hasFormat(const QString & mimetype) const
	{
		return mimetype == MimeFormat;
	}

	Qt::ItemFlags ListModelBase::flags(const QModelIndex & index) const
	{
		auto defFlags = QAbstractListModel::flags(index);
		defFlags |= Qt::ItemIsEditable /* | Qt::ItemNeverHasChildren*/;

		// не валидный индекс - это позиция между строками
		// ее делаем ItemIsDropEnabled что бы можно было скидывать
		// валидные же индексы только ItemIsDragEnabled
		defFlags |= index.isValid() ? Qt::ItemIsDragEnabled : Qt::ItemIsDropEnabled;
		return defFlags;
	}

	Qt::DropActions ListModelBase::supportedDropActions() const
	{
		return Qt::MoveAction | Qt::CopyAction;
	}

	Qt::DropActions ListModelBase::supportedDragActions() const
	{
		return Qt::MoveAction | Qt::CopyAction;
	}

	QStringList ListModelBase::mimeTypes() const
	{
		return ListModel_MimeData::MimeFormats;
	}

	QMimeData * ListModelBase::mimeData(const QModelIndexList & indexes) const
	{
		return new ListModel_MimeData(indexes);
	}

	bool ListModelBase::canDropMimeData(const QMimeData * data, Qt::DropAction action,
										int row, int column, const QModelIndex & parent) const
	{
		return (action == Qt::CopyAction || action == Qt::MoveAction)
			&& qobject_cast<const ListModel_MimeData *>(data);
	}

	bool ListModelBase::dropMimeData(const QMimeData * data, Qt::DropAction action,
									 int row, int column, const QModelIndex & parent)
	{
		auto * mdata = qobject_cast<const ListModel_MimeData *>(data);
		if (!mdata) return false;

		const QModelIndexList & elements = mdata->elements;
		if (elements.empty()) return true;

		auto * model = const_cast<QAbstractItemModel *>(elements[0].model());

		std::vector<int> indexes(elements.size());
		auto first = indexes.data();
		auto last  = first + indexes.size();
		std::transform(elements.begin(), elements.end(), first, std::mem_fn(&QModelIndex::row));
		std::sort(first, last);


		// в нашем случае parent всегда invalid, поскольку мы не TreeModel
		// если row >= 0 - row указывает что скидывание произошло строго перед элементом с индексом row, для заданного parent'а
		// если же row == -1 мы:
		//  * или скидываем на элемент и тогда parent его идентифицирует,
		//     в нашем случае невозможно, поскольку мы запретили это методом flags
		//  * или скидываем после последнего элемента
		
		bool onto;

		if (row != -1)
			onto = false;
		else
		{
			if (parent.isValid())
			{
				onto = true;
				row = parent.row();
			}
			else
			{
				onto = false;
				row = rowCount();
			}
		}

		switch (action)
		{
			default: return false;

			case Qt::CopyAction:
				return onto ?
					DndCopyOnto(*model, first, last, row) :
					DndCopyBefore(*model, first, last, row);

			case Qt::MoveAction:
				return onto ?
					DndMoveOnto(*model, first, last, row) :
					DndMoveBefore(*model, first, last, row);
		}
	}
}
