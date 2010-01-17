/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#include "accountsmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocalizedstring.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "kmymoneyutils.h"

class AccountsModel::Private
{
public:
  Private() : m_file(MyMoneyFile::instance())
  {
  }

  ~Private()
  {
  }

  void loadSubAccounts(QStandardItemModel *model, QStandardItem *accountsItem, QStandardItem *favoriteAccountsItem, const QStringList& list)
  {
    for(QStringList::ConstIterator it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
      const MyMoneyAccount& acc = m_file->account(*it_l);
      if (acc.value("PreferredAccount") == "Yes") {
        QStandardItem *item = new QStandardItem(acc.name());
        favoriteAccountsItem->appendRow(item);
        setAccountData(model, item->index(), acc);
      }

      QStandardItem *item = new QStandardItem(acc.name());
      if (accountsItem) {
        accountsItem->appendRow(item);
        setAccountData(model, item->index(), acc);

        if(acc.accountList().count() > 0) {
          loadSubAccounts(model, item, favoriteAccountsItem, acc.accountList());
        }
      }
    }
  }

  void setAccountData(QStandardItemModel *model, const QModelIndex &index, const MyMoneyAccount &account)
  {
    model->setData(index, QVariant(account.id()), AccountIdRole);
    model->setData(index, QVariant(account.accountType()), AccountTypeRole);
    model->setData(index, QVariant(account.isClosed()), AccountClosedRole);
    model->setData(index, model->data(index.parent(), DisplayOrderRole), DisplayOrderRole);
  }

  MyMoneyFile* m_file;
};

AccountsModel::AccountsModel(QObject *parent /*= 0*/)
  : QStandardItemModel(parent), d(new Private)
{
  QStandardItem *rootItem = invisibleRootItem();

  // Favorite accounts
  QStandardItem *favoriteAccountsItem = new QStandardItem(i18n("Favorites"));
  favoriteAccountsItem->setSelectable(false);
  favoriteAccountsItem->setIcon(QIcon(DesktopIcon("account")));
  QFont font = favoriteAccountsItem->font();
  font.setBold(true);
  favoriteAccountsItem->setFont(font);
  rootItem->appendRow(favoriteAccountsItem);
  setData(favoriteAccountsItem->index(), QVariant(0), DisplayOrderRole);

  for(int mask = 0x01; mask != KMyMoneyUtils::last; mask <<= 1) {
    QStringList list;

    QStandardItem *accountsItem = 0;
    if((mask & KMyMoneyUtils::asset) != 0) {
      // Asset accounts
      list = d->m_file->asset().accountList();

      accountsItem = new QStandardItem(i18n("Asset accounts"));
      accountsItem->setSelectable(false);
      accountsItem->setIcon(QIcon(DesktopIcon("account-types-asset")));
      accountsItem->setFont(font);
      rootItem->appendRow(accountsItem);
      setData(accountsItem->index(), QVariant(MyMoneyAccount::Asset), AccountTypeRole);
      setData(accountsItem->index(), QVariant(1), DisplayOrderRole);
    }

    if((mask & KMyMoneyUtils::liability) != 0) {
      // Liability accounts
      list = d->m_file->liability().accountList();

      accountsItem = new QStandardItem(i18n("Liability accounts"));
      accountsItem->setSelectable(false);
      accountsItem->setIcon(QIcon(DesktopIcon("account-types-liability")));
      accountsItem->setFont(font);
      rootItem->appendRow(accountsItem);
      setData(accountsItem->index(), QVariant(MyMoneyAccount::Liability), AccountTypeRole);
      setData(accountsItem->index(), QVariant(2), DisplayOrderRole);
    }

    if((mask & KMyMoneyUtils::income) != 0) {
      // Income categories
      list = d->m_file->income().accountList();

      accountsItem = new QStandardItem(i18n("Income categories"));
      accountsItem->setSelectable(false);
      accountsItem->setIcon(QIcon(DesktopIcon("account-types-income")));
      accountsItem->setFont(font);
      rootItem->appendRow(accountsItem);
      setData(accountsItem->index(), QVariant(MyMoneyAccount::Income), AccountTypeRole);
      setData(accountsItem->index(), QVariant(3), DisplayOrderRole);
    }

    if((mask & KMyMoneyUtils::expense) != 0) {
      // Expense categories
      list = d->m_file->expense().accountList();

      accountsItem = new QStandardItem(i18n("Expense categories"));
      accountsItem->setSelectable(false);
      accountsItem->setIcon(QIcon(DesktopIcon("account-types-expense")));
      accountsItem->setFont(font);
      rootItem->appendRow(accountsItem);
      setData(accountsItem->index(), QVariant(MyMoneyAccount::Expense), AccountTypeRole);
      setData(accountsItem->index(), QVariant(4), DisplayOrderRole);
    }

    if((mask & KMyMoneyUtils::equity) != 0) {
      // Equity accounts
      list = d->m_file->equity().accountList();

      accountsItem = new QStandardItem(i18n("Equity accounts"));
      accountsItem->setSelectable(false);
      accountsItem->setIcon(QIcon(DesktopIcon("account")));
      accountsItem->setFont(font);
      rootItem->appendRow(accountsItem);
      setData(accountsItem->index(), QVariant(MyMoneyAccount::Equity), AccountTypeRole);
      setData(accountsItem->index(), QVariant(5), DisplayOrderRole);
    }

    for(QStringList::ConstIterator it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
      const MyMoneyAccount& acc = d->m_file->account(*it_l);
      if (acc.value("PreferredAccount") == "Yes") {
        QStandardItem *item = new QStandardItem(acc.name());
        favoriteAccountsItem->appendRow(item);
        d->setAccountData(this, item->index(), acc);
      }
      QStandardItem *item = new QStandardItem(acc.name());
      if (accountsItem) {
        accountsItem->appendRow(item);
        d->setAccountData(this, item->index(), acc);
        if(acc.accountList().count() > 0) {
          d->loadSubAccounts(this, item, favoriteAccountsItem, acc.accountList());
        }
      }
    }
  }
}

class AccountsFilterProxyModel::Private
{
public:
  Private() : m_hideClosedAccounts(true)
  {
  }

  ~Private()
  {
  }

  QList<MyMoneyAccount::accountTypeE> m_typeList;
  bool m_hideClosedAccounts;
};

AccountsFilterProxyModel::AccountsFilterProxyModel(QObject *parent /*= 0*/)
  : QSortFilterProxyModel(parent), d(new Private)
{
}

bool AccountsFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  QVariant leftData = sourceModel()->data(left, DisplayOrderRole);
  QVariant rightData = sourceModel()->data(right, DisplayOrderRole);

  if (leftData.toInt() == rightData.toInt()) {
    // sort items of the same display order alphabetically
    return QSortFilterProxyModel::lessThan(left, right);
  }
  return leftData.toInt() < rightData.toInt();
}

bool AccountsFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
  return acceptSourceItem(index);
}

void AccountsFilterProxyModel::addAccountGroup(MyMoneyAccount::accountTypeE group)
{
  if(group == MyMoneyAccount::Asset) {
    d->m_typeList << MyMoneyAccount::Checkings;
    d->m_typeList << MyMoneyAccount::Savings;
    d->m_typeList << MyMoneyAccount::Cash;
    d->m_typeList << MyMoneyAccount::AssetLoan;
    d->m_typeList << MyMoneyAccount::CertificateDep;
    d->m_typeList << MyMoneyAccount::Investment;
    d->m_typeList << MyMoneyAccount::Stock;
    d->m_typeList << MyMoneyAccount::MoneyMarket;
    d->m_typeList << MyMoneyAccount::Asset;
    d->m_typeList << MyMoneyAccount::Currency;

  } else if(group == MyMoneyAccount::Liability) {
    d->m_typeList << MyMoneyAccount::CreditCard;
    d->m_typeList << MyMoneyAccount::Loan;
    d->m_typeList << MyMoneyAccount::Liability;

  } else if(group == MyMoneyAccount::Income) {
    d->m_typeList << MyMoneyAccount::Income;

  } else if(group == MyMoneyAccount::Expense) {
    d->m_typeList << MyMoneyAccount::Expense;

  } else if(group == MyMoneyAccount::Equity) {
    d->m_typeList << MyMoneyAccount::Equity;
  }
}

void AccountsFilterProxyModel::addAccountType(MyMoneyAccount::accountTypeE type)
{
    d->m_typeList << type;
}

void AccountsFilterProxyModel::removeAccountType(MyMoneyAccount::accountTypeE type)
{
  int index = d->m_typeList.indexOf(type);
  if(index != -1) {
    d->m_typeList.removeAt(index);
  }
}

void AccountsFilterProxyModel::clear(void)
{
  d->m_typeList.clear();
}

bool AccountsFilterProxyModel::acceptSourceItem(const QModelIndex &source) const
{
  if (source.isValid()) {
    QVariant data = sourceModel()->data(source, AccountClosedRole);
    if (data.isValid() && (hideClosedAccounts() && data.toBool()))
      return false;

    data = sourceModel()->data(source, AccountTypeRole);
    if (data.isValid() && d->m_typeList.contains(static_cast<MyMoneyAccount::accountTypeE>(data.toInt())))
      return true;

    int rowCount = sourceModel()->rowCount(source);
    for (int i = 0; i < rowCount; ++i) {
      QModelIndex index = sourceModel()->index(i, 0, source);
      if (acceptSourceItem(index))
        return true;
    }
  }
  return false;
}

void AccountsFilterProxyModel::setHideClosedAccounts(bool hideClosedAccounts)
{
  d->m_hideClosedAccounts = hideClosedAccounts;
}
bool AccountsFilterProxyModel::hideClosedAccounts(void) const
{
  return d->m_hideClosedAccounts;
}

#include "accountsmodel.moc"
