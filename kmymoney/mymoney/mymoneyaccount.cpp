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

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>
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
#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyinstitution.h"
#include "mymoneypayee.h"
#include "payeeidentifier/payeeidentifiertyped.h"
#include "payeeidentifier/ibanandbic/ibanbic.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "mymoneystoragenames.h"
#include "icons/icons.h"

using namespace eMyMoney;
using namespace MyMoneyStorageNodes;
using namespace Icons;

class MyMoneyAccountPrivate {

public:

  MyMoneyAccountPrivate() :
    m_accountType(Account::Unknown),
    m_fraction(-1)
  {
  }

  /**
    * This member variable identifies the type of account
    */
  eMyMoney::Account m_accountType;

  /**
    * This member variable keeps the ID of the MyMoneyInstitution object
    * that this object belongs to.
    */
  QString m_institution;

  /**
    * This member variable keeps the name of the account
    * It is solely for documentation purposes and it's contents is not
    * used otherwise by the mymoney-engine.
    */
  QString m_name;

  /**
    * This member variable keeps the account number at the institution
    * It is solely for documentation purposes and it's contents is not
    * used otherwise by the mymoney-engine.
    */
  QString m_number;

  /**
    * This member variable is a description of the account.
    * It is solely for documentation purposes and it's contents is not
    * used otherwise by the mymoney-engine.
    */
  QString m_description;

  /**
    * This member variable keeps the date when the account
    * was last modified.
    */
  QDate m_lastModified;

  /**
    * This member variable keeps the date when the
    * account was created as an object in a MyMoneyFile
    */
  QDate m_openingDate;

  /**
    * This member variable keeps the date of the last
    * reconciliation of this account
    */
  QDate m_lastReconciliationDate;

  /**
    * This member holds the ID's of all sub-ordinate accounts
    */
  QStringList m_accountList;

  /**
    * This member contains the ID of the parent account
    */
  QString m_parentAccount;

  /**
    * This member contains the ID of the currency associated with this account
    */
  QString m_currencyId;

  /**
    * This member holds the balance of all transactions stored in the journal
    * for this account.
    */
  MyMoneyMoney    m_balance;

  /**
    * This member variable keeps the set of kvp's needed to establish
    * online banking sessions to this account.
    */
  MyMoneyKeyValueContainer m_onlineBankingSettings;

  /**
    * This member keeps the fraction for the account. It is filled by MyMoneyFile
    * when set to -1. See also @sa fraction(const MyMoneySecurity&).
    */
  int             m_fraction;

  /**
    * This member keeps the reconciliation history
    */
  QMap<QDate, MyMoneyMoney> m_reconciliationHistory;

};

MyMoneyAccount::MyMoneyAccount() :
  MyMoneyObject(),
  MyMoneyKeyValueContainer(),
  d_ptr(new MyMoneyAccountPrivate)
{
}

MyMoneyAccount::MyMoneyAccount(const QDomElement& node) :
    MyMoneyObject(node),
    MyMoneyKeyValueContainer(node.elementsByTagName(nodeNames[nnKeyValuePairs]).item(0).toElement()),
    d_ptr(new MyMoneyAccountPrivate)
{
  if (nodeNames[nnAccount] != node.tagName())
    throw MYMONEYEXCEPTION("Node was not ACCOUNT");

  setName(node.attribute(getAttrName(Attribute::Name)));

  // qDebug("Reading information for account %s", acc.name().data());

  setParentAccountId(QStringEmpty(node.attribute(getAttrName(Attribute::ParentAccount))));
  setLastModified(stringToDate(QStringEmpty(node.attribute(getAttrName(Attribute::LastModified)))));
  setLastReconciliationDate(stringToDate(QStringEmpty(node.attribute(getAttrName(Attribute::LastReconciled)))));

  Q_D(MyMoneyAccount);
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

  setInstitutionId(QStringEmpty(node.attribute(getAttrName(Attribute::Institution))));
  setNumber(QStringEmpty(node.attribute(getAttrName(Attribute::Number))));
  setOpeningDate(stringToDate(QStringEmpty(node.attribute(getAttrName(Attribute::Opened)))));
  setCurrencyId(QStringEmpty(node.attribute(getAttrName(Attribute::Currency))));

  QString tmp = QStringEmpty(node.attribute(getAttrName(Attribute::Type)));
  bool bOK = false;
  int type = tmp.toInt(&bOK);
  if (bOK) {
    setAccountType(static_cast<Account>(type));
  } else {
    qWarning("XMLREADER: Account %s had invalid or no account type information.", qPrintable(name()));
  }

  if (node.hasAttribute(getAttrName(Attribute::OpeningBalance))) {
    if (!MyMoneyMoney(node.attribute(getAttrName(Attribute::OpeningBalance))).isZero()) {
      QString msg = i18n("Account %1 contains an opening balance. Please use KMyMoney version 0.8 or later and earlier than version 0.9 to correct the problem.", d->m_name);
      throw MYMONEYEXCEPTION(msg);
    }
  }
  setDescription(node.attribute(getAttrName(Attribute::Description)));

  m_id = QStringEmpty(node.attribute(getAttrName(Attribute::ID)));
  // qDebug("Account %s has id of %s, type of %d, parent is %s.", acc.name().data(), id.data(), type, acc.parentAccountId().data());

  //  Process any Sub-Account information found inside the account entry.
  d->m_accountList.clear();
  QDomNodeList nodeList = node.elementsByTagName(getElName(Element::SubAccounts));
  if (nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName(getElName(Element::SubAccount));
    for (int i = 0; i < nodeList.count(); ++i) {
      addAccountId(QString(nodeList.item(i).toElement().attribute(getAttrName(Attribute::ID))));
    }
  }

  nodeList = node.elementsByTagName(getElName(Element::OnlineBanking));
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
    if (value(getAttrName(Attribute::IBAN)).isEmpty())
      setValue(getAttrName(Attribute::IBAN), value("IBAN"));
    deletePair("IBAN");
  }
}

MyMoneyAccount::MyMoneyAccount(const MyMoneyAccount& other) :
  MyMoneyObject(other.id()),
  MyMoneyKeyValueContainer(other),
  d_ptr(new MyMoneyAccountPrivate(*other.d_func()))
{
}

MyMoneyAccount::MyMoneyAccount(const QString& id, const MyMoneyAccount& other) :
  MyMoneyObject(id),
  MyMoneyKeyValueContainer(other),
  d_ptr(new MyMoneyAccountPrivate(*other.d_func()))
{
}

MyMoneyAccount::~MyMoneyAccount()
{
  Q_D(MyMoneyAccount);
  delete d;
}

void MyMoneyAccount::touch()
{
  setLastModified(QDate::currentDate());
}

eMyMoney::Account MyMoneyAccount::accountType() const
{
  Q_D(const MyMoneyAccount);
  return d->m_accountType;
}

void MyMoneyAccount::setAccountType(const Account type)
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

Account MyMoneyAccount::accountGroup() const
{
  Q_D(const MyMoneyAccount);
  switch (d->m_accountType) {
    case Account::Checkings:
    case Account::Savings:
    case Account::Cash:
    case Account::Currency:
    case Account::Investment:
    case Account::MoneyMarket:
    case Account::CertificateDep:
    case Account::AssetLoan:
    case Account::Stock:
      return Account::Asset;

    case Account::CreditCard:
    case Account::Loan:
      return Account::Liability;

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
  return accountGroup() == Account::Asset || accountGroup() == Account::Liability;
}

bool MyMoneyAccount::isIncomeExpense() const
{
  return accountGroup() == Account::Income || accountGroup() == Account::Expense;
}

bool MyMoneyAccount::isLoan() const
{
  return accountType() == Account::Loan || accountType() == Account::AssetLoan;
}

bool MyMoneyAccount::isInvest() const
{
  return accountType() == Account::Stock;
}

bool MyMoneyAccount::isLiquidAsset() const
{
  return accountType() == Account::Checkings ||
         accountType() == Account::Savings ||
         accountType() == Account::Cash;
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
  payeeIdentifierTyped<payeeIdentifiers::ibanBic> ident = payeeIdentifierTyped<payeeIdentifiers::ibanBic>(new payeeIdentifiers::ibanBic);
  ident->setIban(value(getAttrName(Attribute::IBAN)));

  if (!institutionId().isEmpty()) {
    const MyMoneyInstitution institution = MyMoneyFile::instance()->institution(institutionId());
    ident->setBic(institution.value(getAttrName(Attribute::BIC)));
  }

  ident->setOwnerName(MyMoneyFile::instance()->user().name());

  QList< payeeIdentifierTyped<payeeIdentifiers::ibanBic> > typedList;
  typedList << ident;
  return typedList;
}

void MyMoneyAccount::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement(nodeNames[nnAccount]);

  writeBaseXML(document, el);

  el.setAttribute(getAttrName(Attribute::ParentAccount), parentAccountId());
  el.setAttribute(getAttrName(Attribute::LastReconciled), dateToString(lastReconciliationDate()));
  el.setAttribute(getAttrName(Attribute::LastModified), dateToString(lastModified()));
  el.setAttribute(getAttrName(Attribute::Institution), institutionId());
  el.setAttribute(getAttrName(Attribute::Opened), dateToString(openingDate()));
  el.setAttribute(getAttrName(Attribute::Number), number());
  // el.setAttribute(getAttrName(anOpeningBalance), openingBalance().toString());
  el.setAttribute(getAttrName(Attribute::Type), (int)accountType());
  el.setAttribute(getAttrName(Attribute::Name), name());
  el.setAttribute(getAttrName(Attribute::Description), description());
  if (!currencyId().isEmpty())
    el.setAttribute(getAttrName(Attribute::Currency), currencyId());

  //Add in subaccount information, if this account has subaccounts.
  if (accountCount()) {
    QDomElement subAccounts = document.createElement(getElName(Element::SubAccounts));
    foreach (const auto accountID, accountList()) {
      QDomElement temp = document.createElement(getElName(Element::SubAccount));
      temp.setAttribute(getAttrName(Attribute::ID), accountID);
      subAccounts.appendChild(temp);
    }

    el.appendChild(subAccounts);
  }

  Q_D(const MyMoneyAccount);
  // Write online banking settings
  if (d->m_onlineBankingSettings.pairs().count()) {
    QDomElement onlinesettings = document.createElement(getElName(Element::OnlineBanking));
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

const MyMoneyKeyValueContainer& MyMoneyAccount::onlineBankingSettings() const
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
  if (d->m_accountType == Account::Cash)
    fraction = sec.smallestCashFraction();
  else
    fraction = sec.smallestAccountFraction();
  return fraction;
}

int MyMoneyAccount::fraction(const MyMoneySecurity& sec)
{
  Q_D(MyMoneyAccount);
  if (d->m_accountType == Account::Cash)
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
  return d->m_accountType == Account::Income || d->m_accountType == Account::Expense;
}

QString MyMoneyAccount::brokerageName() const
{
  Q_D(const MyMoneyAccount);
  if (d->m_accountType == Account::Investment)
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
  static const QHash<Account, Icon> accToIco {
    {Account::Asset, Icon::ViewAsset},
    {Account::Investment, Icon::ViewStock},
    {Account::Stock, Icon::ViewStock},
    {Account::MoneyMarket, Icon::ViewStock},
    {Account::Checkings, Icon::ViewChecking},
    {Account::Savings, Icon::ViewSaving},
    {Account::AssetLoan, Icon::ViewLoanAsset},
    {Account::Loan, Icon::ViewLoan},
    {Account::CreditCard, Icon::ViewCreditCard},
    {Account::Asset, Icon::ViewAsset},
    {Account::Cash, Icon::ViewCash},
    {Account::Income, Icon::ViewIncome},
    {Account::Expense, Icon::ViewExpense},
    {Account::Equity, Icon::ViewEquity}
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

QString MyMoneyAccount::accountTypeToString(const Account accountType)
{
  switch (accountType) {
    case Account::Checkings:
      return i18nc("Account type", "Checking");
    case Account::Savings:
      return i18nc("Account type", "Savings");
    case Account::CreditCard:
      return i18nc("Account type", "Credit Card");
    case Account::Cash:
      return i18nc("Account type", "Cash");
    case Account::Loan:
      return i18nc("Account type", "Loan");
    case Account::CertificateDep:
      return i18nc("Account type", "Certificate of Deposit");
    case Account::Investment:
      return i18nc("Account type", "Investment");
    case Account::MoneyMarket:
      return i18nc("Account type", "Money Market");
    case Account::Asset:
      return i18nc("Account type", "Asset");
    case Account::Liability:
      return i18nc("Account type", "Liability");
    case Account::Currency:
      return i18nc("Account type", "Currency");
    case Account::Income:
      return i18nc("Account type", "Income");
    case Account::Expense:
      return i18nc("Account type", "Expense");
    case Account::AssetLoan:
      return i18nc("Account type", "Investment Loan");
    case Account::Stock:
      return i18nc("Account type", "Stock");
    case Account::Equity:
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

const QMap<QDate, MyMoneyMoney>& MyMoneyAccount::reconciliationHistory()
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

  // Iban & Bic
  if (!value(getAttrName(Attribute::IBAN)).isEmpty()) {
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> iban(new payeeIdentifiers::ibanBic);
    iban->setIban(value(getAttrName(Attribute::IBAN)));
    iban->setBic(file->institution(institutionId()).value(getAttrName(Attribute::BIC)));
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

QString MyMoneyAccount::getElName(const Element el)
{
  static const QMap<Element, QString> elNames = {
    {Element::SubAccount,     QStringLiteral("SUBACCOUNT")},
    {Element::SubAccounts,    QStringLiteral("SUBACCOUNTS")},
    {Element::OnlineBanking,  QStringLiteral("ONLINEBANKING")}
  };
  return elNames[el];
}

QString MyMoneyAccount::getAttrName(const Attribute attr)
{
  static const QHash<Attribute, QString> attrNames = {
    {Attribute::ID,             QStringLiteral("id")},
    {Attribute::Name,           QStringLiteral("name")},
    {Attribute::Type,           QStringLiteral("type")},
    {Attribute::ParentAccount,  QStringLiteral("parentaccount")},
    {Attribute::LastReconciled, QStringLiteral("lastreconciled")},
    {Attribute::LastModified,   QStringLiteral("lastmodified")},
    {Attribute::Institution,    QStringLiteral("institution")},
    {Attribute::Opened,         QStringLiteral("opened")},
    {Attribute::Number,         QStringLiteral("number")},
    {Attribute::Type,           QStringLiteral("type")},
    {Attribute::Description,    QStringLiteral("description")},
    {Attribute::Currency,       QStringLiteral("currency")},
    {Attribute::OpeningBalance, QStringLiteral("openingbalance")},
    {Attribute::IBAN,           QStringLiteral("iban")},
    {Attribute::BIC,            QStringLiteral("bic")},
  };
  return attrNames[attr];
}
