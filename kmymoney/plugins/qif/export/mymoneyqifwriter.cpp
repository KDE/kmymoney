/*
    SPDX-FileCopyrightText: 2000-2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Allan Anderson <agander93@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyqifwriter.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QFile>
#include <QList>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kmessagebox.h>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneysplit.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

MyMoneyQifWriter::MyMoneyQifWriter()
{
}

MyMoneyQifWriter::~MyMoneyQifWriter()
{
}

void MyMoneyQifWriter::write(const QString& filename, const QString& profile,
                             const QString& accountId, const bool accountData,
                             const bool categoryData,
                             const QDate& startDate, const QDate& endDate)
{
    m_qifProfile.loadProfile(profile);

    QFile qifFile(filename);
    if (qifFile.open(QIODevice::WriteOnly)) {
        QTextStream s(&qifFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        s.setCodec("UTF-8");
#endif

        try {
            if (categoryData) {
                writeCategoryEntries(s);
            }

            if (accountData) {
                writeAccountEntry(s, accountId, startDate, endDate);
            }
            Q_EMIT signalProgress(-1, -1);

        } catch (const MyMoneyException &e) {
            KMessageBox::error(nullptr, i18n("Unexpected exception '%1'", QString::fromLatin1(e.what())));
        }

        qifFile.close();
        qDebug() << "Export completed.\n";
    } else {
        KMessageBox::error(0, i18n("Unable to open file '%1' for writing", filename));
    }
}

void MyMoneyQifWriter::writeAccountEntry(QTextStream& s, const QString& accountId, const QDate& startDate, const QDate& endDate)
{
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount account;

    account = file->account(accountId);
    MyMoneyTransactionFilter filter(accountId);

    QString openingBalanceTransactionId;
    QString type = m_qifProfile.profileType();

    s << "!Type:" << type << Qt::endl;
    if (type == "Invst") {
        extractInvestmentEntries(s, accountId, startDate, endDate);
    } else {
        filter.setDateFilter(startDate, endDate);
        QList<MyMoneyTransaction> list;
        file->transactionList(list, filter);
        if (!startDate.isValid() || startDate <= account.openingDate()) {
            s << "D" << m_qifProfile.date(account.openingDate()) << Qt::endl;
            openingBalanceTransactionId = file->openingBalanceTransaction(account);
            MyMoneySplit split;
            if (!openingBalanceTransactionId.isEmpty()) {
                MyMoneyTransaction openingBalanceTransaction = file->transaction(openingBalanceTransactionId);
                split = openingBalanceTransaction.splitByAccount(account.id(), true /* match */);
            }
            s << "T" << m_qifProfile.value('T', split.value()) << Qt::endl;
        } else {
            s << "D" << m_qifProfile.date(startDate) << Qt::endl;
            s << "T" << m_qifProfile.value('T', file->balance(accountId, startDate.addDays(-1))) << Qt::endl;
        }
        s << "CX" << Qt::endl;
        s << "P" << m_qifProfile.openingBalanceText() << Qt::endl;
        s << "L";
        if (m_qifProfile.accountDelimiter().length())
            s << m_qifProfile.accountDelimiter().at(0);
        s << account.name();
        if (m_qifProfile.accountDelimiter().length() > 1)
            s << m_qifProfile.accountDelimiter().at(1);
        s << Qt::endl;
        s << "^" << Qt::endl;

        QList<MyMoneyTransaction>::ConstIterator it;
        Q_EMIT signalProgress(0, list.count());
        int count = 0;
        for (it = list.constBegin(); it != list.constEnd(); ++it) {
            // don't include the openingBalanceTransaction again
            if ((*it).id() != openingBalanceTransactionId)
                writeTransactionEntry(s, *it, accountId);
            Q_EMIT signalProgress(++count, 0);
        }
    }
}

void MyMoneyQifWriter::writeCategoryEntries(QTextStream &s)
{
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount income;
    MyMoneyAccount expense;

    income = file->income();
    expense = file->expense();

    s << "!Type:Cat" << Qt::endl;
    QStringList list = income.accountList() + expense.accountList();
    Q_EMIT signalProgress(0, list.count());
    QStringList::Iterator it;
    int count = 0;
    for (it = list.begin(); it != list.end(); ++it) {
        writeCategoryEntry(s, *it, "");
        Q_EMIT signalProgress(++count, 0);
    }
}

void MyMoneyQifWriter::writeCategoryEntry(QTextStream &s, const QString& accountId, const QString& leadIn)
{
    MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
    QString name = acc.name();

    s << "N" << leadIn << name << Qt::endl;
    s << (acc.accountGroup() == eMyMoney::Account::Type::Expense ? "E" : "I") << Qt::endl;
    s << "^" << Qt::endl;

    QStringList list = acc.accountList();
    QStringList::Iterator it;
    name += ':';
    for (it = list.begin(); it != list.end(); ++it) {
        writeCategoryEntry(s, *it, name);
    }
}

void MyMoneyQifWriter::writeTransactionEntry(QTextStream &s, const MyMoneyTransaction& t, const QString& accountId)
{
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneySplit split = t.splitByAccount(accountId);

    s << "D" << m_qifProfile.date(t.postDate()) << Qt::endl;

    switch (split.reconcileFlag()) {
    case eMyMoney::Split::State::Cleared:
        s << "C*" << Qt::endl;
        break;

    case eMyMoney::Split::State::Reconciled:
    case eMyMoney::Split::State::Frozen:
        s << "CX" << Qt::endl;
        break;

    default:
        break;
    }

    if (split.memo().length() > 0) {
        QString m = split.memo();
        m.replace('\n', "\\n");
        s << "M" << m << Qt::endl;
    }

    s << "T" << m_qifProfile.value('T', split.value()) << Qt::endl;

    if (split.number().length() > 0)
        s << "N" << split.number() << Qt::endl;

    if (!split.payeeId().isEmpty()) {
        MyMoneyPayee payee = file->payee(split.payeeId());
        s << "P" << payee.name() << Qt::endl;
    }

    QList<MyMoneySplit> list = t.splits();
    if (list.count() > 1) {
        MyMoneySplit sp = t.splitByAccount(accountId, false);
        MyMoneyAccount acc = file->account(sp.accountId());
        if (acc.accountGroup() != eMyMoney::Account::Type::Income
                && acc.accountGroup() != eMyMoney::Account::Type::Expense) {
            s << "L" << m_qifProfile.accountDelimiter().at(0) << MyMoneyFile::instance()->accountToCategory(sp.accountId())
              << m_qifProfile.accountDelimiter().at(1) << Qt::endl;
        } else {
            s << "L" << file->accountToCategory(sp.accountId()) << Qt::endl;
        }
        if (list.count() > 2) {
            QList<MyMoneySplit>::ConstIterator it;
            for (it = list.constBegin(); it != list.constEnd(); ++it) {
                if (!((*it) == split)) {
                    writeSplitEntry(s, *it);
                }
            }
        }
    }
    s << "^" << Qt::endl;
}

void MyMoneyQifWriter::writeSplitEntry(QTextStream& s, const MyMoneySplit& split)
{
    MyMoneyFile* file = MyMoneyFile::instance();

    s << "S";
    MyMoneyAccount acc = file->account(split.accountId());
    if (acc.accountGroup() != eMyMoney::Account::Type::Income
            && acc.accountGroup() != eMyMoney::Account::Type::Expense) {
        s << m_qifProfile.accountDelimiter().at(0) << file->accountToCategory(split.accountId()) << m_qifProfile.accountDelimiter().at(1);
    } else {
        s << file->accountToCategory(split.accountId());
    }
    s << Qt::endl;

    if (split.memo().length() > 0) {
        QString m = split.memo();
        m.replace('\n', "\\n");
        s << "E" << m << Qt::endl;
    }

    s << "$" << m_qifProfile.value('$', -split.value()) << Qt::endl;
}

void MyMoneyQifWriter::extractInvestmentEntries(QTextStream &s, const QString& accountId, const QDate& startDate, const QDate& endDate)
{
    MyMoneyFile* file = MyMoneyFile::instance();
    QList<QString> accList = file->account(accountId).accountList();
    QList<QString>::ConstIterator itAcc;
    for (itAcc = accList.constBegin(); itAcc != accList.constEnd(); ++itAcc) {
        MyMoneyTransactionFilter filter((*itAcc));
        filter.setDateFilter(startDate, endDate);
        QList<MyMoneyTransaction> list;
        file->transactionList(list, filter);
        QList<MyMoneyTransaction>::ConstIterator it;
        Q_EMIT signalProgress(0, list.count());
        int count = 0;
        for (it = list.constBegin(); it != list.constEnd(); ++it) {
            writeInvestmentEntry(s, *it, ++count);
            Q_EMIT signalProgress(count, 0);
        }
    }
}

void MyMoneyQifWriter::writeInvestmentEntry(QTextStream& stream, const MyMoneyTransaction& t, const int count)
{
    QString s;
    QString memo;
    MyMoneyFile* file = MyMoneyFile::instance();
    QString chkAccnt;
    bool isXfer = false;
    QList<MyMoneySplit> lst = t.splits();
    QList<MyMoneySplit>::Iterator it;
    eMyMoney::Account::Type typ;
    QString chkAccntId;
    MyMoneyMoney qty;
    MyMoneyMoney value;
    QMap<eMyMoney::Account::Type, QString> map;

    for (int i = 0; i < lst.count(); i++) {
        MyMoneyAccount acc = file->account(lst[i].accountId());
        typ = acc.accountType();
        map.insert(typ, lst[i].accountId());
        if (typ == eMyMoney::Account::Type::Stock) {
            memo = lst[i].memo();
        }
    }
    //
    //  Add date.
    //
    s += 'D' + m_qifProfile.date(t.postDate()) + '\n';
    for (it = lst.begin(); it != lst.end(); ++it) {
        QString accName;
        QString actionType = (*it).action();
        MyMoneyAccount acc = file->account((*it).accountId());
        //
        //  eMyMoney::Account::Type::Checkings.
        //
        if ((acc.accountType() == eMyMoney::Account::Type::Checkings) || (acc.accountType() == eMyMoney::Account::Type::Cash)) {
            chkAccntId = (*it).accountId();
            chkAccnt = file->account(chkAccntId).name();
        } else if (acc.accountType() == eMyMoney::Account::Type::Income) {
            //
            //  eMyMoney::Account::Type::Income.
            //
        } else if (acc.accountType() == eMyMoney::Account::Type::Expense) {
            //
            //  eMyMoney::Account::Type::Expense.
            //
        } else if (acc.accountType() == eMyMoney::Account::Type::Stock) {
            //
            //  eMyMoney::Account::Type::Stock.
            //
            qty = (*it).shares();
            value = (*it).value();

            accName = acc.name();
            if ((actionType == "Dividend") || (actionType == "Buy") || (actionType == "IntIncome")) {
                isXfer = true;
            }
            //
            //  Actions.
            //
            QString action;
            if ((*it).action() == "Dividend") {
                action = "DivX";
            } else if ((*it).action() == "IntIncome") {
                action = "IntIncX";
            }
            if ((action == "DivX") || (action == "IntIncX")) {
                if (map.value(eMyMoney::Account::Type::Checkings).isEmpty()) {
                    KMessageBox::error(0,
                                       QString("<qt>%1</qt>").arg(i18n("Transaction number <b>%1</b> is missing an account assignment.\nTransaction dropped.", count)),
                                       i18n("Invalid transaction"));
                    return;
                }
                MyMoneySplit sp = t.splitByAccount(map.value(eMyMoney::Account::Type::Checkings), true);
                QString txt = sp.value().formatMoney("", 2);
                s += 'T' + txt + '\n';
            } else if ((*it).action() == "Buy") {

                if (qty.isNegative()) {
                    action = "Sell";
                } else {
                    action = "Buy";
                }
            } else if ((*it).action() == "Add") {
                qty = (*it).shares();
                if (qty.isNegative()) {
                    action = "Shrsout";
                } else {
                    action = "Shrsin";
                }
            } else if ((*it).action() == "Reinvest") {
                action = "ReinvDiv";
            } else {
                action = (*it).action();
            }

            //
            //  Add action.
            //
            s += 'N' + action + '\n';
            QString txt;
            if ((action == "Buy") || (action == "Sell") || (action == "ReinvDiv")) {
                //
                //  Add total.
                //
                txt = value.formatMoney("", 2);
                if (action == "Sell") {
                    value = -value;
                    txt = value.formatMoney("", 2);
                }
                s += 'T' + txt + '\n';
                //
                //  Add price.
                //
                txt = (*it).price().formatMoney("", 6);
                s += 'I' + txt + '\n';
                if (!qty.isZero()) {
                    //
                    //  Add quantity.
                    //
                    if (action == "Sell") {
                        qty = -qty;
                    }
                    s += 'Q' + m_qifProfile.value('Q', qty) + '\n';
                }
            } else if ((action == "Shrsin") || (action == "Shrsout")) {
                //
                //  Add quantity for "Shrsin" || "Shrsout".
                //
                if (action == "Shrsout") {
                    qty = -qty;
                }
                s += 'Q' + m_qifProfile.value('Q', qty) + '\n';
            }
        }
        if (!accName.isEmpty()) {
            s += 'Y' + accName + '\n';
        }
    }
    if (!memo.isEmpty()) {
        memo.replace('\n', "\\n");
        s += 'M' + memo + '\n';
    }
    if ((!chkAccnt.isEmpty()) && isXfer) {
        //
        //  Add account - including its hierarchy.
        //
        s += 'L' + m_qifProfile.accountDelimiter().at(0) + file->accountToCategory(chkAccntId) + m_qifProfile.accountDelimiter().at(1) + '\n';
    }
    stream << s << '^' << '\n';
}
