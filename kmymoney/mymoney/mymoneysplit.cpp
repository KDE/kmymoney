/*
 * Copyright 2002-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004       Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2005-2006  Ace Jones <acejones@users.sourceforge.net>
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

#include "mymoneysplit.h"
#include "mymoneysplit_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneytransaction.h"
#include "mymoneyexception.h"

MyMoneySplit::MyMoneySplit() :
  MyMoneyObject(*new MyMoneySplitPrivate)
{
}

MyMoneySplit::MyMoneySplit(const QString &id) :
    MyMoneyObject(*new MyMoneySplitPrivate, id)
{
  Q_D(MyMoneySplit);
  d->m_reconcileFlag = eMyMoney::Split::State::NotReconciled;
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

void MyMoneySplit::negateValue()
{
    Q_D(MyMoneySplit);
    d->m_value = d->m_value * MyMoneyMoney::MINUS_ONE;
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
      setAction(actionName(Split::Action::BuyShares));
      break;
    case eMyMoney::Split::InvestmentTransactionType::Dividend:
      setAction(actionName(Split::Action::Dividend));
      break;
    case eMyMoney::Split::InvestmentTransactionType::Yield:
      setAction(actionName(Split::Action::Yield));
      break;
    case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
      setAction(actionName(Split::Action::ReinvestDividend));
      break;
    case eMyMoney::Split::InvestmentTransactionType::AddShares:
    case eMyMoney::Split::InvestmentTransactionType::RemoveShares:
      setAction(actionName(Split::Action::AddShares));
      break;
    case eMyMoney::Split::InvestmentTransactionType::SplitShares:
      setAction(actionName(Split::Action::SplitShares));
      break;
    case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
      setAction(actionName(Split::Action::InterestIncome));
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
  return d->m_action == actionName(Split::Action::Amortization);
}

bool MyMoneySplit::isInterestSplit() const
{
  Q_D(const MyMoneySplit);
  return d->m_action == actionName(Split::Action::Interest);
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
  return d->m_isMatched;
}

void MyMoneySplit::addMatch(const MyMoneyTransaction& _t)
{
  Q_D(MyMoneySplit);
  //  now we allow matching of two manual transactions
  d->m_matchedTransaction = _t;
  d->m_matchedTransaction.clearId();
  d->m_isMatched = true;
}

void MyMoneySplit::removeMatch()
{
  Q_D(MyMoneySplit);
  d->m_matchedTransaction = MyMoneyTransaction();
  d->m_isMatched = false;
}

MyMoneyTransaction MyMoneySplit::matchedTransaction() const
{
  Q_D(const MyMoneySplit);
  if (d->m_isMatched)
    return d->m_matchedTransaction;

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

QString MyMoneySplit::actionName(Split::Action action)
{
  static const QHash<Split::Action, QString> actionNames {
    {Split::Action::Check,            QStringLiteral("Check")},
    {Split::Action::Deposit,          QStringLiteral("Deposit")},
    {Split::Action::Transfer,         QStringLiteral("Transfer")},
    {Split::Action::Withdrawal,       QStringLiteral("Withdrawal")},
    {Split::Action::ATM,              QStringLiteral("ATM")},
    {Split::Action::Amortization,     QStringLiteral("Amortization")},
    {Split::Action::Interest,         QStringLiteral("Interest")},
    {Split::Action::BuyShares,        QStringLiteral("Buy")},
    {Split::Action::Dividend,         QStringLiteral("Dividend")},
    {Split::Action::ReinvestDividend, QStringLiteral("Reinvest")},
    {Split::Action::Yield,            QStringLiteral("Yield")},
    {Split::Action::AddShares,        QStringLiteral("Add")},
    {Split::Action::SplitShares,      QStringLiteral("Split")},
    {Split::Action::InterestIncome,   QStringLiteral("IntIncome")},
  };
  return actionNames[action];
}
