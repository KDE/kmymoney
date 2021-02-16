/*
    SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgermodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerschedule.h"

#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyexception.h"
#include "kmymoneyutils.h"
#include "kmymoneysettings.h"
#include "mymoneyenums.h"
#include "modelenums.h"

using namespace eLedgerModel;
using namespace eMyMoney;

class LedgerModelPrivate
{
public:
  ~LedgerModelPrivate() {
    qDeleteAll(m_ledgerItems);
    m_ledgerItems.clear();
  }
  MyMoneyTransaction    m_lastTransactionStored;
  QVector<LedgerItem*>  m_ledgerItems;
};

LedgerModel::LedgerModel(QObject* parent) :
  QAbstractTableModel(parent),
  d_ptr(new LedgerModelPrivate)
{
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

  Q_D(const LedgerModel);
  return d->m_ledgerItems.count();
}

int LedgerModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return (int)Column::LastColumn;
}

Qt::ItemFlags LedgerModel::flags(const QModelIndex& index) const
{
  Q_D(const LedgerModel);
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
      case (int)Column::Number:
        return i18nc("Cheque Number", "No.");
      case (int)Column::Date:
        return i18n("Date");
      case (int)Column::Security:
        return i18n("Security");
      case (int)Column::CostCenter:
        return i18n("CC");
      case (int)Column::Detail:
        return i18n("Detail");
      case (int)Column::Reconciliation:
        return i18n("C");
      case (int)Column::Payment:
        return i18nc("Payment made from account", "Payment");
      case (int)Column::Deposit:
        return i18nc("Deposit into account", "Deposit");
      case (int)Column::Quantity:
        return i18n("Quantity");
      case (int)Column::Price:
        return i18n("Price");
      case (int)Column::Amount:
        return i18n("Amount");
      case (int)Column::Value:
        return i18n("Value");
      case (int)Column::Balance:
        return i18n("Balance");
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
  Q_D(const LedgerModel);
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
          case (int)Column::Number:
            rc = d->m_ledgerItems[index.row()]->transactionNumber();
            break;
          case (int)Column::Date:
            rc = QLocale().toString(d->m_ledgerItems[index.row()]->postDate(), QLocale::ShortFormat);
            break;
          case (int)Column::Detail:
            rc = d->m_ledgerItems[index.row()]->counterAccount();
            break;
          case (int)Column::Reconciliation:
            rc = d->m_ledgerItems[index.row()]->reconciliationStateShort();
            break;
          case (int)Column::Payment:
            rc = d->m_ledgerItems[index.row()]->payment();
            break;
          case (int)Column::Deposit:
            rc = d->m_ledgerItems[index.row()]->deposit();
            break;
          case (int)Column::Amount:
            rc = d->m_ledgerItems[index.row()]->signedSharesAmount();
            break;
          case (int)Column::Balance:
            rc = d->m_ledgerItems[index.row()]->balance();
            break;
        }
      }
      break;

    case Qt::TextAlignmentRole:
      switch(index.column()) {
        case (int)Column::Payment:
        case (int)Column::Deposit:
        case (int)Column::Amount:
        case (int)Column::Balance:
        case (int)Column::Value:
          rc = QVariant(Qt::AlignRight| Qt::AlignTop);
          break;
        case (int)Column::Reconciliation:
          rc = QVariant(Qt::AlignHCenter | Qt::AlignTop);
          break;
        default:
          rc = QVariant(Qt::AlignLeft | Qt::AlignTop);
          break;
      }
      break;

    case Qt::BackgroundColorRole:
      if(d->m_ledgerItems[index.row()]->isImported()) {
        return KMyMoneySettings::schemeColor(SchemeColor::TransactionImported);
      }
      break;

    case (int)Role::CounterAccount:
      rc = d->m_ledgerItems[index.row()]->counterAccount();
      break;

    case (int)Role::SplitCount:
      rc = d->m_ledgerItems[index.row()]->splitCount();
      break;

    case (int)Role::CostCenterId:
      rc = d->m_ledgerItems[index.row()]->costCenterId();
      break;

    case (int)Role::PostDate:
      rc = d->m_ledgerItems[index.row()]->postDate();
      break;

    case (int)Role::PayeeName:
      rc = d->m_ledgerItems[index.row()]->payeeName();
      break;

    case (int)Role::PayeeId:
      rc = d->m_ledgerItems[index.row()]->payeeId();
      break;

    case (int)Role::AccountId:
      rc = d->m_ledgerItems[index.row()]->accountId();
      break;

    case Qt::EditRole:
    case (int)Role::TransactionSplitId:
      rc = d->m_ledgerItems[index.row()]->transactionSplitId();
      break;

    case (int)Role::TransactionId:
      rc = d->m_ledgerItems[index.row()]->transactionId();
      break;

    case (int)Role::Reconciliation:
      rc = (int)d->m_ledgerItems[index.row()]->reconciliationState();
      break;

    case (int)Role::ReconciliationShort:
      rc = d->m_ledgerItems[index.row()]->reconciliationStateShort();
      break;

    case (int)Role::ReconciliationLong:
      rc = d->m_ledgerItems[index.row()]->reconciliationStateLong();
      break;

    case (int)Role::SplitValue:
      rc.setValue(d->m_ledgerItems[index.row()]->value());
      break;

    case (int)Role::SplitShares:
      rc.setValue(d->m_ledgerItems[index.row()]->shares());
      break;

    case (int)Role::ShareAmount:
      rc.setValue(d->m_ledgerItems[index.row()]->sharesAmount());
      break;

    case (int)Role::ShareAmountSuffix:
      rc.setValue(d->m_ledgerItems[index.row()]->sharesSuffix());
      break;

    case (int)Role::ScheduleId:
      {
      LedgerSchedule* schedule = 0;
      schedule = dynamic_cast<LedgerSchedule*>(d->m_ledgerItems[index.row()]);
      if(schedule) {
        rc = schedule->scheduleId();
      }
      break;
    }

    case (int)Role::Memo:
    case (int)Role::SingleLineMemo:
      rc.setValue(d->m_ledgerItems[index.row()]->memo());
      if(role == (int)Role::SingleLineMemo) {
        QString txt = rc.toString();
        // remove empty lines
        txt.replace("\n\n", "\n");
        // replace '\n' with ", "
        txt.replace('\n', ", ");
        rc.setValue(txt);
      }
      break;

    case (int)Role::Number:
      rc = d->m_ledgerItems[index.row()]->transactionNumber();
      break;

    case (int)Role::Erroneous:
      rc = d->m_ledgerItems[index.row()]->isErroneous();
      break;

    case (int)Role::Import:
      rc = d->m_ledgerItems[index.row()]->isImported();
      break;

    case (int)Role::CounterAccountId:
      rc = d->m_ledgerItems[index.row()]->counterAccountId();
      break;

    case (int)Role::TransactionCommodity:
      rc = d->m_ledgerItems[index.row()]->transactionCommodity();
      break;

    case (int)Role::Transaction:
      rc.setValue(d->m_ledgerItems[index.row()]->transaction());
      break;

    case (int)Role::Split:
      rc.setValue(d->m_ledgerItems[index.row()]->split());
      break;
  }
  return rc;
}

bool LedgerModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  Q_D(LedgerModel);
  if(!index.isValid()) {
    return false;
  }
  if(role == Qt::DisplayRole && index.column() == (int)Column::Balance) {
    d->m_ledgerItems[index.row()]->setBalance(value.toString());
    return true;
  }
  qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  return QAbstractItemModel::setData(index, value, role);
}



void LedgerModel::unload()
{
  Q_D(LedgerModel);
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
  Q_D(LedgerModel);
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
  Q_D(LedgerModel);
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  d->m_ledgerItems.append(new LedgerTransaction(t.transaction(), t.split()));
  endInsertRows();
}

void LedgerModel::addTransaction(const QString& transactionSplitId)
{
  Q_D(LedgerModel);
  QRegExp transactionSplitIdExp("^(\\w+)-(\\w+)$");
  if(transactionSplitIdExp.exactMatch(transactionSplitId)) {
    const QString transactionId = transactionSplitIdExp.cap(1);
    const QString splitId = transactionSplitIdExp.cap(2);
    if(transactionId != d->m_lastTransactionStored.id()) {
      try {
        d->m_lastTransactionStored = MyMoneyFile::instance()->transaction(transactionId);
      } catch (const MyMoneyException &) {
        d->m_lastTransactionStored = MyMoneyTransaction();
      }
    }
    try {
      MyMoneySplit split = d->m_lastTransactionStored.splitById(splitId);
      beginInsertRows(QModelIndex(), rowCount(), rowCount());
      d->m_ledgerItems.append(new LedgerTransaction(d->m_lastTransactionStored, split));
      endInsertRows();
    } catch (const MyMoneyException &) {
      d->m_lastTransactionStored = MyMoneyTransaction();
    }
  }
}

void LedgerModel::addSchedules(const QList<MyMoneySchedule> & list, int previewPeriod)
{
  Q_D(LedgerModel);
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

        // create a model entry for each split of the schedule
        foreach (const auto split, t.splits())
          newList.append(new LedgerSchedule(schedule, t, split));

        // keep track of this payment locally (not in the engine)
        if (schedule.isOverdue()) {
          schedule.setLastPayment(QDate::currentDate());
        } else {
          schedule.setLastPayment(schedule.nextDueDate());
        }

        // if this is a one time schedule, we can bail out here as we're done
        if (schedule.occurrence() == Schedule::Occurrence::Once)
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
    if(!newList.isEmpty()) {
      beginInsertRows(QModelIndex(), rowCount(), rowCount() + newList.count() - 1);
      d->m_ledgerItems += newList;
      endInsertRows();
    }
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

  // load all scheduled transactions and splits into the model
  const int splitCount = rowCount();
  QList<MyMoneySchedule> sList = MyMoneyFile::instance()->scheduleList();
  addSchedules(sList, KMyMoneySettings::schedulePreview());
  qDebug() << "Loaded" << rowCount()-splitCount << "elements";

  // create a dummy entry for new transactions
  addTransaction(LedgerTransaction::newTransactionEntry());

  qDebug() << "Loaded" << rowCount() << "elements";
}

void LedgerModel::slotAddTransaction(File::Object objType, const QString& id)
{
  if(objType != File::Object::Transaction) {
    return;
  }
  Q_D(LedgerModel);
  qDebug() << "Adding transaction" << id;

  const auto t = MyMoneyFile::instance()->transaction(id);

  beginInsertRows(QModelIndex(), rowCount(), rowCount() + t.splitCount() - 1);
  foreach (auto s, t.splits())
    d->m_ledgerItems.append(new LedgerTransaction(t, s));
  endInsertRows();

  // just make sure we're in sync
  Q_ASSERT(d->m_ledgerItems.count() == rowCount());
}

void LedgerModel::slotModifyTransaction(File::Object objType, const QString& id)
{
  if(objType != File::Object::Transaction) {
    return;
  }

  Q_D(LedgerModel);
  const auto t = MyMoneyFile::instance()->transaction(id);
  // get indexes of all existing splits for this transaction
  auto list = match(index(0, 0), (int)Role::TransactionId, id, -1);
  // get list of splits to be stored
  auto splits = t.splits();

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
    qDebug() << "Modify split in row:" << index.row() << t.id() << split.id();
    delete d->m_ledgerItems[index.row()];
    d->m_ledgerItems[index.row()] = new LedgerTransaction(t, split);
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
      d->m_ledgerItems[lastRowUsed] = new LedgerTransaction(t, split);
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
      qDebug() << "Delete split in row:" << index.row() << data(index, (int)Role::TransactionSplitId).toString();
      delete d->m_ledgerItems[index.row()];
    }
    d->m_ledgerItems.remove(firstRowUsed, count);
    endRemoveRows();
  }

  // just make sure we're in sync
  Q_ASSERT(d->m_ledgerItems.count() == rowCount());
}

void LedgerModel::slotRemoveTransaction(File::Object objType, const QString& id)
{
  if(objType != File::Object::Transaction) {
    return;
  }
  Q_D(LedgerModel);

  QModelIndexList list = match(index(0, 0), (int)Role::TransactionId, id, -1);

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

void LedgerModel::slotAddSchedule(File::Object objType, const QString& id)
{
  Q_UNUSED(id);
  if(objType != File::Object::Schedule) {
    return;
  }

  /// @todo implement LedgerModel::addSchedule
}

void LedgerModel::slotModifySchedule(File::Object objType, const QString& id)
{
  Q_UNUSED(id);
  if(objType != File::Object::Schedule) {
    return;
  }

  /// @todo implement LedgerModel::modifySchedule
}

void LedgerModel::slotRemoveSchedule(File::Object objType, const QString& id)
{
  Q_UNUSED(id);
  if(objType != File::Object::Schedule) {
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
