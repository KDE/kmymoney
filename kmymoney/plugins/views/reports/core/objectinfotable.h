/*
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OBJECTINFOTABLE_H
#define OBJECTINFOTABLE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "listtable.h"

class MyMoneyReport;

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

class ObjectInfoTable : public ListTable
{
public:
  explicit ObjectInfoTable(const MyMoneyReport&);
  void init();

protected:
  void constructScheduleTable();
  void constructAccountTable();
  void constructAccountLoanTable();

  bool linkEntries() const final override { return false; }

private:
  /**
    * @param acc the investment account
    * @return the balance in the currency of the investment account
    */
  MyMoneyMoney investmentBalance(const MyMoneyAccount& acc);
};

}

#endif // QUERYREPORT_H
