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
#include "investmentdlg.h"
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
#include "symboltabledlg.h"


KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));


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

  m_dateFormatIndex = 0;
  m_fieldDelimiterIndex = 0;
  m_maxColumnCount = 0;
  m_payeeColumn = 0;
  m_amountColumn = 0;
  m_dateColumn = 0;
  m_feeColumn = 0;
  m_memoColumn = 0;
  m_priceColumn = 0;
  m_quantityColumn = 0;
  m_typeColumn = 0;
  m_symbolColumn = 0;
  m_detailColumn = 0;
  m_endLine = 0;
  m_finalLine = 0;
  m_startLine = 0;
  m_row = 0;
  m_height = 0;
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
  m_endColumn = MAXCOL;
  m_accountName.clear();

  clearSelectedFlags();

  m_securityName = m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->currentText();

  QLineEdit* securityLineEdit = m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->lineEdit();//krazy:exclude=<qclasses>

  m_completer = new QCompleter(m_securityList, this);
  m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  securityLineEdit->setCompleter(m_completer);
  connect(securityLineEdit, SIGNAL(editingFinished()), this, SLOT(securityNameEdited()));
  connect(this, SIGNAL(statementReady(MyMoneyStatement&)), m_csvDialog->m_plugin, SLOT(slotGetStatement(MyMoneyStatement&)));

  m_dateFormatIndex = m_csvDialog->m_pageLinesDate->ui->comboBox_dateFormat->currentIndex();
  m_convertDat->setDateFormatIndex(m_dateFormatIndex);
  m_dateFormat = m_dateFormats[m_dateFormatIndex];

  readSettingsInit();//  Needed for broker names in type selection combo.

//  The following string list strings are descriptions of possible investment
//  activity types.  Each of the lists may also contain alternative descriptions,
//  added by the user to the resource file, to suit his needs.

  m_buyList += i18nc("verb", "buy");//                       some basic entries in case rc file missing
  m_sellList += i18nc("verb", "sell");
  m_divXList += i18nc("noun, cash dividend", "dividend");
  m_intIncList += i18nc("noun, interest income", "interest");
  m_reinvdivList += i18nc("verb, to reinvest", "reinvest");
  m_shrsinList += i18nc("verb", "add");
  m_removeList += i18nc("verb, to delete", "remove");
  m_brokerageList << i18nc("noun, cheque, check", "check") << i18nc("noun", "payment");

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
  if (m_csvDialog->m_fileType != "Invest") {
    return;
  }
  m_endLine = 0;
  m_finalLine = 0;
  int position;
  if (m_invPath.isEmpty()) {
    m_invPath  = "~/";
  }
  m_csvDialog->m_decimalSymbolChanged = false;

  QPointer<KFileDialog> dialog = new KFileDialog(KUrl("kfiledialog:///kmymoney-csvinvest"),
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
  if (m_inFileName.isEmpty())
    return;
  clearComboBoxText();//                    to clear any '*' in memo combo text
  m_importNow = false;//                    Avoid attempting date formatting on headers

  for (int i = 0; i < MAXCOL; i++)
    if (columnType(i) == "memo") {
      clearColumnType(i);   //    ensure no memo entries remain
    }

  //  set large field height to ensure resizing sees all lines in new file

  QRect rect = m_csvDialog->ui->tableWidget->geometry();
  rect.setHeight(9999);
  m_csvDialog->ui->tableWidget->setGeometry(rect);
  readFile(m_inFileName , 0);
  m_invPath  = m_inFileName ;
  position = m_invPath .lastIndexOf("/");
  m_invPath .truncate(position + 1);

#ifndef QT
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup investmentGroup(config, "InvestmentSettings");
  QString str = "$HOME/" + m_invPath.section('/', 3);
  investmentGroup.writeEntry("InvDirectory", str);
  investmentGroup.config()->sync();//               save selected path
#endif
  enableInputs();

  m_csvDialog->m_wizard->next();
}

void InvestProcessing::enableInputs()
{
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setEnabled(true);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setEnabled(true);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setEnabled(true);
  m_csvDialog->m_pageSeparator->ui->comboBox_fieldDelimiter->setEnabled(true);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setEnabled(true);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setEnabled(true);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceFraction->setEnabled(true);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setEnabled(true);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setEnabled(true);
  m_csvDialog->m_pageInvestment->ui->button_clear->setEnabled(true);
  m_csvDialog->m_pageLinesDate->ui->spinBox_skipToLast->setEnabled(true);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->setEnabled(true);
  m_csvDialog->m_pageInvestment->ui->checkBoxInv_feeType->setEnabled(true);
}

void InvestProcessing::clearColumnsSelected()
{
  clearSelectedFlags();
  clearColumnNumbers();
  clearComboBoxText();
}

void InvestProcessing::clearSelectedFlags()
{
  for (int i = 0; i < MAXCOL; i++) {
    m_columnType[i].clear();//               set to all empty
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
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
}


void InvestProcessing::clearComboBoxText()
{
  for (int i = 0; i < MAXCOL; i++) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(i, QString().setNum(i + 1));
  }
}

void InvestProcessing::encodingChanged(int index)
{
  m_encodeIndex = index;
  if (!m_inFileName.isEmpty())
    readFile(m_inFileName, 0);
}

void InvestProcessing::dateColumnSelected(int col)
{
  QString type = "date";
  if (col < 0) { //                              it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_dateColumn] == type)  && (m_dateColumn != col)) {
    m_columnType[m_dateColumn].clear();
  }
  int ret = validateNewColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(col);// accept new column
    m_dateSelected = true;
    if (m_dateColumn != -1) {
//          if a previous date column is detected, but in a different column...
      if ((m_columnType[m_dateColumn] == type)  && (m_dateColumn != col)) {
        m_columnType[m_dateColumn].clear();//   clear it
      }
    }
    m_dateColumn = col;
    m_columnType[m_dateColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
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

    if (sortKey.startsWith("UTF-8")) {  // krazy:exclude=strings
      rank = 1;
    } else if (sortKey.startsWith("UTF-16")) {  // krazy:exclude=strings
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
  //  First check if selection is in range
  if ((col < 0) || (col >= m_endColumn)) {
    return KMessageBox::No;
  }
  if ((col == m_previousColumn) && (type == m_previousType)) {
    return -1;
  }
//                                              selection was in range
//                                              ...but does it clash?
  if ((!m_columnType[col].isEmpty())  && (m_columnType[col] != type)) { // column is already in use
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>", m_columnType[col]));

    m_previousColumn = -1;
    resetComboBox(m_columnType[col], col);//      clash,  so reset ..
    resetComboBox(type, col);//                   ... both comboboxes
    m_previousType.clear();
    m_columnType[col].clear();
    return KMessageBox::Cancel;
  }
  //                                            is this type already in use
  for (int i = 0; i < m_endColumn; i++) {  //  check each column
    if (m_columnType[i] == type) { //            this type already in use
      m_columnType[i].clear();//                ...so clear it
    }//  end this col

  }// end all columns checked                   type not in use
  m_columnType[col] = type;//                   accept new type
  if (m_previousColumn != -1) {
    m_previousColumn = col;
  }
  m_previousType = type;
  return KMessageBox::Ok; //                    accept new type
}

void InvestProcessing::feeColumnSelected(int col)
{
  QString type = "fee";
  if (col < 0) { //                              it is unset
    return;
  }
  // A new column has been selected for this field so clear old one
  if ((m_columnType[m_feeColumn] == type)  && (m_feeColumn != col)) {
    m_columnType[m_feeColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(col);// accept new column
    m_feeSelected = true;
    if (m_feeColumn != -1) {
//          if a previous fee column is detected, but in a different column...
      if ((m_columnType[m_feeColumn] == type)  && (m_feeColumn != col)) {
        m_columnType[m_feeColumn].clear();//    ..clear it
      }
    }
    m_feeColumn = col;
    m_columnType[m_feeColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::typeColumnSelected(int col)
{
  QString type = "type";
  if (col < 0) { //                              it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_typeColumn] == type)  && (m_typeColumn != col)) {
    m_columnType[m_typeColumn].clear();
  }
  int ret = validateNewColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(col);// accept new column
    m_typeSelected = true;
    if (m_typeColumn != -1) {
//          if a previous type column is detected, but in a different column...
      if ((m_columnType[m_typeColumn] == type)  && (m_typeColumn != col)) {
        m_columnType[m_typeColumn].clear();//   ...clear it
      }
    }
    m_typeColumn = col;
    m_columnType[m_typeColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::memoColumnSelected(int col)
{
  QString type = "memo";
  if ((col < 0) || (col >= m_endColumn)) { //      out of range so...
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);// ..clear selection
    return;
  }
  if (m_columnType[col].isEmpty()) { //      accept new  entry
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    m_columnType[col] = type;
    m_memoColumn = col;
    m_memoSelected = true;
    return;
  } else {//                                    clashes with prior selection
    m_memoSelected = false;//                   clear incorrect selection
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>", m_columnType[col]));
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
    m_previousColumn = -1;
    resetComboBox(m_columnType[col], col);//      clash,  so reset ..
    resetComboBox(type, col);//                   ... both comboboxes
    m_previousType.clear();
    m_columnType[col].clear();
    if (m_memoColumn >= 0) {
      m_columnType[m_memoColumn].clear();
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(m_memoColumn, QString().setNum(m_memoColumn + 1));//  reset the '*'
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);//       and this one
    }
  }
}

void InvestProcessing::quantityColumnSelected(int col)
{
  QString type = "quantity";
  if (col < 0) { //                              it is unset
    return;
  }
  m_redefine->setQuantityColumn(col);
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_quantityColumn] == type)  && (m_quantityColumn != col)) {
    m_columnType[m_quantityColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(col);// accept new column
    m_quantitySelected = true;
    if (m_quantityColumn != -1) {
//          if a previous fee column is detected, but in a different column...
      if ((m_columnType[m_quantityColumn] == type)  && (m_quantityColumn != col)) {
        m_columnType[m_quantityColumn].clear();// ...clear it
      }
    }
    m_quantityColumn = col;
    m_columnType[m_quantityColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::priceColumnSelected(int col)
{
  QString type = "price";
  if (col < 0) { //                              it is unset
    return;
  }

// A new column has been selected for this field so clear old one
  if ((m_columnType[m_priceColumn] == type)  && (m_priceColumn != col)) {
    m_columnType[m_priceColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(col);// accept new column
    m_priceSelected = true;
    if (m_priceColumn != -1) {
//          if a previous price column is detected, but in a different column...
      if ((m_columnType[m_priceColumn] == type)  && (m_priceColumn != col)) {
        m_columnType[m_priceColumn].clear();//  ...clear it
      }
    }
    m_priceColumn = col;
    m_columnType[m_priceColumn] = type;
    m_redefine->setPriceColumn(col);
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::amountColumnSelected(int col)
{
  QString type = "amount";
  if (col < 0) { //                              it is unset
    return;
  }
  m_redefine->setAmountColumn(col);
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_amountColumn] == type)  && (m_amountColumn != col)) {
    m_columnType[m_amountColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(col);// accept new column
    m_amountSelected = true;
    if (m_amountColumn != -1) {
//          if a previous amount column is detected, but in a different column...
      if ((m_columnType[m_amountColumn] == type)  && (m_amountColumn != col)) {
        m_columnType[m_amountColumn].clear();// ...clear it
      }
    }
    m_amountColumn = col;
    m_columnType[m_amountColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::symbolColumnSelected(int col)
{
  QString type = "symbol";
  if (col < 0) { //                              it is unset
    return;
  }
  m_redefine->setSymbolColumn(col);
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_symbolColumn] == type)  && (m_symbolColumn != col)) {
    m_columnType[m_symbolColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(col);// accept new column
    m_symbolSelected = true;
    if (m_symbolColumn != -1) {
//          if a previous symbol column is detected, but in a different column...
      if ((m_columnType[m_symbolColumn] == type)  && (m_symbolColumn != col)) {
        m_columnType[m_symbolColumn].clear();// ...clear it
      }
    }
    m_symbolColumn = col;
    m_columnType[m_symbolColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::detailColumnSelected(int col)
{
  QString type = "detail";
  if (col < 0) { //                              it is unset
    return;
  }
  m_redefine->setDetailColumn(col);
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_detailColumn] == type)  && (m_detailColumn != col)) {
    m_columnType[m_detailColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(col);// accept new column
    m_detailSelected = true;
    if (m_detailColumn != -1) {
//          if a previous detail column is detected, but in a different column...
      if ((m_columnType[m_detailSelected] == type)  && (m_detailColumn != col)) {
        m_columnType[m_detailColumn].clear();// ...clear it
      }
    }
    m_detailColumn = col;
    m_columnType[m_detailColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::fieldDelimiterChanged()
{
  if (m_csvDialog->m_fileType != "Invest") {
    return;
  }
  if (!m_inFileName.isEmpty())
    readFile(m_inFileName, 0);
}

void InvestProcessing::readFile(const QString& fname, int skipLines)
{
  MyMoneyStatement st = MyMoneyStatement();
  MyMoneyStatement stBrokerage = MyMoneyStatement();

  m_fieldDelimiterIndex = m_csvDialog->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex();
  m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_textDelimiterIndex = m_csvDialog->m_pageSeparator->ui->comboBox_textDelimiter->currentIndex();
  m_parse->setTextDelimiterIndex(m_textDelimiterIndex);
  m_textDelimiterCharacter = m_parse->textDelimiterCharacter(m_textDelimiterIndex);

  m_csvDialog->ui->tableWidget->clear();// including vert headers
  m_inBuffer.clear();
  m_outBuffer = "!Type:Invst\n";
  m_brokerBuff.clear();
  m_row = 0;
  m_maxColumnCount = 0;
  m_payeeColumn = - 1;

  m_accountName.clear();
  m_redefine->clearAccountName();
  m_brokerageItems = false;
  QString name = QDir::homePath();
  QStringList outFile = name.split('.');
  QString outFileName = (outFile.isEmpty() ? "InvestProcessing" : outFile[0]) + ".qif";

  if (!fname.isEmpty())
    m_inFileName  = fname;

  QFile inFile(m_inFileName);
  inFile.open(QIODevice::ReadOnly | QIODevice::Text);

  QTextStream inStream(&inFile);
  QTextCodec *codec = QTextCodec::codecForMib(m_encodeIndex);
  inStream.setCodec(codec);

  m_buf = inStream.readAll();
  //
  //  Start parsing the buffer
  //
  m_lineList = m_parse->parseFile(m_buf, skipLines, m_endLine);
  m_endLine = m_parse->lastLine();
  if (m_endLine > m_finalLine) {
    m_finalLine = m_endLine;
  }
  m_csvDialog->m_pageLinesDate->ui->spinBox_skipToLast->setValue(m_parse->lastLine());

  m_csvDialog->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  m_screenUpdated = false;

  //  Display the buffer
  m_symblRow = 0;
  for (int i = 0; i < m_lineList.count(); i++) {
    m_inBuffer = m_lineList[i];

    displayLine(m_inBuffer);//                             display it
    if (m_importNow) {
      int ret = processInvestLine(m_inBuffer, i);  //         parse input line
      if (ret == KMessageBox::Ok) {
        if (m_brokerage)
          investCsvImport(stBrokerage);//       add non-investment transaction to Brokerage statement
        else
          investCsvImport(st);//                add investment transaction to statement
      } else
        m_importNow = false;
    }
  }// end of buffer

  //  Adjust table size (drop header lines)

  updateScreen();

  m_csvDialog->m_pageLinesDate->ui->labelSet_skip->setEnabled(true);
  m_csvDialog->m_pageLinesDate->ui->spinBox_skip->setEnabled(true);

  m_endColumn = m_maxColumnCount;

  if (m_importNow) {
    emit statementReady(st);//              investment statement ready
    if (m_brokerageItems) {
      emit statementReady(stBrokerage);//   brokerage statement ready
    }
    m_importNow = false;
  }
  inFile.close();
}

void InvestProcessing::displayLine(const QString& data)
{
  m_fieldDelimiterIndex = m_csvDialog->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex();
  m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_columnList = m_parse->parseLine(data);
  m_redefine->setColumnList(m_columnList);
  int columnCount = m_columnList.count();

  if (columnCount > m_maxColumnCount)
    m_maxColumnCount = columnCount;//           find highest column count
  else
    columnCount = m_maxColumnCount;
  m_csvDialog->ui->tableWidget->setColumnCount(columnCount);

  int col = 0;

  QStringList::const_iterator constIterator;
  for (constIterator = m_columnList.constBegin(); constIterator != m_columnList.constEnd();
       ++constIterator) {
    QString txt = (*constIterator);
    QTableWidgetItem *item = new QTableWidgetItem;        //new item for UI
    item->setText(txt);
    item->setTextAlignment(Qt::AlignLeft);
    m_csvDialog->ui->tableWidget->setRowCount(m_row + 1);
    m_csvDialog->ui->tableWidget->setItem(m_row, col, item);     //add items to UI here
    col ++;
  }
  ++m_row;
}

int InvestProcessing::processInvestLine(const QString& inBuffer, int line)
{
  QString newTxt;

  if ((m_priceColumn >= m_maxColumnCount) || (m_quantityColumn >= m_maxColumnCount) || (m_amountColumn >= m_maxColumnCount)) {
    KMessageBox::sorry(0, (i18n("The price, quantity and/or amount column values appear to be invalid."
                                "<center>Please correct the settings.</center>")),
                       i18n("CSV import"));
    return KMessageBox::Cancel;
  }
  //                                      validate all columns
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

  for (int i = 0; i < m_columnList.count(); i++) {//  Use actual column count for this line instead of m_endColumn, which could be greater.
    if (m_columnType[i] == "date") { //                    Date Col
      txt = m_columnList[i];
      txt = txt.remove('"');
      QDate dat = m_convertDat->convertDate(txt);
      if (dat == QDate()) {
        KMessageBox::sorry(0, i18n("<center>An invalid date has been detected during import.</center> <center><b>%1</b></center> Please check that you have set the correct date format.", txt), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      QString qifDate = dat.toString(m_dateFormats[m_dateFormatIndex]);
      m_tempBuffer = 'D' + qifDate + '\n';
      m_trInvestData.date = dat;
    }

    else if (m_columnType[i] == "type") { //               Type Col
      type = m_columnList[i];
      m_redefine->setTypeColumn(i);
      QString str = m_columnList[i].trimmed();
      if (str.isEmpty()) { //                                No Type specified...
        QString txt = m_csvDialog->m_detailFilter;//        ...but may be one buried in 'detail' col. See if there is a filter
        if (!txt.isEmpty()) { //                             Filter present
          int lngth = m_columnList[m_detailColumn].indexOf(txt);
          if (lngth > -1) { //                               Position of filter.
            lngth = lngth + txt.length();//                 Length of detail.
            QString tmp = m_columnList[m_detailColumn].remove('"');
            tmp = tmp.remove(0, lngth).toLower();//         Remove all but new type.
            type = tmp;
            m_columnList[i] = type;
          }
        }
      } else {
        m_trInvestData.type = str.remove('"');//            There is a type.
      }
      int ret = processActionType(type);
      if (ret == KMessageBox::Cancel) {
        return KMessageBox::Cancel;
      }

      if (m_brokerage) { //                                  Brokerage
        QStringList::const_iterator it;

        QString payee = type.toLower();
        QString typ = m_csvDialog->m_detailFilter;
        if (!typ.isEmpty()) {
          int lngth = m_columnList[m_payeeColumn].indexOf(typ);
          if (lngth > -1) { //                               Found buried type.
            lngth = lngth + typ.length();
            QString tmp = m_columnList[m_payeeColumn];
            tmp = tmp.remove(0, lngth).toLower();
            payee = tmp;
          }
        }
        //
        //  Was brokerage but we might now have genuine investment type.
        //
        for (it = m_brokerageList.constBegin(); it != m_brokerageList.constEnd(); ++it) {  //Brokerage
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

    else if (m_columnType[i] == "memo") { //               Memo Col
      txt = m_columnList[i];
      if (memo.isEmpty()) {
        if (m_brokerage) {
          m_trInvestData.payee = txt;
        }
      } else
        memo += " : ";//        separate multiple memos
      memo += txt;//            next memo
    }//end of memo field

    else if (m_columnType[i] == "quantity") { //           Quantity Col
      txt = m_columnList[i];
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      m_trInvestData.quantity = MyMoneyMoney(newTxt);
      m_tempBuffer += 'Q' + newTxt + '\n';
    }

    else if (m_columnType[i] == "price") { //              Price Col
      txt = m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceFraction->currentText(); //fraction
      txt = txt.replace(m_csvDialog->decimalSymbol(), KGlobal::locale()->decimalSymbol());
      MyMoneyMoney fraction = MyMoneyMoney(txt);
      txt = m_columnList[i].remove('"');//                     price
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney price = MyMoneyMoney(newTxt);
      price = price * fraction;
      double val = price.toDouble();
      newTxt.setNum(val, 'f', 6);
      m_trInvestData.price = price;
      m_tempBuffer +=  'I' + newTxt + '\n';//                 price column
    }

    else if (m_columnType[i] == "amount") {
      txt = m_columnList[i];
      txt = txt.remove('"');
      if (txt.contains(')')) {
        txt = '-' + txt.remove(QRegExp("[()]"));//            Mark as -ve
      }
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney amount = MyMoneyMoney(newTxt);
      m_trInvestData.amount = amount;
      m_csvSplit.m_amount = newTxt;
      m_tempBuffer +=  'T' + newTxt + '\n';//                 amount column
    }

    else if (m_columnType[i] == "fee") { //                Fee Col
      MyMoneyMoney amount;
      double percent = m_columnList[i].toDouble();// fee val or percent
      if (percent > 0.00) {
        if (m_csvDialog->m_pageInvestment->ui->checkBoxInv_feeType->isChecked()) { //   fee is percent
          //have to use amountCol as amount field may not yet have been processed
          txt = inBuffer.section(m_fieldDelimiterCharacter, m_amountColumn, m_amountColumn);
          amount = MyMoneyMoney(txt);
          percent *= amount.toDouble() / 100;//               as percentage
        }
        txt.setNum(percent, 'f', 4);
        m_trInvestData.fee = MyMoneyMoney(percent);
        m_tempBuffer +=  'O' + txt + '\n';//                  fee amount
      }
    }

    else if (m_columnType[i] == "symbol") { //                Symbol Col
      txt = m_columnList[i];
      QString name;
      QString symbol = m_columnList[m_symbolColumn].toLower().trimmed();
      if (!symbol.isEmpty()) {
        name = m_map.value(symbol);
        m_columnList[i] = symbol;
      } else {
        name = m_columnList[m_detailColumn].toLower();
      }

      m_trInvestData.security = name;
    }

    else if (m_columnType[i] == "detail") { //                Detail Col
      QString str = m_csvDialog->m_detailFilter;
      QString name;
      QString symbol = m_columnList[m_symbolColumn].toLower().trimmed();
      if (!symbol.isEmpty()) {
        name = m_map.value(symbol);
      } else {
        name = m_columnList[m_detailColumn].toLower();
      }
      txt = m_columnList[i];
      if (m_csvDialog->m_symbolTableDlg->m_widget->tableWidget->item(line, 2) != 0) {//  If this item exists...
        m_trInvestData.security = m_csvDialog->m_symbolTableDlg->m_widget->tableWidget->item(line, 2)->text() ; //  Get 'edited' name.
      }
      QStringList list;
      if (!m_csvDialog->m_detailFilter.isEmpty()) {//          If filter exists...
        list = txt.split(m_csvDialog->m_detailFilter);//      ...split the detail
      } else {
        list << txt;
      }
      m_columnList[m_detailColumn] = list[0];
      if (list.count() > 1) {
        m_columnList[m_typeColumn] = list[1];//               This is the 'type' we found.
        if ((!m_columnList[m_symbolColumn].trimmed().isEmpty()) && (!m_brokerage)) {//  If there is a symbol & not brokerage...
          if (m_trInvestData.type.isEmpty()) { //              If no investment type already...
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
          txt = txt.remove(index, lngth).toLower();//         If there is filter, drop the 'type' from detail...
        } else {
          txt = txt.toLower();
        }
      }
      m_trInvestData.payee = txt;//                           ... and use rest as payee.
    }

  }   //end of col loop
  m_redefine->setInBuffer(inBuffer);
  if (m_trInvestData.type != "0") {  //                       Don't need to do this check on checking items.
    int ret = (m_redefine->checkValid(m_trInvestData.type, i18n("The quantity, price and amount parameters in the\n current transaction don't match with the action type.\n Please select another action type\n")));
    if (ret == KMessageBox::Cancel) {
      return ret;
    }
  }
  //
  //  A brokerage type could have been changed in m_redefine->checkValid() above, so no longer brokerage.
  //
  if ((m_trInvestData.type == "buy") || (m_trInvestData.type == "sell") || (m_trInvestData.type == "reinvdiv") ||
      (m_trInvestData.type == "divx") || (m_trInvestData.type == "intinc") ||
      (m_trInvestData.type == "shrsin") || (m_trInvestData.type == "shrsout")) {
    m_trInvestData.brokerageAccnt = m_redefine->accountName();
    m_tempBuffer +=  "L[" + m_redefine->accountName() + ']' + '\n';
    m_brokerage = false;
  }

  if (m_brokerage) { //                                        brokerage items
    if (m_brokerBuff.isEmpty()) { //                          start building data

      if (m_redefine->accountName().isEmpty()) {
        m_redefine->setAccountName(accountName(i18n("Enter the name of the Brokerage or Checking Account used for the transfer of funds : ")));
      }
      m_brokerBuff = "!Account\n";
      m_brokerBuff += 'N' + m_redefine->accountName() + '\n';
      m_brokerBuff += "TBank\n^\n";

      m_brokerBuff += "!Type:Bank\n";
    } m_trInvestData.brokerageAccnt = m_redefine->accountName();
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
    m_outBuffer = m_outBuffer + 'Y' + m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->currentText() + '\n';

    if (!memo.isEmpty()) {
      m_outBuffer = m_outBuffer + 'M' + memo + '\n';
    }
    m_outBuffer += "^\n";
    m_outBuffer = m_outBuffer.remove('"');
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

  for (it = typesList.constBegin(); it != typesList.constEnd(); ++it) { //  Check for possible invest type.
    if (type.contains(*it, Qt::CaseInsensitive)) {
      typeFound = true;
    }
  }
  if (!typeFound) {
    for (it = m_brokerageList.constBegin(); it != m_brokerageList.constEnd(); ++it) { //  If not, check for Brokerage type.
      if (type.contains(*it, Qt::CaseInsensitive)) {
        brokerFound = true;
      }
    }
  }
  if ((brokerFound) || (type.isEmpty()))  { //                No investment type, but may still be...
    m_brokerage = true;//                                     ...but assume these are non-investment items, until later.
    if (m_redefine->accountName().isEmpty())
      m_redefine->setAccountName(accountName(i18n("Enter the name of the Brokerage or Checking Account used for the transfer of funds : ")));
    m_tempBuffer +=  "L[" + m_redefine->accountName() + ']' + '\n';

    if (m_payeeColumn < 0) {
      m_payeeColumn = columnNumber(i18n("<center>For a brokerage item, enter the column</center>"
                                        "<center>containing the Payee or Detail :</center>")) - 1;//payee column
    }
    if (m_payeeColumn == 0) {
      KMessageBox::sorry(0, i18n("An invalid column was entered.\n"
                                 "Must be between 1 and %1.", m_endColumn), i18n("CSV import"));
      return KMessageBox::Cancel;
    } else if (m_payeeColumn == -1) {
      return KMessageBox::Cancel;
    }
    m_trInvestData.type = '0';
    m_csvSplit.m_strCategoryName = m_columnList[m_payeeColumn];
    return KMessageBox::Ok;
  }
  //
  //  If not brokerage, look for genuine investment type.
  //
  for (it = m_shrsinList.constBegin(); it != m_shrsinList.constEnd(); ++it) { ///       Shrsin
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "shrsin";
      m_trInvestData.type = "shrsin";
      return KMessageBox::Ok;
    }
  }
  //                            Needs to be before DivX
  //          because of "ReInvestorContract Buy         : ReInvested Units"
  for (it = m_reinvdivList.constBegin(); it != m_reinvdivList.constEnd(); ++it) { ///   Reinvdiv

    QString txt = (*it);
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "reinvdiv";
      m_trInvestData.type = (type);
      return KMessageBox::Ok;
    }
  }

  //                            Needs to be after Reinvdiv
  for (it = m_divXList.constBegin(); it != m_divXList.constEnd(); ++it) {  ///         DivX
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "divx";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName = "dividend";
      return KMessageBox::Ok;
    }
  }

  for (it = m_buyList.constBegin(); it != m_buyList.constEnd(); ++it) { ///            Buy
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "buy";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName.clear();
      return KMessageBox::Ok;
    }
  }

  for (it = m_sellList.constBegin(); it != m_sellList.constEnd(); ++it) { ///          Sell
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "sell";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName.clear();
      return KMessageBox::Ok;
    }
  }

  for (it = m_removeList.constBegin(); it != m_removeList.constEnd(); ++it) { ///      shrsout
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "shrsout";
      m_trInvestData.type = type;
      return KMessageBox::Ok;
    }
  }

  for (it = m_intIncList.constBegin(); it != m_intIncList.constEnd(); ++it) { ///      intinc
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "intinc";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName = "interest";
      return KMessageBox::Ok;
    }
  }
  //   no valid type found

  m_redefine->setInBuffer(m_inBuffer);//                      Ask user to choose valid type.
  int ret = m_redefine->suspectType(i18n(" The transaction below has an unrecognised type/action. \nPlease select an appropriate entry."));
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

  s2.m_accountId = checkCategory(m_csvSplit.m_strCategoryName, s1.m_amount, s2.m_amount);
  if ((tr.m_eAction == (MyMoneyStatement::Transaction::eaCashDividend)) ||
      (tr.m_eAction == (MyMoneyStatement::Transaction::eaBuy)) ||
      (tr.m_eAction == (MyMoneyStatement::Transaction::eaSell)) ||
      (tr.m_eAction == (MyMoneyStatement::Transaction::eaInterest))) {
    tr.m_strBrokerageAccount = m_trInvestData.brokerageAccnt;
    tr.m_amount = - tr.m_amount;
  }

  else if (tr.m_eAction == (MyMoneyStatement::Transaction::eaNone)) {
    tr.m_strBrokerageAccount = m_trInvestData.brokerageAccnt;
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
  if (m_csvDialog->m_fileType != "Invest") {
    return;
  }

  if (m_csvDialog->decimalSymbol().isEmpty()) {
    KMessageBox::sorry(0, i18n("<center>Please select the decimal symbol used in your file.\n</center>"), i18n("Investment import"));
    return;
  }

  m_securityName = m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  if (m_securityName.isEmpty()) {
    m_securityName = m_csvDialog->m_symbolTableDlg->m_securityName;
  } else if (m_securityName.isEmpty()) {
    m_securityName = m_trInvestData.security;
  }

  if ((m_securityName.isEmpty()) && (m_symbolColumn < 1)) {
    KMessageBox::sorry(0, i18n("<center>Please enter a name or symbol for the security.\n</center>"), i18n("CSV import"));
    return;
  }

  bool securitySelected = true;
  if (!m_securityList.contains(m_securityName)) {
    m_securityList << m_securityName;
  }
  m_priceSelected = (m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->currentIndex() > 0);
  m_quantitySelected = (m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->currentIndex() > 0);
  m_amountSelected = (m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->currentIndex() > 0);

  if (m_dateSelected && m_typeSelected && securitySelected && m_quantitySelected && m_priceSelected && m_amountSelected) {
    m_importNow = true;

    //  all necessary data is present

    m_endLine = m_csvDialog->m_pageLinesDate->ui->spinBox_skipToLast->value();
    int skp = m_csvDialog->m_pageLinesDate->ui->spinBox_skip->value(); //         skip all headers

    if (skp > m_endLine) {
      KMessageBox::sorry(0, i18n("<center>The start line is greater than the end line.\n</center>"
                                 "<center>Please correct your settings.</center>"), i18n("CSV import"));
      return;
    }

    readFile(m_inFileName, skp);//StartLines
    m_screenUpdated = true;
    //--- create the vertical (row) headers ---
    QStringList vertHeaders;
    for (int i = skp; i < m_csvDialog->ui->tableWidget->rowCount() + skp; i++) {
      QString hdr = QString::number(i);
      vertHeaders += hdr;
    }
    m_csvDialog->ui->tableWidget->setVerticalHeaderLabels(vertHeaders);
    m_csvDialog->ui->tableWidget->hide();//     to ensure....
    m_csvDialog->ui->tableWidget->show();//    ....vertical header width redraws
  } else {
    KMessageBox::information(0, i18n("The Security Name, and Date and Type columns are needed.<center>Also, the Price, Quantity and Amount columns.</center><center>Please try again.</center>"));
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
  if (m_csvDialog->m_fileType == "Banking") {
    return;
  }
  val = m_csvDialog->m_pageLinesDate->ui->spinBox_skip->value();
  if (val < 1) {
    return;
  }
  m_startLine = val;
}

void InvestProcessing::startLineChanged()
{
  m_startLine = m_csvDialog->m_pageLinesDate->ui->spinBox_skip->value();
}

void InvestProcessing::endLineChanged(int val)
{
  m_endLine = val;
}

void InvestProcessing::endLineChanged()
{
  m_endLine = m_csvDialog->m_pageLinesDate->ui->spinBox_skipToLast->value() ;
}

void InvestProcessing::dateFormatSelected(int dF)
{
  if (dF == -1) {
    return;
  }
  m_dateFormatIndex = dF;
  m_dateFormat = m_dateFormats[m_dateFormatIndex];
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

void InvestProcessing::readSettingsInit()
{
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
  KConfigGroup brokerGroup(config, "Brokers");

  QStringList list = brokerGroup.readEntry("BrokerName", QStringList());
  if (!list.isEmpty()) {
    m_brokerList = list;
  }
}

void InvestProcessing::readSettings()
{
  int tmp;
  QString str;
  clearSelectedFlags();

  KConfigGroup profileGroup(config, "Profile");

  str = profileGroup.readEntry("FileType", QString());

  m_dateFormatIndex = profileGroup.readEntry("DateFormat", QString()).toInt();
  m_csvDialog->m_pageLinesDate->ui->comboBox_dateFormat->setCurrentIndex(m_dateFormatIndex);
  m_encodeIndex = profileGroup.readEntry("Encoding", QString()).toInt();
  int fieldDelimiterIndx = profileGroup.readEntry("FieldDelimiter", QString()).toInt();
  m_csvDialog->m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(fieldDelimiterIndx);
  m_csvDialog->m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(-1);
  m_csvDialog->m_pageCompletion->ui->comboBox_thousandsDelimiter->setCurrentIndex(-1);

  KConfigGroup investmentGroup(config, "InvestmentSettings");

  QStringList list = investmentGroup.readEntry("BuyParam", QStringList());
  if (!list.isEmpty()) {
    m_buyList = list;
  }
  m_shrsinList = investmentGroup.readEntry("ShrsinParam", QStringList());
  m_divXList = investmentGroup.readEntry("DivXParam", QStringList());
  m_intIncList = investmentGroup.readEntry("IntIncParam", QStringList());
  m_brokerageList = investmentGroup.readEntry("BrokerageParam", QStringList());
  list = investmentGroup.readEntry("ReinvdivParam", QStringList());
  if (!list.isEmpty()) {
    m_reinvdivList = list;
  }
  m_sellList = investmentGroup.readEntry("SellParam", QStringList());
  m_removeList = investmentGroup.readEntry("RemoveParam", QStringList());
  m_invPath  = investmentGroup.readEntry("InvDirectory", QString());

  tmp = investmentGroup.readEntry("StartLine", QString()).toInt();
  m_csvDialog->m_pageLinesDate->ui->spinBox_skip->setValue(-1);//     force change of val
  m_csvDialog->m_pageLinesDate->ui->spinBox_skip->setValue(tmp + 1);

  KConfigGroup invcolumnsGroup(config, "InvColumns");
  if (invcolumnsGroup.exists()) {
    tmp = invcolumnsGroup.readEntry("DateCol", QString()).toInt();
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("PayeeCol", QString()).toInt();//use for type col.
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(tmp);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);

    tmp = invcolumnsGroup.readEntry("PriceCol", QString()).toInt();
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("QuantityCol", QString()).toInt();
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("AmountCol", QString()).toInt();
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("FeeCol", QString()).toInt();
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("SymbolCol", QString()).toInt();
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("DetailCol", QString()).toInt();
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(tmp);

  } else {
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
  }

  KConfigGroup securitiesGroup(config, "Securities");
  m_securityList.clear();
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->clear();
  m_securityList = securitiesGroup.readEntry("SecurityNameList", QStringList());
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->addItems(m_securityList);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);
}

void InvestProcessing::readSettings(int index)
{
  int tmp;
  QString str;
  clearSelectedFlags();

  KConfigGroup profileGroup(config, "Profile");

  str = profileGroup.readEntry("FileType", QString());

  m_dateFormatIndex = profileGroup.readEntry("DateFormat", QString()).toInt();
  m_csvDialog->m_pageLinesDate->ui->comboBox_dateFormat->setCurrentIndex(m_dateFormatIndex);
  m_encodeIndex = profileGroup.readEntry("Encoding", QString()).toInt();
  int fieldDelimiterIndx = profileGroup.readEntry("FieldDelimiter", QString()).toInt();
  m_csvDialog->m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(fieldDelimiterIndx);
  m_csvDialog->m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(-1);
  m_csvDialog->m_pageCompletion->ui->comboBox_thousandsDelimiter->setCurrentIndex(-1);

  KConfigGroup securitiesGroup(config, "Securities");
  m_securityList.clear();
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->clear();
  m_securityList = securitiesGroup.readEntry("SecurityNameList", QStringList());
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->addItems(m_securityList);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);

  index = index - 2;
  switch (index) {
    case 0: {
        KConfigGroup investmentGroup(config, "BrokerageSettings1");

        QStringList list = investmentGroup.readEntry("BuyParam", QStringList());
        if (!list.isEmpty()) {
          m_buyList = list;
        }
        m_shrsinList = investmentGroup.readEntry("ShrsinParam", QStringList());
        m_divXList = investmentGroup.readEntry("DivXParam", QStringList());
        m_intIncList = investmentGroup.readEntry("IntIncParam", QStringList());
        m_brokerageList = investmentGroup.readEntry("BrokerageParam", QStringList());
        list = investmentGroup.readEntry("ReinvdivParam", QStringList());
        if (!list.isEmpty()) {
          m_reinvdivList = list;
        }
        m_sellList = investmentGroup.readEntry("SellParam", QStringList());
        m_removeList = investmentGroup.readEntry("RemoveParam", QStringList());
        m_invPath  = investmentGroup.readEntry("InvDirectory", QString());

        tmp = investmentGroup.readEntry("StartLine", QString()).toInt();
        m_csvDialog->m_pageLinesDate->ui->spinBox_skip->setValue(-1);//     force change of val
        m_csvDialog->m_pageLinesDate->ui->spinBox_skip->setValue(tmp + 1);

        str = investmentGroup.readEntry("Filter", QString());
        if (str.endsWith('#')) { //  Terminates a trailing blank
          str.chop(1);
        }
        m_csvDialog->m_pageInvestment->ui->lineEdit_filter->setText(QString());
        m_csvDialog->m_pageInvestment->ui->lineEdit_filter->setText(str);

        KConfigGroup invcolumnsGroup1(config, "BrokerageColumns1");
        if (invcolumnsGroup1.exists()) {
          tmp = invcolumnsGroup1.readEntry("DateCol", QString()).toInt();
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(tmp);

          tmp = invcolumnsGroup1.readEntry("PayeeCol", QString()).toInt();//use for type col.
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(tmp);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);

          tmp = invcolumnsGroup1.readEntry("PriceCol", QString()).toInt();
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(tmp);

          tmp = invcolumnsGroup1.readEntry("QuantityCol", QString()).toInt();
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(tmp);

          tmp = invcolumnsGroup1.readEntry("AmountCol", QString()).toInt();
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(tmp);

          tmp = invcolumnsGroup1.readEntry("FeeCol", QString()).toInt();
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(tmp);

          tmp = invcolumnsGroup1.readEntry("SymbolCol", QString()).toInt();
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(tmp);

          tmp = invcolumnsGroup1.readEntry("DetailCol", QString()).toInt();
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(tmp);

        } else {
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
          m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
        }
        break;
      }
    case 1:
      KConfigGroup investmentGroup2(config, "BrokerageSettings2");

      QStringList list = investmentGroup2.readEntry("BuyParam", QStringList());
      if (!list.isEmpty()) {
        m_buyList = list;
      }
      m_shrsinList = investmentGroup2.readEntry("ShrsinParam", QStringList());
      m_divXList = investmentGroup2.readEntry("DivXParam", QStringList());
      m_intIncList = investmentGroup2.readEntry("IntIncParam", QStringList());
      m_brokerageList = investmentGroup2.readEntry("BrokerageParam", QStringList());
      list = investmentGroup2.readEntry("ReinvdivParam", QStringList());
      if (!list.isEmpty()) {
        m_reinvdivList = list;
      }
      m_sellList = investmentGroup2.readEntry("SellParam", QStringList());
      m_removeList = investmentGroup2.readEntry("RemoveParam", QStringList());
      m_invPath  = investmentGroup2.readEntry("InvDirectory", QString());

      tmp = investmentGroup2.readEntry("StartLine", QString()).toInt();
      m_csvDialog->m_pageLinesDate->ui->spinBox_skip->setValue(-1);//     force change of val
      m_csvDialog->m_pageLinesDate->ui->spinBox_skip->setValue(tmp + 1);

      str = investmentGroup2.readEntry("Filter", QString());
      if (str.endsWith('#')) { //  Terminates a trailing blank
        str.chop(1);
      }
      m_csvDialog->m_pageInvestment->ui->lineEdit_filter->setText(str);

      KConfigGroup invcolumnsGroup2(config, "BrokerageColumns2");
      if (invcolumnsGroup2.exists()) {
        tmp = invcolumnsGroup2.readEntry("DateCol", QString()).toInt();
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(tmp);

        tmp = invcolumnsGroup2.readEntry("PayeeCol", QString()).toInt();//use for type col.
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(tmp);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);

        tmp = invcolumnsGroup2.readEntry("PriceCol", QString()).toInt();
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(tmp);

        tmp = invcolumnsGroup2.readEntry("QuantityCol", QString()).toInt();
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(tmp);

        tmp = invcolumnsGroup2.readEntry("AmountCol", QString()).toInt();
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(tmp);

        tmp = invcolumnsGroup2.readEntry("FeeCol", QString()).toInt();
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(tmp);

        tmp = invcolumnsGroup2.readEntry("SymbolCol", QString()).toInt();
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(tmp);

        tmp = invcolumnsGroup2.readEntry("DetailCol", QString()).toInt();
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(tmp);

      } else {
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
        m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
      }
      break;
  }
}

void InvestProcessing::updateScreen()
{
  if (m_row < 1)
    return;
  m_csvDialog->ui->tableWidget->setRowCount(m_row);
  m_csvDialog->ui->tableWidget->setFocus();
}

void InvestProcessing::clearColumnType(int column)
{
  m_columnType[column].clear();
}

QString InvestProcessing::columnType(int column)
{
  return  m_columnType[column];
}

void InvestProcessing::setColumnType(int column, const QString& type)
{
  m_columnType[column] = type;
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
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
      m_amountSelected = false;
      break;
    case 1://  date
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
      m_dateSelected = false;
      break;
    case 2://  fee
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
      m_feeSelected = false;
      break;
    case 3://  memo
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1));
      m_memoSelected = false;
      break;
    case 4://  price
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
      m_priceSelected = false;
      break;
    case 5://  quantity
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
      m_quantitySelected = false;
      break;
    case 6://  type
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
      m_typeSelected = false;
      break;
    case 7://  symbol
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
      m_symbolSelected = false;
      break;
    case 8://  detail
      m_csvDialog->m_pageInvestment->ui->comboBoxInv_detailCol->setCurrentIndex(-1);
      m_detailSelected = false;
      break;
    default:
      qDebug() << i18n("ERROR. Field name not recognised.") << comboBox;
      KMessageBox::sorry(0, i18n("<center>Field name not recognised.</center><center>'<b>%1</b>'</center>Please re-enter your column selections.", comboBox)
                         , i18n("CSV import"));
  }
  m_columnType[col].clear();
}

int InvestProcessing::lastLine()
{
  return m_endLine;
}

int InvestProcessing::amountColumn()
{
  return m_amountColumn;
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

  m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->setInsertPolicy(QComboBox::InsertAlphabetically);
  m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->setDuplicatesEnabled(false);
  m_securityName = name;
  m_securityList << name;
  m_securityList.removeDuplicates();
  m_securityList.sort();
}

void InvestProcessing::securityNameEdited()
{
  QString name = m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  int index = m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->findText(name);
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
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->clearEditText();
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);
  } else {
    m_securityName = name;
    m_securityList << name;
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->addItem(name);
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
  QString name = m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  if (name.isEmpty()) {
    return;
  }
  int rc = KMessageBox::warningContinueCancel(0, i18n("<center>You have selected to remove from the selection list</center>\n"
           "<center>%1. </center>\n"
           "<center>Click \'Continue\' to remove the name, or</center>\n"
           "<center>Click \'Cancel\'' to leave 'as is'.</center>",
           name), i18n("Hide Security Name"));
  if (rc == KMessageBox::Continue) {
    int index = m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->currentIndex();
    m_csvDialog->m_pageInvestment->ui->comboBoxInv_securityName->removeItem(index);
    m_securityList.removeAt(index);
    m_securityName.clear();
  }
}


const QString InvestProcessing::checkCategory(const QString& name, const MyMoneyMoney& value, const MyMoneyMoney& value2)
{
  //  Borrowed from MyMoneyQifReader::checkCategory()
  QString accountId;
  MyMoneyFile *file = MyMoneyFile::instance();
  MyMoneyAccount account;
  bool found = true;

  if (!name.isEmpty()) {
    // The category might be constructed with an arbitraty depth (number of
    // colon delimited fields). We try to find a parent account within this
    // hierarchy by searching the following sequence:
    //
    //    aaaa:bbbb:cccc:ddddd
    //
    // 1. search aaaa:bbbb:cccc:dddd, create nothing
    // 2. search aaaa:bbbb:cccc     , create dddd
    // 3. search aaaa:bbbb          , create cccc:dddd
    // 4. search aaaa               , create bbbb:cccc:dddd
    // 5. don't search              , create aaaa:bbbb:cccc:dddd

    account.setName(name);
    QString accName;      // part to be created (right side in above list)
    QString parent(name);    // a possible parent part (left side in above list)
    do {
      accountId = file->categoryToAccount(parent);
      if (accountId.isEmpty()) {
        found = false;
        // prepare next step
        if (!accName.isEmpty())
          accName.prepend(':');
        accName.prepend(parent.section(':', -1));
        account.setName(accName);
        parent = parent.section(':', 0, -2);
      } else if (!accName.isEmpty()) {
        account.setParentAccountId(accountId);
      }
    } while (!parent.isEmpty() && accountId.isEmpty());

    // if we did not find the category, we create it
    if (!found) {
      MyMoneyAccount parent;
      if (account.parentAccountId().isEmpty()) {
        if (!value.isNegative() && value2.isNegative())
          parent = file->income();
        else
          parent = file->expense();
      } else {
        parent = file->account(account.parentAccountId());
      }
      account.setAccountType((!value.isNegative() && value2.isNegative()) ? MyMoneyAccount::Income : MyMoneyAccount::Expense);
      MyMoneyAccount brokerage;
      // clear out the parent id, because createAccount() does not like that
      account.setParentAccountId(QString());
      createAccount(account, parent, brokerage, MyMoneyMoney());
      accountId = account.id();
    }
  }

  return accountId;
}


void InvestProcessing::createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // make sure we have a currency. If none is assigned, we assume base currency
  if (newAccount.currencyId().isEmpty())
    newAccount.setCurrencyId(file->baseCurrency().id());

  MyMoneyFileTransaction ft;
  try {
    int pos;
    // check for ':' in the name and use it as separator for a hierarchy
    while ((pos = newAccount.name().indexOf(MyMoneyFile::AccountSeperator)) != -1) {
      QString part = newAccount.name().left(pos);
      QString remainder = newAccount.name().mid(pos + 1);
      const MyMoneyAccount& existingAccount = file->subAccountByName(parentAccount, part);
      if (existingAccount.id().isEmpty()) {
        newAccount.setName(part);

        file->addAccount(newAccount, parentAccount);
        parentAccount = newAccount;
      } else {
        parentAccount = existingAccount;
      }
      newAccount.setParentAccountId(QString());  // make sure, there's no parent
      newAccount.clearId();                       // and no id set for adding
      newAccount.removeAccountIds();              // and no sub-account ids
      newAccount.setName(remainder);
    }

    const MyMoneySecurity& sec = file->security(newAccount.currencyId());
    // Check the opening balance
    if (openingBal.isPositive() && newAccount.accountGroup() == MyMoneyAccount::Liability) {
      QString message = i18n("This account is a liability and if the "
                             "opening balance represents money owed, then it should be negative.  "
                             "Negate the amount?\n\n"
                             "Please click Yes to change the opening balance to %1,\n"
                             "Please click No to leave the amount as %2,\n"
                             "Please click Cancel to abort the account creation."
                             , MyMoneyUtils::formatMoney(-openingBal, newAccount, sec)
                             , MyMoneyUtils::formatMoney(openingBal, newAccount, sec));

      int ans = KMessageBox::questionYesNoCancel(0, message);
      if (ans == KMessageBox::Yes) {
        openingBal = -openingBal;

      } else if (ans == KMessageBox::Cancel)
        return;
    }

    file->addAccount(newAccount, parentAccount);

    if (newAccount.accountType() == MyMoneyAccount::Investment
        && !brokerageAccount.name().isEmpty()) {
      file->addAccount(brokerageAccount, parentAccount);

      // set a link from the investment account to the brokerage account
      file->modifyAccount(newAccount);
      file->createOpeningBalanceTransaction(brokerageAccount, openingBal);

    } else
      file->createOpeningBalanceTransaction(newAccount, openingBal);

    ft.commit();
  } catch (MyMoneyException *e) {
    KMessageBox::information(0, i18n("Unable to add account: %1", e->what()));
    delete e;
  }
}

void InvestProcessing::reloadUI()
{
  m_lineList = m_parse->parseFile(m_buf, 0, m_finalLine);
  m_columnList.clear();
  m_row = 0;
  m_maxColumnCount = 0;
  for (int i = 0; i < m_finalLine; i++) {
    m_inBuffer = m_lineList[i];
    displayLine(m_inBuffer);//                             display it
  }
  //--- update the vertical (row) headers ---
  QStringList vertHeaders;
  for (int i = 1; i < m_csvDialog->ui->tableWidget->rowCount(); i++) {
    QString hdr = QString::number(i);
    vertHeaders += hdr;
  }
  m_csvDialog->ui->tableWidget->setVerticalHeaderLabels(vertHeaders);
}
