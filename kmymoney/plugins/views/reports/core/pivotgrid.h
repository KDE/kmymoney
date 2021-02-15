/*
 * SPDX-FileCopyrightText: 2005-2006 Ace Jones <acejones@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PIVOTGRID_H
#define PIVOTGRID_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "reportaccount.h"
#include "mymoneymoney.h"

namespace reports
{

enum ERowType {eActual, eBudget, eBudgetDiff, eForecast, eAverage, ePrice };

/**
  * The fundamental data construct of this class is a 'grid'.  It is organized as follows:
  *
  * A 'Row' is a row of money values, each column is a month.  The first month corresponds to
  * m_beginDate.
  *
  * A 'Row Pair' is two rows of money values.  Each column is the SAME month.  One row is the
  * 'actual' values for the period, the other row is the 'budgetted' values for the same
  * period.  For ease of implementation, a Row Pair is implemented as a Row which contains
  * another Row.  The inherited Row is the 'actual', the contained row is the 'Budget'.
  *
  * An 'Inner Group' contains a rows for each subordinate account within a single top-level
  * account.  It also contains a mapping from the account descriptor for the subordinate account
  * to its row data.  So if we have an Expense account called "Computers", with sub-accounts called
  * "Hardware", "Software", and "Peripherals", there will be one Inner Group for "Computers"
  * which contains three Rows.
  *
  * An 'Outer Group' contains Inner Row Groups for all the top-level accounts in a given
  * account class.  Account classes are Expense, Income, Asset, Liability.  In the case above,
  * the "Computers" Inner Group is contained within the "Expense" Outer Group.
  *
  * A 'Grid' is the set of all Outer Groups contained in this report.
  *
  */
class PivotCell: public MyMoneyMoney
{
  KMM_MYMONEY_UNIT_TESTABLE

public:
  PivotCell() : m_stockSplit(MyMoneyMoney::ONE), m_cellUsed(false) {}
  explicit PivotCell(const MyMoneyMoney& value);
  virtual ~PivotCell();
  static PivotCell stockSplit(const MyMoneyMoney& factor);
  PivotCell operator += (const PivotCell& right);
  PivotCell operator += (const MyMoneyMoney& value);
  const QString formatMoney(int fraction, bool showThousandSeparator = true) const;
  const QString formatMoney(const QString& currency, const int prec, bool showThousandSeparator = true) const;
  MyMoneyMoney calculateRunningSum(const MyMoneyMoney& runningSum);
  MyMoneyMoney cellBalance(const MyMoneyMoney& _balance);
  bool isUsed() const {
    return m_cellUsed;
  }
private:
  MyMoneyMoney m_stockSplit;
  MyMoneyMoney m_postSplit;
  bool m_cellUsed;
};
class PivotGridRow: public QList<PivotCell>
{
public:

  explicit PivotGridRow(unsigned _numcolumns = 0) {
    for (uint i = 0; i < _numcolumns; i++)
      append(PivotCell());
  }
  MyMoneyMoney m_total;
};

class PivotGridRowSet: public QMap<ERowType, PivotGridRow>
{
public:
  explicit PivotGridRowSet(unsigned _numcolumns = 0);
};

class PivotInnerGroup: public QMap<ReportAccount, PivotGridRowSet>
{
public:
  explicit PivotInnerGroup(unsigned _numcolumns = 0): m_total(_numcolumns) {}

  PivotGridRowSet m_total;
};

class PivotOuterGroup: public QMap<QString, PivotInnerGroup>
{
public:
  explicit PivotOuterGroup(unsigned _numcolumns = 0, unsigned _sort = m_kDefaultSortOrder, bool _inverted = false): m_total(_numcolumns), m_inverted(_inverted), m_sortOrder(_sort) {}
  bool operator<(const PivotOuterGroup& _right) const {
    if (m_sortOrder != _right.m_sortOrder)
      return m_sortOrder < _right.m_sortOrder;
    else
      return m_displayName < _right.m_displayName;
  }
  PivotGridRowSet m_total;

  // An inverted outergroup means that all values placed in subordinate rows
  // should have their sign inverted from typical cash-flow notation.  Also it
  // means that when the report is summed, the values should be inverted again
  // so that the grand total is really "non-inverted outergroup MINUS inverted outergroup".
  bool m_inverted;

  // The localized name of the group for display in the report. Outergoups need this
  // independently, because they will lose their association with the TGrid when the
  // report is rendered.
  QString m_displayName;

  // lower numbers sort toward the top of the report. defaults to 100, which is a nice
  // middle-of-the-road value
  unsigned m_sortOrder;

  // default sort order
  static const unsigned m_kDefaultSortOrder;
};
class PivotGrid: public QMap<QString, PivotOuterGroup>
{
public:
  PivotGridRowSet rowSet(QString id);

  PivotGridRowSet m_total;
};

}

#endif
// PIVOTGRID_H
