/***************************************************************************
                          querytable.h
                         -------------------
    begin                : Fri Jul 23 2004
    copyright            : (C) 2004-2005 by Ace Jones <acejones@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QUERYTABLE_H
#define QUERYTABLE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyreport.h"
#include "listtable.h"

namespace reports
{

class ReportAccount;

/**
  * Calculates a query of information about the transaction database.
  *
  * This is a middle-layer class, between the UI and the engine.  The
  * MyMoneyReport class holds only the CONFIGURATION parameters.  This
  * class actually does the work of retrieving the data from the engine
  * and formatting it for the user.
  *
  * @author Ace Jones
  *
  * @short
**/

class QueryTable : public ListTable
{
public:
  QueryTable(const MyMoneyReport&);
  void init();
  static QString toDateString(const QDate &date);

protected:
  void constructAccountTable();
  void constructTransactionTable();
  void constructPerformanceRow(const ReportAccount& account, TableRow& result) const;
  void constructSplitsTable();

};

}

#endif // QUERYREPORT_H
