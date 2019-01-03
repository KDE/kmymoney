/*
 * Copyright 2016-2017  Thomas Baumgart <tbaumgart@kde.org>
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

#include "payeesmodel.h"

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
#include "mymoneypayee.h"

struct PayeesModel::Private
{
  Private() {}

  QVector<MyMoneyPayee*>  m_payeeItems;
};

PayeesModel::PayeesModel(QObject* parent)
  : QAbstractListModel(parent)
  , d(new Private)
{
  qDebug() << "Payees model created with items" << d->m_payeeItems.count();
  d->m_payeeItems.clear();
}

PayeesModel::~PayeesModel()
{
}

int PayeesModel::rowCount(const QModelIndex& parent) const
{
  // since the payees model is a simple table model, we only
  // return the rowCount for the hiddenRootItem. and zero otherwise
  if(parent.isValid()) {
    return 0;
  }

  return d->m_payeeItems.count();
}

int PayeesModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 1;
}

Qt::ItemFlags PayeesModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags;

  if(!index.isValid())
    return flags;
  if(index.row() < 0 || index.row() >= d->m_payeeItems.count())
    return flags;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}


QVariant PayeesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case 0:
        return i18n("Payee");
        break;
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant PayeesModel::data(const QModelIndex& index, int role) const
{
  if(!index.isValid())
    return QVariant();
  if(index.row() < 0 || index.row() >= d->m_payeeItems.count())
    return QVariant();

  QVariant rc;
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      // make sure to never return any displayable text for the dummy entry
      if(!d->m_payeeItems[index.row()]->id().isEmpty()) {
        rc = d->m_payeeItems[index.row()]->name();
      } else {
        rc = QString();
      }
      break;

    case Qt::TextAlignmentRole:
      rc = QVariant(Qt::AlignLeft | Qt::AlignTop);
      break;

    case PayeeIdRole:
      rc = d->m_payeeItems[index.row()]->id();
      break;
  }
  return rc;
}

bool PayeesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(!index.isValid()) {
    return false;
  }

  qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  return QAbstractItemModel::setData(index, value, role);
}



void PayeesModel::unload()
{
  if(rowCount() > 0) {
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    qDeleteAll(d->m_payeeItems);
    d->m_payeeItems.clear();
    QVector<MyMoneyPayee*> swp;
    d->m_payeeItems.swap(swp); // changed behaviour from Qt 5.7 http://doc.qt.io/qt-5/qvector.html#clear
    endRemoveRows();
  }
}

void PayeesModel::load()
{
  const QList<MyMoneyPayee> list = MyMoneyFile::instance()->payeeList();

  if(list.count() > 0) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + list.count());
    // create an empty entry for those items that do not reference a payee
    d->m_payeeItems.append(new MyMoneyPayee());
    foreach (const auto it, list)
      d->m_payeeItems.append(new MyMoneyPayee(it));
    endInsertRows();
  }
}
