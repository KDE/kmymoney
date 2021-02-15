/*
    SPDX-FileCopyrightText: 2016-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

#include "mymoneyfile.h"
#include "mymoneycostcenter.h"

struct CostCenterModel::Private
{
  Private() {}

  QVector<MyMoneyCostCenter*>  m_costCenterItems;
};

CostCenterModel::CostCenterModel(QObject* parent)
  : QAbstractListModel(parent)
  , d(new Private)
{
  qDebug() << "Cost center model created with items" << d->m_costCenterItems.count();
  d->m_costCenterItems.clear();
}

CostCenterModel::~CostCenterModel()
{
}

int CostCenterModel::rowCount(const QModelIndex& parent) const
{
  // since the ledger model is a simple table model, we only
  // return the rowCount for the hiddenRootItem. and zero otherwise
  if(parent.isValid()) {
    return 0;
  }

  return d->m_costCenterItems.count();
}

int CostCenterModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 1;
}

Qt::ItemFlags CostCenterModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags;

  if(!index.isValid())
    return flags;
  if(index.row() < 0 || index.row() >= d->m_costCenterItems.count())
    return flags;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}


QVariant CostCenterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case 0:
        return i18n("Cost center");
        break;
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant CostCenterModel::data(const QModelIndex& index, int role) const
{
  if(!index.isValid())
    return QVariant();
  if(index.row() < 0 || index.row() >= d->m_costCenterItems.count())
    return QVariant();

  QVariant rc;
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      // make sure to never return any displayable text for the dummy entry
      if(!d->m_costCenterItems[index.row()]->id().isEmpty()) {
        rc = d->m_costCenterItems[index.row()]->name();
      } else {
	rc = QString();
      }
      break;

    case Qt::TextAlignmentRole:
      rc = QVariant(Qt::AlignLeft | Qt::AlignTop);
      break;

    case ShortNameRole:
      rc = d->m_costCenterItems[index.row()]->shortName();
      break;

    case CostCenterIdRole:
      rc = d->m_costCenterItems[index.row()]->id();
      break;
  }
  return rc;
}

bool CostCenterModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(!index.isValid()) {
    return false;
  }

  qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  return QAbstractItemModel::setData(index, value, role);
}



void CostCenterModel::unload()
{
  if(rowCount() > 0) {
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    for(int i = 0; i < rowCount(); ++i) {
      delete d->m_costCenterItems[i];
    }
    d->m_costCenterItems.clear();
    endRemoveRows();
  }
}

void CostCenterModel::load()
{
  QList<MyMoneyCostCenter> list;
  MyMoneyFile::instance()->costCenterList(list);

  if(list.count() > 0) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + list.count());
    // create an empty entry for those items that do not reference a cost center
    d->m_costCenterItems.append((new MyMoneyCostCenter));
    QList< MyMoneyCostCenter >::const_iterator it;
    for(it = list.constBegin(); it != list.constEnd(); ++it) {
      d->m_costCenterItems.append(new MyMoneyCostCenter(*it));
    }
    endInsertRows();
  }
}
