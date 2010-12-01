/*******************************************************************************
*                              investprocessing.cpp
*                              --------------------
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

#include "investprocessing.h"
#include "investmentdlg.h"
// ----------------------------------------------------------------------------
// QT Headers

#include <QtGui/QScrollBar>
#include <QtGui/QDesktopWidget>
#include <QtGui/QCloseEvent>

#include <QtCore/QFile>
#include <QFileDialog>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
// ----------------------------------------------------------------------------
// KDE Headers

#include <kdeversion.h>
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
#include "mymoneymoney.h"
#include "redefinedlg.h"
#include "csvutil.h"

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

  m_dateFormatIndex = 0;
  m_fieldDelimiterIndex = 0;
  m_maxColumnCount = 0;
  m_maxWidth = 0;
  m_payeeColumn = 0;
  m_amountColumn = 0;
  m_dateColumn = 0;
  m_feeColumn = 0;
  m_priceColumn = 0;
  m_quantityColumn = 0;
  m_typeColumn = 0;
  m_width = 0;
  m_endLine = 0;
  m_startLine = 0;
  m_row = 0;
  m_height = 0;
  m_endColumn = 0;

  m_parseline = new ParseLine;
}

InvestProcessing::~InvestProcessing()
{
  delete m_parseline;
}

void InvestProcessing::init()
{
  m_dateFormats << "yyyy/MM/dd" << "MM/dd/yyyy" << "dd/MM/yyyy";
  m_brokerBuff.clear();

  m_investDlg->comboBox_feeCol->setCurrentIndex(-1);//  This col might not get selected, so clear it
  m_investDlg->comboBox_memoCol->setCurrentIndex(-1);// ditto
  m_investDlg->comboBox_fieldDelim->setEnabled(false);

  m_endColumn = MAXCOL;
  m_accountName.clear();

  clearSelectedFlags();

  readSettings();
  m_dateFormatIndex = m_investDlg->comboBox_dateFormat->currentIndex();
  m_investDlg->m_convertDat->setDateFormatIndex(m_dateFormatIndex);
  m_dateFormat = m_dateFormats[m_dateFormatIndex];


  m_buyList += "buy";//          some basic entries in case rc file missing
  m_sellList += "sell";
  m_divXList += "dividend";
  m_reinvdivList += "reinv";
  m_shrsinList += "add";
  m_removeList += "remove";
  m_brokerageList << "check" << "payment";

  findCodecs();
  setCodecList(m_codecs);
}

void InvestProcessing::bankingSelected()
{
  m_investDlg->show();
}

void InvestProcessing::fileDialog()
{
  m_endLine = 0;
  int position;
  if (m_csvPath .isEmpty()) {
    m_csvPath  = "~/";
  }
  QPointer<KFileDialog> dialog = new KFileDialog(KUrl("kfiledialog:///kmymoney-csvinvest"),
      i18n("*.csv *.PRN *.txt | CSV Files\n *.*|All files (*.*)"), 0);
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
                               KIO::NetAccess::lastErrorString(), i18n("File access error"));
    return;
  }
  if (m_inFileName .isEmpty())
    return;
  clearComboBoxText();//                    to clear any '*' in memo combo text
  m_importNow = false;//                    Avoid attempting date formatting on headers
  readFile(m_inFileName , 0);
  m_csvPath  = m_inFileName ;
  position = m_csvPath .lastIndexOf("/");
  m_csvPath .truncate(position + 1);

#ifndef QT
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup investmentGroup(config, "InvestmentSettings");
  QString str = "$HOME/" + m_csvPath.section('/', 3);
  investmentGroup.writeEntry("InvDirectory", str);
  investmentGroup.config()->sync();//               save selected path
#endif

  enableInputs();
}

void InvestProcessing::enableInputs()
{
  m_investDlg->checkBox_qif->setEnabled(true);
  m_investDlg->checkBox_qif->setChecked(false);
  m_investDlg->spinBox_skip->setEnabled(true);
  m_investDlg->comboBox_amountCol->setEnabled(true);
  m_investDlg->comboBox_dateCol->setEnabled(true);
  m_investDlg->comboBox_feeCol->setEnabled(true);
  m_investDlg->comboBox_fieldDelim->setEnabled(true);
  m_investDlg->comboBox_memoCol->setEnabled(true);
  m_investDlg->comboBox_priceCol->setEnabled(true);
  m_investDlg->comboBox_priceFraction->setEnabled(true);
  m_investDlg->comboBox_quantityCol->setEnabled(true);
  m_investDlg->comboBox_typeCol->setEnabled(true);
  m_investDlg->button_clear->setEnabled(true);
  m_investDlg->spinBox_skipLast->setEnabled(true);
  m_investDlg->button_saveAs->setEnabled(true);
  m_investDlg->lineEdit_securityName->setEnabled(true);
  m_investDlg->checkBox_feeType->setEnabled(true);
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
}

void InvestProcessing::clearColumnNumbers()
{
  m_investDlg->comboBox_amountCol->setCurrentIndex(-1);
  m_investDlg->comboBox_dateCol->setCurrentIndex(-1);
  m_investDlg->comboBox_feeCol->setCurrentIndex(-1);
  m_investDlg->comboBox_priceCol->setCurrentIndex(-1);
  m_investDlg->comboBox_quantityCol->setCurrentIndex(-1);
  m_investDlg->comboBox_memoCol->setCurrentIndex(-1);
  m_investDlg->comboBox_typeCol->setCurrentIndex(-1);
}


void InvestProcessing::clearComboBoxText()
{
  for (int i = 0; i < MAXCOL; i++) {
    m_investDlg->comboBox_memoCol->setItemText(i, QString().setNum(i + 1));
  }
}

void InvestProcessing::dateColumnSelected(int col)
{
  QString type = "date";
  if (col < 0) {//                              it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_dateColumn] == type)  && (m_dateColumn != col)) {
    m_columnType[m_dateColumn].clear();
  }
  int ret = validateNewColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_investDlg->comboBox_dateCol->setCurrentIndex(col);// accept new column
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
    m_investDlg->comboBox_dateCol->setCurrentIndex(-1);
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
  if ((!m_columnType[col].isEmpty())  && (m_columnType[col] != type)) {// column is already in use
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected !\
    <center>Please reselect both entries as necessary.</center>", m_columnType[col]));

    m_previousColumn = -1;
    resetComboBox(m_columnType[col], col);//      clash,  so reset ..
    resetComboBox(type, col);//                   ... both comboboxes
    m_previousType.clear();
    m_columnType[col].clear();
    return KMessageBox::Cancel;
  }
  //                                            is this type already in use
  for (int i = 0; i < m_endColumn; i++) {  //  check each column
    if (m_columnType[i] == type) {//            this type already in use
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
  if (col < 0) {//                              it is unset
    return;
  }
  // A new column has been selected for this field so clear old one
  if ((m_columnType[m_feeColumn] == type)  && (m_feeColumn != col)) {
    m_columnType[m_feeColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_investDlg->comboBox_feeCol->setCurrentIndex(col);// accept new column
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
    m_investDlg->comboBox_feeCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::typeColumnSelected(int col)
{
  QString type = "type";
  if (col < 0) {//                              it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_typeColumn] == type)  && (m_typeColumn != col)) {
    m_columnType[m_typeColumn].clear();
  }
  int ret = validateNewColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_investDlg->comboBox_typeCol->setCurrentIndex(col);// accept new column
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
    m_investDlg->comboBox_typeCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::memoColumnSelected(int col)
{
  QString type = "memo";
  if ((col < 0) || (col >= m_endColumn)) {//      out of range so...
    m_investDlg->comboBox_memoCol->setCurrentIndex(-1);// ..clear selection
    return;
  }
  if (m_columnType[col].isEmpty()) {//      accept new  entry
    m_investDlg->comboBox_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    m_columnType[col] = type;
    m_memoColumn = col;
    m_memoSelected = true;
    return;
  } else {//                                    clashes with prior selection
    m_memoSelected = false;//                   clear incorrect selection
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected !\
    <center>Please reselect both entries as necessary.</center>", m_columnType[col]));
    m_investDlg->comboBox_memoCol->setCurrentIndex(-1);
    m_previousColumn = -1;
    resetComboBox(m_columnType[col], col);//      clash,  so reset ..
    resetComboBox(type, col);//                   ... both comboboxes
    m_previousType.clear();
    m_columnType[col].clear();
    if (m_memoColumn >= 0) {
      m_columnType[m_memoColumn].clear();
      m_investDlg->comboBox_memoCol->setItemText(m_memoColumn, QString().setNum(m_memoColumn + 1));//  reset the '*'
      m_investDlg->comboBox_memoCol->setCurrentIndex(-1);//       and this one
    }
  }
}

void InvestProcessing::quantityColumnSelected(int col)
{
  QString type = "quantity";
  if (col < 0) {//                              it is unset
    return;
  }
  m_investDlg->m_redefine->setQuantityColumn(col);
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_quantityColumn] == type)  && (m_quantityColumn != col)) {
    m_columnType[m_quantityColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_investDlg->comboBox_quantityCol->setCurrentIndex(col);// accept new column
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
    m_investDlg->comboBox_quantityCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::priceColumnSelected(int col)
{
  QString type = "price";
  if (col < 0) {//                              it is unset
    return;
  }
  m_investDlg->m_redefine->setPriceColumn(col);
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_priceColumn] == type)  && (m_priceColumn != col)) {
    m_columnType[m_priceColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_investDlg->comboBox_priceCol->setCurrentIndex(col);// accept new column
    m_priceSelected = true;
    if (m_priceColumn != -1) {
//          if a previous price column is detected, but in a different column...
      if ((m_columnType[m_priceColumn] == type)  && (m_priceColumn != col)) {
        m_columnType[m_priceColumn].clear();//  ...clear it
      }
    }
    m_priceColumn = col;
    m_columnType[m_priceColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_investDlg->comboBox_priceCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::amountColumnSelected(int col)
{
  QString type = "amount";
  if (col < 0) {//                              it is unset
    return;
  }
  m_investDlg->m_redefine->setAmountColumn(col);
// A new column has been selected for this field so clear old one
  if ((m_columnType[m_amountColumn] == type)  && (m_amountColumn != col)) {
    m_columnType[m_amountColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_investDlg->comboBox_amountCol->setCurrentIndex(col);// accept new column
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
    m_investDlg->comboBox_amountCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::encodingChanged()
{
  if (!m_inFileName.isEmpty())
    readFile(m_inFileName, 0);
}

void InvestProcessing::fieldDelimiterChanged()
{
  if (!m_inFileName.isEmpty())
    readFile(m_inFileName, 0);
}

void InvestProcessing::readFile(const QString& fname, int skipLines)
{
  MyMoneyStatement st = MyMoneyStatement();
  MyMoneyStatement stBrokerage = MyMoneyStatement();
  m_accountName.clear();
  m_investDlg->m_redefine->clearAccountName();
  m_brokerageItems = false;
  QString name = QDir::homePath();
  QStringList outFile = name.split('.');
  QString outFileName = (outFile.isEmpty() ? "InvestProcessing" : outFile[0]) + ".qif";

  if (!fname.isEmpty())
    m_inFileName  = fname;
  m_startLine = skipLines;
  QFile inFile(m_inFileName);
  inFile.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextStream inStream(&inFile);

  int encodeIndex = m_investDlg->comboBox_encoding->currentIndex();
  QTextCodec *codec = QTextCodec::codecForMib(encodeIndex);

  inStream.setCodec(codec);
  m_investDlg->tableWidget->clear();// including vert headers
  m_inBuffer.clear();
  m_outBuffer = "!Type:Invst\n";
  m_brokerBuff.clear();
  m_row = 0;
  m_maxWidth = 0;
  m_maxColumnCount = 0;
  m_payeeColumn = - 1;

  int lineCount = -1;

  while (!inStream.atEnd()) {
    m_inBuffer = inStream.readLine();
    if (m_inBuffer.isEmpty())
      continue;
    lineCount ++;

    if (lineCount < m_startLine) { //                 not yet at start line
      continue;
    }

    if ((!m_endLine == 0) && (lineCount >= m_endLine)) { //  break if reached last wanted line
      m_investDlg->spinBox_skipLast->setValue(lineCount);
      break;
    }
    displayLine(m_inBuffer);//                            else display it
    if (m_importNow) {
      int ret = processInvestLine(m_inBuffer);  //       parse input line
      if (ret == KMessageBox::Ok) {
        if (m_brokerage)
          investCsvImport(stBrokerage);//       add non-investment transaction to Brokerage statement
        else
          investCsvImport(st);//                 add investment transaction to statement
      } else
        m_importNow = false;
    }
  }// end of while

  updateScreen();

  m_investDlg->label_skip->setEnabled(true);
  m_investDlg->spinBox_skip->setEnabled(true);
  m_investDlg->spinBox_skipLast->setValue(lineCount + 1);

  m_endColumn = m_maxColumnCount;

  if (m_importNow) {
    emit statementReady(st);//              investment statement ready
    if (m_brokerageItems) {
      emit statementReady(stBrokerage);//   brokerage statement ready
    }
    m_importNow = false;
    m_investDlg->checkBox_qif->setChecked(false);
  }
  inFile.close();
}

void InvestProcessing::displayLine(const QString& data)
{
  m_fieldDelimiterIndex = m_investDlg->comboBox_fieldDelim->currentIndex();
  m_parseline->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiter_char = m_parseline->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_columnList = m_parseline->parseLine(data);
  m_investDlg->m_redefine->setColumnList(m_columnList);
  int columnCount = m_columnList.count();

  if (columnCount > m_maxColumnCount)
    m_maxColumnCount = columnCount;//           find highest column count
  else
    columnCount = m_maxColumnCount;
  m_investDlg->tableWidget->setColumnCount(columnCount);

  int col = 0;
  m_width = 6;

  QStringList::const_iterator constIterator;
  for (constIterator = m_columnList.constBegin(); constIterator != m_columnList.constEnd();
       ++constIterator) {
    QString txt = (*constIterator);

    QTableWidgetItem *item = new QTableWidgetItem;        //new item for UI
    item->setText(txt);
    if ((col < 2))
      item->setTextAlignment(Qt::AlignLeft);
    else
      item->setTextAlignment(Qt::AlignRight);
    m_investDlg->tableWidget->setRowCount(m_row + 1);
    m_investDlg->tableWidget->setItem(m_row, col, item);     //add items to UI here
    m_width += m_investDlg->tableWidget->columnWidth(col) ;
    QRect rect = m_investDlg->tableWidget->visualItemRect(item);
    col ++;
  }
  if (m_width > m_maxWidth)
    m_maxWidth = m_width;
  else
    m_width = m_maxWidth;

  m_row += 1;
}

int InvestProcessing::processInvestLine(const QString& inBuffer)
{
  //                                      validate all columns
  QString memo;
  QString payee;
  QString txt;

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

  for (int i = 0; i < m_endColumn; i++) {
    if (m_columnType[i] == "date") { //                    Date Col
      txt = m_columnList[i];
      txt = txt.remove('"');
      QDate dat = m_investDlg->m_convertDat->convertDate(txt);//      Date column
      if (dat == QDate()) {
        KMessageBox::sorry(m_investDlg, i18n("<center>\
        An invalid date has been detected during import.</center>\
        <center><b>%1</b></center>\
        Please check that you have set the correct date format.", txt), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      QString qifDate = dat.toString(m_dateFormats[m_dateFormatIndex]);
      m_tempBuffer = 'D' + qifDate + '\n';
      m_trInvestData.date = dat;
    }

    else if (m_columnType[i] == "type") { //               Type Col
      m_investDlg->m_redefine->setTypeColumn(i);
      txt = m_columnList[i].trimmed();
      QString txt = inBuffer.section(m_fieldDelimiter_char, i, i).trimmed();
      if (txt.isEmpty()) {
        KMessageBox::sorry(m_investDlg, i18n("<center>\
        The type/action that has been detected during import is empty.</center>\
        <center><b>%1</b></center>\
        Check that you have selected the correct column.", txt), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      m_trInvestData.type = txt;
      int ret = processActionType(m_trInvestData.type);
      if (ret == KMessageBox::Cancel) {
        return KMessageBox::Cancel;
      }

      if (m_brokerage) {
        m_trInvestData.payee = inBuffer.section(m_fieldDelimiter_char, m_payeeColumn, m_payeeColumn);
        m_tempBuffer += 'P' + m_trInvestData.payee + '\n';
      }
    }

    else if (m_columnType[i] == "memo") { //               Memo Col
      txt = m_columnList[i];
      if (m_columnType[i] == "memo") {
        if (memo.isEmpty()) {
          if (m_brokerage) {
            payee = txt;
            m_trInvestData.payee = txt;
          }
        } else
          memo += " : ";//        separate multiple memos
        memo += txt;//            next memo
      }
    }//end of memo field

    else if (m_columnType[i] == "quantity") { //           Quantity Col
      txt = m_columnList[i];
      m_trInvestData.quantity = MyMoneyMoney(txt);
      m_tempBuffer += 'Q' + txt + '\n';
    }

    else if (m_columnType[i] == "price") { //              Price Col
      txt = m_investDlg->comboBox_priceFraction->currentText(); //fraction
      MyMoneyMoney fraction = MyMoneyMoney(txt);
      txt = m_columnList[i].remove('"');//                     price
      MyMoneyMoney price = MyMoneyMoney(txt);
      price = price * fraction;    
      double val = price.toDouble();
      txt.setNum(val, 'f', 6);
      m_trInvestData.price = price;
      m_tempBuffer +=  'I' + txt + '\n';//                 price column
    }

    else if (m_columnType[i] == "amount") { //             Amount Col
      txt = m_columnList[i];
      txt = txt.remove('"');
      if (txt.contains(')')) {
        txt = '-' + txt.remove(QRegExp("[()]"));//             Mark as -ve
      }
      MyMoneyMoney amount = MyMoneyMoney(txt);
      m_trInvestData.amount = amount;
      m_tempBuffer +=  'T' + txt + '\n';//                 amount column
    }

    else if (m_columnType[i] == "fee") { //                Fee Col
      MyMoneyMoney amount;
      double percent = m_columnList[i].toDouble();// fee val or percent
      if (percent > 0.00) {
        if (m_investDlg->checkBox_feeType->isChecked()) { //   fee is percent
          //have to use amountCol as amount field may not yet have been processed
          txt = inBuffer.section(m_fieldDelimiter_char, m_amountColumn, m_amountColumn);
          amount = MyMoneyMoney(txt);
          percent *= amount.toDouble() / 100;//                as percentage
        }
        txt.setNum(percent, 'f', 4);
        m_trInvestData.fee = MyMoneyMoney(percent);
        m_tempBuffer +=  'O' + txt + '\n';//                   fee amount
      }
    }

  }//end of col loop
  m_investDlg->m_redefine->setInBuffer(inBuffer);
  if (m_trInvestData.type != "0") { //      Don't need to do this check on checking items.
    int ret = (m_investDlg->m_redefine->checkValid(m_trInvestData.type, i18n("The quantity, price and amount parameters in the\n\
    current transaction don't match with the action type.\n Please select another action type\n")));
    if (ret == KMessageBox::Cancel) return ret;
  }

  if ((m_trInvestData.type == "buy") || (m_trInvestData.type == "sell") || (m_trInvestData.type == "divx")) {
    m_trInvestData.brokerageAccnt = m_investDlg->m_redefine->accountName();
    m_tempBuffer +=  "L[" + m_investDlg->m_redefine->accountName() + ']' + '\n';
  }

  if (m_brokerage) {//                                         brokerage items
    if (m_brokerBuff.isEmpty()) {//                            start building data

      if (m_investDlg->m_redefine->accountName().isEmpty())
        m_investDlg->m_redefine->setAccountName(accountName(i18n("   Brokerage or Chk. Account name:")));

      m_brokerBuff = "!Account\n";
      m_brokerBuff += 'N' + m_investDlg->m_redefine->accountName() + '\n';
      m_brokerBuff += "TBank\n^\n";

      m_brokerBuff += "!Type:Bank\n";
    }
    m_brokerBuff += m_tempBuffer;
    if (!memo.isEmpty())
      m_brokerBuff += 'M' + memo + '\n';
    m_brokerBuff += "^\n";
    m_brokerBuff = m_brokerBuff.remove('"');
  } else {//                                                   end non-brokerage items
    m_tempBuffer += 'N' + m_trInvestData.type + '\n';
    m_outBuffer += m_tempBuffer;
    m_trInvestData.memo = memo;
    m_trInvestData.security = m_investDlg->lineEdit_securityName->text();
    m_outBuffer = m_outBuffer + 'Y' + m_investDlg->lineEdit_securityName->text() + '\n';
    if (!memo.isEmpty())
      m_outBuffer = m_outBuffer + 'M' + memo + '\n';
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

  if (m_buyList.isEmpty()) {
    KMessageBox::information(0, i18n("buyList of transaction types was not found!\
    <center>Check existence of correct resource file - 'csvimporterrc'.</center>"));
    return KMessageBox::Cancel;
  }
  for (it = m_shrsinList.constBegin(); it != m_shrsinList.constEnd(); ++it) { // Shrsin
    if ((type).contains(*it, Qt::CaseInsensitive)) {
      (type) = "shrsin";
      m_trInvestData.type = "shrsin";
      return KMessageBox::Ok;
    }
  }

  //                            Needs to be before DivX
  //          because of "ReInvestorContract Buy         : ReInvested Units"
  for (it = m_reinvdivList.constBegin(); it != m_reinvdivList.constEnd(); ++it) { //Reinvdiv
    if ((type).contains(*it, Qt::CaseInsensitive)) {
      (type) = "reinvdiv";
      m_trInvestData.type = (type);
      return KMessageBox::Ok;
    }
  }

  //                            Needs to be after Reinvdiv
  for (it = m_divXList.constBegin(); it != m_divXList.constEnd(); ++it) { //          DivX
    if ((type).contains(*it, Qt::CaseInsensitive)) {
      (type) = "divx";
      m_trInvestData.type = (type);
      return KMessageBox::Ok;;
    }
  }

  for (it = m_brokerageList.constBegin(); it != m_brokerageList.constEnd(); ++it) { //Brokerage
    if ((type).contains(*it, Qt::CaseInsensitive)) {
      m_brokerage = true;//                   these are non-investment items
      if (m_investDlg->m_redefine->accountName().isEmpty())
        m_investDlg->m_redefine->setAccountName(accountName(i18n(" inv  Brokerage or Chk. Account name:")));
      m_tempBuffer +=  "L[" + m_investDlg->m_redefine->accountName() + ']' + '\n';

      if (m_payeeColumn < 0)
        m_payeeColumn = columnNumber(i18n("Enter Payee or Detail Column:")) - 1;//payee column
      if (m_payeeColumn == 0) {
        KMessageBox::sorry(m_investDlg, i18n("An invalid column was entered.\n"
                                             "Must be between 1 and %1.", m_endColumn), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      m_trInvestData.type = '0';
      return KMessageBox::Ok;
    }
  }

  for (it = m_buyList.constBegin(); it != m_buyList.constEnd(); ++it) { //      Buy
    if ((type).contains(*it, Qt::CaseInsensitive)) {
      (type) = "buy";
      m_trInvestData.type = (type);
      return KMessageBox::Ok;
    }
  }

  for (it = m_sellList.constBegin(); it != m_sellList.constEnd(); ++it) { //     Sell
    if ((type).contains(*it, Qt::CaseInsensitive)) {
      (type) = "sell";
      m_trInvestData.type = (type);
      return KMessageBox::Ok;
    }
  }

  for (it = m_removeList.constBegin(); it != m_removeList.constEnd(); ++it) {//   shrsout
    if ((type).contains(*it, Qt::CaseInsensitive)) {
      (type).clear();
      m_trInvestData.type = (type);
      return KMessageBox::Ok;
    }
  }
  //   no valid type found

  m_investDlg->m_redefine->setInBuffer(m_inBuffer);
  int ret = m_investDlg->m_redefine->suspectType(i18n(" The transaction below has an unrecognised type/action. \n \
  Please select an appropriate entry."));
  return ret;
}//   end of Type Col

void InvestProcessing::investCsvImport(MyMoneyStatement& st)
{
  MyMoneyStatement::Transaction::EAction convType;
  convertType(m_trInvestData.type, convType);
  MyMoneyStatement::Transaction tr;
  QString tmp;
  QString payee = m_trInvestData.payee;//extractLine('P')

  // Process transaction data

  if (m_brokerage) {
    m_brokerageItems = true;
    st.m_eType = MyMoneyStatement::etCheckings;
  } else
    st.m_eType = MyMoneyStatement::etInvestment;
  tr.m_datePosted = m_trInvestData.date;
  if (!m_trInvestData.date.isValid()) {
    int rc = KMessageBox::warningContinueCancel(0, i18n("The date entry \"%1\" read from the file\
    cannot be interpreted through the current date format setting of \"%2.\"\n\n"
             "Pressing \'Continue\' will assign today's date to the transaction. Pressing \'Cancel\'' will\
    abort the import operation. You can then restart the import and select a different date format.",
             m_trInvestData.date.toString(m_dateFormats[m_dateFormatIndex]),
             m_dateFormats[m_dateFormatIndex]), i18n("Invalid date format"));
    switch (rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = (QDate::currentDate());
        break;
      case KMessageBox::Cancel:
        m_importNow = false;//               Don't process statement
        st = MyMoneyStatement();
        return;
    }
  }

  tr.m_strMemo = m_trInvestData.memo;
  tr.m_eAction = convType;
  tr.m_amount = m_trInvestData.amount;
  if ((tr.m_eAction == (MyMoneyStatement::Transaction::eaCashDividend)) ||
      (tr.m_eAction == (MyMoneyStatement::Transaction::eaBuy)) ||
      (tr.m_eAction == (MyMoneyStatement::Transaction::eaSell))) {
    tr.m_strBrokerageAccount = m_trInvestData.brokerageAccnt;
    tr.m_amount = - tr.m_amount;
  }

  else if (tr.m_eAction == (MyMoneyStatement::Transaction::eaNone))
    tr.m_strBrokerageAccount = m_accountName ;

  tr.m_shares = m_trInvestData.quantity;//                extractLine('T'));
  if (!payee.isEmpty()) {
    tr.m_strPayee = m_trInvestData.payee;
  }

  tr.m_price = m_trInvestData.price;
  tr.m_fees = m_trInvestData.fee;
  tr.m_strSecurity = m_trInvestData.security;

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
  else convType = MyMoneyStatement::Transaction::eaNone;
  return;
}

void InvestProcessing::acceptClicked(bool checked)
{
  if (checked) {
    bool securitySelected = (!m_investDlg->lineEdit_securityName->text().isEmpty());
    m_priceSelected = (m_investDlg->comboBox_priceCol->currentIndex() > 0);
    m_quantitySelected = (m_investDlg->comboBox_quantityCol->currentIndex() > 0);
    m_amountSelected = (m_investDlg->comboBox_amountCol->currentIndex() > 0);
    if (m_dateSelected && m_typeSelected && securitySelected &&
        m_quantitySelected && m_priceSelected && m_amountSelected) {//  all necessary data is present
      m_importNow = true;
      m_investDlg->checkBox_qif->setChecked(true);
      m_endLine = m_investDlg->spinBox_skipLast->value();
      int skp = m_investDlg->spinBox_skip->value() - 1;//         skip all headers
      readFile(m_inFileName, skp);//StartLines
      //--- create the vertical (row) headers ---
      QStringList vertHeaders;
      for (int i = skp; i < m_investDlg->tableWidget->rowCount() + skp; i++) {
        QString hdr = QString::number(i + 1);
        vertHeaders += hdr;
      }
      m_investDlg->tableWidget->setVerticalHeaderLabels(vertHeaders);
      m_investDlg->tableWidget->hide();//     to ensure....
      m_investDlg->tableWidget->show();//    ....vertical header width redraws
    } else {

      KMessageBox::information(0, i18n("The Security Name, and Date and Type columns are needed!\
      <center>Also, the Price, Quantity and Amount columns.</center>\
      <center>Please try again.</center>"));
    }
  } else {
    m_importNow = false;
  }
  m_investDlg->checkBox_qif->setChecked(false);
}

void InvestProcessing::saveAs()
{
  QStringList outFile = m_inFileName .split('.');
  const KUrl& name = (outFile.isEmpty() ? "InvestProcessing" : outFile[0]) + ".qif";

  QString outFileName = KFileDialog::getSaveFileName(name, "*.qif | QIF Files", 0, i18n("Save QIF")
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

void InvestProcessing::setCodecList(const QList<QTextCodec *> &list)
{
  m_investDlg->comboBox_encoding->clear();
  foreach (QTextCodec *codec, list)
  m_investDlg->comboBox_encoding->addItem(codec->name(), codec->mibEnum());
}

void InvestProcessing::startLineChanged()
{
  int val = m_investDlg->spinBox_skip->value();
  if (val < 1) return;
  m_startLine = val - 1;
}

void InvestProcessing::endLineChanged()
{
  m_endLine = m_investDlg->spinBox_skipLast->value() ;
}

void InvestProcessing::dateFormatSelected(int dF)
{
  if (dF == -1) return;
  m_dateFormatIndex = dF;
  m_dateFormat = m_dateFormats[m_dateFormatIndex];
}

int InvestProcessing::columnNumber(const QString& column)
{
  bool ok;
  static int ret;
  ret = KInputDialog::getInteger(i18n("Enter column number"), column, 0, 1, m_endColumn, 1, 10, &ok);
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
  int tmp;
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup investmentGroup(config, "InvestmentSettings");
  m_buyList = investmentGroup.readEntry("BuyParam", QStringList());
  m_shrsinList = investmentGroup.readEntry("ShrsinParam", QStringList());
  m_divXList = investmentGroup.readEntry("DivXParam", QStringList());
  m_brokerageList = investmentGroup.readEntry("BrokerageParam", QStringList());
  m_reinvdivList = investmentGroup.readEntry("ReinvdivParam", QStringList());
  m_sellList = investmentGroup.readEntry("SellParam", QStringList());
  m_removeList = investmentGroup.readEntry("RemoveParam", QStringList());
  m_csvPath  = investmentGroup.readEntry("InvDirectory", QString());
  tmp = investmentGroup.readEntry("StartLine", QString()).toInt();
  m_investDlg->spinBox_skip->setValue(tmp + 1);

  KConfigGroup profileGroup(config, "Profile");
  m_dateFormatIndex = profileGroup.readEntry("DateFormat", QString()).toInt();
  m_investDlg->comboBox_dateFormat->setCurrentIndex(m_dateFormatIndex);
  int encodeIndex = profileGroup.readEntry("Encoding", QString()).toInt();
  m_investDlg->comboBox_encoding->setCurrentIndex(encodeIndex);
  int fieldDelimiterIndx = profileGroup.readEntry("FieldDelimiter", QString()).toInt();
  m_investDlg->comboBox_fieldDelim->setCurrentIndex(fieldDelimiterIndx);

  KConfigGroup invcolumnsGroup(config, "InvColumns");
  if (invcolumnsGroup.exists()) {
    tmp = invcolumnsGroup.readEntry("DateCol", QString()).toInt();
    m_investDlg->comboBox_dateCol->setCurrentIndex(-1);
    m_investDlg->comboBox_dateCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("PayeeCol", QString()).toInt();//use for type col.
    m_investDlg->comboBox_typeCol->setCurrentIndex(-1);
    m_investDlg->comboBox_typeCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("MemoCol", QString()).toInt();
    m_investDlg->comboBox_memoCol->setCurrentIndex(-1);
    m_investDlg->comboBox_memoCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("PriceCol", QString()).toInt();
    m_investDlg->comboBox_priceCol->setCurrentIndex(-1);
    m_investDlg->comboBox_priceCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("QuantityCol", QString()).toInt();
    m_investDlg->comboBox_quantityCol->setCurrentIndex(-1);
    m_investDlg->comboBox_quantityCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("AmountCol", QString()).toInt();
    m_investDlg->comboBox_amountCol->setCurrentIndex(-1);
    m_investDlg->comboBox_amountCol->setCurrentIndex(tmp);

    tmp = invcolumnsGroup.readEntry("FeeCol", QString()).toInt();
    m_investDlg->comboBox_feeCol->setCurrentIndex(-1);
    m_investDlg->comboBox_feeCol->setCurrentIndex(tmp);
  } else {
    m_investDlg->comboBox_dateCol->setCurrentIndex(-1);
    m_investDlg->comboBox_typeCol->setCurrentIndex(-1);
    m_investDlg->comboBox_memoCol->setCurrentIndex(-1);
    m_investDlg->comboBox_priceCol->setCurrentIndex(-1);
    m_investDlg->comboBox_quantityCol->setCurrentIndex(-1);
    m_investDlg->comboBox_amountCol->setCurrentIndex(-1);
    m_investDlg->comboBox_feeCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::updateScreen()
{
  if (m_row < 1)
    return;
  m_width = m_maxWidth;
  m_investDlg->setTableFrameHeight(m_investDlg->frame_low->frameGeometry().size().height());
  m_investDlg->setTableFrameWidth(m_investDlg->frame_low->frameGeometry().size().width());

  int hght = 4 + (m_investDlg->tableWidget->rowHeight(m_row - 1)) * m_row;
  hght += m_investDlg->tableWidget->horizontalHeader()->height() + 2;//  frig factor plus vert. headers
  if (m_maxWidth > m_investDlg->tableFrameWidth())
    hght += m_investDlg->tableWidget->horizontalScrollBar()->height();//  ....and for hor. scroll bar
  if (hght > m_investDlg->tableFrameHeight()) {
    int w = m_investDlg->tableWidget->verticalScrollBar()->width() + 2;
    m_width += w;//19
    hght = m_investDlg->tableFrameHeight() - 12;//                        allow for border
  }
  int h = m_investDlg->tableWidget->verticalHeader()->width();
  m_width += h;

  if (m_width >= m_investDlg->tableFrameWidth()) {
    m_width = m_investDlg->tableFrameWidth() - 12;//                        allow for border
    hght += 1;
  }
  m_investDlg->tableWidget->setFixedHeight(hght);
  m_investDlg->tableWidget->setFixedWidth(m_width);
  m_investDlg->tableWidget->setFocus();
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

QString InvestProcessing::csvPath()
{
  return m_csvPath;
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
  fieldType << "amount" << "date" << "fee" << "memo" << "price" << "quantity" << "type";
  int index = fieldType.indexOf(comboBox);
  switch (index) {
    case 0://  amount
      m_investDlg->comboBox_amountCol->setCurrentIndex(-1);
      m_amountSelected = false;
      break;
    case 1://  date
      m_investDlg->comboBox_dateCol->setCurrentIndex(-1);
      m_dateSelected = false;
      break;
    case 2://  fee
      m_investDlg->comboBox_feeCol->setCurrentIndex(-1);
      m_feeSelected = false;
      break;
    case 3://  memo
      m_investDlg->comboBox_memoCol->setCurrentIndex(-1);
      m_investDlg->comboBox_memoCol->setItemText(col, QString().setNum(col + 1));
      m_memoSelected = false;
      break;
    case 4://  price
      m_investDlg->comboBox_priceCol->setCurrentIndex(-1);
      m_priceSelected = false;
      break;
    case 5://  quantity
      m_investDlg->comboBox_quantityCol->setCurrentIndex(-1);
      m_quantitySelected = false;
      break;
    case 6://  type
      m_investDlg->comboBox_typeCol->setCurrentIndex(-1);
      m_typeSelected = false;
      break;
    default:
      qDebug() << i18n("1195 ERROR. Field name not recognised.") << comboBox;
      KMessageBox::sorry(0, i18n("<center>Field name not recognised.</center>\
      <center>'<b>%1</b>'</center>\
      Please re-enter your column selections."
                                 , comboBox), i18n("CSV import"));
  }
  m_columnType[col].clear();
}

