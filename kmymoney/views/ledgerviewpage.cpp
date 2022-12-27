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
#include "kmymoneysettings.h"
#include "ledgerviewsettings.h"
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
}

LedgerViewPage::LedgerViewPage(LedgerViewPage::Private& dd, QWidget* parent, const QString& configGroupName)
    : QWidget(parent)
    , d(&dd)
{
    d->initWidgets(configGroupName);
    d->ui->m_ledgerView->setModel(MyMoneyFile::instance()->journalModel()->newTransaction());
}

void LedgerViewPage::showEvent(QShowEvent* event)
{
    // the first time we are shown, we need to
    // setup the model to get access to data
    if (d->needModelInit) {
        initModel();
        Q_EMIT requestSelectionChanged(d->selections);
    }
    QWidget::showEvent(event);
}

void LedgerViewPage::initModel()
{
    // setup the model stack
    const auto file = MyMoneyFile::instance();
    d->accountFilter =
        new LedgerAccountFilter(d->ui->m_ledgerView,
                                QVector<QAbstractItemModel*>{file->specialDatesModel(), file->schedulesJournalModel(), file->reconciliationModel()});
    connect(file->journalModel(), &JournalModel::balanceChanged, d->accountFilter, &LedgerAccountFilter::recalculateBalancesOnIdle);
    d->accountFilter->setHideReconciledTransactions(LedgerViewSettings::instance()->hideReconciledTransactions());
    d->accountFilter->setHideTransactionsBefore(LedgerViewSettings::instance()->hideTransactionsBefore());

    d->stateFilter = new LedgerFilter(d->ui->m_ledgerView);
    d->stateFilter->setSourceModel(d->accountFilter);
    d->stateFilter->setComboBox(d->ui->m_filterBox);
    d->stateFilter->setLineEdit(d->ui->m_searchWidget);
    d->ui->m_searchWidget->installEventFilter(this);

    d->specialItemFilter = new SpecialLedgerItemFilter(this);
    d->specialItemFilter->setSourceModel(d->stateFilter);
    d->specialItemFilter->setSortRole(eMyMoney::Model::TransactionPostDateRole);

    // prepare the filter container
    d->ui->m_closeButton->setIcon(Icons::get(Icon::DialogClose));
    d->ui->m_closeButton->setAutoRaise(true);
    d->ui->m_filterContainer->hide();

    connect(d->ui->m_closeButton, &QToolButton::clicked, this, [&]() {
        d->clearFilter();
    });
    connect(pActions[eMenu::Action::ShowFilterWidget], &QAction::triggered, this, [&]() {
        if (isVisible()) {
            d->ui->m_filterContainer->show();
            d->ui->m_searchWidget->setFocus();
        }
    });

    // Moving rows in a source model to a QConcatenateTablesProxyModel
    // does not get propagated through it which destructs our ledger in such cases.
    //
    // A workaround is to invalidate the sort filter.
    connect(file->journalModel(), &JournalModel::rowsAboutToBeMoved, this, &LedgerViewPage::keepSelection);
    connect(file->journalModel(), &JournalModel::rowsMoved, this, &LedgerViewPage::reloadFilter, Qt::QueuedConnection);

    d->ui->m_ledgerView->setModel(d->specialItemFilter);

    connect(LedgerViewSettings::instance(), &LedgerViewSettings::settingsChanged, this, [&]() {
        d->accountFilter->setHideReconciledTransactions(LedgerViewSettings::instance()->hideReconciledTransactions());
        d->accountFilter->setHideTransactionsBefore(LedgerViewSettings::instance()->hideTransactionsBefore());
        d->specialItemFilter->forceReload();
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
        auto list = d->ui->m_ledgerView->selectedJournalEntries();
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
        d->reconciliationDate = account.lastReconciliationDate();
        d->precision = MyMoneyMoney::denomToPrec(account.fraction());
        d->accountName = account.name();
    });

    d->reconciliationDate = acc.lastReconciliationDate();
    d->precision = MyMoneyMoney::denomToPrec(acc.fraction());
    d->accountName = acc.name();

    const auto file = MyMoneyFile::instance();
    d->totalBalance = file->balance(d->accountId, QDate());
    d->clearedBalance = file->clearedBalance(d->accountId, QDate());
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

    d->ui->m_ledgerView->ensureCurrentItemIsVisible();
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
        d->selections.setSelection(SelectedObjects::Schedule, selections.selection(SelectedObjects::Schedule));
        Q_EMIT requestSelectionChanged(d->selections);
    }
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
    if (d->stackedView) {
        qDebug() << "view stack already taken, old one destroyed";
        d->stackedView->deleteLater();
    }
    d->ui->m_ledgerView->setSelectedJournalEntries(view->d->ui->m_ledgerView->selectedJournalEntries());
    d->stackedView = view;
}

LedgerViewPage* LedgerViewPage::popView()
{
    const auto view = d->stackedView;
    d->stackedView = nullptr;
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
