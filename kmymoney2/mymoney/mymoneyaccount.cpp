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

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>
//Added by qt3to4:
#include <QPixmap>
#include <QPainter>
#include <kstandarddirs.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyexception.h>
#include <mymoneyaccount.h>
#include <mymoneysplit.h>

MyMoneyAccount::MyMoneyAccount() :
  m_fraction(-1)
{
  m_accountType = UnknownAccountType;
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
  MyMoneyKeyValueContainer(node.elementsByTagName("KEYVALUEPAIRS").item(0).toElement()),
  m_fraction(-1)
{
  if("ACCOUNT" != node.tagName())
    throw new MYMONEYEXCEPTION("Node was not ACCOUNT");

  setName(node.attribute("name"));

  // qDebug("Reading information for account %s", acc.name().data());

  setParentAccountId(QStringEmpty(node.attribute("parentaccount")));
  setLastModified(stringToDate(QStringEmpty(node.attribute("lastmodified"))));
  setLastReconciliationDate(stringToDate(QStringEmpty(node.attribute("lastreconciled"))));

  if(!m_lastReconciliationDate.isValid()) {
    // for some reason, I was unable to access our own kvp at this point through
    // the value() method. It always returned empty strings. The workaround for
    // this is to construct a local kvp the same way as we have done before and
    // extract the value from it.
    //
    // Since we want to get rid of the lastStatementDate record anyway, this seems
    // to be ok for now. (ipwizard - 2008-08-14)
    QString txt = MyMoneyKeyValueContainer(node.elementsByTagName("KEYVALUEPAIRS").item(0).toElement()).value("lastStatementDate");
    if(!txt.isEmpty()) {
      setLastReconciliationDate(QDate::fromString(txt, Qt::ISODate));
    }
  }

  setInstitutionId(QStringEmpty(node.attribute("institution")));
  setNumber(QStringEmpty(node.attribute("number")));
  setOpeningDate(stringToDate(QStringEmpty(node.attribute("opened"))));
  setCurrencyId(QStringEmpty(node.attribute("currency")));

  QString tmp = QStringEmpty(node.attribute("type"));
  bool bOK = false;
  int type = tmp.toInt(&bOK);
  if(bOK) {
    setAccountType(static_cast<MyMoneyAccount::accountTypeE>(type));
  } else {
    qWarning("XMLREADER: Account %s had invalid or no account type information.", qPrintable(name()));
  }

  if(node.hasAttribute("openingbalance")) {
    if(!MyMoneyMoney(node.attribute("openingbalance")).isZero()) {
      QString msg = i18n("Account %1 contains an opening balance. Please use a KMyMoney version >= 0.8 and < 0.9 to correct the problem.", m_name);
      throw new MYMONEYEXCEPTION(msg);
    }
  }
  setDescription(node.attribute("description"));

  m_id = QStringEmpty(node.attribute("id"));
  // qDebug("Account %s has id of %s, type of %d, parent is %s.", acc.name().data(), id.data(), type, acc.parentAccountId().data());

  //  Process any Sub-Account information found inside the account entry.
  m_accountList.clear();
  QDomNodeList nodeList = node.elementsByTagName("SUBACCOUNTS");
  if(nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName("SUBACCOUNT");
    for(int i = 0; i < nodeList.count(); ++i) {
      addAccountId(QString(nodeList.item(i).toElement().attribute("id")));
    }
  }

  nodeList = node.elementsByTagName("ONLINEBANKING");
  if(nodeList.count() > 0) {
    QDomNamedNodeMap attributes = nodeList.item(0).toElement().attributes();
    for(int i = 0; i < attributes.count(); ++i) {
      const QDomAttr& it_attr = attributes.item(i).toAttr();
      m_onlineBankingSettings.setValue(it_attr.name(), it_attr.value());
    }
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
  // compatability. After that, the setValue() statemetn should be removed
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
  if(!m_accountList.contains(account))
    m_accountList += account;
}

void MyMoneyAccount::removeAccountIds(void)
{
  m_accountList.clear();
}

void MyMoneyAccount::removeAccountId(const QString& account)
{
  int pos;

  pos = m_accountList.indexOf(account);
  if(pos != -1)
    m_accountList.removeAt(pos);
}

MyMoneyAccount::accountTypeE MyMoneyAccount::accountGroup(MyMoneyAccount::accountTypeE type)
{
  switch(type) {
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
      return type;
  }
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
      (m_institution == right.m_institution) );
}

MyMoneyAccount::accountTypeE MyMoneyAccount::accountGroup(void) const
{
  return accountGroup(m_accountType);
}

void MyMoneyAccount::setCurrencyId(const QString& id)
{
  m_currencyId = id;
}

bool MyMoneyAccount::isAssetLiability(void) const
{
  return accountGroup() == Asset || accountGroup() == Liability;
}

bool MyMoneyAccount::isIncomeExpense(void) const
{
  return accountGroup() == Income || accountGroup() == Expense;
}

bool MyMoneyAccount::isLoan(void) const
{
  return accountType() == Loan || accountType() == AssetLoan;
}

bool MyMoneyAccount::isInvest(void) const
{
  return accountType() == Stock;
}


MyMoneyAccountLoan::MyMoneyAccountLoan(const MyMoneyAccount& acc)
 : MyMoneyAccount(acc)
{
}

const MyMoneyMoney MyMoneyAccountLoan::loanAmount(void) const
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

  if(!date.isValid())
    return rate;

  key.sprintf("ir-%04d-%02d-%02d", date.year(), date.month(), date.day());

  QRegExp regExp("ir-(\\d{4})-(\\d{2})-(\\d{2})");

  QMap<QString, QString>::ConstIterator it;
  for(it = pairs().begin(); it != pairs().end(); ++it) {
    if(regExp.indexIn(it.key()) > -1) {
      if(qstrcmp(it.key().toLatin1(),key.toLatin1()) <= 0)
        val = *it;
      else
        break;

    } else if(!val.isEmpty())
      break;
  }
  if(!val.isEmpty()) {
    rate = MyMoneyMoney(val);
  }

  return rate;
}

void MyMoneyAccountLoan::setInterestRate(const QDate& date, const MyMoneyMoney& value)
{
  if(!date.isValid())
    return;

  QString key;
  key.sprintf("ir-%04d-%02d-%02d", date.year(), date.month(), date.day());
  setValue(key, value.toString());
}

MyMoneyAccountLoan::interestDueE MyMoneyAccountLoan::interestCalculation(void) const
{
  QString payTime(value("interest-calculation"));
  if(payTime == "paymentDue")
    return paymentDue;
  return paymentReceived;
}

void MyMoneyAccountLoan::setInterestCalculation(const MyMoneyAccountLoan::interestDueE onReception)
{
  if(onReception == paymentDue)
    setValue("interest-calculation", "paymentDue");
  else
    setValue("interest-calculation", "paymentReceived");
}

const QDate MyMoneyAccountLoan::nextInterestChange(void) const
{
  QDate rc;

  QRegExp regExp("(\\d{4})-(\\d{2})-(\\d{2})");
  if(regExp.indexIn(value("interest-nextchange")) != -1) {
    rc.setYMD(regExp.cap(1).toInt(), regExp.cap(2).toInt(), regExp.cap(3).toInt());
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

  if(unit)
    *unit = 1;

  QRegExp regExp("(\\d+)/(\\d{1})");
  if(regExp.indexIn(value("interest-changefrequency")) != -1) {
    rc = regExp.cap(1).toInt();
    if(unit != 0) {
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

const QString MyMoneyAccountLoan::schedule(void) const
{
  return QString(value("schedule").toLatin1());
}

void MyMoneyAccountLoan::setSchedule(const QString& sched)
{
  setValue("schedule", sched);
}

bool MyMoneyAccountLoan::fixedInterestRate(void) const
{
  // make sure, that an empty kvp element returns true
  return !(value("fixed-interest") == "no");
}

void MyMoneyAccountLoan::setFixedInterestRate(const bool fixed)
{
  setValue("fixed-interest", fixed ? "yes" : "no");
  if(fixed) {
    deletePair("interest-nextchange");
    deletePair("interest-changeFrequency");
  }
}

const MyMoneyMoney MyMoneyAccountLoan::finalPayment(void) const
{
  return MyMoneyMoney(value("final-payment"));
}

void MyMoneyAccountLoan::setFinalPayment(const MyMoneyMoney& finalPayment)
{
  setValue("final-payment", finalPayment.toString());
}

unsigned int MyMoneyAccountLoan::term(void) const
{
  return value("term").toUInt();
}

void MyMoneyAccountLoan::setTerm(const unsigned int payments)
{
  setValue("term", QString::number(payments));
}

const MyMoneyMoney MyMoneyAccountLoan::periodicPayment(void) const
{
  return MyMoneyMoney(value("periodic-payment"));
}

void MyMoneyAccountLoan::setPeriodicPayment(const MyMoneyMoney& payment)
{
  setValue("periodic-payment", payment.toString());
}

const QString MyMoneyAccountLoan::payee(void) const
{
  return value("payee");
}

void MyMoneyAccountLoan::setPayee(const QString& payee)
{
  setValue("payee", payee);
}

const QString MyMoneyAccountLoan::interestAccountId(void) const
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

int MyMoneyAccountLoan::interestCompounding(void) const
{
  return value("compoundingFrequency").toInt();
}

void MyMoneyAccount::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("ACCOUNT");

  writeBaseXML(document, el);

  el.setAttribute("parentaccount", parentAccountId());
  el.setAttribute("lastreconciled", dateToString(lastReconciliationDate()));
  el.setAttribute("lastmodified", dateToString(lastModified()));
  el.setAttribute("institution", institutionId());
  el.setAttribute("opened", dateToString(openingDate()));
  el.setAttribute("number", number());
  // el.setAttribute("openingbalance", openingBalance().toString());
  el.setAttribute("type", accountType());
  el.setAttribute("name", name());
  el.setAttribute("description", description());
  if(!currencyId().isEmpty())
    el.setAttribute("currency", currencyId());

  //Add in subaccount information, if this account has subaccounts.
  if(accountCount())
  {
    QDomElement subAccounts = document.createElement("SUBACCOUNTS");
    QStringList::ConstIterator it;
    for(it = accountList().begin(); it != accountList().end(); ++it)
    {
      QDomElement temp = document.createElement("SUBACCOUNT");
      temp.setAttribute("id", (*it));
      subAccounts.appendChild(temp);
    }

    el.appendChild(subAccounts);
  }

  // Write online banking settings
  if(m_onlineBankingSettings.pairs().count()) {
    QDomElement onlinesettings = document.createElement("ONLINEBANKING");
    QMap<QString,QString>::const_iterator it_key = m_onlineBankingSettings.pairs().begin();
    while ( it_key != m_onlineBankingSettings.pairs().end() ) {
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

const MyMoneyKeyValueContainer& MyMoneyAccount::onlineBankingSettings(void) const
{
  return m_onlineBankingSettings;
}

void MyMoneyAccount::setClosed(bool closed)
{
  if(closed)
    setValue("mm-closed", "yes");
  else
    deletePair("mm-closed");
}

bool MyMoneyAccount::isClosed(void) const
{
  return !(value("mm-closed").isEmpty());
}

int MyMoneyAccount::fraction(const MyMoneySecurity& sec) const
{
  int fraction;
  if(m_accountType == Cash)
    fraction = sec.smallestCashFraction();
  else
    fraction = sec.smallestAccountFraction();
  return fraction;
}

int MyMoneyAccount::fraction(const MyMoneySecurity& sec)
{
  if(m_accountType == Cash)
    m_fraction = sec.smallestCashFraction();
  else
    m_fraction = sec.smallestAccountFraction();
  return m_fraction;
}

int MyMoneyAccount::fraction(void) const
{
  //replaced Q_ASSERT by qWarning because the Qt4 Q_ASSERT exits after printing the message
  //For new schedules it is valid to have fraction == -1
  if(m_fraction == -1)
      qWarning("fraction == -1");

  return m_fraction;
}

bool MyMoneyAccount::isCategory(void) const
{
  return m_accountType == Income || m_accountType == Expense;
}

QString MyMoneyAccount::brokerageName(void) const
{
  if(m_accountType == Investment)
    return QString("%1 (%2)").arg(m_name, i18nc("Brokerage (suffix for account names)", "Brokerage"));
  return m_name;
}

void MyMoneyAccount::adjustBalance(const MyMoneySplit& s, bool reverse)
{
  if(s.action() == MyMoneySplit::ActionSplitShares) {
    if(reverse)
      m_balance = m_balance / s.shares();
    else
      m_balance = m_balance * s.shares();
  } else {
    if(reverse)
      m_balance -= s.shares();
    else
      m_balance += s.shares();
  }

}

QPixmap MyMoneyAccount::accountPixmap(bool reconcileFlag, int size) const
{
  QString icon;
  switch(accountType()) {
    default:
      if(accountGroup() == MyMoneyAccount::Asset)
        icon = "account-types_asset";
      else
        icon = "account-types_liability";
      break;

    case MyMoneyAccount::Investment:
    case MyMoneyAccount::Stock:
    case MyMoneyAccount::MoneyMarket:
    case MyMoneyAccount::CertificateDep:
      icon = "account-types_investments";
      break;

    case MyMoneyAccount::Checkings:
      icon = "account-types_checking";
      break;
    case MyMoneyAccount::Savings:
      icon = "account-types_savings";
      break;

    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Loan:
      icon = "account-types_loan";
      break;

    case MyMoneyAccount::CreditCard:
      icon = "account-types_credit-card";
      break;

    case MyMoneyAccount::Asset:
      icon = "account-types_asset";
      break;

    case MyMoneyAccount::Cash:
      icon = "account-types_cash";
      break;

    case MyMoneyAccount::Income:
      icon = "account-types_income";
      break;

    case MyMoneyAccount::Expense:
      icon = "account-types_expense";
      break;

    case MyMoneyAccount::Equity:
      icon = "account";
      break;
  }

  QPixmap result = DesktopIcon(icon, size);
  QPainter pixmapPainter(&result);
  if(isClosed()) {
    QPixmap ovly = DesktopIcon("account-types_closed", size);
    pixmapPainter.drawPixmap(0, 0, ovly.width(), ovly.height(), ovly);
  } else if(reconcileFlag) {
    QPixmap ovly = DesktopIcon("account-types_reconcile.png", size);
    pixmapPainter.drawPixmap(0, 0, ovly.width(), ovly.height(), ovly);
  } else if(!onlineBankingSettings().value("provider").isEmpty()) {
    QPixmap ovly = DesktopIcon("account-types_online.png", size);
    pixmapPainter.drawPixmap(0, 0, ovly.width(), ovly.height(), ovly);
  }
  return result;
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
      returnString = i18nc("Unknown", "Unknown account type");
  }

  return returnString;
}
