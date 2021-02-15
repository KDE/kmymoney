/*
    SPDX-FileCopyrightText: 2016-2019 Thomas Baumgart <tbaumgart@kde.org>
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

struct CostCenterModel::Private
{
  Private() {}
};

CostCenterModel::CostCenterModel(QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<MyMoneyCostCenter>(parent, QStringLiteral("C"), CostCenterModel::ID_SIZE, undoStack)
  , d(new Private)
{
  setObjectName(QLatin1String("CostCenterModel"));
}

CostCenterModel::~CostCenterModel()
{
}

int CostCenterModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 1;
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
  if (!index.isValid())
    return QVariant();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return QVariant();

  QVariant rc;
  const MyMoneyCostCenter& costCenter = static_cast<TreeItem<MyMoneyCostCenter>*>(index.internalPointer())->constDataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      // make sure to never return any displayable text for the dummy entry
      if (!costCenter.id().isEmpty()) {
        rc = costCenter.name();
      } else {
        rc = QString();
      }
      break;

    case Qt::TextAlignmentRole:
      rc = QVariant(Qt::AlignLeft | Qt::AlignVCenter);
      break;

    case eMyMoney::Model::Roles::CostCenterShortNameRole:
      rc = costCenter.shortName();
      break;

    case eMyMoney::Model::Roles::IdRole:
      rc = costCenter.id();
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
