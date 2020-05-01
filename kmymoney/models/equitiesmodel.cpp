/*
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "equitiesmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMenu>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "mymoneyenums.h"

class EquitiesModel::Private
{
public:
  Private() : m_file(MyMoneyFile::instance())
  {
    QVector<Column> columns {Column::Equity, Column::Symbol, Column::Value,
                            Column::Quantity, Column::Price};
    foreach (auto const column, columns)
      m_columns.append(column);
  }

  ~Private() {}

  void loadInvestmentAccount(QStandardItem *node, const MyMoneyAccount &invAcc)
  {
    auto itInvAcc = new QStandardItem(invAcc.name());
    node->appendRow(itInvAcc);                                  // investment account is meant to be added under root item
    itInvAcc->setEditable(false);
    itInvAcc->setColumnCount(m_columns.count());
    setAccountData(node, itInvAcc->row(), invAcc, m_columns);

    foreach (const auto strStkAcc, invAcc.accountList()) { // only stock or bond accounts are expected here
      auto stkAcc = m_file->account(strStkAcc);
      auto itStkAcc = new QStandardItem(strStkAcc);
      itStkAcc->setEditable(false);
      itInvAcc->appendRow(itStkAcc);
      setAccountData(itInvAcc, itStkAcc->row(), stkAcc, m_columns);
    }
  }

  void setAccountData(QStandardItem *node, const int row, const MyMoneyAccount &account, const QList<Column> &columns)
  {
    QStandardItem *cell;

    auto getCell = [&, row](const auto column) {
      cell = node->child(row, column);      // try to get QStandardItem
      if (!cell) {                          // it may be uninitialized
        cell = new QStandardItem;           // so create one
        node->setChild(row, column, cell);  // and add it under the node
        cell->setEditable(false);           // and don't forget that it's non-editable
      }
    };

    auto colNum = m_columns.indexOf(Column::Equity);
    if (colNum == -1)
      return;

    // Equity
    getCell(colNum);
    if (columns.contains(Column::Equity)) {
      cell->setData(account.name(), Qt::DisplayRole);
      cell->setData(account.id(), Role::EquityID);
      cell->setData(account.currencyId(), Role::SecurityID);
    }

    if (account.accountType() == eMyMoney::Account::Type::Investment)  // investments accounts are not meant to be displayed, so stop here
      return;

    // Display the name of the equity with strikeout font in case it is closed
    auto font = cell->data(Qt::FontRole).value<QFont>();
    if (account.isClosed() != font.strikeOut()) {
      font.setStrikeOut(account.isClosed());
      cell->setData(font, Qt::FontRole);
    }

    // Symbol
    if (columns.contains(Column::Symbol)) {
      colNum = m_columns.indexOf(Column::Symbol);
      if (colNum != -1) {
        auto security = m_file->security(account.currencyId());
        getCell(colNum);
        cell->setData(security.tradingSymbol(), Qt::DisplayRole);
      }
    }

    setAccountBalanceAndValue(node, row, account, columns);
  }

  void setAccountBalanceAndValue(QStandardItem *node, const int row, const MyMoneyAccount &account, const QList<Column> &columns)
  {
    QStandardItem *cell;

    auto getCell = [&, row](const auto column) {
      cell = node->child(row, column);      // try to get QStandardItem
      if (!cell) {                          // it may be uninitialized
        cell = new QStandardItem;           // so create one
        node->setChild(row, column, cell);  // and add it under the node
        cell->setEditable(false);           // and don't forget that it's non-editable
      }
    };

    auto colNum = m_columns.indexOf(Column::Equity);
    if (colNum == -1)
      return;

    auto balance = m_file->balance(account.id());
    auto security = m_file->security(account.currencyId());
    auto tradingCurrency = m_file->security(security.tradingCurrency());
    auto price = m_file->price(account.currencyId(), tradingCurrency.id());

    // Value
    if (columns.contains(Column::Value)) {
      colNum = m_columns.indexOf(Column::Value);
      if (colNum != -1) {
        getCell(colNum);
        if (price.isValid()) {
          auto prec = MyMoneyMoney::denomToPrec(tradingCurrency.smallestAccountFraction());
          auto value = balance * price.rate(tradingCurrency.id());
          auto strValue = QVariant(value.formatMoney(tradingCurrency.tradingSymbol(), prec));
          cell->setData(strValue, Qt::DisplayRole);
        } else {
          cell->setData(QVariant("---"), Qt::DisplayRole);
        }
        cell->setData(QVariant(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
      }
    }

    // Quantity
    if (columns.contains(Column::Quantity)) {
      colNum = m_columns.indexOf(Column::Quantity);
      if (colNum != -1) {
        getCell(colNum);
        auto prec = MyMoneyMoney::denomToPrec(security.smallestAccountFraction());
        auto strQuantity = QVariant(balance.formatMoney(QString(), prec));
        cell->setData(strQuantity, Qt::DisplayRole);
        cell->setData(QVariant(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
      }
    }

    // Price
    if (columns.contains(Column::Price)) {
      colNum = m_columns.indexOf(Column::Price);
      if (colNum != -1) {
        getCell(colNum);
        if (price.isValid()) {
          auto prec = security.pricePrecision();
          auto strPrice = QVariant(price.rate(tradingCurrency.id()).formatMoney(tradingCurrency.tradingSymbol(), prec));
          cell->setData(strPrice, Qt::DisplayRole);
        } else {
          cell->setData(QVariant("---"), Qt::DisplayRole);
        }
        cell->setData(QVariant(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
      }
    }
  }

  QStandardItem *itemFromId(QStandardItemModel *model, const QString &id, const Role role)
  {
    const auto itemList = model->match(model->index(0, 0), role, QVariant(id), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
    if (!itemList.isEmpty())
      return model->itemFromIndex(itemList.first());
    return nullptr;
  }

  MyMoneyFile *m_file;
  QList<EquitiesModel::Column> m_columns;
};

EquitiesModel::EquitiesModel(QObject *parent)
    : QStandardItemModel(parent), d(new Private)
{
  init();
}

EquitiesModel::~EquitiesModel()
{
  delete d;
}

void EquitiesModel::init()
{
  QStringList headerLabels;
  foreach (const auto column, d->m_columns)
    headerLabels.append(getHeaderName(column));
  setHorizontalHeaderLabels(headerLabels);
}

void EquitiesModel::load()
{
  this->blockSignals(true);

  auto rootItem = invisibleRootItem();
  QList<MyMoneyAccount> accList;
  d->m_file->accountList(accList);                        // get all available accounts
  foreach (const auto acc, accList)
    if (acc.accountType() == eMyMoney::Account::Type::Investment)  // but add only investment accounts (and its children) to the model
        d->loadInvestmentAccount(rootItem, acc);

  this->blockSignals(false);
}

/**
  * Notify the model that an object has been added. An action is performed only if the object is an account.
  */
void EquitiesModel::slotObjectAdded(eMyMoney::File::Object objType, const QString& id)
{
  // check whether change is about accounts
  if (objType != eMyMoney::File::Object::Account)
    return;

  // check whether change is about either investment or stock account
  const auto acc = MyMoneyFile::instance()->account(id);
  if (acc.accountType() != eMyMoney::Account::Type::Investment &&
      acc.accountType() != eMyMoney::Account::Type::Stock)
    return;
  auto itAcc = d->itemFromId(this, id, Role::EquityID);

  QStandardItem *itParentAcc;
  if (acc.accountType() == eMyMoney::Account::Type::Investment) // if it's investment account then its parent is root item
    itParentAcc = invisibleRootItem();
  else                                                  // otherwise it's stock account and its parent is investment account
    itParentAcc = d->itemFromId(this, acc.parentAccountId(), Role::InvestmentID);

  // if account doesn't exist in model then add it
  if (!itAcc) {
    itAcc = new QStandardItem(acc.name());
    itParentAcc->appendRow(itAcc);
    itAcc->setEditable(false);
  }

  d->setAccountData(itParentAcc, itAcc->row(), acc, d->m_columns);
}

/**
  * Notify the model that an object has been modified. An action is performed only if the object is an account.
  */
void EquitiesModel::slotObjectModified(eMyMoney::File::Object objType, const QString& id)
{
  MyMoneyAccount acc;
  QStandardItem  *itAcc;
  switch (objType) {
    case eMyMoney::File::Object::Account:
      {
        auto tmpAcc = MyMoneyFile::instance()->account(id);
        if (tmpAcc.accountType() != eMyMoney::Account::Type::Stock)
          return;
        acc = MyMoneyAccount(tmpAcc);
        itAcc = d->itemFromId(this, acc.id(), Role::EquityID);
        break;
      }
    case eMyMoney::File::Object::Security:
      {
        auto sec = MyMoneyFile::instance()->security(id);
        // in case we hit a currency, we simply bail out here
        // as there is nothing to do for us
        if(sec.isCurrency())
          return;
        itAcc = d->itemFromId(this, sec.id(), Role::SecurityID);
        if (!itAcc)
          return;
        const auto idAcc = itAcc->data(Role::EquityID).toString();
        acc = d->m_file->account(idAcc);
        break;
      }
    default:
      return;
  }

  auto itParentAcc = d->itemFromId(this, acc.parentAccountId(), Role::InvestmentID);
  // in case something went wrong, we bail out
  if(itParentAcc == nullptr) {
    qWarning() << "EquitiesModel::slotObjectModified: itParentAcc == 0";
    return;
  }

  auto modelID = itParentAcc->data(Role::InvestmentID).toString();      // get parent account from model
  if (modelID == acc.parentAccountId()) {                              // and if it matches with those from file then modify only
    d->setAccountData(itParentAcc, itAcc->row(), acc, d->m_columns);
  } else {                                                              // and if not then reparent
    slotObjectRemoved(eMyMoney::File::Object::Account, acc.id());
    slotObjectAdded(eMyMoney::File::Object::Account, id);
  }
}

/**
  * Notify the model that an object has been removed. An action is performed only if the object is an account.
  *
  */
void EquitiesModel::slotObjectRemoved(eMyMoney::File::Object objType, const QString& id)
{
  if (objType != eMyMoney::File::Object::Account)
    return;

  const auto indexList = match(index(0, 0), Role::EquityID, id, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));
  foreach (const auto index, indexList)
    removeRow(index.row(), index.parent());
}

/**
  * Notify the model that the account balance has been changed.
  */
void EquitiesModel::slotBalanceOrValueChanged(const MyMoneyAccount &account)
{
  if (account.accountType() != eMyMoney::Account::Type::Stock)
    return;

  const auto itAcc = d->itemFromId(this, account.id(), Role::EquityID);
  if (!itAcc)
    return;
  d->setAccountBalanceAndValue(itAcc->parent(), itAcc->row(), account, d->m_columns);
}

auto EquitiesModel::getColumns()
{
  return &d->m_columns;
}

QString EquitiesModel::getHeaderName(const Column column)
{
  switch(column) {
    case Equity:
      return i18n("Equity");
    case Symbol:
      return i18nc("@title stock symbol column", "Symbol");
    case Value:
      return i18n("Value");
    case Quantity:
      return i18n("Quantity");
    case Price:
      return i18n("Price");
    default:
      return QString();
  }
}

class EquitiesFilterProxyModel::Private
{
public:
  Private() :
    m_mdlColumns(nullptr),
    m_file(MyMoneyFile::instance()),
    m_hideClosedAccounts(false),
    m_hideZeroBalanceAccounts(false)
     {}

  ~Private() {}

  QList<EquitiesModel::Column> *m_mdlColumns;
  QList<EquitiesModel::Column> m_visColumns;

  MyMoneyFile *m_file;

  bool m_hideClosedAccounts;
  bool m_hideZeroBalanceAccounts;
};

EquitiesFilterProxyModel::EquitiesFilterProxyModel(QObject *parent, EquitiesModel *model, const QList<EquitiesModel::Column> &columns)
  : KRecursiveFilterProxyModel(parent), d(new Private)
{
  setDynamicSortFilter(true);
  setFilterKeyColumn(-1);
  setSortLocaleAware(true);
  setFilterCaseSensitivity(Qt::CaseInsensitive);
  setSourceModel(model);
  d->m_mdlColumns = model->getColumns();
  d->m_visColumns.append(columns);
}

EquitiesFilterProxyModel::~EquitiesFilterProxyModel()
{
  delete d;
}

/**
  * Set if closed accounts should be hidden or not.
  * @param hideClosedAccounts
  */
void EquitiesFilterProxyModel::setHideClosedAccounts(const bool hideClosedAccounts)
{
  d->m_hideClosedAccounts = hideClosedAccounts;
}

/**
  * Set if zero balance accounts should be hidden or not.
  * @param hideZeroBalanceAccounts
  */
void EquitiesFilterProxyModel::setHideZeroBalanceAccounts(const bool hideZeroBalanceAccounts)
{
  d->m_hideZeroBalanceAccounts = hideZeroBalanceAccounts;
}

bool EquitiesFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
  Q_UNUSED(source_parent)
  if (d->m_visColumns.isEmpty() || d->m_visColumns.contains(d->m_mdlColumns->at(source_column)))
      return true;
  return false;
}

bool EquitiesFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  if (d->m_hideClosedAccounts || d->m_hideZeroBalanceAccounts) {
    const auto ixRow = sourceModel()->index(source_row, EquitiesModel::Equity, source_parent);
    const auto idAcc = sourceModel()->data(ixRow, EquitiesModel::EquityID).toString();
    const auto acc = d->m_file->account(idAcc);

    if (d->m_hideClosedAccounts &&
        acc.isClosed())
      return false;
    if (d->m_hideZeroBalanceAccounts &&
        acc.accountType() != eMyMoney::Account::Type::Investment && acc.balance().isZero())  // we should never hide investment account because all underlaying stocks will be hidden as well
      return false;
  }
  return true;
}

QList<EquitiesModel::Column> &EquitiesFilterProxyModel::getVisibleColumns()
{
  return d->m_visColumns;
}

void EquitiesFilterProxyModel::slotColumnsMenu(const QPoint)
{
  // construct all hideable columns list
  const QList<EquitiesModel::Column> idColumns {
    EquitiesModel::Symbol, EquitiesModel::Value,
    EquitiesModel::Quantity, EquitiesModel::Price
  };

  // create menu
  QMenu menu(i18n("Displayed columns"));
  QList<QAction *> actions;
  foreach (const auto idColumn, idColumns) {
    auto a = new QAction(nullptr);
    a->setObjectName(QString::number(idColumn));
    a->setText(EquitiesModel::getHeaderName(idColumn));
    a->setCheckable(true);
    a->setChecked(d->m_visColumns.contains(idColumn));
    actions.append(a);
  }
  menu.addActions(actions);

  // execute menu and get result
  const auto retAction = menu.exec(QCursor::pos());
  if (retAction) {
    const auto idColumn = static_cast<EquitiesModel::Column>(retAction->objectName().toInt());
    const auto isChecked = retAction->isChecked();
    const auto contains = d->m_visColumns.contains(idColumn);
    if (isChecked && !contains) {           // column has just been enabled
      d->m_visColumns.append(idColumn);     // change filtering variable
      emit columnToggled(idColumn, true);   // emit signal for method to add column to model
      invalidate();                         // refresh model to reflect recent changes
    } else if (!isChecked && contains) {    // column has just been disabled
      d->m_visColumns.removeOne(idColumn);
      emit columnToggled(idColumn, false);
      invalidate();
    }
  }
}
