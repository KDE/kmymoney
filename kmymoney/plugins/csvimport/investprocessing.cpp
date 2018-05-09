/*******************************************************************************
*                              investprocessing.cpp
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

#include "investprocessing.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QtGui/QScrollBar>
#include <QtGui/QDesktopWidget>
#include <QtGui/QCloseEvent>
#include <QtGui/QLineEdit>

#include <QtCore/QFile>
#include <QFileDialog>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
// ----------------------------------------------------------------------------
// KDE Headers

#include <kdeversion.h>
#include <KFileDialog>
#include <KFileWidget>
#include <KInputDialog>
#include <KSharedConfig>
#include <kmessagebox.h>
#include <KStandardDirs>
#include <KLocale>
#include <KIO/NetAccess>
#include <KAboutData>
#include <KAboutApplicationDialog>
#include <kvbox.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyfile.h"
#include "kmymoney.h"

#include "investmentdlg.h"
#include "csvwizard.h"
#include "convdate.h"
#include "csvdialog.h"

#include "mymoneystatement.h"
#include "mymoneystatementreader.h"
#include "mymoneymoney.h"
#include "redefinedlg.h"
#include "csvutil.h"

#include "ui_csvdialog.h"
#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_lines-datewizardpage.h"
#include "ui_completionwizardpage.h"
#include "ui_investmentwizardpage.h"
#include "ui_csvwizard.h"
#include "symboltabledlg.h"

InvestProcessing::InvestProcessing()
{
  m_amountSelected = false;
  m_brokerage = false;
  m_brokerageItems = false;
  m_importNow = false;
  m_dateSelected = false;
  m_feeSelected = false;
  m_memoSelected = false;
  m_priceSelected = false;
  m_quantitySelected = false;
  m_typeSelected = false;
  m_symbolSelected = false;
  m_detailSelected = false;
  m_firstPass = true;

  m_dateFormatIndex = 0;
  m_fieldDelimiterIndex = 0;
  m_maxColumnCount = 0;
  m_payeeColumn = -1;
  m_amountColumn = -1;
  m_dateColumn = -1;
  m_feeColumn = -1;
  m_memoColumn = -1;
  m_priceColumn = -1;
  m_quantityColumn = -1;
  m_typeColumn = -1;
  m_symbolColumn = -1;
  m_detailColumn = -1;
  m_endLine = 0;
  m_fileEndLine = 0;
  m_startLine = 1;
  m_topLine = 0;
  m_row = 0;
  m_endColumn = 0;
  m_completer = 0;

  m_inFileName.clear();
  m_securityName.clear();

  csvSplit m_csvSplit;

  m_convertDat = new ConvertDate;
  m_parse = new Parse;
  m_redefine = new RedefineDlg;
  m_csvUtil = new CsvUtil;

  connect(m_redefine, SIGNAL(changedType(QString)), this, SLOT(changedType(QString)));
}

InvestProcessing::~InvestProcessing()
{
  delete m_parse;
  delete m_convertDat;
  delete m_completer;
  delete m_redefine;
  delete m_csvUtil;
}

void InvestProcessing::init()
{
  m_dateFormats << "yyyy/MM/dd" << "MM/dd/yyyy" << "dd/MM/yyyy";
  m_brokerBuff.clear();
  m_endColumn = 0;
  m_accountName.clear();

  clearSelectedFlags();

  m_securityName = m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();

  QLineEdit* securityLineEdit = m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->lineEdit();//krazy:exclude=<qclasses>

  m_completer = new QCompleter(m_securityList, this);
  m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  securityLineEdit->setCompleter(m_completer);
  connect(securityLineEdit, SIGNAL(editingFinished()), this, SLOT(securityNameEdited()));
  connect(this, SIGNAL(statementReady(MyMoneyStatement&)), m_csvDialog->m_plugin, SLOT(slotGetStatement(MyMoneyStatement&)));

  m_dateFormatIndex = m_csvDialog->m_wiz->m_pageLinesDate->ui->comboBox_dateFormat->currentIndex();
  m_convertDat->setDateFormatIndex(m_dateFormatIndex);
  m_dateFormat = m_dateFormats[m_dateFormatIndex];
  //  The following string list strings are descriptions of possible investment
  //  activity types.  Each of the lists may also contain alternative descriptions,
  //  added by the user to the resource file, to suit his needs.

  QString text = "Type of operation as in financial statement";
  m_buyList += i18nc("%1", "buy", "%1", text);  //                       some basic entries in case rc file missing
  m_sellList << i18nc("%1", "sell", text)
             << i18nc("%1", "repurchase", text);
  m_divXList += i18nc("%1", "dividend", text);
  m_intIncList << i18nc("%1", "interest", text)
               << i18nc("%1", "income", text);
  m_reinvdivList << i18nc("%1", "reinvest", text)
                 << i18nc("%1", "reinv", text)
                 << i18nc("%1", "re-inv", text);
  m_shrsinList << i18nc("%1", "add", text)
               << i18nc("%1", "stock dividend", text)
               << i18nc("%1", "divd reinv", text)
               << i18nc("%1", "transfer in", text)
               << i18nc("%1", "re-registration in", text)
               << i18nc("%1", "journal entry", text);
  m_removeList += i18nc("%1", "remove", text);
  text = "Brokerage type";
  m_brokerageList << i18nc("%1", "check", text)
                  << i18nc("%1", "payment", text)
                  << i18nc("%1", "bill payment", text)
                  << i18nc("%1", "dividend", text)
                  << i18nc("%1", "interest", text)
                  << i18nc("%1", "qualified div", text)
                  << i18nc("%1", "foreign tax paid", text)
                  << i18nc("%1", "adr mgmt fee", text);

  findCodecs();//                             returns m_codecs = codecMap.values();
}

void InvestProcessing::changedType(const QString& newType)
{
  if ((newType == "buy") || (newType == "sell") || (newType == "divx") || (newType == "reinvdiv") ||
      (newType == "shrsin") || (newType == "shrsout")  || (newType == "intinc")) {
    m_trInvestData.type = newType;
  }
}

void InvestProcessing::slotFileDialogClicked()
{
  if ((m_csvDialog->m_fileType != "Invest") || (m_csvDialog->m_profileName.isEmpty())) {
    return;
  }
  m_columnTypeList.clear();//  Needs to be here in case user selects new profile after cancelling prior one.clearColumnTypes()
  //  remove all column widths left-over from previous file
  //  which can screw up row width calculation.
  for (int i = 0; i < m_csvDialog->ui->tableWidget->columnCount(); i++) {
    m_csvDialog->ui->tableWidget->setColumnWidth(i, 0);
  }

  m_inFileName.clear();
  m_url.clear();
  m_csvDialog->m_wiz->m_pageLinesDate->m_isColumnSelectionComplete = false;
  m_firstPass = true;
  m_firstRead = true;
  m_memoColCopied = false;
  m_typeColCopied = false;
  m_detailColCopied = false;
  m_csvDialog->m_wiz->m_pageInvestment->m_investPageInitialized = false;
  m_csvDialog->m_columnsNotSet = true;  //  Don't check columns until they've been selected.
  m_csvDialog->m_separatorPageVisible = false;
  m_symbolTableScanned = false;
  m_listSecurities.clear();
  m_csvDialog->m_delimiterError = false;
  m_importCompleted = false;
  m_csvDialog->m_accept = false;
  m_initWindow = true;

  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locate("config", "csvimporterrc"));
  bool found = false;
  QString profileName;
  for (int i = 0; i < m_csvDialog->m_profileList.count(); i++) {
    if (m_csvDialog->m_profileList[i] != m_csvDialog->m_profileName) {
      continue;
    } else {
      found = true;
      profileName = "Profiles-" + m_csvDialog->m_profileList[i];
    }
  }
  if (!found) {
    return;
  }
  KConfigGroup profilesGroup(config, profileName);
  m_invPath = profilesGroup.readEntry("InvDirectory", QString());
  m_encodeIndex = profilesGroup.readEntry("Encoding", 0);
  m_csvDialog->m_wiz->m_pageLinesDate->m_trailerLines = profilesGroup.readEntry("TrailerLines", 0);
  m_fileEndLine = 0;
  m_endLine = 0;
  m_startLine = 1;
  m_topLine = 0;
  m_maxRowWidth = 0;
  m_maxColumnCount = 0;
  m_csvDialog->m_lastDelimiterIndex = 0;
  m_csvDialog->m_possibleDelimiter = -1;
  m_csvDialog->m_errorColumn = -1;
  int position = 0;

  if (m_invPath.isEmpty()) {
    m_invPath  = "~/";
  }

  QPointer<KFileDialog> dialog = new KFileDialog(KUrl(m_invPath),
      i18n("*.csv *.PRN *.txt|CSV Files\n*|All files"), 0);

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
  m_csvDialog->m_inFileName.clear();

  if (!KIO::NetAccess::download(m_url,  m_csvDialog->m_inFileName, 0)) {
    KMessageBox::detailedError(0, i18n("Error while loading file '%1'.", m_url.prettyUrl()),
                               KIO::NetAccess::lastErrorString(),
                               i18n("File access error"));
    return;
  }
  if (m_csvDialog->m_inFileName.isEmpty())
    return;
  clearComboBoxText();//                        To clear any '*' in memo combo text
  m_importNow = false;//                        Avoid attempting date formatting on headers
  m_csvDialog->m_acceptAllInvalid = false;  //  Don't accept further invalid values.

  for (int i = 0; i < m_columnTypeList.count(); i++)
    if (m_columnTypeList[i] == "memo") {
      m_columnTypeList[i].clear();   //    ensure no memo entries remain
    }
  //
  //  This line seems to be needed when selecting a new file
  //  after previously selecting one.  Reset later.
  //
  disconnect(m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  m_csvDialog->ui->tableWidget->horizontalScrollBar()->setSliderPosition(0);

  KConfigGroup mainGroup(config, "MainWindow");
  m_csvDialog->m_pluginHeight = mainGroup.readEntry("Height", 640);
  m_csvDialog->m_pluginWidth = mainGroup.readEntry("Width", 800);

  readFile(m_csvDialog->m_inFileName);
  m_invPath  =  m_csvDialog->m_inFileName;
  position = m_invPath.lastIndexOf("/");
  m_invPath.truncate(position + 1);

  readSettings();
  QString txt = "Profiles-" + m_csvDialog->m_profileName;
  KConfigGroup profileGroup(config, txt);
  QString str = "~/" + m_invPath.section('/', 3);
  profileGroup.writeEntry("InvDirectory", str);  //                      save selected path
  profileGroup.writeEntry("Encoding", m_encodeIndex);  //                ..and encoding
  profileGroup.writeEntry("FileType", m_csvDialog->m_fileType);  //      ..and fileType
  profileGroup.config()->sync();

  enableInputs();

  if (m_csvDialog->m_wiz->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    m_csvDialog->m_wiz->m_pageCompletion->initializePage();//  Using a profile so skip setup and go to Completion.
    m_csvDialog->m_wiz->m_pageSeparator->initializePage();
  } else {
    m_csvDialog->m_wiz->m_wizard->next();
    if (m_csvDialog->m_possibleDelimiter == -1) {
      m_csvDialog->m_delimiterError = true;
      m_csvDialog->m_possibleDelimiter = m_fieldDelimiterIndex;
    }
  }
}

void InvestProcessing::enableInputs()
{
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setEnabled(true);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setEnabled(true);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setEnabled(true);
  m_csvDialog->m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->setEnabled(true);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setEnabled(true);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setEnabled(true);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceFraction->setEnabled(true);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setEnabled(true);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setEnabled(true);
  m_csvDialog->m_wiz->m_pageInvestment->ui->button_clear->setEnabled(true);
  m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setEnabled(true);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setEnabled(true);
  m_csvDialog->m_wiz->m_pageInvestment->ui->checkBoxInv_feeType->setEnabled(true);
}

void InvestProcessing::clearColumnsSelected()
{
  clearSelectedFlags();
  clearColumnNumbers();
  clearComboBoxText();

  m_memoColCopied = false;
  m_typeColCopied = false;
  m_detailColCopied = false;
  m_memoColList.clear();
}

void InvestProcessing::clearSelectedFlags()
{
  for (int i = 0; i < m_columnTypeList.count(); i++) {
    m_columnTypeList[i].clear();//               set to all empty
  }

  m_amountSelected = false;
  m_dateSelected = false;
  m_priceSelected = false;
  m_quantitySelected = false;
  m_memoSelected = false;
  m_typeSelected = false;
  m_feeSelected = false;
  m_detailSelected = false;
  m_symbolSelected = false;
}

void InvestProcessing::clearColumnNumbers()
{
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
}

void InvestProcessing::clearComboBoxText()
{
  for (int i = 0; i < m_maxColumnCount; i++) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(i, QString().setNum(i + 1));
  }
}

void InvestProcessing::encodingChanged(int index)
{
  m_encodeIndex = index;
  if (!m_inFileName.isEmpty())
    readFile(m_inFileName);
}

void InvestProcessing::dateColumnSelected(int col)
{
  QString type = "date";
  m_dateColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_dateColumn] == type)  && (m_dateColumn != col)) {
    m_columnTypeList[m_dateColumn].clear();
  }
  int ret = validateNewColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(col);  // accept new column
    m_dateSelected = true;
    if (m_dateColumn != -1) {
//          if a previous date column is detected, but in a different column...
      if ((m_columnTypeList[m_dateColumn] == type)  && (m_dateColumn != col)) {
        m_columnTypeList[m_dateColumn].clear();//   clear it
      }
    }
    m_dateColumn = col;
    m_columnTypeList[m_dateColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::findCodecs()
{
  QMap<QString, QTextCodec *> codecMap;
  QRegExp iso8859RegExp("ISO[- ]8859-([0-9]+).*");

  foreach (int mib, QTextCodec::availableMibs()) {
    QTextCodec *codec = QTextCodec::codecForMib(mib);

    QString sortKey = codec->name().toUpper();
    int rank;

    if (sortKey.startsWith("UTF-8")) {       // krazy:exclude=strings
      rank = 1;
    } else if (sortKey.startsWith("UTF-16")) {       // krazy:exclude=strings
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

int InvestProcessing::validateNewColumn(const int& col, const QString& type)
{
  //  Prevent check of column settings until user sees them.
  //  Then check if selection is in range
  if ((col < 0) || (col >= m_endColumn) || (m_csvDialog->m_columnsNotSet)) {
    return KMessageBox::No;
  }
  //  selection was in range
  //  ...but does it clash?
  if ((!m_columnTypeList[col].isEmpty())  && (m_columnTypeList[col] != type) && (m_csvDialog->m_wiz->m_pageInvestment->m_investPageInitialized)) {  // column is already in use
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>", m_columnTypeList[col]));
    m_previousColumn = -1;
    resetComboBox(m_columnTypeList[col], col);  //      clash,  so reset ..
    resetComboBox(type, col);  //                   ... both comboboxes
    m_previousType.clear();
    m_columnTypeList[col].clear();
    return KMessageBox::Cancel;
  }
  //                                                is this type already in use
  for (int i = 0; i < m_maxColumnCount; i++) {  //  check each column
    if (m_columnTypeList[i] == type) {  //          this type already in use
      m_columnTypeList[i].clear();//                ...so clear it
    }//  end this col

  }// end all columns checked                       type not in use
  m_columnTypeList[col] = type;  //                 accept new type
  if (m_previousColumn != -1) {
    m_previousColumn = col;
  }
  m_previousType = type;
  return KMessageBox::Ok; //                        accept new type
}

void InvestProcessing::feeColumnSelected(int col)
{
  QString type = "fee";
  m_feeColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_feeColumn] == type)  && (m_feeColumn != col)) {
    m_columnTypeList[m_feeColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(col);  // accept new column
    m_feeSelected = true;
    if (m_feeColumn != -1) {
//          if a previous fee column is detected, but in a different column...
      if ((m_columnTypeList[m_feeColumn] == type)  && (m_feeColumn != col)) {
        m_columnTypeList[m_feeColumn].clear();//    ..clear it
      }
    }
    m_feeColumn = col;
    m_columnTypeList[m_feeColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::typeColumnSelected(int col)
{
  QString type = "type";
  m_typeColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_typeColumn] == type)  && (m_typeColumn != col)) {
    m_columnTypeList[m_typeColumn].clear();
  }
  int ret = validateNewColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(col);  // accept new column
    m_typeSelected = true;
    if (m_typeColumn != -1) {
//          if a previous type column is detected, but in a different column...
      if ((m_columnTypeList[m_typeColumn] == type)  && (m_typeColumn != col)) {
        m_columnTypeList[m_typeColumn].clear();//   ...clear it
      }
    }
    m_typeColumn = col;
    m_columnTypeList[m_typeColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::memoColumnSelected(int col)
{
  //  Prevent check of column settings until user sees them.
  if ((col < 0) || (col >= m_endColumn) || (m_csvDialog->m_columnsNotSet)) {      //  out of range so...
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);  //  ..clear selection
    return;
  }
  QString type = "memo";
  m_memoColumn = col;

  if (m_columnTypeList[col].isEmpty()) {      //      accept new  entry
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    m_columnTypeList[col] = type;
    m_memoColumn = col;
    if (m_memoColList.contains(col)) {
      //  Restore the '*' as column might have been cleared.
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    } else {
      m_memoColList << col;
    }
    m_memoSelected = true;
    return;
  } else if (m_columnTypeList[col] == type) {  //     nothing changed
    return;
  }
  if ((m_columnTypeList[col] == "type") || (m_columnTypeList[col] == "detail")) {
    if (m_memoColList.contains(col)) {
      m_memoColList.removeOne(col);
    }
    int rc = KMessageBox::Yes;
    if (m_csvDialog->m_wiz->m_pageInvestment->isVisible()) {
      rc = KMessageBox::questionYesNo(0, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                              "<center>If you wish to copy that data to the memo field, click 'Yes'.</center>",
                                              m_columnTypeList[col]));
    }
    if (rc == KMessageBox::Yes) {
      if (m_columnTypeList[col] == "type") {
        m_typeColCopied = true;
      } else if (m_columnTypeList[col] == "detail") {
        m_detailColCopied = true;
      }
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
      m_endColumn += 1;
      m_memoColumn = m_endColumn - 1;
      m_memoColList << col;
      m_columnTypeList << "memo";  //  need one extra for type/detail column copy
      m_memoSelected = true;
      return;
    }
  } else {  //  m_columnTypeList[col] != "type"or "detail"
    //                                           clashes with prior selection
    m_memoSelected = false;//                    clear incorrect selection
    m_typeColCopied = false;
    m_detailColCopied = false;
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>", m_columnTypeList[col]));
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
    m_previousColumn = -1;
    resetComboBox(m_columnTypeList[col], col);  //      clash,  so reset ..
    resetComboBox(type, col);  //                   ... both comboboxes
    m_previousType.clear();
    m_columnTypeList[col].clear();
    if (m_memoColumn >= 0) {
      m_columnTypeList[m_memoColumn].clear();
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(m_memoColumn, QString().setNum(m_memoColumn + 1));   //  reset the '*'
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);  //       and this one
    }
  }
}

void InvestProcessing::quantityColumnSelected(int col)
{
  QString type = "quantity";
  m_quantityColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
  m_redefine->setQuantityColumn(col);
// A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_quantityColumn] == type)  && (m_quantityColumn != col)) {
    m_columnTypeList[m_quantityColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(col);  // accept new column
    m_quantitySelected = true;
    if (m_quantityColumn != -1) {
      //  if a previous quantity column is detected, but in a different column...
      if ((m_columnTypeList[m_quantityColumn] == type)  && (m_quantityColumn != col)) {
        m_columnTypeList[m_quantityColumn].clear();// ...clear it
      }
    }
    m_quantityColumn = col;
    m_columnTypeList[m_quantityColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::priceColumnSelected(int col)
{
  QString type = "price";
  m_priceColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }

  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_priceColumn] == type)  && (m_priceColumn != col)) {
    m_columnTypeList[m_priceColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(col);  // accept new column
    m_priceSelected = true;
    if (m_priceColumn != -1) {
      //          if a previous price column is detected, but in a different column...
      if ((m_columnTypeList[m_priceColumn] == type)  && (m_priceColumn != col)) {
        m_columnTypeList[m_priceColumn].clear();//  ...clear it
      }
    }
    m_priceColumn = col;
    m_columnTypeList[m_priceColumn] = type;
    m_redefine->setPriceColumn(col);
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::amountColumnSelected(int col)
{
  QString type = "amount";
  m_amountColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
  m_redefine->setAmountColumn(col);
  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_amountColumn] == type)  && (m_amountColumn != col)) {
    m_columnTypeList[m_amountColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(col);  // accept new column
    m_amountSelected = true;
    if (m_amountColumn != -1) {
      //          if a previous amount column is detected, but in a different column...
      if ((m_columnTypeList[m_amountColumn] == type)  && (m_amountColumn != col)) {
        m_columnTypeList[m_amountColumn].clear();// ...clear it
      }
    }
    m_amountColumn = col;
    m_columnTypeList[m_amountColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::symbolColumnSelected(int col)
{
  QString type = "symbol";
  m_symbolColumn = col;
  if (col < 0) {
    //  it is not set so remove any prior settings
    int indx = m_columnTypeList.indexOf(type);
    m_symbolSelected = false;
    if (indx > -1) {
      m_columnTypeList[indx].clear();
    }
    return;
  }
  m_redefine->setSymbolColumn(col);
  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_symbolColumn] == type)  && (m_symbolColumn != col)) {
    m_columnTypeList[m_symbolColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(col);  // accept new column
    m_symbolSelected = true;
    if (m_symbolColumn != -1) {
      //          if a previous symbol column is detected, but in a different column...
      if ((m_columnTypeList[m_symbolColumn] == type)  && (m_symbolColumn != col)) {
        m_columnTypeList[m_symbolColumn].clear();// ...clear it
      }
    }
    m_symbolColumn = col;
    m_columnTypeList[m_symbolColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::detailColumnSelected(int col)
{
  QString type = "detail";
  m_detailColumn = col;
  if (col < 0) {
    //  it is not set so remove any prior settings
    int indx = m_columnTypeList.indexOf(type);
    m_detailSelected = false;
    if (indx > -1) {
      m_columnTypeList[indx].clear();
    }
    return;
  }
  m_redefine->setDetailColumn(col);
  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_detailColumn] == type)  && (m_detailColumn != col)) {
    m_columnTypeList[m_detailColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(col);  // accept new column
    m_detailSelected = true;
    if (m_detailColumn != -1) {
      //          if a previous detail column is detected, but in a different column...
      if ((m_columnTypeList[m_detailSelected] == type)  && (m_detailColumn != col)) {
        m_columnTypeList[m_detailColumn].clear();// ...clear it
      }
    }
    m_detailColumn = col;
    m_columnTypeList[m_detailColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::fieldDelimiterChanged()
{
  if ((m_csvDialog->m_fileType != "Invest") || (m_csvDialog->m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex() == -1)) {
    return;
  }
  m_csvDialog->m_wiz->m_pageBanking->m_bankingPageInitialized  = false;
  m_csvDialog->m_wiz->m_pageInvestment->m_investPageInitialized  = false;
  //
  //  Ignore any attempted delimiter change
  //  as is now under program control
  //
  int newIndex = m_csvDialog->m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex();
  m_csvDialog->m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(m_csvDialog->m_possibleDelimiter);
  if (newIndex == m_csvDialog->m_possibleDelimiter) {
    m_csvDialog->m_delimiterError = false;
  }
}

void InvestProcessing::readFile(const QString& fname)
{
  MyMoneyStatement st = MyMoneyStatement();
  MyMoneyStatement stBrokerage = MyMoneyStatement();
  m_csvDialog->m_importError = false;
  m_csvDialog->m_errorFoundAlready = false;
  m_csvDialog->m_rowWidthsDone = false;
  m_initWindow = true;
  m_fieldDelimiterIndex = m_csvDialog->m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex();
  if (m_fieldDelimiterIndex == -1) {
    return;
  }
  m_csvDialog->m_lastDelimiterIndex = m_fieldDelimiterIndex;
  m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_textDelimiterIndex = m_csvDialog->m_wiz->m_pageSeparator->ui->comboBox_textDelimiter->currentIndex();
  m_parse->setTextDelimiterIndex(m_textDelimiterIndex);
  m_textDelimiterCharacter = m_parse->textDelimiterCharacter(m_textDelimiterIndex);

  m_csvDialog->ui->tableWidget->clear();// including vert headers
  m_csvDialog->ui->tableWidget->verticalScrollBar()->setValue(0);
  m_inBuffer.clear();
  m_outBuffer = "!Type:Invst\n";
  m_brokerBuff.clear();
  m_row = 0;
  m_payeeColumn = - 1;
  int columnCount = 0;

  m_accountName.clear();
  m_redefine->clearAccountName();
  m_brokerageItems = false;

  QString name = QDir::homePath();
  QStringList outFile = name.split('.');
  QString outFileName = (outFile.isEmpty() ? "InvestProcessing" : outFile[0]) + ".qif";

  if (!fname.isEmpty())
    m_inFileName  = fname;

  QFile inFile(m_inFileName);
  inFile.open(QIODevice::ReadOnly);  // allow a Carriage return -// QIODevice::Text
  QTextStream inStream(&inFile);
  QTextCodec *codec = QTextCodec::codecForMib(m_codecs.value(m_encodeIndex)->mibEnum());
  inStream.setCodec(codec);

  m_buf = inStream.readAll();
  //
  //  Start parsing the buffer
  //
  m_columnCountList.clear();
  QString data;
  m_lineList = m_parse->parseFile(m_buf, 1, 0);  //                               Changed to display whole file.
  //  Check all lines to find maximum column count.
  //  Also disable column combobox connects till later
  //
  int totalDelimiterCount[4] = {0};  //  Total in file for each delimiter
  int thisDelimiterCount[4] = {0};   //  Total in this line for each delimiter
  int colCount = 0;                  //  Total delimiters in this line
  m_csvDialog->m_possibleDelimiter = 0;
  m_symbolRow = 0;

  m_csvDialog->m_delimiterError = false;
  for (int i = 0; i < m_lineList.count(); i++) {
    data = m_lineList[i];

    for (int count = 0; count < 4; count++) {  //  Four possible delimiters
      //  Count each delimiter to find most likely one to use .
      //  Changed to sum total file, not just individual lines.
      m_parse->setFieldDelimiterIndex(count);
      colCount = m_parse->parseLine(data).count();

      if (colCount > thisDelimiterCount[count]) {
        thisDelimiterCount[count] = colCount;
      }
      if (thisDelimiterCount[count] > m_maxColumnCount) {
        m_maxColumnCount = thisDelimiterCount[count];
      }
      m_columnCountList << colCount;  // Number of columns in each line.
      totalDelimiterCount[count] += colCount;
      if (totalDelimiterCount[count] > totalDelimiterCount[m_csvDialog->m_possibleDelimiter]) {
        m_csvDialog->m_possibleDelimiter = count;
      }
    }
  }
  m_csvDialog->ui->tableWidget->setColumnCount(m_maxColumnCount);
  if ((columnCount < 5) || (m_csvDialog->m_possibleDelimiter != m_fieldDelimiterIndex)) {
    m_csvDialog->m_delimiterError = true;
  }
  if (m_fileEndLine == 0) { // copy from later
    m_fileEndLine = m_parse->lastLine();
  }

  disconnect(m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip, 0, 0, 0);  //  Avoid disruption from start/endline changes.
  disconnect(m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast, 0, 0, 0);  //  Avoid disruption from start/endline changes.

  if (m_firstRead) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->clear();  //  clear all existing items before adding new ones
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->clear();
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->clear();
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->clear();
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->clear();
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->clear();
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->clear();
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->clear();
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol->clear();

    disconnect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColumnSelected(int)));
    disconnect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(typeColumnSelected(int)));
    disconnect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColumnSelected(int)));
    disconnect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), this, SLOT(quantityColumnSelected(int)));
    disconnect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(priceColumnSelected(int)));
    disconnect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(amountColumnSelected(int)));
    disconnect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(feeColumnSelected(int)));
    disconnect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolColumnSelected(int)));
    disconnect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol, SIGNAL(currentIndexChanged(int)), this, SLOT(detailColumnSelected(int)));

    for (int i = 0; i < m_maxColumnCount; i++) {  //  populate comboboxes with col # values
      //  Start to build m_columnTypeList before comboBox stuff below
      //  because that causes connects which access m_columnTypeList
      m_columnTypeList << QString();  //                clear all column types
      QString t;
      t.setNum(i + 1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->addItem(t);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->addItem(t);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->addItem(t);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->addItem(t);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->addItem(t);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->addItem(t);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->addItem(t);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->addItem(t);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol->addItem(t);
    }

    m_firstPass = false;
    m_screenUpdated = false;

    connect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColumnSelected(int)));
    connect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(typeColumnSelected(int)));
    connect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColumnSelected(int)));
    connect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), this, SLOT(quantityColumnSelected(int)));
    connect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(priceColumnSelected(int)));
    connect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(amountColumnSelected(int)));
    connect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(feeColumnSelected(int)));
    connect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolColumnSelected(int)));
    connect(m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol, SIGNAL(currentIndexChanged(int)), this, SLOT(detailColumnSelected(int)));

    //  Display the buffer

    m_rowWidth = 0;
    m_firstRead = false;
    if (m_fileEndLine == 0) {
      m_fileEndLine = m_parse->lastLine();
    }
    m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip->setMaximum(m_fileEndLine);
    m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setMaximum(m_fileEndLine);
    if (m_fileEndLine > m_csvDialog->m_wiz->m_pageLinesDate->m_trailerLines) {
      m_endLine = m_fileEndLine - m_csvDialog->m_wiz->m_pageLinesDate->m_trailerLines;
    } else {
      m_endLine = m_fileEndLine;  //                                 Ignore m_trailerLines as > file length.
    }
  }
  if (m_startLine > m_endLine) {  //                                 Don't allow m_startLine > m_endLine
    m_startLine = m_endLine;
  }

  m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setValue(m_endLine);
  m_csvDialog->ui->tableWidget->setRowCount(m_endLine);
  connect(m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  connect(m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endLineChanged(int)));

  //  Display the buffer
  for (int line = 0; line < m_lineList.count(); line++) {
    m_inBuffer = m_lineList[line];
    displayLine(m_inBuffer);
    if (m_rowWidth > m_maxRowWidth) {
      m_maxRowWidth = m_rowWidth;
    }
    if (m_importNow) {
      m_csvDialog->clearCellsBackground();
    }
    //  user now ready to continue && line is in wanted range
    //
    if ((m_importNow) && (line >= m_startLine - 1) && (line <= m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value() - 1)) {
      reloadUISettings();  //                            Need to reload column settings
      int ret = processInvestLine(m_inBuffer);  //       parse input line
      if (ret == KMessageBox::Ok) {
        if (m_brokerage) {
          investCsvImport(stBrokerage);  //              add non-investment transaction to Brokerage statement
        } else {
          investCsvImport(st);  //                       add investment transaction to statement
        }
      } else {
        m_importNow = false;
        m_csvDialog->m_wiz->m_wizard->back();  //               have another try at the import
      }
    }  //                                                finished parsing
    m_csvDialog->m_wiz->m_pageLinesDate->ui->labelSet_skip->setEnabled(true);
    m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip->setEnabled(true);
    m_endColumn = m_maxColumnCount;
  }// end of buffer

  setWindowSize(-1, -1);

  if ((m_importNow) && (m_csvDialog->m_fileType == "Invest")) {
    if ((!m_inFileName.isEmpty()) && (m_amountColumn >= 0) && (m_quantityColumn >= 0) && (m_priceColumn >= 0)) {
      m_csvDialog->updateDecimalSymbol("amount", m_amountColumn);
      m_csvDialog->updateDecimalSymbol("price", m_priceColumn);
      m_csvDialog->updateDecimalSymbol("quantity", m_quantityColumn);
    } else {
      KMessageBox::sorry(0, i18n("<center>An amount, price, and/or quantity column is missing.</center>Please check your selections."), i18n("CSV import"));
    }

    emit statementReady(st);  //              investment statement ready
    if (m_brokerageItems) {
      emit statementReady(stBrokerage);  //   brokerage statement ready
    }
    m_importNow = false;
    m_importCompleted = true;
  }
  if (!m_symbolTableScanned) {
    m_listSecurities.clear();
  }
  if (m_csvDialog->m_delimiterError) {
    m_fieldDelimiterIndex = m_csvDialog->m_possibleDelimiter;
    m_csvDialog->m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(m_csvDialog->m_possibleDelimiter);
  }
  inFile.close();
  m_csvDialog->m_columnsNotSet = false;  //  Prevent check of column settings until user sees them.
}

void InvestProcessing::setWindowSize(int firstLine, int lastLine)
{
  int screenHeight = QApplication::desktop()->height();
  int launcherHeight = 41;//  allow for horizontal app launch bar - approx
  int variousMarginsEtc = 120;//  all margins, hscrollbar, title, gap between frames, etc.
  int maxLines = (screenHeight - launcherHeight - variousMarginsEtc) / m_csvDialog->m_rowHeight;

  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    m_csvDialog->m_dpiDiff = 0;
  } else {
    m_csvDialog->m_dpiDiff = 5;
  }
  if (m_initWindow) {
    m_csvDialog->m_visibleRows = qMin(m_lineList.count(), maxLines);
    m_initWindow = false;
  }
  m_csvDialog->m_tableHeight = m_csvDialog->m_visibleRows * m_csvDialog->m_rowHeight + m_csvDialog->m_header + m_csvDialog->m_hScrollBarHeight + m_csvDialog->m_dpiDiff;

  if (firstLine == - 1 || lastLine == -1) {
    updateColumnWidths(0, m_lineList.count() - 1);
  } else {
    updateColumnWidths(firstLine, lastLine);
  }

  QRect rect;
  rect = m_csvDialog->ui->frame_main->frameRect();
  m_csvDialog->ui->frame_main->setMinimumHeight(120);

  QMargins hLayout_MainMargin = m_csvDialog->ui->horizontalLayout_Main->layout()->contentsMargins();
  QMargins vLayoutMargin = m_csvDialog->ui->verticalLayout->layout()->contentsMargins();

  m_csvDialog->m_vHeaderWidth = m_csvDialog->ui->tableWidget->verticalHeader()->width();
  if (m_csvDialog->m_visibleRows < 10) {
    m_csvDialog->m_vHeaderWidth = 18;  //  allow space for double-digit row number
  } else {
    m_csvDialog->m_vHeaderWidth = 26;
  }

  if (m_csvDialog->m_visibleRows < m_fileEndLine) {
    //  vert scrollbar is visible
    m_csvDialog->m_vScrollBarWidth = m_csvDialog->ui->tableWidget->verticalScrollBar()->width();
  } else {
    m_csvDialog->m_vScrollBarWidth = 0;
  }
  int scrollbarWidth = 17;  //  scrollbar space for when needed
  int wd = m_rowWidth + m_csvDialog->m_vHeaderWidth +  2 * (vLayoutMargin.left() + 1) + 12 + hLayout_MainMargin.left() + hLayout_MainMargin.right() + scrollbarWidth;
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
  variousMarginsEtc = 58;  //  all margins, hscrollbar, title, gap between frames, etc.
  m_csvDialog->resize(wd , m_csvDialog->m_tableHeight + 4 *(vLayoutMargin.top() + 1) + 8);

  rect.setHeight(m_csvDialog->height() - m_csvDialog->m_hScrollBarHeight - m_csvDialog->m_header - 4 *(vLayoutMargin.top() + 1) + variousMarginsEtc);
  rect.setWidth(m_csvDialog->width() - hLayout_MainMargin.left() - hLayout_MainMargin.right());
  m_csvDialog->ui->frame_main->setFrameRect(rect);
}

void InvestProcessing::displayLine(const QString& data)
{
  QBrush dropBrush;
  QColor dropColor;
  dropColor.setRgb(255, 0, 127, 100);
  dropBrush.setColor(dropColor);
  dropBrush.setStyle(Qt::SolidPattern);
  QFont font(QApplication::font());
  m_csvDialog->ui->tableWidget->setFont(font);
  m_fieldDelimiterIndex = m_csvDialog->m_possibleDelimiter;
  m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  //
  //                 split data into fields
  //
  m_columnList = m_parse->parseLine(data);
  m_redefine->setColumnList(m_columnList);
  int col = 0;

  //  If making copy of detailcol or typecol, check the columns actually exist...
  if ((!m_firstPass) && (m_typeColumn <= m_columnTypeList.count()) &&
      (m_detailColumn <= m_columnTypeList.count()) && (m_memoColumn <= m_columnTypeList.count())) {
    if ((m_typeColCopied) && (m_typeColumn < m_columnList.count()) && (m_typeColumn >= 0)) {  //        ...then make the copy here
      m_columnList << m_columnList[m_typeColumn];
      m_columnTypeList[m_memoColumn] = "memo";
      for (int i = 0; i < m_memoColList.count(); i++) {
        if (m_memoColList[i] == m_typeColumn) {
          continue;
        }
        m_columnTypeList[m_memoColList[i]] = "memo";
      }
    } else if ((m_detailColCopied) && (m_detailColumn < m_columnList.count()) && (m_detailColumn >= 0)) {  //   ...or here
      m_columnList << m_columnList[m_detailColumn];
      m_columnTypeList[m_memoColumn] = "memo";
      for (int i = 0; i < m_memoColList.count(); i++) {
        if (m_memoColList[i] == m_detailColumn) {
          continue;
        }
        m_columnTypeList[m_memoColList[i]] = "memo";
      }
    }
  }

  QStringList::const_iterator constIterator;
  for (constIterator = m_columnList.constBegin(); constIterator != m_columnList.constEnd();
       ++constIterator) {
    QString txt = (*constIterator) + "  ";
    QTableWidgetItem *item = new QTableWidgetItem;  //             new item for UI
    item->setText(txt);
    m_csvDialog->ui->tableWidget->setRowCount(m_row + 1);
    m_csvDialog->ui->tableWidget->setItem(m_row, col, item);  //   add items to UI here
    m_csvDialog->ui->tableWidget->setRowHeight(m_row, 30);
    m_csvDialog->ui->tableWidget->resizeColumnToContents(col);
    col ++;
  }

  if (m_csvDialog->ui->tableWidget->horizontalScrollBar()->isVisible()) {
    m_csvDialog->m_hScrollBarHeight = 17;
  } else {
    m_csvDialog->m_hScrollBarHeight = 0;
  }
  ++m_row;
}

int InvestProcessing::processInvestLine(const QString& inBuffer)
{
  QString newTxt;
  //                                      validate all columns
  int neededFieldsCount = 0;//            ensure essential fields are present

  QString memo;
  QString payee;
  QString txt;
  QString type;

  m_trInvestData.memo.clear();//          initialise in case not overwritten by new data
  m_trInvestData.price = 0;
  m_trInvestData.quantity = 0;
  m_trInvestData.amount = 0;
  m_trInvestData.fee = 0;
  m_trInvestData.payee.clear();
  m_trInvestData.security.clear();
  m_trInvestData.brokerageAccnt.clear();
  m_trInvestData.type.clear();
  m_trInvestData.date = QDate();

  MyMoneyMoney zero = MyMoneyMoney();

  m_brokerage = false;
  memo.clear();

  if (m_columnList.count() < m_endColumn) {
    if (!m_csvDialog->m_accept) {
      QString row = QString::number(m_row);
      int ret = KMessageBox::questionYesNoCancel(0, i18n("<center>Row number %1 does not have the expected number of columns.</center>"
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
        m_csvDialog->m_accept = true;
      }
    }
  }

  for (int i = 0; i < m_columnList.count(); i++) {
    //  Use actual column count for this line instead of m_endColumn, which could be greater.
    if (m_columnTypeList[i] == "date") {      //                    Date Col
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt = txt.remove('"');
      QDate dat = m_convertDat->convertDate(txt);
      if (dat == QDate()) {
        KMessageBox::sorry(0, i18n("<center>An invalid date has been detected during import.</center>"
                                   "<center><b>'%1'</b></center>"
                                   "Please check that you have set the correct date format,\n"
                                   "<center>and start and end lines.</center>"
                                   , txt), i18n("CSV import"));
        m_csvDialog->m_importError = true;
        return KMessageBox::Cancel;
      }
      QString qifDate = dat.toString(m_dateFormats[m_dateFormatIndex]);
      m_tempBuffer = 'D' + qifDate + '\n';
      m_trInvestData.date = dat;
    }

    else if (m_columnTypeList[i] == "type") {      //               Type Col
      type = m_columnList[i];
      m_redefine->setTypeColumn(i);
      QString str = m_columnList[i].trimmed();
      if (str.isEmpty()) {     //                                No Type specified...
        QString txt = m_csvDialog->m_detailFilter;//             ...but may be one buried in 'detail' col. See if there is a filter
        if (!txt.isEmpty()) {     //                             Filter present
          int lngth = m_columnList[m_detailColumn].indexOf(txt);
          if (lngth > -1) {     //                               Position of filter.
            lngth = lngth + txt.length();//                      Length of detail.
            QString tmp = m_columnList[m_detailColumn].remove('"');
            tmp = tmp.remove(0, lngth).toLower();  //            Remove all but new type.
            type = tmp;
            m_columnList[i] = type;
          }
        }
      } else {
        m_trInvestData.type = str.remove('"');  //               There is a type.
      }
      ++neededFieldsCount;
      int ret = processActionType(type);
      if (ret == KMessageBox::Cancel) {
        return KMessageBox::Cancel;
      }

      if (m_brokerage) {     //                                  Brokerage
        QStringList::const_iterator it;

        QString payee = type.toLower();
        QString typ = m_csvDialog->m_detailFilter;
        if (!typ.isEmpty()) {
          int lngth = m_columnList[m_payeeColumn].indexOf(typ);
          if (lngth > -1) {     //                               Found buried type.
            lngth = lngth + typ.length();
            QString tmp = m_columnList[m_payeeColumn];
            tmp = tmp.remove(0, lngth).toLower();
            payee = tmp;
          }
        }
        //
        //  Was brokerage but we might now have genuine investment type.
        //
        for (it = m_brokerageList.constBegin(); it != m_brokerageList.constEnd(); ++it) {      //Brokerage
          if ((payee).contains(*it, Qt::CaseInsensitive)) {
            if (payee.contains("reinv"))  {
              m_trInvestData.type = "reinvdiv";
              m_brokerage = true;
              m_csvSplit.m_strCategoryName = payee;
              break;
            } else if (payee.contains("div"))  {
              m_trInvestData.type = "divx";
              m_brokerage = true;
              m_csvSplit.m_strCategoryName = payee;
              break;
            } else if (payee.contains("interest"))  {
              m_trInvestData.type = "intinc";
              m_brokerage = true;
              m_csvSplit.m_strCategoryName = payee;
              break;
            } else {
              m_trInvestData.type = '0';//  No, so still brokerage.

            }
            m_tempBuffer += 'L' + str + '\n';
            m_tempBuffer += 'P' + m_trInvestData.payee + '\n';
          }
        }//  end of brokerage list 'for loop'
        QString tmp = m_csvSplit.m_strCategoryName;
        m_csvSplit.m_strCategoryName = payee;
      }//  end of brokerage
    }//  end of type col

    else if (m_columnTypeList[i] == "memo") {      //               Memo Col
      txt = m_columnList[i];
      if ((!m_firstPass) && (txt.isEmpty()) && (m_typeColCopied)) {
        txt = m_columnList[m_typeColumn];
        m_columnList[i] = txt;
      }
      if (memo.isEmpty()) {
        if (m_brokerage) {
          m_trInvestData.payee = txt;
        }
      } else {
        memo += " : ";//        separate multiple memos
      }
      memo += txt;//            next memo
    }//end of memo field

    else if (m_columnTypeList[i] == "quantity") {      //           Quantity Col
      ++neededFieldsCount;
      txt = m_columnList[i].remove('-');  //  Remove unwanted -ve sign in quantity.
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      m_trInvestData.quantity = MyMoneyMoney(newTxt);
      m_tempBuffer += 'Q' + newTxt + '\n';
    }

    else if (m_columnTypeList[i] == "price") {      //              Price Col
      ++neededFieldsCount;
      txt = m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceFraction->currentText(); //fraction
      txt = txt.replace(m_csvDialog->decimalSymbol(), KGlobal::locale()->decimalSymbol());
      MyMoneyMoney fraction = MyMoneyMoney(txt);
      txt = m_columnList[i].remove('"');  //                     price
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney price = MyMoneyMoney(newTxt);
      price = price * fraction;
      double val = price.toDouble();
      newTxt.setNum(val, 'f', 6);
      m_trInvestData.price = price;
      m_tempBuffer +=  'I' + newTxt + '\n';//                 price column
    }

    else if (m_columnTypeList[i] == "amount") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt = txt.remove('"');
      if (txt.contains(')')) {
        txt = '-' + txt.remove(QRegExp("[()]"));   //            Mark as -ve
      }
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney amount = MyMoneyMoney(newTxt);
      m_trInvestData.amount = amount;
      m_csvSplit.m_amount = newTxt;
      m_tempBuffer +=  'T' + newTxt + '\n';//                 amount column
    }

    else if (m_columnTypeList[i] == "fee") {      //                Fee Col
      txt = m_columnList[i];
      txt = txt.remove('"');
      if (txt.contains(')')) {
        txt = '-' + txt.remove(QRegExp("[()]"));   //            Mark as -ve
      }
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney fee = MyMoneyMoney(newTxt);
      if (m_csvDialog->m_wiz->m_pageInvestment->ui->checkBoxInv_feeType->isChecked() &&
        fee.toDouble() > 0.00 ) {      //   fee is percent
        txt = m_columnList[m_amountColumn];
        txt = txt.remove('"');
        if (txt.contains(')')) {
          txt = '-' +  txt.remove(QRegExp("[()]"));   //            Mark as -ve
        }
        newTxt = m_parse->possiblyReplaceSymbol(txt);
        MyMoneyMoney amount = MyMoneyMoney(newTxt);
        fee *= amount / MyMoneyMoney(100) ;//               as percentage
      }
      fee.abs();
      m_trInvestData.fee =  fee;
      txt.setNum(fee.toDouble(), 'f', 4);
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      m_tempBuffer +=  'O' + newTxt + '\n';//                  fee amount
    }

    else if (m_columnTypeList[i] == "symbol") { //                Symbol Col
      txt = m_columnList[i];
      QString name;
      if (m_symbolColumn == -1) {
        return KMessageBox::Cancel;
      }
      QString symbol = m_columnList[m_symbolColumn].toUpper().trimmed();
      if (!symbol.isEmpty()) {
        name = m_map.value(symbol);
        m_columnList[i] = symbol;
      } else {
        name = m_columnList[m_detailColumn].trimmed();
      }

      m_trInvestData.symbol = symbol;
      m_trInvestData.security = name;
    }

    else if (m_columnTypeList[i] == "detail") { //                Detail Col
      QString str = m_csvDialog->m_detailFilter;
      QString name;
      QString symbol = m_columnList[m_symbolColumn].toUpper().trimmed();
      if (!symbol.isEmpty()) {
        name = m_map.value(symbol);
      } else {
        name = m_columnList[m_detailColumn].trimmed();
      }
      txt = m_columnList[i];
      if (m_csvDialog->m_symbolTableDlg->m_widget->tableWidget->item(m_symbolRow, 2) != 0) {   //  If this item exists...
        m_trInvestData.security = m_csvDialog->m_symbolTableDlg->m_widget->tableWidget->item(m_symbolRow++, 2)->text() ;  //  Get 'edited' name.
      }
      QStringList list;
      if (!m_csvDialog->m_detailFilter.isEmpty()) {    //          If filter exists...
        list = txt.split(m_csvDialog->m_detailFilter);  //      ...split the detail
      } else {
        list << txt;
      }
      m_columnList[m_detailColumn] = list[0];
      if (list.count() > 1) {
        m_columnList[m_typeColumn] = list[1];//               This is the 'type' we found.
        if ((!m_columnList[m_symbolColumn].trimmed().isEmpty()) && (!m_brokerage)) {    //  If there is a symbol & not brokerage...
          if (m_trInvestData.type.isEmpty()) {  //            If no investment type already...
            m_trInvestData.type = list[1];//                  ...this is investment type.
          }
        } else {
          m_csvSplit.m_strCategoryName = list[1];  //         ...else use as the category.
        }
      }
      if (!txt.isEmpty()) {
        int index = txt.indexOf(str);
        if (index > -1) {
          int lngth = str.length();
          txt = txt.remove(index, lngth).toLower();  //         If there is filter, drop the 'type' from detail...
        } else {
          txt = txt.toLower();
        }
      }
      m_trInvestData.payee = txt;//                           ... and use rest as payee.
    }
  }   //end of col loop
  m_redefine->setInBuffer(inBuffer);
  if (m_trInvestData.type != "0") {       //                       Don't need to do this check on checking items.
    int ret = (m_redefine->checkValid(m_trInvestData.type, i18n("The quantity, price and amount parameters in the\ncurrent transaction do not match with the action type.\nPlease select another action type\n")));
    if (ret == KMessageBox::Cancel) {
      return ret;
    }
  }
  //
  //  A brokerage type could have been changed in m_redefine->checkValid() above, so no longer brokerage.
  //
  if ((m_trInvestData.type == "buy") || (m_trInvestData.type == "sell") ||
      (m_trInvestData.type == "divx") || (m_trInvestData.type == "intinc")) {
    if (m_redefine->accountName().isEmpty()) {
      m_redefine->setAccountName(accountName(i18n("Enter the name of the Brokerage or Checking Account used for the transfer of funds:")));
    }
    m_trInvestData.brokerageAccnt = m_redefine->accountName();
    m_tempBuffer +=  "L[" + m_redefine->accountName() + ']' + '\n';
    m_brokerage = false;
  } else if ((m_trInvestData.type == "reinvdiv") || (m_trInvestData.type == "shrsin") || (m_trInvestData.type == "shrsout")) {
    m_brokerage = false;
  }

  if (m_brokerage) {     //                                        brokerage items
    if (m_brokerBuff.isEmpty()) {      //                          start building data

      if (m_redefine->accountName().isEmpty()) {
        m_redefine->setAccountName(accountName(i18n("Enter the name of the Brokerage or Checking Account used for the transfer of funds:")));
      }
      m_brokerBuff = "!Account\n";
      m_brokerBuff += 'N' + m_redefine->accountName() + '\n';
      m_brokerBuff += "TBank\n^\n";

      m_brokerBuff += "!Type:Bank\n";
    }
    m_trInvestData.brokerageAccnt = m_redefine->accountName();
    m_brokerBuff += m_tempBuffer;
    if (!memo.isEmpty())
      m_brokerBuff += 'M' + memo + '\n';
    m_brokerBuff += "^\n";
    m_brokerBuff = m_brokerBuff.remove('"');
  } else {//                                                  non-brokerage items
    m_tempBuffer += 'N' + m_trInvestData.type + '\n';
    m_outBuffer += m_tempBuffer;
    m_trInvestData.memo = memo;
    if ((m_trInvestData.security.isEmpty()) && (!m_securityName.isEmpty())) {
      m_trInvestData.security = m_securityName;
    }
    m_outBuffer = m_outBuffer + 'Y' + m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText() + '\n';

    if (!memo.isEmpty()) {
      m_outBuffer = m_outBuffer + 'M' + memo + '\n';
    }
    m_outBuffer += "^\n";
    m_outBuffer = m_outBuffer.remove('"');
  }
  if (neededFieldsCount > 3) {
    return KMessageBox::Ok;
  } else {
    KMessageBox::sorry(0, i18n("<center>The columns selected are invalid.\n</center>"
                               "There must an amount or quantity fields, symbol or security name, plus date and type field."
                               "<center>You possibly need to check the start and end line settings, or reset 'Skip setup'.</center>"),
                       i18n("CSV import"));
    return KMessageBox::Cancel;
  }
  return KMessageBox::Ok;
}

int InvestProcessing::processActionType(QString& type)
{
  QStringList::const_iterator it;
  QString memo;
  QString payee;
  QStringList typesList;
  typesList << "buy" << "sell" << "div" << "reinv" << "shrsin" << "shrsout" << "intinc";

  if (m_buyList.isEmpty()) {
    KMessageBox::information(0, i18n("<center>buyList of transaction types was not found.</center>"
                                     "<center>Check existence of correct resource file - 'csvimporterrc'.</center>"));
    return KMessageBox::Cancel;
  }
  bool typeFound = false;
  bool brokerFound = false;

  for (it = typesList.constBegin(); it != typesList.constEnd(); ++it) {    //  Check for possible invest type.
    if (type.contains(*it, Qt::CaseInsensitive)) {
      typeFound = true;
    }
  }
  if (!typeFound) {
    for (it = m_brokerageList.constBegin(); it != m_brokerageList.constEnd(); ++it) {    //  If not, check for Brokerage type.
      if (type.contains(*it, Qt::CaseInsensitive)) {
        brokerFound = true;
      }
    }
  }
  if ((brokerFound) || (type.isEmpty()))  {      //                No investment type, but may still be...
    m_brokerage = true;//                                     ...but assume these are non-investment items, until later.
    if (m_redefine->accountName().isEmpty())
      m_redefine->setAccountName(accountName(i18n("Enter the name of the Brokerage or Checking Account used for the transfer of funds:")));
    m_tempBuffer +=  "L[" + m_redefine->accountName() + ']' + '\n';

    if (m_payeeColumn < 0) {
      m_payeeColumn = columnNumber(i18n("<center>For a brokerage item, enter the column</center>"
                                        "<center>containing the Payee or Detail:</center>")) - 1;//payee column
    }
    if (m_payeeColumn == 0) {
      KMessageBox::sorry(0, i18n("An invalid column was entered.\n"
                                 "Must be between 1 and %1.", m_endColumn), i18n("CSV import"));
      return KMessageBox::Cancel;
    } else if (m_payeeColumn == -1) {
      return KMessageBox::Cancel;
    }
    if (m_detailColumn > -1) {
      m_columnTypeList[m_detailColumn] = "detail";
      m_trInvestData.type = '0';
      m_csvSplit.m_strCategoryName = m_columnList[m_payeeColumn];
      return KMessageBox::Ok;
    } else if (m_securityName.isEmpty()) {
      KMessageBox::information(0, i18n("<center>No Detail field specified</center>"
                                       "<center>and no security name supplied.</center>"
                                       "<center>(Please check the parameters given)</center>"));
      return KMessageBox::Cancel;
    }
  }
  //
  //  If not brokerage, look for genuine investment type.
  //
  for (it = m_shrsinList.constBegin(); it != m_shrsinList.constEnd(); ++it) {    //       Shrsin
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "shrsin";
      m_trInvestData.type = "shrsin";
      return KMessageBox::Ok;
    }
  }
  //                            Needs to be before DivX
  //          because of "ReInvestorContract Buy         : ReInvested Units"
  for (it = m_reinvdivList.constBegin(); it != m_reinvdivList.constEnd(); ++it) {    //   Reinvdiv

    QString txt = (*it);
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "reinvdiv";
      m_trInvestData.type = (type);
      return KMessageBox::Ok;
    }
  }

  //                            Needs to be after Reinvdiv
  for (it = m_divXList.constBegin(); it != m_divXList.constEnd(); ++it) {      //         DivX
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "divx";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName = "dividend";
      return KMessageBox::Ok;
    }
  }

  for (it = m_buyList.constBegin(); it != m_buyList.constEnd(); ++it) {     //            Buy
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "buy";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName.clear();
      return KMessageBox::Ok;
    }
  }

  for (it = m_sellList.constBegin(); it != m_sellList.constEnd(); ++it) {     //          Sell
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "sell";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName.clear();
      return KMessageBox::Ok;
    }
  }

  for (it = m_removeList.constBegin(); it != m_removeList.constEnd(); ++it) {     //      shrsout
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "shrsout";
      m_trInvestData.type = type;
      return KMessageBox::Ok;
    }
  }

  for (it = m_intIncList.constBegin(); it != m_intIncList.constEnd(); ++it) {     //      intinc
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "intinc";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName = "interest";
      return KMessageBox::Ok;
    }
  }
  //   no valid type found
  m_redefine->setInBuffer(m_inBuffer);  //                      Ask user to choose valid type.
  int ret = m_redefine->suspectType(i18n("<center>The transaction below has an unrecognised type or action.</center>"
                                         "<center>Please select an appropriate entry, if available.</center>"
                                         "<center>Otherwise, click Cancel to abort.</center>"));
  //   remember the selection for further transactions
  if (!type.isEmpty() && !m_trInvestData.type.isEmpty() && type.compare(m_trInvestData.type,Qt::CaseInsensitive) != 0) {
      if (m_trInvestData.type == "shrsin") {
        m_shrsinList << type;
      } else if (m_trInvestData.type == "reinvdiv") {
        m_reinvdivList << type;
      } else if (m_trInvestData.type == "divx") {
        m_divXList << type;
      } else if (m_trInvestData.type == "buy") {
        m_buyList << type;
      } else if (m_trInvestData.type == "sell") {
        m_sellList << type;
      } else if (m_trInvestData.type == "shrsout") {
        m_removeList << type;
      } else if (m_trInvestData.type == "intinc") {
        m_intIncList << type;
      }
  }
  return ret;
}//   end of Type Col

void InvestProcessing::investCsvImport(MyMoneyStatement& st)
{
  MyMoneyStatement::Transaction::EAction convType;
  convertType(m_trInvestData.type, convType);
  MyMoneyStatement::Split s1;
  MyMoneyStatement::Transaction tr;
  QString tmp;
  QString payee = m_trInvestData.payee;//                       extractLine('P')
  //
  // Process the securities
  //

  QList<MyMoneyStatement::Security>::const_iterator it_s = m_listSecurities.constBegin();
  while (it_s != m_listSecurities.constEnd()) {
    st.m_listSecurities << (*it_s);
    ++it_s;
  }

  // Process transaction data

  if (m_brokerage) {
    m_brokerageItems = true;
    st.m_eType = MyMoneyStatement::etCheckings;
  } else
    st.m_eType = MyMoneyStatement::etInvestment;
  tr.m_datePosted = m_trInvestData.date;
  if (!m_trInvestData.date.isValid()) {
    int rc = KMessageBox::warningContinueCancel(0, i18n("The date entry \"%1\" read from the file cannot be interpreted through the current date format setting of \"%2.\"\n\n"
             "Pressing \'Continue\' will assign today's date to the transaction. Pressing \'Cancel\'' will abort the import operation. You can then restart the import and select a different date format.",
             m_trInvestData.date.toString(m_dateFormats[m_dateFormatIndex]),
             m_dateFormats[m_dateFormatIndex]), i18n("Invalid date format"));
    switch (rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = (QDate::currentDate());
        break;
      case KMessageBox::Cancel:
        m_importNow = false;//                             Don't process statement
        st = MyMoneyStatement();
        return;
    }
  }
  tr.m_strMemo = m_trInvestData.memo;
  s1.m_strMemo = tr.m_strMemo;
  tr.m_eAction = convType;
  tr.m_amount = m_trInvestData.amount;
  s1.m_amount = tr.m_amount;
  MyMoneyStatement::Split s2 = s1;
  s2.m_amount = MyMoneyMoney(-s1.m_amount);
  tr.m_strInterestCategory = m_csvSplit.m_strCategoryName;
  tr.m_strSecurity = m_trInvestData.security;
  tr.m_strSymbol = m_trInvestData.symbol;

  s2.m_accountId = m_csvUtil->checkCategory(m_csvSplit.m_strCategoryName, s1.m_amount, s2.m_amount);
  tr.m_strBrokerageAccount = m_trInvestData.brokerageAccnt;
  if ((tr.m_eAction == (MyMoneyStatement::Transaction::eaCashDividend)) ||
      (tr.m_eAction == (MyMoneyStatement::Transaction::eaSell)) ||
      (tr.m_eAction == (MyMoneyStatement::Transaction::eaInterest))) {
    /*
     *  need to deduct fees here
     */
    tr.m_amount = tr.m_amount - m_trInvestData.fee.abs();
  }

  else if (tr.m_eAction == (MyMoneyStatement::Transaction::eaBuy)) {
      if (tr.m_amount.isPositive())
          tr.m_amount = -tr.m_amount; //if broker doesn't use minus sings for buy transactions, set it manually here
      tr.m_amount = tr.m_amount - m_trInvestData.fee.abs();
  }

  else if (tr.m_eAction == (MyMoneyStatement::Transaction::eaNone)) {
    tr.m_listSplits += s2;
  }

  tr.m_shares = m_trInvestData.quantity;//                 extractLine('T'));
  if (!payee.isEmpty()) {
    tr.m_strPayee = m_trInvestData.payee;
  }

  tr.m_price = m_trInvestData.price;
  tr.m_fees = m_trInvestData.fee;

  // Add the transaction to the statement

  st.m_listTransactions += tr;
  QList<MyMoneyStatement>   statements;
  if ((st.m_listTransactions.count()) > 0) {
    statements += st;//          this not used
    qDebug("Statement with %d transactions ready", st.m_listTransactions.count());
  }
  m_csvDialog->m_importError = false;  //  Clear error as this import was OK
  // Import the statements
  return;
}

void InvestProcessing::convertType(const QString& type, MyMoneyStatement::Transaction::EAction& convType)
{
  if (type == "buy")
    convType = MyMoneyStatement::Transaction::eaBuy;
  else if (type == "sell")
    convType = MyMoneyStatement::Transaction::eaSell;
  else if (type == "divx")
    convType = MyMoneyStatement::Transaction::eaCashDividend;
  else if (type == "reinvdiv")
    convType = MyMoneyStatement::Transaction::eaReinvestDividend;
  else if (type == "shrsin")
    convType = MyMoneyStatement::Transaction::eaShrsin;
  else if (type == "shrsout")
    convType = MyMoneyStatement::Transaction::eaShrsout;
  else if (type == "intinc")
    convType = MyMoneyStatement::Transaction::eaInterest;
  else convType = MyMoneyStatement::Transaction::eaNone;
  return;
}

void InvestProcessing::slotImportClicked()
{
  m_csvDialog->m_importError = false;
  if (m_csvDialog->m_fileType != "Invest") {
    return;
  }

  if (m_csvDialog->decimalSymbol().isEmpty()) {
    KMessageBox::sorry(0, i18n("<center>Please select the decimal symbol used in your file.\n</center>"), i18n("Investment import"));
    m_csvDialog->m_importError = true;
    return;
  }

  m_securityName = m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  if (m_securityName.isEmpty()) {
    m_securityName = m_csvDialog->m_symbolTableDlg->m_securityName;
  }

  if (m_securityName.isEmpty()) {
    m_securityName = m_trInvestData.security;
  }

  if ((m_securityName.isEmpty()) && (m_symbolColumn < 1)) {
    KMessageBox::sorry(0, i18n("<center>Please enter a name or symbol for the security.\n</center>"), i18n("CSV import"));
    m_csvDialog->m_importError = true;
    return;
  }

  bool securitySelected = true;
  if (!m_securityList.contains(m_securityName)) {
    m_securityList << m_securityName;
  }

  m_dateSelected = (m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->currentIndex() >= 0);
  m_typeSelected = (m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->currentIndex() >= 0);
  m_priceSelected = (m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->currentIndex() >= 0);
  m_quantitySelected = (m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->currentIndex() >= 0);
  m_amountSelected = (m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->currentIndex() >= 0);

  if (m_dateSelected && m_typeSelected && securitySelected && m_quantitySelected && m_priceSelected && m_amountSelected) {
    m_importNow = true;

    //  all necessary data is present

    m_endLine = m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value();
    int skp = m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip->value(); //         skip all headers
    if (skp > m_endLine) {
      KMessageBox::sorry(0, i18n("<center>The start line is greater than the end line.\n</center>"
                                 "<center>Please correct your settings.</center>"), i18n("CSV import"));
      m_csvDialog->m_importError = true;
      return;
    }

    readFile(m_inFileName);
    m_csvDialog->markUnwantedRows();
    m_screenUpdated = true;
  } else {
    KMessageBox::information(0, i18n("The Security Name, and Date and Type columns are needed.<center>Also, the Price, Quantity and Amount columns.</center><center>Please try again.</center>"));
    m_csvDialog->m_importError = true;
    return;
  }
  m_importNow = false;
}

void InvestProcessing::saveAs()
{
  if (m_csvDialog->m_fileType == "Invest") {
    QStringList outFile = m_inFileName .split('.');
    const KUrl& name = QString((outFile.isEmpty() ? "InvestProcessing" : outFile[0]) + ".qif");

    QString outFileName = KFileDialog::getSaveFileName(name, QString::fromLatin1("*.qif | %1").arg(i18n("QIF Files")), 0, i18n("Save QIF")
#if KDE_IS_VERSION(4,4,0)
                          , KFileDialog::ConfirmOverwrite
#endif
                                                      );
    QFile oFile(outFileName);
    oFile.open(QIODevice::WriteOnly);
    QTextStream out(&oFile);
    out << m_outBuffer;//                 output investments to qif file
    out << m_brokerBuff;//                ...also broker type items
    oFile.close();
  }
}

void InvestProcessing::setCodecList(const QList<QTextCodec *> &list)
{
  m_comboBoxEncode->clear();
  foreach (QTextCodec * codec, list)
  m_comboBoxEncode->addItem(codec->name(), codec->mibEnum());
}

void InvestProcessing::startLineChanged(int val)
{
  if (m_csvDialog->m_fileType != "Invest") {
    return;
  }
  if (val > m_fileEndLine) {
    m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip->setValue(m_fileEndLine);
  }
  if (val > m_endLine) {
    m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip->setValue(m_endLine);
    return;
  }
  m_startLine = val;
  m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setMinimum(m_csvDialog->m_investProcessing->m_startLine);  //  to update UI

  if (!m_inFileName.isEmpty()) {
    m_csvDialog->m_vScrollBar->setValue(m_startLine - 1);
    m_csvDialog->markUnwantedRows();
  }
}

void InvestProcessing::startLineChanged()
{
  m_startLine = m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip->value();
}

void InvestProcessing::endLineChanged(int val)
{
  if (m_csvDialog->m_fileType != "Invest") {
    return;
  }
  int tmp = m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value();
  if (tmp > m_fileEndLine) {
    m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setValue(m_fileEndLine);
    return;
  }
  if (tmp < m_startLine) {
    return;
  }
  m_csvDialog->m_wiz->m_pageLinesDate->m_trailerLines = m_fileEndLine - val;
  m_endLine = val;
  if (!m_inFileName.isEmpty()) {
    m_csvDialog->markUnwantedRows();
    int strt = val - m_csvDialog->m_visibleRows;
    if (strt < 0) {  //  start line too low
      strt = 0;
    }
    updateColumnWidths(strt, strt + m_csvDialog->m_visibleRows);
  }
}

void InvestProcessing::dateFormatSelected(int dF)
{
  if (dF == -1 || m_csvDialog->m_fileType != "Invest") {
    return;
  }
  m_dateFormatIndex = dF;
  m_dateFormat = m_dateFormats[m_dateFormatIndex];
  if (m_csvDialog->m_importError) {
    readFile(m_inFileName);
    m_csvDialog->markUnwantedRows();
  }
}

int InvestProcessing::columnNumber(const QString& column)
{
  bool ok;
  static int ret;
  ret = KInputDialog::getInteger(i18n("Brokerage Item"), column, 0, 1, m_endColumn, 1, 10, &ok);
  if (ok && ret > 0)
    return ret;
  return 0;
}

QString InvestProcessing::accountName(const QString& aName)
{
  bool ok;
  static QString accntName;
  accntName = KInputDialog::getText(i18n("Parameters"), aName, QString(), &ok, 0, 0, 0);
  if (ok && !accntName.isEmpty())
    return accntName;
  else return "";
}

void InvestProcessing::readSettings()
{
  m_csvDialog->m_profileExists = false;
  bool found = false;
  //
  //  Only clearColumnTypes if new file is selected.
  //
  if (m_csvDialog->m_inFileName != m_csvDialog->m_lastFileName) {
    clearSelectedFlags();//  Needs to be here in case user selects new profile after cancelling prior one.
    m_csvDialog->m_lastFileName = m_csvDialog->m_inFileName;
  }

  disconnect(m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  int tmp;
  QString str;

  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locate("config", "csvimporterrc"));
  KConfigGroup securitiesGroup(config, "Securities");
  m_securityList.clear();
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->clear();
  m_securityList = securitiesGroup.readEntry("SecurityNameList", QStringList());
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->addItems(m_securityList);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);

  for (int i = 0; i < m_csvDialog->m_profileList.count(); i++) {
    if (m_csvDialog->m_profileList[i] != m_csvDialog->m_profileName) {
      continue;
    } else {
      found = true;
    }
    if (!found) {
      return;
    }

    m_csvDialog->m_profileExists = true;
    QString txt = "Profiles-" + m_csvDialog->m_profileList[i];

    KConfigGroup profilesGroup(config, txt);

    txt = profilesGroup.readEntry("FileType", QString());  //  Read earlier in slotFileDialogClicked()

    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
    tmp = profilesGroup.readEntry("SymbolCol", -1);
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(tmp);

    m_dateFormatIndex = profilesGroup.readEntry("DateFormat", QString()).toInt();
    m_csvDialog->m_wiz->m_pageLinesDate->ui->comboBox_dateFormat->setCurrentIndex(m_dateFormatIndex);
    //    m_encodeIndex = profilesGroup.readEntry("Encoding", QString()).toInt();  //  Read earlier in slotFileDialogClicked().

    if (m_csvDialog->m_needFieldDelimiter) {  //  no columnCount error
      m_csvDialog->m_needFieldDelimiter = false;
      int fieldDelimiterIndx = profilesGroup.readEntry("FieldDelimiter", QString()).toInt();
      m_csvDialog->m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(fieldDelimiterIndx);
    }
    QStringList list = profilesGroup.readEntry("BuyParam", QStringList());
    if (!list.isEmpty()) {
      m_buyList = list;
    }
    list = profilesGroup.readEntry("ShrsinParam", QStringList());
    if (!list.isEmpty()) {
      m_shrsinList = list;
    }
    list = profilesGroup.readEntry("DivXParam", QStringList());
    if (!list.isEmpty()) {
      m_divXList = list;
    }
    list = profilesGroup.readEntry("IntIncParam", QStringList());
    if (!list.isEmpty()) {
      m_intIncList = list;
    }
    list = profilesGroup.readEntry("BrokerageParam", QStringList());
    if (!list.isEmpty()) {
      m_brokerageList = list;
    }
    list = profilesGroup.readEntry("ReinvdivParam", QStringList());
    if (!list.isEmpty()) {
      m_reinvdivList = list;
    }
    list = profilesGroup.readEntry("SellParam", QStringList());
    if (!list.isEmpty()) {
      m_sellList = list;
    }
    list = profilesGroup.readEntry("RemoveParam", QStringList());
    if (!list.isEmpty()) {
      m_removeList = list;
    }
    //    m_invPath  = profilesGroup.readEntry("InvDirectory", QString());  //  Read earlier in fileDialog()

    int tmp = profilesGroup.readEntry("SecurityName", 0);
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(tmp);

    tmp = m_startLine;
    m_startLine = profilesGroup.readEntry("StartLine", 0) + 1;
    if (m_startLine > m_endLine) {
      m_startLine = tmp;
    }
    m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip->setValue(m_startLine);

    str = profilesGroup.readEntry("Filter", QString());
    if (str.endsWith('#')) {     //  Terminates a trailing blank
      str.chop(1);
    }
    m_csvDialog->m_wiz->m_pageInvestment->ui->lineEdit_filter->setText(QString());
    m_csvDialog->m_wiz->m_pageInvestment->ui->lineEdit_filter->setText(str);
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceFraction->setCurrentIndex(profilesGroup.readEntry("PriceFraction", 0));

    if (profilesGroup.exists()) {
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);

      m_dateColumn = profilesGroup.readEntry("DateCol", -1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(m_dateColumn);

      tmp = profilesGroup.readEntry("PayeeCol", -1);  //use for type col.
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(tmp);

      tmp = profilesGroup.readEntry("PriceCol", -1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(tmp);

      tmp = profilesGroup.readEntry("QuantityCol", -1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(tmp);

      tmp = profilesGroup.readEntry("AmountCol", -1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(tmp);

      tmp = profilesGroup.readEntry("DetailCol", -1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(tmp);

      QList<int> list = profilesGroup.readEntry("MemoCol", QList<int>());
      int posn = 0;
      if ((posn = list.indexOf(-1)) > -1) {  //  Look for -1, meaning no memo col
        list.removeOne(-1);  //                  ...and drop the list entry
      }
      m_memoColList = list;
      //
      //  Set up all memo fields...
      //
      for (int i = 0; i < m_memoColList.count(); i++) {
        tmp = m_memoColList[i];
        if (tmp < m_columnTypeList.count()) {
          m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(tmp);
          m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(tmp, QString().setNum(tmp + 1) + '*');
          if (tmp == m_typeColumn) {  //  ...unless also a type field.
            m_typeColCopied = true;
            continue;
          } else if (tmp == m_detailColumn) {  //  ...unless also a type field.
            m_detailColCopied = true;
            continue;
          }
        }
        if (m_columnTypeList.count() > tmp) {
          m_columnTypeList[tmp] = "memo";
          m_memoColumn = tmp;
        }
      }
      tmp = profilesGroup.readEntry("FeeCol", -1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(tmp);

      tmp = profilesGroup.readEntry("SymbolCol", -1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(tmp);
      m_csvDialog->m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(-1);

      tmp = profilesGroup.readEntry("DecimalSymbol", 0);
      m_csvDialog->setDecimalSymbol(tmp);
      m_parse->setDecimalSymbolIndex(tmp);
      m_csvDialog->m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(tmp);
      m_csvDialog->m_wiz->m_pageCompletion->ui->comboBox_thousandsDelimiter->setCurrentIndex(-1);

    } else {
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
    }
  }
  KConfigGroup mainGroup(config, "MainWindow");
  m_csvDialog->m_pluginHeight = mainGroup.readEntry("Height", 640);
  m_csvDialog->m_pluginWidth = mainGroup.readEntry("Width", 800);

  if (m_columnTypeList.count() < 4) {  //  m_columnTypeList invalid
    m_previousColumn = -1;
    m_previousType = -1;
  }
}

void InvestProcessing::reloadUISettings()
{
  m_memoColumn = m_columnTypeList.indexOf("memo");
  m_priceColumn = m_columnTypeList.indexOf("price");
  m_quantityColumn = m_columnTypeList.indexOf("quantity");
  m_dateColumn = m_columnTypeList.indexOf("date");
  m_amountColumn = m_columnTypeList.indexOf("amount");
  m_feeColumn = m_columnTypeList.indexOf("fee");
  m_detailColumn = m_columnTypeList.indexOf("detail");
  m_startLine = m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skip->value();
  m_endLine = m_csvDialog->m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value();
}

void InvestProcessing::clearColumnType(int column)
{
  m_columnTypeList[column].clear();
}

QString InvestProcessing::columnType(int column)
{
  return  m_columnTypeList[column];
}

void InvestProcessing::setColumnType(int column, const QString& type)
{
  m_columnTypeList[column] = type;
}

QString InvestProcessing::previousType()
{
  return m_previousType;
}

void InvestProcessing::clearPreviousType()
{
  m_previousType.clear();
}

void InvestProcessing::setPreviousType(const QString& type)
{
  m_previousType = type;
}

QString InvestProcessing::invPath()
{
  return m_invPath;
}

QString InvestProcessing::inFileName()
{
  return m_inFileName;
}

void InvestProcessing::setTrInvestDataType(const QString& val)
{
  m_trInvestData.type = val;
}

void InvestProcessing::resetComboBox(const QString& comboBox, const int& col)
{
  QStringList fieldType;
  fieldType << "amount" << "date" << "fee" << "memo" << "price" << "quantity" << "type" << "symbol" << "detail";
  int index = fieldType.indexOf(comboBox);
  switch (index) {
    case 0://  amount
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
      m_amountSelected = false;
      break;
    case 1://  date
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
      m_dateSelected = false;
      break;
    case 2://  fee
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
      m_feeSelected = false;
      break;
    case 3://  memo
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1));   //  reset the '*'
      m_memoColList.removeOne(col);  //  We're clearing this memo col.
      m_memoSelected = false;
      break;
    case 4://  price
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
      m_priceSelected = false;
      break;
    case 5://  quantity
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
      m_quantitySelected = false;
      break;
    case 6://  type
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
      m_typeSelected = false;
      break;
    case 7://  symbol
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
      m_symbolSelected = false;
      break;
    case 8://  detail
      m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
      m_detailSelected = false;
      break;
    default:
      KMessageBox::sorry(0, i18n("<center>Field name not recognised.</center><center>'<b>%1</b>'</center>Please re-enter your column selections.", comboBox), i18n("CSV import"));
  }
  m_columnTypeList[col].clear();
}

int InvestProcessing::lastLine()
{
  return m_endLine;
}

int InvestProcessing::amountColumn()
{
  return m_amountColumn;
}

int InvestProcessing::dateColumn()
{
  return m_dateColumn;
}

int InvestProcessing::feeColumn()
{
  return m_feeColumn;
}

int InvestProcessing::quantityColumn()
{
  return m_quantityColumn;
}

int InvestProcessing::priceColumn()
{
  return m_priceColumn;
}

int InvestProcessing::detailColumn()
{
  return m_detailColumn;
}

int InvestProcessing::symbolColumn()
{
  return m_symbolColumn;
}

int InvestProcessing::memoColumn()
{
  return m_memoColumn;
}

int InvestProcessing::typeColumn()
{
  return m_typeColumn;
}

bool InvestProcessing::importNow()
{
  return m_importNow;
}

void  InvestProcessing::setSecurityName(QString name)
{
  m_securityName = name;
}

void InvestProcessing::securityNameSelected(const QString& name)
{
  if ((m_securityList.contains(name)) || (name.isEmpty())) {
    return;
  }

  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setInsertPolicy(QComboBox::InsertAlphabetically);
  m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setDuplicatesEnabled(false);
  m_securityName = name;
  m_securityList << name;
  m_securityList.removeDuplicates();
  m_securityList.sort();
}

void InvestProcessing::securityNameEdited()
{
  QString name = m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  int index = m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->findText(name);
  if ((index >= 0) || (name.isEmpty())) {
    return;
  }
  int rc = KMessageBox::warningContinueCancel(0, i18n("<center>Do you want to add a new security</center>\n"
           "<center>%1 </center>\n"
           "<center>to the selection list?</center>\n"
           "<center>Click \'Continue\' to add the name.</center>\n"
           "<center>Otherwise, click \'Cancel\'.</center>",
           name), i18n("Add Security Name"));
  if (rc == KMessageBox::Cancel) {
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->clearEditText();
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);
  } else {
    m_securityName = name;
    m_securityList << name;
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->addItem(name);
    m_securityList.removeDuplicates();
    m_securityList.sort();
  }
}

QStringList InvestProcessing::securityList()
{
  return m_securityList;
}

void InvestProcessing::hideSecurity()
{
  QString name = m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  if (name.isEmpty()) {
    return;
  }
  int rc = KMessageBox::warningContinueCancel(0, i18n("<center>You have selected to remove from the selection list</center>\n"
           "<center>%1. </center>\n"
           "<center>Click \'Continue\' to remove the name, or</center>\n"
           "<center>Click \'Cancel\'' to leave 'as is'.</center>",
           name), i18n("Hide Security Name"));
  if (rc == KMessageBox::Continue) {
    int index = m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentIndex();
    m_csvDialog->m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->removeItem(index);
    m_securityList.removeAt(index);
    m_securityName.clear();
  }
}

void InvestProcessing::updateColumnWidths(int firstLine, int lastLine)
{
  m_rowWidth = 0;
  m_fileEndLine = m_parse->lastLine();
  QFont font(QApplication::font());
  QFontMetrics cellFontMetrics(font);
  //
  //  Need to recalculate column widths in the visible rows,
  //  to allow shrinking or expanding with the data.
  //
  for (int col = 0; col < m_csvDialog->ui->tableWidget->columnCount(); col ++) {
    int maxColWidth = 0;
    for (int row = firstLine; row <= lastLine; row++) {
      if ((row >= m_lineList.count()) || (row >= m_fileEndLine)) {
        break;
      }

      if (m_csvDialog->ui->tableWidget->item(row, col) == 0) {  //  cell does not exist
        continue;
      }
      //
      //  Ensure colwidth is wide enough for true data width.
      //
      int colWidth = 0;
      QLabel label;
      label.setText(m_csvDialog->ui->tableWidget->item(row, col)->text() + "  ");
      int pad = 0;
      if (col < 10) {
        pad = 6;  //  need to leave extraspace for column number in header
      }
      int wd = 1.05 * cellFontMetrics.width(label.text() + "  ") + pad;
      if (wd > colWidth) {
        colWidth = wd;
      }
      if (colWidth > maxColWidth) {
        maxColWidth = colWidth;
      }
    }  //  end rows
    m_csvDialog->ui->tableWidget->setColumnWidth(col, maxColWidth);
    m_rowWidth += maxColWidth;
  }  //  end cols
  return;
}
