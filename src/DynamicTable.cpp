#include <QtTools/DynamicTable.hqt>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTableView>
#include <QtWidgets/QPushButton>
#include <QtGui/QKeyEvent>

namespace QtTools
{
	void DynamicTable::OnUpItem()
	{
		auto idx = m_view->selectionModel()->currentIndex();
		if (!idx.isValid() || idx.row() == 0)
			return;

		m_model->moveRow(idx.parent(), idx.row(), idx.parent(), idx.row() - 1);
	}

	void DynamicTable::OnDownItem()
	{
		auto idx = m_view->selectionModel()->currentIndex();
		if (!idx.isValid() || idx.row() == m_model->rowCount() - 1)
			return;

		m_model->moveRow(idx.parent(), idx.row(), idx.parent(), idx.row() + 2);
	}

	void DynamicTable::OnNewItem()
	{
		auto rc = m_model->rowCount();
		m_model->insertRow(rc);
	}

	void DynamicTable::OnDeleteItem()
	{
		auto idx = m_view->selectionModel()->currentIndex();
		if (!idx.isValid()) return;

		m_model->removeRow(idx.row());
	}

	void DynamicTable::OnDialogButtonClick(QAbstractButton * button)
	{
		if (m_buttonBox->standardButton(button) == QDialogButtonBox::Reset)
			Q_EMIT Reset();
		else if (m_buttonBox->standardButton(button) == QDialogButtonBox::Apply)
			Q_EMIT Apply();
		else if (m_buttonBox->standardButton(button) == QDialogButtonBox::Ok) {
			Q_EMIT Apply();
			close();
			Q_EMIT Closed();
		}
		else if (m_buttonBox->standardButton(button) == QDialogButtonBox::Close) {
			//ResetAction();
			close();
			Q_EMIT Closed();
		}
	}

	void DynamicTable::Init()
	{
		setupUi();
		retranslateUi();
		connectSignals();
	}

	DynamicTable::DynamicTable(QWidget * parent /* = nullptr */)
		: QWidget(parent)
	{

	}

	DynamicTable::DynamicTable(QAbstractItemModel * model, QWidget * parent /* = nullptr */)
		: QWidget(parent)
	{
		m_model = model;
		m_view = new QTableView(this);
		m_view->setModel(m_model);
		Init();
	}

	void DynamicTable::connectSignals()
	{
		connect(m_buttonBox, &QDialogButtonBox::clicked, this, &DynamicTable::OnDialogButtonClick);
		connect(m_newEntryButton, &QToolButton::clicked, this, &DynamicTable::OnNewItem);
		connect(m_deleteEntryButton, &QToolButton::clicked, this, &DynamicTable::OnDeleteItem);
		connect(m_upEntryButton, &QToolButton::clicked, this, &DynamicTable::OnUpItem);
		connect(m_downEntryButton, &QToolButton::clicked, this, &DynamicTable::OnDownItem);
	}

	void DynamicTable::setupUi()
	{
		if (objectName().isEmpty())
			setObjectName(QStringLiteral("DynamicTableBase"));

		// main layout
		m_verticalLayout = new QVBoxLayout(this);
		m_verticalLayout->setObjectName(QStringLiteral("verticalLayout"));

		{// create toolbar, spacer with 4 buttons
			m_horizontalLayout = new QHBoxLayout();
			m_horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
			m_verticalLayout->addLayout(m_horizontalLayout);

			m_horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
			m_horizontalLayout->addItem(m_horizontalSpacer);

			// buttons
			m_newEntryButton = new QToolButton(this);
			m_newEntryButton->setObjectName(QStringLiteral("newEntryButton"));
			m_deleteEntryButton = new QToolButton(this);
			m_deleteEntryButton->setObjectName(QStringLiteral("deleteEntryButton"));
			m_upEntryButton = new QToolButton(this);
			m_upEntryButton->setObjectName(QStringLiteral("upEntryButton"));
			m_downEntryButton = new QToolButton(this);
			m_downEntryButton->setObjectName(QStringLiteral("downEntryButton"));

			//set icons
			QIcon icon;
			icon.addFile(QStringLiteral(":/icons/new_item.ico"), QSize(), QIcon::Normal, QIcon::Off);
			m_newEntryButton->setIcon(icon);
			QIcon icon1;
			icon1.addFile(QStringLiteral(":/icons/edit_delete.ico"), QSize(), QIcon::Normal, QIcon::Off);
			m_deleteEntryButton->setIcon(icon1);
			QIcon icon2;
			icon2.addFile(QStringLiteral(":/icons/arrow_up.ico"), QSize(), QIcon::Normal, QIcon::Off);
			m_upEntryButton->setIcon(icon2);
			QIcon icon3;
			icon3.addFile(QStringLiteral(":/icons/arrow_down.ico"), QSize(), QIcon::Normal, QIcon::Off);
			m_downEntryButton->setIcon(icon3);

			m_horizontalLayout->addWidget(m_newEntryButton);
			m_horizontalLayout->addWidget(m_deleteEntryButton);
			m_horizontalLayout->addWidget(m_upEntryButton);
			m_horizontalLayout->addWidget(m_downEntryButton);
		}

		m_verticalLayout->addWidget(m_view);

		m_buttonBox = new QDialogButtonBox(this);
		m_buttonBox->setObjectName(QStringLiteral("buttonBox"));
		m_buttonBox->setStandardButtons(QDialogButtonBox::Apply | QDialogButtonBox::Close | QDialogButtonBox::Ok | QDialogButtonBox::Reset);

		m_verticalLayout->addWidget(m_buttonBox);
		setFocusProxy(m_view);
		
		m_buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL + Qt::Key_Return);
		m_buttonBox->button(QDialogButtonBox::Close)->setShortcut(Qt::Key_Escape);

		retranslateUi();
	}

	void DynamicTable::retranslateUi()
	{
		m_newEntryButton->setToolTip(tr("Add new entry (Ctrl+Ins)"));
		m_newEntryButton->setShortcut(tr("Ctrl+Ins"));

		m_deleteEntryButton->setToolTip(tr("Delete current entry (Del)"));
		m_deleteEntryButton->setShortcut(tr("Del"));

		m_upEntryButton->setToolTip(tr("Move current entry up (Ctrl+Up)"));
		m_upEntryButton->setShortcut(tr("Ctrl+Up"));

		m_downEntryButton->setToolTip(tr("Move current entry down (Ctrl+Down)"));
		m_downEntryButton->setShortcut(tr("Ctrl+Down"));
	}
}
