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
#include "investprocessing.h"

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
#include <kvbox.h>
#include <KAction>
#include <KSharedConfig>
#include <KMessageBox>
#include <KInputDialog>
#include <KFileDialog>
#include <KFileWidget>
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
  m_showEmptyCheckBox = true;

  m_dateFormatIndex = 0;
  m_endLine = 0;
  m_fieldDelimiterIndex = 0;
  m_textDelimiterIndex = 0;
  m_endColumn = 0;
  m_flagCol = -1;
  m_row = 0;
  m_startLine = 0;

  m_inFileName.clear();
}

CsvProcessing::~CsvProcessing()
{
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
  m_csvDialog->button_import->setEnabled(false);
  m_csvDialog->tabWidget_Main->setCurrentIndex(0);

  findCodecs();//                             returns m_codecs = codecMap.values();
}

void CsvProcessing::fileDialog()
{
  if(m_csvDialog->m_fileType != "Banking") return;
  KSharedConfigPtr config =
    KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
  KConfigGroup profileGroup(config, "Profile");

  //  The "DebitFlag" setting is used to indicate whether or not to allow the user,
  //  via a dialog, to specify a column which contains a flag to indicate if the
  //  amount field is a debit ('a' or 'af'), a credit ('bij') (ING - Netherlands),
  //   or ignore ('-1').

  m_debitFlag = profileGroup.readEntry("DebitFlag", QString().toInt());
  m_csvDialog->comboBox_decimalSymbol->setEnabled(true);

  m_endLine = 0;
  m_flagCol = -1;
  m_accept = false;
  m_csvDialog->m_decimalSymbolChanged = false;
  int posn;
  if(m_csvPath.isEmpty()) {
    m_csvPath = "~/";
  }

  QPointer<KFileDialog> dialog =
    new KFileDialog(KUrl("kfiledialog:///kmymoney-csvbank"),
                    i18n("*.csv *.PRN *.txt | CSV Files\n *|All files"),
                    0);
//  Add encoding selection to FileDialog
  KHBox* encodeBox = new KHBox();
  m_comboBoxEncode = new KComboBox(encodeBox);
  m_comboBoxEncode->setCurrentIndex(m_encodeIndex);
  setCodecList(m_codecs);
  connect(m_comboBoxEncode, SIGNAL(activated(int)), this, SLOT(encodingChanged(int)));

  dialog->fileWidget()->setCustomWidget("Encoding", m_comboBoxEncode);
  m_comboBoxEncode->setCurrentIndex(m_encodeIndex);
  dialog->setMode(KFile::File | KFile::ExistingOnly);
  if(dialog->exec() == QDialog::Accepted) {
    m_url = dialog->selectedUrl();
  }
  delete dialog;

  if(m_url.isEmpty())
    return;
  m_inFileName.clear();

  if(!KIO::NetAccess::download(m_url, m_inFileName, 0)) {
    KMessageBox::detailedError(0,
                               i18n("Error while loading file '%1'.", m_url.prettyUrl()),
                               KIO::NetAccess::lastErrorString(),
                               i18n("File access error"));
    return;
  }
  if(m_inFileName.isEmpty()) return;
  m_importNow = false;//                       Avoid attempting date formatting on headers
  clearComboBoxText();//                       to clear any '*' in memo combo text

  for(int i = 0; i < MAXCOL; i++)
    if(m_csvDialog->columnType(i) == "memo") {
      m_csvDialog->clearColumnType(i);   //    ensure no memo entries remain
    }

  //  set large table height to ensure resizing sees all lines in new file

  QRect rect = m_csvDialog->tableWidget->geometry();
  rect.setHeight(9999);
  m_csvDialog->tableWidget->setGeometry(rect);

  readFile(m_inFileName, 0);
  m_csvPath = m_inFileName;
  posn = m_csvPath.lastIndexOf("/");
  m_csvPath.truncate(posn + 1);   //   keep last "/"

  QString str = "$HOME/" + m_csvPath.section('/', 3);
  profileGroup.writeEntry("CsvDirectory", str);//          save selected path
  profileGroup.writeEntry("Encoding", m_encodeIndex);//    ..and encoding
  profileGroup.config()->sync();
  enableInputs();

  //The following two items do not *Require* an entry so old values must be cleared.
  m_trData.number.clear();// this needs to be cleared or gets added to next transaction
  m_trData.memo.clear();//   this too, as neither might be overwritten by new data
}

void CsvProcessing::enableInputs()
{
  m_csvDialog->button_import->setEnabled(true);
  m_csvDialog->spinBox_skip->setEnabled(true);
  m_csvDialog->comboBoxBnk_numberCol->setEnabled(true);
  m_csvDialog->comboBoxBnk_dateCol->setEnabled(true);
  m_csvDialog->comboBoxBnk_payeeCol->setEnabled(true);
  m_csvDialog->comboBoxBnk_memoCol->setEnabled(true);
  m_csvDialog->button_clear->setEnabled(true);
  m_csvDialog->spinBox_skipToLast->setEnabled(true);
  m_csvDialog->button_saveAs->setEnabled(true);
  m_csvDialog->comboBox_fieldDelimiter->setEnabled(true);

  if(m_csvDialog->radioBnk_amount->isChecked()) {
    m_csvDialog->comboBoxBnk_amountCol->setEnabled(true);
    m_csvDialog->comboBoxBnk_debitCol->setEnabled(false);
    m_csvDialog->comboBoxBnk_creditCol->setEnabled(false);
  } else {
    m_csvDialog->comboBoxBnk_amountCol->setEnabled(false);
    m_csvDialog->comboBoxBnk_debitCol->setEnabled(true);
    m_csvDialog->comboBoxBnk_creditCol->setEnabled(true);
  }
}

void CsvProcessing::clearColumnsSelected()
{
  if(m_csvDialog->m_fileType == "Banking") {
    m_csvDialog->clearPreviousColumn();
    clearSelectedFlags();
    clearColumnNumbers();
    clearComboBoxText();
  } else if(m_csvDialog->m_fileType == "Invest") {
    m_csvDialog->m_investProcessing->clearSelectedFlags();
    m_csvDialog->m_investProcessing->clearColumnNumbers();
    m_csvDialog->m_investProcessing->clearComboBoxText();
  }
}

void CsvProcessing::clearSelectedFlags()
{
  for(int i = 0; i < MAXCOL; i++)
    m_csvDialog->clearColumnType(i);   //   set to all empty

  m_csvDialog->setDateSelected(false);
  m_csvDialog->setPayeeSelected(false);
  m_csvDialog->setAmountSelected(false);
  m_csvDialog->setDebitSelected(false);
  m_csvDialog->setCreditSelected(false);
  m_csvDialog->setMemoSelected(false);
  m_csvDialog->setNumberSelected(false);
  m_csvDialog->radioBnk_amount->setEnabled(true);
  m_csvDialog->radioBnk_debCred->setEnabled(true);
}

void CsvProcessing::clearColumnNumbers()
{
  m_csvDialog->comboBoxBnk_dateCol->setCurrentIndex(-1);
  m_csvDialog->comboBoxBnk_payeeCol->setCurrentIndex(-1);
  m_csvDialog->comboBoxBnk_memoCol->setCurrentIndex(-1);
  m_csvDialog->comboBoxBnk_numberCol->setCurrentIndex(-1);
  m_csvDialog->comboBoxBnk_amountCol->setCurrentIndex(-1);
  m_csvDialog->comboBoxBnk_debitCol->setCurrentIndex(-1);
  m_csvDialog->comboBoxBnk_creditCol->setCurrentIndex(-1);
}

void CsvProcessing::clearComboBoxText()
{
  for(int i = 0; i < MAXCOL; i++) {
    m_csvDialog->comboBoxBnk_memoCol->setItemText(i, QString().setNum(i + 1));
  }
}

void CsvProcessing::clearColumnTypes()
{
  for(int i = 0; i < MAXCOL; i++) {
    m_csvDialog->clearColumnType(i);
  }
}

void CsvProcessing::encodingChanged(int index)
{
  m_encodeIndex = index;
  if(!m_inFileName.isEmpty())
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

    if(sortKey.startsWith("UTF-8")) {         // krazy:exclude=strings
      rank = 1;
    } else if(sortKey.startsWith("UTF-16")) {        // krazy:exclude=strings
      rank = 2;
    } else if(iso8859RegExp.exactMatch(sortKey)) {
      if(iso8859RegExp.cap(1).size() == 1)
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
  if(m_csvDialog->m_fileType != "Banking") return;
  if(!m_inFileName.isEmpty())
    readFile(m_inFileName, 0);
}

void CsvProcessing::readFile(const QString& fname, int skipLines)
{
  MyMoneyStatement st = MyMoneyStatement();
  if(!fname.isEmpty()) {
    m_inFileName = fname;
  }
  m_startLine = skipLines;
  m_csvDialog->tableWidget->clear();//         including vert headers
  m_inBuffer.clear();
  m_outBuffer.clear();

  m_qifBuffer = "!Type:Bank\n";
  m_row = 0;
  m_csvDialog->setMaxColumnCount(0);

  m_fieldDelimiterIndex = m_csvDialog->comboBox_fieldDelimiter->currentIndex();
  m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_textDelimiterIndex = m_csvDialog->comboBox_textDelimiter->currentIndex();
  m_parse->setTextDelimiterIndex(m_textDelimiterIndex);
  m_textDelimiterCharacter = m_parse->textDelimiterCharacter(m_textDelimiterIndex);

  QFile  m_inFile(m_inFileName);
  m_inFile.open(QIODevice::ReadOnly | QIODevice::Text);

  QTextStream inStream(&m_inFile);
  QTextCodec* codec = QTextCodec::codecForMib(m_encodeIndex);
  inStream.setCodec(codec);

  QString buf = inStream.readAll();

  //  Parse the buffer

  QStringList lineList = m_parse->parseFile(buf, m_startLine, m_endLine);
  m_csvDialog->spinBox_skipToLast->setValue(m_parse->lastLine());
  m_csvDialog->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  m_screenUpdated = false;

  //  Display the buffer

  for(int i = 0; i < lineList.count(); i++) {
    m_inBuffer = lineList[i];

    displayLine(m_inBuffer);

    if(m_importNow) {  //                        user now ready to continue
      int ret = (processQifLine(m_inBuffer));// parse a line
      if(ret == KMessageBox::Ok) {
        csvImportTransaction(st);
      } else
        m_importNow = false;
    }
  }//                                            reached end of buffer

  //  Adjust table size (drop header lines)

  updateScreen();//
  m_csvDialog->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  m_csvDialog->labelSet_skip->setEnabled(true);
  m_csvDialog->spinBox_skip->setEnabled(true);
  m_endColumn = m_csvDialog->maxColumnCount();

  //  Export statement

  if(m_importNow) {
    emit statementReady(st);
    m_screenUpdated = true;
    m_importNow = false;
  }
  m_inFile.close();
}

void CsvProcessing::displayLine(const QString& data)
{
  if(m_importNow) {
    if(m_csvDialog->radioBnk_amount->isChecked()) {
      m_csvDialog->setAmountColumn(m_csvDialog->comboBoxBnk_amountCol->currentIndex());
      m_csvDialog->setDebitColumn(-1);
      m_csvDialog->setCreditColumn(-1);
    } else {
      m_csvDialog->setAmountColumn(-1);
      m_csvDialog->setDebitColumn(m_csvDialog->comboBoxBnk_debitCol->currentIndex());
      m_csvDialog->setCreditColumn(m_csvDialog->comboBoxBnk_creditCol->currentIndex());
    }
  }
  int col = 0;

  m_parse->setFieldDelimiterIndex(m_csvDialog->comboBox_fieldDelimiter->currentIndex());
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_parse->setTextDelimiterIndex(m_csvDialog->comboBox_textDelimiter->currentIndex());
  m_textDelimiterCharacter = m_parse->textDelimiterCharacter(m_textDelimiterIndex);

  m_columnList = m_parse->parseLine(data);//                 split data into fields
  int columnCount = m_columnList.count();
  if(columnCount > m_csvDialog->maxColumnCount())
    m_csvDialog->setMaxColumnCount(columnCount);//               find maximum column count
  else
    columnCount = m_csvDialog->maxColumnCount();
  m_csvDialog->tableWidget->setColumnCount(columnCount);
  m_inBuffer.clear();
  QStringList::const_iterator constIterator;
  QString txt;

  for(constIterator = m_columnList.constBegin(); constIterator != m_columnList.constEnd();
      ++constIterator) {
    txt = (*constIterator);

    QTableWidgetItem *item = new QTableWidgetItem;//             new item for UI
    item->setText(txt);
    m_csvDialog->tableWidget->setRowCount(m_row + 1);
    m_csvDialog->tableWidget->setItem(m_row, col, item);//       add items to UI here
    m_csvDialog->tableWidget->resizeColumnToContents(col);
    m_inBuffer += txt + m_fieldDelimiterCharacter;
    col ++;
  }

  //  if last char. of last column added to UI (txt string) is not '"', ie an unterminated string
  //  remove the unwanted trailing m_fieldDelimiterCharacter
  if(!txt.endsWith('"')) {
    m_inBuffer = m_inBuffer.remove(-1, 1);
  }
  ++m_row;
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
  if(!tr.m_datePosted.isValid()) {
    int rc = KMessageBox::warningContinueCancel(0, i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
             "date format setting of \"%2.\"" "\n\nPressing \'Continue\' will "
             "assign today's date to the transaction. Pressing \'Cancel\'' will abort "
             "the import operation. You can then restart the import and select a different "
             "date format.", m_trData.date.toString(m_date), m_dateFormats[m_dateFormatIndex]), i18n("Invalid date format"));
    switch(rc) {
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

  if(!payee.isEmpty()) {
    tr.m_strPayee = m_trData.payee;
  }

  tr.m_strMemo = m_trData.memo;
  // Add the transaction to the statement
  st.m_listTransactions += tr;
  if((st.m_listTransactions.count()) > 0) {
    statements += st;// this not used
    qDebug("Statement with %d transactions ready.",
           st.m_listTransactions.count());
  }
  // Now to import the statements
  return;
}

int CsvProcessing::processQifLine(QString& iBuff)//   parse input line
{
  if(m_columnList.count() < m_endColumn) {
    if(!m_accept) {
      QString row = QString::number(m_row);
      int ret = KMessageBox::questionYesNoCancel(m_csvDialog, i18n("<center>Row number %1 does not have the expected number of columns.</center>"
                "<center>This might not be a problem, but it may be a header line.</center>"
                "<center>You may accept all similar items, or just this one, or cancel.</center>",
                row), i18n("CSV import"),
                KGuiItem(i18n("Accept All")),
                KGuiItem(i18n("Accept This")),
                KGuiItem(i18n("Cancel")));
      if(ret == KMessageBox::Cancel) {
        return ret;
      }
      if(ret == KMessageBox::Yes) {
        m_accept = true;
      }
    }
  }
  int neededFieldsCount = 0;//                        ensure essential fields are present
  QString memo;
  QString txt;
  iBuff = iBuff.remove(m_textDelimiterCharacter);
  memo.clear();//                                     memo & number may not have been used
  m_trData.number.clear();//                          .. so need to clear prior contents
  for(int i = 0; i < m_endColumn; i++) { //        check each column
    if(m_csvDialog->columnType(i) == "number") {
      txt = m_columnList[i];
      m_trData.number = txt;
      m_qifBuffer = m_qifBuffer + 'N' + txt + '\n';///     Number column
    }

    else if(m_csvDialog->columnType(i) == "date") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt = txt.remove(m_textDelimiterCharacter);//   "16/09/2009
      QDate dat = m_csvDialog->m_convertDate->convertDate(txt);/// Date column
      if(dat == QDate()) {
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

    else if(m_csvDialog->columnType(i) == "payee") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt.remove('~');//                              replace NL which was substituted
      txt = txt.remove('\'');
      m_trData.payee = txt;
      m_qifBuffer = m_qifBuffer + 'P' + txt + '\n';/// Detail column
    }

    else if(m_csvDialog->columnType(i) == "amount") {  // Is this Amount column
      ++neededFieldsCount;
      if(m_flagCol == -1) { //                        it's a new file
        switch(m_debitFlag) { //                      Flag if amount is debit or credit
          case -1://                                  Ignore flag
            m_flagCol = 0;//                          ...and continue
            break;
          case  0://                                  Ask for column no.of flag
            m_flagCol = columnNumber(i18n("Enter debit flag column number"));
            if(m_flagCol == 0) {  //                  0 means Cancel was pressed
              return KMessageBox::Cancel;//           ... so exit
            }
            break;
          default : m_flagCol = m_debitFlag;//        Contains flag/column no.
        }
      }
      if((m_flagCol < 0) || (m_flagCol > m_endColumn)) {  // shouldn't get here
        KMessageBox::sorry(0, i18n("An invalid column was entered.\n"
                                   "Must be between 1 and %1.", m_endColumn), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      QString flag;//                                 m_flagCol == valid column (or zero)
      if(m_flagCol > 0) {
        flag = m_columnList[m_flagCol - 1];//         indicates if amount is debit or credit
      }//                                             if flagCol == 0, flag is empty
      txt = m_columnList[i];
      if((m_csvDialog->amountColumn() == i) &&
          (((txt.contains("("))) || (flag.startsWith('A')))) {//  "(" or "Af" = debit
        txt = txt.remove(QRegExp("[()]"));
        txt = '-' + txt;  //                          Mark as -ve
      } else if(m_csvDialog->debitColumn() == i) {
        txt = '-' + txt;  //                          Mark as -ve
      }
      txt = txt.remove(m_parse->thousandsSeparator());
      txt = txt.replace(m_csvDialog->decimalSymbol(), KGlobal::locale()->decimalSymbol());
      m_trData.amount = txt;
      m_qifBuffer = m_qifBuffer + 'T' + txt + '\n';
    }

    else if((m_csvDialog->columnType(i) == "debit") || (m_csvDialog->columnType(i) == "credit")) { //  Credit or debit?
      ++neededFieldsCount;
      txt = m_columnList[i];
      if(!txt.isEmpty()) {
        if(m_csvDialog->debitColumn() == i)
          txt = '-' + txt;//  Mark as -ve
        if((m_csvDialog->debitColumn() == i) || (m_csvDialog->creditColumn() == i)) {
          txt = txt.replace(m_csvDialog->decimalSymbol(), KGlobal::locale()->decimalSymbol());
          m_trData.amount = txt;
          m_qifBuffer = m_qifBuffer + 'T' + txt + '\n';
        }
      }
    }

    else if(m_csvDialog->columnType(i) == "memo") {  // could be more than one
      txt = m_columnList[i];
      txt.replace('~', "\n");//                       replace NL which was substituted
      if(!memo.isEmpty())
        memo += '\n';//                               separator for multiple memos
      memo += txt;//                                  next memo
    }//end of memo field
  }//end of col loop
  m_trData.memo = memo;
  m_qifBuffer = m_qifBuffer + 'M' + memo + '\n' + "^\n";
  if(neededFieldsCount > 2) {
    return KMessageBox::Ok;
  } else {
    KMessageBox::sorry(0, i18n("<center>The columns selected are invalid.\n</center>"
                               "There must an amount or debit and credit fields, plus date and payee fields."), i18n("CSV import"));
    return KMessageBox::Cancel;
  }
}

void CsvProcessing::importClicked()
{
  if(m_csvDialog->m_fileType != "Banking") return;

  // The following two fields are optional so must be cleared
  // ...of any prior choices in UI
  m_csvDialog->comboBoxBnk_memoCol->setCurrentIndex(-1);
  m_csvDialog->comboBoxBnk_numberCol->setCurrentIndex(-1);

  if((m_csvDialog->dateSelected()) && (m_csvDialog->payeeSelected()) &&
      ((m_csvDialog->amountSelected() || (m_csvDialog->debitSelected() && m_csvDialog->creditSelected())))) {
    m_importNow = true; //                          all necessary data is present
    int skp = m_csvDialog->spinBox_skip->value() - 1;

    readFile(m_inFileName, skp);   //               skip all headers

    //--- create the (revised) vertical (row) headers ---
    QStringList vertHeaders;
    for(int i = skp; i < m_csvDialog->tableWidget->rowCount() + skp; i++) {
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
}

void CsvProcessing::readSettings()
{
  int tmp;
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup profileGroup(config, "Profile");
  m_dateFormatIndex = profileGroup.readEntry("DateFormat", QString()).toInt();
  m_csvDialog->comboBox_dateFormat->setCurrentIndex(m_dateFormatIndex);
  QString txt = profileGroup.readEntry("CurrentUI", QString());
  m_csvDialog->setCurrentUI(txt);
  tmp = profileGroup.readEntry("StartLine", QString()).toInt();
  m_csvDialog->spinBox_skip->setValue(tmp + 1);

  m_encodeIndex = profileGroup.readEntry("Encoding", QString()).toInt();

  m_fieldDelimiterIndex = profileGroup.readEntry("FieldDelimiter", QString()).toInt();
  m_csvDialog->comboBox_fieldDelimiter->setCurrentIndex(m_fieldDelimiterIndex);

  m_textDelimiterIndex = profileGroup.readEntry("TextDelimiter", QString()).toInt();
  m_csvDialog->comboBox_textDelimiter->setCurrentIndex(m_textDelimiterIndex);
  m_csvDialog->comboBox_decimalSymbol->setCurrentIndex(-1);
  m_csvDialog->comboBox_thousandsDelimiter->setCurrentIndex(-1);

  m_csvPath = profileGroup.readEntry("CsvDirectory", QString());

  m_debitFlag = profileGroup.readEntry("DebitFlag", QString().toInt());

  KConfigGroup columnsGroup(config, "Columns");

  if(columnsGroup.exists()) {
    tmp = columnsGroup.readEntry("DateCol", QString()).toInt();
    m_csvDialog->comboBoxBnk_dateCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_dateCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("PayeeCol", QString()).toInt();
    m_csvDialog->comboBoxBnk_payeeCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_payeeCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("AmountCol", QString()).toInt();
    if(tmp >= 0) {  //                            If amount previously selected, set check radio_amount
      m_csvDialog->radioBnk_amount->setChecked(true);
      m_csvDialog->labelBnk_amount->setEnabled(true);
      m_csvDialog->labelBnk_credits->setEnabled(false);
      m_csvDialog->labelBnk_debits->setEnabled(false);
    } else {//                                   ....else set check radio_debCred to clear amount col
      m_csvDialog->radioBnk_debCred->setChecked(true);
      m_csvDialog->labelBnk_credits->setEnabled(true);
      m_csvDialog->labelBnk_debits->setEnabled(true);
      m_csvDialog->labelBnk_amount->setEnabled(false);
    }
    m_csvDialog->comboBoxBnk_amountCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_amountCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("DebitCol", QString()).toInt();
    m_csvDialog->comboBoxBnk_debitCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_debitCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("CreditCol", QString()).toInt();
    m_csvDialog->comboBoxBnk_creditCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_creditCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("NumberCol", QString()).toInt();
    m_csvDialog->comboBoxBnk_numberCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_numberCol->setCurrentIndex(tmp);
    m_csvDialog->comboBoxBnk_memoCol->setCurrentIndex(-1);
  } else {
    m_csvDialog->comboBoxBnk_dateCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_payeeCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_amountCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_debitCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_numberCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_numberCol->setCurrentIndex(-1);
    m_csvDialog->comboBoxBnk_memoCol->setCurrentIndex(-1);
  }
}

void CsvProcessing::saveAs()
{
  if(m_csvDialog->m_fileType == "Banking") {
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
  }//else
}

void CsvProcessing::setCodecList(const QList<QTextCodec *> &list)
{
  m_comboBoxEncode->clear();
  foreach (QTextCodec * codec, list) {
    m_comboBoxEncode->addItem(codec->name(), codec->mibEnum());
  }
}

int CsvProcessing::startLine()
{
  return m_startLine;
}

void CsvProcessing::startLineChanged()
{
  int val = m_csvDialog->spinBox_skip->value();
  if(val < 1) {
    return;
  }
  m_startLine = val - 1;
}

void CsvProcessing::endLineChanged()
{
  m_endLine = m_csvDialog->spinBox_skipToLast->value() ;
}

void CsvProcessing::dateFormatSelected(int dF)
{
  if(dF == -1) return;
  m_dateFormatIndex = dF;
  m_date = m_dateFormats[m_dateFormatIndex];
}

void CsvProcessing::updateScreen()
{
  QRect tableRect = m_csvDialog->tableWidget->geometry();//     need table height
  int hght = m_csvDialog->tableWidget->horizontalHeader()->height() + 8;// find data height
  hght += (m_csvDialog->tableWidget->rowHeight(m_row - 1)) * m_row;
  int ht = (hght < m_csvDialog->frame->height() ? hght : m_csvDialog->frame->height() - 10);// rect.height() reduce height if > frame
  tableRect.setHeight(ht);//ht                                    set table height

  m_csvDialog->tableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);//ResizeToContents
  m_csvDialog->tableWidget->setGeometry(tableRect);//           resize now ***************
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

int CsvProcessing::textDelimiterIndex()
{
  return m_textDelimiterIndex;
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
  if(ok && ret > 0)
    return ret;
  return 0;
}

int CsvProcessing::lastLine()
{
  return m_row;
}

bool CsvProcessing::importNow()
{
  return m_importNow;
}
