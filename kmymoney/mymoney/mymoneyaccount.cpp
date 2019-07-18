/*
 * Copyright 2000-2002  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2001       Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2002-2003  Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2006-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2006       Ace Jones <acejones@users.sourceforge.net>
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

#include "mymoneyaccount.h"
#include "mymoneyaccount_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>
#include <QPixmap>
#include <QPixmapCache>
#include <QPainter>
#include <QIcon>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneyexception.h"
#include "mymoneysplit.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyinstitution.h"
#include "mymoneypayee.h"
#include "payeeidentifier/payeeidentifiertyped.h"
#include "payeeidentifier/ibanbic/ibanbic.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "icons/icons.h"

using namespace Icons;

MyMoneyAccount::MyMoneyAccount() :
  MyMoneyObject(*new MyMoneyAccountPrivate),
  MyMoneyKeyValueContainer()
{
}

MyMoneyAccount::MyMoneyAccount(const QString &id):
  MyMoneyObject(*new MyMoneyAccountPrivate, id),
  MyMoneyKeyValueContainer()
{
}

MyMoneyAccount::MyMoneyAccount(const MyMoneyAccount& other) :
  MyMoneyObject(*new MyMoneyAccountPrivate(*other.d_func()), other.id()),
  MyMoneyKeyValueContainer(other)
{
}

MyMoneyAccount::MyMoneyAccount(const QString& id, const MyMoneyAccount& other) :
  MyMoneyObject(*new MyMoneyAccountPrivate(*other.d_func()), id),
  MyMoneyKeyValueContainer(other)
{
}

MyMoneyAccount::~MyMoneyAccount()
{
}

void MyMoneyAccount::touch()
{
  setLastModified(QDate::currentDate());
}

eMyMoney::Account::Type MyMoneyAccount::accountType() const
{
  Q_D(const MyMoneyAccount);
  return d->m_accountType;
}

void MyMoneyAccount::setAccountType(const Account::Type type)
{
  Q_D(MyMoneyAccount);
  d->m_accountType = type;
}

QString MyMoneyAccount::institutionId() const
{
  Q_D(const MyMoneyAccount);
  return d->m_institution;
}

void MyMoneyAccount::setInstitutionId(const QString& id)
{
  Q_D(MyMoneyAccount);
  d->m_institution = id;
}

QString MyMoneyAccount::name() const
{
  Q_D(const MyMoneyAccount);
  return d->m_name;
}

void MyMoneyAccount::setName(const QString& name)
{
  Q_D(MyMoneyAccount);
  d->m_name = name;
}

QString MyMoneyAccount::number() const
{
  Q_D(const MyMoneyAccount);
  return d->m_number;
}

void MyMoneyAccount::setNumber(const QString& number)
{
  Q_D(MyMoneyAccount);
  d->m_number = number;
}

QString MyMoneyAccount::description() const
{
  Q_D(const MyMoneyAccount);
  return d->m_description;
}

void MyMoneyAccount::setDescription(const QString& desc)
{
  Q_D(MyMoneyAccount);
  d->m_description = desc;
}

QDate MyMoneyAccount::openingDate() const
{
  Q_D(const MyMoneyAccount);
  return d->m_openingDate;
}

void MyMoneyAccount::setOpeningDate(const QDate& date)
{
  Q_D(MyMoneyAccount);
  d->m_openingDate = date;
}

QDate MyMoneyAccount::lastReconciliationDate() const
{
  Q_D(const MyMoneyAccount);
  return d->m_lastReconciliationDate;
}

void MyMoneyAccount::setLastReconciliationDate(const QDate& date)
{
  Q_D(MyMoneyAccount);
  d->m_lastReconciliationDate = date;
}

QDate MyMoneyAccount::lastModified() const
{
  Q_D(const MyMoneyAccount);
  return d->m_lastModified;
}

void MyMoneyAccount::setLastModified(const QDate& date)
{
  Q_D(MyMoneyAccount);
  d->m_lastModified = date;
}

QString MyMoneyAccount::parentAccountId() const
{
  Q_D(const MyMoneyAccount);
  return d->m_parentAccount;
}

void MyMoneyAccount::setParentAccountId(const QString& parent)
{
  Q_D(MyMoneyAccount);
  d->m_parentAccount = parent;
}

QStringList MyMoneyAccount::accountList() const
{
  Q_D(const MyMoneyAccount);
  return d->m_accountList;
}

int MyMoneyAccount::accountCount() const
{
  Q_D(const MyMoneyAccount);
  return d->m_accountList.count();
}

void MyMoneyAccount::addAccountId(const QString& account)
{
  Q_D(MyMoneyAccount);
  if (!d->m_accountList.contains(account))
    d->m_accountList += account;
}

void MyMoneyAccount::removeAccountIds()
{
  Q_D(MyMoneyAccount);
  d->m_accountList.clear();
}

void MyMoneyAccount::removeAccountId(const QString& account)
{
  Q_D(MyMoneyAccount);
  const auto pos = d->m_accountList.indexOf(account);
  if (pos != -1)
    d->m_accountList.removeAt(pos);
}

bool MyMoneyAccount::operator == (const MyMoneyAccount& right) const
{
  Q_D(const MyMoneyAccount);
  auto d2 = static_cast<const MyMoneyAccountPrivate *>(right.d_func());
  return (MyMoneyKeyValueContainer::operator==(right) &&
          MyMoneyObject::operator==(right) &&
          (d->m_accountList == d2->m_accountList) &&
          (d->m_accountType == d2->m_accountType) &&
          (d->m_lastModified == d2->m_lastModified) &&
          (d->m_lastReconciliationDate == d2->m_lastReconciliationDate) &&
          ((d->m_name.length() == 0 && d2->m_name.length() == 0) || (d->m_name == d2->m_name)) &&
          ((d->m_number.length() == 0 && d2->m_number.length() == 0) || (d->m_number == d2->m_number)) &&
          ((d->m_description.length() == 0 && d2->m_description.length() == 0) || (d->m_description == d2->m_description)) &&
          (d->m_openingDate == d2->m_openingDate) &&
          (d->m_parentAccount == d2->m_parentAccount) &&
          (d->m_currencyId == d2->m_currencyId) &&
          (d->m_institution == d2->m_institution));
}

Account::Type MyMoneyAccount::accountGroup() const
{
  Q_D(const MyMoneyAccount);
  switch (d->m_accountType) {
    case Account::Type::Checkings:
    case Account::Type::Savings:
    case Account::Type::Cash:
    case Account::Type::Currency:
    case Account::Type::Investment:
    case Account::Type::MoneyMarket:
    case Account::Type::CertificateDep:
    case Account::Type::AssetLoan:
    case Account::Type::Stock:
      return Account::Type::Asset;

    case Account::Type::CreditCard:
    case Account::Type::Loan:
      return Account::Type::Liability;

    default:
      return d->m_accountType;
  }
}

QString MyMoneyAccount::currencyId() const
{
  Q_D(const MyMoneyAccount);
  return d->m_currencyId;
}

QString MyMoneyAccount::tradingCurrencyId() const
{
  const auto file = MyMoneyFile::instance();

  // First, get the trading currency (formerly deep currency)
  auto deepcurrency = file->security(currencyId());
  if (!deepcurrency.isCurrency())
    deepcurrency = file->security(deepcurrency.tradingCurrency());

  // Return the trading currency's ID
  return deepcurrency.id();
}

bool MyMoneyAccount::isForeignCurrency() const
{
  return (tradingCurrencyId() != MyMoneyFile::instance()->baseCurrency().id());
}

void MyMoneyAccount::setCurrencyId(const QString& id)
{
  Q_D(MyMoneyAccount);
  d->m_currencyId = id;
}

bool MyMoneyAccount::isAssetLiability() const
{
  return accountGroup() == Account::Type::Asset || accountGroup() == Account::Type::Liability;
}

bool MyMoneyAccount::isIncomeExpense() const
{
  return accountGroup() == Account::Type::Income || accountGroup() == Account::Type::Expense;
}

bool MyMoneyAccount::isLoan() const
{
  return accountType() == Account::Type::Loan || accountType() == Account::Type::AssetLoan;
}

bool MyMoneyAccount::isInvest() const
{
  return accountType() == Account::Type::Stock;
}

bool MyMoneyAccount::isLiquidAsset() const
{
  return accountType() == Account::Type::Checkings ||
         accountType() == Account::Type::Savings ||
         accountType() == Account::Type::Cash;
}

bool MyMoneyAccount::isLiquidLiability() const
{
  return accountType() == Account::Type::CreditCard;
}

bool MyMoneyAccount::isCostCenterRequired() const
{
  return value("CostCenter").toLower() == QLatin1String("yes");
}

void MyMoneyAccount::setCostCenterRequired(bool required)
{
  if(required) {
    setValue("CostCenter", "yes");
  } else {
    deletePair("CostCenter");
  }
}

bool MyMoneyAccount::hasReferenceTo(const QString& id) const
{
  Q_D(const MyMoneyAccount);
  return (id == d->m_institution) || (id == d->m_parentAccount) || (id == d->m_currencyId);
}

void MyMoneyAccount::setOnlineBankingSettings(const MyMoneyKeyValueContainer& values)
{
  Q_D(MyMoneyAccount);
  d->m_onlineBankingSettings = values;
}

MyMoneyKeyValueContainer MyMoneyAccount::onlineBankingSettings() const
{
  Q_D(const MyMoneyAccount);
  return d->m_onlineBankingSettings;
}

void MyMoneyAccount::setClosed(bool closed)
{
  if (closed)
    setValue("mm-closed", "yes");
  else
    deletePair("mm-closed");
}

bool MyMoneyAccount::isClosed() const
{
  return !(value("mm-closed").isEmpty());
}

int MyMoneyAccount::fraction(const MyMoneySecurity& sec) const
{
  Q_D(const MyMoneyAccount);
  int fraction;
  if (d->m_accountType == Account::Type::Cash)
    fraction = sec.smallestCashFraction();
  else
    fraction = sec.smallestAccountFraction();
  return fraction;
}

int MyMoneyAccount::fraction(const MyMoneySecurity& sec)
{
  Q_D(MyMoneyAccount);
  if (d->m_accountType == Account::Type::Cash)
    d->m_fraction = sec.smallestCashFraction();
  else
    d->m_fraction = sec.smallestAccountFraction();
  return d->m_fraction;
}

int MyMoneyAccount::fraction() const
{
  Q_D(const MyMoneyAccount);
  return d->m_fraction;
}

bool MyMoneyAccount::isCategory() const
{
  Q_D(const MyMoneyAccount);
  return d->m_accountType == Account::Type::Income || d->m_accountType == Account::Type::Expense;
}

QString MyMoneyAccount::brokerageName() const
{
  Q_D(const MyMoneyAccount);
  if (d->m_accountType == Account::Type::Investment)
    return QString("%1 (%2)").arg(d->m_name, i18nc("Brokerage (suffix for account names)", "Brokerage"));
  return d->m_name;
}

MyMoneyMoney MyMoneyAccount::balance() const
{
  Q_D(const MyMoneyAccount);
  return d->m_balance;
}

void MyMoneyAccount::adjustBalance(const MyMoneySplit& s, bool reverse)
{
  Q_D(MyMoneyAccount);
  const MyMoneyMoney oldBalance = d->m_balance;

  if (s.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares)) {
    if (reverse)
      d->m_balance = d->m_balance / s.shares();
    else
      d->m_balance = d->m_balance * s.shares();
  } else {
    if (reverse)
      d->m_balance -= s.shares();
    else
      d->m_balance += s.shares();
  }
  // adjust total balance by the difference between the current and previous balance
  d->m_totalBalance += (d->m_balance - oldBalance);
}

void MyMoneyAccount::setBalance(const MyMoneyMoney& val)
{
  Q_D(MyMoneyAccount);
  d->m_balance = val;
}

void MyMoneyAccount::setTotalBalance(const MyMoneyMoney& val)
{
  Q_D(MyMoneyAccount);
  d->m_totalBalance = val;
}

QPixmap MyMoneyAccount::accountPixmap(const bool reconcileFlag, const int size) const
{
  static const QHash<Account::Type, Icon> accToIco {
    {Account::Type::Asset, Icon::Asset},
    {Account::Type::Investment, Icon::Stock},
    {Account::Type::Stock, Icon::Stock},
    {Account::Type::MoneyMarket, Icon::Stock},
    {Account::Type::Checkings, Icon::Checking},
    {Account::Type::Savings, Icon::Savings},
    {Account::Type::AssetLoan, Icon::LoanAsset},
    {Account::Type::Loan, Icon::Loan},
    {Account::Type::CreditCard, Icon::CreditCard},
    {Account::Type::Asset, Icon::Asset},
    {Account::Type::Cash, Icon::Cash},
    {Account::Type::Income, Icon::Income},
    {Account::Type::Expense, Icon::Expense},
    {Account::Type::Equity, Icon::Equity}
  };

  Icon ixIcon = accToIco.value(accountType(), Icon::Liability);

  QString kyIcon = accountTypeToString(accountType()) + QString::number(size);
  QPixmap pxIcon;

  if (!QPixmapCache::find(kyIcon, &pxIcon)) {
    pxIcon = Icons::get(ixIcon).pixmap(size); // Qt::AA_UseHighDpiPixmaps (in Qt 5.7) doesn't return highdpi pixmap
    QPixmapCache::insert(kyIcon, pxIcon);
  }

  if (isClosed())
    ixIcon = Icon::AccountClosed;
  else if (reconcileFlag)
    ixIcon = Icon::Reconciled;
  else if (hasOnlineMapping())
    ixIcon = Icon::Download;
  else
    return pxIcon;

  QPixmap pxOverlay = Icons::get(ixIcon).pixmap(size);
  QPainter pxPainter(&pxIcon);
  const QSize szIcon = pxIcon.size();
  pxPainter.drawPixmap(szIcon.width() / 2, szIcon.height() / 2,
                       szIcon.width() / 2, szIcon.height() / 2, pxOverlay);
  return pxIcon;
}

QString MyMoneyAccount::accountTypeToString(const Account::Type accountType)
{
  switch (accountType) {
    case Account::Type::Checkings:
      return i18nc("Account type", "Checking");
    case Account::Type::Savings:
      return i18nc("Account type", "Savings");
    case Account::Type::CreditCard:
      return i18nc("Account type", "Credit Card");
    case Account::Type::Cash:
      return i18nc("Account type", "Cash");
    case Account::Type::Loan:
      return i18nc("Account type", "Loan");
    case Account::Type::CertificateDep:
      return i18nc("Account type", "Certificate of Deposit");
    case Account::Type::Investment:
      return i18nc("Account type", "Investment");
    case Account::Type::MoneyMarket:
      return i18nc("Account type", "Money Market");
    case Account::Type::Asset:
      return i18nc("Account type", "Asset");
    case Account::Type::Liability:
      return i18nc("Account type", "Liability");
    case Account::Type::Currency:
      return i18nc("Account type", "Currency");
    case Account::Type::Income:
      return i18nc("Account type", "Income");
    case Account::Type::Expense:
      return i18nc("Account type", "Expense");
    case Account::Type::AssetLoan:
      return i18nc("Account type", "Investment Loan");
    case Account::Type::Stock:
      return i18nc("Account type", "Stock");
    case Account::Type::Equity:
      return i18nc("Account type", "Equity");
    default:
      return i18nc("Account type", "Unknown");
  }
}

bool MyMoneyAccount::addReconciliation(const QDate& date, const MyMoneyMoney& amount)
{
  Q_D(MyMoneyAccount);
  d->m_reconciliationHistory[date] = amount;
  QString history, sep;
  QMap<QDate, MyMoneyMoney>::const_iterator it;
  for (it = d->m_reconciliationHistory.constBegin();
       it != d->m_reconciliationHistory.constEnd();
       ++it) {

    history += QString("%1%2:%3").arg(sep,
                                      it.key().toString(Qt::ISODate),
                                      (*it).toString());
    sep = QLatin1Char(';');
  }
  setValue("reconciliationHistory", history);
  return true;
}

QMap<QDate, MyMoneyMoney> MyMoneyAccount::reconciliationHistory()
{
  Q_D(MyMoneyAccount);
  // check if the internal history member is already loaded
  if (d->m_reconciliationHistory.count() == 0
      && !value("reconciliationHistory").isEmpty()) {
    QStringList entries = value("reconciliationHistory").split(';');
    foreach (const QString& entry, entries) {
      QStringList parts = entry.split(':');
      QDate date = QDate::fromString(parts[0], Qt::ISODate);
      MyMoneyMoney amount(parts[1]);
      if (parts.count() == 2 && date.isValid()) {
        d->m_reconciliationHistory[date] = amount;
      }
    }
  }

  return d->m_reconciliationHistory;
}

/**
 * @todo Improve setting of country for nationalAccount
 */
QList< payeeIdentifier > MyMoneyAccount::payeeIdentifiers() const
{
  QList< payeeIdentifier > list;

  MyMoneyFile* file = MyMoneyFile::instance();

  const auto strIBAN = QStringLiteral("iban");
  const auto strBIC = QStringLiteral("bic");
  // Iban & Bic
  if (!value(strIBAN).isEmpty()) {
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> iban(new payeeIdentifiers::ibanBic);
    iban->setIban(value(strIBAN));
    iban->setBic(file->institution(institutionId()).value(strBIC));
    iban->setOwnerName(file->user().name());
    list.append(iban);
  }

  // National Account number
  if (!number().isEmpty()) {
    payeeIdentifierTyped<payeeIdentifiers::nationalAccount> national(new payeeIdentifiers::nationalAccount);
    national->setAccountNumber(number());
    national->setBankCode(file->institution(institutionId()).sortcode());
    if (file->user().state().length() == 2)
      national->setCountry(file->user().state());
    national->setOwnerName(file->user().name());
    list.append(national);
  }

  return list;
}

bool MyMoneyAccount::hasOnlineMapping() const
{
  Q_D(const MyMoneyAccount);
  return !d->m_onlineBankingSettings.value(QLatin1String("provider")).isEmpty();
}

QString MyMoneyAccount::stdAccName(eMyMoney::Account::Standard stdAccID)
{
  static const QHash<eMyMoney::Account::Standard, QString> stdAccNames {
    {eMyMoney::Account::Standard::Liability, QStringLiteral("AStd::Liability")},
    {eMyMoney::Account::Standard::Asset,     QStringLiteral("AStd::Asset")},
    {eMyMoney::Account::Standard::Expense,   QStringLiteral("AStd::Expense")},
    {eMyMoney::Account::Standard::Income,    QStringLiteral("AStd::Income")},
    {eMyMoney::Account::Standard::Equity,    QStringLiteral("AStd::Equity")},
  };
  return stdAccNames.value(stdAccID);
}
