/*
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008-2011 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LISTTABLE_H
#define LISTTABLE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QMetaEnum>
#include <QVector>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "reporttable.h"

class MyMoneyReport;

namespace reports
{

class ReportAccount;

/**
  * Calculates a query of information about the transaction database.
  *
  * This is a middle-layer class, between the implementing classes and the engine. The
  * MyMoneyReport class holds only the CONFIGURATION parameters.  This
  * class has some common methods used by querytable and objectinfo classes
  *
  * @author Alvaro Soliverez
  *
  * @short
  **/

class ListTable : public ReportTable
{
    Q_OBJECT
public:
    enum class Rank { OpeningBalance = 0, FirstSplitOfTransaction, SecondarySplitOfTransaction, ClosingBalance, BaseCurrencyTotals, ForeignCurrencyTotals };
    Q_ENUM(Rank)

    explicit ListTable(const MyMoneyReport&);
    QString renderHTML() const final override;
    QString renderCSV() const final override;
    void drawChart(KReportChartView&) const final override {}
    void dump(const QString& file, const QString& context = QString()) const final override;
    bool saveToXml(const QString& file) override;
    QString toXml() const override;
    void init();

public:
    enum cellTypeE /*{*/ /*Money*/
    {
        ctValue,
        ctNetInvValue,
        ctMarketValue,
        ctPrice,
        ctLastPrice,
        ctBuyPrice,
        ctBuys,
        ctSells,
        ctBuysST,
        ctSellsST,
        ctBuysLT,
        ctSellsLT,
        ctCapitalGain,
        ctCapitalGainST,
        ctCapitalGainLT,
        ctCashIncome,
        ctReinvestIncome,
        ctFees,
        ctInterest,
        ctStartingBalance,
        ctEndingBalance,
        ctBalance,
        ctCurrentBalance,
        ctBalanceWarning,
        ctMaxBalanceLimit,
        ctOpeningBalance,
        ctCreditWarning,
        ctMaxCreditLimit,
        ctLoanAmount,
        ctPeriodicPayment,
        ctFinalPayment,
        ctPayment,
        ctReturn,
        /*Shares*/
        ctShares,
        /*Percent*/
        ctAnnualizedReturn,
        ctExtendedInternalRateOfReturn,
        ctReturnInvestment,
        ctInterestRate,
        ctPercentageGain,
        /*Date*/
        ctPostDate,
        ctEntryDate,
        ctNextDueDate,
        ctOpeningDate,
        ctNextInterestChange,
        ctMonth,
        ctWeek,
        ctReconcileDate,
        /*Misc*/
        ctCurrency,
        ctCurrencyName,
        ctCommodity,
        ctID,
        ctRank,
        ctSplit,
        ctMemo,
        ctAccount,
        ctAccountID,
        ctTopAccount,
        ctInvestAccount,
        ctInstitution,
        ctCategory,
        ctTopCategory,
        ctCategoryType,
        ctNumber,
        ctReconcileFlag,
        ctAction,
        ctTag,
        ctPayee,
        ctEquityType,
        ctType,
        ctName,
        ctDepth,
        ctRowsCount,
        ctTax,
        ctFavorite,
        ctDescription,
        ctOccurrence,
        ctPaymentType,
        ctSourceLines,
        ctStartingMarketValue,
        ctEndingMarketValue,
        csID,
    };
    Q_ENUM(cellTypeE)

    /**
      * Contains a single row in the table.
      *
      * Each column is a key/value pair, both strings.  This class is just
      * a QMap with the added ability to specify which columns you'd like to
      * use as a sort key when you qHeapSort a list of these TableRows
      */
    class TableRow: public QMap<cellTypeE, QString>
    {
    public:
        bool operator< (const TableRow&) const;
        bool operator<= (const TableRow&) const;
        bool operator> (const TableRow&) const;
        bool operator== (const TableRow&) const;

        static void setSortCriteria(const QVector<cellTypeE>& _criteria) {
            m_sortCriteria = _criteria;
        }

        void addSourceLine(cellTypeE type, int line)
        {
            QMetaEnum metaEnum = QMetaEnum::fromType<cellTypeE>();
            QString typeName = metaEnum.valueToKey(type);

            QMap::operator[](ctSourceLines).append(QString(" %1:%2").arg(typeName).arg(line));
        }

    private:
        static QVector<cellTypeE> m_sortCriteria;
    };

    const QList<TableRow>& rows() {
        return m_rows;
    }

protected:
    void render(QString&, QString&) const;

    /**
     * If not in expert mode, include all subaccounts for each selected
     * investment account.
     * For investment-only reports, it will also exclude the subaccounts
     * that have a zero balance
     */
    void includeInvestmentSubAccounts();

    QList<TableRow> m_rows;

    QList<cellTypeE> m_group;
    /**
     * Comma-separated list of columns to place BEFORE the subtotal column
     */
    QList<cellTypeE> m_columns;

    /**
     * Temporary storage for the price column
     */
    QList<cellTypeE> m_priceColumn;

    /**
     * Name of the subtotal column
     */
    QList<cellTypeE> m_subtotal;
    /**
     * Comma-separated list of columns to place AFTER the subtotal column
     */
    QList<cellTypeE> m_postcolumns;

    virtual bool linkEntries() const = 0;

private:
    enum cellGroupE { cgMoney, cgShares, cgPercent, cgDate, cgPrice, cgMisc };
    static cellGroupE cellGroup(const cellTypeE cellType);
    static QString tableHeader(const cellTypeE cellType);
};

}

// keep in sync with reports::ListTable::Rank enum class
#define OPEN_BALANCE_RANK QLatin1String("0")
#define FIRST_SPLIT_RANK QLatin1String("1")
#define SECONDARY_SPLIT_RANK QLatin1String("2")
#define END_BALANCE_RANK QLatin1String("3")
#define BASE_CURRENCY_TOTAL_RANK QLatin1String("4")
#define FOREIGN_CURRENCY_TOTAL_RANK QLatin1String("5")

#define FIRST_ID_SORT QLatin1String("A")
#define LAST_ID_SORT QLatin1String("Z")
#endif

