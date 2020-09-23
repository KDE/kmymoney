/*
 * Copyright 2015-2020  Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "ledgerviewpage.h"
#include "mymoneyaccount.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "newtransactionform.h"
#include "ledgeraccountfilter.h"
#include "specialdatesfilter.h"
#include "specialdatesmodel.h"
#include "schedulesjournalmodel.h"
#include "journalmodel.h"
#include "ui_ledgerviewpage.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"

class LedgerViewPage::Private
{
public:
  Private(QWidget* parent)
  : ui(new Ui_LedgerViewPage)
  , accountFilter(nullptr)
  , specialDatesFilter(nullptr)
  , form(nullptr)
  {
    ui->setupUi(parent);

    // make sure, we can disable the detail form but not the ledger view
    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, true);

    // make sure the ledger gets all the stretching
    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 1);
    ui->splitter->setSizes(QList<int>() << 10000 << ui->formWidget->sizeHint().height());
  }

  ~Private()
  {
    delete ui;
  }

  Ui_LedgerViewPage*    ui;
  LedgerAccountFilter*  accountFilter;
  SpecialDatesFilter*   specialDatesFilter;
  NewTransactionForm*   form;
  QSet<QString>         hideFormReasons;
  QString               accountId;
  QStringList           selection;
};

LedgerViewPage::LedgerViewPage(QWidget* parent, const QString& configGroupName)
  : QWidget(parent)
  , d(new Private(this))
{
  connect(d->ui->ledgerView, &LedgerView::transactionSelected, this, &LedgerViewPage::transactionSelected);
  connect(d->ui->ledgerView, &LedgerView::aboutToStartEdit, this, &LedgerViewPage::aboutToStartEdit);
  connect(d->ui->ledgerView, &LedgerView::aboutToFinishEdit, this, &LedgerViewPage::aboutToFinishEdit);
  connect(d->ui->ledgerView, &LedgerView::aboutToStartEdit, this, &LedgerViewPage::startEdit);
  connect(d->ui->ledgerView, &LedgerView::aboutToFinishEdit, this, &LedgerViewPage::finishEdit);
  connect(d->ui->splitter, &QSplitter::splitterMoved, this, &LedgerViewPage::splitterChanged);

  d->ui->ledgerView->setColumnSelectorGroupName(configGroupName);

  // setup the model stack
  const auto file = MyMoneyFile::instance();
  d->accountFilter = new LedgerAccountFilter(d->ui->ledgerView, QVector<QAbstractItemModel*> { file->specialDatesModel(), file->schedulesJournalModel() } );
  connect(file->journalModel(), &JournalModel::balanceChanged, d->accountFilter, &LedgerAccountFilter::recalculateBalancesOnIdle);

  d->specialDatesFilter = new SpecialDatesFilter(file->specialDatesModel(), this);
  d->specialDatesFilter->setSourceModel(d->accountFilter);

  // Moving rows in a source model to a KConcatenateRowsProxyModel
  // does not get propagated through it which destructs our ledger in such cases.
  //
  // A workaround is to invalidate the sort filter.
  //
  // Since KConcatenateRowsProxyModel is deprecated I did not dare to fix it
  // and hope that QConcatenateTablesProxyModel has this fixed. But it was
  // only introduced with Qt 5.13 which is a bit new for some distros. As
  // it looks from the source of it the problem is still present as of 2020-01-04
  connect(file->journalModel(), SIGNAL(rowsAboutToBeMoved(const QModelIndex&,int,int,const QModelIndex&,int)), this, SLOT(keepSelection()));
  connect(file->journalModel(), SIGNAL(rowsMoved(const QModelIndex&,int,int,const QModelIndex&,int)), this, SLOT(reloadFilter()), Qt::QueuedConnection);

  d->ui->ledgerView->setModel(d->specialDatesFilter);
}

LedgerViewPage::~LedgerViewPage()
{
  delete d;
}

void LedgerViewPage::keepSelection()
{
  d->selection = d->ui->ledgerView->selectedTransactions();
}

void LedgerViewPage::reloadFilter()
{
  d->specialDatesFilter->forceReload();

  d->ui->ledgerView->setSelectedTransactions(d->selection);
  d->selection.clear();
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
        JournalModel::Column::Deposit, };
      d->ui->ledgerView->setColumnsHidden(columns);
      columns = {
        JournalModel::Column::Date,
        JournalModel::Column::Security,
        JournalModel::Column::Detail,
        JournalModel::Column::Reconciliation,
        JournalModel::Column::Quantity,
        JournalModel::Column::Price,
        JournalModel::Column::Value,
        JournalModel::Column::Balance, };
      d->ui->ledgerView->setColumnsShown(columns);
      break;

    default:
      columns = { JournalModel::Column::Account,
        JournalModel::Column::Security,
        JournalModel::Column::CostCenter,
        JournalModel::Column::Quantity,
        JournalModel::Column::Price,
        JournalModel::Column::Amount,
        JournalModel::Column::Value, };
      d->ui->ledgerView->setColumnsHidden(columns);
      columns = { JournalModel::Column::Number,
        JournalModel::Column::Date,
        JournalModel::Column::Detail,
        JournalModel::Column::Reconciliation,
        JournalModel::Column::Payment,
        JournalModel::Column::Deposit,
        JournalModel::Column::Balance, };
      d->ui->ledgerView->setColumnsShown(columns);

      d->form = new NewTransactionForm(d->ui->formWidget);
      break;
  }

  if(d->form) {
    d->hideFormReasons.remove(QLatin1String("FormAvailable"));
    // make sure we have a layout
    if(!d->ui->formWidget->layout()) {
      d->ui->formWidget->setLayout(new QHBoxLayout(d->ui->formWidget));
    }
    d->ui->formWidget->layout()->addWidget(d->form);
    connect(d->ui->ledgerView, &LedgerView::transactionSelected, d->form, &NewTransactionForm::showTransaction);
  }
  d->ui->formWidget->setVisible(d->hideFormReasons.isEmpty());
  d->accountFilter->setAccount(acc);
  d->accountId = acc.id();

  d->ui->ledgerView->setAccountId(d->accountId);
  d->ui->ledgerView->selectMostRecentTransaction();
}

void LedgerViewPage::showTransactionForm(bool show)
{
  if(show) {
    d->hideFormReasons.remove(QLatin1String("General"));
  } else {
    d->hideFormReasons.insert(QLatin1String("General"));
  }
  d->ui->formWidget->setVisible(d->hideFormReasons.isEmpty());
}

void LedgerViewPage::startEdit()
{
  d->hideFormReasons.insert(QLatin1String("Edit"));
  d->ui->formWidget->hide();
}

void LedgerViewPage::finishEdit()
{
  d->hideFormReasons.remove(QLatin1String("Edit"));
  d->ui->formWidget->setVisible(d->hideFormReasons.isEmpty());
  // the focus should be on the ledger view once editing ends
  d->ui->ledgerView->setFocus();
}

void LedgerViewPage::splitterChanged(int pos, int index)
{
  Q_UNUSED(pos);
  Q_UNUSED(index);

  d->ui->ledgerView->ensureCurrentItemIsVisible();
}

void LedgerViewPage::setShowEntryForNewTransaction(bool show)
{
  d->accountFilter->setShowEntryForNewTransaction(show);
}

void LedgerViewPage::slotSettingsChanged()
{
  d->ui->ledgerView->slotSettingsChanged();
}
