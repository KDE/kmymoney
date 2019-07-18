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

#include "accountsmodel.h"

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

struct AccountsModel::Private
{
  Private(AccountsModel* qq, QObject* parent)
    : q_ptr(qq)
    , parentObject(parent)
  {
  }


  AccountsModel*    q_ptr;
  QObject*            parentObject;
};

AccountsModel::AccountsModel(QObject* parent)
  : MyMoneyModel<MyMoneyAccount>(parent, QStringLiteral("A"), AccountsModel::ID_SIZE)
  , d(new Private(this, parent))
{
  setObjectName(QLatin1String("AccountsModel"));
}

AccountsModel::~AccountsModel()
{
}

int AccountsModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return Column::MaxColumns;
}

QVariant AccountsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case Column::AccountName:
        return i18n("Name");
      case Column::Type:
        return i18n("Type");
      case Column::Tax:
        return i18n("Tax");
      case Column::Vat:
        return i18n("VAT");
      case Column::CostCenter:
        return i18nc("CostCenter", "CC");
      case Column::TotalBalance:
        return i18n("Total Balance");
      case Column::PostedValue:
        return i18n("Posted Value");
      case Column::TotalValue:
        return i18n("Total Value");
      case Column::Number:
        return i18n("Number");
      case Column::SortCode:
        return i18n("SortCode");
      default:
        return QVariant();
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant AccountsModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return QVariant();

  QVariant rc;
  const MyMoneyAccount& account = static_cast<TreeItem<MyMoneyAccount>*>(index.internalPointer())->constDataRef();
  MyMoneyAccount tradingCurrency;
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(index.column()) {
        case Column::AccountName:
          // make sure to never return any displayable text for the dummy entry
          if (!account.id().isEmpty()) {
            rc = account.name();
          } else {
            rc = QString();
          }
          break;
      case Column::Type:
        return i18n("Type");
      case Column::Tax:
        return i18n("Tax");
      case Column::Vat:
        return i18n("VAT");
      case Column::CostCenter:
        return i18nc("CostCenter", "CC");
      case Column::TotalBalance:
        return i18n("Total Balance");
      case Column::PostedValue:
        return i18n("Posted Value");
      case Column::TotalValue:
        return i18n("Total Value");
      case Column::Number:
        return i18n("Number");
      case Column::SortCode:
        return i18n("SortCode");
        default:
          break;
      }
      break;

    case Qt::TextAlignmentRole:
      rc = QVariant(Qt::AlignLeft | Qt::AlignVCenter);
      break;

    case eMyMoney::Model::Roles::IdRole:
      rc = account.id();
      break;

  }
  return rc;
}

bool AccountsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(!index.isValid()) {
    return false;
  }

  qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  return QAbstractItemModel::setData(index, value, role);
}

void AccountsModel::load(const QMap<QString, MyMoneyAccount>& list)
{
  beginResetModel();
  // first get rid of any existing entries
  clearModelItems();


  // and don't count loading as a modification
  setDirty(false);

  /// @todo here implement hierarchical loading
  int row = 0;
  foreach (const auto item, list) {
    static_cast<TreeItem<MyMoneyAccount>*>(index(row, 0).internalPointer())->dataRef() = item;
    ++row;
  }
  endResetModel();

  qDebug() << "Model for 'A' loaded with" << rowCount() << "items";
}

QList<MyMoneyAccount> AccountsModel::itemList() const
{
  QList<MyMoneyAccount> list;
  QModelIndexList indexes = match(index(0, 0), eMyMoney::Model::Roles::IdRole, m_idLeadin, -1, Qt::MatchStartsWith | Qt::MatchRecursive);
  for (int row = 0; row < indexes.count(); ++row) {
    const MyMoneyAccount& account = static_cast<TreeItem<MyMoneyAccount>*>(indexes.value(row).internalPointer())->constDataRef();
    if (!account.id().startsWith("AStd"))
      list.append(account);
  }
  return list;
}

