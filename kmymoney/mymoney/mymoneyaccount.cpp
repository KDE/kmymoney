/***************************************************************************
                          mymoneyaccount.cpp
                          -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2002 by Thomas Baumgart <ipwizard@users.sourceforge.net>

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

#include <QRegExp>
#include <QPixmap>
#include <QPixmapCache>
#include <QPainter>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneyexception.h"
#include "mymoneysplit.h"
#include "mymoneyfile.h"
#include "payeeidentifier/payeeidentifiertyped.h"
#include "payeeidentifier/ibanandbic/ibanbic.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "mymoneystoragenames.h"
#include "icons/icons.h"

using namespace MyMoneyStorageNodes;
using namespace Icons;

MyMoneyAccount::MyMoneyAccount() :
    m_accountType(UnknownAccountType),
    m_fraction(-1)
{
}

MyMoneyAccount::~MyMoneyAccount()
{
}

MyMoneyAccount::MyMoneyAccount(const QString& id, const MyMoneyAccount& right) :
    MyMoneyObject(id)
{
  *this = right;
  setId(id);
}

MyMoneyAccount::MyMoneyAccount(const QDomElement& node) :
    MyMoneyObject(node),
    MyMoneyKeyValueContainer(node.elementsByTagName(nodeNames[nnKeyValuePairs]).item(0).toElement()),
    m_accountType(UnknownAccountType),
    m_fraction(-1)
{
  if (nodeNames[nnAccount] != node.tagName())
    throw MYMONEYEXCEPTION("Node was not ACCOUNT");

  setName(node.attribute(getAttrName(anName)));

  // qDebug("Reading information for account %s", acc.name().data());

  setParentAccountId(QStringEmpty(node.attribute(getAttrName(anParentAccount))));
  setLastModified(stringToDate(QStringEmpty(node.attribute(getAttrName(anLastModified)))));
  setLastReconciliationDate(stringToDate(QStringEmpty(node.attribute(getAttrName(anLastReconciled)))));

  if (!m_lastReconciliationDate.isValid()) {
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

  setInstitutionId(QStringEmpty(node.attribute(getAttrName(anInstitution))));
  setNumber(QStringEmpty(node.attribute(getAttrName(anNumber))));
  setOpeningDate(stringToDate(QStringEmpty(node.attribute(getAttrName(anOpened)))));
  setCurrencyId(QStringEmpty(node.attribute(getAttrName(anCurrency))));

  QString tmp = QStringEmpty(node.attribute(getAttrName(anType)));
  bool bOK = false;
  int type = tmp.toInt(&bOK);
  if (bOK) {
    setAccountType(static_cast<MyMoneyAccount::accountTypeE>(type));
  } else {
    qWarning("XMLREADER: Account %s had invalid or no account type information.", qPrintable(name()));
  }

  if (node.hasAttribute(getAttrName(anOpeningBalance))) {
    if (!MyMoneyMoney(node.attribute(getAttrName(anOpeningBalance))).isZero()) {
      QString msg = i18n("Account %1 contains an opening balance. Please use KMyMoney version 0.8 or later and earlier than version 0.9 to correct the problem.", m_name);
      throw MYMONEYEXCEPTION(msg);
    }
  }
  setDescription(node.attribute(getAttrName(anDescription)));

  m_id = QStringEmpty(node.attribute(getAttrName(anID)));
  // qDebug("Account %s has id of %s, type of %d, parent is %s.", acc.name().data(), id.data(), type, acc.parentAccountId().data());

  //  Process any Sub-Account information found inside the account entry.
  m_accountList.clear();
  QDomNodeList nodeList = node.elementsByTagName(getElName(enSubAccounts));
  if (nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName(getElName(enSubAccount));
    for (int i = 0; i < nodeList.count(); ++i) {
      addAccountId(QString(nodeList.item(i).toElement().attribute(getAttrName(anID))));
    }
  }

  nodeList = node.elementsByTagName(getElName(enOnlineBanking));
  if (nodeList.count() > 0) {
    QDomNamedNodeMap attributes = nodeList.item(0).toElement().attributes();
    for (int i = 0; i < attributes.count(); ++i) {
      const QDomAttr& it_attr = attributes.item(i).toAttr();
      m_onlineBankingSettings.setValue(it_attr.name(), it_attr.value());
    }
  }

  // Up to and including version 4.6.6 the new account dialog stored the iban in the kvp-key "IBAN".
  // But the rest of the software uses "iban". So correct this:
  if (!value("IBAN").isEmpty()) {
    // If "iban" was not set, set it now. If it is set, the user reseted it already, so remove
    // the garbage.
    if (value(getAttrName(anIBAN)).isEmpty())
      setValue(getAttrName(anIBAN), value("IBAN"));
    deletePair("IBAN");
  }
}

void MyMoneyAccount::setName(const QString& name)
{
  m_name = name;
}

void MyMoneyAccount::setNumber(const QString& number)
{
  m_number = number;
}

void MyMoneyAccount::setDescription(const QString& desc)
{
  m_description = desc;
}

void MyMoneyAccount::setInstitutionId(const QString& id)
{
  m_institution = id;
}

void MyMoneyAccount::setLastModified(const QDate& date)
{
  m_lastModified = date;
}

void MyMoneyAccount::setOpeningDate(const QDate& date)
{
  m_openingDate = date;
}

void MyMoneyAccount::setLastReconciliationDate(const QDate& date)
{
  // FIXME: for a limited time (maybe until we delivered 1.0) we
  // keep the last reconciliation date also in the KVP for backward
  // compatibility. After that, the setValue() statemetn should be removed
  // and the XML ctor should remove the value completely from the KVP
  setValue("lastStatementDate", date.toString(Qt::ISODate));
  m_lastReconciliationDate = date;
}

void MyMoneyAccount::setParentAccountId(const QString& parent)
{
  m_parentAccount = parent;
}

void MyMoneyAccount::setAccountType(const accountTypeE type)
{
  m_accountType = type;
}

void MyMoneyAccount::addAccountId(const QString& account)
{
  if (!m_accountList.contains(account))
    m_accountList += account;
}

void MyMoneyAccount::removeAccountIds()
{
  m_accountList.clear();
}

void MyMoneyAccount::removeAccountId(const QString& account)
{
  int pos;

  pos = m_accountList.indexOf(account);
  if (pos != -1)
    m_accountList.removeAt(pos);
}

bool MyMoneyAccount::operator == (const MyMoneyAccount& right) const
{
  return (MyMoneyKeyValueContainer::operator==(right) &&
          MyMoneyObject::operator==(right) &&
          (m_accountList == right.m_accountList) &&
          (m_accountType == right.m_accountType) &&
          (m_lastModified == right.m_lastModified) &&
          (m_lastReconciliationDate == right.m_lastReconciliationDate) &&
          ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)) &&
          ((m_number.length() == 0 && right.m_number.length() == 0) || (m_number == right.m_number)) &&
          ((m_description.length() == 0 && right.m_description.length() == 0) || (m_description == right.m_description)) &&
          (m_openingDate == right.m_openingDate) &&
          (m_parentAccount == right.m_parentAccount) &&
          (m_currencyId == right.m_currencyId) &&
          (m_institution == right.m_institution));
}

MyMoneyAccount::accountTypeE MyMoneyAccount::accountGroup() const
{
  switch (m_accountType) {
    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Currency:
    case MyMoneyAccount::Investment:
    case MyMoneyAccount::MoneyMarket:
    case MyMoneyAccount::CertificateDep:
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Stock:
      return MyMoneyAccount::Asset;

    case MyMoneyAccount::CreditCard:
    case MyMoneyAccount::Loan:
      return MyMoneyAccount::Liability;

    default:
      return m_accountType;
  }
}

void MyMoneyAccount::setCurrencyId(const QString& id)
{
  m_currencyId = id;
}

bool MyMoneyAccount::isAssetLiability() const
{
  return accountGroup() == Asset || accountGroup() == Liability;
}

bool MyMoneyAccount::isIncomeExpense() const
{
  return accountGroup() == Income || accountGroup() == Expense;
}

bool MyMoneyAccount::isLoan() const
{
  return accountType() == Loan || accountType() == AssetLoan;
}

bool MyMoneyAccount::isInvest() const
{
  return accountType() == Stock;
}

bool MyMoneyAccount::isLiquidAsset() const
{
  return accountType() == Checkings ||
         accountType() == Savings ||
         accountType() == Cash;
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
  ident->setIban(value(getAttrName(anIBAN)));

  if (!institutionId().isEmpty()) {
    const MyMoneyInstitution institution = MyMoneyFile::instance()->institution(institutionId());
    ident->setBic(institution.value(getAttrName(anBIC)));
  }

  ident->setOwnerName(MyMoneyFile::instance()->user().name());

  QList< payeeIdentifierTyped<payeeIdentifiers::ibanBic> > typedList;
  typedList << ident;
  return typedList;
}

MyMoneyAccountLoan::MyMoneyAccountLoan(const MyMoneyAccount& acc)
    : MyMoneyAccount(acc)
{
}

const MyMoneyMoney MyMoneyAccountLoan::loanAmount() const
{
  return MyMoneyMoney(value("loan-amount"));
}

void MyMoneyAccountLoan::setLoanAmount(const MyMoneyMoney& amount)
{
  setValue("loan-amount", amount.toString());
}

const MyMoneyMoney MyMoneyAccountLoan::interestRate(const QDate& date) const
{
  MyMoneyMoney rate;
  QString key;
  QString val;

  if (!date.isValid())
    return rate;

  key.sprintf("ir-%04d-%02d-%02d", date.year(), date.month(), date.day());

  QRegExp regExp("ir-(\\d{4})-(\\d{2})-(\\d{2})");

  QMap<QString, QString>::ConstIterator it;
  for (it = pairs().begin(); it != pairs().end(); ++it) {
    if (regExp.indexIn(it.key()) > -1) {
      if (qstrcmp(it.key().toLatin1(), key.toLatin1()) <= 0)
        val = *it;
      else
        break;

    } else if (!val.isEmpty())
      break;
  }
  if (!val.isEmpty()) {
    rate = MyMoneyMoney(val);
  }

  return rate;
}

void MyMoneyAccountLoan::setInterestRate(const QDate& date, const MyMoneyMoney& value)
{
  if (!date.isValid())
    return;

  QString key;
  key.sprintf("ir-%04d-%02d-%02d", date.year(), date.month(), date.day());
  setValue(key, value.toString());
}

MyMoneyAccountLoan::interestDueE MyMoneyAccountLoan::interestCalculation() const
{
  QString payTime(value("interest-calculation"));
  if (payTime == "paymentDue")
    return paymentDue;
  return paymentReceived;
}

void MyMoneyAccountLoan::setInterestCalculation(const MyMoneyAccountLoan::interestDueE onReception)
{
  if (onReception == paymentDue)
    setValue("interest-calculation", "paymentDue");
  else
    setValue("interest-calculation", "paymentReceived");
}

const QDate MyMoneyAccountLoan::nextInterestChange() const
{
  QDate rc;

  QRegExp regExp("(\\d{4})-(\\d{2})-(\\d{2})");
  if (regExp.indexIn(value("interest-nextchange")) != -1) {
    rc.setDate(regExp.cap(1).toInt(), regExp.cap(2).toInt(), regExp.cap(3).toInt());
  }
  return rc;
}

void MyMoneyAccountLoan::setNextInterestChange(const QDate& date)
{
  setValue("interest-nextchange", date.toString(Qt::ISODate));
}

int MyMoneyAccountLoan::interestChangeFrequency(int* unit) const
{
  int rc = -1;

  if (unit)
    *unit = 1;

  QRegExp regExp("(\\d+)/(\\d{1})");
  if (regExp.indexIn(value("interest-changefrequency")) != -1) {
    rc = regExp.cap(1).toInt();
    if (unit != 0) {
      *unit = regExp.cap(2).toInt();
    }
  }
  return rc;
}

void MyMoneyAccountLoan::setInterestChangeFrequency(const int amount, const int unit)
{
  QString val;
  val.sprintf("%d/%d", amount, unit);
  setValue("interest-changeFrequency", val);
}

const QString MyMoneyAccountLoan::schedule() const
{
  return QString(value("schedule").toLatin1());
}

void MyMoneyAccountLoan::setSchedule(const QString& sched)
{
  setValue("schedule", sched);
}

bool MyMoneyAccountLoan::fixedInterestRate() const
{
  // make sure, that an empty kvp element returns true
  return !(value("fixed-interest") == "no");
}

void MyMoneyAccountLoan::setFixedInterestRate(const bool fixed)
{
  setValue("fixed-interest", fixed ? "yes" : "no");
  if (fixed) {
    deletePair("interest-nextchange");
    deletePair("interest-changeFrequency");
  }
}

const MyMoneyMoney MyMoneyAccountLoan::finalPayment() const
{
  return MyMoneyMoney(value("final-payment"));
}

void MyMoneyAccountLoan::setFinalPayment(const MyMoneyMoney& finalPayment)
{
  setValue("final-payment", finalPayment.toString());
}

unsigned int MyMoneyAccountLoan::term() const
{
  return value("term").toUInt();
}

void MyMoneyAccountLoan::setTerm(const unsigned int payments)
{
  setValue("term", QString::number(payments));
}

const MyMoneyMoney MyMoneyAccountLoan::periodicPayment() const
{
  return MyMoneyMoney(value("periodic-payment"));
}

void MyMoneyAccountLoan::setPeriodicPayment(const MyMoneyMoney& payment)
{
  setValue("periodic-payment", payment.toString());
}

const QString MyMoneyAccountLoan::payee() const
{
  return value("payee");
}

void MyMoneyAccountLoan::setPayee(const QString& payee)
{
  setValue("payee", payee);
}

const QString MyMoneyAccountLoan::interestAccountId() const
{
  return QString();
}

void MyMoneyAccountLoan::setInterestAccountId(const QString& /* id */)
{

}

bool MyMoneyAccountLoan::hasReferenceTo(const QString& id) const
{
  return MyMoneyAccount::hasReferenceTo(id)
         || (id == payee())
         || (id == schedule());
}

void MyMoneyAccountLoan::setInterestCompounding(int frequency)
{
  setValue("compoundingFrequency", QString("%1").arg(frequency));
}

int MyMoneyAccountLoan::interestCompounding() const
{
  return value("compoundingFrequency").toInt();
}

void MyMoneyAccount::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement(nodeNames[nnAccount]);

  writeBaseXML(document, el);

  el.setAttribute(getAttrName(anParentAccount), parentAccountId());
  el.setAttribute(getAttrName(anLastReconciled), dateToString(lastReconciliationDate()));
  el.setAttribute(getAttrName(anLastModified), dateToString(lastModified()));
  el.setAttribute(getAttrName(anInstitution), institutionId());
  el.setAttribute(getAttrName(anOpened), dateToString(openingDate()));
  el.setAttribute(getAttrName(anNumber), number());
  // el.setAttribute(getAttrName(anOpeningBalance), openingBalance().toString());
  el.setAttribute(getAttrName(anType), accountType());
  el.setAttribute(getAttrName(anName), name());
  el.setAttribute(getAttrName(anDescription), description());
  if (!currencyId().isEmpty())
    el.setAttribute(getAttrName(anCurrency), currencyId());

  //Add in subaccount information, if this account has subaccounts.
  if (accountCount()) {
    QDomElement subAccounts = document.createElement(getElName(enSubAccounts));
    QStringList::ConstIterator it;
    for (it = accountList().begin(); it != accountList().end(); ++it) {
      QDomElement temp = document.createElement(getElName(enSubAccount));
      temp.setAttribute(getAttrName(anID), (*it));
      subAccounts.appendChild(temp);
    }

    el.appendChild(subAccounts);
  }

  // Write online banking settings
  if (m_onlineBankingSettings.pairs().count()) {
    QDomElement onlinesettings = document.createElement(getElName(enOnlineBanking));
    QMap<QString, QString>::const_iterator it_key = m_onlineBankingSettings.pairs().begin();
    while (it_key != m_onlineBankingSettings.pairs().end()) {
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
  return (id == m_institution) || (id == m_parentAccount) || (id == m_currencyId);
}

void MyMoneyAccount::setOnlineBankingSettings(const MyMoneyKeyValueContainer& values)
{
  m_onlineBankingSettings = values;
}

const MyMoneyKeyValueContainer& MyMoneyAccount::onlineBankingSettings() const
{
  return m_onlineBankingSettings;
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
  int fraction;
  if (m_accountType == Cash)
    fraction = sec.smallestCashFraction();
  else
    fraction = sec.smallestAccountFraction();
  return fraction;
}

int MyMoneyAccount::fraction(const MyMoneySecurity& sec)
{
  if (m_accountType == Cash)
    m_fraction = sec.smallestCashFraction();
  else
    m_fraction = sec.smallestAccountFraction();
  return m_fraction;
}

int MyMoneyAccount::fraction() const
{
  return m_fraction;
}

bool MyMoneyAccount::isCategory() const
{
  return m_accountType == Income || m_accountType == Expense;
}

QString MyMoneyAccount::brokerageName() const
{
  if (m_accountType == Investment)
    return QString("%1 (%2)").arg(m_name, i18nc("Brokerage (suffix for account names)", "Brokerage"));
  return m_name;
}

void MyMoneyAccount::adjustBalance(const MyMoneySplit& s, bool reverse)
{
  if (s.action() == MyMoneySplit::ActionSplitShares) {
    if (reverse)
      m_balance = m_balance / s.shares();
    else
      m_balance = m_balance * s.shares();
  } else {
    if (reverse)
      m_balance -= s.shares();
    else
      m_balance += s.shares();
  }

}

QPixmap MyMoneyAccount::accountPixmap(const bool reconcileFlag, const int size) const
{
  static const QHash<MyMoneyAccount::accountTypeE, Icon> accToIco {
    {MyMoneyAccount::Asset, Icon::ViewAsset},
    {MyMoneyAccount::Investment, Icon::ViewStock},
    {MyMoneyAccount::Stock, Icon::ViewStock},
    {MyMoneyAccount::MoneyMarket, Icon::ViewStock},
    {MyMoneyAccount::Checkings, Icon::ViewChecking},
    {MyMoneyAccount::Savings, Icon::ViewSaving},
    {MyMoneyAccount::AssetLoan, Icon::ViewLoanAsset},
    {MyMoneyAccount::Loan, Icon::ViewLoan},
    {MyMoneyAccount::CreditCard, Icon::ViewCreditCard},
    {MyMoneyAccount::Asset, Icon::ViewAsset},
    {MyMoneyAccount::Cash, Icon::ViewCash},
    {MyMoneyAccount::Income, Icon::ViewIncome},
    {MyMoneyAccount::Expense, Icon::ViewExpense},
    {MyMoneyAccount::Equity, Icon::ViewEquity}
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

QString MyMoneyAccount::accountTypeToString(const MyMoneyAccount::accountTypeE accountType)
{
  QString returnString;

  switch (accountType) {
    case MyMoneyAccount::Checkings:
      returnString = i18n("Checking");
      break;
    case MyMoneyAccount::Savings:
      returnString = i18n("Savings");
      break;
    case MyMoneyAccount::CreditCard:
      returnString = i18n("Credit Card");
      break;
    case MyMoneyAccount::Cash:
      returnString = i18n("Cash");
      break;
    case MyMoneyAccount::Loan:
      returnString = i18n("Loan");
      break;
    case MyMoneyAccount::CertificateDep:
      returnString = i18n("Certificate of Deposit");
      break;
    case MyMoneyAccount::Investment:
      returnString = i18n("Investment");
      break;
    case MyMoneyAccount::MoneyMarket:
      returnString = i18n("Money Market");
      break;
    case MyMoneyAccount::Asset:
      returnString = i18n("Asset");
      break;
    case MyMoneyAccount::Liability:
      returnString = i18n("Liability");
      break;
    case MyMoneyAccount::Currency:
      returnString = i18n("Currency");
      break;
    case MyMoneyAccount::Income:
      returnString = i18n("Income");
      break;
    case MyMoneyAccount::Expense:
      returnString = i18n("Expense");
      break;
    case MyMoneyAccount::AssetLoan:
      returnString = i18n("Investment Loan");
      break;
    case MyMoneyAccount::Stock:
      returnString = i18n("Stock");
      break;
    case MyMoneyAccount::Equity:
      returnString = i18n("Equity");
      break;
    default:
      returnString = i18nc("Unknown account type", "Unknown");
  }

  return returnString;
}

bool MyMoneyAccount::addReconciliation(const QDate& date, const MyMoneyMoney& amount)
{
  m_reconciliationHistory[date] = amount;
  QString history, sep;
  QMap<QDate, MyMoneyMoney>::const_iterator it;
  for (it = m_reconciliationHistory.constBegin();
       it != m_reconciliationHistory.constEnd();
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
  // check if the internal history member is already loaded
  if (m_reconciliationHistory.count() == 0
      && !value("reconciliationHistory").isEmpty()) {
    QStringList entries = value("reconciliationHistory").split(';');
    foreach (const QString& entry, entries) {
      QStringList parts = entry.split(':');
      QDate date = QDate::fromString(parts[0], Qt::ISODate);
      MyMoneyMoney amount(parts[1]);
      if (parts.count() == 2 && date.isValid()) {
        m_reconciliationHistory[date] = amount;
      }
    }
  }

  return m_reconciliationHistory;
}

/**
 * @todo Improve setting of country for nationalAccount
 */
QList< payeeIdentifier > MyMoneyAccount::payeeIdentifiers() const
{
  QList< payeeIdentifier > list;

  MyMoneyFile* file = MyMoneyFile::instance();

  // Iban & Bic
  if (!value(getAttrName(anIBAN)).isEmpty()) {
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> iban(new payeeIdentifiers::ibanBic);
    iban->setIban(value(getAttrName(anIBAN)));
    iban->setBic(file->institution(institutionId()).value(getAttrName(anBIC)));
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

const QString MyMoneyAccount::getElName(const elNameE _el)
{
  static const QMap<elNameE, QString> elNames = {
    {enSubAccount, QStringLiteral("SUBACCOUNT")},
    {enSubAccounts, QStringLiteral("SUBACCOUNTS")},
    {enOnlineBanking, QStringLiteral("ONLINEBANKING")}
  };
  return elNames[_el];
}

const QString MyMoneyAccount::getAttrName(const attrNameE _attr)
{
  static const QHash<attrNameE, QString> attrNames = {
    {anID, QStringLiteral("id")},
    {anName, QStringLiteral("name")},
    {anType, QStringLiteral("type")},
    {anParentAccount, QStringLiteral("parentaccount")},
    {anLastReconciled, QStringLiteral("lastreconciled")},
    {anLastModified, QStringLiteral("lastmodified")},
    {anInstitution, QStringLiteral("institution")},
    {anOpened, QStringLiteral("opened")},
    {anNumber, QStringLiteral("number")},
    {anType, QStringLiteral("type")},
    {anDescription, QStringLiteral("description")},
    {anCurrency, QStringLiteral("currency")},
    {anOpeningBalance, QStringLiteral("openingbalance")},
    {anIBAN, QStringLiteral("iban")},
    {anBIC, QStringLiteral("bic")},
  };
  return attrNames[_attr];
}
