/*
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008-2011 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "listtable.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QFile>
#include <QTextStream>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// KDE Includes
// This is just needed for i18n().  Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.

#include <alkimia/alkdom.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyreport.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyutils.h"
#include "qmetaobject.h"

namespace reports
{

QVector<ListTable::cellTypeE> ListTable::TableRow::m_sortCriteria;

// ****************************************************************************
//
// ListTable implementation
//
// ****************************************************************************

bool ListTable::TableRow::operator< (const TableRow& _compare) const
{
    bool result = false;
    for (const auto& criterion : qAsConst(m_sortCriteria)) {
        if (this->operator[](criterion) < _compare[criterion]) {
            result = true;
            break;
        } else if (this->operator[](criterion) > _compare[criterion]) {
            break;
        }
    }
    return result;
}

// needed for KDE < 3.2 implementation of qHeapSort
bool ListTable::TableRow::operator<= (const TableRow& _compare) const
{
    return (!(_compare < *this));
}

bool ListTable::TableRow::operator== (const TableRow& _compare) const
{
    return (!(*this < _compare) && !(_compare < *this));
}

bool ListTable::TableRow::operator> (const TableRow& _compare) const
{
    return (_compare < *this);
}

/**
   * TODO
   *
   * - Collapse 2- & 3- groups when they are identical
   * - Way more test cases (especially splits & transfers)
   * - Option to collapse splits
   * - Option to exclude transfers
   *
 */

ListTable::ListTable(const MyMoneyReport& _report):
    ReportTable(_report)
{
}

void ListTable::render(QString& result, QString& csv) const
{
    MyMoneyFile* file = MyMoneyFile::instance();

    result.clear();
    csv.clear();

    // retrieve the configuration parameters from the report definition.
    // the things that we care about for query reports are:
    // how to group the rows, what columns to display, and what field
    // to subtotal on
    QList<cellTypeE> columns = m_columns;
    if (!m_subtotal.isEmpty() && m_subtotal.count() == 1) // constructPerformanceRow has subtotal columns already in columns
        columns.append(m_subtotal);
    QList<cellTypeE> postcolumns = m_postcolumns;
    if (!m_postcolumns.isEmpty()) // prevent creation of empty column
        columns.append(postcolumns);

    result.append(QLatin1String("<table align=\"center\" class=\"report\">\n<thead><tr class=\"itemheader\">"));

    //
    // Table header
    //
    for (const auto& cellType : qAsConst(columns)) {
        result.append(QString::fromLatin1("<th>%1</th>").arg(tableHeader(cellType)));
        csv.append(tableHeader(cellType) + QLatin1Char(','));
    }
    csv.chop(1);  // remove last ',' character

    result.append(QLatin1String("</tr></thead>\n"));
    csv.append(QLatin1Char('\n'));

    // initialize group names to empty, so any group will have to display its header
    QStringList prevGrpNames;
    for (int i = 0; i < m_group.count(); ++i) {
        prevGrpNames.append(QString());
    }

    //
    // Rows
    //

    bool row_odd = true;
    bool isLowestGroupTotal = true;  // hack to inform whether to put separator line or not

    result.append(QLatin1String("<tbody>\n"));
    // ***DV***
    MyMoneyMoney startingBalance;
    MyMoneyMoney balanceChange = MyMoneyMoney();
    for (QList<TableRow>::const_iterator it_row = m_rows.cbegin(); it_row != m_rows.cend(); ++it_row) {
        bool ok;
        const Rank rowRank = static_cast<Rank>((*it_row).value(ctRank).toInt(&ok));
        Q_ASSERT(ok);
        // detect whether any of groups changed and display new group header in that case
        for (int i = 0; i < m_group.count(); ++i) {
            QString curGrpName = (*it_row).value(m_group.at(i));
            if (curGrpName.isEmpty()) // it could be grand total
                continue;
            if (prevGrpNames.at(i) != curGrpName) {
                // group header of lowest group doesn't bring any useful information
                // if hide transaction is enabled, so don't display it
                int lowestGroup = m_group.count() - 1;
                if (!m_config.isHideTransactions() || i != lowestGroup) {
                    row_odd = true;
                    result.append(QString::fromLatin1("<tr class=\"sectionheader\">"
                                                      "<td class=\"left%1\" "
                                                      "colspan=\"%2\">%3</td></tr>\n").arg(QString::number(i),
                                                              QString::number(columns.count()),
                                                              curGrpName));
                    csv.append(QString::fromLatin1("\"%1\"\n").arg(curGrpName));
                }
                if (i == lowestGroup)         // lowest group has been switched...
                    isLowestGroupTotal = true;  // ...so expect lowest group total
                prevGrpNames.replace(i, curGrpName);
            }
        }

        bool need_label = true;

        QString tlink;  // link information to account and transaction

        if (!m_config.isHideTransactions() || rowRank == Rank::BaseCurrencyTotals
            || rowRank == Rank::ForeignCurrencyTotals) { // if hide transaction is enabled display only total rows i.e. rank == *CurrencyTotals
            if (rowRank == Rank::OpeningBalance || rowRank == Rank::ClosingBalance) {
                // skip the opening and closing balance row,
                // if the balance column is not shown
                // rank = 0 for opening balance, rank = 3 for closing balance
                if (!columns.contains(ctBalance))
                    continue;
                result.append(QString::fromLatin1("<tr class=\"item%1\">").arg((*it_row).value(ctID)));
                // ***DV***
            } else if (rowRank == Rank::FirstSplitOfTransaction) {
                row_odd = ! row_odd;
                if (linkEntries()) {
                    tlink = QString::fromLatin1("id=%1&tid=%2").arg((*it_row).value(ctAccountID), (*it_row).value(ctID));
                }
                result.append(QString::fromLatin1("<tr class=\"row-%1\">").arg(row_odd ? QLatin1String("odd") : QLatin1String("even")));

            } else if (rowRank == Rank::SecondarySplitOfTransaction) {
                result.append(QString::fromLatin1("<tr class=\"item%1\">").arg(row_odd ? QLatin1Char('1') : QLatin1Char('0')));
            } else if (rowRank == Rank::BaseCurrencyTotals || rowRank == Rank::ForeignCurrencyTotals) {
                QList<TableRow>::const_iterator nextRow = std::next(it_row);
                if ((m_config.rowType() == eMyMoney::Report::RowType::Tag)) { //If we order by Tags don't show the Grand total as we can have multiple tags per transaction
                    continue;
                } else if (rowRank == Rank::BaseCurrencyTotals) {
                    if (nextRow != m_rows.end()) {
                        if (isLowestGroupTotal && m_config.isHideTransactions()) {
                            result.append(QLatin1String("<tr class=\"sectionfootermiddle\">"));
                            isLowestGroupTotal = false;
                        } else if ((*nextRow).value(ctRank) == FOREIGN_CURRENCY_TOTAL_RANK) {
                            result.append(QLatin1String("<tr class=\"sectionfooterfirst\">"));
                        } else {
                            result.append(QLatin1String("<tr class=\"sectionfooter\">"));
                        }
                    } else {
                        result.append(QLatin1String("<tr class=\"sectionfooter\">"));
                    }
                } else if (rowRank == Rank::ForeignCurrencyTotals) {
                    if (nextRow != m_rows.end()) {
                        if ((*nextRow).value(ctRank) == FOREIGN_CURRENCY_TOTAL_RANK)
                            result.append(QLatin1String("<tr class=\"sectionfootermiddle\">"));
                        else
                            result.append(QLatin1String("<tr class=\"sectionfooterlast\">"));
                    } else {
                        result.append(QLatin1String("<tr class=\"sectionfooterlast\">"));
                    }
                } /* else { dead code
           result.append(QLatin1String("<tr class=\"sectionfooter\">"));
         }*/
            } else {
                result.append(QString::fromLatin1("<tr class=\"row-%1\">").arg(row_odd ? QLatin1String("odd") : QLatin1String("even")));
            }
        } else {
            continue;
        }

        //
        // Columns
        //

        QList<cellTypeE>::const_iterator it_column = columns.cbegin();
        int skippedColumn = 0;
        QString tempResult;
        while (it_column != columns.cend()) {
            QString data = (*it_row).value(*it_column);

            // include a colspan information if we skipped some
            // columns while processing the total and subtotal rows
            auto flushTempResult = [&skippedColumn, &tempResult]() {
                QString finishedResult(tempResult);
                if (!tempResult.isEmpty()) {
                    if (skippedColumn > 0) {
                        finishedResult.replace(QLatin1String("<td"), QString::fromLatin1("<td colspan=\"%1\"").arg(skippedColumn + 1));
                        skippedColumn = 0;
                    }
                    tempResult.clear();
                }
                return finishedResult;
            };

            // ***DV***
            if (rowRank == Rank::SecondarySplitOfTransaction) {
                if (*it_column == ctValue)
                    data = (*it_row).value(ctSplit);
                else if (*it_column == ctPostDate //
                         || *it_column == ctNumber //
                         || *it_column == ctPayee //
                         || *it_column == ctAction //
                         || *it_column == ctShares //
                         || *it_column == ctPrice //
                         || *it_column == ctNextDueDate //
                         || *it_column == ctBalance //
                         || *it_column == ctAccount //
                         || *it_column == ctName)
                    data.clear();
            }

            // ***DV***
            else if (rowRank == Rank::OpeningBalance || rowRank == Rank::ClosingBalance) {
                if (*it_column == ctBalance) {
                    data = (*it_row).value(ctBalance);
                    if ((*it_row).value(ctID) == QLatin1String("A")) {          // opening balance?
                        startingBalance = MyMoneyMoney(data);
                        balanceChange = MyMoneyMoney();
                    }
                }

                if (need_label) {
                    if ((*it_column == ctPayee) ||
                            (*it_column == ctCategory) ||
                            (*it_column == ctMemo)) {
                        if (!(*it_row).value(ctShares).isEmpty()) {
                            data = ((*it_row).value(ctID) == QLatin1String("A"))
                                   ? i18n("Initial Market Value")
                                   : i18n("Ending Market Value");
                        } else {
                            data = ((*it_row).value(ctID) == QLatin1String("A"))
                                   ? i18n("Opening Balance")
                                   : i18n("Closing Balance");
                        }
                        need_label = false;
                    }
                }
            }
            // The 'balance' column is calculated at render-time
            // but not printed on split lines
            else if (*it_column == ctBalance && rowRank == Rank::FirstSplitOfTransaction) {
                // Take the balance off the deepest group iterator
                balanceChange += MyMoneyMoney((*it_row).value(ctValue, QLatin1String("0")));
                data = (balanceChange + startingBalance).toString();
            } else if ((rowRank == Rank::BaseCurrencyTotals || rowRank == Rank::ForeignCurrencyTotals)) {
                // process the columns for which we don't create totals
                if (*it_column == ctPostDate //
                    || *it_column == ctNumber //
                    || *it_column == ctPayee //
                    || *it_column == ctAction //
                    || *it_column == ctNextDueDate //
                    || *it_column == ctBalance //
                    || *it_column == ctBuyPrice //
                    || *it_column == ctLastPrice) {
                    data.clear();
                }

                // display total title but only if first column doesn't contain any data
                if (it_column == columns.cbegin() && data.isEmpty()) {
                    tempResult.append(QString::fromLatin1("<td class=\"left%1\">").arg((*it_row).value(ctDepth)));
                    if (rowRank == Rank::BaseCurrencyTotals) {
                        if (!(*it_row).value(ctDepth).isEmpty()) {
                            tempResult += i18nc("Total balance", "Total") + QLatin1Char(' ') + prevGrpNames.at((*it_row).value(ctDepth).toInt());
                            csv.append(i18nc("Total balance", "Total") + QLatin1Char(' ') + prevGrpNames.at((*it_row).value(ctDepth).toInt())
                                       + QLatin1Char(','));
                        } else {
                            tempResult += i18n("Grand Total");
                            csv.append(i18n("Grand Total") + QLatin1Char(','));
                        }
                    }
                    tempResult.append(QLatin1String("</td>"));
                    ++it_column;
                    continue;

                } else if (!m_subtotal.contains(*it_column)) { // don't display e.g. account in totals row
                    ++skippedColumn;
                    csv.append(QLatin1Char(','));
                    ++it_column;
                    continue;
                }
            }

            // append any pending total or subtotal header
            result += flushTempResult();

            // Figure out how to render the value in this column, depending on
            // what its properties are.
            //
            // TODO: This and the i18n headings are handled
            // as a set of parallel vectors.  Would be much better to make a single
            // vector of a properties class.
            QString tlinkBegin, tlinkEnd;
            if (!tlink.isEmpty()) {
                tlinkBegin = QString::fromLatin1("<a href=\"/ledger?%1\">").arg(tlink);
                tlinkEnd = QLatin1String("</a>");
            }

            QString currencyID = (*it_row).value(ctCurrency);

            if (currencyID.isEmpty())
                currencyID = file->baseCurrency().id();

            const auto currency = file->currency(currencyID);
            int fraction = currency.smallestAccountFraction();
            // show currency symbol or id if not present
            // don't show currency id, if there is only single currency
            const auto currencySymbol = (m_config.isConvertCurrency() || !m_containsNonBaseCurrency)
                ? QString()
                : (currency.tradingSymbol().isEmpty() ? currencyID : currency.tradingSymbol());

            switch (cellGroup(*it_column)) {
            case cgMoney:
                if (data.isEmpty()) {
                    result.append(QString::fromLatin1("<td%1></td>")
                                  .arg((*it_column == ctValue) ? QLatin1String(" class=\"value\"") : QString()));
                    csv.append(QLatin1String("\"\","));
                } else if (MyMoneyMoney(data) == MyMoneyMoney::autoCalc) {
                    result.append(QString::fromLatin1("<td%1>%3%2%4</td>")
                                  .arg((*it_column == ctValue) ? QLatin1String(" class=\"value\"") : QString(),
                                       i18n("Calculated"), tlinkBegin, tlinkEnd));
                    csv.append(QString::fromLatin1("\"%1\",").arg(i18n("Calculated")));
                } else {
                    auto value = MyMoneyMoney(data);
                    auto valueStr = value.formatMoney(fraction);
                    csv.append(QString::fromLatin1("\"%1 %2\",").arg(currencySymbol, valueStr));

                    QString colorBegin;
                    QString colorEnd;
                    if ((rowRank == Rank::BaseCurrencyTotals || rowRank == Rank::ForeignCurrencyTotals) && value.isNegative()) {
                        colorBegin = QString::fromLatin1("<font color=\"%1\">").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name());
                        colorEnd = QLatin1String("</font>");
                    }

                    result.append(QString::fromLatin1("<td%1>%4%6%2&nbsp;%3%7%5</td>")
                                      .arg((*it_column == ctValue) ? QLatin1String(" class=\"value\"") : QString(),
                                           currencySymbol,
                                           valueStr,
                                           tlinkBegin,
                                           tlinkEnd,
                                           colorBegin,
                                           colorEnd));
                }
                break;
            case cgPercent:
                if (data.isEmpty()) {
                    result.append(QLatin1String("<td></td>"));
                    csv.append(QLatin1String("\"\","));
                } else {
                    auto value = MyMoneyMoney(data) * MyMoneyMoney(100, 1);
                    auto valueStr = value.formatMoney(100); // percentage always with 2 fractional digits
                    csv.append(QString::fromLatin1("\"%1 %\",").arg(valueStr));

                    QString colorBegin;
                    QString colorEnd;
                    if ((rowRank == Rank::BaseCurrencyTotals || rowRank == Rank::ForeignCurrencyTotals) && value.isNegative()) {
                        colorBegin = QString::fromLatin1("<font color=\"%1\">").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name());
                        colorEnd = QLatin1String("</font>");
                    }

                    if ((rowRank == Rank::BaseCurrencyTotals || rowRank == Rank::ForeignCurrencyTotals) && value.isNegative())
                        valueStr =
                            QString::fromLatin1("<font color=\"%1\">%2</font>").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name(), valueStr);
                    result.append(QString::fromLatin1("<td>%2%4%1 %%5%3</td>").arg(valueStr, tlinkBegin, tlinkEnd, colorBegin, colorEnd));
                }
                break;
            case cgPrice:
            {
                if (data.isEmpty()) {
                    result.append(QLatin1String("<td></td>"));
                    csv.append(QLatin1String("\"\","));
                } else {
                    int pricePrecision = file->security(file->account((*it_row).value(ctAccountID)).currencyId()).pricePrecision();
                    result.append(QString::fromLatin1("<td>%3%2&nbsp;%1%4</td>")
                                      .arg(MyMoneyMoney(data).formatMoney(QString(), pricePrecision), currencySymbol, tlinkBegin, tlinkEnd));
                    csv.append(QString::fromLatin1("\"%1 %2\",").arg(currencySymbol, MyMoneyMoney(data).formatMoney(QString(), pricePrecision, false)));
                }
            }
            break;
            case cgShares:
                if (data.isEmpty()) {
                    result.append(QLatin1String("<td></td>"));
                    csv.append(QLatin1String("\"\","));
                } else {
                    int sharesPrecision = MyMoneyMoney::denomToPrec(file->security(file->account((*it_row).value(ctAccountID)).currencyId()).smallestAccountFraction());
                    result += QString::fromLatin1("<td>%2%1%3</td>").arg(MyMoneyMoney(data).formatMoney(QString(), sharesPrecision),
                              tlinkBegin, tlinkEnd);
                    csv.append(QString::fromLatin1("\"%1\",").arg(MyMoneyMoney(data).formatMoney(QString(), sharesPrecision, false)));
                }
                break;
            case cgDate:
                // do this before we possibly change data
                csv.append(QString::fromLatin1("\"%1\",").arg(data));

                // if we have a locale() then use its date formatter
                if (!data.isEmpty()) {
                    QDate qd = QDate::fromString(data, Qt::ISODate);
                    data = MyMoneyUtils::formatDate(qd);
                }
                result.append(QString::fromLatin1("<td class=\"left%4\">%2%1%3</td>").arg(data, tlinkBegin, tlinkEnd, QString::number(prevGrpNames.count() - 1)));
                break;
            default:
                result.append(QString::fromLatin1("<td class=\"left0\">%2%1%3</td>").arg(data, tlinkBegin, tlinkEnd));
                csv.append(QString::fromLatin1("\"%1\",").arg(data));
                break;
            }
            ++it_column;
            tlink.clear();
        }

        result.append(QLatin1String("</tr>\n"));
        csv.chop(1);  // remove final comma
        csv.append(QLatin1Char('\n'));
    }
    result.append(QLatin1String("</tbody>\n</table>\n"));
}

QString ListTable::renderHTML() const
{
    QString html, csv;
    render(html, csv);
    return html;
}

QString ListTable::renderCSV() const
{
    QString html, csv;
    render(html, csv);
    return csv;
}

void ListTable::dump(const QString& file, const QString& context) const
{
    QFile g(file);
    g.open(QIODevice::WriteOnly | QIODevice::Text);

    if (! context.isEmpty())
        QTextStream(&g) << context.arg(renderHTML());
    else
        QTextStream(&g) << renderHTML();
    g.close();
}

bool ListTable::saveToXml(const QString& file)
{
    QFile out(file);
    if (!out.open(QIODevice::WriteOnly))
        return false;
    QTextStream stream(&out);
    stream << toXml();
    return true;
}

QString ListTable::toXml() const
{
    AlkDomDocument doc;
    AlkDomElement el = doc.createElement("ListTable");
    QString name = m_config.name();
    el.setAttribute("name", name);

    for (const ListTable::TableRow& row : m_rows) {
        AlkDomElement r = doc.createElement("TableRow");
        QList<cellTypeE> keys = row.keys();
        for (const cellTypeE& key : keys) {
            QMetaEnum metaEnum = QMetaEnum::fromType<cellTypeE>();
            QString attrName = metaEnum.valueToKey(key);
            r.setAttribute(attrName, row[key]);
        }
        el.appendChild(r);
    }
    doc.appendChild(el);
    return doc.toString();
}

void ListTable::includeInvestmentSubAccounts()
{
    // if we're not in expert mode, we need to make sure
    // that all stock accounts for the selected investment
    // account are also selected.
    // In case we get called for a non investment only report we quit
    if (KMyMoneySettings::expertMode() || !m_config.isInvestmentsOnly()) {
        return;
    }

    // get all investment subAccountsList but do not include those with zero balance
    // or those which had no transactions during the timeframe of the report
    QStringList accountIdList;
    QStringList subAccountsList;
    MyMoneyFile* file = MyMoneyFile::instance();

    // get the report account filter
    if (!m_config.accounts(accountIdList)
            && m_config.isInvestmentsOnly()) {
        // this will only execute if this is an investment-only report
        QList<MyMoneyAccount> accountList;
        file->accountList(accountList);
        QList<MyMoneyAccount>::const_iterator it_ma;
        for (it_ma = accountList.cbegin(); it_ma != accountList.cend(); ++it_ma) {
            if ((*it_ma).accountType() == eMyMoney::Account::Type::Investment) {
                accountIdList.append((*it_ma).id());
            }
        }
    }

    for (const auto& sAccount : qAsConst(accountIdList)) {
        const auto acc = file->account(sAccount);
        if (acc.accountType() == eMyMoney::Account::Type::Investment) {
            const auto subAccountList = acc.accountList();
            for (const auto& sSubAccount : qAsConst(subAccountList)) {
                if (!accountIdList.contains(sSubAccount)) {
                    subAccountsList.append(sSubAccount);
                }
            }
        }
    }

    if (m_config.isInvestmentsOnly()
            && !m_config.isIncludingUnusedAccounts()) {
        // if the balance is not zero at the end, include the subaccount
        QStringList::iterator it_balance;
        for (it_balance = subAccountsList.begin(); it_balance != subAccountsList.end();) {
            if (!file->balance((*it_balance), m_config.toDate()).isZero()) {
                m_config.addAccount((*it_balance));
                it_balance = subAccountsList.erase((it_balance));

            } else {
                ++it_balance;
            }
        }

        // if there are transactions for that subaccount, include them
        MyMoneyTransactionFilter filter;
        filter.setDateFilter(m_config.fromDate(), m_config.toDate());
        filter.addAccount(subAccountsList);
        filter.setReportAllSplits(false);

        QList<MyMoneyTransaction> transactions;
        file->transactionList(transactions, filter);
        QList<MyMoneyTransaction>::const_iterator it_t = transactions.cbegin();

        //Check each split for a matching account
        for (; it_t != transactions.cend(); ++it_t) {
            const QList<MyMoneySplit>& splits = (*it_t).splits();
            for (const auto& split : splits) {
                const QString& accountId = split.accountId();
                if (!split.shares().isZero()
                        && subAccountsList.contains(accountId)) {
                    subAccountsList.removeOne(accountId);
                    m_config.addAccount(accountId);
                }
            }
        }
    } else {
        // if not an investment-only report or explicitly including unused accounts
        // add all investment subaccounts
        m_config.addAccount(subAccountsList);
    }
}

ListTable::cellGroupE ListTable::cellGroup(const cellTypeE cellType)
{
    switch (cellType) {
    // the list of columns which represent money, so we can display them correctly
    case ctValue:
    case ctNetInvValue:
    case ctMarketValue:
    case ctBuys:
    case ctSells:
    case ctBuysST:
    case ctSellsST:
    case ctBuysLT:
    case ctSellsLT:
    case ctCapitalGain:
    case ctCapitalGainST:
    case ctCapitalGainLT:
    case ctCashIncome:
    case ctReinvestIncome:
    case ctFees:
    case ctInterest:
    case ctStartingBalance:
    case ctEndingBalance:
    case ctBalance:
    case ctCurrentBalance:
    case ctBalanceWarning:
    case ctMaxBalanceLimit:
    case ctCreditWarning:
    case ctMaxCreditLimit:
    case ctLoanAmount:
    case ctPeriodicPayment:
    case ctFinalPayment:
    case ctPayment:
    case ctReturn:
    case ctStartingMarketValue:
    case ctEndingMarketValue:
        return cgMoney;
    case ctPrice:
    case ctLastPrice:
    case ctBuyPrice:
        return cgPrice;
    /* the list of columns which represent shares, which is like money except the
    transaction currency will not be displayed*/
    case ctShares:
        return cgShares;
    // the list of columns which represent a percentage, so we can display them correctly
    case ctAnnualizedReturn:
    case ctExtendedInternalRateOfReturn:
    case ctReturnInvestment:
    case ctInterestRate:
    case ctPercentageGain:
        return cgPercent;
    // the list of columns which represent dates, so we can display them correctly
    case ctPostDate:
    case ctEntryDate:
    case ctNextDueDate:
    case ctOpeningDate:
    case ctNextInterestChange:
        return cgDate;
    default:
        break;
    }
    return cgMisc;
}

QString ListTable::tableHeader(const cellTypeE cellType)
{
    switch (cellType) {
    case ctPostDate:
        return i18n("Date");
    case ctValue:
        return i18n("Amount");
    case ctNumber:
        return i18n("Num");
    case ctPayee:
        return i18n("Payee");
    case ctTag:
        return i18n("Tags");
    case ctCategory:
        return i18n("Category");
    case ctAccount:
        return i18n("Account");
    case ctMemo:
        return i18n("Memo");
    case ctTopCategory:
        return i18n("Top Category");
    case ctCategoryType:
        return i18n("Category Type");
    case ctMonth:
        return i18n("Month");
    case ctWeek:
        return i18n("Week");
    case ctReconcileFlag:
        return i18n("Reconciled");
    case ctAction:
        return i18n("Action");
    case ctShares:
        return i18n("Shares");
    case ctPrice:
        return i18n("Price");
    case ctLastPrice:
        return i18n("Last Price");
    case ctBuyPrice:
        return i18n("Buy Price");
    case ctNetInvValue:
        return i18n("Net Value");
    case ctBuys:
        return i18n("Buy Value");
    case ctSells:
        return i18n("Sell Value");
    case ctBuysST:
        return i18n("Short-term Buy Value");
    case ctSellsST:
        return i18n("Short-term Sell Value");
    case ctBuysLT:
        return i18n("Long-term Buy Value");
    case ctSellsLT:
        return i18n("Long-term Sell Value");
    case ctReinvestIncome:
        return i18n("Dividends Reinvested");
    case ctCashIncome:
        return i18n("Dividends Paid Out");
    case ctStartingBalance:
        return i18n("Starting Balance");
    case ctEndingBalance:
        return i18n("Ending Balance");
    case ctMarketValue:
        return i18n("Market Value");
    case ctAnnualizedReturn:
        return i18n("Annualized Return");
    case ctExtendedInternalRateOfReturn:
        return i18n("Extended internal rate of return");
    case ctReturn:
        return i18n("Return");
    case ctReturnInvestment:
        return i18n("Return On Investment");
    case ctFees:
        return i18n("Fees");
    case ctInterest:
        return i18n("Interest");
    case ctPayment:
        return i18n("Payment");
    case ctBalance:
        return i18n("Balance");
    case ctType:
        return i18n("Type");
    case ctName:
        return i18nc("Account name", "Name");
    case ctNextDueDate:
        return i18n("Next Due Date");
    case ctOccurrence:
        return i18n("Occurrence");
    case ctPaymentType:
        return i18n("Payment Method");
    case ctInstitution:
        return i18n("Institution");
    case ctDescription:
        return i18n("Description");
    case ctOpeningDate:
        return i18n("Opening Date");
    case ctCurrencyName:
        return i18n("Currency");
    case ctBalanceWarning:
        return i18n("Balance Early Warning");
    case ctMaxBalanceLimit:
        return i18n("Balance Max Limit");
    case ctCreditWarning:
        return i18n("Credit Early Warning");
    case ctMaxCreditLimit:
        return i18n("Credit Max Limit");
    case ctTax:
        return i18n("Tax");
    case ctFavorite:
        return i18n("Preferred");
    case ctLoanAmount:
        return i18n("Loan Amount");
    case ctInterestRate:
        return i18n("Interest Rate");
    case ctNextInterestChange:
        return i18n("Next Interest Change");
    case ctPeriodicPayment:
        return i18n("Periodic Payment");
    case ctFinalPayment:
        return i18n("Final Payment");
    case ctCurrentBalance:
        return i18n("Current Balance");
    case ctCapitalGain:
        return i18n("Capital Gain");
    case ctPercentageGain:
        return i18n("Percentage Gain");
    case ctCapitalGainST:
        return i18n("Short-term Gain");
    case ctCapitalGainLT:
        return i18n("Long-term Gain");
    case ctStartingMarketValue:
        return i18n("Starting Market Value");
    case ctEndingMarketValue:
        return i18n("Ending Market Value");
    default:
        break;
    }
    return QLatin1String("None");
}
}
