/*
    SPDX-FileCopyrightText: 2015-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "ledgerviewpage.h"
#include "ledgerviewpage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QKeyEvent>
#include <QPointer>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include "journalmodel.h"
#include "kmmsearchwidget.h"
#include "kmymoneysettings.h"
#include "menuenums.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "reconciliationmodel.h"
#include "schedulesjournalmodel.h"
#include "specialdatesmodel.h"
#include "tabordereditor.h"
#include "widgetenums.h"

using namespace Icons;
using namespace eWidgets;

LedgerViewPage::LedgerViewPage(QWidget* parent, const QString& configGroupName)
    : QWidget(parent)
    , d(new Private(this))
{
    d->initWidgets(configGroupName);
    d->ui->m_ledgerView->setModel(MyMoneyFile::instance()->journalModel()->newTransaction());

    connect(d->ui->m_ledgerView, &LedgerView::modifySortOrder, this, [&]() {
        d->selectSortOrder();
    });
}

LedgerViewPage::LedgerViewPage(LedgerViewPage::Private& dd, QWidget* parent, const QString& configGroupName)
    : QWidget(parent)
    , d(&dd)
{
    d->initWidgets(configGroupName);
    d->ui->m_ledgerView->setModel(MyMoneyFile::instance()->journalModel()->newTransaction());

    connect(d->ui->m_ledgerView, &LedgerView::modifySortOrder, this, [&]() {
        d->selectSortOrder();
    });
}

void LedgerViewPage::showEvent(QShowEvent* event)
{
    prepareToShow();
    QWidget::showEvent(event);
}

void LedgerViewPage::prepareToShow()
{
    // make sure to run only once
    if (d->needModelInit) {
        initModel();
    }
}

void LedgerViewPage::initModel()
{
    // setup the model stack
    const auto file = MyMoneyFile::instance();
    auto viewSettings = LedgerViewSettings::instance();
    d->accountFilter =
        new LedgerAccountFilter(d->ui->m_ledgerView,
                                QVector<QAbstractItemModel*>{file->specialDatesModel(), file->schedulesJournalModel(), file->reconciliationModel()});
    d->accountFilter->setHideReconciledTransactions(viewSettings->hideReconciledTransactions());
    d->accountFilter->setHideTransactionsBefore(viewSettings->hideTransactionsBefore());

    d->stateFilter = new LedgerFilter(d->ui->m_ledgerView);
    d->stateFilter->setSourceModel(d->accountFilter);
    d->stateFilter->setComboBox(d->ui->m_searchWidget->comboBox());
    d->stateFilter->setLineEdit(d->ui->m_searchWidget->lineEdit());

    d->specialItemFilter = new SpecialLedgerItemFilter(this);
    d->specialItemFilter->setSourceModel(d->stateFilter);
    d->specialItemFilter->setSortRole(eMyMoney::Model::TransactionPostDateRole);
    d->specialItemFilter->setShowReconciliationEntries(viewSettings->showReconciliationEntries());

    connect(d->ui->m_searchWidget, &KMMSearchWidget::closed, this, [&]() {
        d->clearFilter();
        d->ui->m_ledgerView->setFocus();
    });
    connect(pActions[eMenu::Action::ShowFilterWidget], &QAction::triggered, this, [&]() {
        if (isVisible()) {
            d->ui->m_searchWidget->show();
        }
    });

    // Moving rows in a source model to a QConcatenateTablesProxyModel
    // does not get propagated through it which destructs our ledger in such cases.
    //
    // A workaround is to invalidate the sort filter.
    // connect(file->journalModel(), &JournalModel::rowsAboutToBeMoved, this, &LedgerViewPage::keepSelection);
    // connect(file->journalModel(), &JournalModel::rowsMoved, this, &LedgerViewPage::reloadFilter, Qt::QueuedConnection);

    d->ui->m_ledgerView->setModel(d->specialItemFilter);

    connect(viewSettings, &LedgerViewSettings::settingsChanged, this, [&]() {
        const auto settings = LedgerViewSettings::instance();
        d->accountFilter->setHideReconciledTransactions(settings->hideReconciledTransactions());
        d->accountFilter->setHideTransactionsBefore(settings->hideTransactionsBefore());
        d->specialItemFilter->setHideReconciledTransactions(settings->hideReconciledTransactions());
        d->specialItemFilter->setShowReconciliationEntries(settings->showReconciliationEntries());

        // make sure sorting is updated
        const auto acc = MyMoneyFile::instance()->accountsModel()->itemById(d->accountId);
        d->updateAccountData(acc);
    });

    // combine multiple row updates into one
    connect(d->stateFilter, &LedgerFilter::rowsRemoved, this, [&]() {
        // trigger update
        d->delayTimer.start(20);
    });

    connect(d->stateFilter, &LedgerFilter::rowsInserted, this, [&]() {
        // trigger update
        d->delayTimer.start(20);
    });

    connect(&d->delayTimer, &QTimer::timeout, this, [&]() {
        auto list = d->ui->m_ledgerView->selectedJournalEntryIds();
        if (list.isEmpty()) {
            d->ui->m_ledgerView->selectMostRecentTransaction();
        } else {
            d->ui->m_ledgerView->ensureCurrentItemIsVisible();
        }
    });

    connect(d->ui->m_ledgerView, &LedgerView::sectionResized, this, &LedgerViewPage::sectionResized);
    connect(d->ui->m_ledgerView, &LedgerView::sectionMoved, this, &LedgerViewPage::sectionMoved);
    connect(this, &LedgerViewPage::resizeSection, d->ui->m_ledgerView, &LedgerView::resizeSection);
    connect(this, &LedgerViewPage::moveSection, d->ui->m_ledgerView, &LedgerView::moveSection);

    connect(d->ui->m_ledgerView, &LedgerView::requestView, this, &LedgerViewPage::requestView);

    connect(file->journalModel(), &JournalModel::balancesChanged, this, &LedgerViewPage::updateSummaryInformation);

    d->needModelInit = false;

    // now execute all the postponed actions
    const auto acc = file->account(d->accountId);
    this->setAccount(acc);

    setShowEntryForNewTransaction(d->showEntryForNewTransaction);

    // now sort everything
    d->accountFilter->setSortingEnabled(true);
    // the next call will also take care of enabling
    // sorting on the stateFilter.
    d->specialItemFilter->setSortingEnabled(true);
}

LedgerViewPage::~LedgerViewPage()
{
    delete d;
}

/**
 * @todo need to update that in the case when the journalId changes
 * due to a change of the TransactionPostDate
 */
void LedgerViewPage::keepSelection()
{
    d->selections.setSelection(SelectedObjects::JournalEntry, d->ui->m_ledgerView->selectedJournalEntryIds());
}

void LedgerViewPage::reloadFilter()
{
    d->specialItemFilter->forceReload();

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
    // in case we don't have a model, postpone this call
    // but remember the account id
    if (d->needModelInit) {
        d->accountId = acc.id();
        return;
    }

    QVector<int> columns;
    // get rid of current form
    delete d->form;
    d->form = nullptr;
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
            JournalModel::Column::Quantity,
            JournalModel::Column::Price,
            JournalModel::Column::Value,
            JournalModel::Column::Balance,
        };
        d->ui->m_ledgerView->setColumnsShown(columns);
        d->isInvestmentView = true;
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
        columns = {
            JournalModel::Column::Date,
            JournalModel::Column::Detail,
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

    connect(MyMoneyFile::instance()->accountsModel(), &AccountsModel::dataChanged, this, [&]() {
        const auto account = MyMoneyFile::instance()->accountsModel()->itemById(d->accountId);
        if (!account.id().isEmpty()) {
            d->updateAccountData(account);
        }
    });

    d->updateAccountData(acc);

    d->ui->m_ledgerView->setSortOrder(d->sortOrder);

    const auto file = MyMoneyFile::instance();
    d->totalBalance = file->balance(d->accountId, QDate());
    d->clearedBalance = file->clearedBalance(d->accountId, QDate());
    d->selectedTotal = MyMoneyMoney();
    d->updateSummaryInformation();
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

    QMetaObject::invokeMethod(d->ui->m_ledgerView, &LedgerView::ensureCurrentItemIsVisible, Qt::QueuedConnection);
    // d->ui->m_ledgerView->ensureCurrentItemIsVisible();
}

void LedgerViewPage::setShowEntryForNewTransaction(bool show)
{
    // postpone this setting until we are fully initialized
    if (d->needModelInit) {
        d->showEntryForNewTransaction = show;
        return;
    }
    d->accountFilter->setShowEntryForNewTransaction(show);
}

void LedgerViewPage::slotSettingsChanged()
{
    showTransactionForm(KMyMoneySettings::transactionForm());
    d->ui->m_ledgerView->slotSettingsChanged();
}

void LedgerViewPage::slotRequestSelectionChanged(const SelectedObjects& selections) const
{
    if (isVisible()) {
        d->selections.setSelection(SelectedObjects::JournalEntry, selections.selection(SelectedObjects::JournalEntry));
        if (selections.count(SelectedObjects::JournalEntry) > 1) {
            // More than one item selected, so show sum
            MyMoneyMoney balance;
            for (const auto& journalEntryId : selections.selection(SelectedObjects::JournalEntry)) {
                const auto idx = MyMoneyFile::instance()->journalModel()->indexById(journalEntryId);
                if (idx.isValid()) {
                    balance += idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                }
            }
            d->selectedTotal = balance;

        } else { // Only one thing selected, so set selectedTotal back to 0 to show balance
            d->selectedTotal = MyMoneyMoney();
        }

        d->updateSummaryInformation();

        d->selections.setSelection(SelectedObjects::Schedule, selections.selection(SelectedObjects::Schedule));
        Q_EMIT requestSelectionChanged(d->selections);
    }
}

const SelectedObjects& LedgerViewPage::selections() const
{
    d->selections.setSelection(SelectedObjects::JournalEntry, d->ui->m_ledgerView->selectedJournalEntryIds());
    return d->selections;
}

void LedgerViewPage::selectJournalEntry(const QString& id)
{
    d->ui->m_ledgerView->setSelectedJournalEntries(QStringList{id});
}

bool LedgerViewPage::executeAction(eMenu::Action action, const SelectedObjects& selections)
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

    case eMenu::Action::MatchTransaction:
        d->ui->m_ledgerView->reselectJournalEntry(selections.firstSelection(SelectedObjects::JournalEntry));
        break;

    case eMenu::Action::EditTabOrder: {
        const auto editor = d->ui->m_ledgerView->indexWidget(d->ui->m_ledgerView->editIndex());
        if (editor) {
            QPointer<TabOrderDialog> tabOrderDialog = new TabOrderDialog(d->ui->m_ledgerView);
            auto tabOrderWidget = static_cast<TabOrderEditorInterface*>(editor->qt_metacast("TabOrderEditorInterface"));
            tabOrderDialog->setTarget(tabOrderWidget);
            auto tabOrder = editor->property("kmm_defaulttaborder").toStringList();
            tabOrderDialog->setDefaultTabOrder(tabOrder);
            tabOrder = editor->property("kmm_currenttaborder").toStringList();
            tabOrderDialog->setTabOrder(tabOrder);

            if ((tabOrderDialog->exec() == QDialog::Accepted) && tabOrderDialog) {
                tabOrderWidget->storeTabOrder(tabOrderDialog->tabOrder());
            }
            delete tabOrderDialog;
        }
        break;
    }

    case eMenu::Action::ShowTransaction:
        d->ui->m_ledgerView->showEditor();
        break;

    default:
        break;
    }
    return true;
}

void LedgerViewPage::pushView(LedgerViewPage* view)
{
    Q_ASSERT(view != nullptr);

    if (d->stackedView) {
        qDebug() << "view stack already taken, old one destroyed";
        d->stackedView->deleteLater();
    }
    d->ui->m_ledgerView->setSelectedJournalEntries(view->d->ui->m_ledgerView->selectedJournalEntryIds());
    d->stackedView = view;
    d->stackedView->blockSignals(true);
}

LedgerViewPage* LedgerViewPage::popView()
{
    const auto view = d->stackedView;
    d->stackedView = nullptr;
    if (view) {
        view->blockSignals(false);
    }
    return view;
}

bool LedgerViewPage::hasPushedView() const
{
    return d->stackedView != nullptr;
}

QString LedgerViewPage::accountName()
{
    return d->accountName;
}

void LedgerViewPage::updateSummaryInformation(const QHash<QString, AccountBalances>& balances)
{
    if (d->isInvestmentView) {
        d->updateSummaryInformation();

    } else {
        const auto it = balances.find(d->accountId);
        if (it != balances.cend()) {
            d->totalBalance = (*it).m_totalBalance;
            d->clearedBalance = (*it).m_clearedBalance;
            d->updateSummaryInformation();
        }
    }
}

QList<int> LedgerViewPage::splitterSizes() const
{
    return d->ui->m_splitter->sizes();
}

void LedgerViewPage::setSplitterSizes(QList<int> sizes)
{
    d->ui->m_splitter->setSizes(sizes);
}
