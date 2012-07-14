/*******************************************************************************
*                                 csvdialog.cpp
*                              --------------------
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

#include "csvdialog.h"
#include "convdate.h"
#include "csvutil.h"
#include "investmentdlg.h"
#include "investprocessing.h"
#include "symboltabledlg.h"

#include "ui_csvdialog.h"
#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_lines-datewizardpage.h"
#include "ui_completionwizardpage.h"
#include "ui_investmentwizardpage.h"

#include "mymoneyfile.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QtGui/QWizard>
#include <QtGui/QWizardPage>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QFrame>
#include <QGridLayout>
#include <QTableWidget>
#include <QWizard>
#include <QWizardPage>
#include <QTextCodec>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kdeversion.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kvbox.h>
#include <KAction>
#include <KSharedConfig>
#include <KComponentData>
#include <KInputDialog>
#include <KFileDialog>
#include <KFileWidget>
#include <KStandardDirs>
#include <KLocale>
#include <KIO/NetAccess>
#include "KAboutApplicationDialog"
#include <KAboutData>

// ----------------------------------------------------------------------------

CSVDialog::CSVDialog(QWidget *parent) : QWidget(parent), ui(new Ui::CSVDialog)
{
  ui->setupUi(this);

  m_amountSelected = false;
  m_creditSelected = false;
  m_debitSelected = false;
  m_dateSelected = false;
  m_payeeSelected = false;
  m_memoSelected = false;
  m_numberSelected = false;
  m_duplicate = false;
  m_importError = false;
  m_importIsValid = false;
  m_firstPass = true;
  m_firstRead = true;

  m_amountColumn = -1;
  m_creditColumn = -1;
  m_dateColumn = 0;
  m_debitColumn = -1;
  m_memoColumn = 0;
  m_numberColumn = 0;
  m_payeeColumn = 0;
  m_previousColumn = 0;
  m_maxColumnCount = 0;
  m_maxRowWidth = 0;
  m_endLine = 0;
  m_startLine = 1;
  m_row = 0;

  m_curId = -1;
  m_lastId = -1;
  m_fileEndLine = 0;

  m_memoColList.clear();
  m_profileList.clear();
  m_priorCsvProfile.clear();
  m_decimalSymbol.clear();
  m_previousType.clear();
  m_thousandsSeparator = ',';
  m_lastFileName.clear();

  m_iconBack = QPixmap(KIconLoader::global()->loadIcon("go-previous", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconFinish = QPixmap(KIconLoader::global()->loadIcon("dialog-ok-apply", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconCancel = QPixmap(KIconLoader::global()->loadIcon("dialog-cancel", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconCSV = QPixmap(KIconLoader::global()->loadIcon("kmymoney", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconImport = QPixmap(KIconLoader::global()->loadIcon("system-file-manager.", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconQIF = QPixmap(KIconLoader::global()->loadIcon("invest-applet", KIconLoader::Small, KIconLoader::DefaultState));
}

void CSVDialog::init()
{
  m_investProcessing = new InvestProcessing;
  m_investProcessing->m_csvDialog = this;

  readSettingsProfiles();

  m_wizard = new QWizard;
  ui->horizontalLayout->insertWidget(2, m_wizard, 0);
  ui->horizontalLayout->setStretch(0, 150);
  ui->horizontalLayout->setStretch(1, 1);
  ui->horizontalLayout->setStretch(2, 350);
  ui->horizontalLayout->setStretch(3, 1);

  this->setAutoFillBackground(true);

  ui->frame_stage->setPalette(QPalette(Qt::gray));

  QPalette pal = ui->frame_stage->palette();
  pal.setColor(ui->frame_stage->backgroundRole(), QColor(40, 40, 40, 0));
  ui->frame_stage->setPalette(pal);
  ui->frame_stage->setAutoFillBackground(true);


  m_wizard->button(QWizard::BackButton)->setIcon(m_iconBack);
  m_wizard->button(QWizard::CancelButton)->setIcon(m_iconCancel);
  m_wizard->button(QWizard::CustomButton2)->setIcon(m_iconCSV);
  m_wizard->button(QWizard::FinishButton)->setIcon(m_iconFinish);
  m_wizard->button(QWizard::CustomButton1)->setIcon(m_iconImport);
  m_wizard->button(QWizard::CustomButton3)->setIcon(m_iconQIF);
  m_wizard->button(QWizard::NextButton)->setIcon(KStandardGuiItem::forward(KStandardGuiItem::UseRTL).icon());


  m_wizard->setDefaultProperty("QComboBox", "source", SIGNAL(currentIndexChanged(int)));
  m_wizard->setDefaultProperty("QComboBox", "symbolCol", SIGNAL(currentIndexChanged(int)));
  m_wizard->setDefaultProperty("KComboBox", "dateCol", SIGNAL(currentIndexChanged(int)));
  m_wizard->setDefaultProperty("QComboBox", "dateCol", SIGNAL(currentIndexChanged(int)));

  m_parse = new Parse;
  m_parse->m_csvDialog = this;
  m_convertDate = new ConvertDate;

  m_pageIntro = new IntroPage;
  m_wizard->setPage(Page_Intro, m_pageIntro);
  m_pageIntro->setParent(this);


  m_investmentDlg = new InvestmentDlg;
  m_investmentDlg->m_investProcessing = m_investProcessing;
  m_investmentDlg->m_csvDialog = this;
  m_investProcessing->m_convertDat = m_convertDate;

  m_symbolTableDlg  = new SymbolTableDlg;
  m_symbolTableDlg->m_csvDialog = this;

  m_investProcessing->m_parse = m_parse;

  m_pageSeparator = new SeparatorPage;
  m_wizard->setPage(Page_Separator, m_pageSeparator);
  m_pageSeparator->setParent(this);

  m_pageBanking = new BankingPage;
  m_wizard->setPage(Page_Banking, m_pageBanking);
  m_pageBanking->setParent(this);

  m_pageInvestment = new InvestmentPage;
  m_wizard->setPage(Page_Investment, m_pageInvestment);
  m_pageInvestment->setParent(this);

  m_pageLinesDate = new LinesDatePage;
  m_wizard->setPage(Page_LinesDate, m_pageLinesDate);
  m_pageLinesDate->setParent(this);

  m_pageCompletion = new CompletionPage;
  m_wizard->setPage(Page_Completion, m_pageCompletion);
  m_pageCompletion->setParent(this);

  this->setAttribute(Qt::WA_DeleteOnClose, true);

  ui->tableWidget->setWordWrap(false);
  m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(-1);
  m_pageCompletion->ui->comboBox_thousandsDelimiter->setEnabled(false);

  m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);  //  This col might not get selected, so clear it
  m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);  // ditto
  m_pageSeparator->ui->comboBox_fieldDelimiter->setEnabled(false);

  m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);
  m_pageInvestment->ui->comboBoxInv_securityName->setInsertPolicy(QComboBox::InsertAlphabetically);
  m_pageInvestment->ui->comboBoxInv_securityName->setDuplicatesEnabled(false);

  m_setColor.setRgb(0, 255, 127, 100);
  m_errorColor.setRgb(255, 0, 127, 100);
  m_clearColor.setRgb(255, 255, 255, 255);
  m_colorBrush.setColor(m_setColor);
  m_clearBrush.setColor(m_clearColor);
  m_colorBrush.setStyle(Qt::SolidPattern);
  m_clearBrush.setStyle(Qt::SolidPattern);
  m_errorBrush.setColor(m_errorColor);
  m_errorBrush.setStyle(Qt::SolidPattern);

  for (int i = 0; i < MAXCOL; i++) {     //  populate comboboxes with col # values
    QString t;
    t.setNum(i + 1);
    m_pageBanking->ui->comboBoxBnk_numberCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_dateCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_payeeCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_memoCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_amountCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_creditCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_debitCol->addItem(t) ;

    m_columnType[i].clear();//          clear all column types
  }

  m_pageBanking->ui->comboBoxBnk_numberCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_dateCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_payeeCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_memoCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_amountCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_creditCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_debitCol->setMaxVisibleItems(12);

  m_vScrollBar = ui->tableWidget->verticalScrollBar();
  m_hScrollBar = ui->tableWidget->horizontalScrollBar();

  int screenWidth = QApplication::desktop()->width();
  int screenHeight = QApplication::desktop()->height();
  int x = (screenWidth - width()) / 2;
  int y = (screenHeight - height()) / 2;

  this->move(x, y);

  m_dateFormats << "yyyy/MM/dd" << "MM/dd/yyyy" << "dd/MM/yyyy";

  m_stageLabels << ui->label_intro << ui->label_separator << ui->label_banking << ui->label_investing << ui->label_lines << ui->label_finish;

  m_endColumn = MAXCOL;
  clearSelectedFlags();

  m_dateFormatIndex = m_pageLinesDate->ui->comboBox_dateFormat->currentIndex();
  m_date = m_dateFormats[m_dateFormatIndex];
  m_dateFormatIndex = m_dateFormatIndex;

  findCodecs();//                             returns m_codecs = codecMap.values();

  connect(m_hScrollBar, SIGNAL(valueChanged(int)), this, SLOT(slotHorScrollBarValue(int)));
  connect(m_vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(slotVertScrollBarValue(int)));

  connect(m_wizard->button(QWizard::CancelButton), SIGNAL(clicked()), this, SLOT(slotCancel()));

  connect(m_pageIntro->ui->radioButton_bank, SIGNAL(clicked()), m_pageIntro, SLOT(slotRadioButton_bankClicked()));
  connect(m_pageIntro->ui->radioButton_invest, SIGNAL(clicked()), m_pageIntro, SLOT(slotRadioButton_investClicked()));

  connect(m_pageBanking->ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(amountColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_amountCol, SIGNAL(activated(int)), this, SLOT(amountColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(debitColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_debitCol, SIGNAL(activated(int)), this, SLOT(debitColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(creditColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_creditCol, SIGNAL(activated(int)), this, SLOT(creditColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_numberCol, SIGNAL(currentIndexChanged(int)), this, SLOT(numberColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(payeeColumnSelected(int)));
  connect(m_pageBanking->ui->radioBnk_amount, SIGNAL(clicked(bool)), this, SLOT(amountRadioClicked(bool)));
  connect(m_pageBanking->ui->radioBnk_debCred, SIGNAL(clicked(bool)), this, SLOT(debitCreditRadioClicked(bool)));
  connect(m_pageBanking->ui->button_clear, SIGNAL(clicked()), this, SLOT(clearColumnsSelected()));

  connect(m_pageSeparator->ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(fieldDelimiterChanged()));

  connect(m_pageInvestment->ui->comboBoxInv_securityName, SIGNAL(activated(int)), m_pageInvestment, SLOT(slotsecurityNameChanged(int)));

  connect(m_pageLinesDate->ui->spinBox_skip, SIGNAL(editingFinished()), this, SLOT(startLineEdited()));
  connect(m_pageLinesDate->ui->spinBox_skip, SIGNAL(editingFinished()), m_investProcessing, SLOT(startLineChanged()));
  connect(m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endLineChanged(int)));
  connect(m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), m_investProcessing, SLOT(endLineChanged(int)));
  connect(m_pageLinesDate->ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(dateFormatSelected(int)));
  connect(m_pageLinesDate->ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_convertDate, SLOT(dateFormatSelected(int)));
  connect(m_pageLinesDate, SIGNAL(isImportable()), m_pageCompletion, SLOT(slotImportValid()));

  connect(m_pageCompletion->ui->comboBox_decimalSymbol, SIGNAL(currentIndexChanged(int)), m_parse, SLOT(decimalSymbolSelected(int)));
  connect(m_pageCompletion->ui->comboBox_decimalSymbol, SIGNAL(activated(int)), this, SLOT(decimalSymbolSelected(int)));
  connect(m_pageCompletion, SIGNAL(importBanking()), this, SLOT(slotImportClicked()));
  connect(m_pageCompletion, SIGNAL(importInvestment()), m_investProcessing, SLOT(slotImportClicked()));
  connect(m_pageCompletion, SIGNAL(completeChanged()), this, SLOT(slotClose()));


  connect(m_wizard->button(QWizard::CustomButton1), SIGNAL(clicked()), this, SLOT(slotFileDialogClicked()));
  connect(m_wizard->button(QWizard::BackButton), SIGNAL(clicked()), this, SLOT(slotBackButtonClicked()));
  connect(m_wizard->button(QWizard::CustomButton2), SIGNAL(clicked()), m_pageCompletion, SLOT(slotImportClicked()));
  connect(m_wizard->button(QWizard::CustomButton3), SIGNAL(clicked()), this, SLOT(slotSaveAsQIF()));
  connect(m_wizard->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(slotClose()));
  connect(m_wizard, SIGNAL(currentIdChanged(int)), this, SLOT(slotIdChanged(int)));

  connect(this, SIGNAL(isImportable()), m_pageCompletion, SLOT(slotImportValid()));
  connect(this, SIGNAL(namesEdited()), this, SLOT(slotNamesEdited()));

  m_investmentDlg->init();

}//  CSVDialog

CSVDialog::~CSVDialog()
{
  delete ui;
  delete m_symbolTableDlg;
}

void CSVDialog::readSettingsProfiles()
{
  KSharedConfigPtr  newConfig = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
  KConfigGroup newBankGroup(newConfig, "BankProfiles");
  if (newBankGroup.exists()) {     //  If local config file exists, exit
    return;
  }
  KSharedConfigPtr  config = KSharedConfig::openConfig(KStandardDirs::locate("config", "csvimporterrc"));
  KConfigGroup bankGroup(config, "BankProfiles");

  QStringList lst = bankGroup.readEntry("BankNames", QStringList());
  foreach (const QString & group, lst) {
    bankGroup.copyTo(&newBankGroup);
    newBankGroup.config()->sync();

    QString txt = "Profiles-" + group;
    KConfigGroup profilesGroup(config, txt);
    KConfigGroup newProfilesGroup(newConfig, txt);
    profilesGroup.copyTo(&newProfilesGroup);
    newProfilesGroup.config()->sync();
  }
  KConfigGroup securitiesGroup(config, "Securities");
  KConfigGroup newSecuritiesGroup(newConfig, "Securities");
  securitiesGroup.copyTo(&newSecuritiesGroup);
  newSecuritiesGroup.config()->sync();
}

void CSVDialog::readSettingsInit()
{
  m_pageIntro->m_index = 0;
  KSharedConfigPtr  myconfig = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
  KConfigGroup bankProfilesGroup(myconfig, "BankProfiles");

  m_profileList.clear();
  m_pageIntro->ui->combobox_source->clear();
  m_pageIntro->ui->combobox_source->addItem(i18n("Add New Profile"));
  QStringList list = bankProfilesGroup.readEntry("BankNames", QStringList());
  if (!list.isEmpty()) {
    for (int i = 0; i < list.count(); i++) {
      m_profileList.append(list[i]);
      QString txt = "Profiles-" + list[i];
      KConfigGroup profilesGroup(myconfig, txt);

      if (profilesGroup.exists()) {
        txt = profilesGroup.readEntry("FileType", QString());
        m_pageIntro->m_mapFileType.insert(list[i], txt);
        if (txt == m_fileType) {
          m_pageIntro->ui->combobox_source->addItem(list[i]);
          m_pageIntro->m_map.insert(list[i], m_pageIntro->m_index++);
        }
      }
    }
  }
  if (m_fileType == "Banking") {
    m_priorCsvProfile = bankProfilesGroup.readEntry("PriorCsvProfile", QString());
    if (m_priorCsvProfile.isEmpty()) {
      m_pageIntro->ui->combobox_source->setCurrentIndex(0);
    } else {
      m_profileName = m_priorCsvProfile;
      int indx = m_pageIntro->ui->combobox_source->findText(m_priorCsvProfile);
      m_pageIntro->ui->combobox_source->setCurrentIndex(indx);
      m_pageIntro->m_index = indx;
    }
  } else if (m_fileType == "Invest") {
    m_priorInvProfile = bankProfilesGroup.readEntry("PriorInvProfile", QString());
    if (m_priorInvProfile.isEmpty()) {
      m_pageIntro->ui->combobox_source->setCurrentIndex(0);
    } else {
      int indx = m_pageIntro->ui->combobox_source->findText(m_priorInvProfile);
      m_pageIntro->ui->combobox_source->setCurrentIndex(indx);
      m_pageIntro->m_index = indx;
      m_profileName = m_priorInvProfile;
    }
  }
  disconnect(m_pageIntro->ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), m_pageIntro, SLOT(slotLineEditingFinished()));
}

void CSVDialog::readColumnSettings()
{
  m_profileExists = false;
  int tmp;
  QString txt;
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locate("config", "csvimporterrc"));
  bool found = false;
  for (int i = 0; i < m_profileList.count(); i++) {
    if (m_profileList[i] != m_profileName) {
      continue;
    } else {
      found = true;
    }
    if (!found) {
      return;
    }
    m_profileExists = true;
    QString txt = "Profiles-" + m_profileList[i];

    KConfigGroup profilesGroup(config, txt);
    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(profilesGroup.readEntry("DateCol", -1));
    m_payeeColumn = profilesGroup.readEntry("PayeeCol", -1);
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(m_payeeColumn);

    QList<int> list = profilesGroup.readEntry("MemoCol", QList<int>());
    int posn = 0;
    if ((posn = list.indexOf(-1)) > -1) {
      list.removeOne(-1);
    }
    m_memoColList = list;
    //
    //  Set up all memo fields...
    //
    for (int i = 0; i < m_memoColList.count(); i++) {
      tmp = m_memoColList[i];
      m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(tmp, QString().setNum(tmp + 1) + '*');
      if (tmp == m_payeeColumn) {  //  ...unless also a payee field.
        m_payeeColCopied = true;
        continue;
      } else {
        m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(tmp);
      }
      m_memoColumn = tmp;
      columnType(tmp) = "memo";
    }
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(profilesGroup.readEntry("DebitCol", -1));
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(profilesGroup.readEntry("CreditCol", -1));
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(profilesGroup.readEntry("NumberCol", -1));
  }
}

void CSVDialog::readSettings()
{
  m_profileExists = false;
  int tmp;
  QString txt;
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locate("config", "csvimporterrc"));
  bool found = false;
  for (int i = 0; i < m_profileList.count(); i++) {
    if (m_profileList[i] != m_profileName) {
      continue;
    } else {
      found = true;
    }
    if (!found) {
      return;
    }
    disconnect(m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
    m_profileExists = true;
    QString txt = "Profiles-" + m_profileList[i];

    KConfigGroup profilesGroup(config, txt);

    //    txt = profilesGroup.readEntry("FileType", QString());//  Read earlier in readSettingsInit()
    m_dateFormatIndex = profilesGroup.readEntry("DateFormat", -1);
    m_pageLinesDate->ui->comboBox_dateFormat->setCurrentIndex(m_dateFormatIndex);

    //    m_encodeIndex = profilesGroup.readEntry("Encoding", 0);//  Read earlier in fileDialog()

    m_startLine = profilesGroup.readEntry("StartLine", -1) + 1;
    m_pageLinesDate->ui->spinBox_skip->setValue(m_startLine);

    m_fieldDelimiterIndex = profilesGroup.readEntry("FieldDelimiter", 0);
    m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(m_fieldDelimiterIndex);

    m_textDelimiterIndex = profilesGroup.readEntry("TextDelimiter", 0);
    m_pageSeparator->ui->comboBox_textDelimiter->setCurrentIndex(m_textDelimiterIndex);

    //    m_csvPath = profilesGroup.readEntry("CsvDirectory", QString());//  Read earlier in fileDialog()

    m_debitFlag = profilesGroup.readEntry("DebitFlag", -1);

    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);

    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(profilesGroup.readEntry("DateCol", -1));
    m_payeeColumn = profilesGroup.readEntry("PayeeCol", -1);
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(m_payeeColumn);
    QList<int> list = profilesGroup.readEntry("MemoCol", QList<int>());
    int posn = 0;
    if ((posn = list.indexOf(-1)) > -1) {
      list.removeOne(-1);
    }
    m_memoColList = list;
    //
    //  Set up all memo fields...
    //
    for (int i = 0; i < m_memoColList.count(); i++) {
      tmp = m_memoColList[i];
      m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(tmp, QString().setNum(tmp + 1) + '*');
      m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(tmp);
      if (tmp == m_payeeColumn) {  //  ...unless also a payee field.
        m_payeeColCopied = true;
        continue;
      }
      m_memoColumn = tmp;
      columnType(tmp) = "memo";
    }
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(profilesGroup.readEntry("NumberCol", -1));

    m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(-1);  //  Ensure UI gets changed.
    m_decimalSymbolIndex = profilesGroup.readEntry("DecimalSymbol", 0);
    m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(m_decimalSymbolIndex);
    m_pageCompletion->ui->comboBox_thousandsDelimiter->setCurrentIndex(-1);
    break;
  }
  connect(m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
}

void CSVDialog::createProfile(QString newName)
{
  KSharedConfigPtr  config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
  KConfigGroup bankProfilesGroup(config, "BankProfiles");

  bankProfilesGroup.writeEntry("BankNames", m_profileList);
  bankProfilesGroup.config()->sync();

  KConfigGroup bankGroup(config, "BankProfiles");
  QString txt = "Profiles-" + newName;

  KConfigGroup profilesGroup(config, "Profiles-New Profile###");

  KSharedConfigPtr  configBackup = KSharedConfig::openConfig(KStandardDirs::locate("config", "csvimporterrc"));
  KConfigGroup bkprofilesGroup(configBackup, "Profiles-New Profile###");

  KConfigGroup newProfilesGroup(config, txt);
  bkprofilesGroup.copyTo(&newProfilesGroup);
  newProfilesGroup.writeEntry("FileType", m_fileType);
  if (m_fileType == "Invest") {
    m_investProcessing->m_shrsinList = bkprofilesGroup.readEntry("ShrsinParam", QStringList());
    newProfilesGroup.writeEntry("ShrsinParam", m_investProcessing->m_shrsinList);
    m_investProcessing->m_divXList = bkprofilesGroup.readEntry("DivXParam", QStringList());
    newProfilesGroup.writeEntry("DivXParam", m_investProcessing->m_divXList);
    m_investProcessing->m_intIncList = bkprofilesGroup.readEntry("IntIncParam", QStringList());
    newProfilesGroup.writeEntry("IntIncParam", m_investProcessing->m_intIncList);
    m_investProcessing->m_brokerageList = bkprofilesGroup.readEntry("BrokerageParam", QStringList());
    newProfilesGroup.writeEntry("BrokerageParam", m_investProcessing->m_brokerageList);
    m_investProcessing->m_reinvdivList = bkprofilesGroup.readEntry("ReinvdivParam", QStringList());
    newProfilesGroup.writeEntry("ReinvdivParam", m_investProcessing->m_reinvdivList);
    m_investProcessing->m_buyList = bkprofilesGroup.readEntry("BuyParam", QStringList());
    newProfilesGroup.writeEntry("BuyParam", m_investProcessing->m_buyList);
    m_investProcessing->m_sellList = bkprofilesGroup.readEntry("SellParam", QStringList());
    newProfilesGroup.writeEntry("SellParam", m_investProcessing->m_sellList);
    m_investProcessing->m_removeList = bkprofilesGroup.readEntry("RemoveParam", QStringList());
    newProfilesGroup.writeEntry("RemoveParam", m_investProcessing->m_removeList);
  }
  newProfilesGroup.config()->sync();
}

void CSVDialog::slotFileDialogClicked()
{
  if ((m_fileType != "Banking") || (m_profileName.isEmpty())){
    return;
  }
  clearColumnTypes();//  Needs to be here in case user selects new profile after cancelling prior one.
  m_inFileName.clear();
  m_pageLinesDate->m_isColumnSelectionComplete = false;
  m_firstPass = true;
  m_firstRead = true;
  bool found = false;
  m_memoColCopied = false;
  m_payeeColCopied = false;
  m_columnsNotSet = true;  //  Don't check columns until they've been selected.
  m_pageBanking->m_bankingPageInitialized  = false;
  m_maxColumnCount = 0;
  m_fileEndLine = 0;
  m_maxRowWidth = 0;
  ui->tableWidget->verticalScrollBar()->setValue(0);

  QString profileName;
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locate("config", "csvimporterrc"));
  for (int i = 0; i < m_profileList.count(); i++) {
    if (m_profileList[i] != m_profileName) {
      continue;
    } else {
      found = true;
      profileName = "Profiles-" + m_profileList[i];
    }
  }
  if (!found) {
    return;
  }
  KConfigGroup profilesGroup(config, profileName);
  m_csvPath = profilesGroup.readEntry("CsvDirectory", QString());
  m_encodeIndex = profilesGroup.readEntry("Encoding", 0);
  m_pageLinesDate->m_trailerLines = profilesGroup.readEntry("TrailerLines", 0);
  m_fieldDelimiterIndex = profilesGroup.readEntry("FieldDelimiter", 0);
  m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(m_fieldDelimiterIndex);

  //  Set these column values here so they're available in time
  //  to align the columns in first displayLine().
  //  readSettings() is too late.
  //
  m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
  m_debitColumn = profilesGroup.readEntry("DebitCol", -1);
    if (m_debitColumn == -1) {      //                            If amount previously selected, set check radio_amount
      m_pageBanking->ui->radioBnk_amount->setChecked(true);
      m_pageBanking->ui->labelBnk_amount->setEnabled(true);
      m_pageBanking->ui->labelBnk_credits->setEnabled(false);
      m_pageBanking->ui->labelBnk_debits->setEnabled(false);
    } else {//                                                    ....else set check radio_debCred to clear amount col
      m_pageBanking->ui->radioBnk_debCred->setChecked(true);
      m_pageBanking->ui->labelBnk_credits->setEnabled(true);
      m_pageBanking->ui->labelBnk_debits->setEnabled(true);
      m_pageBanking->ui->labelBnk_amount->setEnabled(false);
    }
  m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(m_debitColumn);

  m_creditColumn = profilesGroup.readEntry("CreditCol", -1);
  m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(m_creditColumn);

  m_amountColumn = profilesGroup.readEntry("AmountCol", -1);
  m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(m_amountColumn);

  m_pageCompletion->ui->comboBox_decimalSymbol->setEnabled(true);

  m_startLine = 1;
  m_endLine = 0;
  //  The "DebitFlag" setting is used to indicate whether or not to allow the user,
  //  via a dialog, to specify a column which contains a flag to indicate if the
  //  amount field is a debit ('a' or 'af'), a credit ('bij') (ING - Netherlands),
  //   or ignore ('-1').
  m_flagCol = -1;
  m_debitFlag = -1;

  int posn;
  if (m_csvPath.isEmpty()) {
    m_csvPath = "~/";
  }
  QPointer<KFileDialog> dialog = new KFileDialog(KUrl(m_csvPath),
                    i18n("*.csv *.PRN *.txt | CSV Files\n *|All files"), 0);

  //  Add encoding selection to FileDialog
  KHBox* encodeBox = new KHBox();
  m_comboBoxEncode = new KComboBox(encodeBox);
  m_comboBoxEncode->setCurrentIndex(m_encodeIndex);
  setCodecList(m_codecs);
  connect(m_comboBoxEncode, SIGNAL(activated(int)), this, SLOT(encodingChanged(int)));

  dialog->fileWidget()->setCustomWidget("Encoding", m_comboBoxEncode);
  m_comboBoxEncode->setCurrentIndex(m_encodeIndex);
  dialog->setMode(KFile::File | KFile::ExistingOnly);
  if (dialog->exec() == QDialog::Accepted) {
    m_url = dialog->selectedUrl();
  }
  delete dialog;

  if (m_url.isEmpty())
    return;
  m_inFileName.clear();

  if (!KIO::NetAccess::download(m_url, m_inFileName, 0)) {
    KMessageBox::detailedError(0, i18n("Error while loading file '%1'.", m_url.prettyUrl()),
                               KIO::NetAccess::lastErrorString(),
                               i18n("File access error"));
    return;
  }
  if (m_inFileName.isEmpty()) {
    return;
  }
  m_importNow = false;//                       Avoid attempting date formatting on headers
  m_acceptAllInvalid = false;  //              Don't accept further invalid values.
  clearComboBoxText();//                       to clear any '*' in memo combo text

  m_parse->setSymbolFound(false);
  readFile(m_inFileName, m_startLine);
  m_csvPath = m_inFileName;
  posn = m_csvPath.lastIndexOf("/");
  m_csvPath.truncate(posn + 1);   //           keep last "/"

  readSettings();
  QString str = "~/" + m_csvPath.section('/', 3);
  KConfigGroup dirGroup(config, profileName);
  if (m_pageIntro->ui->checkBoxSkipSetup) {
    dirGroup.writeEntry("CsvDirectory", str);  //          save selected path
    dirGroup.writeEntry("Encoding", m_encodeIndex);  //    ..and encoding
    dirGroup.writeEntry("FileType", m_fileType);  //       ..and fileType
    dirGroup.config()->sync();
  }
  enableInputs();

  int index = m_pageCompletion->ui->comboBox_decimalSymbol->currentIndex();
  decimalSymbolSelected(index);

  //The following two items do not *Require* an entry so old values must be cleared.
  m_trData.number.clear();  //                 this needs to be cleared or gets added to next transaction
  m_trData.memo.clear();  //                   this too, as neither might be overwritten by new data.

  if (m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    m_pageCompletion->initializePage();//      Using a profile so skip setup and go to Completion.
  } else {
    m_wizard->next();
  }
}

void CSVDialog::readFile(const QString& fname, int skipLines)
{
  MyMoneyStatement st = MyMoneyStatement();
  if (!fname.isEmpty()) {
    m_inFileName = fname;
  }
  ui->tableWidget->clear();//         including vert headers
  m_inBuffer.clear();
  m_outBuffer.clear();

  m_qifBuffer = "!Type:Bank\n";
  m_row = 0;

  m_fieldDelimiterIndex = m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex();
  m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_textDelimiterIndex = m_pageSeparator->ui->comboBox_textDelimiter->currentIndex();
  m_parse->setTextDelimiterIndex(m_textDelimiterIndex);
  m_textDelimiterCharacter = m_parse->textDelimiterCharacter(m_textDelimiterIndex);

  QFile  m_inFile(m_inFileName);
  m_inFile.open(QIODevice::ReadOnly | QIODevice::Text);

  QTextStream inStream(&m_inFile);
  QTextCodec* codec = QTextCodec::codecForMib(m_encodeIndex);
  inStream.setCodec(codec);

  QString buf = inStream.readAll();
  disconnect(m_pageLinesDate->ui->spinBox_skipToLast, 0, 0, 0);

  //  Parse the buffer

  m_columnCountList.clear();
  QString data;
  QStringList lineList = m_parse->parseFile(buf, skipLines, 0);  //  Changed to display whole file.
  for (int i = 0; i < lineList.count(); i++) {
    data = lineList[i];
    m_columnList = m_parse->parseLine(data);
    int columnCount = m_columnList.count();
    m_columnCountList << columnCount;  //                            Number of columns in each line.
    m_lineNum = i;
  }
  m_lineNum = 0;
  if (m_firstRead) {
    //  Check all lines to find maximum column count.

    for (int i = 0; i < lineList.count(); i++) {
      QString data = lineList[i];
      m_columnList = m_parse->parseLine(data);
      int columnCount = m_columnList.count();

      if (columnCount > m_maxColumnCount)
        m_maxColumnCount = columnCount;
      else
        columnCount = m_maxColumnCount;
    }
    ui->tableWidget->setColumnCount(m_maxColumnCount + 1);
    m_firstRead = false;
    if (m_fileEndLine == 0) {
      m_fileEndLine = m_parse->lastLine();
    }
    if (m_fileEndLine > m_pageLinesDate->m_trailerLines) {
      m_endLine = m_fileEndLine - m_pageLinesDate->m_trailerLines;
    } else {
      m_endLine = m_fileEndLine;  //                                 Ignore m_trailerLines as > file length.
    }
    m_pageLinesDate->ui->spinBox_skip->setMaximum(m_fileEndLine);
    m_pageLinesDate->ui->spinBox_skipToLast->setMaximum(m_fileEndLine);
  }
  m_pageLinesDate->ui->spinBox_skipToLast->setValue(m_endLine);
  if (m_endLine < m_startLine) {  //                                 Don't have m_endLine < m_startLine
    m_endLine = m_startLine;
  }
  ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  m_screenUpdated = false;

  connect(m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endLineChanged(int)));

  //  Display the buffer

  for (int i = 0; i < lineList.count(); i++) {
    m_inBuffer = lineList[i];
    m_rowWidth = 0;
    displayLine(m_inBuffer, i);
    if (m_rowWidth > m_maxRowWidth) {
      m_maxRowWidth = m_rowWidth;
    }

    //  user now ready to continue && line is in wanted range
    //
    if ((m_importNow) && (i <= m_pageLinesDate->ui->spinBox_skipToLast->value() - m_startLine)) {
      int ret = (processQifLine(m_inBuffer));  //  parse a line
      if (ret == KMessageBox::Ok) {
        csvImportTransaction(st);
      } else
        m_importNow = false;
    }
  }//                                            reached end of buffer

  //  Adjust table size (drop header lines)

  updateScreen();

  ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  m_pageLinesDate->ui->labelSet_skip->setEnabled(true);
  m_pageLinesDate->ui->spinBox_skip->setEnabled(true);
  m_endColumn = m_maxColumnCount;

  //  Export statement

  if (m_importNow) {
    emit statementReady(st);  //  via CsvImporterPlugin::slotGetStatement(MyMoneyStatement& s)
    m_screenUpdated = true;
    m_importNow = false;
    // the life cycle of the contents of this map is one import process
    m_hashMap.clear();
  }
  m_inFile.close();
}


void CSVDialog::displayLine(const QString& data, int row)
{
  QBrush brush;
  QColor colr;
  colr.setRgb(255, 0, 127, 100);
  brush.setColor(colr);
  brush.setStyle(Qt::SolidPattern);

  if (m_importNow) {
    if (m_pageBanking->ui->radioBnk_amount->isChecked()) {
      m_amountColumn = m_pageBanking->ui->comboBoxBnk_amountCol->currentIndex();// setAmountColumn
      m_debitColumn = -1;// setDebitColumn
      m_creditColumn = -1;
    } else {
      m_amountColumn = -1;
      m_debitColumn = m_pageBanking->ui->comboBoxBnk_debitCol->currentIndex();
      m_creditColumn = m_pageBanking->ui->comboBoxBnk_creditCol->currentIndex();
    }
  }
  int col = 0;
  m_parse->setFieldDelimiterIndex(m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex());
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_parse->setTextDelimiterIndex(m_pageSeparator->ui->comboBox_textDelimiter->currentIndex());
  m_textDelimiterCharacter = m_parse->textDelimiterCharacter(m_textDelimiterIndex);
  //
  //                 split data into fields
  //
  m_columnList = m_parse->parseLine(data);
  m_endColumn = m_maxColumnCount;
  if ((!m_firstPass) && (m_payeeColumn <= m_columnCountList[m_lineNum]) && (m_memoColumn <= m_columnCountList[m_lineNum])) {
    if ((m_memoColCopied) && (m_memoColCopy < m_columnList.count())){  //           but exclude short header lines
      m_columnList<<m_columnList[m_memoColCopy];
    } else if ((m_payeeColCopied) && (m_payeeColumn < m_columnList.count())) {  //  but exclude short header lines
      m_columnList<<m_columnList[m_payeeColumn];
      m_columnType[m_columnList.count() - 1] = "memo";
      for (int i = 0; i < m_memoColList.count(); i++) {
        if (m_memoColList[i] == m_payeeColumn) {
          continue;
        }
        m_columnType[m_memoColList[i]] = "memo";
      }
    }
  }
  m_inBuffer.clear();
  QStringList::const_iterator constIterator;
  QString txt;
  for (constIterator = m_columnList.constBegin(); constIterator != m_columnList.constEnd(); ++constIterator) {
    txt = (*constIterator);
    QTableWidgetItem *item = new QTableWidgetItem;//       new item for UI
    if ((col == m_creditColumn) || (col == m_debitColumn) || (col == m_amountColumn)) {
      item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    item->setText(txt);
    if (row + m_startLine > m_pageLinesDate->ui->spinBox_skipToLast->value()) {
      item->setBackground(brush);  //                      highlight unwanted lines instead of not showing them
    }
    ui->tableWidget->setRowCount(m_row + 1);
    ui->tableWidget->setItem(m_row, col, item);  //        add items to UI here
    ui->tableWidget->resizeColumnToContents(col);
    m_rowWidth += ui->label->sizeHint().width();
    m_inBuffer += txt + m_fieldDelimiterCharacter;
    col ++;
  }

  //  if last char. of last column added to UI (txt string) is not '"', ie an unterminated string
  //  remove the unwanted trailing m_fieldDelimiterCharacter
  if (!txt.endsWith('"')) {
    m_inBuffer = m_inBuffer.remove(-1, 1);
  }
  ++m_row;
  ++m_lineNum;
}

int CSVDialog::processQifLine(QString& iBuff)  //   parse input line
{
  QString newTxt;
  bool firstField = true;
  QString firstValue = QString();

  if (m_columnList.count() < m_endColumn) {
    if (!m_accept) {
      QString row = QString::number(m_row);
      int ret = KMessageBox::questionYesNoCancel(this, i18n("<center>Row number %1 does not have the expected number of columns.</center>"
                "<center>This might not be a problem, but it may be a header line.</center>"
                "<center>You may accept all similar items, or just this one, or cancel.</center>",
                row), i18n("CSV import"),
                KGuiItem(i18n("Accept All")),
                KGuiItem(i18n("Accept This")),
                KGuiItem(i18n("Cancel")));
      if (ret == KMessageBox::Cancel) {
        return ret;
      }
      if (ret == KMessageBox::Yes) {
        m_accept = true;
      }
    }
  }
  int neededFieldsCount = 0;//                          ensure essential fields are present
  QString memo;
  QString txt;
  iBuff = iBuff.remove(m_textDelimiterCharacter);
  memo.clear();//                                       memo & number may not have been used
  m_trData.number.clear();//                            .. so need to clear prior contents
  for (int i = 0; i < m_columnList.count(); i++) {  //  check each column
    //  Use actual column count for this line instead of m_endColumn, which could be greater.
    if (columnType(i) == "number") {
      txt = m_columnList[i];
      m_trData.number = txt;
      m_qifBuffer = m_qifBuffer + 'N' + txt + '\n';  // Number column
    } else if (columnType(i) == "date") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt = txt.remove(m_textDelimiterCharacter);  //   "16/09/2009
      QDate dat = m_convertDate->convertDate(txt);  //  Date column
      if (dat == QDate()) {
        KMessageBox::sorry(this, i18n("<center>An invalid date has been detected during import.</center>"
                                      "<center><b>%1</b></center>"
                                      "Please check that you have set the correct date format."
                                      , txt), i18n("CSV import"));
        m_importError = true;
        return KMessageBox::Cancel;
      }
      QString qifDate = dat.toString(m_dateFormats[m_dateFormatIndex]);
      m_qifBuffer = m_qifBuffer + 'D' + qifDate + '\n';
      m_trData.date = dat;
    } else if (columnType(i) == "payee") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt.remove('~');  //                              replace NL which was substituted
      txt = txt.remove('\'');
      if ((!m_firstPass) && (m_memoColCopied)) {
         m_columnList[m_payeeColumn] = txt ;
         columnType(m_memoColumn) = "memo";
      }
      m_trData.payee = txt;
      m_qifBuffer = m_qifBuffer + 'P' + txt + '\n';// Detail column
    }

    else if (columnType(i) == "amount") {      // Is this Amount column
      ++neededFieldsCount;

      //  For a file which uses a flag field value to indicate if amount is a debit or a credit.
      //  Resource file DebitFlag setting of -1 means 'ignore/notused'.
      //  DebitFlag setting of >=0 indicates the column containing the flag.

      if (m_flagCol == -1) {      //                        it's a new file
        switch (m_debitFlag) {  //                      Flag if amount is debit or credit
          case -1://                                  Ignore flag
            m_flagCol = 0;//                          ...and continue
            break;
          case  0://                                  Ask for column no.of flag
            m_flagCol = columnNumber(i18n("Enter debit flag column number"));
            if (m_flagCol == 0) {      //                  0 means Cancel was pressed
              return KMessageBox::Cancel;//           ... so exit
            }
            break;
          default : m_flagCol = m_debitFlag;//        Contains flag/column no.
        }
      }
      if ((m_flagCol < 0) || (m_flagCol > m_endColumn)) {      // shouldn't get here
        KMessageBox::sorry(0, i18n("An invalid column was entered.\n"
                                   "Must be between 1 and %1.", m_endColumn), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      QString flag;//                                 m_flagCol == valid column (or zero)
      if (m_flagCol > 0) {
        flag = m_columnList[m_flagCol - 1];//         indicates if amount is debit or credit
      }//                                             if flagCol == 0, flag is empty

      txt = m_columnList[i];//                        amount column value
      if ((m_amountColumn == i) &&
          (((txt.contains("("))) || (flag.startsWith('A')))) {     //  "(" or "Af" = debit
        txt = txt.remove(QRegExp("[()]"));
        txt = '-' + txt;  //                          Mark as -ve
      } else if (m_debitColumn == i) {
        txt = '-' + txt;  //                          Mark as -ve
      }
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      m_trData.amount = newTxt;
      m_qifBuffer = m_qifBuffer + 'T' + newTxt + '\n';
    }

    else if ((columnType(i) == "debit") || (columnType(i) == "credit")) {      //  Credit or debit?
      ++neededFieldsCount;
      txt = m_columnList[i];
      if ((!txt.isEmpty()) && ((i == m_debitColumn))) {
        txt = '-' + txt;//                                     Mark as -ve
      }
      if (firstField) {  //                                    Debit or credit, whichever comes first.
        firstValue = txt;  //                                  Save field until second arrives.
      } else {  //                                             Second field.
        if (txt.isEmpty()) {  //                               If second field empty,...
          txt = firstValue;  //                                ...use first (which could also be empty..)
        }
      }
      if (txt.isEmpty()) {
        txt = '0';
      }
      if (!firstField) {  //                                   Process outcome.
        newTxt = m_parse->possiblyReplaceSymbol(txt);
        m_trData.amount = newTxt;
        m_qifBuffer = m_qifBuffer + 'T' + newTxt + '\n';
      }
      firstField = !firstField;
    }

    else if (columnType(i) == "memo") {      //         could be more than one
      txt = m_columnList[i];
      txt.replace('~', "\n");  //                       replace NL which was substituted
      if ((!m_firstPass) && (txt.isEmpty()) && (m_payeeColCopied)) {
         txt = m_columnList[m_payeeColumn];
         m_columnList[i] = txt;
      }
      if (!memo.isEmpty())
        memo += '\n';//                                 separator for multiple memos
      memo += txt;//                                    next memo
    }//end of memo field
  }//end of col loop
  m_trData.memo = memo;

  QString hashBase;
  hashBase.sprintf("%s-%07lx", qPrintable(m_trData.date.toString(Qt::ISODate)), MyMoneyTransaction::hash(iBuff));
  int idx = 1;
  QString hash;
  for (;;) {
    hash = QString("%1-%2").arg(hashBase).arg(idx);
    QMap<QString, bool>::const_iterator it;
    it = m_hashMap.constFind(hash);
    if (it == m_hashMap.constEnd()) {
      m_hashMap[hash] = true;
      break;
    }
    ++idx;
  }
  m_trData.id = hash;

  m_qifBuffer = m_qifBuffer + 'M' + memo + '\n' + "^\n";
  if (neededFieldsCount > 2) {
    return KMessageBox::Ok;
  } else {
    KMessageBox::sorry(0, i18n("<center>The columns selected are invalid.\n</center>"
                               "There must an amount or debit and credit fields, plus date and payee fields."), i18n("CSV import"));
    m_importError = true;
    return KMessageBox::Cancel;
  }
}


void CSVDialog::csvImportTransaction(MyMoneyStatement& st)
{
  MyMoneyStatement::Transaction tr;
  QString tmp;
  QString payee = m_trData.payee;//                              extractLine('P')

  // Process transaction data
  tr.m_strBankID = m_trData.id;
  st.m_eType = MyMoneyStatement::etCheckings;
  tr.m_datePosted = m_trData.date;
  if (!tr.m_datePosted.isValid()) {
    int rc = KMessageBox::warningContinueCancel(0, i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
             "date format setting of \"%2.\"" "\n\nPressing \'Continue\' will "
             "assign today's date to the transaction. Pressing \'Cancel\'' will abort "
             "the import operation. You can then restart the import and select a different "
             "date format.", m_trData.date.toString(m_date), m_dateFormats[m_dateFormatIndex]), i18n("Invalid date format"));
    switch (rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = (QDate::currentDate());
        break;

      case KMessageBox::Cancel:
        m_importNow = false;//                             Don't process statement
        st = MyMoneyStatement();
        m_importError = true;
        return ;
    }
  }
  tr.m_amount = MyMoneyMoney(m_trData.amount);
  tr.m_shares = MyMoneyMoney(m_trData.amount);

  tmp = m_trData.number;
  tr.m_strNumber = tmp;

  if (!payee.isEmpty()) {
    tr.m_strPayee = m_trData.payee;
  }

  tr.m_strMemo = m_trData.memo;
  // Add the transaction to the statement
  st.m_listTransactions += tr;
  if ((st.m_listTransactions.count()) > 0) {
    statements += st;// this not used
    qDebug("Statement with %d transactions ready.",
           st.m_listTransactions.count());
  }
  // Now to import the statements
  return;
}


void CSVDialog::slotImportClicked()
{
  m_importError = false;

  if (m_fileType != "Banking") {
    return;
  }
  if ((m_dateSelected) && (m_payeeSelected) &&
      ((m_amountSelected || (m_debitSelected && m_creditSelected)))) {
    m_importNow = true; //                          all necessary data is present

    int skp = m_pageLinesDate->ui->spinBox_skip->value();
    if (skp > m_endLine) {
      KMessageBox::sorry(0, i18n("<center>The start line is greater than the end line.\n</center>"
                                 "<center>Please correct your settings.</center>"), i18n("CSV import"));
      m_importError = true;
      return;
    }
    m_parse->setSymbolFound(false);
    readFile(m_inFileName, skp);   //               skip all headers
    updateRowHeaders(skp);         //--- create the (revised) vertical (row) headers ---
  } else {
    QString errMsg = i18n("<center>An Amount-type column, and Date and Payee columns are needed.</center>");
    if (m_pageIntro->ui->checkBoxSkipSetup->isEnabled()) {
      errMsg += i18n("<center>As you had skipped Setup, the wizard will now return you to the setups.</center>");
    }
    KMessageBox::information(0, errMsg);
    m_importError = true;
  }
}

void CSVDialog::updateRowHeaders(int skp)
{
 //--- create the (revised) vertical (row) headers ---

  QStringList vertHeaders;
  for (int i = skp; i < ui->tableWidget->rowCount() + skp; i++) {
    QString hdr = (QString::number(i));
    vertHeaders += hdr;
  }
  //  verticalHeader()->width() varies with its content so....

  ui->tableWidget->setVerticalHeaderLabels(vertHeaders);
  ui->tableWidget->hide();//             to ensure....
  ui->tableWidget->show();//             ..vertical header width redraws
}

void CSVDialog::slotSaveAsQIF()
{
  if (m_fileType == QLatin1String("Banking")) {
    QStringList outFile = m_inFileName.split('.');
    const KUrl& name = QString((outFile.isEmpty() ? "CsvProcessing" : outFile[0]) + ".qif");

    QString outFileName = KFileDialog::getSaveFileName(name, QString::fromLatin1("*.qif | %1").arg(i18n("QIF Files")), 0, i18n("Save QIF")
#if KDE_IS_VERSION(4,4,0)
                          , KFileDialog::ConfirmOverwrite
#endif
                                                      );

    QFile oFile(outFileName);
    oFile.open(QIODevice::WriteOnly);
    QTextStream out(&oFile);
    out << m_qifBuffer;// output qif file
    oFile.close();
  }//else
}


void CSVDialog::setCodecList(const QList<QTextCodec *> &list)
{
  m_comboBoxEncode->clear();
  foreach (QTextCodec * codec, list) {
    m_comboBoxEncode->addItem(codec->name(), codec->mibEnum());
  }
}


int CSVDialog::columnNumber(const QString& msg)
{
  //  This dialog box is for use with the debit/credit flag resource file entry,
  //  indicating the sign of the value column. ie a debit or a credit.
  bool ok;
  static int ret;
  ret = KInputDialog::getInteger(i18n("Enter column number of debit/credit code"), msg, 0, 1, m_endColumn, 1, 10, &ok);
  if (ok && ret > 0)
    return ret;
  return 0;
}

void CSVDialog::clearColumnsSelected()
{
  if (m_fileType == "Banking") {
    clearPreviousColumn();
    clearSelectedFlags();
    clearColumnNumbers();
    clearComboBoxText();
    m_memoColCopied = false;
    m_payeeColCopied = false;
    m_memoColList.clear();
  }
}

void CSVDialog::clearSelectedFlags()
{
  for (int i = 0; i < MAXCOL; i++)
    clearColumnType(i);   //   set to all empty

  m_dateSelected = false;
  m_payeeSelected = false;
  m_amountSelected = false;
  m_debitSelected = false;
  m_creditSelected = false;
  m_memoSelected = false;
  m_numberSelected = false;
  m_pageBanking->ui->radioBnk_amount->setEnabled(true);
  m_pageBanking->ui->radioBnk_debCred->setEnabled(true);
}

void CSVDialog::clearColumnNumbers()
{
  m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
}

void CSVDialog::clearComboBoxText()
{
  for (int i = 0; i < MAXCOL; i++) {
    m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(i, QString().setNum(i + 1));
  }
}

void CSVDialog::clearColumnTypes()
{
  for (int i = 0; i < MAXCOL; i++) {
    m_columnType[i].clear();
  }
}

void CSVDialog::encodingChanged(int index)
{
  m_encodeIndex = index;
  if (!m_inFileName.isEmpty())
    readFile(m_inFileName, 0);
}

void CSVDialog::findCodecs()
{
  QMap<QString, QTextCodec *> codecMap;
  QRegExp iso8859RegExp("ISO[- ]8859-([0-9]+).*");

  foreach (int mib, QTextCodec::availableMibs()) {
    QTextCodec *codec = QTextCodec::codecForMib(mib);

    QString sortKey = codec->name().toUpper();
    int rank;

    if (sortKey.startsWith("UTF-8")) {             // krazy:exclude=strings
      rank = 1;
    } else if (sortKey.startsWith("UTF-16")) {            // krazy:exclude=strings
      rank = 2;
    } else if (iso8859RegExp.exactMatch(sortKey)) {
      if (iso8859RegExp.cap(1).size() == 1)
        rank = 3;
      else
        rank = 4;
    } else {
      rank = 5;
    }
    sortKey.prepend(QChar('0' + rank));

    codecMap.insert(sortKey, codec);
  }
  m_codecs = codecMap.values();
}


void CSVDialog::enableInputs()
{
  m_pageLinesDate->ui->spinBox_skip->setEnabled(true);
  m_pageBanking->ui->comboBoxBnk_numberCol->setEnabled(true);
  m_pageBanking->ui->comboBoxBnk_dateCol->setEnabled(true);
  m_pageBanking->ui->comboBoxBnk_payeeCol->setEnabled(true);
  m_pageBanking->ui->comboBoxBnk_memoCol->setEnabled(true);
  m_pageBanking->ui->button_clear->setEnabled(true);
  m_pageBanking->ui->gridLayout_2->columnStretch(2);
  m_pageLinesDate->ui->spinBox_skipToLast->setEnabled(true);
  m_pageSeparator->ui->comboBox_fieldDelimiter->setEnabled(true);

  if (m_pageBanking->ui->radioBnk_amount->isChecked()) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(false);
  } else {
    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(true);
  }
}

void CSVDialog::updateScreen()
{
  QRect tableRect = ui->tableWidget->geometry();//     need table height
  int hght = ui->tableWidget->horizontalHeader()->height() + 8;// find data height
  hght += (ui->tableWidget->rowHeight(m_row - 1)) * m_row;
  int ht = (hght < ui->frame_main->height() ? hght : ui->frame_main->height() - 10);// rect.height() reduce height if > frame
  tableRect.setHeight(ht);  //ht                                    set table height

  ui->tableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);  //ResizeToContents
  ui->tableWidget->setGeometry(tableRect);  //           resize now ***************
  ui->tableWidget->setFocus();
  updateRowHeaders(m_startLine);
}

void CSVDialog::slotCancel()
{
  int ret = KMessageBox::warningContinueCancel(this, i18n("<center>Are you sure you wish to exit?</center>"
                  "<center>Restart or Quit?</center>"), i18n("Cancel clicked"), KGuiItem(i18n("Restart")),
                  KStandardGuiItem::quit());
  if (ret == KMessageBox::Continue) {
    if (m_fileType == "Banking") {
      saveSettings();
    } else {
      m_investmentDlg->saveSettings();
    }
    init();
    return;
  }
  slotClose();
}

void CSVDialog::slotClose()
{
  saveSettings();
  m_investmentDlg->saveSettings();
  close();
}

void CSVDialog::saveSettings()
{
  if ((m_fileType != "Banking") || (m_inFileName.isEmpty())) {      //  don't save if no file loaded
    return;
  }

  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup mainGroup(config, "MainWindow");
  mainGroup.writeEntry("Height", height());
  mainGroup.config()->sync();

  KConfigGroup bankProfilesGroup(config, "BankProfiles");

  bankProfilesGroup.writeEntry("BankNames", m_profileList);
  int indx = m_pageIntro->ui->combobox_source->findText(m_priorCsvProfile, Qt::MatchExactly);
  QString str;
  if (indx > 0) {
    str = m_priorCsvProfile;
  }
  bankProfilesGroup.writeEntry("PriorCsvProfile", str);
  bankProfilesGroup.config()->sync();

  for (int i = 0; i < m_profileList.count(); i++) {
    if (m_profileList[i] != m_profileName) {
      continue;
    }

    QString txt = "Profiles-" + m_profileList[i];
    KConfigGroup profilesGroup(config, txt);
    profilesGroup.writeEntry("ProfileName", m_profileList[i]);
    profilesGroup.writeEntry("CurrentUI", m_currentUI);
    QString pth = "~/" + m_csvPath.section('/', 3);
    profilesGroup.writeEntry("CsvDirectory", pth);
    profilesGroup.writeEntry("DateFormat", m_pageLinesDate->ui->comboBox_dateFormat->currentIndex());
    profilesGroup.writeEntry("DebitFlag", m_debitFlag);
    profilesGroup.writeEntry("FieldDelimiter", m_fieldDelimiterIndex);
    profilesGroup.writeEntry("FileType", m_fileType);
    profilesGroup.writeEntry("TextDelimiter", m_textDelimiterIndex);
    profilesGroup.writeEntry("StartLine", m_pageLinesDate->ui->spinBox_skip->value() - 1);
    profilesGroup.writeEntry("DecimalSymbol", m_pageCompletion->ui->comboBox_decimalSymbol->currentIndex());
    profilesGroup.writeEntry("TrailerLines", m_pageLinesDate->m_trailerLines);

    profilesGroup.writeEntry("DateCol", m_pageBanking->ui->comboBoxBnk_dateCol->currentIndex());
    profilesGroup.writeEntry("PayeeCol", m_pageBanking->ui->comboBoxBnk_payeeCol->currentIndex());

    QList<int> list = m_memoColList;
    int posn = 0;
    if ((posn = list.indexOf(-1)) > -1) {
      list.removeOne(-1);
    }
    profilesGroup.writeEntry("MemoCol", list);

    profilesGroup.writeEntry("NumberCol", m_pageBanking->ui->comboBoxBnk_numberCol->currentIndex());
    profilesGroup.writeEntry("AmountCol", m_pageBanking->ui->comboBoxBnk_amountCol->currentIndex());
    profilesGroup.writeEntry("DebitCol", m_pageBanking->ui->comboBoxBnk_debitCol->currentIndex());
    profilesGroup.writeEntry("CreditCol", m_pageBanking->ui->comboBoxBnk_creditCol->currentIndex());
    profilesGroup.config()->sync();
  }
  m_inFileName.clear();
  ui->tableWidget->clear();//     in case later reopening window, clear old contents now
}


int CSVDialog::validateColumn(const int& col, QString& type)
{
  if (m_columnsNotSet) {  //                         Don't check columns until they've been selected.
    return KMessageBox::Ok;
  }
  //  First check if selection is in range
  if ((col < 0) || (col >= m_endColumn)) {
    return KMessageBox::No;
  }
  //                                                 selection is in range
  if (m_columnType[col] == type) {//                 already selected
    return KMessageBox::Ok;
  }
  if (m_columnType[col].isEmpty()) {  //             is this type already in use
    for (int i = 0; i < m_endColumn; i++) {
      //  check each column
      if (m_columnType[i] == type) {  //             this type already in use
        m_columnType[i].clear();//                   ...so clear it
      }//  end this col
    }// end all columns checked                      type not in use
    m_columnType[col] = type;//                      accept new type
    if (m_previousColumn != -1) {
      m_previousColumn = col;
    }
    m_previousType = type;
    return KMessageBox::Ok; //                       accept new type
  }
  if (m_columnType[col] == "memo") {
    int rc = KMessageBox::questionYesNo(0, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                                "<center>If you wish to copy the Memo data to the Payee field, click 'Yes'.</center>",
                                                m_columnType[col]));
    if (rc == KMessageBox::Yes) {
      m_memoColCopied = true;
      m_memoColCopy = col;
      m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
      m_payeeColumn = m_endColumn;
      type = "payee";
      m_columnType[m_payeeColumn] = "payee";
      m_payeeSelected = true;
      m_columnCountList << m_maxColumnCount + 1;
      return rc;
    }
  }
  //                                              BUT column is already in use
  KMessageBox::information(0, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                   "<center>Please reselect both entries as necessary.</center>", m_columnType[col]));
  m_previousColumn = -1;
  resetComboBox(m_columnType[col], col);
  resetComboBox(type, col);  //                    reset this combobox
  m_previousType.clear();
  m_columnType[col].clear();
  for (int i = 0; i < m_maxColumnCount; i++) {
    if (!m_columnType[i].isEmpty()) {
      if (m_columnType[i] == type) {
        m_columnType[i].clear();
      }
    }
  }
  return KMessageBox::Cancel;
}

void CSVDialog::amountColumnSelected(int col)
{
  QString type = "amount";
  m_amountColumn = col;
  if (col < 0) {      //                                 it is unset
    return;
  }
// if a previous amount field is detected, but in a different column...
  if ((m_amountColumn != -1) && (m_columnType[m_amountColumn] == type)  && (m_amountColumn != col)) {
    m_columnType[m_amountColumn].clear();
  }
  int ret = validateColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(col);  //    accept new column
    m_amountSelected = true;
    m_amountColumn = col;
    m_columnType[m_amountColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
  }
}

void CSVDialog::debitCreditRadioClicked(bool checked)
{
  if (checked) {
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(true);  //         if 'debit/credit' selected
    m_pageBanking->ui->labelBnk_debits->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(true);
    m_pageBanking->ui->labelBnk_credits->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(false);  //       disable 'amount' ui choices
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);  //     as credit/debit chosen

    //   the 'm_amountColumn' could just have been reassigned, so ensure
    //   ...m_columnType[m_amountColumn] == "amount" before clearing it
    if ((m_amountColumn >= 0) && (m_columnType[m_amountColumn] == "amount")) {
      m_columnType[m_amountColumn].clear();//          ...drop any amount choice
      m_amountColumn = -1;
    }
    m_pageBanking->ui->labelBnk_amount->setEnabled(false);
  }
}

void CSVDialog::creditColumnSelected(int col)
{
  QString type = "credit";
  m_creditColumn = col;
  if (col < 0) {      //                                    it is unset
    return;
  }
// if a previous credit field is detected, but in a different column...
  if ((m_creditColumn != -1) && (m_columnType[m_creditColumn] == type)  && (m_creditColumn != col)) {
    m_columnType[m_creditColumn].clear();
  }
  int ret = validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(col);  //    accept new column
    m_creditSelected = true;
    m_creditColumn = col;
    m_columnType[m_creditColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
  }
}

void CSVDialog::debitColumnSelected(int col)
{
  QString type = "debit";
  m_debitColumn = col;
  if (col < 0) {      //                                    it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_debitColumn != -1) && (m_columnType[m_debitColumn] == type)  && (m_debitColumn != col)) {
    m_columnType[m_debitColumn].clear();
  }
  int ret = validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(col);  //     accept new column
    m_debitSelected = true;
    m_debitColumn = col;
    m_columnType[m_debitColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
  }
}

void CSVDialog::dateColumnSelected(int col)
{
  QString type = "date";
  m_dateColumn = col;
  if (col < 0) {      //                                 it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_dateColumn != -1) && (m_columnType[m_dateColumn] == type)  && (m_dateColumn != col)) {
    m_columnType[m_dateColumn].clear();
  }
  int ret = validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(col);  //      accept new column
    m_dateSelected = true;
    m_dateColumn = col;
    m_columnType[m_dateColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
  }
}

void CSVDialog::memoColumnSelected(int col)
{
  QString type = "memo";
  m_memoColumn = col;
  if ((col < 0) || (col >= m_endColumn)) {      //                   out of range so...
    m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);  // ..clear selection
    return;
  }
  if (m_columnType[col].isEmpty()) {  //                             accept new  entry
    m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    m_columnType[col] = type;
    m_memoColumn = col;
    if (m_memoColList.contains(col)) {
    //  Restore the '*' as column might have been cleared.
    m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    } else {
      m_memoColList << col;
    }
    m_memoSelected = true;
    return;
  }
  if (m_columnType[col] == type) {      //               nothing changed
    return;
  }
  if (m_columnType[col] == "payee") {
    if (m_memoColList.contains(col)) {
      return;
    }
    int rc = KMessageBox::questionYesNo(0, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                                "<center>If you wish to copy the Payee data to the memo field, click 'Yes'.</center>",
                                                m_columnType[col]));
    if (rc == KMessageBox::Yes) {
      m_payeeColCopied = true;
      m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
      m_memoColumn = m_endColumn;
      m_memoColList << col;
      m_columnType[m_memoColumn] = "memo";
      m_memoSelected = true;
      m_columnCountList << m_maxColumnCount + 1;
      return;
    }
  } else {//                                       clashes with prior selection
    m_memoSelected = false;//                      clear incorrect selection
    m_payeeColCopied = false;
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>"
                                     , m_columnType[col]));
    m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
    m_previousColumn = -1;
    resetComboBox(m_columnType[col], col);  //       clash,  so reset ..
    resetComboBox(type, col);  //                    ... both comboboxes
    m_previousType.clear();
    m_columnType[col].clear();
    m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1));   //  reset the '*'
  }
}

void CSVDialog::payeeColumnSelected(int col)
{
  QString type = "payee";

  if (col < 0) {      //                              it is unset
    return;
  }
  // if a previous payee field is detected, but in a different column...
  if ((m_payeeColumn != -1) && (m_columnType[m_payeeColumn] == type)  && (m_payeeColumn != col)) {
    m_columnType[m_payeeColumn].clear();
  }
  m_payeeColumn = col;

  int ret = validateColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(col);  // accept new column
    m_payeeSelected = true;
    m_payeeColumn = col;
    m_columnType[m_payeeColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
  }
}

void CSVDialog::numberColumnSelected(int col)
{
  QString type = "number";
  m_numberColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
// if a previous number field is detected, but in a different column...
  if ((m_numberColumn != -1) && (m_columnType[m_numberColumn] == type)  && (m_numberColumn != col)) {
    m_columnType[m_numberColumn].clear();
  }
  int ret = validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(col);  // accept new column
    m_numberSelected = true;
    m_numberColumn = col;
    m_columnType[m_numberColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
  }
}

void CSVDialog::closeEvent(QCloseEvent *event)
{
  m_plugin->m_action->setEnabled(true);
  event->accept();
}

void CSVDialog::resizeEvent(QResizeEvent * event)
{
  ui->tableWidget->resizeColumnsToContents();
  event->accept();
}

QString CSVDialog::columnType(int column)
{
  return  m_columnType[column];
}

void CSVDialog::clearColumnType(int column)
{
  m_columnType[column].clear();
}

void CSVDialog::clearPreviousColumn()
{
  m_previousType.clear();
}

void CSVDialog::resetComboBox(const QString& comboBox, const int& col)
{
  QStringList fieldType;
  fieldType << "amount" << "credit" << "date" << "debit" << "memo" << "number" << "payee";
  int index = fieldType.indexOf(comboBox);
  switch (index) {
    case 0://  amount
      m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
      m_amountSelected = false;
      break;
    case 1://  credit
      m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
      m_creditSelected = false;
      break;
    case 2://  date
      m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
      m_dateSelected = false;
      break;
    case 3://  debit
      m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
      m_debitSelected = false;
      break;
    case 4://  memo
      m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
      m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1));   //  reset the '*'
      m_memoSelected = false;
      break;
    case 5://  number
      m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
      m_numberSelected = false;
      break;
    case 6://  payee
      m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
      m_payeeSelected = false;
      break;
    default:
      KMessageBox::sorry(this, i18n("<center>Field name not recognised.</center> <center>'<b>%1</b>'</center> Please re-enter your column selections."
                                    , comboBox), i18n("CSV import"));
  }
  m_columnType[col].clear();
  return;
}

void CSVDialog::updateDecimalSymbol(const QString& type, int col)
{
  QString txt;
  bool symbolFound = false;
  bool invalidResult = false;
  //  Clear background

  for (int row = 0; row < m_endLine; row++) {
    if (ui->tableWidget->item(row, col) != 0) {
      ui->tableWidget->item(row, col)->setBackground(m_clearBrush);
    }
  }
  if (type == "amount" || type == "credit" || type == "debit" || type == "price" || type == "quantity") {

    //  Set first and last rows

    m_parse->setSymbolFound(false);

    QString newTxt;
    int errorRow = 0;
    QTableWidgetItem* errorItem(0);
    //  Check if this col contains empty cells
    for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
      if (ui->tableWidget->item(row, col) == 0) {      //       empty cell
        if ((m_fileType == "Banking") && (importNow())) {
          //                                                    if importing, this is error
          KMessageBox::sorry(this, (i18n("Row number %1 may be a header line, as it has an incomplete set of entries."
                                         "<center>It may be that the start line is incorrectly set.</center>",
                                         row + 1), i18n("CSV import")));
          return;
        }
        //                                                      if not importing, query
        if (row + m_startLine > m_endLine) {
          return;
        }
        ui->tableWidget->verticalScrollBar()->setValue(row + m_startLine);
        int ret = KMessageBox::warningContinueCancel(this, i18n("<center>The cell in column '%1' on row %2 is empty.</center>"
                  "<center>Please check your selections.</center><center>Continue or Cancel?</center>",
                  col + 1, row + m_startLine), i18n("Selections Warning"), KStandardGuiItem::cont(),
                  KStandardGuiItem::cancel());
        if (ret == KMessageBox::Continue) {
          continue;
        }
        return;//                                               empty cell
      } else {

        //  Check if this col contains decimal symbol

        txt = ui->tableWidget->item(row, col)->text(); //       get data
        newTxt = m_parse->possiblyReplaceSymbol(txt);  //       update data
        ui->tableWidget->item(row, col)->setText(newTxt);   //  highlight selection
        ui->tableWidget->item(row, col)->setBackground(m_colorBrush);
        if (m_parse->invalidConversion()) {
          invalidResult = true;
          errorItem = ui->tableWidget->item(row, col);
          errorItem->setBackground(m_errorBrush);
          ui->tableWidget->scrollToItem(errorItem, QAbstractItemView::EnsureVisible);
          if (errorRow == 0) {
            errorRow = row;
          }
        }
        if (m_parse->symbolFound()) {
          symbolFound = true;
        }
        if (newTxt == txt) {      //                        no matching symbol found
          continue;
        }
      }
      if (!symbolFound) {
        errorItem = ui->tableWidget->item(row, col);
        errorItem->setBackground(m_errorBrush);
      }
    }//  last row

    if ((!symbolFound) && (!m_pageIntro->ui->checkBoxSkipSetup)) {     //                            no symbol found
      ui->tableWidget->scrollToItem(errorItem, QAbstractItemView::EnsureVisible);
      KMessageBox::sorry(this, i18n("<center>The selected decimal symbol was not present in column %1,</center>"
                                    "<center>- but may now have been added.</center>"
                                    "<center>If the <b>decimal</b> symbol displayed does not match your system   setting</center>"
                                    "<center>your data is unlikely to import correctly.</center>"
                                    "<center>Please check your selection.</center>",
                                    col + 1), i18n("CSV import"));
      return;
    }

    if (invalidResult) {
      KMessageBox::sorry(0, i18n("<center>The selected decimal symbol/thousands separator</center>"
                                 "<center>have produced invalid results in row %1, and possibly more.</center>"
                                 "<center>Please try again.</center>", errorRow + 1), i18n("Invalid Conversion"));
      return;
    }
  }
}

void CSVDialog::dateFormatSelected(int dF)
{
  if (dF == -1) {
    return;
  }
  m_dateFormatIndex = dF;
  m_date = m_dateFormats[m_dateFormatIndex];
}

void CSVDialog::decimalSymbolSelected(int index)
{
  int startLine = 0;
  int endLine = 0;
  //  If a file has been selected but the transaction is then cancelled, decimal symbol
  //  setting was checked prematurely so...
  if ((!m_pageIntro->ui->checkBoxSkipSetup->isChecked()) && (!m_pageLinesDate->m_isColumnSelectionComplete)) {
    return;
  }

  if ((index < 0) || (m_inFileName.isEmpty())) {
    return;
  }
  restoreBackground();//                              remove selection highlighting

  if (m_fileType == "Banking") {
    startLine = m_startLine;
    endLine = m_endLine;
  } else if (m_fileType == "Invest") {
    startLine = m_investProcessing->m_startLine;
    endLine = m_investProcessing->m_endLine;
  }
  if (startLine > endLine) {
    KMessageBox::sorry(0, i18n("<center>The start line is greater than the end line.\n</center>"
                               "<center>Please correct your settings.</center>"), i18n("CSV import"));
    m_importError = true;
    m_pageIntro->ui->checkBoxSkipSetup->setChecked(false);
    return;
  }
  if (m_fileType == "Banking") {
    readFile("", startLine);
  } else {
    m_investProcessing->readFile("", startLine);
  }
  //  Save new decimal symbol and thousands separator

  m_decimalSymbolIndex = index;
  m_parse->setDecimalSymbolIndex(index);
  m_decimalSymbol = m_parse->decimalSymbol(index);
  m_pageCompletion->ui->comboBox_thousandsDelimiter->setCurrentIndex(index);
  thousandsSeparatorChanged();

  //  Update the UI

  if (m_fileType == "Banking") {
    if ((!m_inFileName.isEmpty()) && ((m_amountColumn >= 0) || ((m_debitColumn >= 0) && (m_creditColumn >= 0)))) {
      if (m_amountColumn >= 0) {
        updateDecimalSymbol("amount", m_amountColumn);
      } else {
        updateDecimalSymbol("debit", m_debitColumn);
        updateDecimalSymbol("credit", m_creditColumn);
      }
    }
  } else {
    if (m_fileType == "Invest") {
      if (!m_inFileName.isEmpty()) {
        updateDecimalSymbol("amount", m_investProcessing->amountColumn());
        updateDecimalSymbol("price", m_investProcessing->priceColumn());
        updateDecimalSymbol("quantity", m_investProcessing->quantityColumn());
        ui->tableWidget->horizontalScrollBar()->setValue(200);
      }
    }
  }
  if (!m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    emit isImportable();
  }
}

QString CSVDialog::decimalSymbol()
{
  return m_decimalSymbol;
}

void CSVDialog::setDecimalSymbol(int val)
{
  m_decimalSymbol = val;
}

void CSVDialog::thousandsSeparatorChanged()
{
  m_thousandsSeparator = m_parse->thousandsSeparator();
}

void CSVDialog::delimiterChanged()
{
  if (m_fileType != "Banking") {
    return;
  }
  if (!m_inFileName.isEmpty()) {
    m_firstRead = true;  //  Used on first read and delimiterChanged, to get true line count.
    m_firstPass = true;
    m_maxColumnCount = 0;
    m_columnCountList.clear();
    m_startLine = m_pageLinesDate->ui->spinBox_skip->value();
    readFile(m_inFileName, m_startLine);
    readColumnSettings();
    updateScreen();
  }
}

void CSVDialog::startLineChanged(int val)
{
  if (m_fileType != "Banking") {
    return;
  }
  if (val > m_fileEndLine) {
    m_pageLinesDate->ui->spinBox_skip->setValue(m_fileEndLine);
  }
  if (val > m_endLine) {
    m_pageLinesDate->ui->spinBox_skip->setValue(m_endLine);
    return;
  }
  m_startLine = val;
  if (!m_inFileName.isEmpty()) {
    readFile(m_inFileName, m_startLine);
  }
}

void CSVDialog::startLineEdited()
{
  if (m_fileType != "Banking") {
    return;
  }
  int val = m_pageLinesDate->ui->spinBox_skip->value();
  if (val > m_fileEndLine) {
    m_pageLinesDate->ui->spinBox_skip->setValue(m_fileEndLine);
  }
  if (val > m_endLine) {
    m_pageLinesDate->ui->spinBox_skip->setValue(m_endLine);
    return;
  }
  m_startLine = val;
  if (!m_inFileName.isEmpty()) {
    readFile(m_inFileName, m_startLine);
  }
}

int CSVDialog::startLine()
{
  return m_startLine;
}

void CSVDialog::endLineChanged(int val)
{
  if (m_fileType != "Banking") {
    return;
  }
  if (val > m_fileEndLine) {
    m_pageLinesDate->ui->spinBox_skipToLast->setValue(val);
    return;
  }
  if (val < m_startLine) {
    m_pageLinesDate->ui->spinBox_skipToLast->setValue(m_startLine);
    return;
  }
  m_pageLinesDate->m_trailerLines = m_fileEndLine - val;
  m_endLine = val;
  if (!m_inFileName.isEmpty()) {
    readFile(m_inFileName, m_startLine);
  }
  ui->tableWidget->verticalScrollBar()->setValue(val - m_startLine - 2);
  ui->tableWidget->resizeColumnsToContents();
}

void CSVDialog::endLineChanged()
{
  if (m_fileType != "Banking") {
    return;
  }
  m_endLine = m_pageLinesDate->ui->spinBox_skipToLast->value();
}

int CSVDialog::lastLine()
{
  return m_endLine;
}

void CSVDialog::restoreBackground()
{
  int lastRow;
  int lastCol;
  if (m_fileType == "Banking") {
    lastRow = m_row;
    lastCol = m_endColumn;
  } else {
    lastRow = m_investProcessing->m_row;
    lastCol = m_investProcessing->m_endColumn;
  }

  for (int row = 0; row < lastRow; row++) {
    for (int col = 0; col < lastCol; col++) {
      if (ui->tableWidget->item(row, col) != 0) {
        ui->tableWidget->item(row, col)->setBackground(m_clearBrush);
      }
    }
  }
}

void CSVDialog::amountRadioClicked(bool checked)
{
  if (checked) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(true);  //  disable credit & debit ui choices
    m_pageBanking->ui->labelBnk_amount->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
    m_pageBanking->ui->labelBnk_debits->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
    m_pageBanking->ui->labelBnk_credits->setEnabled(false);

    //   the 'm_creditColumn/m_debitColumn' could just have been reassigned, so ensure
    //   ...they == "credit or debit" before clearing them
    if ((m_creditColumn >= 0) && (m_columnType[m_creditColumn] == "credit")) {
      m_columnType[m_creditColumn].clear();//       because amount col chosen...
    }
    if ((m_debitColumn >= 0) && (m_columnType[m_debitColumn] == "debit")) {
      m_columnType[m_debitColumn].clear();//        ...drop any credit & debit
    }
    m_debitColumn = -1;
    m_creditColumn = -1;
  }
}

QString CSVDialog::currentUI()
{
  return m_currentUI;
}

void CSVDialog::setCurrentUI(QString val)
{
  m_currentUI = val;
}

bool CSVDialog::importNow()
{
  return m_importNow;
}

void CSVDialog::showStage()
{
  QString str = ui->label_intro->text();
  ui->label_intro->setText("<b>" + str + "</b>");
}

void CSVDialog::slotIdChanged(int id)
{
  QString txt;
  m_lastId = m_curId;
  m_curId = id;
  if ((m_lastId == -1) || (m_curId == -1)) {
    return;
  }
  txt = m_stageLabels[m_lastId]->text();
  txt.remove(QRegExp("[<b>/]"));
  m_stageLabels[m_lastId]->setText(txt);

  txt = m_stageLabels[m_curId]->text();
  txt = "<b>" + txt + "</b>";
  m_stageLabels[m_curId]->setText(txt);
}

void CSVDialog::slotNamesEdited()
{
  QString str;
  int row = 0;
  int symTableRow = 0;

  for (row = 0; row < ui->tableWidget->rowCount(); row ++) {
    str = ui->tableWidget->item(row, m_investProcessing->symbolColumn())->text().trimmed();
    if (ui->tableWidget->item(row, m_investProcessing->symbolColumn())->text().trimmed().isEmpty()) {
      continue;
    }
    //  Replace detail with edited security name.
    str = m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 2)->text();

    ui->tableWidget->item(row, m_investProcessing->detailColumn())->setText(str);
    m_investProcessing->m_map.insert(m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 0)->text(), m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 2)->text());
    symTableRow ++;
  }
  emit isImportable();
}

void CSVDialog::slotBackButtonClicked()
{
  m_goBack = true;
}

int CSVDialog::amountColumn()
{
  return m_amountColumn;
}

int CSVDialog::debitColumn()
{
  return m_debitColumn;
}

int CSVDialog::creditColumn()
{
  return m_creditColumn;
}

int CSVDialog::dateColumn()
{
  return m_dateColumn;
}

int CSVDialog::payeeColumn()
{
  return m_payeeColumn;
}

int CSVDialog::numberColumn()
{
  return m_numberColumn;
}

int CSVDialog::memoColumn()
{
  return m_memoColumn;
}

void CSVDialog::slotHorScrollBarValue(int /*val*/)
{
  ui->tableWidget->resizeColumnsToContents();
}

void CSVDialog::slotVertScrollBarValue(int /*val*/)
{
  ui->tableWidget->resizeColumnsToContents();
}

//-----------------------------------------------------------------------------------------------------------------

IntroPage::IntroPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::IntroPage)
{
  ui->setupUi(this);
  m_pageLayout = new QVBoxLayout;
  m_priorIndex = 0;
  m_priorName = QString();
  m_addRequested = false;
  m_lastRadioButton.clear();
  m_firstLineEdit = true;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
  m_messageBoxJustCancelled = false;
  registerField("source", ui->combobox_source, "currentIndex", SIGNAL(currentIndexChanged()));
  disconnect(ui->combobox_source, 0, 0, 0);

  m_index = 1;

  ui->radioButton_bank->show();
  ui->radioButton_invest->show();
}

IntroPage::~IntroPage()
{
  delete ui;
}

void IntroPage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
  m_set = true;
  registerField("csvdialog", m_dlg, "m_set", SIGNAL(isSet()));
  m_dlg->showStage();

  wizard()->button(QWizard::CustomButton1)->setEnabled(false);
}

void IntroPage::slotComboEditTextChanged(QString txt)
{
  if ((ui->combobox_source->isHidden()) || (m_messageBoxJustCancelled) || (field("source").toInt() < 0)) {
    return;
  }
  m_index = field("source").toInt();
  m_messageBoxJustCancelled = false;
  if ((field("source").toInt() == 0) && (txt.isEmpty())) {
    ui->combobox_source->setCurrentIndex(-1);
    return;
  }
  if (m_priorName.isEmpty()) {
    m_priorName = m_dlg->m_profileName;
  }
  if (txt == m_priorName) {
    int indx = ui->combobox_source->findText(txt);
    ui->combobox_source->setCurrentIndex(indx);
    m_messageBoxJustCancelled = false;
    return;
  }
  if ((ui->combobox_source->count() == m_index) && (!txt.isEmpty())) {
    //  Not finished entering text.
    return;
  }
  if (m_firstLineEdit) {
    m_firstLineEdit = false;
    connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  }
  if ((txt.isEmpty()) && (!m_priorName.isEmpty()) && (m_messageBoxJustCancelled == false)) {
    //
    //  The disconnects are to avoid another messagebox appearing before the response for this one is processed.
    //
    disconnect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
    disconnect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
    m_messageBoxJustCancelled = false;

    int rc = KMessageBox::warningYesNo(0, i18n("<center>You have cleared the profile name '%1'.</center>\n"
             "<center>If you wish to delete the entry, click 'Delete'.</center>\n"
             "<center>Otherwise, click 'Keep'.</center>", m_dlg->m_profileName),
              i18n("Delete or Edit Profile Name"),
              KGuiItem(i18n("Delete")),
              KGuiItem(i18n("Keep")),"");
    if (rc == KMessageBox::No) {
      //
      //  Keep
      //
      ui->combobox_source->setCurrentItem(m_priorName);
      connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
      connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
      return;
    }
    //
    //  Delete, just clear to allow new text entry.
    //
    m_dlg->m_profileList.removeOne(m_dlg->m_profileName);
    int indx = ui->combobox_source->findText(m_dlg->m_profileName);
    ui->combobox_source->removeItem(indx);
    m_map.take(m_dlg->m_profileName);
    ui->combobox_source->setCurrentIndex(-1);
    m_dlg->m_profileName.clear();
    m_priorName.clear();
    KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
    KConfigGroup bankProfilesGroup(config, "BankProfiles");
    if (m_dlg->m_fileType == "Banking") {
      m_dlg->m_priorCsvProfile.clear();
      bankProfilesGroup.writeEntry("PriorCsvProfile", m_dlg->m_priorCsvProfile);
    } else {
      m_dlg->m_priorInvProfile.clear();
      bankProfilesGroup.writeEntry("PriorInvProfile", m_dlg->m_priorInvProfile);
    }
    bankProfilesGroup.writeEntry("BankNames", m_dlg->m_profileList);
    bankProfilesGroup.config()->sync();
    connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
    connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
    return;
  }
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
}

void IntroPage::slotComboSourceClicked(int index)
{
  m_messageBoxJustCancelled = false;
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  switch (index) {   //  Add New Profile selected.
    case 0:
      ui->combobox_source->setCurrentIndex(-1);
      m_action = "add";
      m_addRequested = true;
      break;
    default:
      m_dlg->m_wizard->button(QWizard::CustomButton1)->setEnabled(true);
      if (m_action == "add") {
        m_action.clear();
        QString txt = ui->combobox_source->currentText();
        if ((txt.isEmpty())) {
          return;
        }
        if (addItem(txt) == -1) {    //  Name already known.
          m_dlg->m_profileName = ui->combobox_source->currentText();
          if (m_dlg->m_fileType == "Banking") {
            m_dlg->m_priorCsvProfile = m_dlg->m_profileName;
          } else {
          m_dlg->m_priorInvProfile = m_dlg->m_profileName;
          }
          m_priorName = m_dlg->m_profileName;
          return;
        }
        //
        //  Adding new profile.
        //
        m_addRequested = false;
        addProfileName();
        return;
      } else {
        //
        //  Not adding so must be editing name, or selecting an existing profile.
        //
        QString txt = ui->combobox_source->currentText();
        m_priorName = m_dlg->m_profileName;
        m_priorIndex = m_index;
        if (!m_dlg->m_profileList.contains(txt)) {
          //  But this profile name does not exist.
          int indx = ui->combobox_source->findText(txt);
          if (m_priorName.isEmpty()) {
            disconnect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
            //  Add it perhaps.
            QString question = i18n("<center>The name you have entered does not exist,</center>"
                                    "<center>but you have not elected to add a new profile</center>"
                                    "<center>If you wish to add '%1' as a new profile,</center>"
                                    "<center> click 'Yes'.  Otherwise, click 'No'</center>", txt);
            if (KMessageBox::questionYesNo(0, question, i18n("Adding profile name.")) == KMessageBox::Yes) {
              addProfileName();
              m_index = indx;
              m_priorIndex = indx;
              connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
            } else {
              ui->combobox_source->removeItem(indx);
              ui->combobox_source->setCurrentIndex(-1);
              connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
              return;
            }
          }
          //  Otherwise maybe edit the present name.
          int ret = editProfileName(m_priorName, txt);
          if (ret == KMessageBox::No) {
            txt = m_priorName;
          } else {
            //  Use new name.
          }
          int index = ui->combobox_source->findText(txt, Qt::MatchExactly);
          ui->combobox_source->setCurrentIndex(index);
          return;
        }
        //  This profile is known so use it.

        m_priorName = ui->combobox_source->currentText();
        m_priorIndex = ui->combobox_source->currentIndex();

        m_dlg->m_profileName = ui->combobox_source->currentText();
        if (m_dlg->m_fileType == "Banking") {
          m_dlg->m_priorCsvProfile = m_dlg->m_profileName;
        } else {
          m_dlg->m_priorInvProfile = m_dlg->m_profileName;
        }
        if (m_dlg->m_profileList.contains(m_dlg->m_profileName)) {
          return;
        }
        editProfileName(m_priorName, m_dlg->m_profileName);
      }
  }
}

void  IntroPage::addProfileName()
{
  m_dlg->m_profileName = ui->combobox_source->currentText();
  if (m_dlg->m_fileType == "Banking") {
    m_dlg->m_priorCsvProfile = m_dlg->m_profileName;
  } else {
    m_dlg->m_priorInvProfile = m_dlg->m_profileName;
  }
  m_priorName = m_dlg->m_profileName;
  m_mapFileType.insert(m_dlg->m_profileName, m_dlg->m_fileType);
  m_dlg->m_profileList << m_dlg->m_profileName;
  m_dlg->createProfile(m_dlg->m_profileName);
  int indx = ui->combobox_source->findText(m_dlg->m_profileName);
  if (indx == -1) {
    ui->combobox_source->addItem(m_dlg->m_profileName);
  }
  indx = ui->combobox_source->findText(m_dlg->m_profileName);
  setField("source", indx);
}

int  IntroPage::editProfileName(QString& fromName, QString& toName)
{
  QString from = fromName;
  if (from == toName) {
    return  KMessageBox::No;
  }
  if (from.isEmpty()) {
    return  KMessageBox::Yes;
  }
  m_editAccepted = true;

  disconnect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  int fromIndx = ui->combobox_source->findText(from, Qt::MatchExactly);
  if (fromIndx != -1) {//  From name is in combobox.
    QString question = i18n("<center>You have edited the name of a profile</center>"
                            "<center>from '%1' to '%2'.</center>"
                            "<center>If you wish to accept the new name, click 'Yes'.</center>"
                            "<center>Otherwise, click 'No'</center>", from, toName);
    if (KMessageBox::questionYesNo(0, question, i18n("Edit a profile name or create new one.")) == KMessageBox::Yes) {
      disconnect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
      //  Accept new name.
      m_map.take(from);
      m_dlg->m_profileList.removeOne(from);
      ui->combobox_source->removeItem(ui->combobox_source->findText(from, Qt::MatchExactly));
      int toIndx = ui->combobox_source->findText(toName, Qt::MatchExactly);
      if ((toIndx == -1) && (m_messageBoxJustCancelled == false)){
        ui->combobox_source->addItem(toName);
      }
      m_index = ui->combobox_source->findText(toName, Qt::MatchExactly);
      m_dlg->m_profileName = toName;
      if (m_dlg->m_fileType == "Banking") {
        m_dlg->m_priorCsvProfile = m_dlg->m_profileName;
      } else {
        m_dlg->m_priorInvProfile = m_dlg->m_profileName;
      }
      m_dlg->createProfile(m_dlg->m_profileName);
      m_editAccepted = true;
      m_dlg->m_profileList<<toName;
      m_priorName = toName;
      m_priorIndex = m_index;
      m_messageBoxJustCancelled = false;
      connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
      connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
      return KMessageBox::Yes;
    } else {
      //  Don't accept new name so remove.
      int indx = ui->combobox_source->findText(toName);
      ui->combobox_source->removeItem(indx);
      m_dlg->m_profileList.removeOne(toName);
      if (m_dlg->m_fileType == "Banking") {
	m_dlg->m_priorCsvProfile = from;
      } else {
	m_dlg->m_priorInvProfile = from;
      }
      m_dlg->m_profileName = from;
      ui->combobox_source->setCurrentItem(from);
      m_editAccepted = false;
      connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
      connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
      return KMessageBox::No;
    }
  } else {//  Old entry was deleted from combobox, and we then accepted new name.
  return KMessageBox::Yes;
  }
}

void IntroPage::slotRadioButton_bankClicked()
{
  if ((m_lastRadioButton != "Bank") && (!m_lastRadioButton.isEmpty())){
    int rc = KMessageBox::warningContinueCancel(0, i18n("<center>If you continue, you will lose any recent profile edits.</center>"
                  "<center>Continue or Cancel?</center>"), i18n("Radio button Banking clicked"), KStandardGuiItem::cont(),
                  KStandardGuiItem::cancel());
    if (rc == KMessageBox::Cancel) {
      ui->radioButton_invest->setChecked(true);
      return;
    }
  }
  m_dlg->m_fileType = "Banking";
  ui->combobox_source->setEnabled(true);
  ui->combobox_source->show();

  m_dlg->readSettingsInit();
  m_priorName.clear();

  if ((!ui->combobox_source->currentText().isEmpty()) && (ui->combobox_source->currentIndex() >= 0)) {
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);
  }
  ui->checkBoxSkipSetup->setEnabled(true);
  m_lastRadioButton = "Bank";
  //
  //  This below looks strange, but is necessary (I think, anyway), because if the alternate radio button
  //  is checked, multiple connects occur.  So, disconnect any existing connection then re-enable.
  //
  disconnect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
  connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
}

void IntroPage::slotRadioButton_investClicked()
{
  if ((m_lastRadioButton != "Invest") && (!m_lastRadioButton.isEmpty())) {
    int rc = KMessageBox::warningContinueCancel(0, i18n("<center>If you continue, you will lose any recent profile edits.</center>"
                  "<center>Continue or Cancel?</center>"), i18n("Radio button Investment clicked"), KStandardGuiItem::cont(),
                  KStandardGuiItem::cancel());
    if (rc == KMessageBox::Cancel) {
      ui->radioButton_bank->setChecked(true);
      return;
    }
  }
  m_dlg->m_fileType = "Invest";
  ui->combobox_source->setEnabled(true);
  ui->combobox_source->show();

  m_dlg->readSettingsInit();
  m_priorName.clear();

  if ((!ui->combobox_source->currentText().isEmpty()) && (ui->combobox_source->currentIndex() >= 0)) {
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);
  }
  ui->checkBoxSkipSetup->setEnabled(true);
  m_lastRadioButton = "Invest";
  //
  //  This below looks strange, but is necessary (I think, anyway), because if the alternate radio button
  //  is checked, multiple connects occur.  So, disconnect any existing connection then re-enable.
  //
  disconnect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
  connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
}

int IntroPage::addItem(QString txt)
{
  if (txt.isEmpty()) {
    return -1;
  }
  disconnect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  int ret = -1;
  int indx = ui->combobox_source->findText(txt);

  QString question1 = i18n("<center>The name you have entered does not exist,</center>"
	                   "<center>but you have not elected to add a new profile.</center>");
  QString question2= i18n("<center>If you wish to add '%1'as a new profile,</center>"
                          "<center> click 'Yes'.  Otherwise, click 'No'</center>", txt);
  if (indx == -1) {
    //  Not found.

    if (!m_addRequested) {
      question2 = question1 + question2;
      if (KMessageBox::questionYesNo(0, question2, i18n("Adding profile name.")) == KMessageBox::No) {
        ui->combobox_source->lineEdit()->clear();
        connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
        return ret;
      }
    }
    m_addRequested = false;
    ui->combobox_source->setCurrentItem(txt,true);
    indx = ui->combobox_source->findText(txt);
    m_index = indx;
    ret = 0;
  }  else {  //  Already exists.

    if ((!m_addRequested) && (!m_editAccepted)) {
      if (KMessageBox::questionYesNo(0, question2, i18n("Adding profile name.")) == KMessageBox::No) {
        int indx = ui->combobox_source->findText(txt);
        ui->combobox_source->removeItem(indx);
        return ret;
      }
      m_index = indx;
    }
    if (! m_dlg->m_profileList.contains(txt)) {
      m_dlg->m_profileList<<txt;
      m_dlg->createProfile(txt);
    }
    m_addRequested = false;
  }
  m_dlg->m_profileName = txt;
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  return ret;
}

void IntroPage::initializePage()
{
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::CustomButton1
         <<  QWizard::CancelButton;
  wizard()->setButtonText(QWizard::CustomButton1, i18n("Select File"));
  wizard()->setOption(QWizard::HaveCustomButton1, false);
  wizard()->setButtonLayout(layout);
  m_dlg->m_wizard->button(QWizard::CustomButton1)->setToolTip(i18n("A profile must be selected before selecting a file."));
  m_firstEdit = false;
  m_editAccepted = false;
  m_newProfileCreated  = QString();
  connect(ui->combobox_source, SIGNAL(activated(int)), this, SLOT(slotComboSourceClicked(int)));
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
}

bool IntroPage::validatePage()
{
  m_dlg->m_firstPass = false;
  if (!m_newProfileCreated.isEmpty()) {
    m_dlg->createProfile(m_newProfileCreated);
  }
  ui->checkBoxSkipSetup->setEnabled(false);

  return true;
}

void IntroPage::slotLineEditingFinished()
{
  if ((ui->combobox_source->currentIndex() == -1) && (m_firstEdit == true)){
    m_firstEdit = false;
  }
  QString newName = ui->combobox_source->lineEdit()->text();
  if ((newName.isEmpty()) || (newName == m_priorName)) {
    return;
  }
  m_priorName = m_dlg->m_profileName;
  m_priorIndex = m_index;
  m_dlg->m_profileName = newName;
  if (m_dlg->m_fileType == "Banking") {
    m_dlg->m_priorCsvProfile = m_dlg->m_profileName;
  } else {
    m_dlg->m_priorInvProfile = m_dlg->m_profileName;
  }
  if (ui->combobox_source->currentIndex() < 1) {
    m_action = "add";
    if ((newName == "Add New Profile") || (newName.isEmpty())) {
      return;
    }
  }
  if ((ui->combobox_source->currentIndex() == m_priorIndex) && (m_action != "add")) {  //  Editing current selection.
    int rc = editProfileName(m_priorName, newName);
    if (rc == KMessageBox::No) {
      ui->combobox_source->setCurrentIndex(m_priorIndex);
      return;
    } else {
      ui->combobox_source->setCurrentItem(newName);
    }
  }
  m_index = ui->combobox_source->currentIndex();
  m_priorIndex = m_index;
  if ((m_messageBoxJustCancelled == false) && (m_firstEdit == true) && (m_editAccepted == true)) {
    m_firstEdit = true;
    return;
  }
  m_firstEdit = true;
  int itmIndx = 0;
  itmIndx = addItem(newName);
  if (itmIndx == -1) {//  Already exists.
    m_priorName = newName;
    return;
  }
  //
  //  Adding new profile.
  //
  setField("source", m_index);
  if (m_dlg->m_profileList.contains(newName)) {
    return;
  }
  if (m_action != "add") {
    editProfileName(m_priorName, newName);
  }
  m_dlg->m_profileName = newName;
  if (m_dlg->m_fileType == "Banking") {
    m_dlg->m_priorCsvProfile = m_dlg->m_profileName;
  } else {
    m_dlg->m_priorInvProfile = m_dlg->m_profileName;
  }
  m_dlg->m_profileList.append(m_dlg->m_profileName);
  m_dlg->createProfile(m_dlg->m_profileName);
  m_newProfileCreated = m_dlg->m_profileName;
  m_priorName = m_dlg->m_profileName;
  m_mapFileType.insert(m_dlg->m_profileName, m_dlg->m_fileType);
  m_priorIndex = ui->combobox_source->findText(m_dlg->m_profileName);
  if (m_priorIndex == -1) {
    ui->combobox_source->addItem(m_dlg->m_profileName);
  }
  m_priorIndex = ui->combobox_source->findText(m_dlg->m_profileName);
  ui->combobox_source->setCurrentItem(m_dlg->m_profileName, false);
  m_action.clear();
}

SeparatorPage::SeparatorPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::SeparatorPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
}

SeparatorPage::~SeparatorPage()
{
  delete ui;
}

void SeparatorPage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
}

void SeparatorPage::initializePage()
{
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton
         <<  QWizard::CancelButton;
  wizard()->setButtonLayout(layout);

  connect(ui->comboBox_fieldDelimiter, SIGNAL(activated(int)), m_dlg, SLOT(delimiterChanged()));
  connect(ui->comboBox_fieldDelimiter, SIGNAL(activated(int)), this, SLOT(delimiterChanged()));
  connect(ui->comboBox_textDelimiter, SIGNAL(activated(int)), m_dlg, SLOT(delimiterChanged()));
}

void SeparatorPage::delimiterChanged()
{
  m_dlg->m_pageBanking->m_bankingPageInitialized  = false;
  m_dlg->m_pageInvestment->m_investPageInitialized  = false;
  emit completeChanged();
}

bool SeparatorPage::isComplete() const
{
  //
  //  Check for presence of needed columns.
  //  This is not foolproof, but can help detect wrong delimiter choice.
  //
  bool ret1;
  bool ret2;
  bool ret3;
  bool ret = false;
  if (m_dlg->m_fileType == "Banking") {
    ret1 = (m_dlg->m_endColumn > 2);
    ret2 = ((field("dateColumn").toInt() > -1) && (field("payeeColumn").toInt() > -1)  &&
           ((field("amountColumn").toInt() > -1) || ((field("debitColumn").toInt() > -1)  && (field("creditColumn").toInt() > -1))));
    ret3 = m_dlg->m_pageBanking->m_bankingPageInitialized;
    ret = (ret1 || (ret2 && ret3));
  } else if (m_dlg->m_fileType == "Invest") {
    ret1 = (m_dlg->m_investProcessing->m_endColumn > 3);
    ret2 = ((field("dateCol").toInt() > -1)  && ((field("amountCol").toInt() > -1) || ((field("quantityCol").toInt() > -1)))  &&
           ((field("symbolCol").toInt() > -1) || (field("securityNameIndex").toInt() > -1)));
    ret3 = m_dlg->m_pageInvestment->m_investPageInitialized;
    ret = (ret1 || (ret2 && ret3));
  }
  if (!ret) {
    wizard()->button(QWizard::NextButton)->setToolTip(i18n("Incorrect number or type of fields.  Check the field delimiter."));
  } else {
    wizard()->button(QWizard::NextButton)->setToolTip(QString());
  }
  return ret;
}

bool SeparatorPage::validatePage()
{
  m_dlg->m_firstPass = false;
  return true;
}

void SeparatorPage::cleanupPage()
{
  m_dlg->m_pageIntro->initializePage();
  wizard()->button(QWizard::CustomButton1)->setToolTip(i18n("<center>It is not possible to select another file.</center><center>It is necessary to restart the plugin.</center>"));
  m_dlg->m_pageIntro->ui->checkBoxSkipSetup->setEnabled(true);
}

int SeparatorPage::nextId() const
{
  int ret;
  if (m_dlg->m_fileType == "Banking") {
    ret = CSVDialog::Page_Banking;
  } else {
    ret = CSVDialog::Page_Investment;
  }
  return ret;
}

BankingPage::BankingPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::BankingPage)
{
  ui->setupUi(this);
  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);

  ui->comboBoxBnk_numberCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_dateCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_payeeCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_memoCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_amountCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_creditCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_debitCol->setMaxVisibleItems(12);

  registerField("dateColumn", ui->comboBoxBnk_dateCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("payeeColumn", ui->comboBoxBnk_payeeCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("amountColumn", ui->comboBoxBnk_amountCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("debitColumn", ui->comboBoxBnk_debitCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("creditColumn", ui->comboBoxBnk_creditCol, "currentIndex", SIGNAL(currentIndexChanged()));

  connect(ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColChanged(int)));
  connect(ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAmountColChanged(int)));
  connect(ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPayeeColChanged(int)));
  connect(ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDebitColChanged(int)));
  connect(ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCreditColChanged(int)));
}

BankingPage::~BankingPage()
{
  delete ui;
}

void BankingPage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
}

void BankingPage::initializePage()
{
  connect(m_dlg->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), m_dlg, SLOT(startLineChanged(int)));
  int index = m_dlg->m_pageIntro->ui->combobox_source->currentIndex();
  setField("source", index);
  m_dlg->m_fileType = "Banking";
  m_bankingPageInitialized = true;
  m_dlg->m_columnsNotSet = false;  //            Allow checking of columns now.
}

int BankingPage::nextId() const
{
  return CSVDialog::Page_LinesDate;
}

bool BankingPage::isComplete() const
{
  bool ret = ((field("dateColumn").toInt() > -1)  && (field("payeeColumn").toInt() > -1)  && ((field("amountColumn").toInt() > -1) || ((field("debitColumn").toInt() > -1)  && (field("creditColumn").toInt() > -1))));
  return ret;
}

void BankingPage::cleanupPage()
{
}

void BankingPage::slotDateColChanged(int col)
{
  setField("dateColumn", col);
  emit completeChanged();
}

void BankingPage::slotPayeeColChanged(int col)
{
  setField("payeeColumn", col);
  emit completeChanged();
}

void BankingPage::slotDebitColChanged(int col)
{
  setField("debitColumn", col);
  emit completeChanged();
}

void BankingPage::slotCreditColChanged(int col)
{
  setField("creditColumn", col);
  emit completeChanged();
}

void BankingPage::slotAmountColChanged(int col)
{
  setField("amountColumn", col);
  emit completeChanged();
}

InvestmentPage::InvestmentPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::InvestmentPage)
{
  ui->setupUi(this);
  resize(638, 600);
  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);

  registerField("dateCol", ui->comboBoxInv_dateCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("typeCol", ui->comboBoxInv_typeCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("quantityCol", ui->comboBoxInv_quantityCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("priceCol", ui->comboBoxInv_priceCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("amountCol", ui->comboBoxInv_amountCol, "currentIndex", SIGNAL(currentIndexChanged()));

  registerField("symbolCol", ui->comboBoxInv_symbolCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("detailCol", ui->comboBoxInv_detailCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("securityNameIndex", ui->comboBoxInv_securityName, "currentIndex", SIGNAL(currentIndexChanged()));

  connect(ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColChanged(int)));
  connect(ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTypeColChanged(int)));
  connect(ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotQuantityColChanged(int)));
  connect(ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPriceColChanged(int)));
  connect(ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAmountColChanged(int)));
  connect(ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSymbolColChanged(int)));
  connect(ui->comboBoxInv_detailCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDetailColChanged(int)));

  connect(ui->lineEdit_filter, SIGNAL(returnPressed()), this, SLOT(slotFilterEditingFinished()));
  connect(ui->lineEdit_filter, SIGNAL(editingFinished()), this, SLOT(slotFilterEditingFinished()));
}

InvestmentPage::~InvestmentPage()
{
  delete ui;
}

void InvestmentPage::initializePage()
{
  int index = m_dlg->m_pageIntro->ui->combobox_source->currentIndex();
  setField("source", index);
  m_dlg->m_fileType = "Invest";

  m_investPageInitialized = true;
  connect(m_dlg->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), m_dlg->m_investProcessing, SLOT(startLineChanged(int)));
  wizard()->button(QWizard::NextButton)->setEnabled(false);
  connect(ui->comboBoxInv_securityName, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsecurityNameChanged(int)));
  connect(ui->buttonInv_hideSecurity, SIGNAL(clicked()), m_dlg->m_investProcessing, SLOT(hideSecurity()));
  m_dlg->m_isTableTrimmed = false;
  m_dlg->m_detailFilter = ui->lineEdit_filter->text();//    Load setting from config file.
}

void InvestmentPage::slotDateColChanged(int col)
{
  setField("dateCol", col);
  emit completeChanged();
}

void InvestmentPage::slotTypeColChanged(int col)
{
  setField("typeCol", col);
  emit completeChanged();
}

void InvestmentPage::slotQuantityColChanged(int col)
{
  setField("quantityCol", col);
  emit completeChanged();
}

void InvestmentPage::slotPriceColChanged(int col)
{
  setField("priceCol", col);
  emit completeChanged();
}

void InvestmentPage::slotAmountColChanged(int col)
{
  setField("amountCol", col);
  emit completeChanged();
}

void InvestmentPage::slotSymbolColChanged(int col)
{
  setField("symbolCol", col);
  if (col != -1) {
    setField("securityNameIndex", -1);
    ui->comboBoxInv_securityName->setCurrentIndex(-1);
  }
  emit completeChanged();
}

void InvestmentPage::slotDetailColChanged(int col)
{
  setField("detailCol", col);
  if (col != -1) {
    setField("securityNameIndex", -1);
    ui->comboBoxInv_securityName->setCurrentIndex(-1);
  }
  emit completeChanged();
}

void InvestmentPage::slotsecurityNameChanged(int index)
{
  setField("securityNameIndex", index);

  if (index != -1) {
    setField("symbolCol", -1);
    setField("detailCol", -1);
    ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
    ui->comboBoxInv_detailCol->setCurrentIndex(-1);
  }
  emit completeChanged();
}

void InvestmentPage::slotFilterEditingFinished()
{
  m_dlg->m_detailFilter = ui->lineEdit_filter->text();
}

void InvestmentPage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
  connect(ui->button_clear, SIGNAL(clicked()), m_dlg->m_investProcessing, SLOT(clearColumnsSelected()));
}

bool InvestmentPage::isComplete() const
{
  bool ret = (((field("symbolCol").toInt() > -1) && (field("detailCol").toInt() > -1)) || ((field("securityNameIndex").toInt()) > -1)) &&
             (field("dateCol").toInt() > -1) && (field("typeCol").toInt() > -1) &&
             (field("quantityCol").toInt() > -1) && (field("priceCol").toInt() > -1) && (field("amountCol").toInt() > -1);
  return ret;
}

void InvestmentPage::cleanupPage()
{
}

LinesDatePage::LinesDatePage(QWidget *parent) : QWizardPage(parent), ui(new Ui::LinesDatePage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
  m_trailerLines = 0;
}

LinesDatePage::~LinesDatePage()
{
  delete ui;
}

void LinesDatePage::initializePage()
{
  if (m_dlg->m_fileType == "Banking") {
    m_dlg->readFile(m_dlg->m_inFileName, m_dlg->startLine());
  } else if (m_dlg->m_fileType == "Invest") {
    m_dlg->m_investProcessing->readFile(m_dlg->m_investProcessing->m_inFileName, m_dlg->m_investProcessing->m_startLine);
  }
  m_dlg->m_goBack = false;
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton <<  QWizard::NextButton <<  QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
  m_isColumnSelectionComplete = true;
  if (m_dlg->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    validatePage();
  }
}

void LinesDatePage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
}


bool LinesDatePage::validatePage()
{
  bool ok;
  QString value;
  //
  //  Ensure numeric columns do contain valid numeric values
  //
  if (m_dlg->m_fileType == "Banking") {
    for (int row = 0; row < m_dlg->ui->tableWidget->rowCount(); row++) {
      if (row + m_dlg->startLine() > m_dlg->lastLine()) {
        return KMessageBox::Yes;
      }
      for (int col = 0; col < m_dlg->ui->tableWidget->columnCount(); col++) {
        if ((m_dlg->columnType(col) == "amount") || (m_dlg->columnType(col) == "debit") || (m_dlg->columnType(col) == "credit")) {
          QString pattern = "[" + KGlobal::locale()->currencySymbol() + "(), $]";
          if (m_dlg->ui->tableWidget->item(row, col) == 0) {  //  Does cell exist?
            continue;  //  No.
          }
          value = m_dlg->ui->tableWidget->item(row, col)->text().remove(QRegExp(pattern));
          if (value.isEmpty()) {  //  An empty cell is OK, probably.
            continue;
          }
          value.toDouble(&ok); // Test validity.
          if ((!ok) && (!m_dlg->m_acceptAllInvalid)) {
            QString str = KGlobal::locale()->currencySymbol();
            int rc = KMessageBox::questionYesNoCancel(this, i18n("<center>An invalid value has been detected in column %1 on row %2.</center>"
                                                                 "Please check that you have selected the correct columns."
                                                                 "<center>You may accept all similar items, or just this one, or cancel.</center>",
                                                                 col + 1, row + m_dlg->startLine()), i18n("CSV import"),
                                                                 KGuiItem(i18n("Accept All")),
                                                                 KGuiItem(i18n("Accept This")),
                                                                 KGuiItem(i18n("Cancel")));
            switch (rc) {
            case KMessageBox::Yes:  //  = "Accept All"
              m_dlg->m_acceptAllInvalid = true;
              continue;

            case KMessageBox::No:  //  "Accept This"
              m_dlg->m_acceptAllInvalid = false;
              continue;

            case KMessageBox::Cancel:
              return false;
            }
          }
        }
      }
    }
  } else {
    for (int row = 0; row < m_dlg->m_investProcessing->m_startLine; row++) {
      for (int col = 0; col < m_dlg->ui->tableWidget->columnCount(); col++) {
        if ((m_dlg->m_investProcessing->columnType(col) == "amount") || (m_dlg->m_investProcessing->columnType(col) == "quantity") || (m_dlg->m_investProcessing->columnType(col) == "price")) {
          QString pattern = "[" + KGlobal::locale()->currencySymbol() + "(), $]";
          if (m_dlg->ui->tableWidget->item(row, col) == 0) {  //  Does cell exist?
            continue;  //  No.
          }
          value = m_dlg->ui->tableWidget->item(row, col)->text().remove(QRegExp(pattern));
          value = value.remove("--");  //  Possible blank marker.
          if (value.isEmpty()) {  //       An empty cell is OK, probably.
            continue;
          }
          value.toDouble(&ok); // Test validity.
          if ((!ok) && (!m_dlg->m_acceptAllInvalid)) {
            QString str = KGlobal::locale()->currencySymbol();
            int rc = KMessageBox::questionYesNoCancel(this, i18n("<center>An invalid value has been detected in column %1 on row %2.</center>"
                                                                 "Please check that you have selected the correct columns."
                                                                 "<center>You may accept all similar items, or just this one, or cancel.</center>",
                                                                 col + 1, row + m_dlg->m_investProcessing->m_startLine), i18n("CSV import"),
                                                                 KGuiItem(i18n("Accept All")),
                                                                 KGuiItem(i18n("Accept This")),
                                                                 KGuiItem(i18n("Cancel")));
            switch (rc) {
            case KMessageBox::Yes:  //  = "Accept All"
              m_dlg->m_acceptAllInvalid = true;
              continue;

            case KMessageBox::No:  //  "Accept This"
              m_dlg->m_acceptAllInvalid = false;
              continue;

            case KMessageBox::Cancel:
              return false;
            }
          }
        }
      }
    }
  }

  int symTableRow = -1;
  if ((m_dlg->m_fileType == "Banking") || (field("symbolCol").toInt() == -1)) {  //  Only check symbols if that field is set, and not Banking.
    if ((m_dlg->m_pageIntro->ui->checkBoxSkipSetup->isChecked())) {
      if (m_dlg->m_importError) {
        m_dlg->m_importError = false;
        wizard()->next();
      } else {
        m_dlg->m_pageCompletion->slotImportClicked();
      }
    }
    return true;
  }

  disconnect(m_dlg->m_symbolTableDlg->m_widget->tableWidget, SIGNAL(cellChanged(int, int)), 0, 0);

  MyMoneyStatement::Security security;

  if (field("securityNameIndex").toInt() == -1) {     //  No security name chosen so scan entries
    QString symbl;
    QString securityName;
    if (m_dlg->m_fileType == "Invest") {
    }
    for (int row = 0; row < m_dlg->ui->tableWidget->rowCount(); row++) {
      int col = m_dlg->m_pageInvestment->ui->comboBoxInv_symbolCol->currentIndex();
      symbl = m_dlg->ui->tableWidget->item(row, col)->text().toLower().trimmed();

      if (symbl.trimmed().isEmpty()) {     //  no ticker
        continue;
      }
      int detail = m_dlg->m_pageInvestment->ui->comboBoxInv_detailCol->currentIndex();
      securityName = m_dlg->ui->tableWidget->item(row, detail)->text().toLower();
      bool done = false;
      MyMoneyFile* file = MyMoneyFile::instance();

      // Check if we already have the security on file.
      // Just use the symbol for matching, because the security name
      // field is unstandardised and very variable.
      MyMoneySecurity sec;
      QList<MyMoneySecurity> list = file->securityList();
      bool exists;
      QString name;
      QList<MyMoneySecurity>::ConstIterator it = list.constBegin();
      while (it != list.constEnd()) {
        exists = false;
        if (!symbl.isEmpty())  {     //  symbol already exists
          sec = *it;
          name.clear();
          if (sec.tradingSymbol() == symbl) {
            exists = true;
            name = sec.name();
            break;
          }
        }
        ++it;
      }
      if (!exists) {
        name = securityName;
      }
      if (symbl.isEmpty()) {
        break;
      }
      symTableRow ++;
      m_dlg->m_symbolTableDlg->displayLine(symTableRow, symbl, name, exists);
      m_dlg->m_investProcessing->m_symbolsList << symbl;
      m_dlg->m_investProcessing->m_map.insert(symbl, name);
      if (done) {
        break;
      }
    }
  }
  if (symTableRow > -1) {
    m_dlg->m_symbolTableDlg->show();
  }
  connect(m_dlg->m_symbolTableDlg->m_widget->tableWidget,  SIGNAL(itemChanged(QTableWidgetItem*)), m_dlg->m_symbolTableDlg,  SLOT(slotItemChanged(QTableWidgetItem*)));

  return true;
}

int LinesDatePage::nextId() const
{
  return CSVDialog::Page_Completion;
}


CompletionPage::CompletionPage(QWidget* parent) : QWizardPage(parent), ui(new Ui::CompletionPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
}

CompletionPage::~CompletionPage()
{
  delete ui;
}

void CompletionPage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
}


void CompletionPage::initializePage()
{
  m_dlg->m_firstPass = false;  //  Needs to be here when skipping setup.
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::CustomButton2 << QWizard::CustomButton3
         <<  QWizard::FinishButton <<  QWizard::CancelButton;
  wizard()->setOption(QWizard::HaveCustomButton2, true);
  wizard()->setButtonText(QWizard::CustomButton2, i18n("Import CSV"));
  wizard()->setOption(QWizard::HaveCustomButton3, false);
  wizard()->setButtonText(QWizard::CustomButton3, i18n("Make QIF File"));
  wizard()->button(QWizard::CustomButton3)->setEnabled(false);
  m_dlg->m_isTableTrimmed = true;

  wizard()->setButtonLayout(layout);

  if (m_dlg->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    m_dlg->m_detailFilter = m_dlg->m_pageInvestment->ui->lineEdit_filter->text();//  Load setting from config file.
    m_dlg->m_pageLinesDate->validatePage();

    if (m_dlg->m_importIsValid) {
      slotImportClicked();
    }
  }
}

void CompletionPage::slotImportValid()
{
  QList<QWizard::WizardButton> layout;

  layout << QWizard::Stretch << QWizard::BackButton << QWizard::CustomButton2 << QWizard::FinishButton << QWizard::CancelButton;
  wizard()->setOption(QWizard::HaveCustomButton2, true);
  wizard()->setButtonText(QWizard::CustomButton2, i18n("Import CSV"));
  wizard()->setButtonLayout(layout);
  m_dlg->m_importIsValid = true;
  if (m_dlg->m_pageIntro->ui->checkBoxSkipSetup) {
    initializePage();
  }
}

void CompletionPage::slotImportClicked()
{
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::CustomButton2 << QWizard::CustomButton3
         <<  QWizard::FinishButton <<  QWizard::CancelButton;
  wizard()->setOption(QWizard::HaveCustomButton2, true);
  wizard()->setButtonText(QWizard::CustomButton2, i18n("Import CSV"));
  wizard()->setOption(QWizard::HaveCustomButton3, true);
  wizard()->setButtonText(QWizard::CustomButton3, i18n("Make QIF File"));
  wizard()->button(QWizard::CustomButton3)->setEnabled(true);
  m_dlg->m_isTableTrimmed = true;

  wizard()->setButtonLayout(layout);

  if (m_dlg->m_fileType == "Banking") {
    emit importBanking();
    setFinalPage(true);
  }  else {
    emit importInvestment();
  }
  if (m_dlg->m_importError) {
    wizard()->setOption(QWizard::HaveFinishButtonOnEarlyPages, true);
    wizard()->button(QWizard::QWizard::FinishButton)->setEnabled(true);
    m_dlg->m_pageIntro->ui->checkBoxSkipSetup->setChecked(false);

    m_dlg->m_wizard->setStartId(CSVDialog::Page_Separator);  // Back to square one.
    m_dlg->slotIdChanged(CSVDialog::Page_Separator);
    m_dlg->m_wizard->restart();
  }
  if (m_dlg->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    m_dlg->m_pageIntro->ui->combobox_source->setEnabled(false);
    m_dlg->m_pageIntro->ui->radioButton_bank->setEnabled(false);
    m_dlg->m_pageIntro->ui->radioButton_invest->setEnabled(false);
  }
  wizard()->setOption(QWizard::HaveFinishButtonOnEarlyPages, true);
  wizard()->button(QWizard::QWizard::FinishButton)->setEnabled(true);
  setFinalPage(true);  //  We need FinishButton.

}

void CompletionPage::cleanupPage()
{
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton <<  QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
}

bool CompletionPage::validatePage()
{
  emit completeChanged();
  return true;
}
