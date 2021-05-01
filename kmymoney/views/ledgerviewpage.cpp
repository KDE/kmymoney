/*
    SPDX-FileCopyrightText: 2015-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "ledgerviewpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QKeyEvent>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <ui_ledgerviewpage.h>
#include "newtransactionform.h"
#include "ledgeraccountfilter.h"
#include "ledgerfilter.h"
#include "specialdatesfilter.h"
#include "specialdatesmodel.h"
#include "schedulesjournalmodel.h"
#include "journalmodel.h"
#include "mymoneyenums.h"
#include "widgetenums.h"
#include "menuenums.h"
#include "mymoneyfile.h"
#include "selectedobjects.h"
#include "mymoneyaccount.h"
#include "icons.h"

using namespace Icons;
using namespace eWidgets;

class LedgerViewPage::Private
{
public:
    Private(QWidget* parent)
        : ui(new Ui_LedgerViewPage)
        , accountFilter(nullptr)
        , specialDatesFilter(nullptr)
        , stateFilter(nullptr)
        , form(nullptr)
    {
        ui->setupUi(parent);

        // make sure, we can disable the detail form but not the ledger view
        ui->m_splitter->setCollapsible(0, false);
        ui->m_splitter->setCollapsible(1, true);

        // make sure the ledger gets all the stretching
        ui->m_splitter->setStretchFactor(0, 3);
        ui->m_splitter->setStretchFactor(1, 1);
        ui->m_splitter->setSizes(QList<int>() << 10000 << ui->m_formWidget->sizeHint().height());

        delayTimer.setSingleShot(true);
    }

    ~Private()
    {
        delete ui;
    }

    Ui_LedgerViewPage*    ui;
    LedgerAccountFilter*  accountFilter;
    SpecialDatesFilter*   specialDatesFilter;
    LedgerFilter*         stateFilter;
    NewTransactionForm*   form;
    QSet<QString>         hideFormReasons;
    QString               accountId;
    SelectedObjects       selections;
    QTimer                delayTimer;
};

LedgerViewPage::LedgerViewPage(QWidget* parent, const QString& configGroupName)
    : QWidget(parent)
    , d(new Private(this))
{
    connect(d->ui->m_ledgerView, &LedgerView::transactionSelected, this, &LedgerViewPage::transactionSelected);
    connect(d->ui->m_ledgerView, &LedgerView::aboutToStartEdit, this, &LedgerViewPage::aboutToStartEdit);
    connect(d->ui->m_ledgerView, &LedgerView::aboutToFinishEdit, this, &LedgerViewPage::aboutToFinishEdit);
    connect(d->ui->m_ledgerView, &LedgerView::aboutToStartEdit, this, &LedgerViewPage::startEdit);
    connect(d->ui->m_ledgerView, &LedgerView::aboutToFinishEdit, this, &LedgerViewPage::finishEdit);
    connect(d->ui->m_ledgerView, &LedgerView::transactionSelectionChanged, this, &LedgerViewPage::slotRequestSelectionChanged);
    connect(d->ui->m_ledgerView, &LedgerView::requestCustomContextMenu, this, &LedgerViewPage::requestCustomContextMenu);

    connect(d->ui->m_splitter, &QSplitter::splitterMoved, this, &LedgerViewPage::splitterChanged);

    d->ui->m_ledgerView->setColumnSelectorGroupName(configGroupName);

    // setup the model stack
    const auto file = MyMoneyFile::instance();
    d->accountFilter = new LedgerAccountFilter(d->ui->m_ledgerView, QVector<QAbstractItemModel*> { file->specialDatesModel(), file->schedulesJournalModel() } );
    connect(file->journalModel(), &JournalModel::balanceChanged, d->accountFilter, &LedgerAccountFilter::recalculateBalancesOnIdle);

    d->stateFilter = new LedgerFilter(d->ui->m_ledgerView);
    d->stateFilter->setSourceModel(d->accountFilter);
    d->stateFilter->setComboBox(d->ui->m_filterBox);
    d->stateFilter->setLineEdit(d->ui->m_searchWidget);
    d->ui->m_searchWidget->installEventFilter(this);

    d->specialDatesFilter = new SpecialDatesFilter(file->specialDatesModel(), this);
    d->specialDatesFilter->setSourceModel(d->stateFilter);

    // prepare the filter container
    d->ui->m_closeButton->setIcon(Icons::get(Icon::DialogClose));
    d->ui->m_closeButton->setAutoRaise(true);
    d->ui->m_filterContainer->hide();

    connect(d->ui->m_closeButton, &QToolButton::clicked, this, [&]() {
        d->stateFilter->clearFilter();
        d->ui->m_filterContainer->hide();
        d->ui->m_ledgerView->setFocus();
        QMetaObject::invokeMethod(d->ui->m_ledgerView, &LedgerView::ensureCurrentItemIsVisible, Qt::QueuedConnection);
    });
    connect(pActions[eMenu::Action::ShowFilterWidget], &QAction::triggered, this, [&]() {
        if (isVisible()) {
            d->ui->m_filterContainer->show();
            d->ui->m_searchWidget->setFocus();
        }
    });

    // Moving rows in a source model to a KConcatenateRowsProxyModel
    // does not get propagated through it which destructs our ledger in such cases.
    //
    // A workaround is to invalidate the sort filter.
    //
    // Since KConcatenateRowsProxyModel is deprecated I did not dare to fix it
    // and hope that QConcatenateTablesProxyModel has this fixed. But it was
    // only introduced with Qt 5.13 which is a bit new for some distros. As
    // it looks from the source of it the problem is still present as of 2020-01-04
    connect(file->journalModel(), &JournalModel::rowsAboutToBeMoved, this, &LedgerViewPage::keepSelection);
    connect(file->journalModel(), &JournalModel::rowsMoved, this, &LedgerViewPage::reloadFilter, Qt::QueuedConnection);

    d->ui->m_ledgerView->setModel(d->specialDatesFilter);

    // combine multipe row updates into one
    connect(d->stateFilter, &LedgerFilter::rowsRemoved, this, [&]() {
        // trigger update
        d->delayTimer.start(20);
    });

    connect(d->stateFilter, &LedgerFilter::rowsInserted, this, [&]() {
        // trigger update
        d->delayTimer.start(20);
    });

    connect(&d->delayTimer, &QTimer::timeout, this, [&]() {
        auto list = d->ui->m_ledgerView->selectedJournalEntries();
        if (list.isEmpty()) {
            d->ui->m_ledgerView->selectMostRecentTransaction();
        } else {
            d->ui->m_ledgerView->ensureCurrentItemIsVisible();
        }
    });
}

LedgerViewPage::~LedgerViewPage()
{
    delete d;
}

bool LedgerViewPage::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == d->ui->m_searchWidget) {
        if (event->type() == QEvent::KeyPress) {
            const auto kev = static_cast<QKeyEvent*>(event);
            if (kev->modifiers() == Qt::NoModifier && kev->key() == Qt::Key_Escape) {
                d->ui->m_closeButton->animateClick();
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void LedgerViewPage::keepSelection()
{
    d->selections.setSelection(SelectedObjects::JournalEntry, d->ui->m_ledgerView->selectedJournalEntries());
}

void LedgerViewPage::reloadFilter()
{
    d->specialDatesFilter->forceReload();

    d->ui->m_ledgerView->setSelectedJournalEntries(d->selections.selection(SelectedObjects::JournalEntry));
    // not sure if the following statement must be removed (THB - 2020-09-20)
    d->selections.clearSelections(SelectedObjects::JournalEntry);
}

QString LedgerViewPage::accountId() const
{
    return d->accountId;
}

void LedgerViewPage::setAccount(const MyMoneyAccount& acc)
{
    QVector<int> columns;
    // get rid of current form
    delete d->form;
    d->form = 0;
    d->hideFormReasons.insert(QLatin1String("FormAvailable"));

    switch(acc.accountType()) {
    case eMyMoney::Account::Type::Investment:
        columns = { JournalModel::Column::Number,
                    JournalModel::Column::Account,
                    JournalModel::Column::CostCenter,
                    JournalModel::Column::Amount,
                    JournalModel::Column::Payment,
                    JournalModel::Column::Deposit,
                  };
        d->ui->m_ledgerView->setColumnsHidden(columns);
        columns = {
            JournalModel::Column::Date,
            JournalModel::Column::Security,
            JournalModel::Column::Detail,
            JournalModel::Column::Reconciliation,
            JournalModel::Column::Quantity,
            JournalModel::Column::Price,
            JournalModel::Column::Value,
            JournalModel::Column::Balance,
        };
        d->ui->m_ledgerView->setColumnsShown(columns);
        break;

    default:
        columns = { JournalModel::Column::Account,
                    JournalModel::Column::Security,
                    JournalModel::Column::CostCenter,
                    JournalModel::Column::Quantity,
                    JournalModel::Column::Price,
                    JournalModel::Column::Amount,
                    JournalModel::Column::Value,
                  };
        d->ui->m_ledgerView->setColumnsHidden(columns);
        columns = { JournalModel::Column::Number,
                    JournalModel::Column::Date,
                    JournalModel::Column::Detail,
                    JournalModel::Column::Reconciliation,
                    JournalModel::Column::Payment,
                    JournalModel::Column::Deposit,
                    JournalModel::Column::Balance,
                  };
        d->ui->m_ledgerView->setColumnsShown(columns);

        d->form = new NewTransactionForm(d->ui->m_formWidget);
        break;
    }

    if(d->form) {
        d->hideFormReasons.remove(QLatin1String("FormAvailable"));
        // make sure we have a layout
        if(!d->ui->m_formWidget->layout()) {
            d->ui->m_formWidget->setLayout(new QHBoxLayout(d->ui->m_formWidget));
        }
        d->ui->m_formWidget->layout()->addWidget(d->form);
        connect(d->ui->m_ledgerView, &LedgerView::transactionSelected, d->form, &NewTransactionForm::showTransaction);
    }
    d->ui->m_formWidget->setVisible(d->hideFormReasons.isEmpty());
    d->accountFilter->setAccount(acc);
    d->accountId = acc.id();

    d->ui->m_ledgerView->setAccountId(d->accountId);
    d->selections.setSelection(SelectedObjects::Account, d->accountId);
    if (!acc.institutionId().isEmpty()) {
        d->selections.setSelection(SelectedObjects::Institution, acc.institutionId());
    }
    d->ui->m_ledgerView->selectMostRecentTransaction();
}

void LedgerViewPage::showTransactionForm(bool show)
{
    if(show) {
        d->hideFormReasons.remove(QLatin1String("General"));
    } else {
        d->hideFormReasons.insert(QLatin1String("General"));
    }
    d->ui->m_formWidget->setVisible(d->hideFormReasons.isEmpty());
}

void LedgerViewPage::startEdit()
{
    d->hideFormReasons.insert(QLatin1String("Edit"));
    d->ui->m_formWidget->hide();
}

void LedgerViewPage::finishEdit()
{
    d->hideFormReasons.remove(QLatin1String("Edit"));
    d->ui->m_formWidget->setVisible(d->hideFormReasons.isEmpty());
    // the focus should be on the ledger view once editing ends
    d->ui->m_ledgerView->setFocus();
}

void LedgerViewPage::splitterChanged(int pos, int index)
{
    Q_UNUSED(pos);
    Q_UNUSED(index);

    d->ui->m_ledgerView->ensureCurrentItemIsVisible();
}

void LedgerViewPage::setShowEntryForNewTransaction(bool show)
{
    d->accountFilter->setShowEntryForNewTransaction(show);
}

void LedgerViewPage::slotSettingsChanged()
{
    d->ui->m_ledgerView->slotSettingsChanged();
}

void LedgerViewPage::slotRequestSelectionChanged(const SelectedObjects& selections) const
{
    d->selections.setSelection(SelectedObjects::JournalEntry, selections.selection(SelectedObjects::JournalEntry));
    emit requestSelectionChanged(d->selections);
}

const SelectedObjects& LedgerViewPage::selections() const
{
    d->selections.setSelection(SelectedObjects::JournalEntry, d->ui->m_ledgerView->selectedJournalEntries());
    return d->selections;
}

void LedgerViewPage::selectJournalEntry(const QString& id)
{
    d->ui->m_ledgerView->setSelectedJournalEntries(QStringList{id});
}

void LedgerViewPage::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    const auto journalEntryIds = selections.selection(SelectedObjects::JournalEntry);
    switch (action) {
    case eMenu::Action::GoToAccount:
    case eMenu::Action::OpenAccount:
        if (!journalEntryIds.isEmpty()) {
            selectJournalEntry(journalEntryIds.first());
        }
        break;

    case eMenu::Action::NewTransaction:
        d->ui->m_ledgerView->editNewTransaction();
        break;

    case eMenu::Action::EditTransaction:
        d->ui->m_ledgerView->edit(d->ui->m_ledgerView->currentIndex());
        break;

    case eMenu::Action::EditSplits: {
        d->ui->m_ledgerView->edit(d->ui->m_ledgerView->currentIndex());
        const auto editor = d->ui->m_ledgerView->indexWidget(d->ui->m_ledgerView->editIndex());
        if (editor) {
            QMetaObject::invokeMethod(editor, "editSplits", Qt::QueuedConnection);
        }
        break;
    }
    case eMenu::Action::SelectAllTransactions:
        d->ui->m_ledgerView->selectAllTransactions();
        break;
    default:
        break;
    }
}
