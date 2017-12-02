/***************************************************************************
                          mymoneyaccount.cpp
                          -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2002 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
#include "payeeidentifier/ibanandbic/ibanbic.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "mymoneystoragenames.h"
#include "icons/icons.h"

using namespace MyMoneyStorageNodes;
using namespace Icons;

MyMoneyAccount::MyMoneyAccount() :
  MyMoneyObject(*new MyMoneyAccountPrivate),
  MyMoneyKeyValueContainer()
{
}

MyMoneyAccount::MyMoneyAccount(const QDomElement& node) :
    MyMoneyObject(*new MyMoneyAccountPrivate, node),
    MyMoneyKeyValueContainer(node.elementsByTagName(nodeNames[nnKeyValuePairs]).item(0).toElement())
{
  if (nodeNames[nnAccount] != node.tagName())
    throw MYMONEYEXCEPTION("Node was not ACCOUNT");

  Q_D(MyMoneyAccount);
  setName(node.attribute(d->getAttrName(Account::Attribute::Name)));

  // qDebug("Reading information for account %s", acc.name().data());

  setParentAccountId(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Account::Attribute::ParentAccount))));
  setLastModified(MyMoneyUtils::stringToDate(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Account::Attribute::LastModified)))));
  setLastReconciliationDate(MyMoneyUtils::stringToDate(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Account::Attribute::LastReconciled)))));

  if (!d->m_lastReconciliationDate.isValid()) {
    // for some reason, I was unable to access our own kvp at this point through
    // the value() method. It always returned empty strings. The workaround for
    // this is to construct a local kvp the same way as we have done before and
    // extract the value from it.
    //
    // Since we want to get rid of the lastStatementDate record anyway, this seems
    // to be ok for now. (ipwizard - 2008-08-14)
    QString txt = MyMoneyKeyValueContainer(node.elementsByTagName(nodeNames[nnKeyValuePairs]).item(0).toElement()).value("lastStatementDate");
    if (!txt.isEmpty()) {
      setLastReconciliationDate(QDate::fromString(txt, Qt::ISODate));
    }
  }

  setInstitutionId(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Account::Attribute::Institution))));
  setNumber(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Account::Attribute::Number))));
  setOpeningDate(MyMoneyUtils::stringToDate(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Account::Attribute::Opened)))));
  setCurrencyId(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Account::Attribute::Currency))));

  QString tmp = MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Account::Attribute::Type)));
  bool bOK = false;
  int type = tmp.toInt(&bOK);
  if (bOK) {
    setAccountType(static_cast<Account::Type>(type));
  } else {
    qWarning("XMLREADER: Account %s had invalid or no account type information.", qPrintable(name()));
  }

  if (node.hasAttribute(d->getAttrName(Account::Attribute::OpeningBalance))) {
    if (!MyMoneyMoney(node.attribute(d->getAttrName(Account::Attribute::OpeningBalance))).isZero()) {
      QString msg = i18n("Account %1 contains an opening balance. Please use KMyMoney version 0.8 or later and earlier than version 0.9 to correct the problem.", d->m_name);
      throw MYMONEYEXCEPTION(msg);
    }
  }
  setDescription(node.attribute(d->getAttrName(Account::Attribute::Description)));

  d->m_id = MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Account::Attribute::ID)));
  // qDebug("Account %s has id of %s, type of %d, parent is %s.", acc.name().data(), id.data(), type, acc.parentAccountId().data());

  //  Process any Sub-Account information found inside the account entry.
  d->m_accountList.clear();
  QDomNodeList nodeList = node.elementsByTagName(d->getElName(Account::Element::SubAccounts));
  if (nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName(d->getElName(Account::Element::SubAccount));
    for (int i = 0; i < nodeList.count(); ++i) {
      addAccountId(QString(nodeList.item(i).toElement().attribute(d->getAttrName(Account::Attribute::ID))));
    }
  }

  nodeList = node.elementsByTagName(d->getElName(Account::Element::OnlineBanking));
  if (nodeList.count() > 0) {
    QDomNamedNodeMap attributes = nodeList.item(0).toElement().attributes();
    for (int i = 0; i < attributes.count(); ++i) {
      const QDomAttr& it_attr = attributes.item(i).toAttr();
      d->m_onlineBankingSettings.setValue(it_attr.name(), it_attr.value());
    }
  }

  // Up to and including version 4.6.6 the new account dialog stored the iban in the kvp-key "IBAN".
  // But the rest of the software uses "iban". So correct this:
  if (!value("IBAN").isEmpty()) {
    // If "iban" was not set, set it now. If it is set, the user reseted it already, so remove
    // the garbage.
    if (value(d->getAttrName(Account::Attribute::IBAN)).isEmpty())
      setValue(d->getAttrName(Account::Attribute::IBAN), value("IBAN"));
    deletePair("IBAN");
  }
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
  // FIXME: for a limited time (maybe until we delivered 1.0) we
  // keep the last reconciliation date also in the KVP for backward
  // compatibility. After that, the setValue() statemetn should be removed
  // and the XML ctor should remove the value completely from the KVP
  setValue("lastStatementDate", date.toString(Qt::ISODate));
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

const QStringList& MyMoneyAccount::accountList() const
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

template<>
QList< payeeIdentifierTyped< ::payeeIdentifiers::ibanBic> > MyMoneyAccount::payeeIdentifiersByType() const
{
  Q_D(const MyMoneyAccount);
  payeeIdentifierTyped<payeeIdentifiers::ibanBic> ident = payeeIdentifierTyped<payeeIdentifiers::ibanBic>(new payeeIdentifiers::ibanBic);
  ident->setIban(value(d->getAttrName(Account::Attribute::IBAN)));

  if (!institutionId().isEmpty()) {
    const MyMoneyInstitution institution = MyMoneyFile::instance()->institution(institutionId());
    ident->setBic(institution.value(d->getAttrName(Account::Attribute::BIC)));
  }

  ident->setOwnerName(MyMoneyFile::instance()->user().name());

  QList< payeeIdentifierTyped<payeeIdentifiers::ibanBic> > typedList;
  typedList << ident;
  return typedList;
}

void MyMoneyAccount::writeXML(QDomDocument& document, QDomElement& parent) const
{
  auto el = document.createElement(nodeNames[nnAccount]);

  Q_D(const MyMoneyAccount);
  d->writeBaseXML(document, el);

  el.setAttribute(d->getAttrName(Account::Attribute::ParentAccount), parentAccountId());
  el.setAttribute(d->getAttrName(Account::Attribute::LastReconciled), MyMoneyUtils::dateToString(lastReconciliationDate()));
  el.setAttribute(d->getAttrName(Account::Attribute::LastModified), MyMoneyUtils::dateToString(lastModified()));
  el.setAttribute(d->getAttrName(Account::Attribute::Institution), institutionId());
  el.setAttribute(d->getAttrName(Account::Attribute::Opened), MyMoneyUtils::dateToString(openingDate()));
  el.setAttribute(d->getAttrName(Account::Attribute::Number), number());
  // el.setAttribute(getAttrName(anOpeningBalance), openingBalance().toString());
  el.setAttribute(d->getAttrName(Account::Attribute::Type), (int)accountType());
  el.setAttribute(d->getAttrName(Account::Attribute::Name), name());
  el.setAttribute(d->getAttrName(Account::Attribute::Description), description());
  if (!currencyId().isEmpty())
    el.setAttribute(d->getAttrName(Account::Attribute::Currency), currencyId());

  //Add in subaccount information, if this account has subaccounts.
  if (accountCount()) {
    QDomElement subAccounts = document.createElement(d->getElName(Account::Element::SubAccounts));
    foreach (const auto accountID, accountList()) {
      QDomElement temp = document.createElement(d->getElName(Account::Element::SubAccount));
      temp.setAttribute(d->getAttrName(Account::Attribute::ID), accountID);
      subAccounts.appendChild(temp);
    }

    el.appendChild(subAccounts);
  }

  // Write online banking settings
  if (d->m_onlineBankingSettings.pairs().count()) {
    QDomElement onlinesettings = document.createElement(d->getElName(Account::Element::OnlineBanking));
    QMap<QString, QString>::const_iterator it_key = d->m_onlineBankingSettings.pairs().begin();
    while (it_key != d->m_onlineBankingSettings.pairs().end()) {
      onlinesettings.setAttribute(it_key.key(), it_key.value());
      ++it_key;
    }
    el.appendChild(onlinesettings);
  }

  // FIXME drop the lastStatementDate record from the KVP when it is
  // not stored there after setLastReconciliationDate() has been changed
  // See comment there when this will happen
  // deletePair("lastStatementDate");


  //Add in Key-Value Pairs for accounts.
  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
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
  if (s.action() == MyMoneySplit::ActionSplitShares) {
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

}

void MyMoneyAccount::setBalance(const MyMoneyMoney& val)
{
  Q_D(MyMoneyAccount);
  d->m_balance = val;
}

QPixmap MyMoneyAccount::accountPixmap(const bool reconcileFlag, const int size) const
{
  static const QHash<Account::Type, Icon> accToIco {
    {Account::Type::Asset, Icon::ViewAsset},
    {Account::Type::Investment, Icon::ViewStock},
    {Account::Type::Stock, Icon::ViewStock},
    {Account::Type::MoneyMarket, Icon::ViewStock},
    {Account::Type::Checkings, Icon::ViewChecking},
    {Account::Type::Savings, Icon::ViewSaving},
    {Account::Type::AssetLoan, Icon::ViewLoanAsset},
    {Account::Type::Loan, Icon::ViewLoan},
    {Account::Type::CreditCard, Icon::ViewCreditCard},
    {Account::Type::Asset, Icon::ViewAsset},
    {Account::Type::Cash, Icon::ViewCash},
    {Account::Type::Income, Icon::ViewIncome},
    {Account::Type::Expense, Icon::ViewExpense},
    {Account::Type::Equity, Icon::ViewEquity}
  };

  Icon ixIcon = accToIco.value(accountType(), Icon::ViewLiability);
  QString kyIcon = g_Icons.value(ixIcon) + QString::number(size);
  QPixmap pxIcon;

  if (!QPixmapCache::find(kyIcon, pxIcon)) {
    pxIcon = QIcon::fromTheme(g_Icons.value(ixIcon)).pixmap(size); // Qt::AA_UseHighDpiPixmaps (in Qt 5.7) doesn't return highdpi pixmap
    QPixmapCache::insert(kyIcon, pxIcon);
  }

  if (isClosed())
    ixIcon = Icon::AccountClosed;
  else if (reconcileFlag)
    ixIcon = Icon::FlagGreen;
  else if (!onlineBankingSettings().value("provider").isEmpty())
    ixIcon = Icon::Download;
  else
    return pxIcon;

  QPixmap pxOverlay = QIcon::fromTheme(g_Icons[ixIcon]).pixmap(size);
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
  Q_D(const MyMoneyAccount);
  QList< payeeIdentifier > list;

  MyMoneyFile* file = MyMoneyFile::instance();

  // Iban & Bic
  if (!value(d->getAttrName(Account::Attribute::IBAN)).isEmpty()) {
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> iban(new payeeIdentifiers::ibanBic);
    iban->setIban(value(d->getAttrName(Account::Attribute::IBAN)));
    iban->setBic(file->institution(institutionId()).value(d->getAttrName(Account::Attribute::BIC)));
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
