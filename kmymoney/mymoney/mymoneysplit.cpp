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

MyMoneySplit::MyMoneySplit()
{
  m_reconcileFlag = NotReconciled;
}

MyMoneySplit::MyMoneySplit(const QDomElement& node) :
    MyMoneyObject(node, false),
    MyMoneyKeyValueContainer(node.elementsByTagName("KEYVALUEPAIRS").item(0).toElement())
{
  if ("SPLIT" != node.tagName())
    throw new MYMONEYEXCEPTION("Node was not SPLIT");

  clearId();

  m_payee = QStringEmpty(node.attribute("payee"));
  m_reconcileDate = stringToDate(QStringEmpty(node.attribute("reconciledate")));
  m_action = QStringEmpty(node.attribute("action"));
  m_reconcileFlag = static_cast<MyMoneySplit::reconcileFlagE>(node.attribute("reconcileflag").toInt());
  m_memo = QStringEmpty(node.attribute("memo"));
  m_value = MyMoneyMoney(QStringEmpty(node.attribute("value")));
  m_shares = MyMoneyMoney(QStringEmpty(node.attribute("shares")));
  m_price = MyMoneyMoney(QStringEmpty(node.attribute("price")));
  m_account = QStringEmpty(node.attribute("account"));
  m_number = QStringEmpty(node.attribute("number"));
  m_bankID = QStringEmpty(node.attribute("bankid"));
}

MyMoneySplit::MyMoneySplit(const QString& id, const MyMoneySplit& right) :
    MyMoneyObject(id)
{
  *this = right;
  setId(id);
}

MyMoneySplit::~MyMoneySplit()
{
}

bool MyMoneySplit::operator == (const MyMoneySplit& right) const
{
  return MyMoneyObject::operator==(right) &&
         MyMoneyKeyValueContainer::operator==(right) &&
         m_account == right.m_account &&
         m_payee == right.m_payee &&
         m_memo == right.m_memo &&
         m_action == right.m_action &&
         m_reconcileDate == right.m_reconcileDate &&
         m_reconcileFlag == right.m_reconcileFlag &&
         ((m_number.length() == 0 && right.m_number.length() == 0) || m_number == right.m_number) &&
         m_shares == right.m_shares &&
         m_value == right.m_value &&
         m_price == right.m_price &&
         m_transactionId == right.m_transactionId;
}

void MyMoneySplit::setAccountId(const QString& account)
{
  m_account = account;
}

void MyMoneySplit::setMemo(const QString& memo)
{
  m_memo = memo;
}

void MyMoneySplit::setReconcileDate(const QDate& date)
{
  m_reconcileDate = date;
}

void MyMoneySplit::setReconcileFlag(const reconcileFlagE flag)
{
  m_reconcileFlag = flag;
}

void MyMoneySplit::setShares(const MyMoneyMoney& shares)
{
  m_shares = shares;
}

void MyMoneySplit::setValue(const MyMoneyMoney& value)
{
  m_value = value;
}

void MyMoneySplit::setValue(const MyMoneyMoney& value, const QString& transactionCurrencyId, const QString& splitCurrencyId)
{
  if (transactionCurrencyId == splitCurrencyId)
    setValue(value);
  else
    setShares(value);
}

void MyMoneySplit::setPayeeId(const QString& payee)
{
  m_payee = payee;
}

void MyMoneySplit::setAction(investTransactionTypeE type)
{
  switch (type) {
    case BuyShares:
    case SellShares:
      setAction(ActionBuyShares);
      break;
    case Dividend:
      setAction(ActionDividend);
      break;
    case Yield:
      setAction(ActionYield);
      break;
    case ReinvestDividend:
      setAction(ActionReinvestDividend);
      break;
    case AddShares:
    case RemoveShares:
      setAction(ActionAddShares);
      break;
    case MyMoneySplit::SplitShares:
      setAction(ActionSplitShares);
      break;
    case MyMoneySplit::UnknownTransactionType:
      break;
  }
}

void MyMoneySplit::setAction(const QString& action)
{
  m_action = action;
}

void MyMoneySplit::setNumber(const QString& number)
{
  m_number = number;
}

const MyMoneyMoney MyMoneySplit::value(const QString& transactionCurrencyId, const QString& splitCurrencyId) const
{
  return (transactionCurrencyId == splitCurrencyId) ? m_value : m_shares;
}

void MyMoneySplit::setPrice(const MyMoneyMoney& price)
{
  m_price = price;
}

MyMoneyMoney MyMoneySplit::price(void) const
{
  if (!m_price.isZero())
    return m_price;
  if (!m_value.isZero() && !m_shares.isZero())
    return m_value / m_shares;
  return MyMoneyMoney(1, 1);
}

void MyMoneySplit::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("SPLIT");

  writeBaseXML(document, el);

  el.setAttribute("payee", m_payee);
  el.setAttribute("reconciledate", dateToString(m_reconcileDate));
  el.setAttribute("action", m_action);
  el.setAttribute("reconcileflag", m_reconcileFlag);
  el.setAttribute("value", m_value.toString());
  el.setAttribute("shares", m_shares.toString());
  if (!m_price.isZero())
    el.setAttribute("price", m_price.toString());
  el.setAttribute("memo", m_memo);
  // No need to write the split id as it will be re-assigned when the file is read
  // el.setAttribute("id", split.id());
  el.setAttribute("account", m_account);
  el.setAttribute("number", m_number);
  el.setAttribute("bankid", m_bankID);

  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneySplit::hasReferenceTo(const QString& id) const
{
  bool rc = false;
  if (isMatched()) {
    rc = matchedTransaction().hasReferenceTo(id);
  }
  return rc || (id == m_account) || (id == m_payee);
}

bool MyMoneySplit::isMatched(void) const
{
  return !(value("kmm-matched-tx").isEmpty());
}

void MyMoneySplit::addMatch(const MyMoneyTransaction& _t)
{
  if (_t.isImported() && !isMatched()) {
    MyMoneyTransaction t(_t);
    t.clearId();
    QDomDocument doc("MATCH");
    QDomElement el = doc.createElement("CONTAINER");
    doc.appendChild(el);
    t.writeXML(doc, el);
    QString xml = doc.toString();
    xml.replace('<', "&lt;");
    setValue("kmm-matched-tx", xml);
  }
}

void MyMoneySplit::removeMatch(void)
{
  deletePair("kmm-matched-tx");
}

MyMoneyTransaction MyMoneySplit::matchedTransaction(void) const
{
  QString xml = value("kmm-matched-tx");
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
  bool changed = false;

  if (m_payee == oldId) {
    m_payee = newId;
    changed = true;
  } else if (m_account == oldId) {
    m_account = newId;
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


// vim:cin:si:ai:et:ts=2:sw=2:
