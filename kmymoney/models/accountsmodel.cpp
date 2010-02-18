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
#include "kmymoneyglobalsettings.h"

Q_DECLARE_METATYPE(MyMoneyAccount)
Q_DECLARE_METATYPE(MyMoneyMoney)

class AccountsModel::Private
{
public:
  /**
    * The pimpl.
    */
  Private() :
    m_file(MyMoneyFile::instance()),
    m_lastNetWorth(0)
  {
  }

  ~Private() {
  }

  /**
    * Load all the sub-accounts recursively.
    *
    * @param model The model in which to load the data.
    * @param accountsItem The item from the model of the parent account of the sub-accounts which are being loaded.
    * @param favoriteAccountsItem The item of the favorites accounts groups so favorite accounts can be added here also.
    * @param list The list of the account id's of the sub-accounts which are being loaded.
    *
    */
  void loadSubAccounts(QStandardItemModel *model, QStandardItem *accountsItem, QStandardItem *favoriteAccountsItem, const QStringList& list) {
    for (QStringList::ConstIterator it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
      const MyMoneyAccount& acc = m_file->account(*it_l);

      QStandardItem *item = itemFromAccountId(accountsItem, acc.id());
      if (!item) {
        item = new QStandardItem(acc.name());
        accountsItem->appendRow(item);
        item->setColumnCount(model->columnCount());
        item->setEditable(false);
      }

      item->setData(accountsItem->data(DisplayOrderRole), DisplayOrderRole);

      if (acc.accountList().count() > 0) {
        loadSubAccounts(model, item, favoriteAccountsItem, acc.accountList());
      }
      // set the account data after the childs have been loaded
      setAccountData(model, item->index(), acc);

      if (acc.value("PreferredAccount") == "Yes") {
        QStandardItem *item = itemFromAccountId(favoriteAccountsItem, acc.id());
        if (!item) {
          item = new QStandardItem(acc.name());
          favoriteAccountsItem->appendRow(item);
          item->setColumnCount(model->columnCount());
          item->setEditable(false);
        }
        // set the account data after the childs have been loaded
        setAccountData(model, item->index(), acc);
      }

    }
  }

  /**
    * Note: this functions should only be called after the child account data has been set.
    */
  void setAccountData(QStandardItemModel *model, const QModelIndex &index, const MyMoneyAccount &account) {
    // Account
    model->setData(index, QVariant::fromValue(account), AccountRole);
    model->setData(index, QVariant(account.id()), AccountIdRole);
    model->setData(index, QVariant(account.value("PreferredAccount") == "Yes"), AccountFavoriteRole);
    model->setData(index, QVariant(QIcon(account.accountPixmap())), Qt::DecorationRole);

    // set the balance of the account
    MyMoneyMoney accountBalance = balance(account);
    model->setData(index, QVariant::fromValue(accountBalance), AccountBalanceRole);
    // set the value of the account
    MyMoneyMoney accountValue = value(account, accountBalance);
    model->setData(index, QVariant::fromValue(accountValue), AccountValueRole);
    // set the total value of the account
    MyMoneyMoney accountTotalValue = childsTotalValue(index) + accountValue;
    model->setData(index, QVariant::fromValue(accountTotalValue), AccountTotalValueRole);
    // this account still exists so it should not be cleaned up
    model->setData(index, false, CleanupRole);

    QFont font = model->data(index, Qt::FontRole).value<QFont>();

    // Type
    QModelIndex newIndex = model->index(index.row(), index.column() + Type, index.parent());
    model->setData(newIndex, KMyMoneyUtils::accountTypeToString(account.accountType()), Qt::DisplayRole);
    model->setData(newIndex, font, Qt::FontRole);

    QPixmap checkMark = QPixmap(KIconLoader::global()->loadIcon("dialog-ok", KIconLoader::Small));
    switch (account.accountType()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
      if (account.value("Tax").toLower() == "yes") {
        // Tax
        newIndex = model->index(index.row(), index.column() + Tax, index.parent());
        model->setData(newIndex, checkMark, Qt::DecorationRole);
      }
      if (!account.value("VatAccount").isEmpty()) {
        // VAT
        newIndex = model->index(index.row(), index.column() + VAT, index.parent());
        model->setData(newIndex, checkMark, Qt::DecorationRole);
      }
      if (!account.value("VatRate").isEmpty()) {
        MyMoneyMoney vatRate = MyMoneyMoney(account.value("VatRate")) * MyMoneyMoney(100, 1);
        newIndex = model->index(index.row(), index.column() + VAT, index.parent());
        model->setData(newIndex, QString("%1 %").arg(vatRate.formatMoney("", 1)), Qt::DisplayRole);
        model->setData(newIndex, Qt::AlignRight, Qt::TextAlignmentRole);
      }
      break;
    default:
      break;
    }

    // Total Balance
    newIndex = model->index(index.row(), index.column() + TotalBalance, index.parent());
    // only show the balance, if its a different security/currency
    if (m_file->security(account.currencyId()) != m_file->baseCurrency())
      model->setData(newIndex, accountBalance.formatMoney(m_file->security(account.currencyId())), Qt::DisplayRole);
    model->setData(newIndex, accountBalance.formatMoney(m_file->security(account.currencyId())), AccountBalanceDispalyRole);
    model->setData(newIndex, font, Qt::FontRole);
    model->setData(newIndex, Qt::AlignRight, Qt::TextAlignmentRole);

    // Total Value
    newIndex = model->index(index.row(), index.column() + TotalValue, index.parent());
    model->setData(newIndex, accountValue.formatMoney(m_file->baseCurrency()), Qt::DisplayRole);
    model->setData(newIndex, accountValue.formatMoney(m_file->baseCurrency()), AccountValueDisplayRole);
    model->setData(newIndex, accountTotalValue.formatMoney(m_file->baseCurrency()), AccountTotalValueDisplayRole);
    model->setData(newIndex, font, Qt::FontRole);
    model->setData(newIndex, Qt::AlignRight, Qt::TextAlignmentRole);
  }

  /**
    * Compute the balance of the given account.
    *
    * @param account The account for which the balance is being computed.
    */
  MyMoneyMoney balance(const MyMoneyAccount &account)
  {
    MyMoneyMoney balance;
    // account.balance() is not compatable with stock accounts
    if (account.isInvest())
      balance = m_file->balance(account.id());
    else
      balance = account.balance();
    // for income and liability accounts, we reverse the sign
    switch (account.accountGroup()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Equity:
      balance = -balance;
      break;

    default:
      break;
    }

    return balance;
  }

  /**
    * Compute the value of the given account using the provided balance.
    * The value is defined as the balance of the account converted to the base currency.
    *
    * @param account The account for which the value is being computed.
    * @param balance The balance which should be used.
    *
    * @see balance
    */
  MyMoneyMoney value(const MyMoneyAccount &account, const MyMoneyMoney &balance)
  {
    QList<MyMoneyPrice> prices;
    MyMoneySecurity security = m_file->baseCurrency();
    try {
      if (account.isInvest()) {
        security = m_file->security(account.currencyId());
        prices += m_file->price(account.currencyId(), security.tradingCurrency());
        if (security.tradingCurrency() != m_file->baseCurrency().id()) {
          MyMoneySecurity sec = m_file->security(security.tradingCurrency());
          prices += m_file->price(sec.id(), m_file->baseCurrency().id());
        }
      } else if (account.currencyId() != m_file->baseCurrency().id()) {
        if (account.currencyId() != m_file->baseCurrency().id()) {
          security = m_file->security(account.currencyId());
          prices += m_file->price(account.currencyId(), m_file->baseCurrency().id());
        }
      }

    } catch (MyMoneyException *e) {
      kDebug(2) << Q_FUNC_INFO << " caught exception while adding " << account.name() << "[" << account.id() << "]: " << e->what();
      delete e;
    }

    MyMoneyMoney value = balance;
    {
      QList<MyMoneyPrice>::const_iterator it_p;
      QString security = account.currencyId();
      for (it_p = prices.constBegin(); it_p != prices.constEnd(); ++it_p) {
        value = (value * (MyMoneyMoney(1, 1) / (*it_p).rate(security))).convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision()));
        if ((*it_p).from() == security)
          security = (*it_p).to();
        else
          security = (*it_p).from();
      }
      value = value.convert(m_file->baseCurrency().smallestAccountFraction());
    }

    return value;
  }

  /**
    * Compute the total value of the child accounts of the given account.
    * Note that the value of the current account is not in this sum. Also,
    * before calling this function, the caller must make sure that the values
    * of all sub-account must be already in the model in the @ref AccountValueRole.
    *
    * @param index The index of the account in the model.
    * @see value
    */
  MyMoneyMoney childsTotalValue(const QModelIndex &index)
  {
    if (!index.isValid())
      return MyMoneyMoney(0);

    MyMoneyMoney totalValue(0);
    const QAbstractItemModel *model = index.model();
    for (int i = 0; i < model->rowCount(index); ++i)
    {
      QModelIndex childIndex = model->index(i, index.column(), index);
      if (model->hasChildren(childIndex)) {
        totalValue += childsTotalValue(childIndex);
      }
      QVariant data = model->data(childIndex, AccountValueRole);
      QVariant cleanup = model->data(childIndex, CleanupRole);
      if (data.isValid() && cleanup.isValid() && !cleanup.toBool()) {
        // consider the value of the child only if it has not been marked for cleanup
        totalValue += data.value<MyMoneyMoney>();
      }
    }
    return totalValue;
  }

  /**
    * Function to get the item from an account id.
    *
    * @param parent The parent to localize the seach in the child items of this parameter.
    * @param accountId Search based on this parameter.
    *
    * @return The item corresponding to the given account id, NULL if the account was not found.
    */
  QStandardItem *itemFromAccountId(QStandardItem *parent, const QString &accountId)
  {
    QStandardItemModel *model = parent->model();
    QModelIndexList list = model->match(model->index(0, 0, parent->index()), AccountsModel::AccountIdRole, QVariant(accountId), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));
    if (list.count() > 0) {
      return model->itemFromIndex(list.front());
    }
    // TODO: if not found at this item search for in int the model and if found reparent it.
    return 0;
  }

  /**
    * Used to laod the accounts data.
    */
  MyMoneyFile *m_file;
  /**
    * Used to emit the @ref netWorthChanged signal.
    */
  MyMoneyMoney m_lastNetWorth;
};

const QString AccountsModel::favoritesAccountId("Favorites");

/**
  * The constructor is private so that only the @ref Models object can create such an object.
  */
AccountsModel::AccountsModel(QObject *parent /*= 0*/)
    : QStandardItemModel(parent), d(new Private)
{
  QStringList headerLabels;
  for (int i = 0; i < LastColumnMarker; ++i) {
    switch (i) {
    case Account:
      headerLabels << i18n("Account");
      break;
    case Type:
      headerLabels << i18n("Type");
      break;
    case Tax:
      headerLabels << i18nc("Column heading for category in tax report", "Tax");
      break;
    case VAT:
      headerLabels << i18nc("Column heading for VAT category", "VAT");
      break;
    case TotalBalance:
      headerLabels << i18n("Total Balance");
      break;
    case TotalValue:
      headerLabels << i18n("Total Value");
      break;
    default:
      headerLabels << QString();
    }
  }
  setHorizontalHeaderLabels(headerLabels);
}

AccountsModel::~AccountsModel()
{
  delete d;
}

/**
  * This function synchronizes the data of the model with the data 
  * from the @ref MyMoneyFile.
  */
void AccountsModel::load()
{
  // mark all rows as candidates fro cleaning up
  QModelIndexList list = match(index(0, 0), Qt::DisplayRole, "*", -1, Qt::MatchFlags(Qt::MatchWildcard | Qt::MatchRecursive));
  foreach(QModelIndex index, list) {
    setData(AccountsModel::index(index.row(), 0, index.parent()), true, CleanupRole);
  }

  QStandardItem *rootItem = invisibleRootItem();

  // Favorite accounts
  QStandardItem *favoriteAccountsItem = d->itemFromAccountId(rootItem, favoritesAccountId);
  if (!favoriteAccountsItem) {
    favoriteAccountsItem = new QStandardItem(i18n("Favorites"));
    rootItem->appendRow(favoriteAccountsItem);
    setData(favoriteAccountsItem->index(), favoritesAccountId, AccountIdRole);
    setData(favoriteAccountsItem->index(), 0, DisplayOrderRole);
    favoriteAccountsItem->setColumnCount(columnCount());
    favoriteAccountsItem->setIcon(QIcon(DesktopIcon("account")));
    favoriteAccountsItem->setEditable(false);
  }
  setData(favoriteAccountsItem->index(), false, CleanupRole);
  QFont font = favoriteAccountsItem->font();
  font.setBold(true);
  favoriteAccountsItem->setFont(font);

  for (int mask = 0x01; mask != KMyMoneyUtils::last; mask <<= 1) {
    MyMoneyAccount account;
    QString accountName;
    int displayOrder = 0;

    if ((mask & KMyMoneyUtils::asset) != 0) {
      // Asset accounts
      account = d->m_file->asset();
      accountName = i18n("Asset accounts");
      displayOrder = 1;
    }

    if ((mask & KMyMoneyUtils::liability) != 0) {
      // Liability accounts
      account = d->m_file->liability();
      accountName = i18n("Liability accounts");
      displayOrder = 2;
    }

    if ((mask & KMyMoneyUtils::income) != 0) {
      // Income categories
      account = d->m_file->income();
      accountName = i18n("Income categories");
      displayOrder = 3;
    }

    if ((mask & KMyMoneyUtils::expense) != 0) {
      // Expense categories
      account = d->m_file->expense();
      accountName = i18n("Expense categories");
      displayOrder = 4;
    }

    if ((mask & KMyMoneyUtils::equity) != 0) {
      // Equity accounts
      account = d->m_file->equity();
      accountName = i18n("Equity accounts");
      displayOrder = 5;
    }

    QStandardItem *accountsItem = d->itemFromAccountId(rootItem, account.id());
    if (!accountsItem) {
      accountsItem = new QStandardItem(accountName);
      rootItem->appendRow(accountsItem);
      setData(accountsItem->index(), QVariant(displayOrder), DisplayOrderRole);
      accountsItem->setColumnCount(columnCount());
      accountsItem->setFont(font);
      accountsItem->setEditable(false);
    }

    QStringList list = account.accountList();

    for (QStringList::ConstIterator it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
      const MyMoneyAccount& acc = d->m_file->account(*it_l);
      QStandardItem *item = d->itemFromAccountId(accountsItem, acc.id());
      if (!item) {
        item = new QStandardItem(acc.name());
        accountsItem->appendRow(item);
        item->setColumnCount(columnCount());
        item->setEditable(false);
      }
      if (acc.accountList().count() > 0) {
        d->loadSubAccounts(this, item, favoriteAccountsItem, acc.accountList());
      }
      d->setAccountData(this, item->index(), acc);
      // set the account data after the childs have been loaded
      if (acc.value("PreferredAccount") == "Yes") {
        QStandardItem *item = d->itemFromAccountId(favoriteAccountsItem, acc.id());
        if (!item) {
          item = new QStandardItem(acc.name());
          favoriteAccountsItem->appendRow(item);
          item->setColumnCount(columnCount());
          item->setEditable(false);
        }
        d->setAccountData(this, item->index(), acc);
      }
    }

    d->setAccountData(this, accountsItem->index(), account);
  }

  // run cleanup procedure
  list = match(index(0, 0), AccountsModel::CleanupRole, true, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
  QModelIndexList parentsOnlyList;
  foreach(QModelIndex index, list) {
    bool hasParentInTheList = false;
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
      if (list.contains(parent)) {
        hasParentInTheList = true;
        break;
      }
      parent = parent.parent();
    }
    if (!hasParentInTheList) {
      parentsOnlyList.append(index);
    }
  }
  foreach(QModelIndex index, parentsOnlyList) {
    removeRow(index.row(), index.parent());
  }

  QModelIndexList assetList = match(index(0, 0),
                                    AccountsModel::AccountIdRole,
                                    MyMoneyFile::instance()->asset().id(),
                                    1,
                                    Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));

  QModelIndexList liabilityList = match(index(0, 0),
                                        AccountsModel::AccountIdRole,
                                        MyMoneyFile::instance()->liability().id(),
                                        1,
                                        Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));

  MyMoneyMoney netWorth(0);
  if (!assetList.isEmpty() && !liabilityList.isEmpty()) {
    QVariant assetValue = data(assetList.front(), AccountsModel::AccountTotalValueRole);
    QVariant liabilityValue = data(liabilityList.front(), AccountsModel::AccountTotalValueRole);

    if (assetValue.isValid() && liabilityValue.isValid()) {
      netWorth = assetValue.value<MyMoneyMoney>() - liabilityValue.value<MyMoneyMoney>();
    }
  }
  if (d->m_lastNetWorth != netWorth) {
    d->m_lastNetWorth = netWorth;
    emit netWorthChanged(d->m_lastNetWorth);
  }
}

/**
  * The pimpl.
  */
class AccountsFilterProxyModel::Private
{
public:
  Private() : m_hideClosedAccounts(true) {
  }

  ~Private() {
  }

  QList<MyMoneyAccount::accountTypeE> m_typeList;
  bool m_hideClosedAccounts;
};

AccountsFilterProxyModel::AccountsFilterProxyModel(QObject *parent /*= 0*/)
    : QSortFilterProxyModel(parent), d(new Private)
{
  setDynamicSortFilter(true);
  setFilterKeyColumn(-1);
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

/**
  * This function was re-implemented so we could have a special display order (favorites first)
  */
bool AccountsFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  QVariant leftData = sourceModel()->data(left, AccountsModel::DisplayOrderRole);
  QVariant rightData = sourceModel()->data(right, AccountsModel::DisplayOrderRole);

  if (leftData.toInt() == rightData.toInt()) {
    // sort items of the same display order alphabetically
    return QSortFilterProxyModel::lessThan(left, right);
  }
  return leftData.toInt() < rightData.toInt();
}

/**
  * This function was re-implemented to consider all the filtering aspects that we need in the application.
  */
bool AccountsFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
  return acceptSourceItem(index) && filterAcceptsRowOrChildRows(source_row, source_parent);
}

/**
  * This function implements a recursive matching. It is used to match a row even if it's values
  * don't match the current filtering criteria but it has at least one child row that does match.
  */
bool AccountsFilterProxyModel::filterAcceptsRowOrChildRows(int source_row, const QModelIndex &source_parent) const
{
  if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
    return true;

  QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
  for (int i = 0; i < sourceModel()->rowCount(index); ++i) {
    if (filterAcceptsRowOrChildRows(i, index))
      return true;
  }
  return false;
}

/**
  * Add the given account group to the filter.
  * @param group The account group to be added.
  * @see MyMoneyAccount::accountTypeE
  */
void AccountsFilterProxyModel::addAccountGroup(MyMoneyAccount::accountTypeE group)
{
  if (group == MyMoneyAccount::Asset) {
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
  } else if (group == MyMoneyAccount::Liability) {
    d->m_typeList << MyMoneyAccount::CreditCard;
    d->m_typeList << MyMoneyAccount::Loan;
    d->m_typeList << MyMoneyAccount::Liability;
  } else if (group == MyMoneyAccount::Income) {
    d->m_typeList << MyMoneyAccount::Income;
  } else if (group == MyMoneyAccount::Expense) {
    d->m_typeList << MyMoneyAccount::Expense;
  } else if (group == MyMoneyAccount::Equity) {
    d->m_typeList << MyMoneyAccount::Equity;
  }
  invalidateFilter();
}

/**
  * Add the given account type to the filter.
  * @param type The account type to be added.
  * @see MyMoneyAccount::accountTypeE
  */
void AccountsFilterProxyModel::addAccountType(MyMoneyAccount::accountTypeE type)
{
  d->m_typeList << type;
  invalidateFilter();
}

/**
  * Remove the given account type from the filter.
  * @param type The account type to be removed.
  * @see MyMoneyAccount::accountTypeE
  */
void AccountsFilterProxyModel::removeAccountType(MyMoneyAccount::accountTypeE type)
{
  int index = d->m_typeList.indexOf(type);
  if (index != -1) {
    d->m_typeList.removeAt(index);
    invalidateFilter();
  }
}

/**
  * Use this to reset the filter.
  */
void AccountsFilterProxyModel::clear(void)
{
  d->m_typeList.clear();
  invalidateFilter();
}

/**
  * Implementation function that performs the actual filtering.
  */
bool AccountsFilterProxyModel::acceptSourceItem(const QModelIndex &source) const
{
  if (source.isValid()) {
    QVariant data = sourceModel()->data(source, AccountsModel::AccountRole);
    if (data.isValid()) {
      MyMoneyAccount account = data.value<MyMoneyAccount>();
      if ((hideClosedAccounts() && account.isClosed()))
        return false;

      // we hide stock accounts if not in expert mode
      if (account.isInvest() && !KMyMoneyGlobalSettings::expertMode())
        return false;

      // we hide equity accounts if not in expert mode
      if (account.accountType() == MyMoneyAccount::Equity && !KMyMoneyGlobalSettings::expertMode())
        return false;

      if (d->m_typeList.contains(account.accountType()))
        return true;
    }

    int rowCount = sourceModel()->rowCount(source);
    for (int i = 0; i < rowCount; ++i) {
      QModelIndex index = sourceModel()->index(i, 0, source);
      if (acceptSourceItem(index))
        return true;
    }
  }
  return false;
}

/**
  * Set if closed accounts should be hidden or not.
  * @param hideClosedAccounts
  */
void AccountsFilterProxyModel::setHideClosedAccounts(bool hideClosedAccounts)
{
  if (d->m_hideClosedAccounts != hideClosedAccounts) {
    d->m_hideClosedAccounts = hideClosedAccounts;
    invalidateFilter();
  }
}

/**
  * Check if closed accounts are hidden or not.
  */
bool AccountsFilterProxyModel::hideClosedAccounts(void) const
{
  return d->m_hideClosedAccounts;
}

#include "accountsmodel.moc"
