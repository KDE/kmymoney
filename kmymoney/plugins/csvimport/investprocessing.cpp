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
#include <QtCore/QPointer>
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
#include "redefinedlg.h"

InvestProcessing::InvestProcessing()
{
}

InvestProcessing::~InvestProcessing()
{
}

void InvestProcessing::init()
{
  m_dateFormats << "yyyy/MM/dd" << "MM/dd/yyyy" << "dd/MM/yyyy";
  m_brokerBuff.clear();

  m_investDlg->comboBox_feeCol->setCurrentIndex(-1);//  This col might not get selected, so clear it
  m_investDlg->comboBox_memoCol->setCurrentIndex(-1);// ditto
  m_investDlg->comboBox_fieldDelim->setEnabled(false);

  m_lastColumn = MAXCOL;
  m_accountName.clear();

  clearSelectedFlags();

  readSettings();
  m_dateFormatIndex = m_investDlg->comboBox_dateFormat->currentIndex();
  m_investDlg->m_convertDat->m_dateFormatIndex = m_dateFormatIndex;
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
}

void InvestProcessing::clearSelectedFlags()
{
  for (int i = 0; i < MAXCOL; i++)
    m_columnType[i].clear();//               set to all empty

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

void InvestProcessing::dateColumnSelected(int col)
{
  int dateCol = col;
  if ((dateCol < 0) || (dateCol >= m_lastColumn)) {
    m_investDlg->comboBox_dateCol->setCurrentIndex(-1);
    return;
  }
  if ((m_columnType[dateCol].isEmpty()) && (m_dateSelected == false)) {
    m_columnType[dateCol] = "date";
    m_dateSelected = true;
  } else {
    m_investDlg->comboBox_dateCol->setCurrentIndex(-1);
    KMessageBox::information(0, i18n("That column, or the date field, is already selected!\
                                     <center>Please select a different column or field.</center>"));
  }
}

void InvestProcessing::disableInputs()
{
  m_investDlg->checkBox_qif->setEnabled(false);
  m_investDlg->checkBox_qif->setChecked(false);
  m_investDlg->spinBox_skip->setEnabled(false);
  m_investDlg->comboBox_typeCol->setEnabled(false);
  m_investDlg->comboBox_dateCol->setEnabled(false);
  m_investDlg->comboBox_feeCol->setCurrentIndex(-1);
  m_investDlg->comboBox_memoCol->setEnabled(false);
  m_investDlg->button_clear->setEnabled(false);
  m_investDlg->spinBox_skipLast->setEnabled(false);
  m_investDlg->button_saveAs->setEnabled(false);
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

void InvestProcessing::feeColumnSelected(int col)
{
  int feeCol = col;
  if ((feeCol < 0) || (feeCol >= m_lastColumn)) {
    m_investDlg->comboBox_feeCol->setCurrentIndex(-1);
    return;
  }
  if ((m_columnType[feeCol].isEmpty()) && (m_feeSelected == false)) {
    m_columnType[feeCol] = "fee";
    m_feeSelected = true;
  } else {
    m_investDlg->comboBox_feeCol->setCurrentIndex(-1);
    KMessageBox::information(0, i18n("That column, or the fee field, is already selected!\
                                     <center>Please select a different column or field.</center>"));
  }
}

void InvestProcessing::typeColumnChanged(int col)
{
  int typeCol = col;
  if ((typeCol < 0) || (typeCol >= m_lastColumn)) {
    m_investDlg->comboBox_typeCol->setCurrentIndex(-1);
    return;
  }
  if ((m_columnType[typeCol].isEmpty()) && (m_typeSelected == false)) {
    m_columnType[typeCol] = "type";
    m_typeSelected = true;

  } else {
    m_investDlg->comboBox_typeCol->setCurrentIndex(-1);
    KMessageBox::information(0, i18n("That column, or the type field, is already selected!\
                                     <center>Please select a different column or field.</center>"));
  }
}

void InvestProcessing::memoColumnSelected(int index)
{
  int memoCol = index;
  if ((memoCol < 0) || (memoCol >= m_lastColumn)) {
    m_investDlg->comboBox_memoCol->setCurrentIndex(-1);
    return;
  }
  if (m_columnType[memoCol].isEmpty()) {
    m_columnType[memoCol] = "memo";
    m_memoSelected = true;
  } else {
    m_investDlg->comboBox_memoCol->setCurrentIndex(-1);
    KMessageBox::information(0, i18n("That column is already selected!\
                                     <center>Please select a different column or field.</center>"));
  }
}

void InvestProcessing::quantityColumnSelected(int col)
{
  m_investDlg->m_redefine->m_quantityColumn = col;
  if ((col < 0) || (col >= m_lastColumn)) {
    m_investDlg->comboBox_quantityCol->setCurrentIndex(-1);
    return;
  }
  if ((m_columnType[col].isEmpty()) && (m_quantitySelected == false)) {
    m_columnType[col] = "quantity";
    m_quantitySelected = true;
  } else {
    m_investDlg->comboBox_quantityCol->setCurrentIndex(-1);
    KMessageBox::information(0, i18n("That column, or the quantity field, is already selected!\
                                     <center>Please select a different column or field.</center>"));
  }
}

void InvestProcessing::priceColumnSelected(int col)
{
  m_investDlg->m_redefine->m_priceColumn = col;
  if ((col < 0) || (col >= m_lastColumn)) {
    m_investDlg->comboBox_priceCol->setCurrentIndex(-1);
    return;
  }
  if ((m_columnType[col].isEmpty()) && (m_priceSelected == false)) {
    m_columnType[col] = "price";
    m_priceSelected = true;
  } else {
    m_investDlg->comboBox_priceCol->setCurrentIndex(-1);
    KMessageBox::information(0, i18n("That column, or the price field, is already selected!\
                                     <center>Please select a different column or field.</center>"));
  }
}

void InvestProcessing::amountColumnSelected(int col)
{
  m_amountColumn = col;
  m_investDlg->m_redefine->m_amountColumn = col;
  if ((col < 0) || (col >= m_lastColumn)) {
    m_investDlg->comboBox_amountCol->setCurrentIndex(-1);
    return;
  }
  if ((m_columnType[col].isEmpty()) && (m_amountSelected == false)) {
    m_columnType[col] = "amount";
    m_amountSelected = true;
  } else {
    m_investDlg->comboBox_amountCol->setCurrentIndex(-1);
    KMessageBox::information(0, i18n("That column, or the amount field, is already selected!\
                                     <center>Please select a different column or field.</center>"));
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
  m_investDlg->m_redefine->m_accountName.clear();
  m_brokerageItems = false;
  QString name = QDir::homePath();
  QStringList outFile[1] = name.split('.');
  QString outFileName = outFile[0].at(0) + ".qif";

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

  m_lastColumn = m_maxColumnCount;

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
  const QString  delimChar[] = {",", ";", ":", "\t", "\",\""};
  m_inBuffer = data;
  int fieldDelimiterIndx = m_investDlg->comboBox_fieldDelim->currentIndex();
  m_fieldDelimiter_char = delimChar[fieldDelimiterIndx];

  QStringList columnList = m_inBuffer.split(m_fieldDelimiter_char);// split the fields
  m_investDlg->m_redefine->m_columnList = columnList;
  int columnCount = columnList.count();
  if (columnCount > m_maxColumnCount)
    m_maxColumnCount = columnCount;//           find highest column count
  else
    columnCount = m_maxColumnCount;
  m_investDlg->tableWidget->setColumnCount(columnCount);

  m_width = 6;

  for (int col = 0; col <= columnCount; col++) {
    QString txt = m_inBuffer.section(m_fieldDelimiter_char, col, col);
    txt = txt.remove('"');
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
  m_trInvestData.price.clear();
  m_trInvestData.quantity.clear();
  m_trInvestData.amount.clear();
  m_trInvestData.fee.clear();
  m_trInvestData.payee.clear();
  m_trInvestData.security.clear();
  m_trInvestData.brokerageAccnt.clear();
  m_trInvestData.type.clear();
  m_trInvestData.date = QDate();

  m_brokerage = false;
  memo.clear();

  for (int i = 0; i < m_lastColumn; i++) {
    if (m_columnType[i] == "date") { //                             Date Col
      txt = inBuffer.section(m_fieldDelimiter_char, i, i);
      txt = txt.remove('"');
      QDate dat = m_investDlg->m_convertDat->convertDate(txt);//          Date column
      if (dat == QDate()) {
        KMessageBox::sorry(m_investDlg, i18n("<center>\
                    An invalid date has been detected during import.</center>\
                    <center><b>%1</b></center>\
                    Please check that you have set the correct date format."
                                             ,                  txt), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      QString qifDate = dat.toString(m_dateFormats[m_dateFormatIndex]);
      m_tempBuffer = 'D' + qifDate + '\n';
      m_trInvestData.date = dat;
    }

    else if (m_columnType[i] == "type") { //                             Type Col
      m_investDlg->m_redefine->m_typeColumn = i;
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

    else if (m_columnType[i] == "memo") { //                            Memo Col
      txt = inBuffer.section(m_fieldDelimiter_char, i, i);
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

    else if (m_columnType[i] == "quantity") { //                       Quantity Col
      txt = inBuffer.section(m_fieldDelimiter_char, i, i);
      m_trInvestData.quantity = txt;
      m_tempBuffer += 'Q' + txt + '\n';
    }

    else if (m_columnType[i] == "price") { //                          Price Col
      txt = m_investDlg->comboBox_priceFraction->currentText(); //fraction
      float val = txt.toFloat();                        //as float
      txt = inBuffer.section(m_fieldDelimiter_char, i, i);          //price
      float val1 = txt.toFloat();                     //as float
      val = val * val1;
      txt.setNum(val, 'f', 6);
      m_trInvestData.price = txt;
      m_tempBuffer +=  'I' + txt + '\n';//       price column
    }

    else if (m_columnType[i] == "amount") { //                         Amount Col
      txt = inBuffer.section(m_fieldDelimiter_char, i, i);
      if (txt.contains(')')) {
        txt = txt.remove('(');
        txt = '-' + txt.remove(')');
      }
      m_trInvestData.amount = txt;
      m_tempBuffer +=  'T' + txt + '\n';//       amount column
    }

    else if (m_columnType[i] == "fee") { //              Fee Col
      float val;
      float val1 = inBuffer.section(m_fieldDelimiter_char, i, i).toFloat();//  fee val or percent
      if (val1 > 0.00) {
        if (m_investDlg->checkBox_feeType->isChecked()) { //   fee is percent
          //have to use amountCol as amount field may not yet have been processed
          val = inBuffer.section(m_fieldDelimiter_char, m_amountColumn, m_amountColumn).toFloat();
          val1 = (val * val1 / 100);//            as percentage
        }
        txt.setNum(val1, 'f', 6);
        m_trInvestData.fee = txt;
        m_tempBuffer +=  'O' + txt + '\n';//           fee amount
      }
    }

  }//end of col loop

  m_investDlg->m_redefine->m_inBuffer = inBuffer;
  if (m_trInvestData.type != "0") { //      Don't need to do this check on checking items.
    int ret = (m_investDlg->m_redefine->checkValid(m_trInvestData.type, i18n("The quantity, Price and amount parameters in the\n\
    current transaction don't match with the action type.\n Please select another action type\n")));
    if (ret == KMessageBox::Cancel) return ret;
  }
  if ((m_trInvestData.type == "buy") || (m_trInvestData.type == "sell") || (m_trInvestData.type == "divx")) {
    m_trInvestData.brokerageAccnt = m_investDlg->m_redefine->m_accountName ;
    m_tempBuffer +=  "L[" + m_investDlg->m_redefine->m_accountName  + ']' + '\n';
  }

  if (m_brokerage) {//                                    brokerage items
    if (m_brokerBuff.isEmpty()) {//               start building data

      if (m_investDlg->m_redefine->m_accountName .isEmpty())
        m_investDlg->m_redefine->m_accountName  = getAccountName(i18n("   Brokerage or Chk. Account name:"));

      m_brokerBuff = "!Account\n";
      m_brokerBuff += 'N' + m_investDlg->m_redefine->m_accountName  + '\n';
      m_brokerBuff += "TBank\n^\n";

      m_brokerBuff += "!Type:Bank\n";
    }
    m_brokerBuff += m_tempBuffer;
    if (!memo.isEmpty())
      m_brokerBuff += 'M' + memo + '\n';
    m_brokerBuff += "^\n";
    m_brokerBuff = m_brokerBuff.remove('"');
  } else {//                                                  non-brokerage items
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
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "shrsin";
      m_trInvestData.type = "shrsin";
      return KMessageBox::Ok;
    }
  }

  //                            Needs to be before DivX
  //          because of "ReInvestorContract Buy         : ReInvested Units"
  for (it = m_reinvdivList.constBegin(); it != m_reinvdivList.constEnd(); ++it) { //Reinvdiv
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "reinvdiv";
      m_trInvestData.type = type;
      return KMessageBox::Ok;
    }
  }

  //                            Needs to be after Reinvdiv
  for (it = m_divXList.constBegin(); it != m_divXList.constEnd(); ++it) { //          DivX
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "divx";
      m_trInvestData.type = type;
      return KMessageBox::Ok;;
    }
  }

  for (it = m_brokerageList.constBegin(); it != m_brokerageList.constEnd(); ++it) { //Brokerage
    if (type.contains(*it, Qt::CaseInsensitive)) {
      m_brokerage = true;//                   these are non-investment items
      if (m_investDlg->m_redefine->m_accountName.isEmpty())
        m_investDlg->m_redefine->m_accountName  =  getAccountName(i18n(" inv  Brokerage or Chk. Account name:"));
      m_tempBuffer +=  "L[" + m_investDlg->m_redefine->m_accountName  + ']' + '\n';

      if (m_payeeColumn < 0)
        m_payeeColumn = getColumnNumber(i18n("Enter Payee or Detail Column:")) - 1;//payee column
      if (m_payeeColumn == 0) {
        KMessageBox::sorry(m_investDlg, i18n("An invalid column was entered.\n"
                                             "  Must be between 1 and %1.", m_lastColumn), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      m_trInvestData.type = '0';
      return KMessageBox::Ok;
    }
  }

  for (it = m_buyList.constBegin(); it != m_buyList.constEnd(); ++it) { //      Buy
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "buy";
      m_trInvestData.type = type;
      return KMessageBox::Ok;
    }
  }

  for (it = m_sellList.constBegin(); it != m_sellList.constEnd(); ++it) { //     Sell
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "sell";
      m_trInvestData.type = type;
      return KMessageBox::Ok;
    }
  }

  for (it = m_removeList.constBegin(); it != m_removeList.constEnd(); ++it) {//   shrsout
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "";
      m_trInvestData.type = type;
      return KMessageBox::Ok;
    }
  }
  //   no valid type found

  m_investDlg->m_redefine->m_inBuffer = m_inBuffer;
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
                          cannot be interpreted through the current "
             "date format setting of \"%2.\"" "\n\nPressing \'Continue\' will "
             "assign today's date to the transaction. Pressing \'Cancel\'' will abort "
             "the import operation. You can then restart the import and select a different "
             "date format.",
             m_trInvestData.date.toString(m_dateFormats[m_dateFormatIndex]),
             m_dateFormats[m_dateFormatIndex]), i18n("Invalid date format"));
    switch (rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = (QDate::currentDate());
        break;
      case KMessageBox::Cancel:
        m_importNow = false;//               Don't process statement
        disableInputs();
        st = MyMoneyStatement();
        return;
    }
  }

  tr.m_strMemo = m_trInvestData.memo;
  tr.m_eAction = convType;
  tr.m_amount = m_trInvestData.amount.remove('"');
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
        QString hdr = (QString::number(i + 1));
        vertHeaders += hdr;
      }
      m_investDlg->tableWidget->setVerticalHeaderLabels(vertHeaders);
      m_investDlg->tableWidget->hide();//     to ensure....
      m_investDlg->tableWidget->show();//    ....vertical header width redraws
      m_investDlg->spinBox_skip->setEnabled(false);
    } else {

      KMessageBox::information(0, i18n("The Security Name, and Date and Type columns are needed!\
                                       <center>Also, the Price, Quantity and Amount columns.</center>\
                                       <center>Please try again.</center>"));
    }
  } else {
    m_importNow = false;
    m_investDlg->checkBox_qif->setChecked(false);
  }
}

void InvestProcessing::saveAs()
{
  QStringList outFile[1] = m_inFileName .split('.');
  const KUrl& name = outFile[0].at(0) + ".qif";

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

int InvestProcessing::getColumnNumber(QString column)
{
  bool ok;
  static int ret;
  ret = KInputDialog::getInteger(i18n("Enter column number"), column, 0, 1, m_lastColumn, 1, 10, &ok);
  if (ok && ret > 0)
    return ret;
  return 0;
}

QString InvestProcessing::getAccountName(QString aName)
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
  m_investDlg->m_tableFrameHeight = m_investDlg->frame_low->frameGeometry().size().height();
  m_investDlg->m_tableFrameWidth = m_investDlg->frame_low->frameGeometry().size().width();

  int hght = 4 + (m_investDlg->tableWidget->rowHeight(m_row - 1)) * m_row;
  hght += m_investDlg->tableWidget->horizontalHeader()->height() + 2;//  frig factor plus vert. headers
  if (m_maxWidth > m_investDlg->m_tableFrameWidth)
    hght += m_investDlg->tableWidget->horizontalScrollBar()->height();//  ....and for hor. scroll bar
  if (hght > m_investDlg->m_tableFrameHeight) {
    int w = m_investDlg->tableWidget->verticalScrollBar()->width() + 2;
    m_width += w;//19
    hght = m_investDlg->m_tableFrameHeight - 12;//                        allow for border
  }
  int h = m_investDlg->tableWidget->verticalHeader()->width();
  m_width += h;

  if (m_width >= m_investDlg->m_tableFrameWidth) {
    m_width = m_investDlg->m_tableFrameWidth - 12;//                        allow for border
    hght += 1;
  }
  m_investDlg->tableWidget->setFixedHeight(hght);
  m_investDlg->tableWidget->setFixedWidth(m_width);
  m_investDlg->tableWidget->setFocus();
}

