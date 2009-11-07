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

#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QLayout>
#include <qframe.h>
#include <QHBoxLayout>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kbalancechartdlg.h"

#include <mymoneyreport.h>
#include "kreportchartview.h"
#include "pivottable.h"

#include <kmymoneyglobalsettings.h>

KBalanceChartDlg::KBalanceChartDlg(const MyMoneyAccount& account, QWidget* parent) :
  KDialog(parent)
{
  setCaption(i18n("Balance of %1",account.name()));
  setSizeGripEnabled( true );
  setModal( true );

  QVBoxLayout* KBalanceChartDlgLayout = new QVBoxLayout(this);
  KBalanceChartDlgLayout->setContentsMargins(11,11,11,11);
  KBalanceChartDlgLayout->setSpacing(6);
  KBalanceChartDlgLayout->setObjectName("KBalanceChartDlgLayout");


  MyMoneyReport reportCfg = MyMoneyReport(
                                          MyMoneyReport::eAssetLiability,
                                          MyMoneyReport::eMonths,
                                          MyMoneyTransactionFilter::last3Months,
                                          MyMoneyReport::eDetailTotal,
                                          i18n("%1 Balance History",account.name()),
                                               i18n("Generated Report")
                                         );
  reportCfg.setChartByDefault(true);
  reportCfg.setChartGridLines(false);
  reportCfg.setChartDataLabels(false);
  reportCfg.setChartType(MyMoneyReport::eChartLine);
  reportCfg.setIncludingSchedules( true );
  if(account.accountType() == MyMoneyAccount::Investment) {
    QStringList::const_iterator it_a;
    for(it_a = account.accountList().begin(); it_a != account.accountList().end(); ++it_a)
      reportCfg.addAccount(*it_a);
  } else
    reportCfg.addAccount(account.id());
  reportCfg.setColumnsAreDays( true );
  reportCfg.setConvertCurrency( false );
  reports::PivotTable table(reportCfg);

  reports::KReportChartView* chartWidget = new reports::KReportChartView(this);

  table.drawChart(*chartWidget);

  //resize the chart
  chartWidget->resize(height()-20, width()-20);

  KBalanceChartDlgLayout->addWidget(chartWidget, 10, Qt::AlignVCenter);


  // add another row for limit
  bool needRow = false;
  bool haveMinBalance = false;
  bool haveMaxCredit = false;
  MyMoneyMoney minBalance, maxCredit;
  MyMoneyMoney factor(1,1);
  if(account.accountGroup() == MyMoneyAccount::Asset)
    factor = -factor;

  if(account.value("maxCreditEarly").length() > 0) {
    needRow = true;
    haveMaxCredit = true;
    maxCredit = MyMoneyMoney(account.value("maxCreditEarly")) * factor;
  }
  if(account.value("maxCreditAbsolute").length() > 0) {
    needRow = true;
    haveMaxCredit = true;
    maxCredit = MyMoneyMoney(account.value("maxCreditAbsolute")) * factor;
  }

  if(account.value("minBalanceEarly").length() > 0) {
    needRow = true;
    haveMinBalance = true;
    minBalance = MyMoneyMoney(account.value("minBalanceEarly"));
  }
  if(account.value("minBalanceAbsolute").length() > 0) {
    needRow = true;
    haveMinBalance = true;
    minBalance = MyMoneyMoney(account.value("minBalanceAbsolute"));
  }

  if(needRow) {
    if(haveMinBalance) {
      chartWidget->drawLimitLine(minBalance.toDouble());
    }
    if(haveMaxCredit) {
      chartWidget->drawLimitLine(maxCredit.toDouble());
    }
  }

  //remove the legend
  chartWidget->removeLegend();

  QFrame* line1 = new QFrame( this );
  line1->setFrameShape( QFrame::HLine );
  line1->setFrameShadow( QFrame::Sunken );
  line1->setFrameShape( QFrame::HLine );

  KBalanceChartDlgLayout->addWidget(line1);
  //QVBoxLayout* Layout1 = new QVBoxLayout(KBalanceChartDlgLayout);
  //Layout1->setSpacing(6);
  //Layout1->setObjectName("Layout1");
#if 0
  KPushButton* buttonHelp = new KPushButton( this, "buttonHelp" );
  buttonHelp->setAutoDefault( TRUE );
  buttonHelp->setText(i18n("&Help"));
  Layout1->addWidget( buttonHelp );
#endif

  //QSpacerItem* Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  //Layout1->addItem( Horizontal_Spacing2 );

#if 0
  KPushButton* buttonOk = new KPushButton( this, "buttonOk" );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  buttonOk->setText(i18n("&OK"));
  Layout1->addWidget( buttonOk );

  KPushButton* buttonClose = new KPushButton( this );
  buttonClose->setEnabled( true );
  buttonClose->setAutoDefault( true );
  buttonClose->setGuiItem(KStandardGuiItem::Close);
  Layout1->addWidget( buttonClose );

  // connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( buttonClose, SIGNAL( clicked() ), this, SLOT( accept() ) );
#endif
  resize( QSize(700, 500).expandedTo(minimumSizeHint()) );
}


KBalanceChartDlg::~KBalanceChartDlg()
{
}

#include "kbalancechartdlg.moc"

