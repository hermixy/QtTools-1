#include <QtTools/HeaderConfigurationWidget.hqt>
#include <QtTools/ToggleChecked.hpp>

#include <QtWidgets/QShortcut>
#include <QtWidgets/QLineEdit>
#include <QtTools/Delegates/SearchDelegate.hpp>

namespace QtTools
{
	void HeaderConfigurationWidget::OnFilterChanged(QString text)
	{
		auto * d = static_cast<QtTools::Delegates::SearchDelegate *>(m_view->itemDelegate());
		d->SetFilterText(text);
		m_view->viewport()->update();
	}

	void HeaderConfigurationWidget::OnUpItem()
	{
		auto idx = m_view->selectionModel()->currentIndex();
		if (!idx.isValid() || idx.row() == 0)
			return;

		m_model->moveRow(idx.parent(), idx.row(), idx.parent(), idx.row() - 1);
	}

	void HeaderConfigurationWidget::OnDownItem()
	{
		auto idx = m_view->selectionModel()->currentIndex();
		if (!idx.isValid() || idx.row() == m_model->rowCount() - 1)
			return;

		m_model->moveRow(idx.parent(), idx.row(), idx.parent(), idx.row() + 2);
	}

	void HeaderConfigurationWidget::OnToggleSelected()
	{
		auto indxes = m_view->selectionModel()->selectedIndexes();
		QtTools::ToggleChecked(indxes);
	}

	HeaderConfigurationWidget::HeaderConfigurationWidget(HeaderControlModel & model, QWidget * parent /*= nullptr*/, Qt::WindowFlags flags /*= 0*/)
		: QDialog(parent, flags | Qt::Tool), m_model(&model)
	{
		Init();
	}

	void HeaderConfigurationWidget::Init()
	{
		setupUi();
		retranslateUi();
		connectSignals();
	}

	void HeaderConfigurationWidget::connectSignals()
	{
		//: filter shortcut in generic BasicTableWidget, probably should not be translated
		auto * searchShortcut = new QShortcut(QKeySequence(tr("Ctrl+F")), this);
		connect(searchShortcut, &QShortcut::activated,
		        m_searchEdit, static_cast<void (QLineEdit::*)()>(&QLineEdit::setFocus));
		connect(m_searchEdit, &QLineEdit::textChanged, this, &HeaderConfigurationWidget::OnFilterChanged);
		
		connect(m_resetButton, &QToolButton::clicked, m_model, &HeaderControlModel::Reset);
		connect(m_eraseNonPresentButton, &QToolButton::clicked, m_model, &HeaderControlModel::EraseNonPresent);

		connect(m_toggleSelectedButton, &QToolButton::clicked, this, &HeaderConfigurationWidget::OnToggleSelected);
		connect(m_upEntryButton, &QToolButton::clicked, this, &HeaderConfigurationWidget::OnUpItem);
		connect(m_downEntryButton, &QToolButton::clicked, this, &HeaderConfigurationWidget::OnDownItem);
	}

	void HeaderConfigurationWidget::setupUi()
	{
		// main layout
		m_verticalLayout = new QVBoxLayout(this);

		{// create toolbar, spacer with 4 buttons
			m_horizontalLayout = new QHBoxLayout();
			m_verticalLayout->addLayout(m_horizontalLayout);

			m_searchEdit = new QLineEdit(this);
			m_searchEdit->setClearButtonEnabled(true);
			m_horizontalLayout->addWidget(m_searchEdit);

			m_horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
			m_horizontalLayout->addItem(m_horizontalSpacer);

			// buttons
			m_resetButton = new QToolButton(this);
			m_eraseNonPresentButton = new QToolButton(this);
			m_toggleSelectedButton = new QToolButton(this);
			m_upEntryButton = new QToolButton(this);
			m_downEntryButton = new QToolButton(this);
			
			auto * style = this->style();

			//set icons
			m_toggleSelectedButton->setIcon(style->standardIcon(QStyle::SP_DialogOkButton));
			m_resetButton->setIcon(style->standardIcon(QStyle::SP_DialogResetButton));
			m_eraseNonPresentButton->setIcon(style->standardIcon(QStyle::SP_DialogDiscardButton));
			m_upEntryButton->setIcon(style->standardIcon(QStyle::SP_ArrowUp));
			m_downEntryButton->setIcon(style->standardIcon(QStyle::SP_ArrowDown));

			m_horizontalLayout->addWidget(m_resetButton);
			m_horizontalLayout->addWidget(m_eraseNonPresentButton);
			m_horizontalLayout->addWidget(m_toggleSelectedButton);
			m_horizontalLayout->addWidget(m_upEntryButton);
			m_horizontalLayout->addWidget(m_downEntryButton);
		}

		m_view = new QListView(this);
		
		m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_view->setDragDropMode(QAbstractItemView::InternalMove);
		m_view->setDragEnabled(true);

		m_view->setWrapping(true);
		m_view->setResizeMode(QListView::Adjust);

		m_view->setModel(m_model);
		m_view->setParent(this);

		auto * dlg = new QtTools::Delegates::SearchDelegate(this);
		m_view->setItemDelegate(dlg);
		
		m_verticalLayout->addWidget(m_view);

		retranslateUi();
	}

	void HeaderConfigurationWidget::retranslateUi()
	{
		m_searchEdit->setPlaceholderText(tr("column search(Ctrl+F)"));

		//: resets to original order of elements in source model
		m_resetButton->setShortcut(tr("Ctrl+R"));
		m_resetButton->setToolTip(tr("Reset to original state(Ctrl+R)"));
		
		m_eraseNonPresentButton->setShortcut(tr("Ctrl+Delete"));
		m_eraseNonPresentButton->setToolTip(tr("Delete not present elements(Ctrl+Delete)"));
		
		m_toggleSelectedButton->setShortcut(tr("Ctrl+Space"));
		m_toggleSelectedButton->setToolTip(tr("Toggle selected elements(Ctrl+Space)"));

		m_upEntryButton->setShortcut(tr("Ctrl+Up"));
		m_upEntryButton->setToolTip(tr("Move current up(Ctrl+Up)"));

		m_downEntryButton->setShortcut(tr("Ctrl+Down"));
		m_downEntryButton->setToolTip(tr("Move current down(Ctrl+Down)"));

		setWindowTitle(tr("Header configuration"));
	}
}
