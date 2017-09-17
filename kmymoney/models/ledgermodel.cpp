/***************************************************************************
                          ledgermodel.cpp
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ledgermodel.h"
#include "models.h"
#include "costcentermodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransaction.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "kmymoneyutils.h"
#include "kmymoneyglobalsettings.h"

LedgerItem::LedgerItem()
{
}

LedgerItem::~LedgerItem()
{
}

LedgerTransaction::LedgerTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s)
  : LedgerItem()
  , m_transaction(t)
  , m_split(s)
  , m_erroneous(false)
{
  // extract the payee id
  QString payeeId = m_split.payeeId();
  if(payeeId.isEmpty()) {
    QList<MyMoneySplit>::const_iterator it;
    for(it = m_transaction.splits().constBegin(); it != m_transaction.splits().constEnd(); ++it) {
      if(!(*it).payeeId().isEmpty()) {
        payeeId = (*it).payeeId();
        break;
      }
    }
  }
  if(!payeeId.isEmpty()) {
    m_payeeId = payeeId;
    m_payeeName = MyMoneyFile::instance()->payee(payeeId).name();
  }

  m_account = MyMoneyFile::instance()->accountToCategory(m_split.accountId());
  m_costCenterId = m_split.costCenterId();

  // A transaction can have more than 2 splits ...
  if(m_transaction.splitCount() > 2) {
    m_counterAccount = i18n("Split transaction");

  // ... exactly two splits ...
  } else if(m_transaction.splitCount() == 2) {
    QList<MyMoneySplit>::const_iterator it;
    for(it = m_transaction.splits().constBegin(); it != m_transaction.splits().constEnd(); ++it) {
      if((*it).id() != m_split.id()) {
        m_counterAccountId = (*it).accountId();
        m_counterAccount = MyMoneyFile::instance()->accountToCategory(m_counterAccountId);
        // in case the own split does not have a costcenter, but the counter split does
        // we use it nevertheless
        if(m_costCenterId.isEmpty())
          m_costCenterId = (*it).costCenterId();
        break;
      }
    }

  // ... or a single split
  } else if(!m_split.shares().isZero()) {
    m_counterAccount = i18n("*** UNASSIGNED ***");
  }

  // The transaction is erroneous in case it is not balanced
  m_erroneous = !m_transaction.splitSum().isZero();

  // now take care of the values
  setupValueDisplay();
}


LedgerTransaction::~LedgerTransaction()
{
}

void LedgerTransaction::setupValueDisplay()
{
  const MyMoneyFile* file = MyMoneyFile::instance();
  const MyMoneyAccount acc = file->account(m_split.accountId());

  MyMoneyMoney value = m_split.value(m_transaction.commodity(), acc.currencyId());
  m_signedShares = value.formatMoney(acc.fraction());

  if(value.isNegative()) {
    m_shares = m_payment = (-value).formatMoney(acc.fraction());
  } else {
    m_shares = m_deposit = m_signedShares;
  }

  // figure out if it is a debit or credit split. s.a. https://en.wikipedia.org/wiki/Debits_and_credits#Aspects_of_transactions
  if(m_split.shares().isNegative()) {
    m_sharesSuffix = i18nc("Credit suffix", "Cr.");
  } else {
    m_sharesSuffix = i18nc("Debit suffix", "Dr.");
  }
}

QDate LedgerTransaction::postDate() const
{
  return m_transaction.postDate();
}

QString LedgerTransaction::transactionSplitId() const
{
  QString rc;
  if(!m_transaction.id().isEmpty()) {
    rc = QString("%1-%2").arg(m_transaction.id()).arg(m_split.id());
  }
  return rc;
}

MyMoneySplit::reconcileFlagE LedgerTransaction::reconciliationState() const
{
  return m_split.reconcileFlag();
}

QString LedgerTransaction::reconciliationStateShort() const
{
  QString rc;
  switch(m_split.reconcileFlag()) {
    case MyMoneySplit::NotReconciled:
    default:
      break;
    case MyMoneySplit::Cleared:
      rc = i18nc("Reconciliation flag C", "C");
      break;
    case MyMoneySplit::Reconciled:
      rc = i18nc("Reconciliation flag R", "R");
      break;
    case MyMoneySplit::Frozen:
      rc = i18nc("Reconciliation flag F", "F");
      break;
  }
  return rc;
}

QString LedgerTransaction::reconciliationStateLong() const
{
  QString rc;
  switch(m_split.reconcileFlag()) {
    case MyMoneySplit::NotReconciled:
    default:
      rc = i18nc("Reconciliation flag empty", "Not reconciled");
      break;
    case MyMoneySplit::Cleared:
      rc = i18nc("Reconciliation flag C", "Cleared");
      break;
    case MyMoneySplit::Reconciled:
      rc = i18nc("Reconciliation flag R", "Reconciled");
      break;
    case MyMoneySplit::Frozen:
      rc = i18nc("Reconciliation flag F", "Frozen");
      break;
  }
  return rc;
}


void LedgerTransaction::setBalance(QString txt)
{
  m_balance = txt;
}

QString LedgerTransaction::memo() const
{
  QString memo = m_split.memo();
  if(memo.isEmpty()) {
    memo = m_transaction.memo();
  }
  return memo;
}

LedgerTransaction LedgerTransaction::newTransactionEntry()
{
  // create a dummy entry for new transactions
  MyMoneyTransaction t;
  t.setPostDate(QDate(2900,12,31));
  return LedgerTransaction(t, MyMoneySplit());
}

bool LedgerTransaction::isNewTransactionEntry() const
{
  return m_transaction.id().isEmpty() && m_split.id().isEmpty();
}

QString LedgerTransaction::transactionCommodity() const
{
  return m_transaction.commodity();
}




QString LedgerSchedule::transactionSplitId() const
{
  return QString("%1-%2").arg(m_schedule.id()).arg(m_split.id());
}

LedgerSchedule::LedgerSchedule(const MyMoneySchedule& s, const MyMoneyTransaction& t, const MyMoneySplit& sp)
  : LedgerTransaction(t, sp)
  , m_schedule(s)
{
}

LedgerSchedule::~LedgerSchedule()
{
}

QString LedgerSchedule::scheduleId() const
{
  return m_schedule.id();
}


LedgerSplit::LedgerSplit(const MyMoneyTransaction& t, const MyMoneySplit& s)
  : LedgerTransaction(t, s)
{
  // override the settings made in the base class
  m_payeeName.clear();
  m_payeeId = m_split.payeeId();
  if(!m_payeeId.isEmpty()) {
    try {
      m_payeeName = MyMoneyFile::instance()->payee(m_payeeId).name();
    } catch(MyMoneyException&) {
      qDebug() << "payee" << m_payeeId << "not found.";
    }
  }
}

LedgerSplit::~LedgerSplit()
{
}

QString LedgerSplit::memo() const
{
  return m_split.memo();
}








LedgerSortFilterProxyModel::LedgerSortFilterProxyModel(QObject* parent)
  : QSortFilterProxyModel(parent)
  , m_showNewTransaction(false)
  , m_accountType(MyMoneyAccount::Asset)
{
  setFilterRole(LedgerRole::AccountIdRole);
  setFilterKeyColumn(0);
  setSortRole(LedgerRole::PostDateRole);
  setDynamicSortFilter(true);
}

LedgerSortFilterProxyModel::~LedgerSortFilterProxyModel()
{
}

void LedgerSortFilterProxyModel::setAccountType(MyMoneyAccount::accountTypeE type)
{
  m_accountType = type;
}

QVariant LedgerSortFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case LedgerModel::PaymentColumn:
        switch(m_accountType) {
          case MyMoneyAccount::CreditCard:
            return i18nc("Payment made with credit card", "Charge");

          case MyMoneyAccount::Asset:
          case MyMoneyAccount::AssetLoan:
            return i18nc("Decrease of asset/liability value", "Decrease");

          case MyMoneyAccount::Liability:
          case MyMoneyAccount::Loan:
            return i18nc("Increase of asset/liability value", "Increase");

          case MyMoneyAccount::Income:
          case MyMoneyAccount::Expense:
            return i18n("Income");

          default:
            break;
        }
        break;

      case LedgerModel::DepositColumn:
        switch(m_accountType) {
          case MyMoneyAccount::CreditCard:
            return i18nc("Payment towards credit card", "Payment");

          case MyMoneyAccount::Asset:
          case MyMoneyAccount::AssetLoan:
            return i18nc("Increase of asset/liability value", "Increase");

          case MyMoneyAccount::Liability:
          case MyMoneyAccount::Loan:
            return i18nc("Decrease of asset/liability value", "Decrease");

          case MyMoneyAccount::Income:
          case MyMoneyAccount::Expense:
            return i18n("Expense");

          default:
            break;
        }
        break;
    }
  }
  return QSortFilterProxyModel::headerData(section, orientation, role);
}

bool LedgerSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  // make sure that the dummy transaction is shown last in any case
  if(left.data(LedgerRole::TransactionSplitIdRole).toString().isEmpty()) {
    return false;
  } else if(right.data(LedgerRole::TransactionSplitIdRole).toString().isEmpty()) {
    return true;
  }

  // make sure schedules are shown past real transactions
  if(!left.data(LedgerRole::ScheduleIdRole).toString().isEmpty()
  && right.data(LedgerRole::ScheduleIdRole).toString().isEmpty()) {
    // left is schedule, right is not
    return false;

  } else if(left.data(LedgerRole::ScheduleIdRole).toString().isEmpty()
         && !right.data(LedgerRole::ScheduleIdRole).toString().isEmpty()) {
    // right is schedule, left is not
    return true;
  }

  // otherwise use normal sorting
  return QSortFilterProxyModel::lessThan(left, right);
}

bool LedgerSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  if(m_showNewTransaction) {
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if(idx.data(LedgerRole::TransactionSplitIdRole).toString().isEmpty()) {
      return true;
    }
  }
  return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool LedgerSortFilterProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  QModelIndex sourceIndex = mapToSource(index);
  return sourceModel()->setData(sourceIndex, value, role);
}

void LedgerSortFilterProxyModel::setShowNewTransaction(bool show)
{
  const bool changed = show != m_showNewTransaction;
  m_showNewTransaction = show;
  if(changed) {
    invalidate();
  }
}














struct LedgerModel::Private
{
  Private() {}

  ~Private() {
    for(int i = 0; i < m_ledgerItems.count(); ++i) {
      delete m_ledgerItems.at(i);
    }
  }
  MyMoneyTransaction    m_lastTransactionStored;
  QVector<LedgerItem*>  m_ledgerItems;
};



LedgerModel::LedgerModel(QObject* parent)
  : QAbstractTableModel(parent)
  , d(new Private)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  connect(file, SIGNAL(objectAdded(MyMoneyFile::notificationObjectT,MyMoneyObject*const)), this, SLOT(addTransaction(MyMoneyFile::notificationObjectT,MyMoneyObject*const)));
  connect(file, SIGNAL(objectModified(MyMoneyFile::notificationObjectT,MyMoneyObject*const)), this, SLOT(modifyTransaction(MyMoneyFile::notificationObjectT,MyMoneyObject*const)));
  connect(file, SIGNAL(objectRemoved(MyMoneyFile::notificationObjectT,QString)), this, SLOT(removeTransaction(MyMoneyFile::notificationObjectT,QString)));

  connect(file, SIGNAL(objectAdded(MyMoneyFile::notificationObjectT,MyMoneyObject*const)), this, SLOT(addSchedule(MyMoneyFile::notificationObjectT,MyMoneyObject*const)));
  connect(file, SIGNAL(objectModified(MyMoneyFile::notificationObjectT,MyMoneyObject*const)), this, SLOT(modifySchedule(MyMoneyFile::notificationObjectT,MyMoneyObject*const)));
  connect(file, SIGNAL(objectRemoved(MyMoneyFile::notificationObjectT,QString)), this, SLOT(removeSchedule(MyMoneyFile::notificationObjectT,QString)));
}

LedgerModel::~LedgerModel()
{
}

int LedgerModel::rowCount(const QModelIndex& parent) const
{
  // since the ledger model is a simple table model, we only
  // return the rowCount for the hiddenRootItem. and zero otherwise
  if(parent.isValid()) {
    return 0;
  }

  return d->m_ledgerItems.count();
}

int LedgerModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return NumberOfLedgerColumns;
}

Qt::ItemFlags LedgerModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags;

  if(!index.isValid())
    return flags;
  if(index.row() < 0 || index.row() >= d->m_ledgerItems.count())
    return flags;

  return d->m_ledgerItems[index.row()]->flags();
}


QVariant LedgerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case NumberColumn:
        return i18nc("Cheque Number", "No.");
      case DateColumn:
        return i18n("Date");
        break;
      case SecurityColumn:
        return i18n("Security");
        break;
      case CostCenterColumn:
        return i18n("CC");
        break;
      case DetailColumn:
        return i18n("Detail");
        break;
      case ReconciliationColumn:
        return i18n("C");
        break;
      case PaymentColumn:
        return i18nc("Payment made from account", "Payment");
        break;
      case DepositColumn:
        return i18nc("Deposit into account", "Deposit");
        break;
      case QuantityColumn:
        return i18n("Quantity");
        break;
      case PriceColumn:
        return i18n("Price");
        break;
      case AmountColumn:
        return i18n("Amount");
        break;
      case ValueColumn:
        return i18n("Value");
        break;
      case BalanceColumn:
        return i18n("Balance");
        break;
    }
  }
  else if(orientation == Qt::Vertical && role == Qt::SizeHintRole) {
    // as small as possible, so that the delegate has a chance
    // to override the information
    return QSize(10, 10);
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant LedgerModel::data(const QModelIndex& index, int role) const
{
  if(!index.isValid())
    return QVariant();
  if(index.row() < 0 || index.row() >= d->m_ledgerItems.count())
    return QVariant();

  QVariant rc;
  switch(role) {
    case Qt::DisplayRole:
      // make sure to never return any displayable text for the dummy entry
      if(!d->m_ledgerItems[index.row()]->transactionSplitId().isEmpty()) {
        switch(index.column()) {
          case NumberColumn:
            rc = d->m_ledgerItems[index.row()]->transactionNumber();
            break;
          case DateColumn:
            rc = QLocale().toString(d->m_ledgerItems[index.row()]->postDate(), QLocale::ShortFormat);
            break;
          case DetailColumn:
            rc = d->m_ledgerItems[index.row()]->counterAccount();
            break;
          case ReconciliationColumn:
            rc = d->m_ledgerItems[index.row()]->reconciliationStateShort();
            break;
          case PaymentColumn:
            rc = d->m_ledgerItems[index.row()]->payment();
            break;
          case DepositColumn:
            rc = d->m_ledgerItems[index.row()]->deposit();
            break;
          case AmountColumn:
            rc = d->m_ledgerItems[index.row()]->signedSharesAmount();
            break;
          case BalanceColumn:
            rc = d->m_ledgerItems[index.row()]->balance();
            break;
        }
      }
      break;

    case Qt::TextAlignmentRole:
      switch(index.column()) {
        case PaymentColumn:
        case DepositColumn:
        case AmountColumn:
        case BalanceColumn:
        case ValueColumn:
          rc = QVariant(Qt::AlignRight| Qt::AlignTop);
          break;
        case ReconciliationColumn:
          rc = QVariant(Qt::AlignHCenter | Qt::AlignTop);
          break;
        default:
          rc = QVariant(Qt::AlignLeft | Qt::AlignTop);
          break;
      }
      break;

    case Qt::BackgroundColorRole:
      if(d->m_ledgerItems[index.row()]->isImported()) {
        return KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionImported);
      }
      break;

    case LedgerRole::CounterAccountRole:
      rc = d->m_ledgerItems[index.row()]->counterAccount();
      break;

    case LedgerRole::SplitCountRole:
      rc = d->m_ledgerItems[index.row()]->splitCount();
      break;

    case LedgerRole::CostCenterIdRole:
      rc = d->m_ledgerItems[index.row()]->costCenterId();
      break;

    case LedgerRole::PostDateRole:
      rc = d->m_ledgerItems[index.row()]->postDate();
      break;

    case LedgerRole::PayeeNameRole:
      rc = d->m_ledgerItems[index.row()]->payeeName();
      break;

    case LedgerRole::PayeeIdRole:
      rc = d->m_ledgerItems[index.row()]->payeeId();
      break;

    case LedgerRole::AccountIdRole:
      rc = d->m_ledgerItems[index.row()]->accountId();
      break;

    case Qt::EditRole:
    case LedgerRole::TransactionSplitIdRole:
      rc = d->m_ledgerItems[index.row()]->transactionSplitId();
      break;

    case LedgerRole::TransactionIdRole:
      rc = d->m_ledgerItems[index.row()]->transactionId();
      break;

    case LedgerRole::ReconciliationRole:
      rc = d->m_ledgerItems[index.row()]->reconciliationState();
      break;

    case LedgerRole::ReconciliationRoleShort:
      rc = d->m_ledgerItems[index.row()]->reconciliationStateShort();
      break;

    case LedgerRole::ReconciliationRoleLong:
      rc = d->m_ledgerItems[index.row()]->reconciliationStateLong();
      break;

    case LedgerRole::SplitValueRole:
      rc.setValue(d->m_ledgerItems[index.row()]->value());
      break;

    case LedgerRole::SplitSharesRole:
      rc.setValue(d->m_ledgerItems[index.row()]->shares());
      break;

    case LedgerRole::ShareAmountRole:
      rc.setValue(d->m_ledgerItems[index.row()]->sharesAmount());
      break;

    case LedgerRole::ShareAmountSuffixRole:
      rc.setValue(d->m_ledgerItems[index.row()]->sharesSuffix());
      break;

    case LedgerRole::ScheduleIdRole:
      {
      LedgerSchedule* schedule = 0;
      schedule = dynamic_cast<LedgerSchedule*>(d->m_ledgerItems[index.row()]);
      if(schedule) {
        rc = schedule->scheduleId();
      }
      break;
    }

    case LedgerRole::MemoRole:
    case LedgerRole::SingleLineMemoRole:
      rc.setValue(d->m_ledgerItems[index.row()]->memo());
      if(role == LedgerRole::SingleLineMemoRole) {
        QString txt = rc.toString();
        // remove empty lines
        txt.replace("\n\n", "\n");
        // replace '\n' with ", "
        txt.replace('\n', ", ");
        rc.setValue(txt);
      }
      break;

    case LedgerRole::NumberRole:
      rc = d->m_ledgerItems[index.row()]->transactionNumber();
      break;

    case LedgerRole::ErroneousRole:
      rc = d->m_ledgerItems[index.row()]->isErroneous();
      break;

    case LedgerRole::ImportRole:
      rc = d->m_ledgerItems[index.row()]->isImported();
      break;

    case LedgerRole::CounterAccountIdRole:
      rc = d->m_ledgerItems[index.row()]->counterAccountId();
      break;

    case LedgerRole::TransactionCommodityRole:
      rc = d->m_ledgerItems[index.row()]->transactionCommodity();
      break;

    case LedgerRole::TransactionRole:
      rc.setValue(d->m_ledgerItems[index.row()]->transaction());
      break;

    case LedgerRole::SplitRole:
      rc.setValue(d->m_ledgerItems[index.row()]->split());
      break;
  }
  return rc;
}

bool LedgerModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(!index.isValid()) {
    return false;
  }
  if(role == Qt::DisplayRole && index.column() == BalanceColumn) {
    d->m_ledgerItems[index.row()]->setBalance(value.toString());
    return true;
  }
  qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  return QAbstractItemModel::setData(index, value, role);
}



void LedgerModel::unload()
{
  if(rowCount() > 0) {
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    for(int i = 0; i < rowCount(); ++i) {
      delete d->m_ledgerItems[i];
    }
    d->m_ledgerItems.clear();
    endRemoveRows();
  }
}

void LedgerModel::addTransactions(const QList< QPair<MyMoneyTransaction, MyMoneySplit> >& list)
{
  if(list.count() > 0) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + list.count() - 1);
    QList< QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
    for(it = list.constBegin(); it != list.constEnd(); ++it) {
      d->m_ledgerItems.append(new LedgerTransaction((*it).first, (*it).second));
    }
    endInsertRows();
  }
}

void LedgerModel::addTransaction(const LedgerTransaction& t)
{
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  d->m_ledgerItems.append(new LedgerTransaction(t.transaction(), t.split()));
  endInsertRows();
}

void LedgerModel::addTransaction(const QString& transactionSplitId)
{
  QRegExp transactionSplitIdExp("^(\\w+)-(\\w+)$");
  if(transactionSplitIdExp.exactMatch(transactionSplitId)) {
    const QString transactionId = transactionSplitIdExp.cap(1);
    const QString splitId = transactionSplitIdExp.cap(2);
    if(transactionId != d->m_lastTransactionStored.id()) {
      try {
        d->m_lastTransactionStored = MyMoneyFile::instance()->transaction(transactionId);
      } catch(MyMoneyException& e) {
        d->m_lastTransactionStored = MyMoneyTransaction();
      }
    }
    try {
      MyMoneySplit split = d->m_lastTransactionStored.splitById(splitId);
      beginInsertRows(QModelIndex(), rowCount(), rowCount());
      d->m_ledgerItems.append(new LedgerTransaction(d->m_lastTransactionStored, split));
      endInsertRows();
    } catch(MyMoneyException& e) {
      d->m_lastTransactionStored = MyMoneyTransaction();
    }
  }
}

void LedgerModel::addSchedules(const QList<MyMoneySchedule> & list, int previewPeriod)
{
  if(list.count() > 0) {
    QVector<LedgerItem*> newList;

    // create dummy entries for the scheduled transactions if sorted by postdate
    // show scheduled transactions which have a scheduled postdate
    // within the next 'previewPeriod' days. In reconciliation mode, the
    // previewPeriod starts on the statement date.
    QDate endDate = QDate::currentDate().addDays(previewPeriod);

#if 0
    if (isReconciliationAccount())
      endDate = reconciliationDate.addDays(previewPeriod);
#endif

    QList<MyMoneySchedule>::const_iterator it;
    for(it = list.constBegin(); it != list.constEnd(); ++it) {
      MyMoneySchedule schedule = *it;

      // now create entries for this schedule until the endDate is reached
      for (;;) {
        if (schedule.isFinished() || schedule.adjustedNextDueDate() > endDate) {
          break;
        }

        MyMoneyTransaction t(schedule.id(), KMyMoneyUtils::scheduledTransaction(schedule));
        // if the transaction is scheduled and overdue, it can't
        // certainly be posted in the past. So we take today's date
        // as the alternative
        if (schedule.isOverdue()) {
          t.setPostDate(schedule.adjustedDate(QDate::currentDate(), schedule.weekendOption()));
        } else {
          t.setPostDate(schedule.adjustedNextDueDate());
        }

        const QList<MyMoneySplit>& splits = t.splits();
        QList<MyMoneySplit>::const_iterator it_s;

        // create a model entry for each split of the schedule
        for(it_s = splits.constBegin(); it_s != splits.constEnd(); ++it_s) {
          newList.append(new LedgerSchedule(schedule, t, (*it_s)));
        }

        // keep track of this payment locally (not in the engine)
        if (schedule.isOverdue()) {
          schedule.setLastPayment(QDate::currentDate());
        } else {
          schedule.setLastPayment(schedule.nextDueDate());
        }

        // if this is a one time schedule, we can bail out here as we're done
        if (schedule.occurrence() == MyMoneySchedule::OCCUR_ONCE)
          break;

        // for all others, we check if the next payment date is still 'in range'
        QDate nextDueDate = schedule.nextPayment(schedule.nextDueDate());
        if (nextDueDate.isValid()) {
          schedule.setNextDueDate(nextDueDate);
        } else {
          break;
        }
      }
    }
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + newList.count() - 1);
    d->m_ledgerItems += newList;
    endInsertRows();
  }
}

void LedgerModel::load()
{
  qDebug() << "Start loading splits";
  // load all transactions and splits into the model
  QList<QPair<MyMoneyTransaction, MyMoneySplit> > tList;
  MyMoneyTransactionFilter filter;
  MyMoneyFile::instance()->transactionList(tList, filter);
  addTransactions(tList);
  qDebug() << "Loaded" << rowCount() << "elements";

  // load all scheduled transactoins and splits into the model
  const int splitCount = rowCount();
  QList<MyMoneySchedule> sList = MyMoneyFile::instance()->scheduleList();
  addSchedules(sList, KMyMoneyGlobalSettings::schedulePreview());
  qDebug() << "Loaded" << rowCount()-splitCount << "elements";

  // create a dummy entry for new transactions
  addTransaction(LedgerTransaction::newTransactionEntry());

  qDebug() << "Loaded" << rowCount() << "elements";
}

void LedgerModel::addTransaction(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj)
{
  if(objType != MyMoneyFile::notifyTransaction) {
    return;
  }

  qDebug() << "Adding transaction" << obj->id();

  const MyMoneyTransaction * const t = static_cast<const MyMoneyTransaction * const>(obj);

  beginInsertRows(QModelIndex(), rowCount(), rowCount() + t->splitCount() - 1);
  foreach(MyMoneySplit s, t->splits()) {
    d->m_ledgerItems.append(new LedgerTransaction(*t, s));
  }
  endInsertRows();

  // just make sure we're in sync
  Q_ASSERT(d->m_ledgerItems.count() == rowCount());
}

void LedgerModel::modifyTransaction(MyMoneyFile::notificationObjectT objType, const MyMoneyObject* const obj)
{
  if(objType != MyMoneyFile::notifyTransaction) {
    return;
  }

  const MyMoneyTransaction * const t = static_cast<const MyMoneyTransaction * const>(obj);
  // get indexes of all existing splits for this transaction
  QModelIndexList list = match(index(0, 0), LedgerRole::TransactionIdRole, obj->id(), -1);
  // get list of splits to be stored
  QList<MyMoneySplit> splits = t->splits();

  int lastRowUsed = -1;
  int firstRowUsed = 99999999;
  if(list.count()) {
    firstRowUsed = list.first().row();
    lastRowUsed = list.last().row();
  }

  qDebug() << "first:" << firstRowUsed << "last:" << lastRowUsed;

  while(!list.isEmpty() && !splits.isEmpty()) {
    QModelIndex index = list.takeFirst();
    MyMoneySplit split = splits.takeFirst();
    // get rid of the old split and store new split
    qDebug() << "Modify split in row:" << index.row() << t->id() << split.id();
    delete d->m_ledgerItems[index.row()];
    d->m_ledgerItems[index.row()] = new LedgerTransaction(*t, split);
  }

  // inform every one else about the changes
  if(lastRowUsed != -1) {
    qDebug() << "emit dataChanged from" << firstRowUsed << "to" << lastRowUsed;
    emit dataChanged(index(firstRowUsed, 0), index(lastRowUsed, columnCount()-1));

  } else {
    lastRowUsed = rowCount();
  }

  // now check if we need to add more splits ...
  if(!splits.isEmpty() && list.isEmpty()) {
    beginInsertRows(QModelIndex(), lastRowUsed, lastRowUsed + splits.count() - 1);
    d->m_ledgerItems.insert(lastRowUsed, splits.count(), 0);
    while(!splits.isEmpty()) {
      MyMoneySplit split = splits.takeFirst();
      d->m_ledgerItems[lastRowUsed] = new LedgerTransaction(*t, split);
      lastRowUsed++;
    }
    endInsertRows();
  }

  // ... or remove some leftovers
  if(splits.isEmpty() && !list.isEmpty()) {
    firstRowUsed = lastRowUsed - list.count() + 1;
    beginRemoveRows(QModelIndex(), firstRowUsed, lastRowUsed);
    int count = 0;
    while(!list.isEmpty()) {
      ++count;
      QModelIndex index = list.takeFirst();
      // get rid of the old split and store new split
      qDebug() << "Delete split in row:" << index.row() << data(index, LedgerRole::TransactionSplitIdRole).toString();
      delete d->m_ledgerItems[index.row()];
    }
    d->m_ledgerItems.remove(firstRowUsed, count);
    endRemoveRows();
  }

  // just make sure we're in sync
  Q_ASSERT(d->m_ledgerItems.count() == rowCount());
}

void LedgerModel::removeTransaction(MyMoneyFile::notificationObjectT objType, const QString& id)
{
  if(objType != MyMoneyFile::notifyTransaction) {
    return;
  }

  QModelIndexList list = match(index(0, 0), LedgerRole::TransactionIdRole, id, -1);

  if(list.count()) {
    const int firstRowUsed = list[0].row();
    beginRemoveRows(QModelIndex(), firstRowUsed, firstRowUsed + list.count() - 1);
    for(int row = firstRowUsed; row < firstRowUsed + list.count(); ++row) {
      delete d->m_ledgerItems[row];
    }
    d->m_ledgerItems.remove(firstRowUsed, list.count());
    endRemoveRows();

    // just make sure we're in sync
    Q_ASSERT(d->m_ledgerItems.count() == rowCount());
  }
}

void LedgerModel::addSchedule(MyMoneyFile::notificationObjectT objType, const MyMoneyObject*const obj)
{
  Q_UNUSED(obj);
  if(objType != MyMoneyFile::notifySchedule) {
    return;
  }

  /// @todo implement LedgerModel::addSchedule
}

void LedgerModel::modifySchedule(MyMoneyFile::notificationObjectT objType, const MyMoneyObject*const obj)
{
  Q_UNUSED(obj);
  if(objType != MyMoneyFile::notifySchedule) {
    return;
  }

  /// @todo implement LedgerModel::modifySchedule
}

void LedgerModel::removeSchedule(MyMoneyFile::notificationObjectT objType, const QString& id)
{
  Q_UNUSED(id);
  if(objType != MyMoneyFile::notifySchedule) {
    return;
  }

  /// @todo implement LedgerModel::removeSchedule
}

QString LedgerModel::transactionIdFromTransactionSplitId(const QString& transactionSplitId) const
{
  QRegExp transactionSplitIdExp("^(\\w+)-\\w+$");
  if(transactionSplitIdExp.exactMatch(transactionSplitId)) {
    return transactionSplitIdExp.cap(1);
  }
  return QString();
}














struct SplitModel::Private
{
  Private()
  : m_invertValues(false)
  {}

  bool isCreateSplitEntry(const QString& id) const {
    return id.isEmpty();
  }

  MyMoneyTransaction    m_transaction;
  QVector<MyMoneySplit> m_splits;
  bool                  m_invertValues;
};




SplitModel::SplitModel(QObject* parent)
  : QAbstractTableModel(parent)
  , d(new Private)
{
}

SplitModel::~SplitModel()
{
}


QString SplitModel::newSplitId()
{
  return QLatin1String("New-ID");
}

bool SplitModel::isNewSplitId(const QString& id)
{
  return id.compare(newSplitId()) == 0;
}


int SplitModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return d->m_splits.count();
}

int SplitModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return NumberOfLedgerColumns;
}


void SplitModel::deepCopy(const SplitModel& right, bool revertSplitSign)
{
  beginInsertRows(QModelIndex(), 0, right.rowCount());
  d->m_splits = right.d->m_splits;
  d->m_transaction = right.d->m_transaction;
  if(revertSplitSign) {
    for(int idx = 0; idx < d->m_splits.count(); ++idx) {
      MyMoneySplit& split = d->m_splits[idx];
      split.setShares(-split.shares());
      split.setValue(-split.value());
    }
  }
  endInsertRows();
}

QVariant SplitModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case CostCenterColumn:
        return i18n("Cost Center");
        break;
      case DetailColumn:
        return i18n("Category");
        break;
      case NumberColumn:
        return i18n("No");
        break;
      case DateColumn:
        return i18n("Date");
        break;
      case SecurityColumn:
        return i18n("Security");
        break;
      case ReconciliationColumn:
        return i18n("C");
        break;
      case PaymentColumn:
        return i18n("Payment");
        break;
      case DepositColumn:
        return i18n("Deposit");
        break;
      case QuantityColumn:
        return i18n("Quantity");
        break;
      case PriceColumn:
        return i18n("Price");
        break;
      case AmountColumn:
        return i18n("Amount");
        break;
      case ValueColumn:
        return i18n("Value");
        break;
      case BalanceColumn:
        return i18n("Balance");
        break;
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant SplitModel::data(const QModelIndex& index, int role) const
{
  if(!index.isValid())
    return QVariant();
  if(index.row() < 0 || index.row() >= d->m_splits.count())
    return QVariant();

  QVariant rc;
  MyMoneyAccount acc;
  MyMoneyMoney value;
  const MyMoneySplit& split = d->m_splits[index.row()];
  QModelIndex subIndex;
  CostCenterModel* ccModel = Models::instance()->costCenterModel();

  switch(role) {
    case Qt::DisplayRole:
      // make sure to never return any displayable text for the dummy entry
      if(!d->isCreateSplitEntry(split.id())) {
        switch(index.column()) {
          case DetailColumn:
            rc = MyMoneyFile::instance()->accountToCategory(split.accountId());
            break;
          case CostCenterColumn:
            subIndex = Models::indexById(ccModel, CostCenterModel::CostCenterIdRole, split.costCenterId());
            rc = ccModel->data(subIndex);
            break;
          case NumberColumn:
            rc = split.number();
            break;
          case ReconciliationColumn:
            rc = KMyMoneyUtils::reconcileStateToString(split.reconcileFlag(), false);
            break;
          case PaymentColumn:
            if(split.value().isNegative()) {
              acc = MyMoneyFile::instance()->account(split.accountId());
              rc = (-split).value(d->m_transaction.commodity(), acc.currencyId()).formatMoney(acc.fraction());
            }
            break;
          case DepositColumn:
            if(!split.value().isNegative()) {
              acc = MyMoneyFile::instance()->account(split.accountId());
              rc = split.value(d->m_transaction.commodity(), acc.currencyId()).formatMoney(acc.fraction());
            }
            break;
          default:
            break;
        }
      }
      break;

    case Qt::TextAlignmentRole:
      switch(index.column()) {
        case PaymentColumn:
        case DepositColumn:
        case AmountColumn:
        case BalanceColumn:
        case ValueColumn:
          rc = QVariant(Qt::AlignRight| Qt::AlignTop);
          break;
        case ReconciliationColumn:
          rc = QVariant(Qt::AlignHCenter | Qt::AlignTop);
          break;
        default:
          rc = QVariant(Qt::AlignLeft | Qt::AlignTop);
          break;
      }
      break;

    case LedgerRole::AccountIdRole:
      rc = split.accountId();
      break;

    case LedgerRole::AccountRole:
      rc = MyMoneyFile::instance()->accountToCategory(split.accountId());
      break;

    case LedgerRole::TransactionIdRole:
      rc = QString("%1").arg(d->m_transaction.id());
      break;

    case LedgerRole::TransactionSplitIdRole:
      rc = QString("%1-%2").arg(d->m_transaction.id()).arg(split.id());
      break;

    case LedgerRole::SplitIdRole:
      rc = split.id();
      break;

    case LedgerRole::MemoRole:
    case LedgerRole::SingleLineMemoRole:
      rc = split.memo();
      if(role == LedgerRole::SingleLineMemoRole) {
        QString txt = rc.toString();
        // remove empty lines
        txt.replace("\n\n", "\n");
        // replace '\n' with ", "
        txt.replace('\n', ", ");
        rc = txt;
      }
      break;

    case LedgerRole::SplitSharesRole:
      rc = QVariant::fromValue<MyMoneyMoney>(split.shares());
      break;

    case LedgerRole::SplitValueRole:
      acc = MyMoneyFile::instance()->account(split.accountId());
      rc = QVariant::fromValue<MyMoneyMoney>(split.value(d->m_transaction.commodity(), acc.currencyId()));
      break;

    case LedgerRole::PayeeNameRole:
      try {
        rc = MyMoneyFile::instance()->payee(split.payeeId()).name();
      } catch(MyMoneyException&e) {
      }
      break;

    case LedgerRole::CostCenterIdRole:
      rc = split.costCenterId();
      break;

    case LedgerRole::TransactionCommodityRole:
      return d->m_transaction.commodity();
      break;

    case LedgerRole::NumberRole:
      rc = split.number();
      break;

    case LedgerRole::PayeeIdRole:
      rc = split.payeeId();
      break;

    default:
      if(role >= Qt::UserRole) {
        qWarning() << "Undefined role" << role << "(" << role-Qt::UserRole << ") in SplitModel::data";
      }
      break;
  }
  return rc;
}

bool SplitModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  bool rc = false;
  if(index.isValid()) {
    MyMoneySplit& split = d->m_splits[index.row()];
    if(split.id().isEmpty()) {
      split = MyMoneySplit(newSplitId(), split);
    }
    QString val;
    rc = true;
    switch(role) {
      case LedgerRole::PayeeIdRole:
        split.setPayeeId(value.toString());
        break;

      case LedgerRole::AccountIdRole:
        split.setAccountId(value.toString());
        break;

      case LedgerRole::MemoRole:
        split.setMemo(value.toString());
        break;

      case LedgerRole::CostCenterIdRole:
        val = value.toString();
        split.setCostCenterId(value.toString());
        break;

      case LedgerRole::NumberRole:
        split.setNumber(value.toString());
        break;

      case LedgerRole::SplitSharesRole:
        split.setShares(value.value<MyMoneyMoney>());
        break;

      case LedgerRole::SplitValueRole:
        split.setValue(value.value<MyMoneyMoney>());
        break;

      case LedgerRole::EmitDataChangedRole:
        {
          // the whole row changed
          QModelIndex topLeft = this->index(index.row(), 0);
          QModelIndex bottomRight = this->index(index.row(), this->columnCount()-1);
          emit dataChanged(topLeft, bottomRight);
        }
        break;

      default:
        rc = false;
        break;
    }
  }

  return rc;
}


void SplitModel::addSplit(const QString& transactionSplitId)
{
  QRegExp transactionSplitIdExp("^(\\w+)-(\\w+)$");
  if(transactionSplitIdExp.exactMatch(transactionSplitId)) {
    const QString transactionId = transactionSplitIdExp.cap(1);
    const QString splitId = transactionSplitIdExp.cap(2);
    if(transactionId != d->m_transaction.id()) {
      try {
        d->m_transaction = MyMoneyFile::instance()->transaction(transactionId);
      } catch(MyMoneyException& e) {
        d->m_transaction = MyMoneyTransaction();
      }
    }
    try {
      beginInsertRows(QModelIndex(), rowCount(), rowCount());
      d->m_splits.append(d->m_transaction.splitById(splitId));
      endInsertRows();
    } catch(MyMoneyException& e) {
      d->m_transaction = MyMoneyTransaction();
    }
  }
}

void SplitModel::addEmptySplitEntry()
{
  QModelIndexList list = match(index(0, 0), LedgerRole::SplitIdRole, QString(), -1, Qt::MatchExactly);
  if(list.count() == 0) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    // d->m_splits.append(MyMoneySplit(d->newSplitEntryId(), MyMoneySplit()));
    d->m_splits.append(MyMoneySplit());
    endInsertRows();
  }
}

void SplitModel::removeEmptySplitEntry()
{
  // QModelIndexList list = match(index(0, 0), SplitIdRole, d->newSplitEntryId(), -1, Qt::MatchExactly);
  QModelIndexList list = match(index(0, 0), LedgerRole::SplitIdRole, QString(), -1, Qt::MatchExactly);
  if(list.count()) {
    QModelIndex index = list.at(0);
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    d->m_splits.remove(index.row(), 1);
    endRemoveRows();
  }
}

bool SplitModel::removeRows(int row, int count, const QModelIndex& parent)
{
  bool rc = false;
  if(count > 0) {
    beginRemoveRows(parent, row, row + count - 1);
    d->m_splits.remove(row, count);
    endRemoveRows();
    rc = true;
  }
  return rc;
}

Qt::ItemFlags SplitModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags;

  if(!index.isValid())
    return flags;
  if(index.row() < 0 || index.row() >= d->m_splits.count())
    return flags;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}


#if 0
void SplitModel::removeSplit(const LedgerTransaction& t)
{
  QModelIndexList list = match(index(0, 0), TransactionSplitIdRole, t.transactionSplitId(), -1, Qt::MatchExactly);
  if(list.count()) {
    QModelIndex index = list.at(0);
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    delete d->m_ledgerItems[index.row()];
    d->m_ledgerItems.remove(index.row(), 1);
    endRemoveRows();

    // just make sure we're in sync
    Q_ASSERT(d->m_ledgerItems.count() == rowCount());
  }
}
#endif
