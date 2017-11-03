/***************************************************************************
                          mymoneysplit.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#include "mymoneysplit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include "mymoneytransaction.h"

const char MyMoneySplit::ActionCheck[] = "Check";
const char MyMoneySplit::ActionDeposit[] = "Deposit";
const char MyMoneySplit::ActionTransfer[] = "Transfer";
const char MyMoneySplit::ActionWithdrawal[] = "Withdrawal";
const char MyMoneySplit::ActionATM[] = "ATM";

const char MyMoneySplit::ActionAmortization[] = "Amortization";
const char MyMoneySplit::ActionInterest[] = "Interest";

const char MyMoneySplit::ActionBuyShares[] = "Buy";
const char MyMoneySplit::ActionDividend[] = "Dividend";
const char MyMoneySplit::ActionReinvestDividend[] = "Reinvest";
const char MyMoneySplit::ActionYield[] = "Yield";
const char MyMoneySplit::ActionAddShares[] = "Add";
const char MyMoneySplit::ActionSplitShares[] = "Split";
const char MyMoneySplit::ActionInterestIncome[] = "IntIncome";

class MyMoneySplitPrivate {

public:
  /**
    * This member contains the ID of the payee
    */
  QString        m_payee;

  /**
    * This member contains a list of the IDs of the tags
    */
  QList<QString> m_tagList;

  /**
    * This member contains the ID of the account
    */
  QString        m_account;

  /**
   * This member contains the ID of the cost center
   */
  QString        m_costCenter;

  /**
    */
  MyMoneyMoney   m_shares;

  /**
    */
  MyMoneyMoney   m_value;

  /**
    * If the quotient of m_shares divided by m_values is not the correct price
    * because of truncation, the price can be stored in this member. For display
    * purpose and transaction edit this value can be used by the application.
    */
  MyMoneyMoney   m_price;

  QString        m_memo;

  /**
    * This member contains information about the reconciliation
    * state of the split. Possible values are
    *
    * @li NotReconciled
    * @li Cleared
    * @li Reconciled
    * @li Frozen
    *
    */
  eMyMoney::Split::State m_reconcileFlag;

  /**
    * In case the reconciliation flag is set to Reconciled or Frozen
    * this member contains the date of the reconciliation.
    */
  QDate          m_reconcileDate;

  /**
    * The m_action member is an arbitrary string, but is intended to
    * be conveniently limited to a menu of selections such as
    * "Buy", "Sell", "Interest", etc.
    */
  QString        m_action;

  /**
    * The m_number member is used to store a reference number to
    * the split supplied by the user (e.g. check number, etc.).
    */
  QString        m_number;

  /**
    * This member keeps the bank's unique ID for the split, so we can
    * avoid duplicates.  This is only used for electronic statement downloads.
    *
    * This should only be set on the split which refers to the account
    * that was downloaded.
    */
  QString        m_bankID;

  /**
    * This member keeps a backward id to the transaction that this
    * split can be found in. It is the purpose of the MyMoneyTransaction
    * object to maintain this member variable.
    */
  QString        m_transactionId;
};

MyMoneySplit::MyMoneySplit() :
d_ptr(new MyMoneySplitPrivate)
{
  Q_D(MyMoneySplit);
  d->m_reconcileFlag = eMyMoney::Split::State::NotReconciled;
}

MyMoneySplit::MyMoneySplit(const QDomElement& node) :
    MyMoneyObject(node, false),
    MyMoneyKeyValueContainer(node.elementsByTagName(getElName(Element::KeyValuePairs)).item(0).toElement()),
    d_ptr(new MyMoneySplitPrivate)
{
  if (getElName(Element::Split) != node.tagName())
    throw MYMONEYEXCEPTION("Node was not SPLIT");

  clearId();

  Q_D(MyMoneySplit);
  d->m_payee = QStringEmpty(node.attribute(getAttrName(Attribute::Payee)));

  QDomNodeList nodeList = node.elementsByTagName(getElName(Element::Tag));
  for (int i = 0; i < nodeList.count(); i++)
    d->m_tagList << QStringEmpty(nodeList.item(i).toElement().attribute(getAttrName(Attribute::ID)));

  d->m_reconcileDate = stringToDate(QStringEmpty(node.attribute(getAttrName(Attribute::ReconcileDate))));
  d->m_action = QStringEmpty(node.attribute(getAttrName(Attribute::Action)));
  d->m_reconcileFlag = static_cast<eMyMoney::Split::State>(node.attribute(getAttrName(Attribute::ReconcileFlag)).toInt());
  d->m_memo = QStringEmpty(node.attribute(getAttrName(Attribute::Memo)));
  d->m_value = MyMoneyMoney(QStringEmpty(node.attribute(getAttrName(Attribute::Value))));
  d->m_shares = MyMoneyMoney(QStringEmpty(node.attribute(getAttrName(Attribute::Shares))));
  d->m_price = MyMoneyMoney(QStringEmpty(node.attribute(getAttrName(Attribute::Price))));
  d->m_account = QStringEmpty(node.attribute(getAttrName(Attribute::Account)));
  d->m_costCenter = QStringEmpty(node.attribute(getAttrName(Attribute::CostCenter)));
  d->m_number = QStringEmpty(node.attribute(getAttrName(Attribute::Number)));
  d->m_bankID = QStringEmpty(node.attribute(getAttrName(Attribute::BankID)));
}

MyMoneySplit::MyMoneySplit(const MyMoneySplit& other) :
  MyMoneyObject(other.id()),
  MyMoneyKeyValueContainer(other),
  d_ptr(new MyMoneySplitPrivate(*other.d_func()))
{
}

MyMoneySplit::MyMoneySplit(const QString& id, const MyMoneySplit& other) :
  MyMoneyObject(id),
  MyMoneyKeyValueContainer(other),
  d_ptr(new MyMoneySplitPrivate(*other.d_func()))
{
}

MyMoneySplit::~MyMoneySplit()
{
  Q_D(MyMoneySplit);
  delete d;
}

bool MyMoneySplit::operator == (const MyMoneySplit& right) const
{
  Q_D(const MyMoneySplit);
  auto d2 = static_cast<const MyMoneySplitPrivate *>(right.d_func());
  return MyMoneyObject::operator==(right) &&
         MyMoneyKeyValueContainer::operator==(right) &&
         d->m_account == d2->m_account &&
         d->m_costCenter == d2->m_costCenter &&
         d->m_payee == d2->m_payee &&
         d->m_tagList == d2->m_tagList &&
         d->m_memo == d2->m_memo &&
         d->m_action == d2->m_action &&
         d->m_reconcileDate == d2->m_reconcileDate &&
         d->m_reconcileFlag == d2->m_reconcileFlag &&
         ((d->m_number.length() == 0 && d2->m_number.length() == 0) || d->m_number == d2->m_number) &&
         d->m_shares == d2->m_shares &&
         d->m_value == d2->m_value &&
         d->m_price == d2->m_price &&
         d->m_transactionId == d2->m_transactionId;
}

MyMoneySplit MyMoneySplit::operator-() const
{
    MyMoneySplit rc(*this);
  rc.d_func()->m_shares = -rc.d_func()->m_shares;
  rc.d_func()->m_value = -rc.d_func()->m_value;
  return rc;
}

QString MyMoneySplit::accountId() const
{
  Q_D(const MyMoneySplit);
  return d->m_account;
}

void MyMoneySplit::setAccountId(const QString& account)
{
  Q_D(MyMoneySplit);
  d->m_account = account;
}

QString MyMoneySplit::costCenterId() const
{
  Q_D(const MyMoneySplit);
  return d->m_costCenter;
}

void MyMoneySplit::setCostCenterId(const QString& costCenter)
{
  Q_D(MyMoneySplit);
  d->m_costCenter = costCenter;
}

QString MyMoneySplit::memo() const
{
  Q_D(const MyMoneySplit);
  return d->m_memo;
}

void MyMoneySplit::setMemo(const QString& memo)
{
  Q_D(MyMoneySplit);
  d->m_memo = memo;
}

eMyMoney::Split::State MyMoneySplit::reconcileFlag() const
{
  Q_D(const MyMoneySplit);
  return d->m_reconcileFlag;
}

QDate MyMoneySplit::reconcileDate() const
{
  Q_D(const MyMoneySplit);
  return d->m_reconcileDate;
}

void MyMoneySplit::setReconcileDate(const QDate& date)
{
  Q_D(MyMoneySplit);
  d->m_reconcileDate = date;
}

void MyMoneySplit::setReconcileFlag(const eMyMoney::Split::State flag)
{
  Q_D(MyMoneySplit);
  d->m_reconcileFlag = flag;
}

MyMoneyMoney MyMoneySplit::shares() const
{
  Q_D(const MyMoneySplit);
  return d->m_shares;
}

void MyMoneySplit::setShares(const MyMoneyMoney& shares)
{
  Q_D(MyMoneySplit);
  d->m_shares = shares;
}

QString MyMoneySplit::value(const QString& key) const
{
  return MyMoneyKeyValueContainer::value(key);
}

void MyMoneySplit::setValue(const QString& key, const QString& value)
{
  MyMoneyKeyValueContainer::setValue(key, value);
}

void MyMoneySplit::setValue(const MyMoneyMoney& value)
{
  Q_D(MyMoneySplit);
  d->m_value = value;
}

void MyMoneySplit::setValue(const MyMoneyMoney& value, const QString& transactionCurrencyId, const QString& splitCurrencyId)
{
  if (transactionCurrencyId == splitCurrencyId)
    setValue(value);
  else
    setShares(value);
}

QString MyMoneySplit::payeeId() const
{
  Q_D(const MyMoneySplit);
  return d->m_payee;
}

void MyMoneySplit::setPayeeId(const QString& payee)
{
  Q_D(MyMoneySplit);
  d->m_payee = payee;
}

QList<QString> MyMoneySplit::tagIdList() const
{
  Q_D(const MyMoneySplit);
  return d->m_tagList;
}

void MyMoneySplit::setTagIdList(const QList<QString>& tagList)
{
  Q_D(MyMoneySplit);
  d->m_tagList = tagList;
}

void MyMoneySplit::setAction(eMyMoney::Split::InvestmentTransactionType type)
{
  switch (type) {
    case eMyMoney::Split::InvestmentTransactionType::BuyShares:
    case eMyMoney::Split::InvestmentTransactionType::SellShares:
      setAction(ActionBuyShares);
      break;
    case eMyMoney::Split::InvestmentTransactionType::Dividend:
      setAction(ActionDividend);
      break;
    case eMyMoney::Split::InvestmentTransactionType::Yield:
      setAction(ActionYield);
      break;
    case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
      setAction(ActionReinvestDividend);
      break;
    case eMyMoney::Split::InvestmentTransactionType::AddShares:
    case eMyMoney::Split::InvestmentTransactionType::RemoveShares:
      setAction(ActionAddShares);
      break;
    case eMyMoney::Split::InvestmentTransactionType::SplitShares:
      setAction(ActionSplitShares);
      break;
    case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
      setAction(ActionInterestIncome);
      break;
    case eMyMoney::Split::InvestmentTransactionType::UnknownTransactionType:
      break;
  }
}

QString MyMoneySplit::action() const
{
  Q_D(const MyMoneySplit);
  return d->m_action;
}

void MyMoneySplit::setAction(const QString& action)
{
  Q_D(MyMoneySplit);
  d->m_action = action;
}

bool MyMoneySplit::isAmortizationSplit() const
{
  Q_D(const MyMoneySplit);
  return d->m_action == ActionAmortization;
}

bool MyMoneySplit::isInterestSplit() const
{
  Q_D(const MyMoneySplit);
  return d->m_action == ActionInterest;
}

QString MyMoneySplit::number() const
{
  Q_D(const MyMoneySplit);
  return d->m_number;
}

void MyMoneySplit::setNumber(const QString& number)
{
  Q_D(MyMoneySplit);
  d->m_number = number;
}

bool MyMoneySplit::isAutoCalc() const
{
  Q_D(const MyMoneySplit);
  return (d->m_shares == MyMoneyMoney::autoCalc) || (d->m_value == MyMoneyMoney::autoCalc);
}

QString MyMoneySplit::bankID() const
{
  Q_D(const MyMoneySplit);
  return d->m_bankID;
}

void MyMoneySplit::setBankID(const QString& bankID)
{
  Q_D(MyMoneySplit);
  d->m_bankID = bankID;
}

QString MyMoneySplit::transactionId() const
{
  Q_D(const MyMoneySplit);
  return d->m_transactionId;
}

void MyMoneySplit::setTransactionId(const QString& id)
{
  Q_D(MyMoneySplit);
  d->m_transactionId = id;
}


MyMoneyMoney MyMoneySplit::value() const
{
  Q_D(const MyMoneySplit);
  return d->m_value;
}

MyMoneyMoney MyMoneySplit::value(const QString& transactionCurrencyId, const QString& splitCurrencyId) const
{
  Q_D(const MyMoneySplit);
  return (transactionCurrencyId == splitCurrencyId) ? d->m_value : d->m_shares;
}

MyMoneyMoney MyMoneySplit::actualPrice() const
{
  Q_D(const MyMoneySplit);
  return d->m_price;
}

void MyMoneySplit::setPrice(const MyMoneyMoney& price)
{
  Q_D(MyMoneySplit);
  d->m_price = price;
}

MyMoneyMoney MyMoneySplit::price() const
{
  Q_D(const MyMoneySplit);
  if (!d->m_price.isZero())
    return d->m_price;
  if (!d->m_value.isZero() && !d->m_shares.isZero())
    return d->m_value / d->m_shares;
  return MyMoneyMoney::ONE;
}

void MyMoneySplit::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement(getElName(Element::Split));

  writeBaseXML(document, el);

  Q_D(const MyMoneySplit);
  el.setAttribute(getAttrName(Attribute::Payee), d->m_payee);
  //el.setAttribute(getAttrName(Attribute::Tag), m_tag);
  el.setAttribute(getAttrName(Attribute::ReconcileDate), dateToString(d->m_reconcileDate));
  el.setAttribute(getAttrName(Attribute::Action), d->m_action);
  el.setAttribute(getAttrName(Attribute::ReconcileFlag), (int)d->m_reconcileFlag);
  el.setAttribute(getAttrName(Attribute::Value), d->m_value.toString());
  el.setAttribute(getAttrName(Attribute::Shares), d->m_shares.toString());
  if (!d->m_price.isZero())
    el.setAttribute(getAttrName(Attribute::Price), d->m_price.toString());
  el.setAttribute(getAttrName(Attribute::Memo), d->m_memo);
  // No need to write the split id as it will be re-assigned when the file is read
  // el.setAttribute(getAttrName(Attribute::ID), split.id());
  el.setAttribute(getAttrName(Attribute::Account), d->m_account);
  el.setAttribute(getAttrName(Attribute::Number), d->m_number);
  el.setAttribute(getAttrName(Attribute::BankID), d->m_bankID);
  if(!d->m_costCenter.isEmpty())
    el.setAttribute(getAttrName(Attribute::CostCenter), d->m_costCenter);

  for (int i = 0; i < d->m_tagList.count(); i++) {
    QDomElement sel = document.createElement(getElName(Element::Tag));
    sel.setAttribute(getAttrName(Attribute::ID), d->m_tagList[i]);
    el.appendChild(sel);
  }

  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneySplit::hasReferenceTo(const QString& id) const
{
  Q_D(const MyMoneySplit);
  auto rc = false;
  if (isMatched()) {
    rc = matchedTransaction().hasReferenceTo(id);
  }
  for (int i = 0; i < d->m_tagList.size(); i++)
    if (id == d->m_tagList[i])
      return true;
  return rc || (id == d->m_account) || (id == d->m_payee) || (id == d->m_costCenter);
}

bool MyMoneySplit::isMatched() const
{
  return !(value(getAttrName(Attribute::KMMatchedTx)).isEmpty());
}

void MyMoneySplit::addMatch(const MyMoneyTransaction& _t)
{
  //  now we allow matching of two manual transactions
  if (!isMatched()) {
    MyMoneyTransaction t(_t);
    t.clearId();
    QDomDocument doc(getElName(Element::Match));
    QDomElement el = doc.createElement(getElName(Element::Container));
    doc.appendChild(el);
    t.writeXML(doc, el);
    QString xml = doc.toString();
    xml.replace('<', "&lt;");
    setValue(getAttrName(Attribute::KMMatchedTx), xml);
  }
}

void MyMoneySplit::removeMatch()
{
  deletePair(getAttrName(Attribute::KMMatchedTx));
}

MyMoneyTransaction MyMoneySplit::matchedTransaction() const
{
  auto xml = value(getAttrName(Attribute::KMMatchedTx));
  if (!xml.isEmpty()) {
    xml.replace("&lt;", "<");
    QDomDocument doc;
    QDomElement node;
    doc.setContent(xml);
    node = doc.documentElement().firstChild().toElement();
    MyMoneyTransaction t(node, false);
    return t;
  }
  return MyMoneyTransaction();
}

bool MyMoneySplit::replaceId(const QString& newId, const QString& oldId)
{
  auto changed = false;
  Q_D(MyMoneySplit);

  if (d->m_payee == oldId) {
    d->m_payee = newId;
    changed = true;
  } else if (d->m_account == oldId) {
    d->m_account = newId;
    changed = true;
  } else if (d->m_costCenter == oldId) {
    d->m_costCenter = newId;
    changed = true;
  }

  if (isMatched()) {
    MyMoneyTransaction t = matchedTransaction();
    if (t.replaceId(newId, oldId)) {
      removeMatch();
      addMatch(t);
      changed = true;
    }
  }

  return changed;
}

QString MyMoneySplit::getElName(const Element el)
{
  static const QHash<Element, QString> elNames = {
    {Element::Split,          QStringLiteral("SPLIT")},
    {Element::Tag,            QStringLiteral("TAG")},
    {Element::Match,          QStringLiteral("MATCH")},
    {Element::Container,      QStringLiteral("CONTAINER")},
    {Element::KeyValuePairs,  QStringLiteral("KEYVALUEPAIRS")}
  };
  return elNames[el];
}

QString MyMoneySplit::getAttrName(const Attribute attr)
{
  static const QHash<Attribute, QString> attrNames = {
    {Attribute::ID,             QStringLiteral("id")},
    {Attribute::BankID,         QStringLiteral("bankid")},
    {Attribute::Account,        QStringLiteral("account")},
    {Attribute::Payee,          QStringLiteral("payee")},
    {Attribute::Tag,            QStringLiteral("tag")},
    {Attribute::Number,         QStringLiteral("number")},
    {Attribute::Action,         QStringLiteral("action")},
    {Attribute::Value,          QStringLiteral("value")},
    {Attribute::Shares,         QStringLiteral("shares")},
    {Attribute::Price,          QStringLiteral("price")},
    {Attribute::Memo,           QStringLiteral("memo")},
    {Attribute::CostCenter,     QStringLiteral("costcenter")},
    {Attribute::ReconcileDate,  QStringLiteral("reconciledate")},
    {Attribute::ReconcileFlag,  QStringLiteral("reconcileflag")},
    {Attribute::KMMatchedTx,    QStringLiteral("kmm-matched-tx")}
  };
  return attrNames[attr];
}
