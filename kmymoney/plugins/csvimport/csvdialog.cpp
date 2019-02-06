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
#include <QTextCodec>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kdeversion.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
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
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Headers

#include "convdate.h"
#include "csvutil.h"
#include "investmentdlg.h"
#include "investprocessing.h"
#include "symboltabledlg.h"
#include "csvwizard.h"

#include "ui_csvdialog.h"
#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_lines-datewizardpage.h"
#include "ui_completionwizardpage.h"
#include "ui_investmentwizardpage.h"
#include "ui_csvwizard.h"

#include "mymoneyfile.h"

// ----------------------------------------------------------------------------

CSVDialog::CSVDialog(CsvImporterPlugin *plugin)
  : m_plugin(plugin),
    ui(new Ui::CSVDialog),
    m_vHeaderWidth(0),
    m_possibleDelimiter(0),
    m_lastDelimiterIndex(0),
    m_errorColumn(0),
    m_pluginWidth(0),
    m_pluginHeight(0),
    m_comboBoxEncode(0),
    m_clearAll(0),
    m_firstIsValid(0),
    m_secondIsValid(0),
    m_initWindow(0),
    m_vScrollBarVisible(0),
    m_maxRowWidth(0),
    m_rowWidth(0),
    m_decimalSymbolIndex(0),
    m_lineNum(0),
    m_memoColCopy(0),
    m_lastHeight(0),
    m_round(0),
    m_minimumHeight(0),
    m_windowWidth(0),
    m_initialHeight(0),
    m_rectWidth(0)
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
  m_resizing = false;
  m_closing = false;

  m_amountColumn = -1;
  m_creditColumn = -1;
  m_dateColumn = 0;
  m_debitColumn = -1;
  m_categoryColumn = -1;
  m_memoColumn = 0;
  m_numberColumn = 0;
  m_payeeColumn = 0;
  m_previousColumn = 0;
  m_maxColumnCount = 0;
  m_endLine = 0;
  m_startLine = 1;
  m_topLine = 0;
  m_row = 0;
  m_visibleRows = 10;
  m_fieldDelimiterIndex = 0;
  m_header = 27;
  m_vScrollBarWidth = 0;
  m_hScrollBarHeight = 17;
  m_rowHeight = 30;
  m_header = 27;
  m_tableHeight = m_visibleRows * m_rowHeight + m_header - 3;
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

  m_investProcessing = new InvestProcessing;
  m_investProcessing->m_csvDialog = this;
  m_convertDate = new ConvertDate;
  m_parse = new Parse;
  m_parse->m_csvDialog = this;
  m_wiz = new CSVWizard;
  m_wiz->m_csvDialog = this;
  m_wiz->init();
  m_wiz->m_investProcessing = m_investProcessing;

  ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

  QFont font(QApplication::font());
  QFontMetrics cellFontMetrics(font);
  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    m_dpiDiff = 0;
  }  else {
    m_dpiDiff = 10;
  }
  init();
}

void CSVDialog::init()
{
  readSettingsProfiles();

  m_hScrollBarHeight = ui->tableWidget->horizontalScrollBar()->height();
  if (m_hScrollBarHeight < 17) {
    m_hScrollBarHeight = 17;  // for compatibility
  }

  installEventFilter(this);

  m_parse = new Parse;
  m_parse->m_csvDialog = this;

  m_investmentDlg = new InvestmentDlg;
  m_investmentDlg->m_investProcessing = m_investProcessing;
  m_investmentDlg->m_csvDialog = this;
  m_investProcessing->m_convertDat = m_convertDate;
  m_csvUtil = new CsvUtil;

  m_symbolTableDlg  = new SymbolTableDlg;
  m_symbolTableDlg->m_csvDialog = this;

  m_investProcessing->m_parse = m_parse;

  this->setAttribute(Qt::WA_DeleteOnClose, true);

  ui->tableWidget->setWordWrap(false);
  m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(-1);
  m_wiz->m_pageCompletion->ui->comboBox_thousandsDelimiter->setEnabled(false);

  m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);  //  The memo col might not get selected, so clear it
  m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->setEnabled(false);

  m_clearBrush = KColorScheme(QPalette::Normal).background(KColorScheme::NormalBackground);
  m_clearBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::NormalText);
  m_colorBrush = KColorScheme(QPalette::Normal).background(KColorScheme::PositiveBackground);
  m_colorBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::PositiveText);
  m_errorBrush = KColorScheme(QPalette::Normal).background(KColorScheme::NegativeBackground);
  m_errorBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::NegativeText);

  m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol->setMaxVisibleItems(12);
  m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol->setMaxVisibleItems(12);
  m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol->setMaxVisibleItems(12);
  m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setMaxVisibleItems(12);
  m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->setMaxVisibleItems(12);
  m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->setMaxVisibleItems(12);
  m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->setMaxVisibleItems(12);
  m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol->setMaxVisibleItems(12);

  m_vScrollBar = ui->tableWidget->verticalScrollBar();
  m_vScrollBar->setTracking(false);

  m_vHeaderWidth = 26;
  m_rectWidth = this->width() - 24;

  m_dateFormats << "yyyy/MM/dd" << "MM/dd/yyyy" << "dd/MM/yyyy";

  m_endColumn = 0;
  m_flagCol = -1;
  clearSelectedFlags();

  m_dateFormatIndex = m_wiz->m_pageLinesDate->ui->comboBox_dateFormat->currentIndex();
  m_date = m_dateFormats[m_dateFormatIndex];

  findCodecs();//                             returns m_codecs = codecMap.values();

  connect(m_vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(slotVertScrollBarMoved(int)));

  connect(m_wiz->m_pageLinesDate->ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_convertDate, SLOT(dateFormatSelected(int)));

  connect(m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol, SIGNAL(currentIndexChanged(int)), m_parse, SLOT(decimalSymbolSelected(int)));

  m_investmentDlg->init();
  Qt::WindowFlags eFlags = windowFlags();
  m_wiz->setWindowFlags(eFlags);
  m_wiz->show();
}//  CSVDialog

CSVDialog::~CSVDialog()
{
  delete ui;
  delete m_symbolTableDlg;
  delete m_wiz;
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
  m_wiz->m_pageIntro->m_index = 0;
  KSharedConfigPtr  myconfig = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
  KConfigGroup bankProfilesGroup(myconfig, "BankProfiles");

  m_profileList.clear();
  m_wiz->m_pageIntro->ui->combobox_source->clear();
  m_wiz->m_pageIntro->ui->combobox_source->addItem(i18n("Add New Profile"));
  QStringList list = bankProfilesGroup.readEntry("BankNames", QStringList());
  if (!list.isEmpty()) {
    for (int i = 0; i < list.count(); i++) {
      m_profileList.append(list[i]);
      QString txt = "Profiles-" + list[i];
      KConfigGroup profilesGroup(myconfig, txt);

      if (profilesGroup.exists()) {
        txt = profilesGroup.readEntry("FileType", QString());
        m_wiz->m_pageIntro->m_mapFileType.insert(list[i], txt);
        if (txt == m_fileType) {
          m_wiz->m_pageIntro->ui->combobox_source->addItem(list[i]);
          m_wiz->m_pageIntro->m_map.insert(list[i], m_wiz->m_pageIntro->m_index++);
        }
      }
    }
  }
  if (m_fileType == "Banking") {
    m_priorCsvProfile = bankProfilesGroup.readEntry("PriorCsvProfile", QString());
    if (m_priorCsvProfile.isEmpty()) {
      m_wiz->m_pageIntro->ui->combobox_source->setCurrentIndex(0);
    } else {
      m_profileName = m_priorCsvProfile;
      int indx = m_wiz->m_pageIntro->ui->combobox_source->findText(m_priorCsvProfile);
      m_wiz->m_pageIntro->ui->combobox_source->setCurrentIndex(indx);
      m_wiz->m_pageIntro->m_index = indx;
    }
  } else if (m_fileType == "Invest") {
    m_priorInvProfile = bankProfilesGroup.readEntry("PriorInvProfile", QString());
    if (m_priorInvProfile.isEmpty()) {
      m_wiz->m_pageIntro->ui->combobox_source->setCurrentIndex(0);
    } else {
      int indx = m_wiz->m_pageIntro->ui->combobox_source->findText(m_priorInvProfile);
      m_wiz->m_pageIntro->ui->combobox_source->setCurrentIndex(indx);
      m_wiz->m_pageIntro->m_index = indx;
      m_profileName = m_priorInvProfile;
    }
  }
  disconnect(m_wiz->m_pageIntro->ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), m_wiz->m_pageIntro, SLOT(slotLineEditingFinished()));
}

void CSVDialog::readSettings()
{
  m_profileExists = false;
  int tmp = -1;
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
    //  Allow payee col to be set here.
    connect(m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(payeeColumnSelected(int)));

    disconnect(m_wiz->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
    m_profileExists = true;
    QString txt = "Profiles-" + m_profileList[i];

    KConfigGroup profilesGroup(config, txt);

    //    txt = profilesGroup.readEntry("FileType", QString());//  Read earlier in readSettingsInit()
    m_dateFormatIndex = profilesGroup.readEntry("DateFormat", -1);
    m_wiz->m_pageLinesDate->ui->comboBox_dateFormat->setCurrentIndex(m_dateFormatIndex);

    //    m_encodeIndex = profilesGroup.readEntry("Encoding", 0);//  Read earlier in fileDialog()

    //    m_startLine = profilesGroup.readEntry("StartLine", -1) + 1;//  Read earlier in fileDialog()
    //    ...otherwises causes problem when may have been edited earlier in readFile()
    //    ...if start line was > end line.
    //  Keep present delimiter if 'Keep' chosen
    //  otherwise....
    if (m_needFieldDelimiter) {  //  no columnCount error
      m_needFieldDelimiter = false;
    }
    m_textDelimiterIndex = profilesGroup.readEntry("TextDelimiter", 0);

    //    m_csvPath = profilesGroup.readEntry("CsvDirectory", QString());//  Read earlier in fileDialog()

    m_debitFlag = profilesGroup.readEntry("DebitFlag", -1);
    //
    //  The following are needed should the same file be re-imported
    //  immediately, as column numbers will not be changed so no connects.
    //
    m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);

    m_payeeColumn = profilesGroup.readEntry("PayeeCol", -1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(m_payeeColumn);
    m_numberColumn = profilesGroup.readEntry("NumberCol", -1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(m_numberColumn);
    m_amountColumn = profilesGroup.readEntry("AmountCol", -1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(m_amountColumn);
    m_debitColumn = profilesGroup.readEntry("DebitCol", -1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(m_debitColumn);
    m_creditColumn = profilesGroup.readEntry("CreditCol", -1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(m_creditColumn);
    m_dateColumn = profilesGroup.readEntry("DateCol", -1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(m_dateColumn);
    m_categoryColumn = profilesGroup.readEntry("CategoryCol", -1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol->setCurrentIndex(m_categoryColumn);

    QList<int> list = profilesGroup.readEntry("MemoCol", QList<int>());
    int posn = 0;
    if ((posn = list.indexOf(-1)) > -1) {
      list.removeOne(-1);
    }
    m_memoColList = list;
    //
    //  Set up all memo fields...
    //
    if (m_maxColumnCount >= 3) {  //  if no m_columnCountError
      for (int i = 0; i < m_memoColList.count(); i++) {
        tmp = m_memoColList[i];
        if (tmp >= m_maxColumnCount) {
          list.removeOne(tmp);
          continue;
        }
        m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(tmp, QString().setNum(tmp + 1) + '*');
        if (tmp == m_payeeColumn) {  //  ...unless also a payee field.
          m_payeeColCopied = true;
          m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(tmp);
          continue;
        } else if (!m_payeeColCopied) {
          m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(tmp);
        }
        if (tmp < m_endColumn) {  //   Ensure profile memo column is valid
          m_memoColumn = tmp;
          if ((posn = list.indexOf(tmp) > -1) && (m_columnTypeList[tmp] != "memo") && (!m_columnTypeList[tmp].isEmpty())) {
            //  This saved memo setting would overwrite another value so drop it
            list.removeOne(tmp);
            m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(tmp, QString().setNum(tmp + 1));
          } else {
            m_columnTypeList[tmp] = "memo";
          }
        }
      }
    }
    m_memoColList = list;

    m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(-1);  //  Ensure UI gets changed.
    m_decimalSymbolIndex = profilesGroup.readEntry("DecimalSymbol", 0);
    m_decimalSymbol = m_parse->decimalSymbol(m_decimalSymbolIndex);
    m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(m_decimalSymbolIndex);
    m_wiz->m_pageCompletion->ui->comboBox_thousandsDelimiter->setCurrentIndex(-1);
    break;
  }
  KConfigGroup mainGroup(config, "MainWindow");

  m_pluginHeight = mainGroup.readEntry("Height", 640);
  m_pluginWidth = mainGroup.readEntry("Width", 800);

  connect(m_wiz->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  //  Index change after an activate, when payeeColumnCopy(), causes a second connect with confusing msg. so...
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(payeeColumnSelected(int)));
}

void CSVDialog::reloadUISettings()
{
  m_payeeColumn =  m_columnTypeList.indexOf("payee");
  m_numberColumn = m_columnTypeList.indexOf("number");
  m_debitColumn = m_columnTypeList.indexOf("debit");
  m_creditColumn = m_columnTypeList.indexOf("credit");
  m_dateColumn = m_columnTypeList.indexOf("date");
  m_amountColumn = m_columnTypeList.indexOf("amount");
  m_categoryColumn = m_columnTypeList.indexOf("category");
  m_startLine = m_wiz->m_pageLinesDate->ui->spinBox_skip->value();
  m_endLine = m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value();
}

void CSVDialog::createProfile(QString newName)
{
  KSharedConfigPtr  config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
  KConfigGroup bankProfilesGroup(config, "BankProfiles");

  bankProfilesGroup.writeEntry("BankNames", m_profileList);
  bankProfilesGroup.config()->sync();

  KConfigGroup bankGroup(config, "BankProfiles");
  QString txt = "Profiles-" + newName;

  KConfigGroup newProfilesGroup(config, txt);
  newProfilesGroup.writeEntry("FileType", m_fileType);
  if (m_fileType == "Invest") {
    newProfilesGroup.writeEntry("ShrsinParam", m_investProcessing->m_shrsinList);
    newProfilesGroup.writeEntry("DivXParam", m_investProcessing->m_divXList);
    newProfilesGroup.writeEntry("IntIncParam", m_investProcessing->m_intIncList);
    newProfilesGroup.writeEntry("BrokerageParam", m_investProcessing->m_brokerageList);
    newProfilesGroup.writeEntry("ReinvdivParam", m_investProcessing->m_reinvdivList);
    newProfilesGroup.writeEntry("BuyParam", m_investProcessing->m_buyList);
    newProfilesGroup.writeEntry("SellParam", m_investProcessing->m_sellList);
    newProfilesGroup.writeEntry("RemoveParam", m_investProcessing->m_removeList);
  }
  newProfilesGroup.config()->sync();
}

void CSVDialog::slotFileDialogClicked()
{
  if ((m_fileType != "Banking") || (m_profileName.isEmpty())) {
    if (m_fileType == "Banking") {
      KMessageBox::information(nullptr, i18n("Please select a profile type and enter a profile name."));
    }
    return;
  }
  //  remove all column widths left-over from previous file
  //  which can screw up row width calculation.
  for (int i = 0; i < ui->tableWidget->columnCount(); i++) {
    ui->tableWidget->setColumnWidth(i, 0);
    //  Needs to be here in case user selects new profile after cancelling prior one,
    //  or after selecting a file, reverses and does another select,
    //  but keep m_columnTypeList structure.
    m_columnTypeList << QString();
  }
  m_columnTypeList.clear();//  Needs to be here in case user selects new profile after cancelling prior one.

  m_inFileName.clear();
  m_url.clear();
  m_wiz->m_pageLinesDate->m_isColumnSelectionComplete = false;
  m_firstPass = true;
  m_firstRead = true;
  bool found = false;
  m_memoColCopied = false;
  m_payeeColCopied = false;
  m_columnsNotSet = true;  //  Don't check columns until they've been selected.
  m_wiz->m_pageBanking->m_bankingPageInitialized  = false;
  m_separatorPageVisible = false;
  m_delimiterError = false;
  m_needFieldDelimiter = true;
  m_initWindow = true;
  m_decimalSymbolIndex = 0;
  m_maxColumnCount = 0;
  m_fileEndLine = 0;
  ui->tableWidget->verticalScrollBar()->setValue(0);
  m_lastDelimiterIndex = 0;
  m_errorColumn = -1;
  m_accept = false;

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
  m_wiz->m_pageLinesDate->m_trailerLines = profilesGroup.readEntry("TrailerLines", 0);
  //  Set these column values here so they're available in time
  //  to align the columns in first displayLine().
  //  readSettings() is too late.
  //
  m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
  m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
  m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
  m_debitColumn = profilesGroup.readEntry("DebitCol", -1);
  if (m_debitColumn == -1) {      //                            If amount previously selected, set check radio_amount
    m_wiz->m_pageBanking->ui->radioBnk_amount->setChecked(true);
    m_wiz->m_pageBanking->ui->labelBnk_amount->setEnabled(true);
    m_wiz->m_pageBanking->ui->labelBnk_credits->setEnabled(false);
    m_wiz->m_pageBanking->ui->labelBnk_debits->setEnabled(false);
  } else {//                                                    ....else set check radio_debCred to clear amount col
    m_wiz->m_pageBanking->ui->radioBnk_debCred->setChecked(true);
    m_wiz->m_pageBanking->ui->labelBnk_credits->setEnabled(true);
    m_wiz->m_pageBanking->ui->labelBnk_debits->setEnabled(true);
    m_wiz->m_pageBanking->ui->labelBnk_amount->setEnabled(false);
  }

  m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol->setEnabled(true);

  m_startLine = profilesGroup.readEntry("StartLine", -1) + 1;
  m_wiz->m_pageLinesDate->ui->spinBox_skip->setValue(m_startLine);

  m_topLine = 0;
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

  if (m_url.isEmpty()) {
    return;
  }
  m_inFileName.clear();

  if (!KIO::NetAccess::download(m_url, m_inFileName, 0)) {
    KMessageBox::detailedError(nullptr, i18n("Error while loading file '%1'.", m_url.prettyUrl()),
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

  KConfigGroup mainGroup(config, "MainWindow");
  m_pluginHeight = mainGroup.readEntry("Height", 640);
  m_pluginWidth = mainGroup.readEntry("Width", 800);

  readFile(m_inFileName);
  m_csvPath = m_inFileName;
  posn = m_csvPath.lastIndexOf("/");
  m_csvPath.truncate(posn + 1);   //           keep last "/"

  readSettings();
  QString str = "~/" + m_csvPath.section('/', 3);
  KConfigGroup dirGroup(config, profileName);
  if (m_wiz->m_pageIntro->ui->checkBoxSkipSetup) {
    dirGroup.writeEntry("CsvDirectory", str);  //          save selected path
    dirGroup.writeEntry("Encoding", m_encodeIndex);  //    ..and encoding
    dirGroup.writeEntry("FileType", m_fileType);  //       ..and fileType
    dirGroup.config()->sync();
  }
  enableInputs();

  int index = m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol->currentIndex();
  decimalSymbolSelected(index);

  //The following two items do not *Require* an entry so old values must be cleared.
  m_trData.number.clear();  //                 this needs to be cleared or gets added to next transaction
  m_trData.memo.clear();  //                   this too, as neither might be overwritten by new data.

  if (m_wiz->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    m_wiz->m_pageCompletion->initializePage();//      Skip setup and go to Completion.
    m_wiz->m_pageIntro->initializePage();
  } else {
    m_wiz->m_wizard->next();
  }
}

void CSVDialog::readFile(const QString& fname)
{
  if (m_fieldDelimiterIndex == -1) {
    return;
  }
  m_rowWidth = 0;
  m_importError = false;
  m_payeeColAdded = false;
  m_clearAll = false;
  m_firstIsValid = false;
  m_secondIsValid = false;
  m_firstField = true;
  m_errorFoundAlready = false;
  m_rowWidthsDone = false;
  m_initWindow = true;
  m_vScrollBarVisible = false;

  int columnCount = 0;
  MyMoneyStatement st = MyMoneyStatement();
  if (!fname.isEmpty()) {
    m_inFileName = fname;
  }
  ui->tableWidget->clear();//         including vert headers
  m_inBuffer.clear();
  m_outBuffer.clear();

  m_qifBuffer = "!Type:Bank\n";
  m_row = 0;

  disconnect(m_wiz->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  disconnect(m_wiz->m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endLineChanged(int)));

  m_fieldDelimiterIndex = m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex();
  m_lastDelimiterIndex = m_fieldDelimiterIndex;
  m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_textDelimiterIndex = m_wiz->m_pageSeparator->ui->comboBox_textDelimiter->currentIndex();
  m_parse->setTextDelimiterIndex(m_textDelimiterIndex);
  m_textDelimiterCharacter = m_parse->textDelimiterCharacter(m_textDelimiterIndex);

  QFile  m_inFile(m_inFileName);
  m_inFile.open(QIODevice::ReadOnly);  // allow a Carriage return -// QIODevice::Text
  QTextStream inStream(&m_inFile);
  QTextCodec* codec = QTextCodec::codecForMib(m_codecs.value(m_encodeIndex)->mibEnum());
  inStream.setCodec(codec);

  QString buf = inStream.readAll();

  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(amountColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol, SIGNAL(activated(int)), m_wiz, SLOT(amountColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(debitColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol, SIGNAL(activated(int)), m_wiz, SLOT(debitColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(creditColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol, SIGNAL(activated(int)), m_wiz, SLOT(creditColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(memoColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol, SIGNAL(activated(int)), m_wiz, SLOT(memoColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(numberColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol, SIGNAL(activated(int)), m_wiz, SLOT(numberColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(dateColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol, SIGNAL(activated(int)), m_wiz, SLOT(dateColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(payeeColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol, SIGNAL(activated(int)), m_wiz, SLOT(payeeColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(categoryColumnSelected(int)));
  disconnect(m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol, SIGNAL(activated(int)), m_wiz, SLOT(categoryColumnSelected(int)));

  //  Parse the buffer
  m_columnCountList.clear();
  QString data;
  m_lineList = m_parse->parseFile(buf, 1, 0);  //                    Changed to display whole file.
  m_lineNum = 0;

  int totalDelimiterCount[4] = {0}; //  Total in file for each delimiter
  int thisDelimiterCount[4] = {0};  //  Total in this line for each delimiter
  int colCount = 0;                 //  Total delimiters in this line
  m_possibleDelimiter = 0;

  //  Check all lines to find maximum column count.
  //  Also check field delimiter
  m_delimiterError = false;
  for (int i = 0; i < m_lineList.count(); i++) {
    QString data = m_lineList[i];

    for (int delim = 0; delim < 4; delim++) {  //  Four possible delimiters
      m_parse->setFieldDelimiterIndex(delim);
      //  parse each line using each delimiter
      m_columnList = m_parse->parseLine(data);
      //  m_columnList.count() reflects number of delimiters per line so shows most likely one to use .
      //  Changed to sum total file, not just individual lines.
      colCount = m_columnList.count();

      if (colCount > thisDelimiterCount[delim]) {
        thisDelimiterCount[delim] = colCount;
      }
      if (thisDelimiterCount[delim] > m_maxColumnCount) {
        m_maxColumnCount = thisDelimiterCount[delim];
      }
      m_columnCountList << colCount;  // Number of columns in each line.
      totalDelimiterCount[delim] += thisDelimiterCount[delim];
      if (totalDelimiterCount[delim] > totalDelimiterCount[m_possibleDelimiter]) {
        m_possibleDelimiter = delim;
      }
    }
  }
  ui->tableWidget->setColumnCount(m_maxColumnCount);
  if ((columnCount < 5) || (m_possibleDelimiter != m_fieldDelimiterIndex)) {
    m_delimiterError = true;
  }

  if (m_firstRead) {
    m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol->clear();  //   clear all existing items before adding new ones
    m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol->clear();
    m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol->clear();
    m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->clear();
    m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->clear();
    m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->clear();
    m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->clear();
    m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol->clear();

    for (int i = 0; i < m_maxColumnCount; i++) {  //         populate comboboxes with col # values
      //  Start to build m_columnTypeList before comboBox stuff below
      //  because that causes connects which access m_columnTypeList
      //
      m_columnTypeList << QString();  //                       clear all column types
      QString t;
      t.setNum(i + 1);
      m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol->addItem(t);
      m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol->addItem(t);
      m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol->addItem(t);
      m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->addItem(t);
      m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->addItem(t);
      m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->addItem(t);
      m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->addItem(t);
      m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol->addItem(t);
      //  Will have to reload comboboxes after exit.
    }
    m_firstRead = false;
  }
  if (m_fileEndLine == 0) {
    m_fileEndLine = m_parse->lastLine();
  }
  if (m_fileEndLine > m_wiz->m_pageLinesDate->m_trailerLines) {
    m_endLine = m_fileEndLine - m_wiz->m_pageLinesDate->m_trailerLines;
  } else {
    m_endLine = m_fileEndLine;  //                           Ignore m_trailerLines as > file length.
  }
  m_wiz->m_pageLinesDate->ui->spinBox_skip->setMaximum(m_fileEndLine);
  m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setMaximum(m_fileEndLine);

  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(amountColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol, SIGNAL(activated(int)), m_wiz, SLOT(amountColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(debitColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol, SIGNAL(activated(int)), m_wiz, SLOT(debitColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(creditColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol, SIGNAL(activated(int)), m_wiz, SLOT(creditColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(memoColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol, SIGNAL(activated(int)), m_wiz, SLOT(memoColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(numberColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol, SIGNAL(activated(int)), m_wiz, SLOT(numberColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(dateColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol, SIGNAL(activated(int)), m_wiz, SLOT(dateColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol, SIGNAL(activated(int)), m_wiz, SLOT(payeeColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol, SIGNAL(currentIndexChanged(int)), m_wiz, SLOT(categoryColumnSelected(int)));
  connect(m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol, SIGNAL(activated(int)), m_wiz, SLOT(categoryColumnSelected(int)));

  m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setValue(m_endLine);
  if (m_startLine > m_endLine) {  //                                                     Don't allow m_startLine > m_endLine
    m_startLine = m_endLine;
  }
  m_wiz->m_pageLinesDate->ui->spinBox_skip->setValue(m_startLine);
  for (int i = 0; i < ui->tableWidget->columnCount(); i++) {
    ui->tableWidget->setColumnWidth(i, 0);
  }
  m_screenUpdated = false;

  //  Display the buffer
  st.m_eType = MyMoneyStatement::etNone;
  for (int line = 0; line < m_lineList.count(); line++) {
    m_inBuffer = m_lineList[line];

    displayLine(m_inBuffer);
    if (m_importNow) {
      clearCellsBackground();
    }
    //  user now ready to continue && line is in wanted range
    //
    if ((m_importNow) && (line >= m_startLine - 1) && (line <= m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value() - 1)) {
      reloadUISettings();
      //  Need to reload column settings
      int ret = processQifLine(m_outBuffer);  //        parse a line
      if (ret == KMessageBox::Ok) {
        csvImportTransaction(st);
      } else {
        m_importNow = false;
        m_wiz->m_wizard->back();  //                          Have another try at the import
      }
    }
  }  //  reached end of buffer

  if (ui->tableWidget->verticalScrollBar()->isVisible()) {
    m_vScrollBarWidth = 17;
  } else {
    m_vScrollBarWidth = 0;
  }

  setWindowSize(-1, -1);

  m_wiz->m_pageLinesDate->ui->labelSet_skip->setEnabled(true);
  m_wiz->m_pageLinesDate->ui->spinBox_skip->setEnabled(true);

  m_endColumn = m_maxColumnCount;
  connect(m_wiz->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  connect(m_wiz->m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endLineChanged(int)));

  //  Export statement

  if (m_importNow) {

    if (m_fileType == "Banking") {
      if ((!m_inFileName.isEmpty()) && ((m_amountColumn >= 0) || ((m_debitColumn >= 0) && (m_creditColumn >= 0)))) {
        if (m_amountColumn >= 0) {
          updateDecimalSymbol("amount", m_amountColumn);
        } else {
          updateDecimalSymbol("debit", m_debitColumn);
          updateDecimalSymbol("credit", m_creditColumn);
        }
      }
    }

    emit statementReady(st);  //  via CsvImporterPlugin::slotGetStatement(MyMoneyStatement& s)
    m_screenUpdated = true;
    m_importNow = false;
    // the life cycle of the contents of this map is one import process
    m_hashMap.clear();
  }
  if (m_delimiterError) {
    m_fieldDelimiterIndex = m_possibleDelimiter;
    m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
    m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(m_possibleDelimiter);
  }
  m_inFile.close();
  m_columnsNotSet = false;  //            Allow checking of columns now.
}

void CSVDialog::setWindowSize(int firstLine, int lastLine)
{
  int screenHeight = QApplication::desktop()->height();
  int launcherHeight = 41;      //  allow for horizontal app launch bar - approx
  int variousMarginsEtc = 120;  //  all margins, hscrollbar, title, gap between frames, etc.
  int maxLines = (screenHeight - launcherHeight - variousMarginsEtc) / m_rowHeight;

  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    m_dpiDiff = 0;
  } else {
    m_dpiDiff = 5;
  }
  if (m_initWindow) {
    m_visibleRows = qMin(m_lineList.count(), maxLines);
    m_initWindow = false;
  }
  m_tableHeight = m_visibleRows * m_rowHeight + m_header + m_hScrollBarHeight + m_dpiDiff;
  if (firstLine == - 1 || lastLine == -1) {
    updateColumnWidths(0, m_lineList.count() - 1);
  } else {
    updateColumnWidths(firstLine, lastLine);
  }
  QRect rect;
  rect = ui->frame_main->frameRect();
  ui->frame_main->setMinimumHeight(120);

  //  scrollbar.isVisible() is unreliable so ..
  if (m_visibleRows < m_fileEndLine) {
    //  vert scrollbar is visible
    m_vScrollBarWidth = ui->tableWidget->verticalScrollBar()->width();
  } else {
    m_vScrollBarWidth = 0;
  }
  QMargins hLayout_MainMargin = ui->horizontalLayout_Main->layout()->contentsMargins();  //  table frame margins
  QMargins vLayoutMargin = ui->verticalLayout->layout()->contentsMargins();              //  window margins

  int scrollbarWidth = 17;  //  scrollbar space for when needed
  int wd = m_rowWidth + m_vHeaderWidth +  2 * (vLayoutMargin.left() + 1) + 12 + hLayout_MainMargin.left() + hLayout_MainMargin.right() + scrollbarWidth;
  if (wd > QApplication::desktop()->width()) {
    //
    //  if set to full desktop()->width(), causes a spontaneous resize event
    //  and upsets wanted resizes,  so ...
    //
    wd = QApplication::desktop()->width() - 5;
  }
  //
  //  resize
  //
  resize(wd , m_tableHeight + 4 *(vLayoutMargin.top() + 1) + 15);

  rect.setHeight(m_tableHeight + 2 *(vLayoutMargin.left() + 1) + 2 + 1);
  rect.setWidth(width() - hLayout_MainMargin.left() - hLayout_MainMargin.right());
  ui->frame_main->setFrameRect(rect);
}

void CSVDialog::updateColumnWidths(int firstLine, int lastLine)
{
  m_rowWidth = 0;
  m_fileEndLine = m_parse->lastLine();

  QFont font(QApplication::font());
  QFontMetrics cellFontMetrics(font);
  //
  //  Need to recalculate column widths in the visible rows,
  //  to allow shrinking or expanding with the data.
  //
  for (int col = 0; col < ui->tableWidget->columnCount(); col++) {
    int maxColWidth = 0;

    for (int row = firstLine; row <= lastLine; row++) {
      if ((row >= m_lineList.count()) || (row >= m_fileEndLine)) {
        break;
      }
      if (ui->tableWidget->item(row, col) == 0) {  //  cell does not exist
        continue;
      }
      //
      //  Ensure colwidth is wide enough for true data width.
      //
      int colWidth = 0;
      QLabel label;
      label.setText(ui->tableWidget->item(row, col)->text() + "  ");
      int wd = 1.05 * cellFontMetrics.width(label.text() + "  ");  //  *1.05 for distro compatibility
      if (wd > colWidth) {
        colWidth = wd;
      }
      if (colWidth > maxColWidth) {
        maxColWidth = colWidth;
      }
    }  //  end rows

    ui->tableWidget->setColumnWidth(col, maxColWidth);

    m_rowWidth += maxColWidth;
  }  //  end cols
  return;
}

void CSVDialog::displayLine(const QString& data)
{
  QString str = data;
  QFont font(QApplication::font());
  ui->tableWidget->setFont(font);

  if (m_wiz->m_pageBanking->ui->radioBnk_amount->isChecked()) {
    m_amountColumn = m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->currentIndex();
    m_debitColumn = -1;
    m_creditColumn = -1;
  } else {
    m_amountColumn = -1;
    m_debitColumn = m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->currentIndex();
    m_creditColumn = m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->currentIndex();
  }
  int col = 0;
  m_fieldDelimiterIndex = m_possibleDelimiter;
  m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_parse->setTextDelimiterIndex(m_wiz->m_pageSeparator->ui->comboBox_textDelimiter->currentIndex());
  m_textDelimiterCharacter = m_parse->textDelimiterCharacter(m_textDelimiterIndex);
  //
  //                 split data into fields
  //
  m_columnList = m_parse->parseLine(str);
  if ((m_memoColCopied) || (m_payeeColCopied))  {
    while (m_columnList.count() < m_maxColumnCount) {
      m_columnList << "";  //  Get m_columnList to correct size
    }
  }
  //  If making copy of memocol or payeecol, check the columns actually exist...
  if ((!m_firstPass) && (m_payeeColumn <= m_columnCountList[m_lineNum]) && (m_memoColumn <= m_columnCountList[m_lineNum])) {
    if ((m_memoColCopied) && (m_memoColCopy < m_columnList.count())) {  //           ...then make the copy here
      m_columnList[m_endColumn - 1] = m_columnList[m_payeeColumn];
      m_columnTypeList[m_columnTypeList.count() - 1] = "memo";
    } else if ((m_payeeColCopied) && (m_payeeColumn < m_columnList.count())) {  //   ...or here
      m_columnList[m_endColumn - 1] = m_columnList[m_payeeColumn];
      m_columnTypeList[m_columnTypeList.count() - 1] = "memo";
    }
  }

  m_inBuffer.clear();
  QStringList::const_iterator constIterator;
  QString txt;
  for (constIterator = m_columnList.constBegin(); constIterator != m_columnList.constEnd(); ++constIterator) {
    txt = (*constIterator) + "  ";
    QTableWidgetItem *item = new QTableWidgetItem;//       new item for UI
    item->setText(txt);
    if (txt.toDouble() != 0.0) {
      item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    } else {
      item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    ui->tableWidget->setRowCount(m_row + 1);
    ui->tableWidget->setItem(m_row, col, item);  //        add items to UI here
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

void CSVDialog::markUnwantedRows()
{
  if (m_fileType == "Banking") {
    if (!m_wiz->m_pageBanking->m_bankingPageInitialized) {
      return;
    }
  } else if (m_fileType == "Invest") {
    if (!m_wiz->m_pageInvestment->m_investPageInitialized) {
      return;
    }
  }
  int first = m_wiz->m_pageLinesDate->ui->spinBox_skip->value() - 1;
  int last = m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value() - 1;
  //
  //  highlight unwanted lines instead of not showing them.
  //
  QBrush brush;
  QBrush brushText;
  for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
    if ((row < first) || (row > last)) {
      brush = m_errorBrush;
      brushText = m_errorBrushText;
    } else {
      brush = m_clearBrush;
      brushText = m_clearBrushText;
    }
    for (int col = 0; col < ui->tableWidget->columnCount(); col ++) {
      if (ui->tableWidget->item(row, col) != 0) {
        ui->tableWidget->item(row, col)->setBackground(brush);
        ui->tableWidget->item(row, col)->setForeground(brushText);
      }
    }
  }
}

int CSVDialog::processQifLine(QString& iBuff)
{
  //   parse an input line
  QString newTxt;
  m_firstField = true;
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
  for (int i = 0; i < m_columnList.count(); i++) {
    //  Use actual column count for this line instead of m_endColumn, which could be greater.
    if (m_columnTypeList[i] == "number") {
      txt = m_columnList[i];
      m_trData.number = txt;
      m_qifBuffer = m_qifBuffer + 'N' + txt + '\n';     // Number column
    } else if (m_columnTypeList[i] == "date") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt = txt.remove(m_textDelimiterCharacter);       //   "16/09/2009
      QDate dat = m_convertDate->convertDate(txt);      //  Date column
      if (dat == QDate()) {
        KMessageBox::sorry(this, i18n("<center>An invalid date has been detected during import.</center>"
                                      "<center><b>'%1'</b></center>"
                                      "Please check that you have set the correct date format,\n"
                                      "<center>and start and end lines.</center>"
                                      , txt), i18n("CSV import"));
        m_importError = true;
        return KMessageBox::Cancel;
      }
      QString qifDate = dat.toString(m_dateFormats[m_dateFormatIndex]);
      m_qifBuffer = m_qifBuffer + 'D' + qifDate + '\n';
      m_trData.date = dat;
    } else if (m_columnTypeList[i] == "payee") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      if (txt.trimmed().isEmpty()) {  //             just blanks would confuse any matching
        txt.clear();
        m_columnList[m_payeeColumn] = txt;
      }
      txt.remove('~');  //                              replace NL which was substituted
      txt = txt.remove('\'');
      if ((!m_firstPass) && (m_memoColCopied)) {
        m_columnList[m_payeeColumn] = txt;
      }
      m_trData.payee = txt;
      m_qifBuffer = m_qifBuffer + 'P' + txt + '\n';  //  Detail column
    }

    else if (m_columnTypeList[i] == "amount") {      // Is this Amount column
      ++neededFieldsCount;

      //  For a file which uses a flag field value to indicate if amount is a debit or a credit.
      //  Resource file DebitFlag setting of -1 means 'ignore/notused'.
      //  DebitFlag setting of >=0 indicates the column containing the flag.

      if (m_flagCol == -1) {      //                    it's a new file
        switch (m_debitFlag) {  //                      Flag if amount is debit or credit
          case -1://                                    Ignore flag
            m_flagCol = 0;//                            ...and continue
            break;
          case  0://                                    Ask for column no.of flag
            m_flagCol = columnNumber(i18n("Enter debit flag column number"));
            if (m_flagCol == 0) {      //               0 means Cancel was pressed
              return KMessageBox::Cancel;//           ... so exit
            }
            break;
          default : m_flagCol = m_debitFlag;//          Contains flag/column no.
        }
      }
      if ((m_flagCol < 0) || (m_flagCol > m_endColumn)) {      // shouldn't get here
        KMessageBox::sorry(nullptr, i18n("An invalid column was entered.\n"
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

    else if ((m_columnTypeList[i] == "debit") || (m_columnTypeList[i] == "credit")) {      //  Credit or debit?
      ++neededFieldsCount;
      if (!ensureBothFieldsValid(i)) {
        return KMessageBox::Cancel;
      }
    } else if (m_columnTypeList[i] == "memo") {     //         could be more than one
      txt = m_columnList[i];
      txt.replace('~', "\n");  //                             replace NL which was substituted
      if ((!m_firstPass) && (txt.isEmpty()) && (m_payeeColCopied)) {
        txt = m_columnList[m_payeeColumn];
        m_columnList[i] = txt;
      }
      if (!memo.isEmpty()) {
        memo += '\n';//                                       separator for multiple memos
      }
      memo += txt;//                                          next memo
    }//end of memo field

    else if (m_columnTypeList[i] == "category") {  //         "category"
      txt = m_columnList[i];
      txt.replace('~', "\n");  //                             replace NL which was substituted
      txt = m_columnList[m_categoryColumn];
      m_columnList[i] = txt;
      txt.remove('~');  //                                    replace NL which was substituted
      txt = txt.remove('\'');

      m_trData.category = txt;
      m_csvSplit.m_strCategoryName = m_columnList[m_categoryColumn];
      m_csvSplit.m_strMemo = m_trData.memo;
      m_csvSplit.m_amount = m_trData.amount;
      m_qifBuffer = m_qifBuffer + 'L' + txt + '\n';  //       Category column
    }//end of category field
    m_outBuffer += m_columnList[i];  //                       keep any changes
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

    QString errMsg = i18n("<center>The columns selected are invalid.</center>"
                          "There must an amount or debit and credit fields, plus date and payee fields.");
    if (m_wiz->m_pageIntro->ui->checkBoxSkipSetup->isEnabled()) {
      errMsg += i18n("<center>You possibly need to check the start and end line settings, or reset 'Skip setup'.</center>");
    }
    KMessageBox::information(nullptr, errMsg);
    m_importError = true;
    return KMessageBox::Cancel;
  }
}

QString CSVDialog::clearInvalidField(QString m_firstValue, QString m_secondValue)
{
  if (MyMoneyMoney(m_firstValue).isZero()) {
    m_firstValue = QString();
    return m_secondValue;
  } else {
    m_secondValue = QString();
    return m_firstValue;
  }
}

int CSVDialog::ensureBothFieldsValid(int col)
{
  //  if debit and credit fields are present,
  //  ensure the combination is valid
  int ret = 0;
  QString zero = "0" + m_decimalSymbol + "00";
  QString newTxt;
  QString txt = m_columnList[col].trimmed();  //              A field of blanks is not good...
  if ((!txt.isEmpty()) && ((col == m_debitColumn))) {
    txt = '-' + txt;  //                                      Mark as -ve
  }
  if (!txt.isEmpty() && !txt.contains(m_decimalSymbol)) {
    //  This field has no decimal part
    txt += m_decimalSymbol + "00";
  }
  if (m_firstField) {  //                                     Debit or credit, whichever comes first.
    m_firstValue = txt;  //                                   Save first field until second arrives.
    m_firstType = m_columnTypeList[col];
  } else {  //                                                Second field.
    m_secondType = m_columnTypeList[col];
    if (txt.isEmpty()) {
      m_secondValue = txt;
    } else if (QString::number(txt.toDouble(), 'f', 2) == zero) {
      m_secondValue = QString();
      txt = m_firstValue;
    }
    if ((txt.isEmpty()) || (QString::number(txt.toDouble(), 'f', 2) == zero)) {   //  If second field empty,...
      m_secondValue = txt;
      txt = m_firstValue;  //                                                      ...use first (which could also be empty..)
    } else {
      m_secondValue = txt;
    }
  }  //  end of second field.
  bool bothFieldsNotZero = false;

  if (!m_firstField) {  //                                    Process outcome.
    //  a field is valid only if it is non-zero and if the other (credit/debit) field is empty
    m_firstIsValid = m_firstValue != zero && m_secondValue.isEmpty();
    m_secondIsValid = m_secondValue != zero && m_firstValue.isEmpty();
    //  need to remove temporarily any minus sign so keep both originalfields
    QString firstTemp = m_firstValue;
    firstTemp = firstTemp.remove('-');
    QString secondTemp = m_secondValue;
    secondTemp = secondTemp.remove('-');
    bothFieldsNotZero = firstTemp != zero && secondTemp != zero;
    //  beware - an empty field is not zero so bypasses this message
    if ((firstTemp == zero || secondTemp == zero) && (m_clearAll == false)) {
      //  Warn user if either field is zero - so needs to be cleared
      // user may opt to clear just this or all similar
      int ret = KMessageBox::questionYesNoCancel(this, i18n("<center>On row '%5', the '%1' field contains '%2', and the '%3' field contains '%4'.</center>"
                "<center>This combination is not valid.</center>"
                "<center>If you wish for just this zero field to be cleared, click 'Clear this'.</center>"
                "<center>Or, if you wish for all such zero fields to be cleared, click 'Clear all'.</center>"
                "<center>Otherwise, click 'Cancel'.</center>",
                m_firstType, m_firstValue, m_secondType, m_secondValue, m_row), i18n("CSV invalid field values"),
                KGuiItem(i18n("Clear this")),
                KGuiItem(i18n("Clear all")),
                KGuiItem(i18n("Cancel")));
      switch (ret) {
        case KMessageBox::Yes:
          txt = clearInvalidField(m_firstValue, m_secondValue);
          break;
        case KMessageBox::No:
          m_clearAll = true;
          txt = clearInvalidField(m_firstValue, m_secondValue);
          break;
        case KMessageBox::Cancel:
          m_clearAll = false;
          return ret;
      }
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      m_trData.amount = newTxt;
      m_qifBuffer = m_qifBuffer + 'T' + m_trData.amount + '\n';
    }  //  end of first error test
    else if (bothFieldsNotZero && !m_firstValue.isEmpty() && !m_secondValue.isEmpty()) {  //  credit and debit contain values - not good
      //  both debit and credit have entries so ask user how to proceed.
      //  if just one field is empty, that's OK - bypass this message
      ret = KMessageBox::questionYesNoCancel(this, i18n("<center>The %1 field contains '%2'</center>"
                                             "<center>and the %3 field contains '%4'.</center>"
                                             "<center>Please choose which you wish to accept.</center>",
                                             m_columnTypeList[m_debitColumn], m_columnList[m_debitColumn], m_columnTypeList[m_creditColumn], m_columnList[m_creditColumn]), i18n("CSV invalid field values"),
                                             KGuiItem(i18n("Accept %1", m_columnTypeList[m_debitColumn])),
                                             KGuiItem(i18n("Accept %1", m_columnTypeList[m_creditColumn])),
                                             KGuiItem(i18n("Cancel")));
      if (ret == KMessageBox::Cancel) {
        return ret;
      }
      if (ret == KMessageBox::Yes) {
        m_trData.amount = '-' + m_parse->possiblyReplaceSymbol(m_columnList[m_debitColumn]);
      } else if (ret == KMessageBox::No) {
        m_trData.amount = m_parse->possiblyReplaceSymbol(m_columnList[m_creditColumn]);
      }
      m_qifBuffer = m_qifBuffer + 'T' + m_trData.amount + '\n';
    } //  end of second error test
    else {  //  resolved amount
      if (!m_firstValue.isEmpty() && m_firstValue != zero) {  //           m_firstIsValid
        m_trData.amount = m_firstValue;
      } else if (!m_secondValue.isEmpty() && m_secondValue != zero) {  //  m_secondIsValid
        m_trData.amount = m_secondValue;
      } else {
        m_trData.amount = QString(zero);
      }
      m_qifBuffer = m_qifBuffer + 'T' + m_trData.amount + '\n';
    }  //  end of second field
  }
  m_firstField = !m_firstField;
  return KMessageBox::Yes;
}

void CSVDialog::csvImportTransaction(MyMoneyStatement& st)
{
  MyMoneyStatement::Transaction tr;
  MyMoneyStatement::Split s1;
  QString tmp;
  QString accountId;
  QString payee = m_trData.payee;//                              extractLine('P')
  // Process transaction data
  tr.m_strBankID = m_trData.id;
  tr.m_datePosted = m_trData.date;
  if (!tr.m_datePosted.isValid()) {
    int rc = KMessageBox::warningContinueCancel(nullptr, i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
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

  s1.m_amount = tr.m_amount;

  tmp = m_trData.number;
  tr.m_strNumber = tmp;

  if (!payee.isEmpty()) {
    tr.m_strPayee = m_trData.payee;
  }

  tr.m_strMemo = m_trData.memo;
  s1.m_strMemo = tr.m_strMemo;

  MyMoneyAccount account;
  // use the same values for the second split, but clear the ID and reverse the value
  MyMoneyStatement::Split s2 = s1;
  s2.m_reconcile = tr.m_reconcile;
  s2.m_amount = (-s1.m_amount);

  // standard transaction

  if (m_categoryColumn >= 0) {
    tmp = m_trData.category;
    // it's an expense / income
    tmp = tmp.trimmed();
    accountId = m_csvUtil->checkCategory(tmp, s1.m_amount, s2.m_amount);

    if (!accountId.isEmpty()) {
      s2.m_accountId = accountId;
      s2.m_strCategoryName = tmp;
      tr.m_listSplits.append(s2);
    }
  }
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
  if (m_fileType != "Banking") {
    return;
  }
  if ((m_dateSelected) && (m_payeeSelected) &&
      ((m_amountSelected || (m_debitSelected && m_creditSelected)))) {
    m_importNow = true; //                  all necessary data is present

    int skp = m_wiz->m_pageLinesDate->ui->spinBox_skip->value() - 1;
    if (skp > m_endLine) {
      KMessageBox::sorry(nullptr, i18n("<center>The start line is greater than the end line.\n</center>"
                                 "<center>Please correct your settings.</center>"), i18n("CSV import"));
      m_importError = true;
      return;
    }
    if (m_importError) {  //                possibly from wrong decimal symbol or date format
      return;
    }
    m_parse->setSymbolFound(false);
    readFile(m_inFileName);
    markUnwantedRows();
    m_screenUpdated = true;
  } else {
    QString errMsg = i18n("<center>There must an amount or debit and credit fields, plus date and payee fields.</center>");
    if (m_wiz->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
      errMsg += i18n("<center>As you had skipped Setup, the wizard will now return you to the setups.</center>");
    }
    KMessageBox::information(nullptr, errMsg);
    m_importError = true;
  }
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

void CSVDialog::clearSelectedFlags()
{
  for (int i = 0; i < m_maxColumnCount; i++) {
    m_columnTypeList[i].clear();   //           set to all empty but keep size
  }
  if (m_columnTypeList.contains("memo")) {  //  need to remove a payee copy item
    int pos = m_columnTypeList.indexOf("memo");
    m_columnTypeList.takeAt(pos);
  }

  m_dateSelected = false;
  m_payeeSelected = false;
  m_amountSelected = false;
  m_debitSelected = false;
  m_creditSelected = false;
  m_memoSelected = false;
  m_numberSelected = false;
  m_wiz->m_pageBanking->ui->radioBnk_amount->setEnabled(true);
  m_wiz->m_pageBanking->ui->radioBnk_debCred->setEnabled(true);
}

void CSVDialog::clearColumnNumbers()
{
  m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
  m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
  m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
  m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
  m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
  m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
  m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
  m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol->setCurrentIndex(-1);
}

void CSVDialog::clearComboBoxText()
{
  for (int i = 0; i < m_maxColumnCount; i++) {
    m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(i, QString().setNum(i + 1));
  }
}

void CSVDialog::encodingChanged(int index)
{
  m_encodeIndex = index;
  if (!m_inFileName.isEmpty())
    readFile(m_inFileName);
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
  m_wiz->m_pageLinesDate->ui->spinBox_skip->setEnabled(true);
  m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol->setEnabled(true);
  m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol->setEnabled(true);
  m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol->setEnabled(true);
  m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setEnabled(true);
  m_wiz->m_pageBanking->ui->button_clear->setEnabled(true);
  m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setEnabled(true);
  m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->setEnabled(true);

  if (m_wiz->m_pageBanking->ui->radioBnk_amount->isChecked()) {
    m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(true);
    m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(false);
    m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(false);
  } else {
    m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(false);
    m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(true);
    m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(true);
  }
}

void CSVDialog::saveSettings()
{
  if ((m_fileType != "Banking") || (m_inFileName.isEmpty())) {      //  don't save if no file loaded
    return;
  }

  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup mainGroup(config, "MainWindow");
  mainGroup.writeEntry("Height", height());
  mainGroup.writeEntry("Width", width());
  mainGroup.config()->sync();

  KConfigGroup bankProfilesGroup(config, "BankProfiles");

  bankProfilesGroup.writeEntry("BankNames", m_profileList);
  int indx = m_wiz->m_pageIntro->ui->combobox_source->findText(m_priorCsvProfile, Qt::MatchExactly);
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
    profilesGroup.writeEntry("DateFormat", m_wiz->m_pageLinesDate->ui->comboBox_dateFormat->currentIndex());
    profilesGroup.writeEntry("DebitFlag", m_debitFlag);
    profilesGroup.writeEntry("FieldDelimiter", m_fieldDelimiterIndex);
    profilesGroup.writeEntry("FileType", m_fileType);
    profilesGroup.writeEntry("TextDelimiter", m_textDelimiterIndex);
    profilesGroup.writeEntry("StartLine", m_wiz->m_pageLinesDate->ui->spinBox_skip->value() - 1);
    profilesGroup.writeEntry("DecimalSymbol", m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol->currentIndex());
    profilesGroup.writeEntry("TrailerLines", m_wiz->m_pageLinesDate->m_trailerLines);

    profilesGroup.writeEntry("DateCol", m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol->currentIndex());
    profilesGroup.writeEntry("PayeeCol", m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol->currentIndex());

    QList<int> list = m_memoColList;
    int posn = 0;
    if ((posn = list.indexOf(-1)) > -1) {
      list.removeOne(-1);
    }
    profilesGroup.writeEntry("MemoCol", list);

    profilesGroup.writeEntry("NumberCol", m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol->currentIndex());
    profilesGroup.writeEntry("AmountCol", m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->currentIndex());
    profilesGroup.writeEntry("DebitCol", m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->currentIndex());
    profilesGroup.writeEntry("CreditCol", m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->currentIndex());
    profilesGroup.writeEntry("CategoryCol", m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol->currentIndex());
    profilesGroup.config()->sync();
  }
  m_inFileName.clear();
  ui->tableWidget->clear();//     in case later reopening window, clear old contents now
}


int CSVDialog::validateColumn(const int& col, QString& type)
{
  if ((!m_wiz->m_pageBanking->m_bankingPageInitialized) || (m_fileType != "Banking")) {
    return KMessageBox::Ok;
  }
  if (m_columnsNotSet) {  //  Don't check columns until they've been selected.
    return KMessageBox::Ok;
  }
  //  First check if selection is in range
  if ((col < 0) || (col >= m_endColumn)) {
    return KMessageBox::No;
  }
  //  selection is in range
  if (m_columnTypeList[col] == type) {//  already selected
    return KMessageBox::Ok;
  }
  if (m_columnTypeList[col].isEmpty()) {  //  is this type already in use
    for (int i = 0; i < m_endColumn; i++) {
      //  check each column
      if (m_columnTypeList[i] == type) {  //  this type already in use
        m_columnTypeList[i].clear();//        ...so clear it
      }//  end this col
    }// end all columns checked                type not in use
    m_columnTypeList[col] = type;//            accept new type
    if (m_previousColumn != -1) {
      m_previousColumn = col;
    }
    m_previousType = type;
    return KMessageBox::Ok; //                 accept new type
  }
  if ((m_columnTypeList[col] == "memo")  && (type == "payee") && (m_wiz->m_pageBanking->isVisible())) {
    int rc = KMessageBox::questionYesNo(nullptr, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                        "<center>If you wish to copy the Memo data to the Payee field, click 'Yes'.</center>",
                                        m_columnTypeList[col]));
    if (rc == KMessageBox::Yes) {
      m_memoColCopied = true;
      m_memoColCopy = col;
      m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
      m_payeeColumn = col;
      m_columnTypeList[col] = type;
      m_columnTypeList << "memo";  //  the payeecolumn copy goes here

      if (m_columnList.count() < m_columnTypeList.count()) {
        m_columnList << "";
        m_maxColumnCount ++;
        m_endColumn ++;
      }
      m_memoColumn = m_endColumn;
      m_payeeSelected = true;
      m_columnCountList << m_maxColumnCount + 1;
      return rc;
    }
  }
  //  BUT column is already in use
  if (m_wiz->m_pageBanking->isVisible()) {
    KMessageBox::information(nullptr, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                     "<center>Please reselect both entries as necessary.</center>", m_columnTypeList[col]));
    if (m_columnTypeList[col] == "memo") {  //  If memo col has now been cleared, remove it from m_columnTypeList too
      m_memoColList.removeOne(col);
    }
    m_previousColumn = -1;
    m_wiz->resetComboBox(m_columnTypeList[col], col);
    m_wiz->resetComboBox(type, col);
    m_previousType.clear();
    m_columnTypeList[col].clear();

    for (int i = 0; i < m_maxColumnCount; i++) {
      if (!m_columnTypeList[i].isEmpty()) {
        if (m_columnTypeList[i] == type) {
          m_columnTypeList[i].clear();
        }
      }
    }
  }
  return KMessageBox::Cancel;
}

void CSVDialog::closeEvent(QCloseEvent *event)
{
  m_plugin->m_action->setEnabled(true);
  m_closing = true;
  event->accept();
}

bool CSVDialog::eventFilter(QObject *object, QEvent *event)
{
  // prevent the wizard from closing on escape leaving the importer empty
  if (object == m_wiz && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      close();
    }
    return true;
  } else {
    //  raise wizard above table window
    if (event->type() == QEvent::ContextMenu) {
      Qt::WindowFlags eFlags = windowFlags();
      eFlags |= Qt::WindowStaysOnTopHint;
      m_wiz->setWindowFlags(eFlags);
      m_wiz->show();
      eFlags &= ~Qt::WindowStaysOnTopHint;
      m_wiz->setWindowFlags(eFlags);
      m_wiz->show();
      return true;
    }
    return false;
  }
}

QString CSVDialog::columnType(int column)
{
  if (column >= m_columnTypeList.count()) {
    return QString();
  }
  return  m_columnTypeList[column];
}

void CSVDialog::clearPreviousColumn()
{
  m_previousType.clear();
}

void CSVDialog::setPreviousColumn(int val)
{
  m_previousColumn = val;
}

void CSVDialog::updateDecimalSymbol(const QString& type, int col)
{
  QString txt;
  bool symbolFound = false;
  bool invalidResult = false;
  int startLine;
  int endLine;

  if (m_fileType == "Banking") {
    startLine = m_startLine;
    endLine = m_endLine;
  } else {
    startLine = m_investProcessing->m_startLine;
    endLine = m_investProcessing->m_endLine;
  }
  //  Clear background

  for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
    if (ui->tableWidget->item(row, col) != 0) {
      ui->tableWidget->item(row, col)->setBackground(m_clearBrush);
      ui->tableWidget->item(row, col)->setForeground(m_clearBrushText);
    }
  }

  int errorRow = 0;
  if (type == "amount" || type == "credit" || type == "debit" || type == "price" || type == "quantity") {

    //  Set first and last rows

    m_parse->setSymbolFound(false);

    QString newTxt;
    QTableWidgetItem* errorItem(0);
    //  Check if this col contains empty cells
    for (int row = startLine - 1; row < ui->tableWidget->rowCount(); row++) {
      if (row > endLine - 1) {
        break;
      }
      if (ui->tableWidget->item(row, col) == 0) {      //       empty cell
        if (importNow()) {
          //                                                    if importing, this is error
          KMessageBox::sorry(this, i18n("Row number %1 may be a header line, as it has an incomplete set of entries."
                                        "<center>It may be that the start line is incorrectly set.</center>",
                                        row + 1), i18n("CSV import"));
          return;
        }
        //                                                      if not importing, query

        int ret = KMessageBox::warningContinueCancel(this, i18n("<center>The cell in column '%1' on row %2 is empty.</center>"
                  "<center>Please check your selections.</center><center>Continue or Cancel?</center>",
                  col + 1, row + 1), i18n("Selections Warning"), KStandardGuiItem::cont(),
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
        ui->tableWidget->item(row, col)->setForeground(m_colorBrushText);
        if (m_parse->invalidConversion()) {
          invalidResult = true;
          errorItem = ui->tableWidget->item(row, col);
          errorItem->setBackground(m_errorBrush);
          errorItem->setForeground(m_errorBrushText);
          ui->tableWidget->scrollToItem(errorItem, QAbstractItemView::EnsureVisible);
          if (errorRow == 0) {
            errorRow = row;
          }
        }
        if (m_wiz->m_pageIntro->isVisible() || m_wiz->m_pageLinesDate->isVisible()) {
          ui->tableWidget->horizontalScrollBar()->setValue(col);  //                   ensure col visible
        }
        if (m_parse->symbolFound()) {
          symbolFound = true;
        }
        if (newTxt == txt) {      //                                                 no matching symbol found
          continue;
        }
      }
      if (!symbolFound) {
        errorItem = ui->tableWidget->item(row, col);
        errorItem->setBackground(m_errorBrush);
        errorItem->setForeground(m_errorBrushText);
      }
    }//  last row

    if (!symbolFound && !m_wiz->m_pageIntro->ui->checkBoxSkipSetup->isChecked() && !m_errorFoundAlready) {  //  no symbol found
      ui->tableWidget->horizontalScrollBar()->setValue(col);  //                     ensure col visible
      KMessageBox::sorry(this, i18n("<center>The selected decimal symbol was not present in column %1,</center>"
                                    "<center>- but may now have been added.</center>"
                                    "<center>If the <b>decimal</b> symbol displayed does not match your system setting</center>"
                                    "<center>your data is unlikely to import correctly.</center>"
                                    "<center>Please check your selection.</center>",
                                    col + 1), i18n("CSV import"));
      m_errorColumn = col + 1;
      m_errorFoundAlready = true;
    }

    if (invalidResult && !m_errorFoundAlready) {
      ui->tableWidget->verticalScrollBar()->setValue(errorRow - 1);  //              ensure row visible
      KMessageBox::sorry(this, i18n("<center>The selected decimal symbol ('%1') was not present</center>"
                                    "<center>or has produced invalid results in row %2, and possibly more.</center>"
                                    "<center>Please try again.</center>", decimalSymbol(), errorRow + 1), i18n("Invalid Conversion"));
      m_importError = true;
      m_importNow = false;
      m_wiz->m_wizard->button(QWizard::NextButton)->hide();
      m_wiz->m_wizard->button(QWizard::CustomButton1)->hide();
      return;
    } else {  //  allow user to change setting and try again
      m_importError = false;
      m_importNow = true;
      m_errorFoundAlready = true;
    }
  }
}

void CSVDialog::dateFormatSelected(int dF)
{
  if (dF == -1 || m_fileType == "Invest") {
    return;
  }
  m_dateFormatIndex = dF;
  m_date = m_dateFormats[m_dateFormatIndex];
  if (m_importError) {
    readFile(m_inFileName);
    markUnwantedRows();
  }
}

void CSVDialog::decimalSymbolSelected(int index)
{
  int startLine = 0;
  int endLine = 0;
  //  If a file has been selected but the transaction is then cancelled, decimal symbol
  //  setting was checked prematurely so...
  if ((!m_wiz->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) && (!m_wiz->m_pageLinesDate->m_isColumnSelectionComplete)) {
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
    KMessageBox::sorry(nullptr, i18n("<center>The start line is greater than the end line.\n</center>"
                               "<center>Please correct your settings.</center>"), i18n("CSV import"));
    m_importError = true;
    m_wiz->m_pageIntro->ui->checkBoxSkipSetup->setChecked(false);
    return;
  }
  markUnwantedRows();

  //  Save new decimal symbol and thousands separator

  m_decimalSymbolIndex = index;
  m_parse->setDecimalSymbolIndex(index);
  m_decimalSymbol = m_parse->decimalSymbol(index);
  m_wiz->m_pageCompletion->ui->comboBox_thousandsDelimiter->setCurrentIndex(index);
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
      }
      if (m_errorColumn == -1) {
        m_errorColumn = m_investProcessing->amountColumn();
      }
      ui->tableWidget->horizontalScrollBar()->setValue(m_errorColumn);  //                     ensure col visible
    }
  }
  if (!m_wiz->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    emit isImportable();
  }
}

void CSVDialog::decimalSymbolSelected()
{
  decimalSymbolSelected(m_parse->decimalSymbolIndex());
}

QString CSVDialog::decimalSymbol()
{
  return m_decimalSymbol;
}

int CSVDialog::decimalSymbolIndex()
{
  return m_decimalSymbolIndex;
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

  if (m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex() == -1) {
    return;
  }
  m_wiz->m_pageBanking->m_bankingPageInitialized  = false;
  m_wiz->m_pageInvestment->m_investPageInitialized  = false;
  m_lastDelimiterIndex = m_fieldDelimiterIndex;
}

void CSVDialog::delimiterActivated()
{
  if (m_fileType != "Banking") {
    return;
  }

  if (m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex() == -1) {
    return;
  }
  m_wiz->m_pageBanking->m_bankingPageInitialized  = false;
  m_wiz->m_pageInvestment->m_investPageInitialized  = false;
  //
  //  Ignore any attempted delimiter change by user
  //  as is now under program control
  //
  int newIndex = m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex();
  m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(m_lastDelimiterIndex);

  if (newIndex == m_possibleDelimiter) {
    m_delimiterError = false;
  }
}

void CSVDialog::startLineChanged(int val)
{
  if (m_fileType != "Banking") {
    return;
  }
  if (val > m_fileEndLine) {
    m_wiz->m_pageLinesDate->ui->spinBox_skip->setValue(m_fileEndLine);
  }
  if (val > m_endLine) {
    m_wiz->m_pageLinesDate->ui->spinBox_skip->setValue(m_endLine);
    return;
  }
  m_startLine = val;
  m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setMinimum(m_startLine);//  need to update UI
  if (!m_inFileName.isEmpty()) {
    m_vScrollBar->setValue(m_startLine - 1);
    markUnwantedRows();
  }
}

int CSVDialog::startLine() const
{
  return m_startLine;
}

void CSVDialog::setStartLine(int val)
{
  m_startLine = val;
}

void CSVDialog::endLineChanged(int val)
{
  if (m_fileType != "Banking") {
    return;
  }
  int tmp = m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value();
  if (tmp > m_fileEndLine) {
    m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setValue(m_fileEndLine);
    return;
  }
  if (tmp < m_startLine) {
    return;
  }
  ui->tableWidget->verticalScrollBar()->setValue(val - m_visibleRows);
  m_wiz->m_pageLinesDate->m_trailerLines = m_fileEndLine - val;
  m_endLine = val;
  if (!m_inFileName.isEmpty()) {
    markUnwantedRows();
    int strt = val - m_visibleRows;
    if (strt < 0) {  //  start line too low
      strt = 0;
    }
  }
}

int CSVDialog::lastLine() const
{
  return m_endLine;
}

int CSVDialog::fileLastLine() const
{
  return m_fileEndLine;
}

int CSVDialog::maxColumnCount() const
{
  return m_maxColumnCount;
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
        ui->tableWidget->item(row, col)->setForeground(m_clearBrushText);
      }
    }
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
  QString str = m_wiz->ui->label_intro->text();
  m_wiz->ui->label_intro->setText(QLatin1String("<b>") + str + QLatin1String("</b>"));
}

void CSVDialog::slotNamesEdited()
{
  int row = 0;
  int symTableRow = -1;

  for (row = m_investProcessing->m_startLine - 1; row < m_investProcessing->m_endLine; row ++) {
    if (ui->tableWidget->item(row, m_investProcessing->symbolColumn()) == 0) {  //  Item does not exist
      continue;
    }
    symTableRow++;
    if (ui->tableWidget->item(row, m_investProcessing->symbolColumn())->text().trimmed().isEmpty()) {
      continue;
    }
    //  Replace detail with edited security name.
    QString securityName = m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 2)->text();
    ui->tableWidget->item(row, m_investProcessing->detailColumn())->setText(securityName);
    //  Replace symbol with edited symbol.
    QString securitySymbol = m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 0)->text();
    ui->tableWidget->item(row, m_investProcessing->symbolColumn())->setText(securitySymbol);
    m_investProcessing->m_map.insert(securitySymbol, securityName);
  }

  emit isImportable();
}

void CSVDialog::slotBackButtonClicked()
{
  m_goBack = true;
}

int CSVDialog::amountColumn() const
{
  return m_amountColumn;
}

void CSVDialog::setAmountColumn(int val)
{
  m_amountColumn = val;
}

int CSVDialog::debitColumn() const
{
  return m_debitColumn;
}

void CSVDialog::setDebitColumn(int val)
{
  m_debitColumn = val;
}

int CSVDialog::creditColumn() const
{
  return m_creditColumn;
}

void CSVDialog::setCreditColumn(int val)
{
  m_creditColumn = val;
}

int CSVDialog::dateColumn() const
{
  return m_dateColumn;
}

void CSVDialog::setDateColumn(int val)
{
  m_dateColumn = val;
}

int CSVDialog::payeeColumn() const
{
  return m_payeeColumn;
}

void CSVDialog::setPayeeColumn(int val)
{
  m_payeeColumn = val;
}

int CSVDialog::numberColumn() const
{
  return m_numberColumn;
}

void CSVDialog::setNumberColumn(int val)
{
  m_numberColumn = val;
}

int CSVDialog::memoColumn() const
{
  return m_memoColumn;
}

void CSVDialog::setMemoColumn(int val)
{
  m_memoColumn = val;
}

int CSVDialog::categoryColumn() const
{
  return m_categoryColumn;
}

void CSVDialog::setCategoryColumn(int val)
{
  m_categoryColumn = val;
}

void CSVDialog::slotVertScrollBarMoved(int val)
{
  int top = val;
  int bottom = val + m_visibleRows - 1;
  if (m_fileType == "Banking") {
    if (m_fileEndLine == 0) {  // file not read yet
      return;
    }
    if (bottom > m_fileEndLine) {
      bottom = m_fileEndLine;
    }
    updateColumnWidths(top, bottom);
    setWindowSize(top, bottom);
  } else {
    if (m_investProcessing->m_fileEndLine == 0) {  // file not read yet
      return;
    }
    if (bottom > m_investProcessing->m_fileEndLine) {
      bottom = m_investProcessing->m_fileEndLine;
    }
    m_investProcessing->updateColumnWidths(top, bottom);
    m_investProcessing->setWindowSize(top, bottom);
  }
}

void CSVDialog::clearCellsBackground()
{
  //
  //  Clear cells background.
  //
  for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
    for (int col = 0; col < ui->tableWidget->columnCount(); col ++) {
      if (ui->tableWidget->item(row, col) != 0) {
        ui->tableWidget->item(row, col)->setBackground(m_clearBrush);
        ui->tableWidget->item(row, col)->setForeground(m_clearBrushText);
      }
    }
  }
}

void CSVDialog::clearColumnTypeList()
{
  m_columnTypeList.clear();
}

void CSVDialog::resizeEvent(QResizeEvent* ev)
{
  QRect rect = ui->frame_main->frameRect();
  if (m_fileType.isEmpty() || m_resizing || ev->spontaneous()) {
    ev->ignore();
    return;
  }
  QMargins margn = ui->verticalLayout->layout()->contentsMargins();
  int height = ev->size().height() - m_hScrollBarHeight - m_header - 2 * margn.top() + 4;

  m_visibleRows = (height - m_hScrollBarHeight - m_header  + 14) / m_rowHeight;
  height = m_visibleRows * m_rowHeight + m_hScrollBarHeight + m_header + 2 * (margn.top() + m_dpiDiff);

  int top = ui->tableWidget->rowAt(ui->tableWidget->geometry().top() + m_rowHeight / 2);
  int bottom = top + m_visibleRows - 1;
  if (bottom < 0) {
    bottom = top + m_visibleRows;
  }
  m_vScrollBarVisible = false;
  m_resizing = true;

  rect.setHeight(height + 4);

  QMargins vLayoutMargin = ui->verticalLayout->layout()->contentsMargins();
  int widt = ev->size().width() - vLayoutMargin.left() - vLayoutMargin.right() - 2;
  rect.setWidth(widt);
  ui->frame_main->setFrameRect(rect);
  ev->accept();
  m_resizing = false;
}

