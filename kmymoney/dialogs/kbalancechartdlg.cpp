/***************************************************************************
                          kbalancechartdlg  -  description
                             -------------------
    begin                : Mon Nov 26 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QWindow>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KSharedConfig>
#include <KWindowConfig>
#include <KConfigGroup>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyreport.h"
#include "pivottable.h"
#include "kreportchartview.h"

using namespace reports;

KBalanceChartDlg::KBalanceChartDlg(const MyMoneyAccount& account, QWidget* parent) :
    QDialog(parent)
{
  setWindowTitle(i18n("Balance of %1", account.name()));
  setSizeGripEnabled(true);
  setModal(true);

  // restore the last used dialog size
  winId(); // needs to be called to create the QWindow
  KConfigGroup grp = KSharedConfig::openConfig()->group("KBalanceChartDlg");
  if (grp.isValid()) {
    KWindowConfig::restoreWindowSize(windowHandle(), grp);
  }
  // let the minimum size be 700x500
  resize(QSize(700, 500).expandedTo(windowHandle() ? windowHandle()->size() : QSize()));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  //draw the chart and add it to the main layout
  KReportChartView* chartWidget = drawChart(account);
  mainLayout->addWidget(chartWidget);

  // add the buttons
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(buttonBox);
}


KBalanceChartDlg::~KBalanceChartDlg()
{
  // store the last used dialog size
  KConfigGroup grp = KSharedConfig::openConfig()->group("KBalanceChartDlg");
  if (grp.isValid()) {
    KWindowConfig::saveWindowSize(windowHandle(), grp);
  }
}

KReportChartView* KBalanceChartDlg::drawChart(const MyMoneyAccount& account)
{
  MyMoneyReport reportCfg = MyMoneyReport(
                              MyMoneyReport::eAssetLiability,
                              MyMoneyReport::eMonths,
                              eMyMoney::TransactionFilter::Date::Last3ToNext3Months,
                              MyMoneyReport::eDetailTotal,
                              i18n("%1 Balance History", account.name()),
                              i18n("Generated Report")
                            );
  reportCfg.setChartByDefault(true);
  reportCfg.setChartCHGridLines(false);
  reportCfg.setChartSVGridLines(false);
  reportCfg.setChartDataLabels(false);
  reportCfg.setChartType(MyMoneyReport::eChartLine);
  reportCfg.setIncludingForecast(true);
  reportCfg.setIncludingBudgetActuals(true);
  if (account.accountType() == eMyMoney::Account::Investment) {
    foreach (const auto accountID, account.accountList())
      reportCfg.addAccount(accountID);
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
  if (account.accountGroup() == eMyMoney::Account::Asset)
    factor = -factor;

  if (!account.value("maxCreditEarly").isEmpty()) {
    needRow = true;
    haveMaxCredit = true;
    maxCredit = MyMoneyMoney(account.value("maxCreditEarly")) * factor;
  }
  if (!account.value("maxCreditAbsolute").isEmpty()) {
    needRow = true;
    haveMaxCredit = true;
    maxCredit = MyMoneyMoney(account.value("maxCreditAbsolute")) * factor;
  }

  if (!account.value("minBalanceEarly").isEmpty()) {
    needRow = true;
    haveMinBalance = true;
    minBalance = MyMoneyMoney(account.value("minBalanceEarly"));
  }
  if (!account.value("minBalanceAbsolute").isEmpty()) {
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
  // TODO: port to KF5 - this crashes KChart
  //chartWidget->drawLimitLine(0);

  //remove the legend
  chartWidget->removeLegend();

  return chartWidget;
}
