#include <QtTools/MdiModel.hqt>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QMdiArea>
#include <QtCore/QStringBuilder>
#include <QtCore/QDebug>

namespace QtTools
{
	void MdiModel::AddAction(QMdiSubWindow * subwindow)
	{
		auto idx = m_actions.size();

		QString text = '&' % QString::number(idx + 1) % ' ' % subwindow->windowTitle();
		auto * action = m_menu->addAction(text);
		m_actions.append(action);


		connect(action, &QAction::triggered,
				[this, subwindow] { m_mdiArea->setActiveSubWindow(subwindow); });

		connect(subwindow, &QWidget::windowTitleChanged, this, &MdiModel::SynchronizeMdiWindowTitle);
				//[this, action](const QString & title) { return SynchronizeMdiWindowTitle(action, title); });

		action->setIcon(subwindow->windowIcon());
		action->setIconText(subwindow->windowIconText());

		connect(subwindow, &QWidget::windowIconChanged, action, &QAction::setIcon);
		connect(subwindow, &QWidget::windowIconTextChanged, action, &QAction::setIconText);
	}

	void MdiModel::UnregisterMdiWindow()
	{
		auto * subwindow = static_cast<QMdiSubWindow *>(QObject::sender());
		auto idx = m_subwindows.indexOf(subwindow);

		beginRemoveRows({}, idx, idx);
		m_subwindows.removeAt(idx);
		endRemoveRows();

		if (m_menu)
		{
			delete m_actions.takeAt(idx);
			if (m_addSeparator && m_actions.empty())
			{
				delete m_separator;
				m_separator = nullptr;
			}

			/// подправляем числа
			int sz = m_actions.size();
			for (; idx < sz; ++idx)
			{
				auto * action = m_actions[idx];
				QString text = action->text();

				// генерируемый нами текст начинается с &<number><пробел>
				auto start = text.indexOf(' ');
				text = '&' % QString::number(idx + 1) % text.midRef(start);
				action->setText(text);
			}
		}
	}

	void MdiModel::SynchronizeMdiWindowTitle(const QString & title)
	{
		auto * subwindow = static_cast<QMdiSubWindow *>(QObject::sender());
		auto pos = m_subwindows.indexOf(subwindow);
		
		auto idx = index(pos);
		dataChanged(idx, idx);

		auto action = m_actions[pos];
		auto text = action->text();

		// генерируемый нами текст начинается с &<number><пробел>
		auto start = text.indexOf(' ');
		text = text.midRef(0, start) % ' ' % title;
		action->setText(text);
	}

	void MdiModel::RegisterMdiWindow(QMdiSubWindow * window)
	{
		if (not window) return;

		auto pos = m_subwindows.size() + 1;
		beginInsertRows({}, pos, pos);

		m_subwindows.append(window);
		connect(window, &QMdiSubWindow::destroyed, this, &MdiModel::UnregisterMdiWindow);
				//[this, window] { UnregisterMdiWindow(window); });

		endInsertRows();

		if (m_menu)
		{
			if (m_addSeparator && m_actions.empty())
				m_separator = m_menu->addSeparator();

			AddAction(window);
		}
	}

	QMdiSubWindow * MdiModel::AddSubWindow(QWidget * wgt, Qt::WindowFlags flags /* = 0 */)
	{
		auto * subwindow = m_mdiArea->addSubWindow(wgt, flags);
		RegisterMdiWindow(subwindow);
		return subwindow;
	}

	QMdiSubWindow * MdiModel::GetSubWindow(int idx) const 
	{
		return m_subwindows[idx];
	}

	QAction * MdiModel::GetAction(int idx) const
	{
		if (m_menu)
			return m_actions[idx];
		else
			return nullptr;
	}

	void MdiModel::TrackMenu(QMenu * menu)
	{
		DetachMenu();

		m_menu = menu;
		if (m_menu)
		{
			if (m_addSeparator) 
				m_separator = m_menu->addSeparator();

			for (auto * subwindow : m_subwindows)
				AddAction(subwindow);
		}
	}

	void MdiModel::DetachMenu()
	{
		qDeleteAll(m_actions);
		m_actions.clear();

		delete m_separator;
		m_separator = nullptr;
		m_menu = nullptr;
	}

	int MdiModel::rowCount(const QModelIndex & parent /* = QModelIndex() */) const
	{
		return m_subwindows.size();
	}

	QVariant MdiModel::data(const QModelIndex & idx, int role /* = Qt::DisplayRole */) const
	{
		if (!idx.isValid())
			return {};

		switch (role)
		{
			case Qt::DisplayRole:
				return m_subwindows[idx.row()]->windowTitle();
			default:
				return {};
		}
	}

	QVariant MdiModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal)
			return QVariant {};

		switch (role)
		{
			case Qt::DisplayRole:
				return m_colTitle;
			default:
				return QVariant {};
		}
	}

	MdiModel::MdiModel(QMdiArea & mdiArea, QMenu * menu, QObject * parent /* = nullptr */)
		: QAbstractListModel(parent)
	{
		m_mdiArea = &mdiArea;
		m_menu = menu;

		m_colTitle = tr("window title");
	}

	MdiModel::~MdiModel() Q_DECL_NOEXCEPT
	{
		qDeleteAll(m_actions);
	}
}