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

#include "kbalancechartdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyreport.h>
#include <pivottable.h>
#include <kmymoneyglobalsettings.h>

KBalanceChartDlg::KBalanceChartDlg(const MyMoneyAccount& account, QWidget* parent) :
    KDialog(parent)
{
  setCaption(i18n("Balance of %1", account.name()));
  setSizeGripEnabled(true);
  setModal(true);
  setButtons(KDialog::Close);
  setButtonsOrientation(Qt::Horizontal);

  // restore the last used dialog size
  KConfigGroup grp = KGlobal::config()->group("KBalanceChartDlg");
  if (grp.isValid()) {
    restoreDialogSize(grp);
  }
  // let the minimum size be 700x500
  resize(QSize(700, 500).expandedTo(size()));

  //draw the chart and add it to the main layout
  KReportChartView* chartWidget = drawChart(account);
  setMainWidget(chartWidget);
}


KBalanceChartDlg::~KBalanceChartDlg()
{
  // store the last used dialog size
  KConfigGroup grp = KGlobal::config()->group("KBalanceChartDlg");
  if (grp.isValid()) {
    saveDialogSize(grp);
  }
}

KReportChartView* KBalanceChartDlg::drawChart(const MyMoneyAccount& account)
{
  MyMoneyReport reportCfg = MyMoneyReport(
                              MyMoneyReport::Row::AssetLiability,
                              MyMoneyReport::Column::Months,
                              MyMoneyTransactionFilter::last3ToNext3Months,
                              MyMoneyReport::eDetailTotal,
                              i18n("%1 Balance History", account.name()),
                              i18n("Generated Report")
                            );
  reportCfg.setChartByDefault(true);
  reportCfg.setChartGridLines(false);
  reportCfg.setChartDataLabels(false);
  reportCfg.setChartType(MyMoneyReport::eChartLine);
  reportCfg.setIncludingForecast(true);
  reportCfg.setIncludingBudgetActuals(true);
  if (account.accountType() == MyMoneyAccount::Investment) {
    QStringList::const_iterator it_a;
    for (it_a = account.accountList().begin(); it_a != account.accountList().end(); ++it_a)
      reportCfg.addAccount(*it_a);
  } else
    reportCfg.addAccount(account.id());
  reportCfg.setColumnsAreDays(true);
  reportCfg.setConvertCurrency(false);
  reportCfg.setMixedTime(true);
  reports::PivotTable table(reportCfg);

  reports::KReportChartView* chartWidget = new reports::KReportChartView(this);

  table.drawChart(*chartWidget);

  // add another row for limit
  bool needRow = false;
  bool haveMinBalance = false;
  bool haveMaxCredit = false;
  MyMoneyMoney minBalance, maxCredit;
  MyMoneyMoney factor(1, 1);
  if (account.accountGroup() == MyMoneyAccount::Asset)
    factor = -factor;

  if (account.value("maxCreditEarly").length() > 0) {
    needRow = true;
    haveMaxCredit = true;
    maxCredit = MyMoneyMoney(account.value("maxCreditEarly")) * factor;
  }
  if (account.value("maxCreditAbsolute").length() > 0) {
    needRow = true;
    haveMaxCredit = true;
    maxCredit = MyMoneyMoney(account.value("maxCreditAbsolute")) * factor;
  }

  if (account.value("minBalanceEarly").length() > 0) {
    needRow = true;
    haveMinBalance = true;
    minBalance = MyMoneyMoney(account.value("minBalanceEarly"));
  }
  if (account.value("minBalanceAbsolute").length() > 0) {
    needRow = true;
    haveMinBalance = true;
    minBalance = MyMoneyMoney(account.value("minBalanceAbsolute"));
  }

  if (needRow) {
    if (haveMinBalance) {
      chartWidget->drawLimitLine(minBalance.toDouble());
    }
    if (haveMaxCredit) {
      chartWidget->drawLimitLine(maxCredit.toDouble());
    }
  }

  // always draw the y axis zero value line
  chartWidget->drawLimitLine(0);

  //remove the legend
  chartWidget->removeLegend();

  return chartWidget;
}
