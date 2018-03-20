#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QAbstractListModel>
#include <QtCore/QMimeData>

#include <QtWidgets/QHeaderView>
#include <QtTools/ToolsBase.hpp>

#include <vector>
#include <algorithm>
#include <functional>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/global_fun.hpp>

#include <QtTools/BasicHeaderControlModelHelper.hqt>

namespace QtTools
{
	/// модель которая следит и контролирует QHeaderView
	/// есть 2 способа воздействия на QHeaderView:
	///   * прямой, когда пользователь на прямую двигает колонки в QHeaderView
	///     модель отслеживает данные изменения и синхронизирует внутреннее состояние
	///   * через данную модель по средством вызова moveRows и других.
	///     модель вызовет необходимые методы отслеживаемого QHeaderView,
	///     что бы привести отслеживаемый QHeaderView в соответствие.
	///
	/// видимость колонок представлена Qt::CheckStateRole
	/// модель идентифицирует колонки по кодам, это позволяет работать в ситуации когда:
	///   * таблица заполняется в фоне и колонки могут приходит не сразу, а в процессе загрузки с задержкой
	///   * мы хотим запоминать порядок колонок между сеансами приложения
	/// на практике данная модель может знать и учитывать порядок колонок о которых QHeaderView еще не знает,
	/// но может узнать в будущем когда придет соответствующее событие insertColumns
	/// 
	/// @Param SectionInfoTraits класс описывающий информацию о секциях.
	///        должен содержать типы:
	///          * section_type - тип информации о секции.
	///            Это должен быть класс/структура содержащий как минимум:
	///              QString code;
	///              int width;
	///              bool hidden;
	///            структура может содержать дополнительные поля по желанию,
	///            расширьте класс - для предоставления доступа к расширенной информации;
	///            класс должен быть как минимум default constructible, move constructible, move assignable, swappable
	///        должен предоставлять методы к полям section_info:
	///           const QString & get_code(const section_type & s)
	///           void            set_code(section_type & s, const QString & code)
	///            int get_width(const section_info & s)
	///           void set_width(section_info & s, int width)
	///           bool get_hidden(const section_info & s)
	///           void set_hidden(section_info & s, bool visible)
	///           
	///        Смотри пример в QtTools/HeaderSectionInfo.h
	///
	/// @Param BaseModel QAbstrctItemModel которую наследует данный класс, по-умолчанию QAbstractListModel
	/// 
	template <class SectionInfoTraits, class BaseModel = QAbstractListModel>
	class BasicHeaderControlModel : public BaseModel, protected SectionInfoTraits
	{
		typedef BaseModel               base_type;
		typedef BasicHeaderControlModel self_type;

	protected:
		typedef BasicHeaderControlModelHelper::CodeListMime CodeListMime;
		typedef SectionInfoTraits traits_type;
		typedef typename traits_type::section_type section_type;

		typedef QString code_type;
		typedef QtTools::QtHasher code_hasher;
		typedef std::equal_to<> code_equal;

		struct section_info
		{
			section_type info;
			int logicalIndex;

			static const int notpresent = -1;

		public:
			section_info(section_type && s, int logicalIndex = notpresent)
				: info(std::move(s)), logicalIndex(logicalIndex) {}

			section_info(const code_type & code, int logicalIndex = notpresent)
				: logicalIndex(logicalIndex)
			{ traits_type::set_code(info, code); }
		};

	protected:
		static const code_type & get_code(const section_info & s)                   { return traits_type::get_code(s.info); }
		static              void set_code(section_info & s, const code_type & code) { traits_type::set_code(s.info, code); }
		
		static  int get_width(const section_info & s)      { return traits_type::get_width(s.info); }
		static void set_width(section_info & s, int width) { traits_type::set_width(s.info, width); }

		static bool get_hidden(const section_info & s)         { return traits_type::get_hidden(s.info); }
		static void set_hidden(section_info & s, bool visible) { traits_type::set_hidden(s.info, visible); }

	protected:
		typedef boost::multi_index_container
		<
			section_info,
			boost::multi_index::indexed_by<
				boost::multi_index::random_access<>,
				boost::multi_index::hashed_unique<
					boost::multi_index::global_fun<const section_info &, const code_type &, self_type::get_code>
				>
			>
		> SectionContainer;

		static const int BySeq = 0;
		static const int ByCode = 1;

		typedef typename SectionContainer::template nth_index<BySeq>::type BySeqView;
		typedef typename SectionContainer::template nth_index<ByCode>::type ByCodeView;
		typedef typename SectionContainer::template nth_index_const_iterator<BySeq>::type BySeqViewConstIterator;
		typedef typename SectionContainer::template nth_index_const_iterator<ByCode>::type ByCodeViewConstIterator;

		struct IsPresentType
		{
			typedef bool result_type;
			result_type operator()(const section_info & s) const { return s.logicalIndex >= 0; }
			result_type operator()(const section_info * s) const { return s->logicalIndex >= 0; }
		};

		const IsPresentType IsPresent = IsPresentType();

		/// небольшой вспомогательный RAII класс для bool флажка
		class pass_slot_lock
		{
			bool & val;
		public:
			pass_slot_lock(bool & val) : val(val) { val = true; }
			~pass_slot_lock() { val = false; }
		};

		struct HelperImpl : BasicHeaderControlModelHelper
		{
			self_type * owner;
			HelperImpl(self_type * owner) : owner(owner) {}
			void OnSectionVisibleChangedHelper(int logicalIndex) override { owner->OnSectionVisibleChangedHelper(logicalIndex); }
		};

	protected:
		SectionContainer m_sectionContainer;
		QHeaderView * m_headerView = nullptr;
		QAbstractItemModel * m_headerModel = nullptr;
		bool m_pass_slot = false;
		
		int m_displayRole = Qt::DisplayRole;
		const int m_codeRole = Qt::DisplayRole;
		HelperImpl m_delayedHelper;

	protected:
		/// Есть 4 вида индексации:
		///  * code каждая колонка уникально идентифицируется неким строковым кодом
		///    задается traits классом, по умолчанию это DisplayRole модели QHeaderView::model()
		///  * внутренняя, это родная индексация для данной модели
		///  * логический индекс - это logical index QHeaderView
		///  * визуальный индекс - это visual index QHeaderView

		/// получает отображемый текст из headerModel соотвествующий Qt::DisplayRole
		virtual QString DisplayTextByIdx(int logicalIndex) const
		{ return m_headerModel->headerData(logicalIndex, Qt::Horizontal, m_displayRole).toString(); }

		virtual code_type CodeFromLogicalIndex(int logicalIndex) const
		{ return m_headerModel->headerData(logicalIndex, Qt::Horizontal, m_codeRole).toString(); }

		BySeqViewConstIterator ToSeqIterator(ByCodeViewConstIterator codeit) const
		{ return m_sectionContainer.template project<BySeq>(codeit); }
		
		/// получаем внутренний индекс по итератору
		int IndexFromIt(BySeqViewConstIterator seqit) const
		{ return seqit - m_sectionContainer.begin(); }
		
		/// получает визуальный индекс по коду
		int VisualIndexFromIt(BySeqViewConstIterator seqit) const;

	public:
		/// получает внутренний индекс по визуальному индексу QHeaderView
		/// NOTE: спрятанные секции не исчезают из нумерации
		int VisualIndexToIndex(int visualIndex) const;
		/// получает визуальный индекс отслеживаемой QHeaderView по внутреннему
		/// (если отслеживаемой модели нет - это индекс по порядку)
		/// NOTE: спрятанные секции не исчезают из нумерации
		int VisualIndexFromIndex(int internalIndex) const;

	protected: //core methods
		/// перемещает колонку идентифицируемую итератором в destinationChild
		/// уведомляет QHeaderView(вызывает SyncSectionPos), если отслеживается
		/// destinationChild - номер строки, перед которой вставляются элемент, как в QAbstractItemModel::moveRows
		void MoveSection(BySeqViewConstIterator it, int destinationChild);
		/// устанавливает свойство hidden секции
		/// если notifyView - уведомляет QHeaderView
		void SetSectionHidden(BySeqViewConstIterator it, bool hidden, bool notifyView);
		/// устанавливает свойство width секции с кодом code
		/// если notifyView - уведомляет QHeaderView
		void SetSectionSize(BySeqViewConstIterator it, int newWidth, bool notifyView);

		/// перемещает колонку в отслеживаемой модели с oldVisualIndex перед newVisibleIndex
		/// newVisualIndex фактически становиться новым индексом
		/// NOTE: QHeaderView::moveSection интерпретирует аргументы немного по-другому.
		///       Данный метод производит коррекцию newVisualIndex перед вызовом moveSection.
		void SyncSectionPos(int oldVisualIndex, int newVisualIndex);

		/// перемещает строки внутри модели, не генерирует вызовы QHeaderView,
		/// может использоваться для обработки сигналов от QHeaderView
		/// destinationChild - номер строки, перед которой вставляются элементы, как в QAbstractItemModel::moveRows
		bool MoveInternalRows(int sourceRow, int count, int destinationChild);
		/// перемещает строки внутри модели и уведомляет QHeaderView, фактически реализация moveRows
		bool MoveRows(int sourceRow, int count, int destinationChild);
		/// выполняет перемещение колонок, учитывает есть ли отслеживаемый QHeaderView,
		/// sections - указатели в m_sectionContainer на перемещаемые секции
		/// destinationChild - номер строки, перед которой вставляются элементы, как в QAbstractItemModel::moveRows
		void MoveSections(const std::vector<const void *> & sections, int destinationChild);

		/// заполняет секцию данными из QHeaderView
		void InitSectionFromHeader(section_info & info, int logicalIndex);
		/// добавляет секцию info к QHeaderView,
		/// если такая секция уже была(с таким кодом) она будет сконфигурирована новой информацией
		void AssignSection(section_info && info);

	public:
		// model support
		int rowCount(const QModelIndex & parent = QModelIndex()) const override;
		Qt::ItemFlags flags(const QModelIndex & index) const override;
		QVariant data(const QModelIndex & idx, int role) const override;
		/// поддерживается только Qt::CheckStateRole, который отвечает за видимость колонок
		/// если есть отслеживаемый QHeaderView - генерирует вызовы setSectionHidden
		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;

		/// перемещает строки, если есть отслеживаемый QHeaderView - генерирует вызовы sectionMoved.
		/// таким образом синхронизирует QHeaderView с внутренним состоянием
		bool moveRows(const QModelIndex & sourceParent, int sourceRow, int count,
		              const QModelIndex & destinationParent, int destinationChild) override;

		// drag&drop support
		// поддерживается только внутреннее перемещение, тип mime - implementation defined
		Qt::DropActions supportedDropActions() const override;
		Qt::DropActions supportedDragActions() const override;
		QStringList mimeTypes() const override;
		QMimeData * mimeData(const QModelIndexList & indexes) const override;

		bool canDropMimeData(const QMimeData * data, Qt::DropAction action,
		                     int row, int column, const QModelIndex & parent) const override;
		bool dropMimeData(const QMimeData * data, Qt::DropAction action,
		                  int row, int column, const QModelIndex & parent) override;

	protected: //QHeaderView monitor
		// вызываются как реакция на сигналы QHeaderView
		void OnSectionMoved(const code_type & code, int oldVisibleIndex, int newVisibleIndex);
		void OnSectionVisibleChanged(const code_type & code, bool hidden);
		void OnSectionSizeChanged(const code_type & code, int newWidth);

		void OnSectionMoved(int logicalIndex, int oldVisibleIndex, int newVisibleIndex);
		void OnSectionSizeChanged(int logicalIndex, int oldSize, int newWidth);
		// смотри реализацию OnSectionSizeChanged
		void OnSectionVisibleChangedHelper(int logicalIndex);

		void OnColumnsInserted(const QModelIndex & parent, int start, int end);
		void OnColumnsRemoved(const QModelIndex & parent, int start, int end);
		//void OnColumnsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd,
		//                    const QModelIndex & destinationParent, int destinationColumn);
		void OnHeaderDataChanged(Qt::Orientation orientation, int first, int last);

	protected: //QHeaderView configuration block
		/// соединяет сигналы QHeaderView со слотами данного класса
		void ConnectHeader();
		/// отсоединяет сигналы QHeaderView от слотов данного класса
		void DisconnectHeader();
		/// устанавливает QHeaderView для отслеживание, настраивает мониторинг
		/// если header == nullptr - просто синаем отслеживание
		void Assign(QHeaderView * header);
		/// заполняет внутренние структуры, из QHeaderView
		/// выставляет логические индексы, не меняет порядок
		void SynchronizeInternalsWithHeader();

	public:
		QString GetCode(int index) const;
		QString GetDisplayText(int index) const;
		bool IsVisible(int index) const;
		int GetWidth(int index) const;

		QString GetCode(const QModelIndex & idx) const { return GetCode(idx.row()); }
		QString GetDisplayText(const QModelIndex & idx) const { return GetDisplayText(idx.row()); }
		bool IsVisible(const QModelIndex & idx) const { return IsVisible(idx.row()); }
		int GetWidth(const QModelIndex & idx) const { return GetWidth(idx.row()); }

		void SetVisible(int index, bool visible);
		void SetWidth(int index, int newWidth);

		int  CodeRole() const { return m_codeRole; }
		int  DisplayRole() const { return m_displayRole; }
		void SetDisplayRole(int newDisplayRole) { m_displayRole = newDisplayRole; }

		/// устанавливает QHeaderView для отслеживание,
		/// предыдущее отслеживание снимается,
		/// если header == nullptr - просто синаем отслеживание
		/// если есть загруженная конфигурация - она имеет приоритет перед конфигурацией QHeaderView
		/// и последний будет переконфигурирован
		void Track(QHeaderView * header);
		/// загружает конфигурацию из заданного списка, и настраивает отслеживаемый QHeaderView
		/// (если отслеживаемого еще нет - конфигурация сохраняется и будет применена с вызовом Track)
		template <class SectionsRange>
		void Configurate(SectionsRange && sections);
		/// проверяет что порядок колонок в отслеживаемой модели и конфигурация одинаков.
		/// при проверке не существующие колонки в отслеживаемой модели не влияют на результат
		bool IsNaturalOrder() const;
		/// сохраняет конфигурацию в список
		std::vector<section_type> SaveConfiguration() const;
		/// сбрасывает конфигурацию, в том числе и в отслеживаемом QHeaderView
		/// выставляя в последнем натуральный порядок из модели QHeaderView
		/// Если нет отслеживаемого QHeaderView - просто сбрасывает всю конфигурацию
		void Reset();
		/// удаляет из конфигурации колонки, которых нет в QHeaderView, последний фактически не меняется
		void EraseNonPresent();

		/// ctors
		/// codeRole is DisplayRole
		BasicHeaderControlModel(QObject * parent = nullptr)
			: base_type(parent), m_delayedHelper(this) { }

		BasicHeaderControlModel(int codeRole, QObject * parent = nullptr)
			: base_type(parent), m_codeRole(codeRole), m_delayedHelper(this) { }
	};

	/************************************************************************/
	/*                    Index methods                                     */
	/************************************************************************/
	template <class SectionInfoTraits, class BaseModel>
	int BasicHeaderControlModel<SectionInfoTraits, BaseModel>::VisualIndexFromIt(BySeqViewConstIterator seqit) const
	{
		auto first = m_sectionContainer.begin();
		int count = 0;
		for (; first != seqit; ++first)
			if (IsPresent(*first)) ++count;
		return count;
	}

	template <class SectionInfoTraits, class BaseModel>
	int BasicHeaderControlModel<SectionInfoTraits, BaseModel>::VisualIndexToIndex(int visualIndex) const
	{
		auto first = m_sectionContainer.begin();
		auto it = first;
		for (; visualIndex; ++it)
			if (IsPresent(*it)) --visualIndex;

		return qint(it - first);
	}

	template <class SectionInfoTraits, class BaseModel>
	int BasicHeaderControlModel<SectionInfoTraits, BaseModel>::VisualIndexFromIndex(int internalIndex) const
	{
		auto first = m_sectionContainer.begin();
		auto last = first + internalIndex;
		int visualIndex = 0;
		for (; first != last; ++first)
			if (IsPresent(*first)) ++visualIndex;

		return visualIndex;
	}

	/************************************************************************/
	/*                    Core methods                                      */
	/************************************************************************/
	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::MoveSection(BySeqViewConstIterator it, int destinationChild)
	{
		int sourceRow = IndexFromIt(it);
		MoveInternalRows(sourceRow, 1, destinationChild);

		if (m_headerView && IsPresent(*it))
		{
			int oldVisualIndex = m_headerView->visualIndex(it->logicalIndex);
			int newVisualIndex = VisualIndexFromIndex(destinationChild);
			SyncSectionPos(oldVisualIndex, newVisualIndex);
		}
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::SetSectionHidden(BySeqViewConstIterator it, bool hidden, bool notifyView)
	{
		section_info & s = const_cast<section_info &>(*it);
		set_hidden(s, hidden);

		if (notifyView && m_headerView && IsPresent(*it))
			m_headerView->setSectionHidden(it->logicalIndex, hidden);

		int pos = IndexFromIt(it);
		auto idx = index(pos);
		Q_EMIT dataChanged(idx, idx, {Qt::CheckStateRole});
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::SetSectionSize(BySeqViewConstIterator it, int newWidth, bool notifyView)
	{
		// 0 используется QHeaderView при сокрытии секций,
		// если мы будем выставлять в 0 - секция будет невидима, даже когда фактически должна быть
		if (newWidth <= 0) return;

		section_info & s = const_cast<section_info &>(*it);
		set_width(s, newWidth);

		if (notifyView && m_headerView && IsPresent(*it))
			m_headerView->resizeSection(it->logicalIndex, newWidth);

		int pos = IndexFromIt(it);
		auto idx = index(pos);
		Q_EMIT dataChanged(idx, idx);
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::SyncSectionPos(int oldVisualIndex, int newVisualIndex)
	{
		// коррекция индекса
		// связано с тем как интерпретируется аргументы QHeaderView::moveSection(from, to).
		// to должен быть индексом, как если, после выполнения операции.
		// т.е. если мы хотим переместить с 1го перед 5ым, то аргументы должны быть moveSection(1, 4),
		// поскольку после выполнения операции индекс станет равным 4
		if (newVisualIndex > oldVisualIndex)
			newVisualIndex -= 1;

		pass_slot_lock lk {m_pass_slot};
		m_headerView->moveSection(oldVisualIndex, newVisualIndex);
	}

	template <class SectionInfoTraits, class BaseModel>
	bool BasicHeaderControlModel<SectionInfoTraits, BaseModel>::MoveInternalRows(int sourceRow, int count, int destinationChild)
	{
		bool allowed = beginMoveRows({}, sourceRow, sourceRow + count - 1, {}, destinationChild);
		if (!allowed) return false;

		auto first = m_sectionContainer.begin() + sourceRow;
		auto last = first + count;
		auto dest = m_sectionContainer.begin() + destinationChild;
		m_sectionContainer.relocate(dest, first, last);

		endMoveRows();
		return true;
	}

	template <class SectionInfoTraits, class BaseModel>
	bool BasicHeaderControlModel<SectionInfoTraits, BaseModel>::MoveRows(int sourceRow, int count, int destinationChild)
	{
		if (!m_headerView)
			return MoveInternalRows(sourceRow, count, destinationChild);
		else
		{
			auto first = m_sectionContainer.begin() + sourceRow;
			auto last = m_sectionContainer.begin() + sourceRow + count;

			std::vector<const void *> sections;
			sections.resize(count);
			for (auto out = sections.begin(); first != last; ++first, ++out)
				*out = &*first;

			MoveSections(sections, destinationChild);
			return true;
		}
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::MoveSections(const std::vector<const void *> & sections, int destinationChild)
	{
		auto & seqview = m_sectionContainer.template get<BySeq>();
		for (const void * ptr : sections)
		{
			const section_info & seqref = *static_cast<const section_info *>(ptr);
			auto seqit = seqview.iterator_to(seqref);
			int sourceRow = IndexFromIt(seqit);

			MoveSection(seqit, destinationChild);

			// коррекция индекса, если сдвигаем перед sourceRow,
			// то destinationChild сдвигается на 1 элемент
			if (destinationChild < sourceRow)
				destinationChild += 1;
		}
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::InitSectionFromHeader(section_info & s, int logicalIndex)
	{
		s.logicalIndex = logicalIndex;
		set_hidden(s, m_headerView->isSectionHidden(logicalIndex));
		set_width(s, m_headerView->sectionSize(logicalIndex));
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::AssignSection(section_info && s)
	{
		int li = s.logicalIndex;
		auto & codeview = m_sectionContainer.template get<ByCode>();

		bool inserted;
		ByCodeViewConstIterator it;
		std::tie(it, inserted) = codeview.insert(std::move(s));
		auto seqit = ToSeqIterator(it);

		if (inserted)
		{
			int pos = IndexFromIt(seqit);
			beginInsertRows({}, pos, pos);
			endInsertRows();
		}
		else
		{
			const_cast<section_info &>(*it).logicalIndex = li;
			SetSectionSize(seqit, get_width(*seqit), true);
			SetSectionHidden(seqit, get_hidden(*seqit), true);

			if (m_headerView)
			{
				// синхронизируем порядок
				int oldVi = m_headerView->visualIndex(seqit->logicalIndex);
				int newVi = VisualIndexFromIt(seqit);
				SyncSectionPos(oldVi, newVi);
			}
		}
	}

	/************************************************************************/
	/*                      model support                                   */
	/************************************************************************/
	template <class SectionInfoTraits, class BaseModel>
	int BasicHeaderControlModel<SectionInfoTraits, BaseModel>::rowCount(const QModelIndex & parent /* = QModelIndex() */) const
	{
		return qint(m_sectionContainer.size());
	}

	template <class SectionInfoTraits, class BaseModel>
	Qt::ItemFlags BasicHeaderControlModel<SectionInfoTraits, BaseModel>::flags(const QModelIndex & index) const
	{
		auto defFlags = base_type::flags(index) | Qt::ItemIsUserCheckable;
		// не валидный индекс - это позиция между строками
		// ее делаем ItemIsDropEnabled что бы можно было скидывать
		// валидные же индексы только ItemIsDragEnabled
		return defFlags | (index.isValid() ? Qt::ItemIsDragEnabled : Qt::ItemIsDropEnabled);
	}

	template <class SectionInfoTraits, class BaseModel>
	QVariant BasicHeaderControlModel<SectionInfoTraits, BaseModel>::data(const QModelIndex & idx, int role) const
	{
		std::size_t row = idx.row();
		const auto & s = m_sectionContainer.at(row);

		switch (role)
		{
			case Qt::ToolTipRole:
			case Qt::DisplayRole:
				return IsPresent(s) ?
			           QVariant::fromValue(DisplayTextByIdx(s.logicalIndex)) :
			           QVariant::fromValue(get_code(s));
		
			case Qt::CheckStateRole: return get_hidden(s) ? Qt::Unchecked : Qt::Checked;
			case Qt::TextColorRole:  return IsPresent(s) ? QVariant {} : QColor {Qt::red};
		
			default:
				return QVariant {};
		}
	}

	template <class SectionInfoTraits, class BaseModel>
	bool BasicHeaderControlModel<SectionInfoTraits, BaseModel>::setData(const QModelIndex & index, const QVariant & value, int role /* = Qt::EditRole */)
	{
		bool invalid = role != Qt::CheckStateRole ||
			!index.isValid() ||
			!value.canConvert<int>();

		if (invalid)
			return false;

		auto it = m_sectionContainer.begin() + index.row();
		auto state = static_cast<Qt::CheckState>(value.toInt());
		bool hidden = state == Qt::Unchecked;

		SetSectionHidden(it, hidden, true);
		return true;
	}

	template <class SectionInfoTraits, class BaseModel>
	bool BasicHeaderControlModel<SectionInfoTraits, BaseModel>::moveRows(const QModelIndex & sourceParent, int sourceRow, int count,
	                                                                     const QModelIndex & destinationParent, int destinationChild)
	{
		return MoveRows(sourceRow, count, destinationChild);
	}

	/************************************************************************/
	/*                       drag n drop                                    */
	/************************************************************************/
	template <class SectionInfoTraits, class BaseModel>
	Qt::DropActions BasicHeaderControlModel<SectionInfoTraits, BaseModel>::supportedDropActions() const
	{
		return Qt::MoveAction;
	}

	template <class SectionInfoTraits, class BaseModel>
	Qt::DropActions BasicHeaderControlModel<SectionInfoTraits, BaseModel>::supportedDragActions() const
	{
		return Qt::MoveAction;
	}

	template <class SectionInfoTraits, class BaseModel>
	QStringList BasicHeaderControlModel<SectionInfoTraits, BaseModel>::mimeTypes() const
	{
		return CodeListMime::MimeFormats;
	}

	template <class SectionInfoTraits, class BaseModel>
	QMimeData * BasicHeaderControlModel<SectionInfoTraits, BaseModel>::mimeData(const QModelIndexList & indexes) const
	{
		if (indexes.count() <= 0)
			return nullptr;
		
		QScopedPointer<CodeListMime> data {new CodeListMime};
		data->model = this;
		auto & sections = data->sections;

		sections.reserve(indexes.size());
		for (const QModelIndex & idx : indexes)
			sections.push_back(&m_sectionContainer[idx.row()]);
		
		return data.take();
	}

	template <class SectionInfoTraits, class BaseModel>
	bool BasicHeaderControlModel<SectionInfoTraits, BaseModel>::canDropMimeData(const QMimeData * data, Qt::DropAction action,
	                                                                            int row, int column, const QModelIndex & parent) const
	{
		const CodeListMime * cmime;
		return action == Qt::MoveAction &&
			(cmime = qobject_cast<const CodeListMime *>(data)) &&
			cmime->model == this; // allow only internal move
	}

	template <class SectionInfoTraits, class BaseModel>
	bool BasicHeaderControlModel<SectionInfoTraits, BaseModel>::dropMimeData(const QMimeData * data, Qt::DropAction action,
	                                                                         int row, int column, const QModelIndex & parent)
	{
		// в нашем случае parent всегда invalid, поскольку мы не TreeModel
		// если row >= 0 - row указывает что скидывание произошло строго перед элементом с индексом row, для заданного parent'а
		// если же row == -1 мы:
		//  * или скидываем на элемент и тогда parent его идентифицирует,
		//     в нашем случае невозможно, поскольку мы запретили это методом flags
		//  * или скидываем после последнего элемента
		const std::vector<const void *> & sections = qobject_cast<const CodeListMime *>(data)->sections;

		if (row == -1) // row == -1 если мы скидываем ниже последнего item'а
			row = rowCount();
		
		MoveSections(sections, row);
		return true;
	}

	/************************************************************************/
	/*                 QHeaderView monitor                                  */
	/************************************************************************/
	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::OnSectionMoved(const code_type & code, int oldVisibleIndex, int newVisibleIndex)
	{
		int source = VisualIndexToIndex(oldVisibleIndex);
		int dest = VisualIndexToIndex(newVisibleIndex);
		if (dest > source) ++dest; //коррекция индекса

		MoveInternalRows(source, 1, dest);
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::OnSectionVisibleChanged(const code_type & code, bool hidden)
	{
		auto & codeview = m_sectionContainer.template get<ByCode>();
		auto it = codeview.find(code);
		SetSectionHidden(ToSeqIterator(it), hidden, false);
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::OnSectionSizeChanged(const code_type & code, int newWidth)
	{
		auto & codeview = m_sectionContainer.template get<ByCode>();
		auto it = codeview.find(code);
		SetSectionSize(ToSeqIterator(it), newWidth, false);
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::OnSectionMoved(int logicalIndex, int oldVisibleIndex, int newVisibleIndex)
	{
		if (m_pass_slot)
			return;

		code_type code = CodeFromLogicalIndex(logicalIndex);
		OnSectionMoved(code, oldVisibleIndex, newVisibleIndex);
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::OnSectionSizeChanged(int logicalIndex, int oldSize, int newWidth)
	{
		code_type code = CodeFromLogicalIndex(logicalIndex);
		OnSectionSizeChanged(code, newWidth);

		/// отложенный вызов OnSectionVisibleChangedHelper
		/// на момент вызова sectionResized, QHeaderView еще не выставляет флаг о том что колонка скрыта
		/// делая отложенный вызов мы можем проверить флаг, после того как он будет выставлен внутри QHeaderView
		if (newWidth == 0)
		{
			QMetaObject::invokeMethod(&m_delayedHelper, "OnSectionVisibleChangedHelper", Qt::QueuedConnection,
			                          Q_ARG(int, logicalIndex));
		}
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::OnSectionVisibleChangedHelper(int logicalIndex)
	{
		code_type code = CodeFromLogicalIndex(logicalIndex);
		bool hidden = m_headerView->isSectionHidden(logicalIndex);
		OnSectionVisibleChanged(code, hidden);
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::OnColumnsInserted(const QModelIndex & parent, int start, int end)
	{
		/// что бы корректно обработать данный сигнал нужно, в дополнение к random_access visual index, иметь random_access logical index,
		/// но мы этого не делаем - поэтому будем делать пересинхронизацию
		SynchronizeInternalsWithHeader();
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::OnColumnsRemoved(const QModelIndex & parent, int start, int end)
	{
		/// что бы корректно обработать данный сигнал нужно, в дополнение к random_access visual index, иметь random_access logical index,
		/// но мы этого не делаем - поэтому будем делать пересинхронизацию
		SynchronizeInternalsWithHeader();
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::OnHeaderDataChanged(Qt::Orientation orientation, int first, int last)
	{
		if (orientation != Qt::Horizontal)
			return;

		for (int i = first; i <= last; ++i)
		{
			auto code = CodeFromLogicalIndex(i);
			if (code.isEmpty()) // такое бывает %)
				continue;

			section_info info(code);
			InitSectionFromHeader(info, i);

			auto first = m_sectionContainer.begin();
			auto last = m_sectionContainer.end();
			auto seqit = std::find_if(first, last, [i](const section_info & inf) { return inf.logicalIndex == i; });
			BOOST_ASSERT(seqit != last);

			if (get_code(*seqit) == get_code(info))
				continue;

			// update now
			bool replaced = m_sectionContainer.replace(seqit, info);
			if (replaced)
			{
				// заменить удалось, имя изменилось на какое-то, которое нам еще не известно
				int idx = seqit - first;
				auto modelIndex = index(idx);
				Q_EMIT dataChanged(modelIndex, modelIndex);
			}
			else
			{
				// заменить не удалось, имя изменилось на какое-то, которое нам уже известно
				// удаляем старый элемент
				int row = seqit - first;
				beginRemoveRows({}, row, row);
				m_sectionContainer.erase(seqit);
				endRemoveRows();
				// устанавливаем секцию и синхронизируем View
				AssignSection(std::move(info));
			}
		}
	}

	/************************************************************************/
	/*                      configuration block                             */
	/************************************************************************/
	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::ConnectHeader()
	{
		connect(m_headerView, &QHeaderView::sectionMoved,
				this, static_cast<void (self_type::*)(int, int, int)>(&self_type::OnSectionMoved));
		connect(m_headerView, &QHeaderView::sectionResized,
				this, static_cast<void (self_type::*)(int, int, int)>(&self_type::OnSectionSizeChanged));

		connect(m_headerModel, &QAbstractItemModel::columnsInserted,   this, &self_type::OnColumnsInserted);
		connect(m_headerModel, &QAbstractItemModel::columnsRemoved,    this, &self_type::OnColumnsRemoved);
		connect(m_headerModel, &QAbstractItemModel::headerDataChanged, this, &self_type::OnHeaderDataChanged);
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::DisconnectHeader()
	{
		if (m_headerView) m_headerView->disconnect(this);
		if (m_headerModel) m_headerModel->disconnect(this);
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::Assign(QHeaderView * header)
	{
		DisconnectHeader();
		m_headerView = header;
		if (header == nullptr)
			m_headerModel = nullptr;
		else
		{
			m_headerModel = m_headerView->model();
			ConnectHeader();
		}
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::SynchronizeInternalsWithHeader()
	{
		BOOST_ASSERT(m_headerView);
		BOOST_ASSERT(m_headerModel);

		/// сбрасываем logicalIndex bindings
		for (const section_info & info : m_sectionContainer)
		{
			const_cast<section_info &>(info).logicalIndex = section_info::notpresent;
		}

		auto count = m_headerModel->columnCount();
		auto oldSz = m_sectionContainer.size();
		
		/// сначала заполняем внутреннее состояние из QHeaderView
		for (int i = 0; i < count; ++i)
		{
			auto code = CodeFromLogicalIndex(i);
			section_info info(code);
			InitSectionFromHeader(info, i);
			AssignSection(std::move(info));
		}

		/// нарушение этого assert'а говорит, что в headerModel 2 колонки имеет одинаковый код
		BOOST_ASSERT(m_sectionContainer.size() >= qsizet(count));

		/// выставляем внутренний порядок в соответствии с QHeaderView для новых элементов
		std::vector<std::reference_wrapper<const section_info>> tmpview;
		tmpview.assign(m_sectionContainer.begin(), m_sectionContainer.end());

		std::sort(tmpview.begin() + oldSz, tmpview.end(),
		          [this](const section_info & op1, const section_info & op2)
		{
			int vi1 = m_headerView->visualIndex(op1.logicalIndex);
			int vi2 = m_headerView->visualIndex(op2.logicalIndex);
			return vi1 < vi2;
		});

		m_sectionContainer.rearrange(tmpview.begin());
	}

	template <class SectionInfoTraits, class BaseModel>
	QString BasicHeaderControlModel<SectionInfoTraits, BaseModel>::GetCode(int index) const
	{
		auto i = qsizet(index);
		return i < m_sectionContainer.size() ? m_sectionContainer[i].code : QString::null;
	}

	template <class SectionInfoTraits, class BaseModel>
	QString BasicHeaderControlModel<SectionInfoTraits, BaseModel>::GetDisplayText(int index) const
	{
		auto i = qsizet(index);
		if (i >= m_sectionContainer.size()) return QString::null;
		auto & info = m_sectionContainer[i];
		return IsPresent(info) ? DisplayTextByIdx(info.logicalIndex) : info.code;
	}

	template <class SectionInfoTraits, class BaseModel>
	bool BasicHeaderControlModel<SectionInfoTraits, BaseModel>::IsVisible(int index) const
	{
		auto i = qsizet(index);
		return i < m_sectionContainer.size() ? !m_sectionContainer[i].hidden : false;
	}

	template <class SectionInfoTraits, class BaseModel>
	int BasicHeaderControlModel<SectionInfoTraits, BaseModel>::GetWidth(int index) const
	{
		auto i = qsizet(index);
		return i < m_sectionContainer.size() ? m_sectionContainer[i].width : 0;
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::SetVisible(int index, bool visible)
	{
		auto i = qsizet(index);
		if (i < m_sectionContainer.size())
			return;

		SetSectionHidden(m_sectionContainer.begin() + i, !visible, true);
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::SetWidth(int index, int newWidth)
	{
		auto i = qsizet(index);
		if (i < m_sectionContainer.size())
			return;

		SetSectionSize(m_sectionContainer.begin() + i, newWidth, true);
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::Track(QHeaderView * header)
	{
		Assign(header);
		if (header) SynchronizeInternalsWithHeader();
	}

	template <class SectionInfoTraits, class BaseModel>
	template <class SectionsRange>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::Configurate(SectionsRange && sections)
	{
		beginResetModel();

		auto & codeview = m_sectionContainer.template get<ByCode>();

		int pos = 0;
		for (auto && s : sections)
		{
			bool inserted;
			ByCodeViewConstIterator it;
			std::tie(it, inserted) = codeview.emplace(std::move(s));
			auto seqIt = ToSeqIterator(it);

			if (!inserted)
			{
				// данная секция уже была, настраиваем ее
				SetSectionHidden(seqIt, traits_type::get_hidden(s), true);
				SetSectionSize(seqIt, traits_type::get_width(s), true);
			}

			MoveSection(seqIt, pos);
			++pos;
		}

		endResetModel();
	}

	template <class SectionInfoTraits, class BaseModel>
	bool BasicHeaderControlModel<SectionInfoTraits, BaseModel>::IsNaturalOrder() const
	{
		auto & seqview = m_sectionContainer.template get<BySeq>();
		int curIdx = 0;

		for (const auto & info : seqview)
		{
			if (info.logicalIndex == notpresent)
				continue;
			else if (info.logicalIndex != curIdx++)
				return false;
		}

		return true;
	}

	template <class SectionInfoTraits, class BaseModel>
	auto BasicHeaderControlModel<SectionInfoTraits, BaseModel>::SaveConfiguration() const -> std::vector<section_type>
	{
		std::vector<section_type> list;
		list.reserve(m_sectionContainer.size());
		auto first = m_sectionContainer.begin();
		auto last = m_sectionContainer.end();
		std::transform(first, last, std::back_inserter(list), std::mem_fn(&section_info::info));
		return list;
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::Reset()
	{
		if (m_headerView == nullptr)
		{
			beginResetModel();
			m_sectionContainer.clear();
			endResetModel();
		}
		else
		{
			beginResetModel();
			if (m_headerView)
			{
				m_headerView->reset();
				///m_headerView->reset() - не сбрасывает порядок... сволота, делаем руками
				int count = m_headerView->count();
				for (int i = 0; i < count; ++i)
				{
					int oldVi = m_headerView->visualIndex(i);
					m_headerView->moveSection(oldVi, i);
				}
			}
			endResetModel();
		}
	}

	template <class SectionInfoTraits, class BaseModel>
	void BasicHeaderControlModel<SectionInfoTraits, BaseModel>::EraseNonPresent()
	{
		beginResetModel();
		for (auto it = m_sectionContainer.begin(); it != m_sectionContainer.end(); )
		{
			if (!IsPresent(*it))
				it = m_sectionContainer.erase(it);
			else
				++it;
		}
		endResetModel();
	}
}
