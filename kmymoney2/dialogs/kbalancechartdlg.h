/***************************************************************************
                          kbalancechartdlg  -  description
                             -------------------
    begin                : Mon Nov 26 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBALANCECHARTDLG_H
#define KBALANCECHARTDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdialog.h>

// ----------------------------------------------------------------------------
// Project Includes
#include <kreportchartview.h>
class MyMoneyAccount;

using namespace reports;

/**
 *	@author Thomas Baumgart <ipwizard@users.sourceforge.net>
 *  This dialog displays a chart with the account balance for the last 90 days.
 *  It also draws the account limit if the account has any.
 */
class KBalanceChartDlg : public KDialog
{
  Q_OBJECT
  public:
    explicit KBalanceChartDlg(const MyMoneyAccount& account, QWidget* parent = 0);
    ~KBalanceChartDlg();

  protected:
  /**
    * Draw the chart and calculate and draw the account limits if any
    */
    KReportChartView* drawChart(const MyMoneyAccount& account);

};

#endif
