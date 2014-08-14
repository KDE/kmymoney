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

#include <QScrollBar>
#include <QDesktopWidget>
#include <QCloseEvent>

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
#include <QStandardPaths>

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

  connect(m_csvDialog->m_wizard->button(QWizard::CustomButton1), SIGNAL(clicked()), m_investProcessing, SLOT(slotFileDialogClicked()));

  connect(m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName, SIGNAL(currentIndexChanged(int)), m_csvDialog->m_pageInvestment, SLOT(slotsecurityNameChanged(int)));

  connect(m_investProcessing, SIGNAL(statementReady(MyMoneyStatement&)), this, SIGNAL(statementReady(MyMoneyStatement&)));
}

void InvestmentDlg::saveSettings()
{
  if ((m_csvDialog->m_fileType != "Invest") || (m_investProcessing->inFileName().isEmpty())) {  // don't save if no file loaded
    return;
  }
  QString str;
  KSharedConfigPtr config = KSharedConfig::openConfig(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + "csvimporterrc");

  KConfigGroup bankProfilesGroup(config, "BankProfiles");

  bankProfilesGroup.writeEntry("BankNames", m_csvDialog->m_profileList);
  int indx = m_csvDialog->m_pageIntro->ui->combobox_source->findText(m_csvDialog->m_priorInvProfile, Qt::MatchExactly);
  if (indx > 0) {
    str = m_csvDialog->m_priorInvProfile;
  }
  bankProfilesGroup.writeEntry("PriorInvProfile", str);
  bankProfilesGroup.config()->sync();

  for (int i = 0; i < m_csvDialog->m_profileList.count(); i++) {
    if (m_csvDialog->m_profileList[i] != m_csvDialog->m_profileName) {
      continue;
    }

    QString txt = "Profiles-" + m_csvDialog->m_profileList[i];

    KConfigGroup profilesGroup(config, txt);
    profilesGroup.writeEntry("FileType", m_csvDialog->m_fileType);
    profilesGroup.writeEntry("DateFormat", m_csvDialog->m_pageLinesDate->ui->comboBox_dateFormat->currentIndex());
    profilesGroup.writeEntry("FieldDelimiter", m_csvDialog->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex());
    profilesGroup.writeEntry("DecimalSymbol", m_csvDialog->m_pageCompletion->ui->comboBox_decimalSymbol->currentIndex());
    profilesGroup.writeEntry("ProfileName", m_csvDialog->m_profileName);
    profilesGroup.writeEntry("PriceFraction", m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceFraction->currentIndex());
    profilesGroup.writeEntry("StartLine", m_csvDialog->m_pageLinesDate->ui->spinBox_skip->value() - 1);
    profilesGroup.writeEntry("SecurityName", m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->currentIndex());
    profilesGroup.writeEntry("TrailerLines", m_csvDialog->m_pageLinesDate->m_trailerLines);

    m_investProcessing->inFileName().clear();

    //    The strings in these resource file lists may be edited,
    //    or expanded in the file by the user, to suit his needs.

    profilesGroup.writeEntry("ShrsinParam", m_investProcessing->m_shrsinList);
    profilesGroup.writeEntry("DivXParam", m_investProcessing->m_divXList);
    profilesGroup.writeEntry("IntIncParam", m_investProcessing->m_intIncList);
    profilesGroup.writeEntry("BrokerageParam", m_investProcessing->m_brokerageList);
    profilesGroup.writeEntry("ReinvdivParam", m_investProcessing->m_reinvdivList);
    profilesGroup.writeEntry("BuyParam", m_investProcessing->m_buyList);
    profilesGroup.writeEntry("SellParam", m_investProcessing->m_sellList);
    profilesGroup.writeEntry("RemoveParam", m_investProcessing->m_removeList);

    str = m_csvDialog->m_pageInvestment->ui->lineEdit_filter->text();
    if (str.endsWith(' ')) {
      str.append('#');  //  Terminate trailing blank
    }
    profilesGroup.writeEntry("Filter", str);

    QString pth = "~/" + m_investProcessing->invPath().section('/', 3);
    profilesGroup.writeEntry("InvDirectory", pth);
    profilesGroup.writeEntry("DateCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->currentIndex());
    profilesGroup.writeEntry("PayeeCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->currentIndex());

    QList<int> list = m_investProcessing->m_memoColList;
    int posn = 0;
    if ((posn = list.indexOf(-1)) > -1) {
      list.removeOne(-1);
    }
    profilesGroup.writeEntry("MemoCol", list);
    profilesGroup.writeEntry("QuantityCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->currentIndex());
    profilesGroup.writeEntry("AmountCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->currentIndex());
    profilesGroup.writeEntry("PriceCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->currentIndex());
    profilesGroup.writeEntry("FeeCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->currentIndex());
    profilesGroup.writeEntry("SymbolCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->currentIndex());
    profilesGroup.writeEntry("DetailCol", m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->currentIndex());
    profilesGroup.config()->sync();

    KConfigGroup securitiesGroup(config, "Securities");
    securitiesGroup.writeEntry("SecurityNameList", m_investProcessing->securityList());
    securitiesGroup.config()->sync();
  }
  m_csvDialog->ui->tableWidget->clear();//     in case later reopening window, clear old contents now
}

