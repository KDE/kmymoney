/*
 * SPDX-FileCopyrightText: 2007-2011 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KBALANCECHARTDLG_H
#define KBALANCECHARTDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
namespace reports {
  class KReportChartView;
}
class MyMoneyAccount;

/**
 * @author Thomas Baumgart <ipwizard@users.sourceforge.net>
 *  This dialog displays a chart with the account balance for the last 90 days.
 *  It also draws the account limit if the account has any.
 */
class KBalanceChartDlg : public QDialog
{
  Q_OBJECT
public:
  explicit KBalanceChartDlg(const MyMoneyAccount& account, QWidget* parent = nullptr);
  ~KBalanceChartDlg();

protected:
  /**
    * Draw the chart and calculate and draw the account limits if any
    */
  reports::KReportChartView* drawChart(const MyMoneyAccount& account);

};

#endif
