/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tagsmodel.h"

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

struct TagsModel::Private
{
  Private() {}
};

TagsModel::TagsModel(QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<MyMoneyTag>(parent, QStringLiteral("G"), TagsModel::ID_SIZE, undoStack)
  , d(new Private)
{
  setObjectName(QLatin1String("TagsModel"));
}

TagsModel::~TagsModel()
{
}

int TagsModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 1;
}

QVariant TagsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case 0:
        return i18nc("Tagname", "Name");
        break;
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant TagsModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return QVariant();

  QVariant rc;
  const MyMoneyTag& tag = static_cast<TreeItem<MyMoneyTag>*>(index.internalPointer())->constDataRef();
  switch(role) {
    case eMyMoney::Model::TagNameRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
      // make sure to never return any displayable text for the dummy entry
      if (!tag.id().isEmpty()) {
        rc = tag.name();
      } else {
        rc = QString();
      }
      break;

    case Qt::TextAlignmentRole:
      rc = QVariant(Qt::AlignLeft | Qt::AlignVCenter);
      break;

    case eMyMoney::Model::Roles::IdRole:
      rc = tag.id();
      break;

    case eMyMoney::Model::ClosedRole:
      rc = tag.isClosed();
      break;
  }
  return rc;
}

bool TagsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid())
    return false;
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return false;

  MyMoneyTag& tag = static_cast<TreeItem<MyMoneyTag>*>(index.internalPointer())->dataRef();

  switch(role) {
    case eMyMoney::Model::TagNameRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
      // make sure to never return any displayable text for the dummy entry
      if (!tag.id().isEmpty()) {
        tag.setName(value.toString());
        return true;
      }
      break;

    default:
      break;
  }
  return false;
}
