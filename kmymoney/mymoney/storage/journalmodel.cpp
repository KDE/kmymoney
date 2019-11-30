/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#include "journalmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>
#include <QDate>
#include <QSize>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "accountsmodel.h"
#include "costcentermodel.h"
#include "payeesmodel.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneymoney.h"
#include "mymoneytransactionfilter.h"

struct JournalModel::Private
{
  Private(JournalModel* qq)
    : q(qq)
    , newTransactionModel(nullptr)
    , headerData(QHash<Column, QString> ({
      { Number, i18nc("Cheque Number", "No.") },
      { Date, i18n("Date") },
      { Account, i18n("Account") },
      { Security, i18n("Security") },
      { CostCenter, i18n("CC") },
      { Detail, i18n("Detail") },
      { Reconciliation, i18n("C") },
      { Payment, i18nc("Payment made from account", "Payment") },
      { Deposit, i18nc("Deposit into account", "Deposit") },
      { Quantity, i18n("Quantity") },
      { Price, i18n("Price") },
      { Amount, i18n("Amount") },
      { Value, i18n("Value") },
      { Balance, i18n("Balance") },
    }))
  {
  }

  QString reconciliationStateShort(eMyMoney::Split::State reconcileState) const
  {
    switch(reconcileState) {
      case eMyMoney::Split::State::NotReconciled:
      default:
        break;
      case eMyMoney::Split::State::Cleared:
        return i18nc("Reconciliation flag C", "C");
      case eMyMoney::Split::State::Reconciled:
        return i18nc("Reconciliation flag R", "R");
      case eMyMoney::Split::State::Frozen:
        return i18nc("Reconciliation flag F", "F");
    }
    return QString();
  }

  QString reconciliationStateLong(eMyMoney::Split::State reconcileState) const
  {
    switch(reconcileState) {
      case eMyMoney::Split::State::NotReconciled:
      default:
        return i18nc("Reconciliation flag empty", "Not reconciled");
      case eMyMoney::Split::State::Cleared:
        return i18nc("Reconciliation flag C", "Cleared");
      case eMyMoney::Split::State::Reconciled:
        return i18nc("Reconciliation flag R", "Reconciled");
      case eMyMoney::Split::State::Frozen:
        return i18nc("Reconciliation flag F", "Frozen");
    }
    return QString();
  }

  QString counterAccountId(const JournalEntry& journalEntry, const MyMoneyTransaction& transaction)
  {
    if (transaction.splitCount() == 2) {
      const auto& splitId = journalEntry.split().id();
      for (const auto& sp : transaction.splits()) {
        if(splitId != sp.id()) {
          return sp.accountId();
        }
      }
    }
    return {};
  }

  QString counterAccount(const JournalEntry& journalEntry, const MyMoneyTransaction& transaction)
  {
    // A transaction can have more than 2 splits ...
    if(transaction.splitCount() > 2) {
      return i18n("Split transaction");

      // ... exactly two splits ...
    } else if(transaction.splitCount() == 2) {
      const auto& splitId = journalEntry.split().id();
      for (const auto& sp : transaction.splits()) {
        if(splitId != sp.id()) {
          return MyMoneyFile::instance()->accountsModel()->accountIdToHierarchicalName(sp.accountId());
        }
      }

      // ... or a single split
    } else if(!journalEntry.split().shares().isZero()) {
      return i18n("*** UNASSIGNED ***");
    }
    return QString();
  }

  void removeIdKeyMapping(const QString& id)
  {
    transactionIdKeyMap.remove(id);
  }

  void addIdKeyMapping(const QString& id, const QString& key)
  {
    transactionIdKeyMap[id] = key;
  }

  QString mapIdToKey(const QString& id) const
  {
    return transactionIdKeyMap.value(id);
  }

  void loadAccountCache()
  {
    accountCache.clear();
    const auto accountList = MyMoneyFile::instance()->accountsModel()->itemList();
    foreach(const auto& acc, accountList) {
      accountCache[acc.id()] = acc;
    }
  }

  void startBalanceCacheOperation()
  {
    balanceChangedSet.clear();
    fullBalanceRecalc.clear();
  }

  void removeTransactionFromBalance(int startRow, int rows)
  {
    for (int row = 0; row < rows; ++row)  {
      const auto journalEntry = static_cast<TreeItem<JournalEntry>*>(q->index(startRow, 0).internalPointer())->constDataRef();
      balanceChangedSet.insert(journalEntry.split().accountId());
      if (Q_UNLIKELY(journalEntry.transaction().isStockSplit())) {
        fullBalanceRecalc.insert(journalEntry.split().accountId());
      } else {
        balanceCache[journalEntry.split().accountId()] -= journalEntry.split().shares();
      }
      ++startRow;
    }
  }

  void addTransactionToBalance(int startRow, int rows)
  {
    for (int row = 0; row < rows; ++row)  {
      const auto journalEntry = static_cast<TreeItem<JournalEntry>*>(q->index(startRow, 0).internalPointer())->constDataRef();
      balanceChangedSet.insert(journalEntry.split().accountId());
      if (Q_UNLIKELY(journalEntry.transaction().isStockSplit())) {
        fullBalanceRecalc.insert(journalEntry.split().accountId());
      } else {
        balanceCache[journalEntry.split().accountId()] += journalEntry.split().shares();
      }
      ++startRow;
    }
  }

  void finishBalanceCacheOperation()
  {
    if (!fullBalanceRecalc.isEmpty()) {
      const auto journalRows = q->rowCount();
      for (const auto& accountId : qAsConst(fullBalanceRecalc)) {
        balanceCache[accountId] = MyMoneyMoney();
      }

      for (int row = 0; row < journalRows; ++row) {
        const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(q->index(row, 0).internalPointer())->constDataRef();
        if (fullBalanceRecalc.contains(journalEntry.split().accountId())) {
          if (journalEntry.transaction().isStockSplit()) {
            balanceCache[journalEntry.split().accountId()] *= journalEntry.split().shares();
          } else {
            balanceCache[journalEntry.split().accountId()] += journalEntry.split().shares();
          }
        }
      }
    }

    // inform others about the changes
    QHash<QString, MyMoneyMoney> balances;
    for (const auto& accountId : qAsConst(balanceChangedSet)) {
      balances.insert(accountId, balanceCache.value(accountId));
    }
    emit q->balancesChanged(balances);
  }

  JournalModel*                   q;
  JournalModelNewTransaction*     newTransactionModel;
  QMap<QString, QString>          transactionIdKeyMap;
  QHash<Column, QString>          headerData;
  QHash<QString, MyMoneyMoney>    balanceCache;
  QHash<QString, MyMoneyAccount>  accountCache;
  QSet<QString>                   fullBalanceRecalc;
  QSet<QString>                   balanceChangedSet;
};

JournalModelNewTransaction::JournalModelNewTransaction(QObject* parent)
  : JournalModel(parent)
{
  setObjectName(QLatin1String("JournalModelNewTransaction"));
  QMap<QString, MyMoneyTransaction> list;
  MyMoneyTransaction t;
  MyMoneySplit sp;
  sp.setAccountId(QStringLiteral("FakeID"));
  t.addSplit(sp);
  list[QString()] = t;
  JournalModel::load(list);
}

JournalModelNewTransaction::~JournalModelNewTransaction()
{
}

QVariant JournalModelNewTransaction::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid())
    return QVariant();
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
    return QVariant();

  // never show any data for the empty transaction
  if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
    return QVariant();

  return JournalModel::data(idx, role);
}





JournalModel::JournalModel(QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<JournalEntry>(parent, QStringLiteral("T"), JournalModel::ID_SIZE, undoStack)
  , d(new Private(this))
{
  setObjectName(QLatin1String("JournalModel"));
}

JournalModel::JournalModel(const QString& idLeadin, QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<JournalEntry>(parent, idLeadin, JournalModel::ID_SIZE, undoStack)
  , d(new Private(this))
{
  setObjectName(QLatin1String("JournalModel"));
}

JournalModel::~JournalModel()
{
}

JournalModelNewTransaction * JournalModel::newTransaction()
{
  if (d->newTransactionModel == nullptr) {
    d->newTransactionModel = new JournalModelNewTransaction(this);
  }
  return d->newTransactionModel;
}

int JournalModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  Q_ASSERT(d->headerData.count() == MaxColumns);

  return d->headerData.count();
}

QVariant JournalModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    switch (role) {
      case Qt::DisplayRole:
        return d->headerData.value(static_cast<Column>(section));

      case Qt::SizeHintRole:
        return QSize(20, 20);;
    }
    return {};
  }
  if (orientation == Qt::Vertical && role == Qt::SizeHintRole) {
    return QSize(10, 10);
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags JournalModel::flags(const QModelIndex& index) const
{
  Q_UNUSED(index);
  return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
}

QVariant JournalModel::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid())
    return QVariant();
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
    return QVariant();

  const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef();
  if (journalEntry.transactionPtr() == nullptr) {
    return QVariant();
  }

  const MyMoneyTransaction& transaction = journalEntry.transaction();

  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(idx.column()) {
        case Number:
          return journalEntry.split().number();

        case Date:
          return QLocale().toString(transaction.postDate(), QLocale::ShortFormat);

        case Account:
          return MyMoneyFile::instance()->accountsModel()->itemById(journalEntry.split().accountId()).name();

        case Security:
          break;

        case CostCenter:
#if 0
          /// @todo finish implementation
          // in case the own split does not have a costcenter, but the counter split does
          // we use it nevertheless
          if(m_costCenterId.isEmpty())
            m_costCenterId = split.costCenterId();
#endif
          break;

        case Detail:
          return d->counterAccount(journalEntry, transaction);

        case Reconciliation:
          return d->reconciliationStateShort(journalEntry.split().reconcileFlag());
          break;

        case Payment:
          if (journalEntry.split().value().isNegative()) {
            const auto split = journalEntry.split();
            auto acc = MyMoneyFile::instance()->accountsModel()->itemById(split.accountId());
            auto value = split.value(transaction.commodity(), acc.currencyId());
            return (-value).formatMoney(acc.fraction());
          }
          break;

        case Deposit:
          if (!journalEntry.split().value().isNegative()) {
            const auto split = journalEntry.split();
            auto acc = MyMoneyFile::instance()->accountsModel()->itemById(split.accountId());
            auto value = split.value(transaction.commodity(), acc.currencyId());
            return value.formatMoney(acc.fraction());
          }
        case Quantity:
        case Price:
        case Amount:
        case Value:
          break;

        case Balance:
          {
            const auto split = journalEntry.split();
            auto acc = MyMoneyFile::instance()->accountsModel()->itemById(split.accountId());
            return journalEntry.balance().formatMoney(acc.fraction());
          }
          break;
      }
      break;

    case Qt::TextAlignmentRole:
      switch( idx.column()) {
        case Quantity:
        case Price:
        case Amount:
        case Payment:
        case Deposit:
        case Balance:
          return QVariant(Qt::AlignRight | Qt::AlignTop);

        case Reconciliation:
          return QVariant(Qt::AlignHCenter | Qt::AlignTop);

        default:
          break;
      }
      return QVariant(Qt::AlignLeft | Qt::AlignTop);

    case eMyMoney::Model::IdRole:
      return journalEntry.id();

    case eMyMoney::Model::Roles::SplitAccountIdRole:
      return journalEntry.split().accountId();

    case eMyMoney::Model::SplitReconcileFlagRole:
      return QVariant::fromValue<eMyMoney::Split::State>(journalEntry.split().reconcileFlag());

    case eMyMoney::Model::SplitReconcileDateRole:
      return journalEntry.split().reconcileDate();

    case eMyMoney::Model::SplitActionRole:
      return journalEntry.split().action();

    case eMyMoney::Model::JournalSplitIdRole:
      return journalEntry.split().id();

    case eMyMoney::Model::JournalTransactionIdRole:
      return journalEntry.transaction().id();

    case eMyMoney::Model::TransactionSplitCountRole:
      return journalEntry.transaction().splitCount();

    case eMyMoney::Model::TransactionIsTransferRole:
      if (journalEntry.transaction().splitCount() == 2) {
        for (const auto& split : journalEntry.transaction().splits()) {
          const auto acc = MyMoneyFile::instance()->accountsModel()->itemById(split.accountId());
          if (acc.isIncomeExpense()) {
            return false;
            break;
          }
        }
      }
      return true;

    case eMyMoney::Model::TransactionPostDateRole:
      return transaction.postDate();

    case eMyMoney::Model::Roles::TransactionErroneousRole:
      return !transaction.splitSum().isZero();

    case eMyMoney::Model::Roles::SplitSharesSuffixRole:
      // figure out if it is a debit or credit split. s.a. https://en.wikipedia.org/wiki/Debits_and_credits#Aspects_of_transactions
      if(journalEntry.split().shares().isNegative()) {
        return i18nc("Credit suffix", "Cr.");
      }
      return i18nc("Debit suffix", "Dr.");

    case eMyMoney::Model::Roles::SplitSharesRole:
      {
        QVariant rc;
        rc.setValue(journalEntry.split().shares());
        return rc;
      }

    case eMyMoney::Model::Roles::SplitValueRole:
    {
      QVariant rc;
      rc.setValue(journalEntry.split().value());
      return rc;
    }

    case eMyMoney::Model::SplitPriceRole:
    {
      QVariant rc;
      rc.setValue(journalEntry.split().price());
      return rc;
    }

    case eMyMoney::Model::SplitPayeeIdRole:
      if (journalEntry.split().payeeId().isEmpty()) {
        // not sure if we want to replace it with the payeeId
        // of another split. Anyway, here would be the spot to do it
#if 0
        const MyMoneySplit split = journalEntry.split();
        foreach (const auto sp, transaction.splits()) {
          if(split.id() != sp.id()) {
            if (!split.payeeId().isEmpty())

          }
        }
#endif
        return QVariant();
      }
      return journalEntry.split().payeeId();

    case eMyMoney::Model::SplitSingleLineMemoRole:
    case eMyMoney::Model::SplitMemoRole:
      {
        QString rc(journalEntry.split().memo());
        if(role == eMyMoney::Model::SplitSingleLineMemoRole) {
          // remove empty lines
          rc.replace("\n\n", "\n");
          // replace '\n' with ", "
          rc.replace('\n', ", ");
        }
        return rc;
      }

    case eMyMoney::Model::SplitPayeeRole:
      return MyMoneyFile::instance()->payeesModel()->itemById(journalEntry.split().payeeId()).name();

    case eMyMoney::Model::TransactionCounterAccountRole:
      return d->counterAccount(journalEntry, transaction);

    case eMyMoney::Model::TransactionCounterAccountIdRole:
      return d->counterAccountId(journalEntry, transaction);

    default:
      if (role >= Qt::UserRole)
        qDebug() << "JournalModel::data(), role" << role << "offset" << role-Qt::UserRole << "not implemented";
      break;
  }
  return QVariant();
}

bool JournalModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
  if(!idx.isValid()) {
    return false;
  }
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent())) {
    return false;
  }

  JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->dataRef();
  if (journalEntry.transactionPtr() == nullptr) {
    return false;
  }

  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(idx.column()) {
        case Balance:
          journalEntry.setBalance(value.value<MyMoneyMoney>());
          return true;

        default:
          return false;
      }
      break;

    default:
      return false;

  }
  // qDebug() << "setData(" << idx.row() << idx.column() << ")" << value << role;
  return QAbstractItemModel::setData(idx, value, role);
}

void JournalModel::load(const QMap<QString, MyMoneyTransaction>& list)
{
  beginResetModel();
  // first get rid of any existing entries
  clearModelItems();

  // create the number of required items
  int itemCount = 0;
  foreach (const auto item, list) {
    itemCount += item.splitCount();
  }
  insertRows(0, itemCount);

  m_nextId = 0;

  int row = 0;
  QMap<QString, MyMoneyTransaction>::const_iterator it;
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    const QString& id = (*it).id();
    updateNextObjectId(id);
    d->addIdKeyMapping(id, it.key());
    auto transaction = QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(*it));
    foreach (const auto split, (*transaction).splits()) {
      const JournalEntry journalEntry(it.key(), transaction, split);
      static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->dataRef() = journalEntry;
      ++row;
    }
  }
  endResetModel();

  emit modelLoaded();

  // and don't count loading as a modification
  setDirty(false);

  qDebug() << "Model for" << m_idLeadin << "loaded with" << rowCount() << "items";
}

void JournalModel::unload()
{
  d->balanceCache.clear();
  d->accountCache.clear();
  MyMoneyModel::unload();
}

MyMoneyTransaction JournalModel::transactionById(const QString& id) const
{
  const QModelIndex idx = firstIndexById(id);
  if (idx.isValid()) {
    return static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef().transaction();
  }
  return MyMoneyTransaction();
}

QModelIndex JournalModel::firstIndexById(const QString& id) const
{
  const QString key = d->mapIdToKey(id);
  // in case we do not have a key, the transactionId does not exist
  // so no need to search for it
  if (key.isEmpty()) {
    return QModelIndex();
  }

  return MyMoneyModelBase::lowerBound(key);
}

QString JournalModel::keyForDate(const QDate& date) const
{
  return MyMoneyTransaction::uniqueSortKey(date, QString());
}

void JournalModel::addTransaction(MyMoneyTransaction& item)
{
  item = MyMoneyTransaction(nextId(), item);
  auto transaction = QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(item));
  JournalEntry entry(QString(), transaction, MyMoneySplit());

  m_undoStack->push(new UndoCommand(this, JournalEntry(), entry));
}

void JournalModel::doAddItem(const JournalEntry& item, const QModelIndex& parentIdx)
{
  Q_UNUSED(parentIdx);
  auto transaction = item.sharedtransactionPtr();
  QString key = (*transaction).uniqueSortKey();

  // add mapping
  d->addIdKeyMapping((*transaction).id(), key);

  const auto idx = MyMoneyModelBase::lowerBound(key);
  auto startRow = idx.row();
  if (!idx.isValid()) {
    startRow = rowCount();
  }

  const int rows = (*transaction).splitCount();

  // insert the items into the model
  insertRows(startRow, rows);
  const QModelIndex startIdx = index(startRow, 0);
  const QModelIndex endIdx = index(startRow+rows-1, columnCount());

  d->startBalanceCacheOperation();

  const auto originalStartRow = startRow;
  foreach (const auto split, (*transaction).splits()) {
    JournalEntry journalEntry(key, transaction, split);
    static_cast<TreeItem<JournalEntry>*>(index(startRow, 0).internalPointer())->dataRef() = journalEntry;
    ++startRow;
  }

  // add the splits to the balance cache
  d->addTransactionToBalance(originalStartRow, rows);

  emit dataChanged(startIdx, endIdx);

  d->finishBalanceCacheOperation();
  setDirty();
}

void JournalModel::removeTransaction(const MyMoneyTransaction& item)
{
  const auto idx = firstIndexById(item.id());
  if (idx.isValid()) {
    const auto currentItem = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef();
    m_undoStack->push(new UndoCommand(this, currentItem, JournalEntry()));
  }
}

void JournalModel::doRemoveItem(const JournalEntry& before)
{

  const auto& transaction = before.transaction();
  const auto idx = firstIndexById(transaction.id());
  const auto rows = transaction.splitCount();
  d->startBalanceCacheOperation();
  d->removeTransactionFromBalance(idx.row(), rows);

  removeRows(idx.row(), rows);
  d->removeIdKeyMapping(transaction.id());

  d->finishBalanceCacheOperation();
  setDirty();
}

void JournalModel::modifyTransaction(const MyMoneyTransaction& newTransaction)
{
  const auto idx = firstIndexById(newTransaction.id());
  if (idx.isValid()) {
    auto transaction = QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(newTransaction));
    JournalEntry entry(QString(), transaction, MyMoneySplit());

    const auto currentItem = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef();
    m_undoStack->push(new UndoCommand(this, currentItem, entry));
  }
}

void JournalModel::doModifyItem(const JournalEntry& before, const JournalEntry& after)
{
  Q_UNUSED(before);
  const auto startIdx = firstIndexById(after.transaction().id());

  if (!startIdx.isValid())
    return;

  // we keep a copy of the original transaction
  // (we don't believe the caller except for the id)
  const auto newTransaction = after.transaction();
  const auto oldTransaction = static_cast<TreeItem<JournalEntry>*>(startIdx.internalPointer())->constDataRef().transaction();
  const auto oldKey = oldTransaction.uniqueSortKey();

  d->startBalanceCacheOperation();
  d->removeTransactionFromBalance(startIdx.row(), oldTransaction.splitCount());
  // we have to deal with several cases here. The first differentiation
  // is the unique key. It remains the same as long as the postDate()
  // of the two transactions is identical. In case the postDate changed, we
  // need to move the transaction to a new spot in the model. The next
  // important thing is the number of splits. If it differs, we have to
  // add or remove entries to/from the journal. In case the postDate()
  // remains the same and the number of splits is the same we can simply
  // assign new JournalEntry items to the model. In the other special
  // cases it seems easier to simply remove the existing transaction
  // and add a new one by re-using the id.
  auto transaction = after.sharedtransactionPtr();
  if ((newTransaction.postDate() == oldTransaction.postDate())
  && (newTransaction.splitCount() == oldTransaction.splitCount())) {
    int row = startIdx.row();
    foreach (const auto split, newTransaction.splits()) {
      JournalEntry journalEntry(oldKey, transaction, split);
      static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->dataRef() = journalEntry;
      ++row;
    }
    // let the world know that things have changed
    const QModelIndex endIdx = index(row-1, columnCount());
    emit dataChanged(startIdx, endIdx);

    d->addTransactionToBalance(startIdx.row(), newTransaction.splitCount());

  } else {

    // remove the old transaction
    d->removeIdKeyMapping(oldTransaction.id());
    removeRows(startIdx.row(), oldTransaction.splitCount());

    JournalEntry journalEntry(QString(), transaction, MyMoneySplit());
    doAddItem(journalEntry, QModelIndex());
  }

  d->finishBalanceCacheOperation();
  setDirty();
}

void JournalModel::transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
  list.clear();

  const int rows = rowCount();
  for (int row = 0; row < rows;) {
    const auto journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
    const auto cnt = filter.matchingSplitsCount(journalEntry.transaction());
    for (uint i = 0; i < cnt; ++i) {
      list.append(journalEntry.transaction());
    }
    row += journalEntry.transaction().splitCount();
  }
}

void JournalModel::transactionList(QList< QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
  list.clear();

  const int rows = rowCount();
  QVector<MyMoneySplit> splits;
  for (int row = 0; row < rows; ) {
    const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
    splits = filter.matchingSplits(journalEntry.transaction());
    if (!splits.isEmpty()) {
      for (const auto& split : qAsConst(splits)) {
        list.append(qMakePair(journalEntry.transaction(), split));
      }
    }
    row += journalEntry.transaction().splitCount();
  }
}

unsigned int JournalModel::transactionCount(const QString& accountid) const
{
  unsigned int result = 0;

  if (accountid.isEmpty()) {
    result = d->transactionIdKeyMap.count();

  } else {
    const int rows = rowCount();
    for (int row = 0; row < rows; ++row) {
      const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
      if (journalEntry.split().accountId() == accountid) {
        ++result;
      }
    }
  }
  return result;
}

void JournalModel::updateBalances()
{
  d->loadAccountCache();

  // calculate the balances
  d->balanceCache.clear();
  const int rows = rowCount();
  qDebug() << "Start calculating balances:" << rows << "splits";
  for (int row = 0; row < rows; ++row) {
    const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
    if (journalEntry.transaction().isStockSplit()) {
      d->balanceCache[journalEntry.split().accountId()] *= journalEntry.split().shares();
    } else {
      d->balanceCache[journalEntry.split().accountId()] += journalEntry.split().shares();
    }
  }
  qDebug() << "End calculating balances";

  // and store the results in the accountsModel
  emit balancesChanged(d->balanceCache);
}

MyMoneyMoney JournalModel::balance(const QString& accountId, const QDate& date) const
{
  if (date.isValid()) {
    MyMoneyMoney balance;
    QModelIndex lastIdx = upperBound(MyMoneyTransaction::uniqueSortKey(date, QStringLiteral("x")), 0, rowCount()-1);
    // in case the index is invalid, we search for a data past
    // the end of the journal, so we can simply use the cached
    // balance.
    if (lastIdx.isValid()) {
      // in case the entry is in the first half,
      // we start from the beginning and go forward
      if (lastIdx.row() < rowCount()/2) {
        for (int row = 0; row < lastIdx.row(); ++row) {
          const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
          if (journalEntry.split().accountId() == accountId) {
            if (journalEntry.transaction().isStockSplit()) {
              balance *= journalEntry.split().shares();
            } else {
              balance += journalEntry.split().shares();
            }
          }
        }
      } else {
        // in case the entry is in the second half,
        // we start at the end and go backwards
        // This requires the balance cache to always
        // be up-to-date
        balance = d->balanceCache.value(accountId);
        for (int row = rowCount()-1; row >= lastIdx.row(); --row) {
          const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
          if (journalEntry.split().accountId() == accountId) {
            if (journalEntry.transaction().isStockSplit()) {
              balance /= journalEntry.split().shares();
            } else {
              balance -= journalEntry.split().shares();
            }
          }
        }
      }
      return balance;
    }
  }
  return d->balanceCache.value(accountId);
}

// determine for which type we were created:
//
// a) add item: m_after.id() is filled, m_before.id() is empty
// b) modify item: m_after.id() is filled, m_before.id() is filled
// c) add item: m_after.id() is empty, m_before.id() is filled
JournalModel::Operation JournalModel::undoOperation(const JournalEntry& before, const JournalEntry& after) const
{
  const auto afterIdEmpty = (after.transactionPtr() == nullptr) || (after.transaction().id().isEmpty());
  const auto beforeIdEmpty = (before.transactionPtr() == nullptr) || (before.transaction().id().isEmpty());
  if (!afterIdEmpty && beforeIdEmpty)
    return Add;
  if (!afterIdEmpty && !beforeIdEmpty)
    return Modify;
  if (afterIdEmpty && !beforeIdEmpty)
    return Remove;
  return Invalid;
}

