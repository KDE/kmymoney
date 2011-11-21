/*******************************************************************************
*                              investmentdlg.cpp
*                              -----------------
* begin                       : Sat Jan 01 2010
* copyright                   : (C) 2010 by Allan Anderson
* email                       : agander93@gmail.com
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#include "investmentdlg.h"
#include "investprocessing.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QtGui/QScrollBar>
#include <QtGui/QDesktopWidget>
#include <QtGui/QCloseEvent>

#include <QtCore/QFile>
#include <QFileDialog>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QPointer>
#include <QtCore/QDebug>
// ----------------------------------------------------------------------------
// KDE Headers

#include <KFileDialog>
#include <KInputDialog>
#include <KSharedConfig>
#include <KMessageBox>
#include <KStandardDirs>
#include <KLocale>
#include <KIO/NetAccess>
#include <KAboutData>
#include <KAction>
#include <KAboutApplicationDialog>

// ----------------------------------------------------------------------------
// Project Headers

#include "convdate.h"
#include "csvdialog.h"
#include "mymoneystatement.h"
#include "redefinedlg.h"

#include "ui_csvdialog.h"
#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_lines-datewizardpage.h"
#include "ui_completionwizardpage.h"
#include "ui_investmentwizardpage.h"

InvestmentDlg::InvestmentDlg()
{
}

InvestmentDlg::~InvestmentDlg()
{
}

void InvestmentDlg::init()
{
  m_csvDialog->m_investProcessing->init();
  m_csvDialog->m_investProcessing->m_investDlg = this;

  m_csvDialog->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_csvDialog->ui->tableWidget->setWordWrap(false);
  m_csvDialog->m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(-1);

  for(int i = 0; i < MAXCOL; i++) {
    QString t;
    t.setNum(i + 1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->addItem(t);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->addItem(t);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->addItem(t);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->addItem(t);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->addItem(t);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->addItem(t);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->addItem(t);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->addItem(t);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->addItem(t);
  }

  connect(m_csvDialog->m_wizard->button(QWizard::CustomButton1), SIGNAL(clicked()), m_investProcessing, SLOT(slotFileDialogClicked()));

  connect(m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(memoColumnSelected(int)));
  connect(m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(typeColumnSelected(int)));
  connect(m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(dateColumnSelected(int)));
  connect(m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(quantityColumnSelected(int)));
  connect(m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(priceColumnSelected(int)));
  connect(m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(amountColumnSelected(int)));
  connect(m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(feeColumnSelected(int)));
  connect(m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(symbolColumnSelected(int)));
  connect(m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(detailColumnSelected(int)));

  connect(m_investProcessing, SIGNAL(statementReady(MyMoneyStatement&)), this, SIGNAL(statementReady(MyMoneyStatement&)));
}

void InvestmentDlg::changedType(const QString& newType)
{
  if((newType == "buy") || (newType == "sell") || (newType == "divx") ||
      (newType == "reinvdiv") || (newType == "shrsin") || (newType == "shrsout")) {
    m_investProcessing->setTrInvestDataType(newType);
  }
}

void InvestmentDlg::saveSettings()
{
  if(m_investProcessing->inFileName().isEmpty()) {  //          don't save column numbers if no file loaded
    return;
  }
  QString str;

  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup brokerGroup(config, "Brokers");

  brokerGroup.writeEntry("BrokerName", m_csvDialog->m_investProcessing->m_brokerList);
  brokerGroup.config()->sync();

  KConfigGroup profileGroup(config, "Profile");
  profileGroup.writeEntry("DateFormat", m_csvDialog->m_pageLinesDate->ui->comboBox_dateFormat->currentIndex());
  profileGroup.writeEntry("FieldDelimiter", m_csvDialog->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex());
  profileGroup.config()->sync();

  KConfigGroup securitiesGroup(config, "Securities");
  securitiesGroup.writeEntry("SecurityNameList", m_investProcessing->securityList());
  securitiesGroup.config()->sync();

  m_investProcessing->inFileName().clear();

  switch(m_csvDialog->m_activityType) {
    case 0://  Banking
      return;
      break;
    case 1:  {//  Investment
        KConfigGroup investmentGroup(config, "InvestmentSettings");
        if(str == "Invest") {
          investmentGroup.writeEntry("StartLine", m_csvDialog->m_pageLinesDate->ui->spinBox_skip->value() - 1);
        }

        //    The strings in these resource file lists may be edited,
        //    or expanded in the file by the user, to suit his needs.

        investmentGroup.writeEntry("ShrsinParam", m_investProcessing->m_shrsinList);
        investmentGroup.writeEntry("DivXParam", m_investProcessing->m_divXList);
        investmentGroup.writeEntry("IntIncParam", m_investProcessing->m_intIncList);
        investmentGroup.writeEntry("BrokerageParam", m_investProcessing->m_brokerageList);
        investmentGroup.writeEntry("ReinvdivParam", m_investProcessing->m_reinvdivList);
        investmentGroup.writeEntry("BuyParam", m_investProcessing->m_buyList);
        investmentGroup.writeEntry("SellParam", m_investProcessing->m_sellList);
        investmentGroup.writeEntry("RemoveParam", m_investProcessing->m_removeList);

        QString pth = "$HOME/" + m_investProcessing->invPath().section('/', 3);
        investmentGroup.writeEntry("InvDirectory", pth);

        investmentGroup.config()->sync();


        KConfigGroup invcolumnsGroup(config, "InvColumns");
        invcolumnsGroup.writeEntry("DateCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->currentIndex());
        invcolumnsGroup.writeEntry("PayeeCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->currentIndex());
        invcolumnsGroup.writeEntry("MemoCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->currentIndex());
        invcolumnsGroup.writeEntry("QuantityCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->currentIndex());
        invcolumnsGroup.writeEntry("AmountCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->currentIndex());
        invcolumnsGroup.writeEntry("PriceCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->currentIndex());
        invcolumnsGroup.writeEntry("FeeCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->currentIndex());
        invcolumnsGroup.writeEntry("SymbolCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->currentIndex());
        invcolumnsGroup.writeEntry("DetailCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->currentIndex());
        invcolumnsGroup.config()->sync();
        break;
      }
    case 2: { //  Broker1
        KConfigGroup BrokerageSettingsGroup1(config, "BrokerageSettings1");
        if(str == "Invest") {
          BrokerageSettingsGroup1.writeEntry("StartLine", m_csvDialog->m_pageLinesDate->ui->spinBox_skip->value() - 1);
        }

        //    The strings in these resource file lists may be edited,
        //    or expanded in the file by the user, to suit his needs.

        BrokerageSettingsGroup1.writeEntry("ShrsinParam", m_investProcessing->m_shrsinList);
        BrokerageSettingsGroup1.writeEntry("DivXParam", m_investProcessing->m_divXList);
        BrokerageSettingsGroup1.writeEntry("IntIncParam", m_investProcessing->m_intIncList);
        BrokerageSettingsGroup1.writeEntry("BrokerageParam", m_investProcessing->m_brokerageList);
        BrokerageSettingsGroup1.writeEntry("ReinvdivParam", m_investProcessing->m_reinvdivList);
        BrokerageSettingsGroup1.writeEntry("BuyParam", m_investProcessing->m_buyList);
        BrokerageSettingsGroup1.writeEntry("SellParam", m_investProcessing->m_sellList);
        BrokerageSettingsGroup1.writeEntry("RemoveParam", m_investProcessing->m_removeList);

        str = m_csvDialog->m_pageInvestment->ui->lineEdit_filter->text();
        if(str.endsWith(' ')) {
          str.append('#');//  Terminate trailing blank
        }
        BrokerageSettingsGroup1.writeEntry("Filter", str);

        QString pth = "$HOME/" + m_investProcessing->invPath().section('/', 3);
        BrokerageSettingsGroup1.writeEntry("InvDirectory", pth);

        BrokerageSettingsGroup1.config()->sync();


        KConfigGroup BrokerageColumnsGroup1(config, "BrokerageColumns1");
        BrokerageColumnsGroup1.writeEntry("DateCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->currentIndex());
        BrokerageColumnsGroup1.writeEntry("PayeeCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->currentIndex());
        BrokerageColumnsGroup1.writeEntry("MemoCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->currentIndex());
        BrokerageColumnsGroup1.writeEntry("QuantityCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->currentIndex());
        BrokerageColumnsGroup1.writeEntry("AmountCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->currentIndex());
        BrokerageColumnsGroup1.writeEntry("PriceCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->currentIndex());
        BrokerageColumnsGroup1.writeEntry("FeeCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->currentIndex());
        BrokerageColumnsGroup1.writeEntry("SymbolCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->currentIndex());
        BrokerageColumnsGroup1.writeEntry("DetailCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->currentIndex());
        BrokerageColumnsGroup1.config()->sync();
        break;
      }
    case 3: { //  Broker2
        KConfigGroup BrokerageSettingsGroup2(config, "BrokerageSettings2");
        if(str == "Invest") {
          BrokerageSettingsGroup2.writeEntry("StartLine", m_csvDialog->m_pageLinesDate->ui->spinBox_skip->value() - 1);
        }
        //    The strings in these resource file lists may be edited,
        //    or expanded in the file by the user, to suit his needs.

        BrokerageSettingsGroup2.writeEntry("ShrsinParam", m_investProcessing->m_shrsinList);
        BrokerageSettingsGroup2.writeEntry("DivXParam", m_investProcessing->m_divXList);
        BrokerageSettingsGroup2.writeEntry("IntIncParam", m_investProcessing->m_intIncList);
        BrokerageSettingsGroup2.writeEntry("BrokerageParam", m_investProcessing->m_brokerageList);
        BrokerageSettingsGroup2.writeEntry("ReinvdivParam", m_investProcessing->m_reinvdivList);
        BrokerageSettingsGroup2.writeEntry("BuyParam", m_investProcessing->m_buyList);
        BrokerageSettingsGroup2.writeEntry("SellParam", m_investProcessing->m_sellList);
        BrokerageSettingsGroup2.writeEntry("RemoveParam", m_investProcessing->m_removeList);

        str = m_csvDialog->m_pageInvestment->ui->lineEdit_filter->text();
        if(str.endsWith(' ')) {
          str.append('#');//  Terminate trailing blank
        }
        BrokerageSettingsGroup2.writeEntry("Filter", str);

        QString pth = "$HOME/" + m_investProcessing->invPath().section('/', 3);
        BrokerageSettingsGroup2.writeEntry("InvDirectory", pth);

        BrokerageSettingsGroup2.config()->sync();


        KConfigGroup BrokerageColumnsGroup2(config, "BrokerageColumns2");
        BrokerageColumnsGroup2.writeEntry("DateCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->currentIndex());
        BrokerageColumnsGroup2.writeEntry("PayeeCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->currentIndex());
        BrokerageColumnsGroup2.writeEntry("MemoCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->currentIndex());
        BrokerageColumnsGroup2.writeEntry("QuantityCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->currentIndex());
        BrokerageColumnsGroup2.writeEntry("AmountCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->currentIndex());
        BrokerageColumnsGroup2.writeEntry("PriceCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->currentIndex());
        BrokerageColumnsGroup2.writeEntry("FeeCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->currentIndex());
        BrokerageColumnsGroup2.writeEntry("SymbolCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->currentIndex());
        BrokerageColumnsGroup2.writeEntry("DetailCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->currentIndex());
        BrokerageColumnsGroup2.config()->sync();
        break;
      }
  }
  m_csvDialog->ui->tableWidget->clear();//     in case later reopening window, clear old contents now
}

void InvestmentDlg::resizeEvent(QResizeEvent * event)
{
  event->accept();
  if(!m_investProcessing->inFileName().isEmpty())
    m_investProcessing->updateScreen();
}
