/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2001 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneytransaction.h"
#include "mymoneytransaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>
#include <QMap>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

MyMoneyTransaction::MyMoneyTransaction() :
    MyMoneyObject(*new MyMoneyTransactionPrivate)
{
}

MyMoneyTransaction::MyMoneyTransaction(const QString &id) :
    MyMoneyObject(*new MyMoneyTransactionPrivate, id)
{
}

MyMoneyTransaction::MyMoneyTransaction(const MyMoneyTransaction& other) :
    MyMoneyObject(*new MyMoneyTransactionPrivate(*other.d_func()), other.id()),
    MyMoneyKeyValueContainer(other)
{
}

MyMoneyTransaction::MyMoneyTransaction(const QString& id, const MyMoneyTransaction& other) :
    MyMoneyObject(*new MyMoneyTransactionPrivate(*other.d_func()), id),
    MyMoneyKeyValueContainer(other)
{
    Q_D(MyMoneyTransaction);
    if (d->m_entryDate == QDate())
        d->m_entryDate = QDate::currentDate();

    for (auto& split : d->m_splits)
        split.setTransactionId(id);
}

MyMoneyTransaction::~MyMoneyTransaction()
{
}

QDate MyMoneyTransaction::entryDate() const
{
    Q_D(const MyMoneyTransaction);
    return d->m_entryDate;
}

void MyMoneyTransaction::setEntryDate(const QDate& date)
{
    Q_D(MyMoneyTransaction);
    d->m_entryDate = date;
}

QDate MyMoneyTransaction::postDate() const
{
    Q_D(const MyMoneyTransaction);
    return d->m_postDate;
}

void MyMoneyTransaction::setPostDate(const QDate& date)
{
    Q_D(MyMoneyTransaction);
    d->m_postDate = date;
}

QString MyMoneyTransaction::memo() const
{
    Q_D(const MyMoneyTransaction);
    return d->m_memo;
}

void MyMoneyTransaction::setMemo(const QString& memo)
{
    Q_D(MyMoneyTransaction);
    d->m_memo = memo;
}

QList<MyMoneySplit> MyMoneyTransaction::splits() const
{
    Q_D(const MyMoneyTransaction);
    return d->m_splits;
}

QList<MyMoneySplit>& MyMoneyTransaction::splits()
{
    Q_D(MyMoneyTransaction);
    return d->m_splits;
}

MyMoneySplit MyMoneyTransaction::firstSplit() const
{
    Q_D(const MyMoneyTransaction);
    return d->m_splits.first();
}

uint MyMoneyTransaction::splitCount() const
{
    Q_D(const MyMoneyTransaction);
    return d->m_splits.count();
}

uint MyMoneyTransaction::splitCountWithValue() const
{
    uint rc = 0;
    Q_D(const MyMoneyTransaction);
    const auto splitCount = d->m_splits.count();
    for (int split = 0; split < splitCount; ++split) {
        rc += (d->m_splits[split].value().isZero() ? 0 : 1);
    }
    return rc;
}

QString MyMoneyTransaction::commodity() const
{
    Q_D(const MyMoneyTransaction);
    return d->m_commodity;
}

void MyMoneyTransaction::setCommodity(const QString& commodityId)
{
    Q_D(MyMoneyTransaction);
    d->m_commodity = commodityId;
    d->clearReferences();
}

QString MyMoneyTransaction::bankID() const
{
    Q_D(const MyMoneyTransaction);
    return d->m_bankID;
}

void MyMoneyTransaction::setBankID(const QString& bankID)
{
    Q_D(MyMoneyTransaction);
    d->m_bankID = bankID;
}

bool MyMoneyTransaction::operator == (const MyMoneyTransaction& right) const
{
    Q_D(const MyMoneyTransaction);
    auto d2 = static_cast<const MyMoneyTransactionPrivate *>(right.d_func());
    return (MyMoneyObject::operator==(right)
            && MyMoneyKeyValueContainer::operator==(right) //
            && (d->m_commodity == d2->m_commodity) //
            && ((d->m_memo.length() == 0 && d2->m_memo.length() == 0) || (d->m_memo == d2->m_memo))  //
            && (d->m_splits == d2->m_splits) //
            && (d->m_entryDate == d2->m_entryDate) //
            && (d->m_postDate == d2->m_postDate));
}

bool MyMoneyTransaction::operator != (const MyMoneyTransaction& r) const
{
    return !(*this == r);
}

bool MyMoneyTransaction::operator< (const MyMoneyTransaction& r) const
{
    return postDate() < r.postDate();
}

bool MyMoneyTransaction::operator<= (const MyMoneyTransaction& r) const
{
    return postDate() <= r.postDate();
}

bool MyMoneyTransaction::operator> (const MyMoneyTransaction& r) const
{
    return postDate() > r.postDate();
}

bool MyMoneyTransaction::accountReferenced(const QString& id) const
{
    Q_D(const MyMoneyTransaction);

    for (const auto& split : d->m_splits) {
        if (split.accountId() == id)
            return true;
    }
    return false;
}

void MyMoneyTransaction::addSplit(MyMoneySplit &split)
{
    if (!split.id().isEmpty())
        throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot add split with assigned id '%1' to transaction %2").arg(split.id(), id()));

    if (split.accountId().isEmpty())
        throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot add split that does not contain an account reference to transaction %1").arg(id()));

    Q_D(MyMoneyTransaction);
    MyMoneySplit newSplit(d->nextSplitID(), split);
    split = newSplit;
    split.setTransactionId(id());
    d->m_splits.append(split);
    d->clearReferences();
}

void MyMoneyTransaction::modifySplit(const MyMoneySplit& split)
{
// This is the other version which allows having more splits referencing
// the same account.
    if (split.accountId().isEmpty())
        throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot modify split that does not contain an account reference in transaction %1").arg(id()));

    Q_D(MyMoneyTransaction);
    for (auto& it_split : d->m_splits) {
        if (split.id() == it_split.id()) {
            it_split = split;
            d->clearReferences();
            return;
        }
    }
    throw MYMONEYEXCEPTION(QString::fromLatin1("Invalid split id '%1' in transaction %2").arg(split.id(), id()));
}

void MyMoneyTransaction::removeSplit(const MyMoneySplit& split)
{
    Q_D(MyMoneyTransaction);
    for (int end = d->m_splits.size(), i = 0; i < end; ++i) {
        if (split.id() == d->m_splits.at(i).id()) {
            d->m_splits.removeAt(i);
            d->clearReferences();
            return;
        }
    }

    throw MYMONEYEXCEPTION(QString::fromLatin1("Invalid split id '%1' in transaction %2").arg(split.id(), id()));
}

void MyMoneyTransaction::removeSplits()
{
    Q_D(MyMoneyTransaction);
    d->m_splits.clear();
    d->clearReferences();
}

MyMoneySplit MyMoneyTransaction::splitByPayee(const QString& payeeId) const
{
    Q_D(const MyMoneyTransaction);
    for (const auto& split : d->m_splits) {
        if (split.payeeId() == payeeId)
            return split;
    }
    throw MYMONEYEXCEPTION(QString::fromLatin1("Split not found for payee '%1' in transaction %2").arg(payeeId, id()));
}

MyMoneySplit MyMoneyTransaction::splitByAccount(const QString& accountId, const bool match) const
{
    Q_D(const MyMoneyTransaction);
    for (const auto& split : d->m_splits) {
        if ((match == true && split.accountId() == accountId) ||
                (match == false && split.accountId() != accountId))
            return split;
    }
    throw MYMONEYEXCEPTION(QString::fromLatin1("Split not found for account %1%2 in transaction %3").arg(match ? "" : "!", accountId, id()));
}

MyMoneySplit MyMoneyTransaction::splitByAccount(const QStringList& accountIds, const bool match) const
{
    Q_D(const MyMoneyTransaction);
    for (const auto& split : d->m_splits) {
        if ((match == true && accountIds.contains(split.accountId())) ||
                (match == false && !accountIds.contains(split.accountId())))
            return split;
    }
    throw MYMONEYEXCEPTION(
        QString::fromLatin1("Split not found for account  %1%2...%3 in transaction %4").arg(match ? "" : "!", accountIds.front(), accountIds.back(), id()));
}

MyMoneySplit MyMoneyTransaction::splitById(const QString& splitId) const
{
    Q_D(const MyMoneyTransaction);
    for (const auto& split : d->m_splits) {
        if (split.id() == splitId)
            return split;
    }
    throw MYMONEYEXCEPTION(QString::fromLatin1("Split not found for id '%1' in transaction %2").arg(splitId, id()));
}

QString MyMoneyTransaction::firstSplitID()
{
    QString id;
    id = 'S' + id.setNum(1).rightJustified(MyMoneyTransactionPrivate::SPLIT_ID_SIZE, '0');
    return id;
}

MyMoneyMoney MyMoneyTransaction::splitSum() const
{
    MyMoneyMoney result;

    Q_D(const MyMoneyTransaction);
    for (const auto& split : d->m_splits)
        result += split.value();
    return result;
}

void MyMoneyTransaction::reverse()
{
    Q_D(MyMoneyTransaction);
    for (MyMoneySplit& split : d->m_splits) {
        split.negateValue();
        split.negateShares();
    }
}

bool MyMoneyTransaction::isLoanPayment() const
{
    try {

        Q_D(const MyMoneyTransaction);
        for (const auto& split : d->m_splits) {
            if (split.isAmortizationSplit())
                return true;
        }
    } catch (const MyMoneyException &) {
    }
    return false;
}

MyMoneySplit MyMoneyTransaction::amortizationSplit() const
{
    static MyMoneySplit nullSplit;

    Q_D(const MyMoneyTransaction);
    for (const auto& split : d->m_splits) {
        if (split.isAmortizationSplit() && split.isAutoCalc())
            return split;
    }
    return nullSplit;
}

MyMoneySplit MyMoneyTransaction::interestSplit() const
{
    static MyMoneySplit nullSplit;

    Q_D(const MyMoneyTransaction);
    for (const auto& split : d->m_splits) {
        if (split.isInterestSplit() && split.isAutoCalc())
            return split;
    }
    return nullSplit;
}

unsigned long MyMoneyTransaction::hash(const QString& txt, unsigned long h)
{
    unsigned long g;

    for (int i = 0; i < txt.length(); ++i) {
        unsigned short uc = txt[i].unicode();
        for (unsigned j = 0; j < 2; ++j) {
            unsigned char c = uc & 0xff;
            // if either the cell or the row of the Unicode char is 0, stop processing
            if (!c)
                break;
            h = (h << 4) + c;
            if ((g = (h & 0xf0000000))) {
                h = h ^(g >> 24);
                h = h ^ g;
            }
            uc >>= 8;
        }
    }
    return h;
}

bool MyMoneyTransaction::isStockSplit() const
{
    Q_D(const MyMoneyTransaction);
    return (d->m_splits.count() == 1 && d->m_splits.first().action() == MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares));
}

bool MyMoneyTransaction::isImported() const
{
    return value("Imported").toLower() == QString("true");
}

void MyMoneyTransaction::setImported(bool state)
{
    if (state)
        setValue("Imported", "true");
    else
        deletePair("Imported");
}

bool MyMoneyTransaction::hasAutoCalcSplit() const
{
    Q_D(const MyMoneyTransaction);

    for (const auto& split : d->m_splits)
        if (split.isAutoCalc())
            return true;
    return false;
}

QString MyMoneyTransaction::accountSignature(bool includeSplitCount) const
{
    Q_D(const MyMoneyTransaction);
    QMap<QString, int> accountList;
    for (const auto& split : d->m_splits)
        accountList[split.accountId()] += 1;

    QMap<QString, int>::const_iterator it_a;
    QString rc;
    for (it_a = accountList.cbegin(); it_a != accountList.cend(); ++it_a) {
        if (it_a != accountList.cbegin())
            rc += '-';
        rc += it_a.key();
        if (includeSplitCount)
            rc += QString("*%1").arg(*it_a);
    }
    return rc;
}

QString MyMoneyTransaction::uniqueSortKey() const
{
    Q_D(const MyMoneyTransaction);
    return uniqueSortKey(postDate(), d->m_id);
}

QString MyMoneyTransaction::uniqueSortKey(const QDate& date, const QString& id)
{
    QString year, month, day;
    year = year.setNum(date.year()).rightJustified(MyMoneyTransactionPrivate::YEAR_SIZE, '0');
    month = month.setNum(date.month()).rightJustified(MyMoneyTransactionPrivate::MONTH_SIZE, '0');
    day = day.setNum(date.day()).rightJustified(MyMoneyTransactionPrivate::DAY_SIZE, '0');
    const auto key = QString::fromLatin1("%1-%2-%3-%4").arg(year, month, day, id);
    return key;
}

bool MyMoneyTransaction::replaceId(const QString& newId, const QString& oldId)
{
    auto changed = false;
    Q_D(MyMoneyTransaction);
    for (MyMoneySplit& split : d->m_splits)
        changed |= split.replaceId(newId, oldId);

    if (changed) {
        d->clearReferences();
    }

    return changed;
}
