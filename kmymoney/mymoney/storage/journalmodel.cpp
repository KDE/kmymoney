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

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneytransactionfilter.h"

struct JournalModel::Private
{
  Private() {}

  void removeIdKeyMapping(const QString& id)
  {
    m_transactionIdKeyMap.remove(id);
  }

  void addIdKeyMapping(const QString& id, const QString& key)
  {
    m_transactionIdKeyMap[id] = key;
  }

  QString mapIdToKey(const QString& id) const
  {
    return m_transactionIdKeyMap.value(id);
  }

  QMap<QString, QString>    m_transactionIdKeyMap;
};

JournalModel::JournalModel(QObject* parent)
  : MyMoneyModel<JournalEntry>(parent, QStringLiteral("T"), JournalModel::ID_SIZE)
  , d(new Private)
{
  setObjectName(QLatin1String("JournalModel"));
}

JournalModel::~JournalModel()
{
}

int JournalModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 1;
}

QVariant JournalModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case 0:
        return i18nc("Postdate", "Date");
        break;
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant JournalModel::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid())
    return QVariant();
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
    return QVariant();

  QVariant rc;
  const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(idx.column()) {
        case 0:
          break;
      }
      break;

    case Qt::TextAlignmentRole:
      rc = QVariant(Qt::AlignLeft | Qt::AlignVCenter);
      break;

    case eMyMoney::Model::Roles::IdRole:
      rc = journalEntry.id();
      break;
  }
  return rc;
}

bool JournalModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
  if(!idx.isValid()) {
    return false;
  }

  qDebug() << "setData(" << idx.row() << idx.column() << ")" << value << role;
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
      JournalEntry journalEntry(it.key(), transaction, split);
      static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->dataRef() = journalEntry;
      ++row;
    }
  }
  endResetModel();

  /// @todo add/update balance cache

  emit modelLoaded();

  // and don't count loading as a modification
  setDirty(false);

  qDebug() << "Model for" << m_idLeadin << "loaded with" << rowCount() << "items";
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

  return firstIndexByKey(key);
}

QModelIndex JournalModel::firstIndexByKey(const QString& key) const
{
  // the following two could be turned into additional parameters
  // to this function to search only in sections of the journal
  int first = 0;
  const int last = rowCount();

  int count = last - first;
  int row = -1;
  int step;
  while (count > 0) {
    row = first;
    step = count / 2;
    row = first + step;
    const JournalEntry& journalEntry = static_cast<TreeItem<JournalEntry>*>(index(row, 0).internalPointer())->constDataRef();
    if (journalEntry.id() < key) {
      first = ++row;
      count -= step + 1;
    } else {
      count = step;
    }
  }
  return index(row, 0);
}

void JournalModel::addTransaction(MyMoneyTransaction& t)
{
  addTransaction(nextId(), t);
}

void JournalModel::addTransaction(const QString& id, MyMoneyTransaction& t)
{
  auto transaction = QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(id, t));
  QString key = (*transaction).uniqueSortKey();

  // add mapping
  d->addIdKeyMapping((*transaction).id(), key);

  const auto idx = firstIndexByKey(key);
  auto startRow = idx.row();
  if (!idx.isValid()) {
    startRow = rowCount();
  }

  const int rows = (*transaction).splitCount();

  // insert the items into the model
  insertRows(startRow, rows);
  const QModelIndex startIdx = index(startRow, 0);
  const QModelIndex endIdx = index(startRow+rows, columnCount());

  foreach (const auto split, (*transaction).splits()) {
    JournalEntry journalEntry(key, transaction, split);
    static_cast<TreeItem<JournalEntry>*>(index(startRow, 0).internalPointer())->dataRef() = journalEntry;
    ++startRow;
  }

  // tell the caller what we have done to his transaction
  t = (*transaction);

  /// @todo add/update balance cache

  emit dataChanged(startIdx, endIdx);
}

void JournalModel::removeTransaction(const MyMoneyTransaction& t)
{
  const auto startIdx = firstIndexById(t.id());

  if (!startIdx.isValid())
    return;

  removeTransaction(startIdx);
}

void JournalModel::removeTransaction(const QModelIndex& idx)
{
  // we keep a copy of the original transaction
  // (we don't believe the caller except for the id)
  const auto transaction = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef().transaction();

  removeRows(idx.row(), transaction.splitCount());
  d->removeIdKeyMapping(transaction.id());

  /// @todo add/update balance cache

}


void JournalModel::modifyTransaction(const MyMoneyTransaction& newTransaction)
{
  const auto startIdx = firstIndexById(newTransaction.id());

  if (!startIdx.isValid())
    return;

  // we keep a copy of the original transaction
  // (we don't believe the caller except for the id)
  const auto oldTransaction = static_cast<TreeItem<JournalEntry>*>(startIdx.internalPointer())->constDataRef().transaction();
  const auto oldKey = oldTransaction.uniqueSortKey();

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
  auto transaction = QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(newTransaction));
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

    /// @todo add/update balance cache

  } else {

    // remove the old transaction
    d->removeIdKeyMapping(oldTransaction.id());
    removeRows(startIdx.row(), oldTransaction.splitCount());

    // and add the new ones (we need a local copy here to pass a non-const ref)
    MyMoneyTransaction t(newTransaction);
    addTransaction(newTransaction.id(), t);
  }
}

void JournalModel::transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
  list.clear();

  QString lastKey;
  const int rows = rowCount();
  for (int row = 0; row < rows; ++row) {
    const QModelIndex idx = index(row, 0);
    const auto journalEntry = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef();
    if (journalEntry.id() != lastKey) {
      const auto cnt = filter.matchingSplitsCount(journalEntry.transaction());
      for (uint i = 0; i < cnt; ++i) {
        list.append(journalEntry.transaction());
      }
      lastKey = journalEntry.id();
    }
  }
}

void JournalModel::transactionList(QList< QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
  list.clear();

  QString lastKey;
  const int rows = rowCount();
  for (int row = 0; row < rows; ++row) {
    const QModelIndex idx = index(row, 0);
    const auto journalEntry = static_cast<TreeItem<JournalEntry>*>(idx.internalPointer())->constDataRef();
    if (journalEntry.id() != lastKey) {
      const auto& splits = filter.matchingSplits(journalEntry.transaction());
      for (const auto& split : splits) {
        list.append(qMakePair(journalEntry.transaction(), split));
      }
    }
  }
}
