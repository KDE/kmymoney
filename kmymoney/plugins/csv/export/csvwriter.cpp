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

#include "accountsmodel.h"
#include "csvexportdlg.h"
#include "csvexporter.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "securitiesmodel.h"

CsvWriter::CsvWriter()
    : m_plugin(nullptr)
    , m_firstSplit(false)
    , m_highestSplitCount(0)
    , m_noError(true)
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
        KMessageBox::error(nullptr, i18n("Unable to open file '%1' for writing", filename).append(QString::fromLatin1(": ") + csvFile.errorString()));
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
    data = i18n("Account Type:");
    data += QString("%1").arg(type);
    data += m_separator;

    data += i18n("Account Name:");
    data += QString("%1\n\n").arg(account.name());

    if (account.accountType() == eMyMoney::Account::Type::Investment) {
        m_headerLine << i18nc("@title:column header in CSV export", "Date") << i18nc("@title:column header in CSV export", "Security")
                     << i18nc("@title:column header in CSV export", "Symbol") << i18nc("@title:column header in CSV export", "Action/Type")
                     << i18nc("@title:column header in CSV export", "Amount") << i18nc("@title:column header in CSV export", "Quantity")
                     << i18nc("@title:column header in CSV export", "Price") << i18nc("@title:column header in CSV export", "Interest")
                     << i18nc("@title:column header in CSV export", "Fees") << i18nc("@title:column header in CSV export", "Account")
                     << i18nc("@title:column header in CSV export", "Memo") << i18nc("@title:column header in CSV export", "Status");
        data += m_headerLine.join(m_separator);
        extractInvestmentEntries(accountId, startDate, endDate);
    } else {
        m_headerLine << i18nc("@title:column header in CSV export", "Date") << i18nc("@title:column header in CSV export", "Payee")
                     << i18nc("@title:column header in CSV export", "Amount") << i18nc("@title:column header in CSV export", "Account/Cat")
                     << i18nc("@title:column header in CSV export", "Memo") << i18nc("@title:column header in CSV export", "Status")
                     << i18nc("@title:column header in CSV export", "Number");
        filter.setDateFilter(startDate, endDate);

        QList<MyMoneyTransaction> trList;
        file->transactionList(trList, filter);
        QList<MyMoneyTransaction>::const_iterator it;
        Q_EMIT signalProgress(0, trList.count());
        int count = 0;
        m_highestSplitCount = 0;
        for (it = trList.cbegin(); it != trList.cend(); ++it) {
            writeTransactionEntry(*it, accountId, ++count);
            if (m_noError)
                Q_EMIT signalProgress(count, 0);
        }
        data += m_headerLine.join(m_separator);
    }

    QString result;
    auto it_map = m_map.cbegin();
    while (it_map != m_map.cend()) {
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
    const MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
    QString name = format(acc.name());

    s << leadIn << name;
    s << (acc.accountGroup() == eMyMoney::Account::Type::Expense ? QLatin1Char('E') : QLatin1Char('I'));
    s << Qt::endl;

    const auto accountList = acc.accountList();
    for (const auto& sAccount : qAsConst(accountList))
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
        KMessageBox::error(nullptr,
                           i18n("Transaction number '%1' is missing an account assignment.\n"
                                "Date '%2', Payee '%3'.\nTransaction dropped.\n",
                                count,
                                t.postDate().toString(Qt::ISODate),
                                file->payee(split.payeeId()).name()),
                           i18n("Invalid transaction"));
        m_noError = false;
        return;
    }

    QString str;
    str += QLatin1Char('\n');

    str += QString("%1" + m_separator).arg(t.postDate().toString(Qt::ISODate));
    MyMoneyPayee payee = file->payee(split.payeeId());
    str += format(payee.name());

    str += format(split.value(), split.accountId());

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
        QList<MyMoneySplit>::const_iterator it;
        for (it = splits.cbegin(); it != splits.cend(); ++it) {
            if (!((*it) == split)) {
                writeSplitEntry(str, *it, splits.count() - 1, it + 1 == splits.cend());
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
        m_headerLine << i18nc("@title:column header in CSV export", "splitCategory") << i18nc("@title:column header in CSV export", "splitMemo")
                     << i18nc("@title:column header in CSV export", "splitAmount");
        m_headerLine.join(m_separator);
    }
    str += format(split.memo());

    str += format(split.value(), split.accountId(), !lastEntry);
}

void CsvWriter::extractInvestmentEntries(const QString& accountId, const QDate& startDate, const QDate& endDate)
{
    MyMoneyFile* file = MyMoneyFile::instance();

    const auto accountList = file->account(accountId).accountList();
    for (const auto& sAccount : qAsConst(accountList)) {
        MyMoneyTransactionFilter filter(sAccount);
        filter.setDateFilter(startDate, endDate);
        QList<MyMoneyTransaction> list;
        file->transactionList(list, filter);
        QList<MyMoneyTransaction>::const_iterator itList;
        Q_EMIT signalProgress(0, list.count());
        int count = 0;
        for (itList = list.cbegin(); itList != list.cend(); ++itList) {
            writeInvestmentEntry(*itList, ++count);
            Q_EMIT signalProgress(count, 0);
        }
    }
}

void CsvWriter::writeInvestmentEntry(const MyMoneyTransaction& t, const int count)
{
    QString strQuantity;
    QString strAmount;
    QString strPrice;
    QString strAccName;
    QString strAccSymbol;
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
    MyMoneySecurity security;

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
            strAmount = format((*itSplit).value(), (*itSplit).accountId());
        } else if (acc.accountType() == eMyMoney::Account::Type::Income) {
            //
            //  eMyMoney::Account::Type::Income.
            //
            qty = (*itSplit).shares();
            value = (*itSplit).value();
            strInterest = format(value, (*itSplit).accountId());
        } else if (acc.accountType() == eMyMoney::Account::Type::Expense) {
            //
            //  eMyMoney::Account::Type::Expense.
            //
            qty = (*itSplit).shares();
            value = (*itSplit).value();
            strFees = format(value, (*itSplit).accountId());
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
                    KMessageBox::error(nullptr,
                                       i18n("Transaction number '%1' is missing an account assignment.\n"
                                            "Date '%2', Amount '%3'.\nTransaction dropped.\n",
                                            count,
                                            t.postDate().toString(Qt::ISODate),
                                            strAmount),
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
                strAmount = format((*itSplit).value(), (*itSplit).accountId());
                strAction = QLatin1String("ReinvDiv");
            } else {
                strAction = (*itSplit).action();
            }

            strAccName = format(acc.name());
            security = file->security(acc.currencyId());
            strAccSymbol = format(security.tradingSymbol());
            //
            //  Add action.
            //
            if ((strAction == QLatin1String("Buy")) || (strAction == QLatin1String("Sell")) || (strAction == QLatin1String("ReinvDiv"))) {
                //
                //  Add total.
                // TODO: value is not used below
                if (strAction == QLatin1String("Sell")) {
                    value = -value;
                    qty = -qty;
                }
                //
                //  Add price.
                //
                strPrice = format((*itSplit).possiblyCalculatedPrice(), security.pricePrecision());
                if (!qty.isZero()) {
                    //
                    //  Add quantity.
                    //
                    strQuantity = format(qty, MyMoneyMoney::denomToPrec(acc.fraction()));
                }
            } else if ((strAction == QLatin1String("Shrsin")) || (strAction == QLatin1String("Shrsout"))) {
                //
                //  Add quantity for "Shrsin" || "Shrsout".
                //
                if (strAction == QLatin1String("Shrsout")) {
                    qty = -qty;
                }
                strQuantity = format(qty, MyMoneyMoney::denomToPrec(acc.fraction()));
            }
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
    str += strAccName + strAccSymbol + strAction + strAmount + strQuantity + strPrice + strInterest + strFees + strCheckingAccountName + strMemo + strStatus;
    QString date = t.postDate().toString(Qt::ISODate);
    m_map.insert(date, str);
}

/**
 * Format string field according to csv rules
 * @param s string to format
 * @param withSeparator append field separator to string (default = true)
 * @return csv formatted string
 */
QString CsvWriter::format(const QString& s, bool withSeparator) const
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
QString CsvWriter::format(const MyMoneyMoney& value, int prec, bool withSeparator) const
{
    return QString("\"%1\"%2").arg(value.formatMoney("", prec, false), withSeparator ? m_separator : QString());
}

QString CsvWriter::format(const MyMoneyMoney& value, const QString& accountId, bool withSeparator) const
{
    const auto account = MyMoneyFile::instance()->account(accountId);
    return format(value, MyMoneyMoney::denomToPrec(account.fraction()), withSeparator);
}
