/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2017  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com       *
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

#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "kmymoneyglobalsettings.h"
#include "icons.h"
#include "modelenums.h"
#include "mymoneyenums.h"

using namespace Icons;
using namespace eAccountsModel;
using namespace eMyMoney;

class AccountsModel::Private
{
public:
  /**
    * The pimpl.
    */
  Private() :
      m_file(MyMoneyFile::instance()) {
    m_columns.append(Column::Account);
  }

  ~Private() {
  }

  void loadPreferredAccount(const MyMoneyAccount &acc, QStandardItem *fromNode /*accounts' regular node*/, const int row, QStandardItem *toNode /*accounts' favourite node*/)
  {
    if (acc.value(QStringLiteral("PreferredAccount")) != QLatin1String("Yes"))
      return;

    auto favRow = toNode->rowCount();
    auto favItem = itemFromAccountId(toNode, acc.id());
    if (favItem)
      favRow = favItem->row();

    for (auto i = 0; i < fromNode->columnCount(); ++i) {
      auto itemToClone = fromNode->child(row, i);
      if (itemToClone)
        toNode->setChild(favRow, i, itemToClone->clone());
    }
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
  void loadSubaccounts(QStandardItem *node, QStandardItem *favoriteAccountsItem, const QStringList& subaccounts)
  {
    if (subaccounts.isEmpty())
      return;
    foreach (const auto subaccStr, subaccounts) {
      const auto subacc = m_file->account(subaccStr);

      auto item = new QStandardItem(subacc.name());                         // initialize first column of subaccount
      node->appendRow(item);                                                // add subaccount row to node
      item->setEditable(false);

      item->setData(node->data((int)Role::DisplayOrder), (int)Role::DisplayOrder);        // inherit display order role from node

      loadSubaccounts(item, favoriteAccountsItem, subacc.accountList());    // subaccount may have subaccounts as well

      // set the account data after the children have been loaded
      const auto row = item->row();
      setAccountData(node, row, subacc, m_columns);                          // initialize rest of columns of subaccount
      loadPreferredAccount(subacc, node, row, favoriteAccountsItem);         // add to favourites node if preferred
    }
  }

  /**
    * Note: this functions should only be called after the child account data has been set.
    */
  void setAccountData(QStandardItem *node, const int row, const MyMoneyAccount &account, const QList<Column> &columns)
  {
    QStandardItem *cell;

    auto getCell = [&, row](const auto column) {
      cell = node->child(row, column);      // try to get QStandardItem
      if (!cell) {                          // it may be uninitialized
        cell = new QStandardItem;           // so create one
        node->setChild(row, column, cell);  // and add it under the node
      }
    };

    auto colNum = m_columns.indexOf(Column::Account);
    if (colNum == -1)
      return;
    getCell(colNum);
    auto font = cell->data(Qt::FontRole).value<QFont>();
    // display the names of closed accounts with strikeout font
    if (account.isClosed() != font.strikeOut())
      font.setStrikeOut(account.isClosed());

    if (columns.contains(Column::Account)) {
      // setting account column
      cell->setData(account.name(), Qt::DisplayRole);
//      cell->setData(QVariant::fromValue(account), (int)Role::Account); // is set in setAccountBalanceAndValue
      cell->setData(QVariant(account.id()), (int)Role::ID);
      cell->setData(QVariant(account.value("PreferredAccount") == QLatin1String("Yes")), (int)Role::Favorite);
      cell->setData(QVariant(QIcon(account.accountPixmap(m_reconciledAccount.id().isEmpty() ? false : account.id() == m_reconciledAccount.id()))), Qt::DecorationRole);
      cell->setData(MyMoneyFile::instance()->accountToCategory(account.id(), true), (int)Role::FullName);
      cell->setData(font, Qt::FontRole);
    }

    // Type
    if (columns.contains(Column::Type)) {
      colNum = m_columns.indexOf(Column::Type);
      if (colNum != -1) {
        getCell(colNum);
        cell->setData(account.accountTypeToString(account.accountType()), Qt::DisplayRole);
        cell->setData(font, Qt::FontRole);
      }
    }

    // Account's number
    if (columns.contains(Column::AccountNumber)) {
      colNum = m_columns.indexOf(Column::AccountNumber);
      if (colNum != -1) {
        getCell(colNum);
        cell->setData(account.number(), Qt::DisplayRole);
        cell->setData(font, Qt::FontRole);
      }
    }

    // Account's sort code
    if (columns.contains(Column::AccountSortCode)) {
      colNum = m_columns.indexOf(Column::AccountSortCode);
      if (colNum != -1) {
        getCell(colNum);
        cell->setData(account.value("iban"), Qt::DisplayRole);
        cell->setData(font, Qt::FontRole);
      }
    }

    const auto checkMark = QIcon::fromTheme(g_Icons[Icon::DialogOK]);
    switch (account.accountType()) {
      case Account::Type::Income:
      case Account::Type::Expense:
      case Account::Type::Asset:
      case Account::Type::Liability:
        // Tax
        if (columns.contains(Column::Tax)) {
          colNum = m_columns.indexOf(Column::Tax);
          if (colNum != -1) {
            getCell(colNum);
            if (account.value("Tax").toLower() == "yes")
              cell->setData(checkMark, Qt::DecorationRole);
            else
              cell->setData(QIcon(), Qt::DecorationRole);
          }
        }

        // VAT Account
        if (columns.contains(Column::VAT)) {
          colNum = m_columns.indexOf(Column::VAT);
          if (colNum != -1) {
            getCell(colNum);
            if (!account.value("VatAccount").isEmpty()) {
              const auto vatAccount = MyMoneyFile::instance()->account(account.value("VatAccount"));
              cell->setData(vatAccount.name(), Qt::DisplayRole);
              cell->setData(QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);

              // VAT Rate
            } else if (!account.value("VatRate").isEmpty()) {
              const auto vatRate = MyMoneyMoney(account.value("VatRate")) * MyMoneyMoney(100, 1);
              cell->setData(QString::fromLatin1("%1 %").arg(vatRate.formatMoney(QString(), 1)), Qt::DisplayRole);
              cell->setData(QVariant(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);

            } else {
              cell->setData(QString(), Qt::DisplayRole);
            }
          }
        }

        // CostCenter
        if (columns.contains(Column::CostCenter)) {
          colNum = m_columns.indexOf(Column::CostCenter);
          if (colNum != -1) {
            getCell(colNum);
            if (account.isCostCenterRequired())
              cell->setData(checkMark, Qt::DecorationRole);
            else
              cell->setData(QIcon(), Qt::DecorationRole);
          }
        }
        break;
      default:
        break;
    }

    // balance and value
    setAccountBalanceAndValue(node, row, account, columns);
  }

  void setInstitutionTotalValue(QStandardItem *node, const int row)
  {
    const auto colInstitution = m_columns.indexOf(Column::Account);
    auto itInstitution = node->child(row, colInstitution);
    const auto valInstitution = childrenTotalValue(itInstitution, true);
    itInstitution->setData(QVariant::fromValue(valInstitution ), (int)Role::TotalValue);

    const auto colTotalValue = m_columns.indexOf(Column::TotalValue);
    if (colTotalValue == -1)
      return;
    auto cell = node->child(row, colTotalValue);
    if (!cell) {
      cell = new QStandardItem;
      node->setChild(row, colTotalValue, cell);
    }
    QColor color;
    if (valInstitution.isNegative())
      color = KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative);
    else
      color = KColorScheme(QPalette::Active).foreground(KColorScheme::NormalText).color();

    cell->setData(QVariant(color),                                                   Qt::ForegroundRole);
    cell->setData(QVariant(itInstitution->data(Qt::FontRole).value<QFont>()),        Qt::FontRole);
    cell->setData(QVariant(Qt::AlignRight | Qt::AlignVCenter),                       Qt::TextAlignmentRole);
    cell->setData(MyMoneyUtils::formatMoney(valInstitution, m_file->baseCurrency()), Qt::DisplayRole);
  }

  void setAccountBalanceAndValue(QStandardItem *node, const int row, const MyMoneyAccount &account, const QList<Column> &columns)
  {
    QStandardItem *cell;

    auto getCell = [&, row](auto column)
    {
      cell = node->child(row, column);
      if (!cell) {
        cell = new QStandardItem;
        node->setChild(row, column, cell);
      }
    };

    // setting account column
    auto colNum = m_columns.indexOf(Column::Account);
    if (colNum == -1)
      return;
    getCell(colNum);

    MyMoneyMoney accountBalance, accountValue, accountTotalValue;
    if (columns.contains(Column::Account)) { // update values only when requested
      accountBalance    = balance(account);
      accountValue      = value(account, accountBalance);
      accountTotalValue = childrenTotalValue(cell) + accountValue;
      cell->setData(QVariant::fromValue(account),           (int)Role::Account);
      cell->setData(QVariant::fromValue(accountBalance),    (int)Role::Balance);
      cell->setData(QVariant::fromValue(accountValue),      (int)Role::Value);
      cell->setData(QVariant::fromValue(accountTotalValue), (int)Role::TotalValue);
    } else {  // otherwise save up on tedious calculations
      accountBalance    = cell->data((int)Role::Balance).value<MyMoneyMoney>();
      accountValue      = cell->data((int)Role::Value).value<MyMoneyMoney>();
      accountTotalValue = cell->data((int)Role::TotalValue).value<MyMoneyMoney>();
    }

    const auto font = QVariant(cell->data(Qt::FontRole).value<QFont>());
    const auto alignment = QVariant(Qt::AlignRight | Qt::AlignVCenter);

    // setting total balance column
    if (columns.contains(Column::TotalBalance)) {
      colNum = m_columns.indexOf(Column::TotalBalance);
      if (colNum != -1) {
        const auto accountBalanceStr = QVariant::fromValue(MyMoneyUtils::formatMoney(accountBalance, m_file->security(account.currencyId())));
        getCell(colNum);
        // only show the balance, if its a different security/currency
        if (m_file->security(account.currencyId()) != m_file->baseCurrency()) {
          cell->setData(accountBalanceStr, Qt::DisplayRole);
        }
        cell->setData(font,       Qt::FontRole);
        cell->setData(alignment,  Qt::TextAlignmentRole);
      }
    }

    // setting posted value column
    if (columns.contains(Column::PostedValue)) {
      colNum = m_columns.indexOf(Column::PostedValue);
      if (colNum != -1) {
        const auto accountValueStr = QVariant::fromValue(MyMoneyUtils::formatMoney(accountValue, m_file->baseCurrency()));
        getCell(colNum);
        cell->setData(accountValueStr, Qt::DisplayRole);
        cell->setData(font,       Qt::FontRole);
        cell->setData(alignment,  Qt::TextAlignmentRole);
      }
    }

    // setting total value column
    if (columns.contains(Column::TotalValue)) {
      colNum = m_columns.indexOf(Column::TotalValue);
      if (colNum != -1) {
        const auto accountTotalValueStr = QVariant::fromValue(MyMoneyUtils::formatMoney(accountTotalValue, m_file->baseCurrency()));
        getCell(colNum);
        QColor color;
        if (accountTotalValue.isNegative())
          color = KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative);
        else
          color = KColorScheme(QPalette::Active).foreground(KColorScheme::NormalText).color();

        cell->setData(accountTotalValueStr, Qt::DisplayRole);
        cell->setData(font,                 Qt::FontRole);
        cell->setData(QVariant(color),      Qt::ForegroundRole);
        cell->setData(alignment,            Qt::TextAlignmentRole);
      }
    }
  }

  /**
    * Compute the balance of the given account.
    *
    * @param account The account for which the balance is being computed.
    */
  MyMoneyMoney balance(const MyMoneyAccount &account)
  {
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
      case Account::Type::Income:
      case Account::Type::Liability:
      case Account::Type::Equity:
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

    } catch (const MyMoneyException &e) {
      qDebug() << Q_FUNC_INFO << " caught exception while adding " << account.name() << "[" << account.id() << "]: " << e.what();
    }

    MyMoneyMoney value = balance;
    {
      QList<MyMoneyPrice>::const_iterator it_p;
      QString security = account.currencyId();
      for (it_p = prices.constBegin(); it_p != prices.constEnd(); ++it_p) {
        value = (value * (MyMoneyMoney::ONE / (*it_p).rate(security))).convertPrecision(m_file->security(security).pricePrecision());
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
    * of all sub-account must be already in the model in the @ref Role::Value.
    *
    * @param index The index of the account in the model.
    * @see value
    */
  MyMoneyMoney childrenTotalValue(const QStandardItem *node, const bool isInstitutionsModel = false)
  {
    MyMoneyMoney totalValue;
    if (!node)
      return totalValue;

    for (auto i = 0; i < node->rowCount(); ++i) {
      const auto childNode = node->child(i, (int)Column::Account);
      if (childNode->hasChildren())
        totalValue += childrenTotalValue(childNode, isInstitutionsModel);
      const auto data = childNode->data((int)Role::Value);
      if (data.isValid()) {
        auto value = data.value<MyMoneyMoney>();
        if (isInstitutionsModel) {
          const auto account = childNode->data((int)Role::Account).value<MyMoneyAccount>();
          if (account.accountGroup() == Account::Type::Liability)
            value = -value;
        }
      totalValue += value;
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
    auto const model = parent->model();
    const auto list = model->match(model->index(0, 0, parent->index()), (int)Role::ID, QVariant(accountId), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));
    if (!list.isEmpty())
      return model->itemFromIndex(list.front());
    // TODO: if not found at this item search for it in the model and if found reparent it.
    return nullptr;
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
  QStandardItem *itemFromAccountId(QStandardItemModel *model, const QString &accountId)
  {
    const auto list = model->match(model->index(0, 0), (int)Role::ID, QVariant(accountId), -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
    foreach (const QModelIndex &index, list) {
      // always return the account which is not the child of the favorite accounts item
      if (index.parent().data((int)Role::ID).toString() != AccountsModel::favoritesAccountId)
        return model->itemFromIndex(index);
    }
    return nullptr;
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

  QList<Column> m_columns;
  static const QString m_accountsModelConfGroup;
  static const QString m_accountsModelColumnSelection;
};

const QString AccountsModel::Private::m_accountsModelConfGroup = QStringLiteral("AccountsModel");
const QString AccountsModel::Private::m_accountsModelColumnSelection = QStringLiteral("ColumnSelection");

const QString AccountsModel::favoritesAccountId(QStringLiteral("Favorites"));

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
  foreach (const auto column, d->m_columns)
    headerLabels.append(getHeaderName(column));
  setHorizontalHeaderLabels(headerLabels);
}

/**
  * Perform the initial load of the model data
  * from the @ref MyMoneyFile.
  *
  */
void AccountsModel::load()
{
  this->blockSignals(true);
  QStandardItem *rootItem = invisibleRootItem();

  QFont font;
  font.setBold(true);

  // adding favourite accounts node
  auto favoriteAccountsItem = new QStandardItem();
  favoriteAccountsItem->setEditable(false);
  rootItem->appendRow(favoriteAccountsItem);
  {
    QMap<int, QVariant> itemData;
    itemData[Qt::DisplayRole] = itemData[Qt::EditRole] = itemData[(int)Role::FullName] = i18n("Favorites");
    itemData[Qt::FontRole] = font;
    itemData[Qt::DecorationRole] = QIcon::fromTheme(g_Icons.value(Icon::ViewBankAccount));
    itemData[(int)Role::ID] = favoritesAccountId;
    itemData[(int)Role::DisplayOrder] = 0;
    this->setItemData(favoriteAccountsItem->index(), itemData);
  }

  // adding account categories (asset, liability, etc.) node
  QVector <Account::Type> categories {
    Account::Type::Asset, Account::Type::Liability,
    Account::Type::Income, Account::Type::Expense,
    Account::Type::Equity
  };

  foreach (const auto category, categories) {
    MyMoneyAccount account;
    QString accountName;
    int displayOrder;

    switch (category) {
      case Account::Type::Asset:
        // Asset accounts
        account = d->m_file->asset();
        accountName = i18n("Asset accounts");
        displayOrder = 1;
        break;
      case Account::Type::Liability:
        // Liability accounts
        account = d->m_file->liability();
        accountName = i18n("Liability accounts");
        displayOrder = 2;
        break;
      case Account::Type::Income:
        // Income categories
        account = d->m_file->income();
        accountName = i18n("Income categories");
        displayOrder = 3;
        break;
      case Account::Type::Expense:
        // Expense categories
        account = d->m_file->expense();
        accountName = i18n("Expense categories");
        displayOrder = 4;
        break;
      case Account::Type::Equity:
        // Equity accounts
        account = d->m_file->equity();
        accountName = i18n("Equity accounts");
        displayOrder = 5;
        break;
      default:
        continue;
    }

    auto accountsItem = new QStandardItem(accountName);
    accountsItem->setEditable(false);
    rootItem->appendRow(accountsItem);

    {
      QMap<int, QVariant> itemData;
      itemData[Qt::DisplayRole] = accountName;
      itemData[(int)Role::FullName] = itemData[Qt::EditRole] = QVariant::fromValue(MyMoneyFile::instance()->accountToCategory(account.id(), true));
      itemData[Qt::FontRole] = font;
      itemData[(int)Role::DisplayOrder] = displayOrder;
      this->setItemData(accountsItem->index(), itemData);
    }

    // adding accounts (specific bank/investment accounts) belonging to given accounts category
    foreach (const auto accStr, account.accountList()) {
      const auto acc = d->m_file->account(accStr);

      auto item = new QStandardItem(acc.name());
      accountsItem->appendRow(item);
      item->setEditable(false);
      auto subaccountsStr = acc.accountList();
      // filter out stocks with zero balance if requested by user
      for (auto subaccStr = subaccountsStr.begin(); subaccStr != subaccountsStr.end();) {
        const auto subacc = d->m_file->account(*subaccStr);
        if (subacc.isInvest() && KMyMoneyGlobalSettings::hideZeroBalanceEquities() && subacc.balance().isZero())
          subaccStr = subaccountsStr.erase(subaccStr);
        else
          ++subaccStr;
      }

      // adding subaccounts (e.g. stocks under given investment account) belonging to given account
      d->loadSubaccounts(item, favoriteAccountsItem, subaccountsStr);
      const auto row = item->row();
      d->setAccountData(accountsItem, row, acc, d->m_columns);
      d->loadPreferredAccount(acc, accountsItem, row, favoriteAccountsItem);
    }

    d->setAccountData(rootItem, accountsItem->row(), account, d->m_columns);
  }

  checkNetWorth();
  checkProfit();
  this->blockSignals(false);
}

QModelIndex AccountsModel::accountById(const QString& id) const
{
  QModelIndexList accountList = match(index(0, 0),
                                    (int)Role::ID,
                                    id,
                                    1,
                                    Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

  if(accountList.count() == 1) {
    return accountList.first();
  }
  return QModelIndex();
}

QList<Column> *AccountsModel::getColumns()
{
  return &d->m_columns;
}

void AccountsModel::setColumnVisibility(const Column column, const bool show)
{
  const auto ixCol = d->m_columns.indexOf(column);  // get column index in our column's map
  if (!show && ixCol != -1) {                       // start removing column row by row from bottom to up
    d->m_columns.removeOne(column);                 // remove it from our column's map
    blockSignals(true);                             // block signals to not emit resources consuming dataChanged
    for (auto i = 0; i < rowCount(); ++i) {
      // recursive lambda function to remove cell belonging to unwanted column from all rows
      auto removeCellFromRow = [=](auto &&self, QStandardItem *item) -> bool {
        for(auto j = 0; j < item->rowCount(); ++j) {
          auto childItem = item->child(j);
          if (childItem->hasChildren())
            self(self, childItem);
          childItem->removeColumn(ixCol);
        }
        return true;
      };

      auto topItem = item(i);
      if (topItem->hasChildren())
        removeCellFromRow(removeCellFromRow, topItem);
      topItem->removeColumn(ixCol);
    }
    blockSignals(false);                           // unblock signals, so model can update itself with new column
    removeColumn(ixCol);                           // remove column from invisible root item which triggers model's view update
  } else if (show && ixCol == -1) {                // start inserting columns row by row  from up to bottom (otherwise columns will be inserted automatically)
    auto model = qobject_cast<InstitutionsModel *>(this);
    const auto isInstitutionsModel = model ? true : false;  // if it's institution's model, then don't set any data on institution nodes

    auto newColPos = 0;
    for(; newColPos < d->m_columns.count(); ++newColPos) {
      if (d->m_columns.at(newColPos) > column)
        break;
    }
    d->m_columns.insert(newColPos, column);       // insert columns according to enum order for cleanliness

    insertColumn(newColPos);
    setHorizontalHeaderItem(newColPos, new QStandardItem(getHeaderName(column)));
    blockSignals(true);
    for (auto i = 0; i < rowCount(); ++i) {
      // recursive lambda function to remove cell belonging to unwanted column from all rows
      auto addCellToRow = [&, newColPos](auto &&self, QStandardItem *item) -> bool {
        for(auto j = 0; j < item->rowCount(); ++j) {
          auto childItem = item->child(j);
          childItem->insertColumns(newColPos, 1);
          if (childItem->hasChildren())
            self(self, childItem);
          this->d->setAccountData(item, j, childItem->data((int)Role::Account).value<MyMoneyAccount>(), QList<Column> {column});
        }
        return true;
      };

      auto topItem = item(i);
      topItem->insertColumns(newColPos, 1);
      if (topItem->hasChildren())
        addCellToRow(addCellToRow, topItem);

      if (isInstitutionsModel)
        d->setInstitutionTotalValue(invisibleRootItem(), i);
      else if (i !=0)  // favourites node doesn't play well here, so exclude it from update
        d->setAccountData(invisibleRootItem(), i, topItem->data((int)Role::Account).value<MyMoneyAccount>(), QList<Column> {column});
    }
    blockSignals(false);
  }
}

QString AccountsModel::getHeaderName(const Column column)
{
  switch(column) {
    case Column::Account:
      return i18n("Account");
    case Column::Type:
      return i18n("Type");
    case Column::Tax:
      return i18nc("Column heading for category in tax report", "Tax");
    case Column::VAT:
      return i18nc("Column heading for VAT category", "VAT");
    case Column::CostCenter:
      return i18nc("Column heading for Cost Center", "CC");
    case Column::TotalBalance:
      return i18n("Total Balance");
    case Column::PostedValue:
      return i18n("Posted Value");
    case Column::TotalValue:
      return i18n("Total Value");
    case Column::AccountNumber:
      return i18n("Number");
    case Column::AccountSortCode:
      return i18nc("IBAN, SWIFT, etc.", "Sort Code");
    default:
      return QString();
  }
}

/**
  * Check if netWorthChanged should be emitted.
  */
void AccountsModel::checkNetWorth()
{
  // compute the net woth
  QModelIndexList assetList = match(index(0, 0),
                                    (int)Role::ID,
                                    MyMoneyFile::instance()->asset().id(),
                                    1,
                                    Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));

  QModelIndexList liabilityList = match(index(0, 0),
                                        (int)Role::ID,
                                        MyMoneyFile::instance()->liability().id(),
                                        1,
                                        Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));

  MyMoneyMoney netWorth;
  if (!assetList.isEmpty() && !liabilityList.isEmpty()) {
    const auto  assetValue = data(assetList.front(), (int)Role::TotalValue);
    const auto  liabilityValue = data(liabilityList.front(), (int)Role::TotalValue);

    if (assetValue.isValid() && liabilityValue.isValid())
      netWorth = assetValue.value<MyMoneyMoney>() - liabilityValue.value<MyMoneyMoney>();
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
  const auto incomeList = match(index(0, 0),
                                (int)Role::ID,
                                MyMoneyFile::instance()->income().id(),
                                1,
                                Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));

  const auto expenseList = match(index(0, 0),
                                 (int)Role::ID,
                                 MyMoneyFile::instance()->expense().id(),
                                 1,
                                 Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));

  MyMoneyMoney profit;
  if (!incomeList.isEmpty() && !expenseList.isEmpty()) {
    const auto incomeValue = data(incomeList.front(), (int)Role::TotalValue);
    const auto expenseValue = data(expenseList.front(), (int)Role::TotalValue);

    if (incomeValue.isValid() && expenseValue.isValid())
      profit = incomeValue.value<MyMoneyMoney>() - expenseValue.value<MyMoneyMoney>();
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
      const auto list = match(index(0, 0), (int)Role::ID, QVariant(d->m_reconciledAccount.id()), -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
      foreach (const auto index, list)
        setData(index, QVariant(QIcon(account.accountPixmap(false))), Qt::DecorationRole);
    }

    // then set the reconciliation flag of the new reconciliation account
    const auto list = match(index(0, 0), (int)Role::ID, QVariant(account.id()), -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
    foreach (const auto index, list)
      setData(index, QVariant(QIcon(account.accountPixmap(true))), Qt::DecorationRole);
    d->m_reconciledAccount = account;
  }
}

/**
  * Notify the model that an object has been added. An action is performed only if the object is an account.
  *
  */
void AccountsModel::slotObjectAdded(File::Object objType, const MyMoneyObject * const obj)
{
  if (objType != File::Object::Account)
    return;

  const MyMoneyAccount * const account = dynamic_cast<const MyMoneyAccount * const>(obj);
  if (!account)
    return;

  auto favoriteAccountsItem = d->itemFromAccountId(this, favoritesAccountId);
  auto parentAccountItem = d->itemFromAccountId(this, account->parentAccountId());
  auto item = d->itemFromAccountId(parentAccountItem, account->id());
  if (!item) {
    item = new QStandardItem(account->name());
    parentAccountItem->appendRow(item);
    item->setEditable(false);
  }
  // load the sub-accounts if there are any - there could be sub accounts if this is an add operation
  // that was triggered in slotObjectModified on an already existing account which went trough a hierarchy change
  d->loadSubaccounts(item, favoriteAccountsItem, account->accountList());

  const auto row = item->row();
  d->setAccountData(parentAccountItem, row, *account, d->m_columns);
  d->loadPreferredAccount(*account, parentAccountItem, row, favoriteAccountsItem);

  checkNetWorth();
  checkProfit();
}

/**
  * Notify the model that an object has been modified. An action is performed only if the object is an account.
  *
  */
void AccountsModel::slotObjectModified(File::Object objType, const MyMoneyObject * const obj)
{
  if (objType != File::Object::Account)
    return;

  const MyMoneyAccount * const account = dynamic_cast<const MyMoneyAccount * const>(obj);
  if (!account)
    return;
  auto favoriteAccountsItem = d->itemFromAccountId(this, favoritesAccountId);
  auto accountItem = d->itemFromAccountId(this, account->id());
  const auto oldAccount = accountItem->data((int)Role::Account).value<MyMoneyAccount>();
  if (oldAccount.parentAccountId() == account->parentAccountId()) {
    // the hierarchy did not change so update the account data
    auto parentAccountItem = accountItem->parent();
    if (!parentAccountItem)
      parentAccountItem = this->invisibleRootItem();
    const auto row = accountItem->row();
    d->setAccountData(parentAccountItem, row, *account, d->m_columns);
    // and the child of the favorite item if the account is a favorite account or it's favorite status has just changed
    auto favItem = d->itemFromAccountId(favoriteAccountsItem, account->id());
    if (account->value("PreferredAccount") == QLatin1String("Yes"))
      d->loadPreferredAccount(*account, parentAccountItem, row, favoriteAccountsItem);
    else if (favItem)
      favoriteAccountsItem->removeRow(favItem->row()); // it's not favorite anymore
  } else {
    // this means that the hierarchy was changed - simulate this with a remove followed by and add operation
    slotObjectRemoved(File::Object::Account, oldAccount.id());
    slotObjectAdded(File::Object::Account, obj);
  }

  checkNetWorth();
  checkProfit();
}

/**
  * Notify the model that an object has been removed. An action is performed only if the object is an account.
  *
  */
void AccountsModel::slotObjectRemoved(File::Object objType, const QString& id)
{
  if (objType != File::Object::Account)
    return;

  auto list = match(index(0, 0), (int)Role::ID, id, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
  foreach (const auto index, list)
    removeRow(index.row(), index.parent());

  checkNetWorth();
  checkProfit();
}

/**
  * Notify the model that the account balance has been changed.
  */
void AccountsModel::slotBalanceOrValueChanged(const MyMoneyAccount &account)
{
  auto itParent = d->itemFromAccountId(this, account.id()); // get node of account in model
  auto isTopLevel = false;                                  // it could be top-level but we don't know it yet
  while (itParent && !isTopLevel) {                         // loop in which we set total values and balances from the bottom to the top
    auto itCurrent = itParent;
    const auto accCurrent = d->m_file->account(itCurrent->data((int)Role::Account).value<MyMoneyAccount>().id());
    if (accCurrent.id().isEmpty()) {   // this is institution
      d->setInstitutionTotalValue(invisibleRootItem(), itCurrent->row());
      break;                            // it's top-level node so nothing above that;
    }
    itParent = itCurrent->parent();
    if (!itParent) {
      itParent = this->invisibleRootItem();
      isTopLevel = true;
    }
    d->setAccountBalanceAndValue(itParent, itCurrent->row(), accCurrent, d->m_columns);
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
    const auto list = model->match(model->index(0, 0), (int)Role::ID, QVariant(institutionId), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));
    if (!list.isEmpty())
      return model->itemFromIndex(list.front());
    return nullptr; // this should rarely fail as we add all institutions early on
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

    // we've got account but don't know under which institution it should be added, so we find it out
    auto idInstitution = account.institutionId();
    if (account.isInvest()) {                                                 // if it's stock account then...
      const auto investmentAccount = m_file->account(account.parentAccountId());  // ...get investment account it's under and...
      idInstitution = investmentAccount.institutionId();                          // ...get institution from investment account
    }
    auto itInstitution = institutionItemFromId(model, idInstitution);
    auto itAccount = itemFromAccountId(itInstitution, account.id());  // check if account already exists under institution
    // only stock accounts are added to their parent in the institutions view
    // this makes hierarchy maintenance a lot easier since the stock accounts
    // are the only ones that always have the same institution as their parent
    auto itInvestmentAccount = account.isInvest() ? itemFromAccountId(itInstitution, account.parentAccountId()) : nullptr;
    if (!itAccount) {
      itAccount = new QStandardItem(account.name());
      if (itInvestmentAccount)                       // stock account nodes go under investment account nodes and...
        itInvestmentAccount->appendRow(itAccount);
      else if (itInstitution)                       // ...the rest goes under institution's node
        itInstitution->appendRow(itAccount);
      else
        return;
      itAccount->setEditable(false);
    }
    if (itInvestmentAccount) {
      setAccountData(itInvestmentAccount, itAccount->row(), account, m_columns);                                         // set data for stock account node
      setAccountData(itInstitution, itInvestmentAccount->row(), m_file->account(account.parentAccountId()), m_columns);  // set data for investment account node
    } else if (itInstitution) {
      setAccountData(itInstitution, itAccount->row(), account, m_columns);
    }
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
    auto itInstitution = new QStandardItem(QIcon::fromTheme(g_Icons.value(Icon::ViewInstitutions)), institution.name());
    itInstitution->setFont(font);
    itInstitution->setData(QVariant::fromValue(MyMoneyMoney()), (int)Role::TotalValue);
    itInstitution->setData(institution.id(), (int)Role::ID);
    itInstitution->setData(QVariant::fromValue(institution), (int)Role::Account);
    itInstitution->setData(6, (int)Role::DisplayOrder);
    itInstitution->setEditable(false);
    model->invisibleRootItem()->appendRow(itInstitution);
    setInstitutionTotalValue(model->invisibleRootItem(), itInstitution->row());
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
  * Perform the initial load of the model data
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
  auto modelUtils = static_cast<InstitutionsPrivate *>(d);
  foreach (const auto institution, institutionList)   // add all known institutions as top-level nodes
    modelUtils->addInstitutionItem(this, institution);

  QList<MyMoneyAccount> accountsList;
  QList<MyMoneyAccount> stocksList;
  d->m_file->accountList(accountsList);
  foreach (const auto account, accountsList) {  // add account nodes under institution nodes...
    if (account.isInvest())                     // ...but wait with stocks until investment accounts appear
      stocksList.append(account);
    else
      modelUtils->loadInstitution(this, account);
  }

  foreach (const auto stock, stocksList) {
    if (!(KMyMoneyGlobalSettings::hideZeroBalanceEquities() && stock.balance().isZero()))
      modelUtils->loadInstitution(this, stock);
  }

  for (auto i = 0 ; i < rowCount(); ++i)
    d->setInstitutionTotalValue(invisibleRootItem(), i);
}

/**
  * Notify the model that an object has been added. An action is performed only if the object is an account or an institution.
  *
  */
void InstitutionsModel::slotObjectAdded(File::Object objType, const MyMoneyObject * const obj)
{
  auto modelUtils = static_cast<InstitutionsPrivate *>(d);
  if (objType == File::Object::Institution) {
    // if an institution was added then add the item which will represent it
    const MyMoneyInstitution * const institution = dynamic_cast<const MyMoneyInstitution * const>(obj);
    if (!institution)
      return;
    modelUtils->addInstitutionItem(this, *institution);
  }

  if (objType != File::Object::Account)
    return;

  // if an account was added then add the item which will represent it only for real accounts
  const MyMoneyAccount * const account = dynamic_cast<const MyMoneyAccount * const>(obj);
  // nothing to do for root accounts and categories
  if (!account || account->parentAccountId().isEmpty() || account->isIncomeExpense())
    return;

  // load the account into the institution
  modelUtils->loadInstitution(this, *account);

  // load the investment sub-accounts if there are any - there could be sub-accounts if this is an add operation
  // that was triggered in slotObjectModified on an already existing account which went trough a hierarchy change
  const auto sAccounts = account->accountList();
  if (!sAccounts.isEmpty()) {
    QList<MyMoneyAccount> subAccounts;
    d->m_file->accountList(subAccounts, sAccounts);
    foreach (const auto subAccount, subAccounts) {
      if (subAccount.isInvest()) {
        modelUtils->loadInstitution(this, subAccount);
      }
    }
  }
}

/**
  * Notify the model that an object has been modified. An action is performed only if the object is an account or an institution.
  *
  */
void InstitutionsModel::slotObjectModified(File::Object objType, const MyMoneyObject * const obj)
{
  if (objType == File::Object::Institution) {
    // if an institution was modified then modify the item which represents it
    const MyMoneyInstitution * const institution = dynamic_cast<const MyMoneyInstitution * const>(obj);
    if (!institution)
      return;
    auto institutionItem = static_cast<InstitutionsPrivate *>(d)->institutionItemFromId(this, institution->id());
    institutionItem->setData(institution->name(), Qt::DisplayRole);
    institutionItem->setData(QVariant::fromValue(*institution), (int)Role::Account);
    institutionItem->setIcon(institution->pixmap());
  }

  if (objType != File::Object::Account)
    return;

  // if an account was modified then modify the item which represents it
  const MyMoneyAccount * const account = dynamic_cast<const MyMoneyAccount * const>(obj);
  // nothing to do for root accounts, categories and equity accounts since they don't have a representation in this model
  if (!account || account->parentAccountId().isEmpty() || account->isIncomeExpense() || account->accountType() == Account::Type::Equity)
    return;

  auto accountItem = d->itemFromAccountId(this, account->id());
  const auto oldAccount = accountItem->data((int)Role::Account).value<MyMoneyAccount>();
  if (oldAccount.institutionId() == account->institutionId()) {
    // the hierarchy did not change so update the account data
    d->setAccountData(accountItem->parent(), accountItem->row(), *account, d->m_columns);
  } else {
    // this means that the hierarchy was changed - simulate this with a remove followed by and add operation
    slotObjectRemoved(File::Object::Account, oldAccount.id());
    slotObjectAdded(File::Object::Account, obj);
  }
}

/**
  * Notify the model that an object has been removed. An action is performed only if the object is an account or an institution.
  *
  */
void InstitutionsModel::slotObjectRemoved(File::Object objType, const QString& id)
{
  if (objType == File::Object::Institution) {
    // if an institution was removed then remove the item which represents it
    auto itInstitution = static_cast<InstitutionsPrivate *>(d)->institutionItemFromId(this, id);
    if (itInstitution)
      removeRow(itInstitution->row(), itInstitution->index().parent());
  }

  if (objType != File::Object::Account)
    return;

  // if an account was removed then remove the item which represents it and recompute the institution's value
  auto itAccount = d->itemFromAccountId(this, id);
  if (!itAccount)
    return; // this could happen if the account isIncomeExpense

  const auto account = itAccount->data((int)Role::Account).value<MyMoneyAccount>();
  auto itInstitution = d->itemFromAccountId(this, account.institutionId());

  AccountsModel::slotObjectRemoved(objType, id);
  d->setInstitutionTotalValue(invisibleRootItem(), itInstitution->row());
}
