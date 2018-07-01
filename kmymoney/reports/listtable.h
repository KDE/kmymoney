/***************************************************************************
                          listtable.h
                         -------------------
    begin                : Sat 28 jun 2008
    copyright            : (C) 2004-2005 by Ace Jones <acejones@users.sourceforge.net>
                               2008 by Alvaro Soliverez <asoliverez@gmail.com>

****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LISTTABLE_H
#define LISTTABLE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyreport.h"
#include "reporttable.h"

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
public:
  ListTable(const MyMoneyReport&);
  QString renderBody() const;
  QString renderCSV() const;
  void drawChart(KReportChartView&) const {}
  void dump(const QString& file, const QString& context = QString()) const;
  void init();

public:
  /**
    * Contains a single row in the table.
    *
    * Each column is a key/value pair, both strings.  This class is just
    * a QMap with the added ability to specify which columns you'd like to
    * use as a sort key when you qHeapSort a list of these TableRows
    */
  class TableRow: public QMap<QString, QString>
  {
  public:
    bool operator< (const TableRow&) const;
    bool operator<= (const TableRow&) const;
    bool operator> (const TableRow&) const;
    bool operator== (const TableRow&) const;

    static void setSortCriteria(const QString& _criteria) {
      m_sortCriteria = _criteria.split(",");
    }
  private:
    static QStringList m_sortCriteria;
  };

  const QList<TableRow>& rows() {
    return m_rows;
  };

  MyMoneyReport& report() { return m_config; }

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

  QString m_group;
  /**
   * Comma-separated list of columns to place BEFORE the subtotal column
   */
  QString m_columns;
  /**
   * Name of the subtotal column
   */
  QString m_subtotal;
  /**
   * Comma-separated list of columns to place AFTER the subtotal column
   */
  QString m_postcolumns;
  QString m_summarize;
  QString m_propagate;

  MyMoneyReport m_config;


};

}

#endif

