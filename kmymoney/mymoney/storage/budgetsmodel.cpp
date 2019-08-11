/*
 * Copyright 2016-2019  Thomas Baumgart <tbaumgart@kde.org>
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

#include "budgetsmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>
#include <QDate>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

struct BudgetsModel::Private
{
  Private()
  {}

};

BudgetsModel::BudgetsModel(QObject* parent)
  : MyMoneyModel<MyMoneyBudget>(parent, QStringLiteral("B"), BudgetsModel::ID_SIZE)
  , d(new Private)
{
  setObjectName(QLatin1String("BudgetsModel"));
}

BudgetsModel::~BudgetsModel()
{
}

int BudgetsModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 2;
}

QVariant BudgetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case 0:
        return i18n("Budget");

      case 1:
        return i18nc("Budget year", "Year");
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant BudgetsModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return QVariant();

  QVariant rc;
  const MyMoneyBudget& budget = static_cast<TreeItem<MyMoneyBudget>*>(index.internalPointer())->constDataRef();
  switch (role) {
    case eMyMoney::Model::Roles::BudgetNameRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(index.column()) {
        case Columns::Name:
          return budget.name();

        case Columns::Year:
          return budget.budgetStart().year();

      }
      return QVariant();

    case Qt::TextAlignmentRole:
      return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case eMyMoney::Model::Roles::IdRole:
      return budget.id();

    default:
      break;
  }
  return QVariant();
}

bool BudgetsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid()) {
    return false;
  }

  if (!index.isValid())
    return false;
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return false;

  MyMoneyBudget& budget = static_cast<TreeItem<MyMoneyBudget>*>(index.internalPointer())->dataRef();

  bool rc = true;
  switch(role) {
    case eMyMoney::Model::Roles::BudgetNameRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
      // make sure to never return any displayable text for the dummy entry
      if (!budget.id().isEmpty()) {
        budget.setName(value.toString());
      } else {
        rc = false;
      }
      break;

    case Qt::TextAlignmentRole:
      break;

    case eMyMoney::Model::Roles::IdRole:
      rc = false;
      break;
    default:
      rc = false;
      break;
  }

  if (rc) {
    setDirty();
    const auto topLeft = BudgetsModel::index(index.row(), 0);
    const auto bottomRight = BudgetsModel::index(index.row(), columnCount()-1);
    emit dataChanged(topLeft, bottomRight);
  }
  return rc;
}

