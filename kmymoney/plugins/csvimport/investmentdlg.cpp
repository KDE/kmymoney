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
#include <KAboutApplicationDialog>
// ----------------------------------------------------------------------------
// Project Headers

#include "convdate.h"
#include "csvimporterdlg.h"
#include "mymoneystatement.h"
#include "redefinedlg.h"

InvestmentDlg::InvestmentDlg(QWidget* parent) :
    InvestmentDlgDecl(parent)
{
  m_tableFrameHeight = groupBox->frameGeometry().bottom() - frame_main->frameGeometry().bottom();
  m_tableFrameWidth = tableWidget->size().width();

  for (int i = 0; i < MAXCOL; i++) {
    QString t;
    t.setNum(i + 1);
    comboBox_amountCol->addItem(t) ;
    comboBox_dateCol->addItem(t) ;
    comboBox_memoCol->addItem(t) ;
    comboBox_priceCol->addItem(t) ;
    comboBox_quantityCol->addItem(t) ;
    comboBox_typeCol->addItem(t) ;
    comboBox_feeCol->addItem(t) ;
  }

  int screenWidth = QApplication::desktop()->width();
  int screenHeight = QApplication::desktop()->height();
  int x = (screenWidth - width()) / 2;
  int y = (screenHeight - height()) / 2;

  this->move(x, y);

  m_convertDat = new ConvertDate;
  m_investProcessing = new InvestProcessing;
  m_investProcessing->m_investDlg = this;
  m_redefine = new RedefineDlg;

  connect(checkBox_qif, SIGNAL(clicked(bool)), m_investProcessing, SLOT(acceptClicked(bool)));
  connect(button_clear, SIGNAL(clicked()), m_investProcessing, SLOT(clearColumnsSelected()));
  connect(button_close, SIGNAL(clicked()), this, SLOT(slotClose()));
  connect(button_banking, SIGNAL(clicked()), this, SLOT(bankingSelected()));
  connect(button_open, SIGNAL(clicked()), m_investProcessing, SLOT(fileDialog()));
  connect(button_saveAs, SIGNAL(clicked()), m_investProcessing, SLOT(saveAs()));
  connect(pushButton, SIGNAL(clicked()), this, SLOT(helpSelected()));
  connect(comboBox_encoding, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(encodingChanged()));
  connect(comboBox_fieldDelim, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(fieldDelimiterChanged()));
  connect(comboBox_memoCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(memoColumnSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(activated(int)), m_investProcessing, SLOT(dateFormatSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(dateFormatSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_convertDat, SLOT(dateFormatSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(activated(int)), m_convertDat, SLOT(dateFormatSelected(int)));
  connect(comboBox_typeCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(typeColumnChanged(int)));
  connect(comboBox_dateCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(dateColumnSelected(int)));
  connect(comboBox_quantityCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(quantityColumnSelected(int)));
  connect(comboBox_priceCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(priceColumnSelected(int)));
  connect(comboBox_amountCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(amountColumnSelected(int)));
  connect(comboBox_feeCol, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(feeColumnSelected(int)));
  connect(spinBox_skip, SIGNAL(editingFinished()), m_investProcessing, SLOT(startLineChanged()));
  connect(spinBox_skipLast, SIGNAL(editingFinished()), m_investProcessing, SLOT(endLineChanged()));
  connect(m_investProcessing, SIGNAL(statementReady(MyMoneyStatement&)), this, SIGNAL(statementReady(MyMoneyStatement&)));
  connect(m_redefine, SIGNAL(changedType(const QString&)), this, SLOT(changedType(const QString&)));

  m_investProcessing->init();
}

InvestmentDlg::~InvestmentDlg()
{
  delete       m_investProcessing;
  delete       m_convertDat;
  delete       m_redefine;
}

void InvestmentDlg::changedType(const QString& newType)
{
  if ((newType == "buy") || (newType == "sell") || (newType == "divx") ||
      (newType == "reinvdiv") || (newType == "shrsin") || (newType == "shrsout")) {
    m_investProcessing->m_trInvestData.type = newType;
  }
}

void InvestmentDlg::bankingSelected()
{
  this->hide();
  m_csvImportDlg->show();
}

void InvestmentDlg::slotClose()
{
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup investmentGroup(config, "InvestmentSettings");

  QString str = "$HOME/" + m_investProcessing->m_csvPath.section('/', 3);
  investmentGroup.writeEntry("InvDirectory", str);
  investmentGroup.writeEntry("StartLine", spinBox_skip->value() - 1);
  investmentGroup.config()->sync();

  KConfigGroup profileGroup(config, "Profile");
  profileGroup.writeEntry("DateFormat", comboBox_dateFormat->currentIndex());
  profileGroup.writeEntry("Encoding", comboBox_encoding->currentIndex());
  profileGroup.writeEntry("FieldDelimiter", comboBox_fieldDelim->currentIndex());
  profileGroup.config()->sync();

  if (!m_investProcessing->m_inFileName.isEmpty()) { //          don't save column numbers if no file loaded
    KConfigGroup invcolumnsGroup(config, "InvColumns");
    invcolumnsGroup.writeEntry("DateCol", comboBox_dateCol->currentIndex());
    invcolumnsGroup.writeEntry("PayeeCol", comboBox_typeCol->currentIndex());
    invcolumnsGroup.writeEntry("MemoCol", comboBox_memoCol->currentIndex());
    invcolumnsGroup.writeEntry("QuantityCol", comboBox_quantityCol->currentIndex());
    invcolumnsGroup.writeEntry("AmountCol", comboBox_amountCol->currentIndex());
    invcolumnsGroup.writeEntry("PriceCol", comboBox_priceCol->currentIndex());
    invcolumnsGroup.writeEntry("FeeCol", comboBox_feeCol->currentIndex());
    invcolumnsGroup.config()->sync();
  }
  /*    These settings do not get altered so need not be saved.

  investmentGroup.writeEntry( "ShrsinParam", invcsv->shrsinList);
  investmentGroup.writeEntry( "DivXParam", invcsv->divXList);
  investmentGroup.writeEntry( "BrokerageParam", invcsv->brokerageList);
  investmentGroup.writeEntry( "ReinvdivParam", invcsv->reinvdivList);
  investmentGroup.writeEntry( "BuyParam", invcsv->buyList);
  investmentGroup.writeEntry( "SellParam", invcsv->sellList);
  investmentGroup.writeEntry( "RemoveParam", invcsv->removeList);
  investmentGroup.config()->sync();*/

  m_investProcessing->m_inFileName.clear();
  tableWidget->clear();//     in case later reopening window, clear old contents now
  m_csvImportDlg->show();
  InvestmentDlg::close();
}

void InvestmentDlg::closeEvent(QCloseEvent *event)
{
  slotClose();
  event->accept();
}

void InvestmentDlg::resizeEvent(QResizeEvent * event)
{
  event->accept();
  if (!m_investProcessing->m_inFileName.isEmpty())
    m_investProcessing->updateScreen();
}

void InvestmentDlg::helpSelected()
{
  KAboutData aboutData("csvImporter",  0,  ki18n("CSV Importer"),  "0.2.5",
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
