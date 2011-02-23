/*******************************************************************************
*                              investmentdlg.cpp
*                              -----------------
* begin                       : Sat Jan 01 2010
* copyright                   : (C) 2010 by Allan Anderson
* email                       : aganderson@ukonline.co.uk
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
#include "csvimporterdlg.h"
#include "mymoneystatement.h"
#include "redefinedlg.h"

InvestmentDlg::InvestmentDlg()
{
}

InvestmentDlg::~InvestmentDlg()
{
}

void InvestmentDlg::init()
{
  m_investProcessing->init();
  m_investProcessing->m_investDlg = this;

  m_csvDialog->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_csvDialog->tableWidget->setWordWrap(false);

  for(int i = 0; i < MAXCOL; i++) {
    QString t;
    t.setNum(i + 1);
    m_csvDialog->comboBoxInv_amountCol->addItem(t) ;
    m_csvDialog->comboBoxInv_dateCol->addItem(t) ;
    m_csvDialog->comboBoxInv_memoCol->addItem(t) ;
    m_csvDialog->comboBoxInv_priceCol->addItem(t) ;
    m_csvDialog->comboBoxInv_quantityCol->addItem(t) ;
    m_csvDialog->comboBoxInv_typeCol->addItem(t) ;
    m_csvDialog->comboBoxInv_feeCol->addItem(t) ;
  }

  connect(m_csvDialog->button_open, SIGNAL(clicked()), m_investProcessing, SLOT(fileDialog()));

  connect(m_csvDialog->comboBoxInv_memoCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(memoColumnSelected(int)));
  connect(m_csvDialog->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(typeColumnSelected(int)));
  connect(m_csvDialog->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(dateColumnSelected(int)));
  connect(m_csvDialog->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(quantityColumnSelected(int)));
  connect(m_csvDialog->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(priceColumnSelected(int)));
  connect(m_csvDialog->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(amountColumnSelected(int)));
  connect(m_csvDialog->comboBoxInv_feeCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(feeColumnSelected(int)));
  connect(m_investProcessing, SIGNAL(statementReady(MyMoneyStatement&)), this, SIGNAL(statementReady(MyMoneyStatement&)));
}

void InvestmentDlg::changedType(const QString& newType)
{
  if((newType == "buy") || (newType == "sell") || (newType == "divx") ||
      (newType == "reinvdiv") || (newType == "shrsin") || (newType == "shrsout")) {
    m_investProcessing->setTrInvestDataType(newType);
  }
}

void InvestmentDlg::slotClose()
{
  if(!m_investProcessing->inFileName().isEmpty()) {  //          don't save column numbers if no file loaded
    KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

    KConfigGroup investmentGroup(config, "InvestmentSettings");

    QString str = "$HOME/" + m_investProcessing->invPath().section('/', 3);
    investmentGroup.writeEntry("InvDirectory", str);
    investmentGroup.writeEntry("StartLine", m_csvDialog->spinBox_skip->value() - 1);
    investmentGroup.config()->sync();

    KConfigGroup profileGroup(config, "Profile");
    profileGroup.writeEntry("DateFormat", m_csvDialog->comboBox_dateFormat->currentIndex());
    profileGroup.writeEntry("FieldDelimiter", m_csvDialog->comboBox_fieldDelimiter->currentIndex());
    profileGroup.config()->sync();

    KConfigGroup invcolumnsGroup(config, "InvColumns");
    invcolumnsGroup.writeEntry("DateCol", m_csvDialog->comboBoxInv_dateCol->currentIndex());
    invcolumnsGroup.writeEntry("PayeeCol", m_csvDialog->comboBoxInv_typeCol->currentIndex());
    invcolumnsGroup.writeEntry("MemoCol", m_csvDialog->comboBoxInv_memoCol->currentIndex());
    invcolumnsGroup.writeEntry("QuantityCol", m_csvDialog->comboBoxInv_quantityCol->currentIndex());
    invcolumnsGroup.writeEntry("AmountCol", m_csvDialog->comboBoxInv_amountCol->currentIndex());
    invcolumnsGroup.writeEntry("PriceCol", m_csvDialog->comboBoxInv_priceCol->currentIndex());
    invcolumnsGroup.writeEntry("FeeCol", m_csvDialog->comboBoxInv_feeCol->currentIndex());
    invcolumnsGroup.config()->sync();

    /*    These settings do not get altered so need not be saved.

    investmentGroup.writeEntry( "ShrsinParam", invcsv->shrsinList);
    investmentGroup.writeEntry( "DivXParam", invcsv->divXList);
    investmentGroup.writeEntry( "BrokerageParam", invcsv->brokerageList);
    investmentGroup.writeEntry( "ReinvdivParam", invcsv->reinvdivList);
    investmentGroup.writeEntry( "BuyParam", invcsv->buyList);
    investmentGroup.writeEntry( "SellParam", invcsv->sellList);
    investmentGroup.writeEntry( "RemoveParam", invcsv->removeList);
    investmentGroup.config()->sync();*/

    m_investProcessing->inFileName().clear();
  }
  m_csvDialog->tableWidget->clear();//     in case later reopening window, clear old contents now
  m_csvDialog->m_plugin->m_action->setEnabled(true);
  m_csvDialog->CsvImporterDlg::close();
}

void InvestmentDlg::closeEvent(QCloseEvent *event)
{
  slotClose();
  event->accept();
}

void InvestmentDlg::resizeEvent(QResizeEvent * event)
{
  event->accept();
  if(!m_investProcessing->inFileName().isEmpty())
    m_investProcessing->updateScreen();
}

void InvestmentDlg::helpSelected()
{
  KAboutData aboutData("csvImporter",  0,  ki18n("CSV Importer"),  "0.3.0",
                       ki18n("A plugin to enable import of \n   CSV format files into KMyMoney."),
                       KAboutData::License_GPL,  ki18n("Copyright (c) 2010 Allan Anderson"),
                       // Optional text shown in the About box.
                       // Can contain any information desired.
                       ki18n("Information..."),
                       // The program homepage string.
                       "http://kmymoney-devel@kde.org/",
                       // The bug report email address
                       "submit@bugs.kde.org"
                      );

  aboutData.addAuthor(ki18n("Allan Anderson"), ki18n("CSV Importer plugin"), "aganderson@ukonline.co.uk");
  aboutData.addAuthor(ki18n("Ace Jones"), ki18n("Original OFX Importer plugin"), "acejones@users.sourceforge.net");
  aboutData.addCredit(ki18n("The KMyMoney development team"), ki18n("For help and guidance"), "");
  aboutData.setProgramIconName("kmymoney");

  KAboutApplicationDialog* aboutDlg = new KAboutApplicationDialog(&aboutData,  0);
  aboutDlg->show();
}

void InvestmentDlg::fileDialog()
{
  m_investProcessing->fileDialog();
}
