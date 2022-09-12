/*
    SPDX-FileCopyrightText: 2013-2014 Allan Anderson <agander93@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "csvwriter.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QFile>
#include <QList>
#include <QDebug>
#include <QStringBuilder>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneysplit.h"
#include "mymoneypayee.h"
#include "mymoneyexception.h"
#include "csvexportdlg.h"
#include "csvexporter.h"
#include "mymoneyenums.h"

CsvWriter::CsvWriter() :
    m_plugin(0),
    m_firstSplit(false),
    m_highestSplitCount(0),
    m_noError(true)
{
}

CsvWriter::~CsvWriter()
{
}

void CsvWriter::write(const QString& filename,
                      const QString& accountId, const bool accountData,
                      const bool categoryData,
                      const QDate& startDate, const QDate& endDate,
                      const QString& separator)
{
    m_separator = separator;
    QFile csvFile(filename);
    if (csvFile.open(QIODevice::WriteOnly)) {
        QTextStream s(&csvFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        s.setCodec("UTF-8");
#endif

        m_plugin->exporterDialog()->show();
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

        csvFile.close();
        qDebug() << i18n("Export completed.\n");
        delete m_plugin->exporterDialog();  //  Can now delete as export finished
    } else {
        KMessageBox::error(0, i18n("Unable to open file '%1' for writing", filename));
    }
}

void CsvWriter::writeAccountEntry(QTextStream& stream, const QString& accountId, const QDate& startDate, const QDate& endDate)
{
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount account;
    QString data;

    account = file->account(accountId);
    MyMoneyTransactionFilter filter(accountId);

    QString type = account.accountTypeToString(account.accountType());
    data = QString(i18n("Account Type:"));

    if (account.accountType() == eMyMoney::Account::Type::Investment) {
        data += QString("%1\n\n").arg(type);
        m_headerLine << QString(i18n("Date")) << QString(i18n("Security")) << QString(i18n("Action/Type")) << QString(i18n("Amount")) << QString(i18n("Quantity")) << QString(i18n("Price")) << QString(i18n("Interest")) << QString(i18n("Fees")) << QString(i18n("Account")) << QString(i18n("Memo")) << QString(i18n("Status"));
        data += m_headerLine.join(m_separator);
        extractInvestmentEntries(accountId, startDate, endDate);
    } else {
        data += QString("%1\n\n").arg(type);
        m_headerLine << QString(i18n("Date")) << QString(i18n("Payee")) << QString(i18n("Amount")) << QString(i18n("Account/Cat")) << QString(i18n("Memo")) << QString(i18n("Status")) << QString(i18n("Number"));
        filter.setDateFilter(startDate, endDate);

        QList<MyMoneyTransaction> trList;
        file->transactionList(trList, filter);
        QList<MyMoneyTransaction>::ConstIterator it;
        signalProgress(0, trList.count());
        int count = 0;
        m_highestSplitCount = 0;
        for (it = trList.constBegin(); it != trList.constEnd(); ++it) {
            writeTransactionEntry(*it, accountId, ++count);
            if (m_noError)
                signalProgress(count, 0);
        }
        data += m_headerLine.join(m_separator);
    }

    QString result;
    auto it_map = m_map.constBegin();
    while (it_map != m_map.constEnd()) {
        result += it_map.value();
        ++it_map;
    }

    stream << data << result << QLatin1Char('\n');
}

void CsvWriter::writeCategoryEntries(QTextStream &s)
{
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount income;
    MyMoneyAccount expense;

    income = file->income();
    expense = file->expense();

    QStringList list = income.accountList() + expense.accountList();
    Q_EMIT signalProgress(0, list.count());
    QStringList::Iterator it_catList;
    int count = 0;
    for (it_catList = list.begin(); it_catList != list.end(); ++it_catList) {
        writeCategoryEntry(s, *it_catList, "");
        Q_EMIT signalProgress(++count, 0);
    }
}

void CsvWriter::writeCategoryEntry(QTextStream &s, const QString& accountId, const QString& leadIn)
{
    MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
    QString name = format(acc.name());

    s << leadIn << name;
    s << (acc.accountGroup() == eMyMoney::Account::Type::Expense ? QLatin1Char('E') : QLatin1Char('I'));
    s << Qt::endl;

    Q_FOREACH (const auto sAccount, acc.accountList())
        writeCategoryEntry(s, sAccount, name);
}


void CsvWriter::writeTransactionEntry(const MyMoneyTransaction& t, const QString& accountId, const int count)
{
    m_firstSplit = true;
    m_noError = true;
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneySplit split = t.splitByAccount(accountId);
    QList<MyMoneySplit> splits = t.splits();
    if (splits.count() < 2) {
        KMessageBox::error(0, i18n("Transaction number '%1' is missing an account assignment.\n"
                                   "Date '%2', Payee '%3'.\nTransaction dropped.\n", count, t.postDate().toString(Qt::ISODate), file->payee(split.payeeId()).name()),
                           i18n("Invalid transaction"));
        m_noError = false;
        return;
    }

    QString str;
    str += QLatin1Char('\n');

    str += QString("%1" + m_separator).arg(t.postDate().toString(Qt::ISODate));
    MyMoneyPayee payee = file->payee(split.payeeId());
    str += format(payee.name());

    str += format(split.value());

    if (splits.count() > 1) {
        MyMoneySplit sp = t.splitByAccount(accountId, false);
        str += format(file->accountToCategory(sp.accountId()));
    }

    str += format(split.memo());

    switch (split.reconcileFlag()) {
    case eMyMoney::Split::State::Cleared:
        str += QLatin1String("C") + m_separator;
        break;

    case eMyMoney::Split::State::Reconciled:
    case eMyMoney::Split::State::Frozen:
        str += QLatin1String("R") + m_separator;
        break;

    default:
        str += m_separator;
        break;
    }

    str += format(split.number(), false);

    if (splits.count() > 2) {
        QList<MyMoneySplit>::ConstIterator it;
        for (it = splits.constBegin(); it != splits.constEnd(); ++it) {
            if (!((*it) == split)) {
                writeSplitEntry(str, *it, splits.count() - 1, it+1 == splits.constEnd());
            }
        }
    }
    QString date = t.postDate().toString(Qt::ISODate);
    m_map.insert(date, str);
}

void CsvWriter::writeSplitEntry(QString &str, const MyMoneySplit& split, const int splitCount, const int lastEntry)
{
    if (m_firstSplit) {
        m_firstSplit = false;
        str += m_separator;
    }
    MyMoneyFile* file = MyMoneyFile::instance();
    str += format(file->accountToCategory(split.accountId()));

    if (splitCount > m_highestSplitCount) {
        m_highestSplitCount++;
        m_headerLine << QString(i18n("splitCategory")) << QString(i18n("splitMemo")) << QString(i18n("splitAmount"));
        m_headerLine.join(m_separator);
    }
    str += format(split.memo());

    str += format(split.value(), 2, !lastEntry);
}

void CsvWriter::extractInvestmentEntries(const QString& accountId, const QDate& startDate, const QDate& endDate)
{
    MyMoneyFile* file = MyMoneyFile::instance();

    Q_FOREACH (const auto sAccount, file->account(accountId).accountList()) {
        MyMoneyTransactionFilter filter(sAccount);
        filter.setDateFilter(startDate, endDate);
        QList<MyMoneyTransaction> list;
        file->transactionList(list, filter);
        QList<MyMoneyTransaction>::ConstIterator itList;
        signalProgress(0, list.count());
        int count = 0;
        for (itList = list.constBegin(); itList != list.constEnd(); ++itList) {
            writeInvestmentEntry(*itList, ++count);
            signalProgress(count, 0);
        }
    }
}

void CsvWriter::writeInvestmentEntry(const MyMoneyTransaction& t, const int count)
{
    QString strQuantity;
    QString strAmount;
    QString strPrice;
    QString strAccName;
    QString strCheckingAccountName;
    QString strMemo;
    QString strAction;
    QString strStatus;
    QString strInterest;
    QString strFees;
    MyMoneyFile* file = MyMoneyFile::instance();
    QString chkAccnt;
    QList<MyMoneySplit> lst = t.splits();
    QList<MyMoneySplit>::Iterator itSplit;
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
            switch (lst[i].reconcileFlag()) {
            case eMyMoney::Split::State::Cleared:
                strStatus =  QLatin1Char('C');
                break;

            case eMyMoney::Split::State::Reconciled:
            case eMyMoney::Split::State::Frozen:
                strStatus =  QLatin1Char('R');
                break;

            default:
                strStatus.clear();
                break;
            }
        }
    }
    //
    //  Add date.
    //
    QString str = QString("\n%1" + m_separator).arg(t.postDate().toString(Qt::ISODate));
    for (itSplit = lst.begin(); itSplit != lst.end(); ++itSplit) {
        MyMoneyAccount acc = file->account((*itSplit).accountId());
        //
        //  eMyMoney::Account::Type::Checkings.
        //
        if ((acc.accountType() == eMyMoney::Account::Type::Checkings) || (acc.accountType() == eMyMoney::Account::Type::Cash) || (acc.accountType() == eMyMoney::Account::Type::Savings)) {
            chkAccntId = (*itSplit).accountId();
            chkAccnt = file->account(chkAccntId).name();
            strCheckingAccountName = format(file->accountToCategory(chkAccntId));
            strAmount = format((*itSplit).value());
        } else if (acc.accountType() == eMyMoney::Account::Type::Income) {
            //
            //  eMyMoney::Account::Type::Income.
            //
            qty = (*itSplit).shares();
            value = (*itSplit).value();
            strInterest = format(value);
        } else if (acc.accountType() == eMyMoney::Account::Type::Expense) {
            //
            //  eMyMoney::Account::Type::Expense.
            //
            qty = (*itSplit).shares();
            value = (*itSplit).value();
            strFees = format(value);
        }  else if (acc.accountType() == eMyMoney::Account::Type::Stock) {
            //
            //  eMyMoney::Account::Type::Stock.
            //
            strMemo = format((*itSplit).memo());
            //
            //  Actions.
            //
            if ((*itSplit).action() == QLatin1String("Dividend")) {
                strAction = QLatin1String("DivX");
            } else if ((*itSplit).action() == QLatin1String("IntIncome")) {
                strAction = QLatin1String("IntIncX");
            }
            if ((strAction == QLatin1String("DivX")) || (strAction == QLatin1String("IntIncX"))) {
                if ((map.value(eMyMoney::Account::Type::Checkings).isEmpty()) && (map.value(eMyMoney::Account::Type::Cash).isEmpty())) {
                    KMessageBox::error(0, i18n("Transaction number '%1' is missing an account assignment.\n"
                                               "Date '%2', Amount '%3'.\nTransaction dropped.\n", count, t.postDate().toString(Qt::ISODate), strAmount),
                                       i18n("Invalid transaction"));
                    return;
                }
            } else if ((*itSplit).action() == QLatin1String("Buy")) {
                qty = (*itSplit).shares();
                if (qty.isNegative()) {
                    strAction = QLatin1String("Sell");
                } else {
                    strAction = QLatin1String("Buy");
                }
            } else if ((*itSplit).action() == QLatin1String("Add")) {
                qty = (*itSplit).shares();
                if (qty.isNegative()) {
                    strAction = QLatin1String("Shrsout");
                } else {
                    strAction = QLatin1String("Shrsin");
                }
            } else if ((*itSplit).action() == QLatin1String("Reinvest")) {
                qty = (*itSplit).shares();
                strAmount = format((*itSplit).value());
                strAction = QLatin1String("ReinvDiv");
            } else {
                strAction = (*itSplit).action();
            }
            //
            //  Add action.
            //
            if ((strAction == QLatin1String("Buy")) || (strAction == QLatin1String("Sell")) || (strAction == QLatin1String("ReinvDiv"))) {
                //
                //  Add total.
                // TODO: value is not used below
                if (strAction == QLatin1String("Sell")) {
                    value = -value;
                }
                //
                //  Add price.
                //
                strPrice = format((*itSplit).price(), 6);
                if (!qty.isZero()) {
                    //
                    //  Add quantity.
                    //
                    if (strAction == QLatin1String("Sell")) {
                        qty = -qty;
                    }
                    strQuantity = format(qty);
                }
            } else if ((strAction == QLatin1String("Shrsin")) || (strAction == QLatin1String("Shrsout"))) {
                //
                //  Add quantity for "Shrsin" || "Shrsout".
                //
                if (strAction == QLatin1String("Shrsout")) {
                    qty = -qty;
                }
                strQuantity = format(qty);
            }
            strAccName = format(acc.name());
            strAction += m_separator;
        }

        if (strAmount.isEmpty())
            strAmount = m_separator;

        if (strQuantity.isEmpty())
            strQuantity = m_separator;

        if (strPrice.isEmpty())
            strPrice = m_separator;

        if (strCheckingAccountName.isEmpty()) {
            strCheckingAccountName = m_separator;
        }
        if (strInterest.isEmpty()) {
            strInterest = m_separator;
        }
        if (strFees.isEmpty()) {
            strFees = m_separator;
        }
    }  //  end of itSplit loop
    str += strAccName + strAction + strAmount + strQuantity + strPrice + strInterest + strFees + strCheckingAccountName + strMemo + strStatus;
    QString date = t.postDate().toString(Qt::ISODate);
    m_map.insert(date, str);
}

/**
 * Format string field according to csv rules
 * @param s string to format
 * @param withSeparator append field separator to string (default = true)
 * @return csv formatted string
 */
QString CsvWriter::format(const QString &s, bool withSeparator)
{
    if (s.isEmpty())
        return withSeparator ? m_separator : QString();
    QString m = s;
    m.remove('\'');
    m.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QString("\"%1\"%2").arg(m, withSeparator ? m_separator : QString());
}

/**
 * format money value according to csv rules
 * @param value value to format
 * @param prec precision used for formatting (default = 2)
 * @param withSeparator append field separator to string (default = true)
 * @return formatted value as string
 */
QString CsvWriter::format(const MyMoneyMoney &value, int prec, bool withSeparator)
{
    return QString("\"%1\"%2").arg(value.formatMoney("", prec, false), withSeparator ? m_separator : QString());
}
