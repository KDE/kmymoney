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
#include "mymoneysplit_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDomDocument>
#include <QDomElement>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneytransaction.h"
#include "mymoneyexception.h"

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

MyMoneySplit::MyMoneySplit() :
  MyMoneyObject(*new MyMoneySplitPrivate)
{
  Q_D(MyMoneySplit);
  d->m_reconcileFlag = eMyMoney::Split::State::NotReconciled;
}

MyMoneySplit::MyMoneySplit(const QDomElement& node) :
    MyMoneyObject(*new MyMoneySplitPrivate, node, false),
    MyMoneyKeyValueContainer(node.elementsByTagName(MyMoneySplitPrivate::getElName(Split::Element::KeyValuePairs)).item(0).toElement())
{
  Q_D(MyMoneySplit);
  if (d->getElName(Split::Element::Split) != node.tagName())
    throw MYMONEYEXCEPTION("Node was not SPLIT");

  clearId();

  d->m_payee = MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::Payee)));

  QDomNodeList nodeList = node.elementsByTagName(d->getElName(Split::Element::Tag));
  for (int i = 0; i < nodeList.count(); i++)
    d->m_tagList << MyMoneyUtils::QStringEmpty(nodeList.item(i).toElement().attribute(d->getAttrName(Split::Attribute::ID)));

  d->m_reconcileDate = MyMoneyUtils::stringToDate(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::ReconcileDate))));
  d->m_action = MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::Action)));
  d->m_reconcileFlag = static_cast<eMyMoney::Split::State>(node.attribute(d->getAttrName(Split::Attribute::ReconcileFlag)).toInt());
  d->m_memo = MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::Memo)));
  d->m_value = MyMoneyMoney(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::Value))));
  d->m_shares = MyMoneyMoney(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::Shares))));
  d->m_price = MyMoneyMoney(MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::Price))));
  d->m_account = MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::Account)));
  d->m_costCenter = MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::CostCenter)));
  d->m_number = MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::Number)));
  d->m_bankID = MyMoneyUtils::QStringEmpty(node.attribute(d->getAttrName(Split::Attribute::BankID)));
}

MyMoneySplit::MyMoneySplit(const MyMoneySplit& other) :
  MyMoneyObject(*new MyMoneySplitPrivate(*other.d_func()), other.id()),
  MyMoneyKeyValueContainer(other)
{
}

MyMoneySplit::MyMoneySplit(const QString& id, const MyMoneySplit& other) :
  MyMoneyObject(*new MyMoneySplitPrivate(*other.d_func()), id),
  MyMoneyKeyValueContainer(other)
{
}

MyMoneySplit::~MyMoneySplit()
{
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
  Q_D(const MyMoneySplit);
  auto el = document.createElement(d->getElName(Split::Element::Split));

  d->writeBaseXML(document, el);

  el.setAttribute(d->getAttrName(Split::Attribute::Payee), d->m_payee);
  //el.setAttribute(getAttrName(Split::Attribute::Tag), m_tag);
  el.setAttribute(d->getAttrName(Split::Attribute::ReconcileDate), MyMoneyUtils::dateToString(d->m_reconcileDate));
  el.setAttribute(d->getAttrName(Split::Attribute::Action), d->m_action);
  el.setAttribute(d->getAttrName(Split::Attribute::ReconcileFlag), (int)d->m_reconcileFlag);
  el.setAttribute(d->getAttrName(Split::Attribute::Value), d->m_value.toString());
  el.setAttribute(d->getAttrName(Split::Attribute::Shares), d->m_shares.toString());
  if (!d->m_price.isZero())
    el.setAttribute(d->getAttrName(Split::Attribute::Price), d->m_price.toString());
  el.setAttribute(d->getAttrName(Split::Attribute::Memo), d->m_memo);
  // No need to write the split id as it will be re-assigned when the file is read
  // el.setAttribute(getAttrName(Split::Attribute::ID), split.id());
  el.setAttribute(d->getAttrName(Split::Attribute::Account), d->m_account);
  el.setAttribute(d->getAttrName(Split::Attribute::Number), d->m_number);
  el.setAttribute(d->getAttrName(Split::Attribute::BankID), d->m_bankID);
  if(!d->m_costCenter.isEmpty())
    el.setAttribute(d->getAttrName(Split::Attribute::CostCenter), d->m_costCenter);

  for (int i = 0; i < d->m_tagList.count(); i++) {
    QDomElement sel = document.createElement(d->getElName(Split::Element::Tag));
    sel.setAttribute(d->getAttrName(Split::Attribute::ID), d->m_tagList[i]);
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
  Q_D(const MyMoneySplit);
  return !(value(d->getAttrName(Split::Attribute::KMMatchedTx)).isEmpty());
}

void MyMoneySplit::addMatch(const MyMoneyTransaction& _t)
{
  Q_D(MyMoneySplit);
  //  now we allow matching of two manual transactions
  if (!isMatched()) {
    MyMoneyTransaction t(_t);
    t.clearId();
    QDomDocument doc(d->getElName(Split::Element::Match));
    QDomElement el = doc.createElement(d->getElName(Split::Element::Container));
    doc.appendChild(el);
    t.writeXML(doc, el);
    QString xml = doc.toString();
    xml.replace('<', "&lt;");
    setValue(d->getAttrName(Split::Attribute::KMMatchedTx), xml);
  }
}

void MyMoneySplit::removeMatch()
{
  Q_D(MyMoneySplit);
  deletePair(d->getAttrName(Split::Attribute::KMMatchedTx));
}

MyMoneyTransaction MyMoneySplit::matchedTransaction() const
{
  Q_D(const MyMoneySplit);
  auto xml = value(d->getAttrName(Split::Attribute::KMMatchedTx));
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
