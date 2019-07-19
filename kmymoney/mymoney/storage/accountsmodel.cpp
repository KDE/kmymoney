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
#include "mymoneyenums.h"

struct AccountsModel::Private
{
  Q_DECLARE_PUBLIC(AccountsModel)

  Private(AccountsModel* qq, QObject* parent)
    : q_ptr(qq)
    , parentObject(parent)
  {
  }

  int loadSubAccounts(const QModelIndex parent, const QMap<QString, MyMoneyAccount>& list)
  {
    Q_Q(AccountsModel);
    const auto parentAccount = static_cast<TreeItem<MyMoneyAccount>*>(parent.internalPointer())->constDataRef();

    // create entries for the sub accounts
    const int subAccounts = parentAccount.accountCount();
    int itemCount = subAccounts;
    if (subAccounts > 0) {
      q->insertRows(0, subAccounts, parent);
      for (int row = 0; row < subAccounts; ++row) {
        const auto subAccountId = parentAccount.accountList().at(row);
        const auto subAccount = list.value(subAccountId);
        if (subAccount.id() == subAccountId) {
          q->updateNextObjectId(subAccount.id());
          const auto idx = q->index(row, 0, parent);
          static_cast<TreeItem<MyMoneyAccount>*>(idx.internalPointer())->dataRef() = subAccount;
          if (subAccount.value("PreferredAccount") == QLatin1String("Yes")) {
            q->addFavorite(subAccountId);
            ++itemCount;
          }
          itemCount += loadSubAccounts(idx, list);

        } else {
          qDebug() << "Account" << parentAccount.id() << ": subaccount with ID" << subAccountId << "not found in list";
        }
      }
    }
    return itemCount;
  }

  bool isParentFavorite(const QModelIndex& parent) const
  {
    if (parent.isValid()) {
      TreeItem<MyMoneyAccount> *parentItem;
      parentItem = static_cast<TreeItem<MyMoneyAccount>*>(parent.internalPointer());
      if (parentItem->constDataRef().id() == MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Favorite)) {
        return true;
      }
    }
    return false;
  }

  const QVector<QPair<eMyMoney::Account::Standard, const char*> > types = {
    { eMyMoney::Account::Standard::Favorite, I18N_NOOP("Favorite")},
    { eMyMoney::Account::Standard::Asset, I18N_NOOP("Asset accounts") },
    { eMyMoney::Account::Standard::Liability, I18N_NOOP("Liability accounts") },
    { eMyMoney::Account::Standard::Income, I18N_NOOP("Income categories") },
    { eMyMoney::Account::Standard::Expense, I18N_NOOP("Expense categories") },
    { eMyMoney::Account::Standard::Equity, I18N_NOOP("Equity accounts") },
  };

  AccountsModel*    q_ptr;
  QObject*          parentObject;
  QList<QString>    favorites;
};

AccountsModel::AccountsModel(QObject* parent)
  : MyMoneyModel<MyMoneyAccount>(parent, QStringLiteral("A"), AccountsModel::ID_SIZE)
  , d(new Private(this, parent))
{
  setObjectName(QLatin1String("AccountsModel"));

  // force creation of empty account structure
  unload();
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

  bool rc = false;

  // check if something is performed on a favorite entry
  // and skip right away
  if (d->isParentFavorite(index.parent())) {
    return true;
  }

  MyMoneyAccount& account = static_cast<TreeItem<MyMoneyAccount>*>(index.internalPointer())->dataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
#if 0
      switch(index.column()) {
        case Column::Name:
          account.setName(value.toString());
          rc = true;
          break;
        default:
          break;
      }
#endif
      break;
  }
  qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  return QAbstractItemModel::setData(index, value, role);
}

void AccountsModel::clearModelItems()
{
  MyMoneyModel<MyMoneyAccount>::clearModelItems();
  d->favorites.clear();

  // create the account groups with favorite entry as the first thing
  int row = 0;
  insertRows(0, static_cast<int>(eMyMoney::Account::Standard::MaxGroups));
  foreach(auto type, d->types) {
    MyMoneyAccount baseAccount;
    baseAccount.setName(i18n(type.second));
    auto account = MyMoneyAccount(MyMoneyAccount::stdAccName(type.first), baseAccount);
    static_cast<TreeItem<MyMoneyAccount>*>(index(row, 0).internalPointer())->dataRef() = account;
    ++row;
  }
}

void AccountsModel::load(const QMap<QString, MyMoneyAccount>& list)
{
  beginResetModel();
  // first get rid of any existing entries
  clearModelItems();

  // and don't count loading as a modification
  setDirty(false);

  int itemCount = 0;
  foreach(auto type, d->types) {
    ++itemCount;
    // we have nothing to do for favorites
    if (type.first == eMyMoney::Account::Standard::Favorite)
      continue;
    const auto baseAccount = list.value(MyMoneyAccount::stdAccName(type.first));
    if (baseAccount.id() == MyMoneyAccount::stdAccName(type.first)) {
      const auto idx = indexById(baseAccount.id());
      static_cast<TreeItem<MyMoneyAccount>*>(idx.internalPointer())->dataRef() = baseAccount;
      itemCount += d->loadSubAccounts(idx, list);
    } else {
      qDebug() << "Baseaccount for" << MyMoneyAccount::stdAccName(type.first) << "not found in list";
    }
  }
  endResetModel();

  qDebug() << "Model for \"A\" loaded with" << itemCount << "items";
}

QList<MyMoneyAccount> AccountsModel::itemList() const
{
  QList<MyMoneyAccount> list;
  // never search in the first row which is favorites
  QModelIndexList indexes = match(index(1, 0), eMyMoney::Model::Roles::IdRole, m_idLeadin, -1, Qt::MatchStartsWith | Qt::MatchRecursive);
  for (int row = 0; row < indexes.count(); ++row) {
    const MyMoneyAccount& account = static_cast<TreeItem<MyMoneyAccount>*>(indexes.value(row).internalPointer())->constDataRef();
    if (!account.id().startsWith("AStd"))
      list.append(account);
  }
  return list;
}

bool AccountsModel::insertRows(int startRow, int rows, const QModelIndex &parent)
{
  // special handling for favorites
  if (d->isParentFavorite(parent)) {
    beginInsertRows(parent, startRow, startRow + rows - 1);
    do {
      d->favorites.insert(startRow, QString());
      --rows;
    } while(rows > 0);
    endInsertRows();
    setDirty();
    return true;
  }
  return MyMoneyModel<MyMoneyAccount>::insertRows(startRow, rows, parent);
}

bool AccountsModel::removeRows(int startRow, int rows, const QModelIndex &parent)
{
  if (rows == 0)
    return true;

  if (d->isParentFavorite(parent)) {
    beginRemoveRows(parent, startRow, startRow + rows - 1);
    do {
      d->favorites.removeAt(startRow);
      --rows;
    } while(rows > 0);
    endRemoveRows();
    setDirty();
    return true;
  }
  return MyMoneyModel<MyMoneyAccount>::removeRows(startRow, rows, parent);
}

QModelIndex AccountsModel::indexById(const QString& id) const
{
  // never search in the first row which is favorites
  const QModelIndexList indexes = match(index(1, 0), eMyMoney::Model::Roles::IdRole, id, 1, Qt::MatchFixedString | Qt::MatchRecursive);
  if (indexes.isEmpty())
    return QModelIndex();
  return indexes.first();
}

QModelIndexList AccountsModel::indexListByName(const QString& name) const
{
  // never search in the first row which is favorites
  return match(index(1, 0), Qt::DisplayRole, name, 1, Qt::MatchFixedString | Qt::MatchCaseSensitive);
}

void AccountsModel::addFavorite(const QString& id)
{
  // no need to do anything if it is already listed
  if (!d->favorites.contains(id)) {
    // bypass our own indexById as it does not return favorites
    const auto favoriteIdx = MyMoneyModel<MyMoneyAccount>::indexById(MyMoneyAccount::stdAccName(d->types[0].first));
    // we append a single row at the end
    beginInsertRows(favoriteIdx, d->favorites.count(), d->favorites.count());
    d->favorites.append(id);
    endInsertRows();
    // don't modify the dirty flag here. This is done elsewhere.
  }
}

void AccountsModel::removeFavorite(const QString& id)
{
  // no need to do anything if it is not listed
  if (d->favorites.contains(id)) {
    // bypass our own indexById as it does not return favorites
    const auto favoriteIdx = MyMoneyModel<MyMoneyAccount>::indexById(MyMoneyAccount::stdAccName(d->types[0].first));
    const int row = d->favorites.indexOf(id);
    // we remove a single row
    beginRemoveRows(favoriteIdx, row, row);
    d->favorites.removeAt(row);
    endRemoveRows();
    // don't modify the dirty flag here. This is done elsewhere.
  }
}
