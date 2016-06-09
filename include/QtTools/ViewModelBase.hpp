#pragma once
#include <QtCore/QString>
#include <QtCore/QAbstractTableModel>

namespace QtTools
{
	/// База для построения qt-моделей проекций основанных на проекциях из namespace viewed
	/// 
	/// реализует протокол обновления qt-моделей
	/// ViewType должно реализовывать stl интерфейс, size, begin, end, RandomAccessContainer
	///          должно предоставлять virtual методы подобно viewed::view_base: 
	///               merge_newdata, erase_records, clear_view, reinit_view
	///
	/// Наследник должен реализовывать методы:
	///  * int ColumnCount() - возвращает кол-во колонок в проекции
	///  * int FindRecord(view_type::const_pointer) - находит позицию элемента по указателю
	template <class ViewType, class ModelBase = QAbstractTableModel>
	class ViewModelBase : 
		public ModelBase, // QObject derived должен идти первым
		public ViewType
	{
	protected:
		typedef ModelBase base_type;
		typedef ViewType view_type;

		using typename view_type::container_type;
		using typename view_type::signal_range_type;

		/// данное поле содержит текущее кол-во строк в модели
		/// view_type::size() == m_currentRowCount всегда, кроме момента обновления модели, приходящее через
		/// сигналы от соответствующего ViewType
		/// подробнее смотри описание алгоритма обновления ниже
		int m_currentRowCount = 0;
		int m_currentColumnCount = 0;

	protected:
		/// Qt модели требуют выполнения протокола(begin...Rows, end...Rows) для корректной работы постоянных индексов.
		/// Постоянные индексы используются, например, механизмом выделения текущего элемента в таблице.
		/// Протокол весьма строг, и сильно препятствует использованию классов, которые не знают о Qt.
		/// Фактически нужно вставлять вызовы dataChanged, beginInsertRows между точками выполнения, в код, 
		/// которому совсем не нужно знать о Qt, и он мог бы быть прилично эффективнее.
		/// 
		/// Мы попытаемся схитрить. 
		/// 1 перед собственно обновлением, мы фиктивно увеличим размер модели на размер обновления(это потребуется в дальнейшем)
		///   вызовы beginInsertRows/endInsertRows
		/// 2 генерируем сигнал layoutAboutToBeChanged(это заставит selectionModel разбить selectionRanges на индексы),
		///   соответствующий сигнал layoutChanged будет сгенерирован позднее
		/// 3 получаем список индексов, которые нужно обновить
		/// 4 запомним соответствующие им указатели
		/// 5 подготовка на этом заканчивается, и выполняется собственно обновление
		/// 6 проходим по запомненным индексам и ищем позицию сохраненных элементов
		///     * если элемент найден - найдена позиция
		///     * если элемент не найден - он удален, временно перемещаем индекс в конец.
		///       тут что-то типа буферной зоны, куда мы складываем элементы на удаление, 
		///       место мы выделили на шаге 1.
		///   
		/// * генерируем сигнал layoutChanged
		/// * генерируем сигналы beginRemoveRows/endRemoveRows и удаляем индексы строки в конце
		/// * генерируем сигналы beginColumnsInsert/endColumnsInsert, 
		///   тут все просто, на данный момент колонки могут только добавляться
		/// 
		/// контекст для реализации протокола обновления постоянных индексов модели Qt
		struct UpdateContext
		{
			// набор постоянных индексов, запоминается перед обновлением
			QModelIndexList idxs;
			// набор старых записей, до обновления, соответствующих набору индексов
			// если индексов много - это может быть не эффективно
			std::vector<typename view_type::const_pointer> oldRecPtrs;
		};

		UpdateContext m_updCtx;

	protected:
		virtual void merge_newdata(const signal_range_type & sorted_updated, const signal_range_type & inserted) override;
		virtual void erase_records(const signal_range_type & sorted_newrecs) override;
		
		virtual void clear_view() override;
		virtual void reinit_view() override;

	protected:
		void AddExtraRows(std::size_t extraRows);
		void RemoveExtraRows();
		void PrepareForUpdate();
		void RecalcualtePersistentIndexes();
		void AddColumns(int oldColumnCount, int newColumnCount);

		/// обработчик события upsert, вызывается перед собственно обновлением, тут шаги 1-4.
		/// newrecs_size - кол-во обновляемых записей 
		virtual void OnBeforeUpsert(std::size_t newrecs_size);
		/// обработчик события upsert, вызывается после реального выполнения обновления, тут шаги 5-6+
		virtual void OnAfterUpsert();

		/// вызывается из OnAfterUpsert.
		/// на момент вызова m_currentColumnCount старое кол-во колонок,
		/// ColumnCount() - новое кол-во колонок
		virtual void AddColumns();

		/// находит позицию элемента по указателю, должно быть реализовано наследником.
		/// используется в RecalcualtePersistentIndexes
		/// возможно стоит сделать public методом
		virtual int FindRecord(typename view_type::const_pointer recptr) const = 0;

	protected:
		inline static int ToInt(std::size_t val) { return static_cast<int>(val); };
		inline static std::size_t ToSize(int val) { return static_cast<std::size_t>(val); }

	public:
		int rowCount(const QModelIndex & parent = QModelIndex()) const { return m_currentRowCount; }
		int columnCount(const QModelIndex & parent = QModelIndex()) const { return m_currentColumnCount; }

	public:
		/// возвращает кол-во колонок, должно быть реализовано наследником
		virtual int ColumnCount() const = 0;
		/// всегда возвращает реальное кол-во строк, в отличие от rowCount(), побробнее смотри описание класса
		/// по умолчанию вызвает view_size::size();
		virtual int RowCount() const { return ToInt(this->size()); }

	protected:
		ViewModelBase(container_type * owner, QObject * parent)
			: base_type(parent), view_type(owner) {}
	};

	/************************************************************************/
	/*                  View notifications                                  */
	/************************************************************************/
	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::merge_newdata(const signal_range_type & sorted_updated, const signal_range_type & inserted)
	{
		OnBeforeUpsert(inserted.size());
		view_type::merge_newdata(sorted_updated, inserted);
		OnAfterUpsert();
	}

	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::erase_records(const signal_range_type & sorted_newrecs)
	{
		assert(false); // not implemented
	}

	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::clear_view()
	{
		beginResetModel();
		
		view_type::clear_view();
		m_currentRowCount = ToInt(this->size());

		endResetModel();
	}

	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::reinit_view()
	{
		beginResetModel();

		view_type::reinit_view();
		m_currentRowCount = ToInt(this->size());
		m_currentColumnCount = ColumnCount();
		
		endResetModel();
	}

	/************************************************************************/
	/*                  Qt update block                                     */
	/************************************************************************/
	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::AddExtraRows(std::size_t extraRows)
	{
		if (extraRows == 0)
			return;

		beginInsertRows({}, m_currentRowCount, m_currentRowCount + ToInt(extraRows) - 1);
		m_currentRowCount += ToInt(extraRows);
		endInsertRows();
	}

	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::RemoveExtraRows()
	{
		int newSize = ToInt(this->size());
		Q_ASSERT(newSize <= m_currentRowCount);

		if (newSize < m_currentRowCount)
		{
			beginRemoveRows({}, newSize, m_currentRowCount - 1);
			m_currentRowCount = newSize;
			endRemoveRows();
		}
	}

	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::PrepareForUpdate()
	{
		/// recBatch.records приходит отсортированным
		Q_EMIT layoutAboutToBeChanged({}, QAbstractTableModel::VerticalSortHint);

		m_updCtx.idxs = persistentIndexList();
		m_updCtx.oldRecPtrs.resize(ToSize(m_updCtx.idxs.size()));

		auto it = m_updCtx.oldRecPtrs.begin();
		for (const QModelIndex & idx : m_updCtx.idxs)
		{
			std::size_t i = ToSize(idx.row());
			// в хитрых случаях возможны не существующие persistent indexes
			// как минимум такая ситуация складывается, если часть колонок скрыта, а таблица пустая, 
			// тогда есть persistence index'ы на 0-вую строку.
			// в таких случаях в oldRec кладем nullptr(фактически это происходит при инициализации в resize, выше)
			// позднее в RecalcualtePersistentIndexes если проверка на nullptr
			if (i < this->size())
				*it = &*(this->begin() + i);
			++it;
		}
	}

	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::RecalcualtePersistentIndexes()
	{
		int rc = m_currentRowCount;

		auto beg = this->begin();
		int size = ToInt(this->size());

		auto ptrIt = m_updCtx.oldRecPtrs.begin();
		auto idxIt = m_updCtx.idxs.begin();

		for (; idxIt != m_updCtx.idxs.end(); ++idxIt, ++ptrIt)
		{
			auto ptr = *ptrIt;
			const QModelIndex & oldIdx = *idxIt;

			/// если это был "плохой" индекс - кидаем его в начало таблицы
			if (ptr == nullptr) {
				auto newIdx = index(0, oldIdx.column());
				changePersistentIndex(oldIdx, newIdx);
			}
			else {
				int newPos = FindRecord(ptr);
				/// вторая проверка возможно слишком строгая
				Q_ASSERT(newPos == size || &(*this)[newPos] == ptr);

				if (newPos == size)
					newPos = --rc;
				auto newIdx = index(newPos, oldIdx.column());
				changePersistentIndex(oldIdx, newIdx);
			}
		}

		Q_EMIT layoutChanged({}, QAbstractTableModel::VerticalSortHint);
	}

	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::AddColumns(int oldColumnsCount, int newColumnsCount)
	{
		// на данный момент колонок меньше не становится
		Q_ASSERT(newColumnsCount >= oldColumnsCount);
		if (newColumnsCount > oldColumnsCount)
		{
			beginInsertColumns({}, oldColumnsCount, newColumnsCount - 1);
			m_currentColumnCount = newColumnsCount;
			endInsertColumns();
		}
	}

	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::AddColumns()
	{
		AddColumns(m_currentColumnCount, ColumnCount());
	}

	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::OnBeforeUpsert(std::size_t newrecs_size)
	{
		m_updCtx = {}; // сбрасываем контекст
		AddExtraRows(newrecs_size);
		PrepareForUpdate();
	}

	template <class ViewType, class ModelBase>
	void ViewModelBase<ViewType, ModelBase>::OnAfterUpsert()
	{
		RecalcualtePersistentIndexes();
		RemoveExtraRows();
		AddColumns();
	}

} // namespace QtTools
