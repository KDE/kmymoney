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

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"


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
  : MyMoneyModel<JournalEntry>(parent, QStringLiteral("J"), JournalModel::ID_SIZE)
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
