/***************************************************************************
*                             csvprocessing.cpp                            *
*                             -----------------                            *
*  begin: Sat Jan 01 2010                                                  *
*  copyright: (C) 2010 by Allan Anderson                                   *
*  email: aganderson@ukonline.co.uk                                        *
****************************************************************************/

/***************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/

#include "csvprocessing.h"
#include "csvimporterdlg.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QtGui/QScrollBar>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QCloseEvent>
#include <QtCore/QDebug>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QPointer>
#include <QtCore/QFile>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kdeversion.h>
#include <KAction>
#include <KSharedConfig>
#include <KMessageBox>
#include <KInputDialog>
#include <KFileDialog>
#include <KStandardDirs>
#include <KLocale>
#include <KIO/NetAccess>
#include "KAboutApplicationDialog"
#include <KAboutData>

// ----------------------------------------------------------------------------
// Project Headers

#include "convdate.h"
#include "investmentdlg.h"
#include "mymoneystatement.h"
#include "mymoneyaccount.h"
#include "csvutil.h"

CsvProcessing::CsvProcessing()
{
  m_importNow = false;

  m_dateFormatIndex = 0;
  m_endLine = 0;
  m_fieldDelimiterIndex = 0;
  m_textDelimiterIndex = 0;
  m_endColumn = 0;
  m_flagCol = -1;
  m_row = 0;
  m_startLine = 0;

  m_parseline = new ParseLine;
}

CsvProcessing::~CsvProcessing()
{
  delete m_parseline;
}

void CsvProcessing::init()
{
  m_dateFormats << "yyyy/MM/dd" << "MM/dd/yyyy" << "dd/MM/yyyy";
  m_endColumn = MAXCOL;
  clearSelectedFlags();
  readSettings();

  m_dateFormatIndex = m_csvDialog->comboBox_dateFormat->currentIndex();
  m_date = m_dateFormats[m_dateFormatIndex];
  m_csvDialog->m_convertDate->setDateFormatIndex(m_dateFormatIndex);

  findCodecs();
  setCodecList(m_codecs);
}

void CsvProcessing::fileDialog()
{
  KSharedConfigPtr config =
    KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
  KConfigGroup profileGroup(config, "Profile");
  m_debitFlag = profileGroup.readEntry("DebitFlag", QString().toInt());

  m_endLine = 0;
  m_flagCol = -1;
  int posn;
  if (m_csvPath.isEmpty()) {
    m_csvPath = "~/";
  }
  QPointer<KFileDialog> dialog =
    new KFileDialog(KUrl("kfiledialog:///kmymoney-csvbank"),
                    i18n("*.csv *.PRN *.txt | CSV Files\n *|All files"),
                    0);
  dialog->setMode(KFile::File | KFile::ExistingOnly);
  if (dialog->exec() == QDialog::Accepted) {
    m_url = dialog->selectedUrl();
  }
  delete dialog;

  if (m_url.isEmpty())
    return;
  m_inFileName.clear();

  if (!KIO::NetAccess::download(m_url, m_inFileName, 0)) {
    KMessageBox::detailedError(0,
                               i18n("Error while loading file '%1'.", m_url.prettyUrl()),
                               KIO::NetAccess::lastErrorString(),
                               i18n("File access error"));
    return;
  }

  if (m_inFileName.isEmpty()) return;
  m_importNow = false;//                    Avoid attempting date formatting on headers
  clearComboBoxText();//                    to clear any '*' in memo combo text
  for (int i = 0; i < MAXCOL; i++)
    if (m_csvDialog->columnType(i) == "memo") {
      m_csvDialog->clearColumnType(i);   //   ensure no memo entries remain
    }

  readFile(m_inFileName, 0);
  m_csvPath = m_inFileName;
  posn = m_csvPath.lastIndexOf("/");
  m_csvPath.truncate(posn + 1);   //   keep last "/"

  QString str = "$HOME/" + m_csvPath.section('/', 3);
  profileGroup.writeEntry("CsvDirectory", str);
  profileGroup.config()->sync();//save selected path
  enableInputs();

  //The following two items do not *Require* an entry so old values must be cleared.
  m_trData.number.clear();// this needs to be cleared or gets added to next transaction
  m_trData.memo.clear();//   this too, as neither might be overwritten by new data
}

void CsvProcessing::enableInputs()
{
  m_csvDialog->checkBox_qif->setEnabled(true);
  m_csvDialog->checkBox_qif->setChecked(false);
  m_csvDialog->spinBox_skip->setEnabled(true);
  m_csvDialog->comboBox_numberCol->setEnabled(true);
  m_csvDialog->comboBox_dateCol->setEnabled(true);
  m_csvDialog->comboBox_payeeCol->setEnabled(true);
  m_csvDialog->comboBox_memoCol->setEnabled(true);
  m_csvDialog->button_clear->setEnabled(true);
  m_csvDialog->spinBox_skipLast->setEnabled(true);
  m_csvDialog->button_saveAs->setEnabled(true);

  if (m_csvDialog->radio_amount->isChecked()) {
    m_csvDialog->comboBox_amountCol->setEnabled(true);
    m_csvDialog->comboBox_debitCol->setEnabled(false);
    m_csvDialog->comboBox_creditCol->setEnabled(false);
  } else {
    m_csvDialog->comboBox_amountCol->setEnabled(false);
    m_csvDialog->comboBox_debitCol->setEnabled(true);
    m_csvDialog->comboBox_creditCol->setEnabled(true);
  }
}

void CsvProcessing::clearColumnsSelected()
{
  m_csvDialog->clearPreviousColumn();
  clearSelectedFlags();
  clearColumnNumbers();
  clearComboBoxText();
  clearColumnNumbers();
}

void CsvProcessing::clearSelectedFlags()
{
  for (int i = 0; i < MAXCOL; i++)
    m_csvDialog->clearColumnType(i);   //   set to all empty

  m_csvDialog->setDateSelected(false);
  m_csvDialog->setPayeeSelected(false);
  m_csvDialog->setAmountSelected(false);
  m_csvDialog->setDebitSelected(false);
  m_csvDialog->setCreditSelected(false);
  m_csvDialog->setMemoSelected(false);
  m_csvDialog->setNumberSelected(false);
  m_csvDialog->radio_amount->setEnabled(true);
  m_csvDialog->radio_debCred->setEnabled(true);
}

void CsvProcessing::clearColumnNumbers()
{
  m_csvDialog->comboBox_dateCol->setCurrentIndex(-1);
  m_csvDialog->comboBox_payeeCol->setCurrentIndex(-1);
  m_csvDialog->comboBox_memoCol->setCurrentIndex(-1);
  m_csvDialog->comboBox_numberCol->setCurrentIndex(-1);
  m_csvDialog->comboBox_amountCol->setCurrentIndex(-1);
  m_csvDialog->comboBox_debitCol->setCurrentIndex(-1);
  m_csvDialog->comboBox_creditCol->setCurrentIndex(-1);
}

void CsvProcessing::clearComboBoxText()
{
  for (int i = 0; i < MAXCOL; i++) {
    m_csvDialog->comboBox_memoCol->setItemText(i, QString().setNum(i + 1));
  }
}

void CsvProcessing::clearColumnTypes()
{
  for (int i = 0; i < MAXCOL; i++) {
    m_csvDialog->clearColumnType(i);
  }
}

void CsvProcessing::encodingChanged()
{
  if (!m_inFileName.isEmpty())
    readFile(m_inFileName, 0);
}

void CsvProcessing::findCodecs()
{
  QMap<QString, QTextCodec *> codecMap;
  QRegExp iso8859RegExp("ISO[- ]8859-([0-9]+).*");

  foreach (int mib, QTextCodec::availableMibs()) {
    QTextCodec *codec = QTextCodec::codecForMib(mib);

    QString sortKey = codec->name().toUpper();
    int rank;

    if (sortKey.startsWith("UTF-8")) {        // krazy:exclude=strings
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


void CsvProcessing::delimiterChanged()
{
  if (!m_inFileName.isEmpty())
    readFile(m_inFileName, 0);
}

void CsvProcessing::readFile(const QString& fname, int skipLines)
{
  MyMoneyStatement st = MyMoneyStatement();
  if (!fname.isEmpty()) {
    m_inFileName = fname;
  }
  m_startLine = skipLines;

  QFile  m_inFile(m_inFileName);
  m_inFile.open(QIODevice::ReadOnly | QIODevice::Text);

  QTextStream inStream(&m_inFile);

  int encodeIndex = m_csvDialog->comboBox_encoding->currentIndex();
  QTextCodec* codec = QTextCodec::codecForMib(encodeIndex);
  inStream.setCodec(codec);

  m_csvDialog->tableWidget->clear();// including vert headers
  m_inBuffer.clear();
  m_outBuffer.clear();

  m_qifBuffer = "!Type:Bank\n";
  m_row = 0;
  m_csvDialog->setMaxColumnCount(0);

  m_fieldDelimiterIndex = m_csvDialog->comboBox_fieldDelim->currentIndex();
  m_textDelimiterCharacter = m_csvDialog->comboBox_textDelimiter->currentText();

  int lineCount = -1;

  QString Buffer = inStream.readAll();
  bool inQuotes = false;
  int count = Buffer.count();
  QString::const_iterator constIterator;

  for (constIterator = Buffer.constBegin(); constIterator != Buffer.constEnd();
       ++constIterator) {
    QString chr = (*constIterator);
    count -= 1;
    if (chr == m_textDelimiterCharacter) {
      m_outBuffer += chr;
      if (inQuotes == true) {
        inQuotes = false;
      } else {
        inQuotes = true;
      }
      continue;
    } else if (chr == "\n") {
      if (inQuotes == true) {//                embedded '\n'
        chr = '~';//                           substitute for '\n'
        m_outBuffer += chr;
        if (count > 0)//                       more chars yet               
          continue;
      }
      //                                       true EOL
      if (m_outBuffer.isEmpty()) {
        continue;
      }
      lineCount ++;
      if (lineCount < m_startLine) {//         not yet reached first wanted line
        m_outBuffer.clear();
        continue;
      }
//      m_outBuffer += chr;// was adding a trailing '\n'
      m_inBuffer = m_outBuffer;
      m_outBuffer.clear();

      //  if first pass or if not at last line, proceed
      if ((!m_endLine == 0) && (lineCount >= m_endLine)) {// m_endLine is set from UI after first pass
        m_csvDialog->spinBox_skipLast->setValue(lineCount - 1); //  else break
        break;
      }
    }//                                        end of EOL detected loop
    else {
      m_outBuffer += chr;
      if (count > 0) {//                       more chars yet
        continue;
      }//                                      else eoFile = true;
    }

    if (!m_outBuffer.isEmpty()) {
      m_inBuffer = m_outBuffer;
    }
    displayLine(m_inBuffer);
    if (m_importNow) { //                      user now ready to continue
      int ret = (processQifLine(m_inBuffer));// parse a line
      if (ret == KMessageBox::Ok) {
        csvImportTransaction(st);
      } else
        m_importNow = false;

    }//                                        reached end of data
  }
  updateScreen();//                            discard unwanted header lines

  m_csvDialog->label_skip->setEnabled(true);
  m_csvDialog->spinBox_skip->setEnabled(true);
  m_csvDialog->spinBox_skipLast->setValue(lineCount + 1);
  m_endColumn = m_csvDialog->maxColumnCount();

  if (m_importNow) {
    emit statementReady(st);
    m_importNow = false;
  }
  m_csvDialog->checkBox_qif->setChecked(false);
  m_inFile.close();
}

void CsvProcessing::displayLine(const QString& data)
{
  if (m_importNow) {
    if (m_csvDialog->radio_amount->isChecked()) {
      m_csvDialog->setAmountColumn(m_csvDialog->comboBox_amountCol->currentIndex());
      m_csvDialog->setDebitColumn(-1);
      m_csvDialog->setCreditColumn(-1);
    } else {
      m_csvDialog->setAmountColumn(-1);
      m_csvDialog->setDebitColumn(m_csvDialog->comboBox_debitCol->currentIndex());
      m_csvDialog->setCreditColumn(m_csvDialog->comboBox_creditCol->currentIndex());
    }
  }
  int col = 0;

  m_parseline->setFieldDelimiterIndex(m_csvDialog->comboBox_fieldDelim->currentIndex());
  m_fieldDelimiterCharacter = m_parseline->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_parseline->setTextDelimiterCharacter(m_csvDialog->comboBox_textDelimiter->currentText());
  m_textDelimiterCharacter = m_parseline->textDelimiterCharacter();

  m_columnList = m_parseline->parseLine(data);//                 split data into fields
  int columnCount = m_columnList.count();
  if (columnCount > m_csvDialog->maxColumnCount())
    m_csvDialog->setMaxColumnCount(columnCount);//               find maximum column count
  else
    columnCount = m_csvDialog->maxColumnCount();
  m_csvDialog->tableWidget->setColumnCount(columnCount);
  m_inBuffer.clear();
  QStringList::const_iterator constIterator;
  QString txt;
  int width = 0;
  for (constIterator = m_columnList.constBegin(); constIterator != m_columnList.constEnd();
       ++constIterator) {
    txt = (*constIterator);
    QTableWidgetItem *item = new QTableWidgetItem;//             new item for UI
    item->setText(txt);
    m_csvDialog->tableWidget->setRowCount(m_row + 1);
    m_csvDialog->tableWidget->setItem(m_row, col, item);//       add items to UI here
    m_inBuffer += txt + m_fieldDelimiterCharacter;
    width += m_csvDialog->tableWidget->columnWidth(col);
    col ++;
  }
  
  //  if last char. of last column added to UI (txt string) is not '"', ie an unterminated string
  //  remove the unwanted trailing m_fieldDelimiterCharacter
  if (!txt.endsWith('"')) {
   m_inBuffer = m_inBuffer.remove(-1,1);
  }
  m_row += 1;
}

void CsvProcessing::csvImportTransaction(MyMoneyStatement& st)
{
  MyMoneyStatement::Transaction tr;
  QString tmp;
  QString payee = m_trData.payee;//                              extractLine('P')
  // Process transaction data

  char result[100];

  int rand = qrand();
  sprintf(result, "%d", rand);
  tr.m_strBankID = result;
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
        return ;
    }
  }
  tr.m_amount = m_trData.amount;
  tr.m_shares = m_trData.amount;

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

int CsvProcessing::processQifLine(QString& iBuff)//   parse input line
{
  int neededFieldsCount = 0;///                       ensure essential fields are present
  QString memo;
  QString txt;
  iBuff = iBuff.remove(m_textDelimiterCharacter);
  memo.clear();//                                     memo & number may not have been used
  m_trData.number.clear();//                          .. so need to clear prior contents
  for (int i = 0; i < m_endColumn; i++) {   //        check each column
    if (m_csvDialog->columnType(i) == "number") {
      txt = m_columnList[i];
      m_trData.number = txt;
      m_qifBuffer = m_qifBuffer + 'N' + txt + '\n';// Number column
    }

    else if (m_csvDialog->columnType(i) == "date") {
      neededFieldsCount +=1;
      txt = m_columnList[i];
      txt = txt.remove(m_textDelimiterCharacter);//   "16/09/2009
      QDate dat = m_csvDialog->m_convertDate->convertDate(txt);// Date column
      if (dat == QDate()) {
        qDebug() << i18n("date ERROR");

        KMessageBox::sorry(m_csvDialog, i18n("<center>An invalid date has been detected during import.</center>"
                                             "<center><b>%1</b></center>"
                                             "Please check that you have set the correct date format."
                                             , txt), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      QString qifDate = dat.toString(m_dateFormats[m_dateFormatIndex]);
      m_qifBuffer = m_qifBuffer + 'D' + qifDate + '\n';
      m_trData.date = dat;
    }

    else if (m_csvDialog->columnType(i) == "payee") {
      neededFieldsCount +=1;
      txt = m_columnList[i];
      txt.remove('~');//                              replace NL which was substituted
      txt = txt.remove('\'');
      m_trData.payee = txt;
      m_qifBuffer = m_qifBuffer + 'P' + txt + '\n';// Detail column
    }

    else if (m_csvDialog->columnType(i) == "amount") { // Is this Amount column
      neededFieldsCount +=1;
      if (m_flagCol == -1) { //                        it's a new file
        switch (m_debitFlag) { //                     Flag if amount is debit or credit
          case -1://                                  Ignore flag
            m_flagCol = 0;//                          ...and continue
            break;
          case  0://                                  Ask for column no.of flag
            m_flagCol = columnNumber(i18n("Enter debit flag column number"));
            if (m_flagCol == 0) { //                   0 means Cancel was pressed
              return KMessageBox::Cancel;//           ... so exit
            }
            break;
          default : m_flagCol = m_debitFlag;//        Contains flag/column no.
        }
      }
      if ((m_flagCol < 0) || (m_flagCol > m_endColumn)) { // shouldn't get here
        KMessageBox::sorry(0, i18n("An invalid column was entered.\n"
                                   "Must be between 1 and %1.", m_endColumn), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      QString flag;//                                 m_flagCol == valid column (or zero)
      if (m_flagCol > 0) {
        flag = m_columnList[m_flagCol - 1];//         indicates if amount is debit or credit
      }//                                             if flagCol == 0, flag is empty
      txt = m_columnList[i];
      if ((m_csvDialog->amountColumn() == i) &&
          (((txt.contains("("))) || (flag.startsWith('A')))) {//  "(" or "Af" = debit
        txt = txt.remove(QRegExp("[()]"));
        txt = '-' + txt;  //                          Mark as -ve
      } else if (m_csvDialog->debitColumn() == i) {
        txt = '-' + txt;  //                          Mark as -ve
      }
      m_trData.amount = txt;
      m_qifBuffer = m_qifBuffer + 'T' + txt + '\n';
    }

    else if ((m_csvDialog->columnType(i) == "debit") || (m_csvDialog->columnType(i) == "credit")) {//  Credit or debit?
      neededFieldsCount +=1;
      txt = m_columnList[i];
      if (!txt.isEmpty()) {
        if (m_csvDialog->debitColumn() == i)
          txt = '-' + txt;//  Mark as -ve
        if ((m_csvDialog->debitColumn() == i) || (m_csvDialog->creditColumn() == i)) {
          m_trData.amount = txt;
          m_qifBuffer = m_qifBuffer + 'T' + txt + '\n';
        }
      }
    }

    else if (m_csvDialog->columnType(i) == "memo") { // could be more than one
      txt = m_columnList[i];
      txt.replace('~', "\n");//                       replace NL which was substituted
      if (!memo.isEmpty())
        memo += '\n';//                               separator for multiple memos
      memo += txt;//                                  next memo
    }//end of memo field
  }//end of col loop

  m_trData.memo = memo;
  m_qifBuffer = m_qifBuffer + 'M' + memo + '\n' + "^\n";
  if (neededFieldsCount > 2) {///
    return KMessageBox::Ok;///
  } else {///
    KMessageBox::sorry(0, i18n("<center>The columns selected are invalid.\n</center>"
                                   "There must be an amount or debit and credit fields, plus date and payee fields."), i18n("CSV import"));
    return KMessageBox::Cancel;///
  }
  return KMessageBox::Ok;
}

void CsvProcessing::importClicked(bool checked)
{
  // The following two fields are optional so must be cleared
  // ...of any prior choices in UI
  m_csvDialog->comboBox_memoCol->setCurrentIndex(-1);
  m_csvDialog->comboBox_numberCol->setCurrentIndex(-1);

  if (checked) {
    if ((m_csvDialog->dateSelected()) && (m_csvDialog->payeeSelected()) &&
        ((m_csvDialog->amountSelected() || (m_csvDialog->debitSelected() && m_csvDialog->creditSelected())))) {
      m_importNow = true; //                          all necessary data is present
      m_csvDialog->checkBox_qif->setChecked(true);
      int skp = m_csvDialog->spinBox_skip->value() - 1;

      readFile(m_inFileName, skp);   //               skip all headers
      //--- create the (revised) vertical (row) headers ---
      QStringList vertHeaders;
      for (int i = skp; i < m_csvDialog->tableWidget->rowCount() + skp; i++) {
        QString hdr = (QString::number(i + 1));
        vertHeaders += hdr;
      }
      //  verticalHeader()->width() varies with its content so....
      m_csvDialog->tableWidget->setVerticalHeaderLabels(vertHeaders);
      m_csvDialog->tableWidget->hide();//             to ensure....
      m_csvDialog->tableWidget->show();//             ..vertical header width redraws
    } else {
      KMessageBox::information(0, i18n("<center>An Amount-type column, and Date and Payee columns are needed!</center> <center>Please try again.</center>"));
    }
  } else {
    m_importNow = false;
  }
  m_csvDialog->checkBox_qif->setChecked(false);
}

void CsvProcessing::readSettings()
{
  int tmp;
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup profileGroup(config, "Profile");
  m_dateFormatIndex = profileGroup.readEntry("DateFormat", QString()).toInt();
  m_csvDialog->comboBox_dateFormat->setCurrentIndex(m_dateFormatIndex);
  tmp = profileGroup.readEntry("StartLine", QString()).toInt();
  m_csvDialog->spinBox_skip->setValue(tmp + 1);

  int encodeIndex = profileGroup.readEntry("Encoding", QString()).toInt();
  m_csvDialog->comboBox_encoding->setCurrentIndex(encodeIndex);

  m_fieldDelimiterIndex = profileGroup.readEntry("FieldDelimiter", QString()).toInt();
  m_csvDialog->comboBox_fieldDelim->setCurrentIndex(m_fieldDelimiterIndex);

  m_csvPath = profileGroup.readEntry("CsvDirectory", QString());

  m_debitFlag = profileGroup.readEntry("DebitFlag", QString().toInt());

  KConfigGroup columnsGroup(config, "Columns");

  if (columnsGroup.exists()) {
    tmp = columnsGroup.readEntry("DateCol", QString()).toInt();
    m_csvDialog->comboBox_dateCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_dateCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("PayeeCol", QString()).toInt();
    m_csvDialog->comboBox_payeeCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_payeeCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("AmountCol", QString()).toInt();
    if (tmp >= 0) { //                            If amount previously selected, set check radio_amount
      m_csvDialog->radio_amount->setChecked(true);
      m_csvDialog->label_amount->setEnabled(true);
      m_csvDialog->label_credits->setEnabled(false);
      m_csvDialog->label_debits->setEnabled(false);
    } else {//                                   ....else set check radio_debCred to clear amount col
      m_csvDialog->radio_debCred->setChecked(true);
      m_csvDialog->label_credits->setEnabled(true);
      m_csvDialog->label_debits->setEnabled(true);
      m_csvDialog->label_amount->setEnabled(false);
    }
    m_csvDialog->comboBox_amountCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_amountCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("DebitCol", QString()).toInt();
    m_csvDialog->comboBox_debitCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_debitCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("CreditCol", QString()).toInt();
    m_csvDialog->comboBox_creditCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_creditCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("NumberCol", QString()).toInt();
    m_csvDialog->comboBox_numberCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_numberCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("MemoCol", QString()).toInt();
    m_csvDialog->comboBox_memoCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_memoCol->setCurrentIndex(tmp);
  } else {
    m_csvDialog->comboBox_dateCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_payeeCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_amountCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_debitCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_creditCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_numberCol->setCurrentIndex(-1);
    m_csvDialog->comboBox_memoCol->setCurrentIndex(-1);
  }
}

void CsvProcessing::saveAs()
{
  QStringList outFile = m_inFileName.split('.');
  const KUrl& name = (outFile.isEmpty() ? "CsvProcessing" : outFile[0]) + ".qif";

  QString outFileName = KFileDialog::getSaveFileName(name, "*.qif | QIF Files", 0, i18n("Save QIF")
#if KDE_IS_VERSION(4,4,0)
                        , KFileDialog::ConfirmOverwrite
#endif
                                                    );

  QFile oFile(outFileName);
  oFile.open(QIODevice::WriteOnly);
  QTextStream out(&oFile);
  out << m_qifBuffer;// output qif file
  oFile.close();
}

void CsvProcessing::setCodecList(const QList<QTextCodec *> &list)
{
  m_csvDialog->comboBox_encoding->clear();
  foreach (QTextCodec * codec, list)
  m_csvDialog->comboBox_encoding->addItem(codec->name(), codec->mibEnum());
}

void CsvProcessing::startLineChanged()
{
  int val = m_csvDialog->spinBox_skip->value();
  if (val < 1) {
    return;
  }
  m_startLine = val - 1;
}

void CsvProcessing::endLineChanged()
{
  m_endLine = m_csvDialog->spinBox_skipLast->value() ;
}

void CsvProcessing::dateFormatSelected(int dF)
{
  if (dF == -1) return;
  m_dateFormatIndex = dF;
  m_date = m_dateFormats[m_dateFormatIndex];
}

void CsvProcessing::updateScreen()
{
  m_csvDialog->tableWidget->setRowCount(m_row);
  m_csvDialog->tableWidget->setFocus();
}

QString CsvProcessing::csvPath()
{
  return m_csvPath;
}

QString CsvProcessing::inFileName()
{
  return m_inFileName;
}

int CsvProcessing::fieldDelimiterIndex()
{
  return m_fieldDelimiterIndex;
}

int CsvProcessing::endColumn()
{
  return m_endColumn;
}

int CsvProcessing::columnNumber(const QString& msg)
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
