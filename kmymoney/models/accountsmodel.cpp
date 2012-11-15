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

#include <KLocalizedString>
#include <KIconLoader>
#include <KColorScheme>
#include <KDebug>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "kmymoneyutils.h"
#include "kmymoneyglobalsettings.h"

class AccountsModel::Private
{
public:
  /**
    * The pimpl.
    */
  Private() :
      m_file(MyMoneyFile::instance()) {
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

      QStandardItem *item = new QStandardItem(acc.name());
      accountsItem->appendRow(item);
      item->setColumnCount(model->columnCount());
      item->setEditable(false);

      item->setData(accountsItem->data(DisplayOrderRole), DisplayOrderRole);

      if (acc.accountList().count() > 0) {
        loadSubAccounts(model, item, favoriteAccountsItem, acc.accountList());
      }
      // set the account data after the children have been loaded
      setAccountData(model, item->index(), acc);

      if (acc.value("PreferredAccount") == "Yes") {
        QStandardItem *item = new QStandardItem(acc.name());
        favoriteAccountsItem->appendRow(item);
        item->setColumnCount(model->columnCount());
        item->setEditable(false);
        // set the account data after the children have been loaded
        setAccountData(model, item->index(), acc);
      }
    }
  }

  /**
    * Note: this functions should only be called after the child account data has been set.
    */
  void setAccountData(QStandardItemModel *model, const QModelIndex &index, const MyMoneyAccount &account) {
    // Account
    model->setData(index, account.name(), Qt::DisplayRole);
    model->setData(index, QVariant::fromValue(account), AccountRole);
    model->setData(index, QVariant(account.id()), AccountIdRole);
    model->setData(index, QVariant(account.value("PreferredAccount") == "Yes"), AccountFavoriteRole);
    model->setData(index, QVariant(QIcon(account.accountPixmap(account.id() == m_reconciledAccount.id()))), Qt::DecorationRole);

    QFont font = model->data(index, Qt::FontRole).value<QFont>();

    // display the names of closed accounts with strikeout font
    if (account.isClosed() != font.strikeOut()) {
      font.setStrikeOut(account.isClosed());
      model->setData(index, font, Qt::FontRole);
    }

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
          model->setData(newIndex, QVariant(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
        }
        break;
      default:
        break;
    }

    // balance and value
    setAccountBalanceAndValue(model, index, account);
  }

  void setAccountBalanceAndValue(QStandardItemModel *model, const QModelIndex &index, const MyMoneyAccount &account) {
    // set the account object also since it's balance has changed
    model->setData(index, QVariant::fromValue(account), AccountRole);
    // set the balance of the account
    MyMoneyMoney accountBalance = balance(account);
    model->setData(index, QVariant::fromValue(accountBalance), AccountBalanceRole);
    // set the value of the account
    MyMoneyMoney accountValue = value(account, accountBalance);
    model->setData(index, QVariant::fromValue(accountValue), AccountValueRole);
    // set the total value of the account
    MyMoneyMoney accountTotalValue = childrenTotalValue(index) + accountValue;
    model->setData(index, QVariant::fromValue(accountTotalValue), AccountTotalValueRole);

    QFont font = model->data(index, Qt::FontRole).value<QFont>();

    // Total Balance
    QModelIndex newIndex = model->index(index.row(), index.column() + TotalBalance, index.parent());
    // only show the balance, if its a different security/currency
    if (m_file->security(account.currencyId()) != m_file->baseCurrency()) {
      model->setData(newIndex, MyMoneyUtils::formatMoney(accountBalance, m_file->security(account.currencyId())), Qt::DisplayRole);
      model->setData(newIndex, MyMoneyUtils::formatMoney(accountBalance, m_file->security(account.currencyId())), AccountBalanceDisplayRole);
    }
    model->setData(newIndex, font, Qt::FontRole);
    model->setData(newIndex, QVariant(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);

    // Total Value
    newIndex = model->index(index.row(), index.column() + TotalValue, index.parent());
    model->setData(newIndex, MyMoneyUtils::formatMoney(accountValue, m_file->baseCurrency()), Qt::DisplayRole);
    model->setData(newIndex, MyMoneyUtils::formatMoney(accountValue, m_file->baseCurrency()), AccountValueDisplayRole);
    model->setData(newIndex, MyMoneyUtils::formatMoney(accountTotalValue, m_file->baseCurrency()), AccountTotalValueDisplayRole);
    model->setData(newIndex, font, Qt::FontRole);
    model->setData(newIndex, QVariant(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
  }

  /**
    * Compute the balance of the given account.
    *
    * @param account The account for which the balance is being computed.
    */
  MyMoneyMoney balance(const MyMoneyAccount &account) {
    MyMoneyMoney balance;
    // a closed account has a zero balance by definition
    if (!account.isClosed()) {
      // account.balance() is not compatable with stock accounts
      if (account.isInvest())
        balance = m_file->balance(account.id());
      else
        balance = account.balance();
    }

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
  MyMoneyMoney value(const MyMoneyAccount &account, const MyMoneyMoney &balance) {
    if (account.isClosed())
      return MyMoneyMoney();

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
        security = m_file->security(account.currencyId());
        prices += m_file->price(account.currencyId(), m_file->baseCurrency().id());
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
  MyMoneyMoney childrenTotalValue(const QModelIndex &index) {
    MyMoneyMoney totalValue;
    if (!index.isValid())
      return totalValue;

    const QAbstractItemModel *model = index.model();
    for (int i = 0; i < model->rowCount(index); ++i) {
      QModelIndex childIndex = model->index(i, index.column(), index);
      if (model->hasChildren(childIndex)) {
        totalValue += childrenTotalValue(childIndex);
      }
      QVariant data = model->data(childIndex, AccountValueRole);
      if (data.isValid()) {
        totalValue += data.value<MyMoneyMoney>();
      }
    }
    return totalValue;
  }

  /**
    * Function to get the item from an account id.
    *
    * @param parent The parent to localize the search in the child items of this parameter.
    * @param accountId Search based on this parameter.
    *
    * @return The item corresponding to the given account id, NULL if the account was not found.
    */
  QStandardItem *itemFromAccountId(QStandardItem *parent, const QString &accountId) {
    QStandardItemModel *model = parent->model();
    QModelIndexList list = model->match(model->index(0, 0, parent->index()), AccountsModel::AccountIdRole, QVariant(accountId), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));
    if (list.count() > 0) {
      return model->itemFromIndex(list.front());
    }
    // TODO: if not found at this item search for it in the model and if found reparent it.
    return 0;
  }

  /**
    * Function to get the item from an account id without knowing it's parent item.
    * Note that for the accounts which have two items in the model (favorite accounts)
    * the account item which is not the child of the favorite accounts item is always returned.
    *
    * @param model The model in which to search.
    * @param accountId Search based on this parameter.
    *
    * @return The item corresponding to the given account id, NULL if the account was not found.
    */
  QStandardItem *itemFromAccountId(QStandardItemModel *model, const QString &accountId) {
    QModelIndexList list = model->match(model->index(0, 0), AccountsModel::AccountIdRole, QVariant(accountId), -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
    foreach (const QModelIndex &index, list) {
      // always return the account which is not the child of the favorite accounts item
      if (model->data(index.parent(), AccountsModel::AccountIdRole).toString() != AccountsModel::favoritesAccountId) {
        return model->itemFromIndex(index);
      }
    }
    return 0;
  }

  /**
    * Used to load the accounts data.
    */
  MyMoneyFile *m_file;
  /**
    * Used to emit the @ref netWorthChanged signal.
    */
  MyMoneyMoney m_lastNetWorth;
  /**
    * Used to emit the @ref profitChanged signal.
    */
  MyMoneyMoney m_lastProfit;
  /**
    * Used to set the reconciliation flag.
    */
  MyMoneyAccount m_reconciledAccount;
};

const QString AccountsModel::favoritesAccountId("Favorites");

/**
  * The constructor is private so that only the @ref Models object can create such an object.
  */
AccountsModel::AccountsModel(QObject *parent /*= 0*/)
    : QStandardItemModel(parent), d(new Private)
{
  init();
}

AccountsModel::AccountsModel(Private* const priv, QObject *parent /*= 0*/)
    : QStandardItemModel(parent), d(priv)
{
  init();
}

AccountsModel::~AccountsModel()
{
  delete d;
}

void AccountsModel::init()
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

/**
  * Perform the initial liad of the model data
  * from the @ref MyMoneyFile.
  *
  */
void AccountsModel::load()
{
  //::timetrace("Start AccountsModel::load");
  QStandardItem *rootItem = invisibleRootItem();

  QFont font;
  font.setBold(true);

  // Favorite accounts
  QStandardItem *favoriteAccountsItem = new QStandardItem(i18n("Favorites"));
  rootItem->appendRow(favoriteAccountsItem);
  setData(favoriteAccountsItem->index(), favoritesAccountId, AccountIdRole);
  setData(favoriteAccountsItem->index(), 0, DisplayOrderRole);
  favoriteAccountsItem->setColumnCount(columnCount());
  favoriteAccountsItem->setIcon(QIcon(DesktopIcon("account"))); //krazy:exclude=iconnames
  favoriteAccountsItem->setEditable(false);
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

    QStandardItem *accountsItem = new QStandardItem(accountName);
    rootItem->appendRow(accountsItem);
    setData(accountsItem->index(), QVariant(displayOrder), DisplayOrderRole);
    accountsItem->setColumnCount(columnCount());
    accountsItem->setFont(font);
    accountsItem->setEditable(false);

    QStringList list = account.accountList();

    for (QStringList::ConstIterator it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
      const MyMoneyAccount& acc = d->m_file->account(*it_l);

      QStandardItem *item = new QStandardItem(acc.name());
      accountsItem->appendRow(item);
      item->setColumnCount(columnCount());
      item->setEditable(false);
      if (acc.accountList().count() > 0) {
        d->loadSubAccounts(this, item, favoriteAccountsItem, acc.accountList());
      }
      d->setAccountData(this, item->index(), acc);
      // set the account data after the children have been loaded
      if (acc.value("PreferredAccount") == "Yes") {
        item = new QStandardItem(acc.name());
        favoriteAccountsItem->appendRow(item);
        item->setColumnCount(columnCount());
        item->setEditable(false);
        d->setAccountData(this, item->index(), acc);
      }
    }

    d->setAccountData(this, accountsItem->index(), account);
  }

  checkNetWorth();
  checkProfit();
  //::timetrace("Done AccountsModel::load");
}

/**
  * Check if netWorthChanged should be emitted.
  */
void AccountsModel::checkNetWorth()
{
  // compute the net woth
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

  MyMoneyMoney netWorth;
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
  * Check if profitChanged should be emitted.
  */
void AccountsModel::checkProfit()
{
  // compute the profit
  QModelIndexList incomeList = match(index(0, 0),
                                     AccountsModel::AccountIdRole,
                                     MyMoneyFile::instance()->income().id(),
                                     1,
                                     Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));

  QModelIndexList expenseList = match(index(0, 0),
                                      AccountsModel::AccountIdRole,
                                      MyMoneyFile::instance()->expense().id(),
                                      1,
                                      Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));

  MyMoneyMoney profit;
  if (!incomeList.isEmpty() && !expenseList.isEmpty()) {
    QVariant incomeValue = data(incomeList.front(), AccountsModel::AccountTotalValueRole);
    QVariant expenseValue = data(expenseList.front(), AccountsModel::AccountTotalValueRole);

    if (incomeValue.isValid() && expenseValue.isValid()) {
      profit = incomeValue.value<MyMoneyMoney>() - expenseValue.value<MyMoneyMoney>();
    }
  }
  if (d->m_lastProfit != profit) {
    d->m_lastProfit = profit;
    emit profitChanged(d->m_lastProfit);
  }
}

MyMoneyMoney AccountsModel::accountValue(const MyMoneyAccount &account, const MyMoneyMoney &balance)
{
  return d->value(account, balance);
}

/**
  * This slot should be connected so that the model will be notified which account is being reconciled.
  */
void AccountsModel::slotReconcileAccount(const MyMoneyAccount &account, const QDate &reconciliationDate, const MyMoneyMoney &endingBalance)
{
  Q_UNUSED(reconciliationDate)
  Q_UNUSED(endingBalance)
  if (d->m_reconciledAccount.id() != account.id()) {
    // first clear the flag of the old reconciliation account
    if (!d->m_reconciledAccount.id().isEmpty()) {
      QModelIndexList list = match(index(0, 0), AccountsModel::AccountIdRole, QVariant(d->m_reconciledAccount.id()), -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
      foreach (const QModelIndex &index, list) {
        setData(index, QVariant(QIcon(account.accountPixmap(false))), Qt::DecorationRole);
      }
    }

    // then set the reconciliation flag of the new reconciliation account
    QModelIndexList list = match(index(0, 0), AccountsModel::AccountIdRole, QVariant(account.id()), -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
    foreach (const QModelIndex &index, list) {
      setData(index, QVariant(QIcon(account.accountPixmap(true))), Qt::DecorationRole);
    }
    d->m_reconciledAccount = account;
  }
}

/**
  * Notify the model that an object has been added. An action is performed only if the object is an account.
  *
  */
void AccountsModel::slotObjectAdded(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj)
{
  if (objType != MyMoneyFile::notifyAccount)
    return;

  const MyMoneyAccount * const account = dynamic_cast<const MyMoneyAccount * const>(obj);
  if (!account)
    return;

  QStandardItem *favoriteAccountsItem = d->itemFromAccountId(this, favoritesAccountId);
  QStandardItem *parentAccountItem = d->itemFromAccountId(this, account->parentAccountId());
  QStandardItem *item = d->itemFromAccountId(parentAccountItem, account->id());
  if (!item) {
    item = new QStandardItem(account->name());
    parentAccountItem->appendRow(item);
    item->setColumnCount(columnCount());
    item->setEditable(false);
  }
  // load the sub-accounts if there are any - there could be sub accounts if this is an add operation
  // that was triggered in slotObjectModified on an already existing account which went trough a hierarchy change
  if (account->accountList().count() > 0) {
    d->loadSubAccounts(this, item, favoriteAccountsItem, account->accountList());
  }
  d->setAccountData(this, item->index(), *account);
  // set the account data
  if (account->value("PreferredAccount") == "Yes") {
    QStandardItem *item = d->itemFromAccountId(favoriteAccountsItem, account->id());
    if (!item) {
      item = new QStandardItem(account->name());
      favoriteAccountsItem->appendRow(item);
      item->setColumnCount(columnCount());
      item->setEditable(false);
    }
    d->setAccountData(this, item->index(), *account);
  }

  checkNetWorth();
  checkProfit();
}

/**
  * Notify the model that an object has been modified. An action is performed only if the object is an account.
  *
  */
void AccountsModel::slotObjectModified(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj)
{
  if (objType != MyMoneyFile::notifyAccount)
    return;

  const MyMoneyAccount * const account = dynamic_cast<const MyMoneyAccount * const>(obj);
  if (!account)
    return;

  QStandardItem *favoriteAccountsItem = d->itemFromAccountId(this, favoritesAccountId);
  QStandardItem *accountItem = d->itemFromAccountId(this, account->id());
  MyMoneyAccount oldAccount = accountItem->data(AccountRole).value<MyMoneyAccount>();
  if (oldAccount.parentAccountId() == account->parentAccountId()) {
    // the hierarchy did not change so update the account data
    d->setAccountData(this, accountItem->index(), *account);
    // and the child of the favorite item if the account is a favorite account or it's favorite status has just changed
    QStandardItem *item = d->itemFromAccountId(favoriteAccountsItem, account->id());
    if (account->value("PreferredAccount") == "Yes") {
      if (!item) {
        // the favorite item for this account does not exist and the account is favorite
        item = new QStandardItem(account->name());
        favoriteAccountsItem->appendRow(item);
        item->setColumnCount(columnCount());
        item->setEditable(false);
      }
      d->setAccountData(this, item->index(), *account);
    } else {
      if (item) {
        // it's not favorite anymore
        removeRow(item->index().row(), item->index().parent());
      }
    }
  } else {
    // this means that the hierarchy was changed - simulate this with a remove followed by and add operation
    slotObjectRemoved(MyMoneyFile::notifyAccount, oldAccount.id());
    slotObjectAdded(MyMoneyFile::notifyAccount, obj);
  }

  checkNetWorth();
  checkProfit();
}

/**
  * Notify the model that an object has been removed. An action is performed only if the object is an account.
  *
  */
void AccountsModel::slotObjectRemoved(MyMoneyFile::notificationObjectT objType, const QString& id)
{
  if (objType != MyMoneyFile::notifyAccount)
    return;

  QModelIndexList list = match(index(0, 0), AccountsModel::AccountIdRole, id, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
  foreach (const QModelIndex &index, list) {
    removeRow(index.row(), index.parent());
  }

  checkNetWorth();
  checkProfit();
}

/**
  * Notify the model that the account balance has been changed.
  */
void AccountsModel::slotBalanceOrValueChanged(const MyMoneyAccount &account)
{
  QStandardItem *currentItem = d->itemFromAccountId(this, account.id());
  const MyMoneyAccount *currentAccount = &account;
  while (currentItem) {
    d->setAccountBalanceAndValue(this, currentItem->index(), *currentAccount);
    currentItem = currentItem->parent();
    currentAccount = &d->m_file->account(currentAccount->parentAccountId());
  }
  checkNetWorth();
  checkProfit();
}

/**
  * The pimpl of the @ref InstitutionsModel derived from the pimpl of the @ref AccountsModel.
  */
class InstitutionsModel::InstitutionsPrivate : public AccountsModel::Private
{
public:
  /**
    * Function to get the institution item from an institution id.
    *
    * @param model The model in which to look for the item.
    * @param institutionId Search based on this parameter.
    *
    * @return The item corresponding to the given institution id, NULL if the institution was not found.
    */
  QStandardItem *institutionItemFromId(QStandardItemModel *model, const QString &institutionId) {
    QModelIndexList list = model->match(model->index(0, 0), AccountsModel::AccountIdRole, QVariant(institutionId), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));
    if (list.count() > 0) {
      return model->itemFromIndex(list.front());
    }
    return 0;
  }

  /**
    * Function to add the account item to it's corresponding institution item.
    *
    * @param model The model where to add the item.
    * @param account The account for which to create the item.
    *
    */
  void loadInstitution(QStandardItemModel *model, const MyMoneyAccount &account) {
    if (!account.isAssetLiability() && !account.isInvest())
      return;

    QFont font;
    font.setBold(true);

    QString institutionId = account.institutionId();
    if (account.isInvest()) {
      MyMoneyAccount parentAccount = m_file->account(account.parentAccountId());
      institutionId = parentAccount.institutionId();
    }
    QStandardItem *institutionItem = institutionItemFromId(model, institutionId);
    QStandardItem *item = itemFromAccountId(institutionItem, account.id());
    // only investment accounts are added to their parent in the institutions view
    // this makes hierarchy maintenance a lot easier since the investment accounts
    // are the only ones that always have the same institution as their parent
    QStandardItem *parentAccounItem = account.isInvest() ? itemFromAccountId(institutionItem, account.parentAccountId()) : 0;
    if (!item) {
      item = new QStandardItem(account.name());
      if (parentAccounItem) {
        parentAccounItem->appendRow(item);
      } else {
        institutionItem->appendRow(item);
      }
      item->setColumnCount(model->columnCount());
      item->setEditable(false);
    }
    setAccountData(model, item->index(), account);
    if (parentAccounItem)
      setAccountData(model, parentAccounItem->index(), m_file->account(account.parentAccountId()));
    MyMoneyMoney accountTotalValue = item->data(AccountTotalValueRole).value<MyMoneyMoney>();
    if (account.accountGroup() == MyMoneyAccount::Liability) {
      accountTotalValue = -accountTotalValue; // the account is a liability so change the account value sign
    }
    MyMoneyMoney institutionValue = institutionItem->data(AccountTotalValueRole).value<MyMoneyMoney>() + accountTotalValue;
    institutionItem->setData(QVariant::fromValue(institutionValue), AccountTotalValueRole);
    QModelIndex instIndex = institutionItem->index();
    QModelIndex newIndex = model->index(instIndex.row(), instIndex.column() + TotalValue, instIndex.parent());
    if (institutionValue.isNegative()) {
      model->setData(newIndex, KMyMoneyGlobalSettings::listNegativeValueColor(), Qt::ForegroundRole);
    } else {
      model->setData(newIndex, KColorScheme(QPalette::Active).foreground(KColorScheme::NormalText).color(), Qt::ForegroundRole);
    }

    model->setData(newIndex, MyMoneyUtils::formatMoney(institutionValue, m_file->baseCurrency()), Qt::DisplayRole);
    model->setData(newIndex, MyMoneyUtils::formatMoney(institutionValue, m_file->baseCurrency()), AccountTotalValueDisplayRole);
    model->setData(newIndex, font, Qt::FontRole);
    model->setData(newIndex, QVariant(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
  }

  /**
    * Function to add an institution item to the model.
    *
    * @param model The model in which to add the item.
    * @param institution The institution object which should be represented by the item.
    *
    */
  void addInstitutionItem(QStandardItemModel *model, const MyMoneyInstitution &institution) {
    QFont font;
    font.setBold(true);
    QStandardItem *institutionItem = new QStandardItem(institution.name());
    model->invisibleRootItem()->appendRow(institutionItem);
    institutionItem->setData(institution.name(), Qt::DisplayRole);
    institutionItem->setData(QVariant::fromValue(institution), AccountRole);
    institutionItem->setData(QVariant::fromValue(MyMoneyMoney()), AccountBalanceRole);
    institutionItem->setData(QVariant::fromValue(MyMoneyMoney()), AccountValueRole);
    institutionItem->setData(QVariant::fromValue(MyMoneyMoney()), AccountTotalValueRole);
    institutionItem->setData(institution.id(), AccountIdRole);
    institutionItem->setData(6, DisplayOrderRole);
    institutionItem->setColumnCount(model->columnCount());
    institutionItem->setIcon(institution.pixmap());
    institutionItem->setFont(font);
    institutionItem->setEditable(false);
  }
};

/**
  * The institution model contains the accounts grouped by institution.
  *
  */
InstitutionsModel::InstitutionsModel(QObject *parent /*= 0*/)
    : AccountsModel(new InstitutionsPrivate, parent)
{
}

/**
  * Perform the initial liad of the model data
  * from the @ref MyMoneyFile.
  *
  */
void InstitutionsModel::load()
{
  // create items for all the institutions
  QList<MyMoneyInstitution> institutionList;
  d->m_file->institutionList(institutionList);
  MyMoneyInstitution none;
  none.setName(i18n("Accounts with no institution assigned"));
  institutionList.append(none);
  foreach (const MyMoneyInstitution &institution, institutionList) {
    static_cast<InstitutionsPrivate *>(d)->addInstitutionItem(this, institution);
  }

  // create items for all the accounts
  QList<MyMoneyAccount> list;
  d->m_file->accountList(list);
  for (QList<MyMoneyAccount>::ConstIterator it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
    static_cast<InstitutionsPrivate *>(d)->loadInstitution(this, *it_l);
  }
}

/**
  * Notify the model that an object has been added. An action is performed only if the object is an account or an institution.
  *
  */
void InstitutionsModel::slotObjectAdded(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj)
{
  if (objType == MyMoneyFile::notifyInstitution) {
    // if an institution was added then add the item which will represent it
    const MyMoneyInstitution * const institution = dynamic_cast<const MyMoneyInstitution * const>(obj);
    if (!institution)
      return;
    static_cast<InstitutionsPrivate *>(d)->addInstitutionItem(this, *institution);
  }

  if (objType != MyMoneyFile::notifyAccount)
    return;

  // if an account was added then add the item which will represent it only for real accounts
  const MyMoneyAccount * const account = dynamic_cast<const MyMoneyAccount * const>(obj);
  // nothing to do for root accounts and categories
  if (!account || account->parentAccountId().isEmpty() || account->isIncomeExpense())
    return;

  // load the account into the institution
  static_cast<InstitutionsPrivate *>(d)->loadInstitution(this, *account);

  // load the investment sub-accounts if there are any - there could be sub-accounts if this is an add operation
  // that was triggered in slotObjectModified on an already existing account which went trough a hierarchy change
  QList<MyMoneyAccount> subAccounts;
  d->m_file->accountList(subAccounts, account->accountList(), true);
  for (QList<MyMoneyAccount>::ConstIterator it_a = subAccounts.constBegin(); it_a != subAccounts.constEnd(); ++it_a) {
    if ((*it_a).isInvest()) {
      static_cast<InstitutionsPrivate *>(d)->loadInstitution(this, *it_a);
    }
  }
}

/**
  * Notify the model that an object has been modified. An action is performed only if the object is an account or an institution.
  *
  */
void InstitutionsModel::slotObjectModified(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj)
{
  if (objType == MyMoneyFile::notifyInstitution) {
    // if an institution was modified then modify the item which represents it
    const MyMoneyInstitution * const institution = dynamic_cast<const MyMoneyInstitution * const>(obj);
    if (!institution)
      return;
    QStandardItem *institutionItem = static_cast<InstitutionsPrivate *>(d)->institutionItemFromId(this, institution->id());
    institutionItem->setData(institution->name(), Qt::DisplayRole);
    institutionItem->setData(QVariant::fromValue(*institution), AccountRole);
    institutionItem->setIcon(institution->pixmap());
  }

  if (objType != MyMoneyFile::notifyAccount)
    return;

  // if an account was modified then modify the item which represents it
  const MyMoneyAccount * const account = dynamic_cast<const MyMoneyAccount * const>(obj);
  // nothing to do for root accounts, categories and equity accounts since they don't have a representation in this model
  if (!account || account->parentAccountId().isEmpty() || account->isIncomeExpense() || account->accountType() == MyMoneyAccount::Equity)
    return;

  QStandardItem *accountItem = d->itemFromAccountId(this, account->id());
  MyMoneyAccount oldAccount = accountItem->data(AccountRole).value<MyMoneyAccount>();
  if (oldAccount.institutionId() == account->institutionId()) {
    // the hierarchy did not change so update the account data
    d->setAccountData(this, accountItem->index(), *account);
  } else {
    // this means that the hierarchy was changed - simulate this with a remove followed by and add operation
    slotObjectRemoved(MyMoneyFile::notifyAccount, oldAccount.id());
    slotObjectAdded(MyMoneyFile::notifyAccount, obj);
  }
}

/**
  * Notify the model that an object has been removed. An action is performed only if the object is an account or an institution.
  *
  */
void InstitutionsModel::slotObjectRemoved(MyMoneyFile::notificationObjectT objType, const QString& id)
{
  if (objType == MyMoneyFile::notifyInstitution) {
    // if an institution was removed then remove the item which represents it
    QStandardItem *institutionItem = static_cast<InstitutionsPrivate *>(d)->institutionItemFromId(this, id);
    if (institutionItem)
      removeRow(institutionItem->row(), institutionItem->index().parent());
  }

  if (objType != MyMoneyFile::notifyAccount)
    return;

  // if an account was removed then remove the item which represents it and recompute the institution's value
  QStandardItem *accountItem = d->itemFromAccountId(this, id);
  if (!accountItem)
    return; // this could happen if the account isIncomeExpense
  MyMoneyAccount oldAccount = accountItem->data(AccountRole).value<MyMoneyAccount>();
  QStandardItem *institutionItem = d->itemFromAccountId(this, oldAccount.institutionId());

  MyMoneyMoney accountTotalValue = accountItem->data(AccountTotalValueRole).value<MyMoneyMoney>();
  if (oldAccount.accountGroup() == MyMoneyAccount::Liability) {
    accountTotalValue = -accountTotalValue; // the account is a liability so change the account value sign
  }

  MyMoneyMoney institutionValue = institutionItem->data(AccountTotalValueRole).value<MyMoneyMoney>() - accountTotalValue;
  institutionItem->setData(QVariant::fromValue(institutionValue), AccountTotalValueRole);
  QModelIndex instIndex = institutionItem->index();
  QModelIndex newIndex = index(instIndex.row(), instIndex.column() + TotalValue, instIndex.parent());
  if (institutionValue.isNegative()) {
    setData(newIndex, KMyMoneyGlobalSettings::listNegativeValueColor(), Qt::ForegroundRole);
  } else {
    setData(newIndex, KColorScheme(QPalette::Active).foreground(KColorScheme::NormalText).color(), Qt::ForegroundRole);
  }

  QFont font;
  font.setBold(true);

  setData(newIndex, MyMoneyUtils::formatMoney(institutionValue, d->m_file->baseCurrency()), Qt::DisplayRole);
  setData(newIndex, MyMoneyUtils::formatMoney(institutionValue, d->m_file->baseCurrency()), AccountTotalValueDisplayRole);
  setData(newIndex, font, Qt::FontRole);
  setData(newIndex, QVariant(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);

  AccountsModel::slotObjectRemoved(objType, id);
}

/**
  * The pimpl.
  */
class AccountsFilterProxyModel::Private
{
public:
  Private() :
      m_hideClosedAccounts(true),
      m_hideEquityAccounts(true),
      m_hideUnusedIncomeExpenseAccounts(false) {
  }

  ~Private() {
  }

  QList<MyMoneyAccount::accountTypeE> m_typeList;
  bool m_hideClosedAccounts;
  bool m_hideEquityAccounts;
  bool m_hideUnusedIncomeExpenseAccounts;
  bool m_haveHiddenUnusedIncomeExpenseAccounts;
};

AccountsFilterProxyModel::AccountsFilterProxyModel(QObject *parent /*= 0*/)
    : QSortFilterProxyModel(parent), d(new Private)
{
  setDynamicSortFilter(true);
  setFilterKeyColumn(-1);
  setSortLocaleAware(true);
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

AccountsFilterProxyModel::~AccountsFilterProxyModel()
{
  delete d;
}

/**
  * This function was re-implemented so we could have a special display order (favorites first)
  */
bool AccountsFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  // different sorting based on the column which is being sorted
  switch (left.column()) {
      // for the accounts column sort based on the DisplayOrderRole
    case AccountsModel::Account: {
        QVariant leftData = sourceModel()->data(left, AccountsModel::DisplayOrderRole);
        QVariant rightData = sourceModel()->data(right, AccountsModel::DisplayOrderRole);

        if (leftData.toInt() == rightData.toInt()) {
          // sort items of the same display order alphabetically
          return QSortFilterProxyModel::lessThan(left, right);
        }
        return leftData.toInt() < rightData.toInt();
      }
      // the total balance and value columns are sorted based on the value of the account
    case AccountsModel::TotalBalance:
    case AccountsModel::TotalValue: {
        QVariant leftData = sourceModel()->data(sourceModel()->index(left.row(), 0, left.parent()), AccountsModel::AccountTotalValueRole);
        QVariant rightData = sourceModel()->data(sourceModel()->index(right.row(), 0, right.parent()), AccountsModel::AccountTotalValueRole);
        return leftData.value<MyMoneyMoney>() < rightData.value<MyMoneyMoney>();
      }
  }
  return QSortFilterProxyModel::lessThan(left, right);
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
  invalidate();
}

/**
  * Add the given account type to the filter.
  * @param type The account type to be added.
  * @see MyMoneyAccount::accountTypeE
  */
void AccountsFilterProxyModel::addAccountType(MyMoneyAccount::accountTypeE type)
{
  d->m_typeList << type;
  invalidate();
}

/**
  * Remove the given account type from the filter.
  * @param type The account type to be removed.
  * @see MyMoneyAccount::accountTypeE
  */
void AccountsFilterProxyModel::removeAccountType(MyMoneyAccount::accountTypeE type)
{
  if (d->m_typeList.removeAll(type) > 0) {
    invalidate();
  }
}

/**
  * Use this to reset the filter.
  */
void AccountsFilterProxyModel::clear(void)
{
  d->m_typeList.clear();
  invalidate();
}

/**
  * Implementation function that performs the actual filtering.
  */
bool AccountsFilterProxyModel::acceptSourceItem(const QModelIndex &source) const
{
  if (source.isValid()) {
    QVariant data = sourceModel()->data(source, AccountsModel::AccountRole);
    if (data.isValid()) {
      if (data.canConvert<MyMoneyAccount>()) {
        MyMoneyAccount account = data.value<MyMoneyAccount>();
        if ((hideClosedAccounts() && account.isClosed()))
          return false;

        // we hide stock accounts if not in expert mode
        if (account.isInvest() && hideEquityAccounts())
          return false;

        // we hide equity accounts if not in expert mode
        if (account.accountType() == MyMoneyAccount::Equity && hideEquityAccounts())
          return false;

        // we hide unused income and expense accounts if the specific flag is set
        if ((account.accountType() == MyMoneyAccount::Income || account.accountType() == MyMoneyAccount::Expense) && hideUnusedIncomeExpenseAccounts()) {
          QVariant totalValue = sourceModel()->data(source, AccountsModel::AccountTotalValueRole);
          if (totalValue.isValid() && totalValue.value<MyMoneyMoney>().isZero()) {
            emit unusedIncomeExpenseAccountHidden();
            return false;
          }
        }

        if (d->m_typeList.contains(account.accountType()))
          return true;
      }
      if (data.canConvert<MyMoneyInstitution>() && sourceModel()->rowCount(source) == 0) {
        // if this is an institution that has no children show it only if hide unused institutions (hide closed accounts for now) is not checked
        return !hideClosedAccounts();
      }
      // let the visibility of all other institutions (the ones with children) be controlled by the visibility of their children
    }

    // all parents that have at least one visible child must be visible
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
    invalidate();
  }
}

/**
  * Check if closed accounts are hidden or not.
  */
bool AccountsFilterProxyModel::hideClosedAccounts(void) const
{
  return d->m_hideClosedAccounts;
}

/**
  * Set if equity and investment accounts should be hidden or not.
  * @param hideEquityAccounts
  */
void AccountsFilterProxyModel::setHideEquityAccounts(bool hideEquityAccounts)
{
  if (d->m_hideEquityAccounts != hideEquityAccounts) {
    d->m_hideEquityAccounts = hideEquityAccounts;
    invalidate();
  }
}

/**
  * Check if equity and investment accounts are hidden or not.
  */
bool AccountsFilterProxyModel::hideEquityAccounts(void) const
{
  return d->m_hideEquityAccounts;
}

/**
  * Set if empty categories should be hidden or not.
  * @param hideUnusedIncomeExpenseAccounts
  */
void AccountsFilterProxyModel::setHideUnusedIncomeExpenseAccounts(bool hideUnusedIncomeExpenseAccounts)
{
  if (d->m_hideUnusedIncomeExpenseAccounts != hideUnusedIncomeExpenseAccounts) {
    d->m_hideUnusedIncomeExpenseAccounts = hideUnusedIncomeExpenseAccounts;
    invalidate();
  }
}

/**
  * Check if empty categories are hidden or not.
  */
bool AccountsFilterProxyModel::hideUnusedIncomeExpenseAccounts(void) const
{
  return d->m_hideUnusedIncomeExpenseAccounts;
}

#include "accountsmodel.moc"
