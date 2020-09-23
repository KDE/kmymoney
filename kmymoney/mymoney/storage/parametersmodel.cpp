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

#include "parametersmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes


ParameterItem::ParameterItem()
{
}

ParameterItem::ParameterItem(const QString& key, const QString& value)
  : m_id(key)
  , m_value(value)
{
}



struct ParametersModel::Private
{
  Private() {}
};

ParametersModel::ParametersModel(QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<ParameterItem>(parent, QStringLiteral("p"), ParametersModel::ID_SIZE, undoStack)
  , d(new Private)
{
  setObjectName(QLatin1String("ParametersModel"));
}

ParametersModel::~ParametersModel()
{
}

int ParametersModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 2;
}

QVariant ParametersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case 0:
        return i18nc("ParameterModel key", "Key");
      case 1:
        return i18nc("ParameterModel value", "Value");
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant ParametersModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return QVariant();

  const ParameterItem& parameter = static_cast<TreeItem<ParameterItem>*>(index.internalPointer())->constDataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(index.column()) {
        case eMyMoney::Model::IdRole:
          // make sure to never return any displayable text for the dummy entry
          if (!parameter.id().isEmpty()) {
            return parameter.id();
          }
          return QVariant();

      }
      return QVariant();

    case Qt::TextAlignmentRole:
      return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case eMyMoney::Model::Roles::IdRole:
      return parameter.id();
  }
  return QVariant();
}

bool ParametersModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(!index.isValid()) {
    return false;
  }

  qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  return QAbstractItemModel::setData(index, value, role);
}

void ParametersModel::addItem(const QString& key, const QString& val)
{
  const auto idx = indexById(key);
  int row;
  if (idx.isValid()) {
    row = idx.row();

  } else {
    row = rowCount();
    insertRow(row);
  }
  ParameterItem item(key, val);
  static_cast<TreeItem<ParameterItem>*>(index(row, 0).internalPointer())->dataRef() = item;
}

void ParametersModel::deleteItem(const QString& key)
{
  const auto idx = indexById(key);
  if (!idx.isValid()) {
    removeRow(idx.row());
  }
}

void ParametersModel::load(const QMap<QString, QString>& pairs)
{
  beginResetModel();
  // first get rid of any existing entries
  clearModelItems();

  // create the number of required items
  const auto itemCount = pairs.count();
  insertRows(0, itemCount);

  int row = 0;
  for (auto it = pairs.constBegin(); it != pairs.constEnd(); ++it) {
    ParameterItem item(it.key(), it.value());
    static_cast<TreeItem<ParameterItem>*>(index(row, 0).internalPointer())->dataRef() = item;
    ++row;
  }

  // and don't count loading as a modification
  setDirty(false);

  endResetModel();

  emit modelLoaded();

  qDebug() << "Model for parameters loaded with" << itemCount << "items";
}

QMap<QString, QString> ParametersModel::pairs() const
{
  QMap<QString, QString> result;
  const int rows = rowCount();
  for (int row = 0; row < rows; ++row) {
    const auto& item = static_cast<TreeItem<ParameterItem>*>(index(row, 0).internalPointer())->constDataRef();
    result.insert(item.id(), item.value());
  }
  return result;
}
