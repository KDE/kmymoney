/*******************************************************************************
*                                 csvdialog.cpp
*                              --------------------
* begin                       : Sat Jan 01 2010
* copyright                   : (C) 2010 by Allan Anderson
* email                       : agander93@gmail.com
* copyright                   : (C) 2016 by Łukasz Wojniłowicz
* email                       : lukasz.wojnilowicz@gmail.com
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

#include <QWizard>
#include <QWizardPage>
#include <QDebug>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QFrame>
#include <QGridLayout>
#include <QTableWidget>
#include <QTextCodec>
#include <QTimer>
#include <QStandardPaths>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QUrl>
#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kmessagebox.h>
#include <kguiitem.h>
#include <KSharedConfig>
#include "KAboutApplicationDialog"
#include <KAboutData>
#include <KIconLoader>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <kjobwidgets.h>
#include <kio/job.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "convdate.h"
#include "csvutil.h"
#include "csvwizard.h"

#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_lines-datewizardpage.h"
#include "ui_completionwizardpage.h"
#include "ui_investmentwizardpage.h"
#include "ui_csvwizard.h"

#include "mymoneyfile.h"

// ----------------------------------------------------------------------------
CSVDialog::CSVDialog()
{
  m_firstPass = true;
  m_firstRead = true;
  m_closing = false;
  m_previousColumn = 0;
  m_curId = -1;
  m_lastId = -1;
  m_previousType.clear();
  m_lastFileName.clear();

  QFont font(QApplication::font());
  QFontMetrics cellFontMetrics(font);
}

void CSVDialog::init()
{
  connect(this, SIGNAL(statementReady(MyMoneyStatement&)), m_wiz->m_plugin, SLOT(slotGetStatement(MyMoneyStatement&)));
}//  CSVDialog

CSVDialog::~CSVDialog()
{
}

void CSVDialog::readSettings()
{
  KSharedConfigPtr config = KSharedConfig::openConfig(QStandardPaths::locate(QStandardPaths::ConfigLocation, "csvimporterrc"));
  for (int i = 0; i < m_wiz->m_profileList.count(); i++) {
    if (m_wiz->m_profileList[i] != m_wiz->m_profileName)
      continue;
    KConfigGroup profilesGroup(config, "Profiles-" + m_wiz->m_profileList[i]);
    m_csvPath = profilesGroup.readEntry("CsvDirectory", QString());
    m_debitFlag = profilesGroup.readEntry("DebitFlag", -1);
    m_payeeColumn = profilesGroup.readEntry("PayeeCol", -1);
    m_numberColumn = profilesGroup.readEntry("NumberCol", -1);
    m_amountColumn = profilesGroup.readEntry("AmountCol", -1);
    m_debitColumn = profilesGroup.readEntry("DebitCol", -1);
    m_creditColumn = profilesGroup.readEntry("CreditCol", -1);
    m_wiz->m_dateColumn = profilesGroup.readEntry("DateCol", -1);
    m_categoryColumn = profilesGroup.readEntry("CategoryCol", -1);
    m_oppositeSigns = profilesGroup.readEntry("OppositeSigns", 0);
    m_wiz->m_memoColList = profilesGroup.readEntry("MemoCol", QList<int>());
    m_wiz->m_dateFormatIndex = profilesGroup.readEntry("DateFormat", -1);
    m_wiz->m_textDelimiterIndex = profilesGroup.readEntry("TextDelimiter", 0);
    m_wiz->m_fieldDelimiterIndex = profilesGroup.readEntry("FieldDelimiter", -1);
    m_wiz->m_decimalSymbolIndex = profilesGroup.readEntry("DecimalSymbol", -1);

    if (m_wiz->m_decimalSymbolIndex == -1) { // if no decimal symbol in config, then get one from locale settings
      if (QLocale().decimalPoint() == '.')
        m_wiz->m_decimalSymbolIndex = 0;
      else
        m_wiz->m_decimalSymbolIndex = 1;
    }
    if (m_wiz->m_decimalSymbolIndex == 0)
      m_wiz->m_ThousandsSeparatorIndex = 1;
    else
      m_wiz->m_ThousandsSeparatorIndex = 0;

    m_wiz->m_parse->setDecimalSymbolIndex(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_parse->setDecimalSymbol(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_parse->setThousandsSeparatorIndex(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_parse->setThousandsSeparator(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_decimalSymbol = m_wiz->m_parse->decimalSymbol(m_wiz->m_decimalSymbolIndex);

    m_wiz->m_parse->setFieldDelimiterIndex(m_wiz->m_fieldDelimiterIndex);
    m_wiz->m_parse->setTextDelimiterIndex(m_wiz->m_textDelimiterIndex);
    m_wiz->m_fieldDelimiterCharacter = m_wiz->m_parse->fieldDelimiterCharacter(m_wiz->m_fieldDelimiterIndex);
    m_wiz->m_textDelimiterCharacter = m_wiz->m_parse->textDelimiterCharacter(m_wiz->m_textDelimiterIndex);
    m_wiz->m_decimalSymbol = m_wiz->m_parse->decimalSymbol(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_startLine = profilesGroup.readEntry("StartLine", 0) + 1;
    m_wiz->m_pageLinesDate->m_trailerLines = profilesGroup.readEntry("TrailerLines", 0);
    m_wiz->m_encodeIndex = profilesGroup.readEntry("Encoding", 0);
    break;
  }
  KConfigGroup mainGroup(config, "MainWindow");
  m_wiz->m_pluginHeight = mainGroup.readEntry("Height", 640);
  m_wiz->m_pluginWidth = mainGroup.readEntry("Width", 800);
}

void CSVDialog::reloadUISettings()
{
  m_payeeColumn =  m_columnTypeList.indexOf("payee");
  m_numberColumn = m_columnTypeList.indexOf("number");
  m_debitColumn = m_columnTypeList.indexOf("debit");
  m_creditColumn = m_columnTypeList.indexOf("credit");
  m_wiz->m_dateColumn = m_columnTypeList.indexOf("date");
  m_amountColumn = m_columnTypeList.indexOf("amount");
  m_categoryColumn = m_columnTypeList.indexOf("category");
  m_wiz->m_startLine = m_wiz->m_pageLinesDate->ui->spinBox_skip->value();
  m_wiz->m_endLine = m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value();
}

void CSVDialog::slotFileDialogClicked()
{
  if ((m_wiz->m_fileType != "Banking") || (m_wiz->m_profileName.isEmpty())) {
    if (m_wiz->m_fileType == "Banking") {
      KMessageBox::information(0, i18n("Please select a profile type and enter a profile name."));
    }
    return;
  }
  m_wiz->m_skipSetup = m_wiz->m_pageIntro->ui->checkBoxSkipSetup->isChecked();
  m_wiz->m_columnsNotSet = true;  //  Don't check columns until they've been selected.
  m_wiz->m_inFileName.clear();
  m_url.clear();
  m_firstPass = true;
  m_firstRead = true;
  m_wiz->m_accept = false;

  //  The "DebitFlag" setting is used to indicate whether or not to allow the user,
  //  via a dialog, to specify a column which contains a flag to indicate if the
  //  amount field is a debit ('a' or 'af'), a credit ('bij') (ING - Netherlands),
  //   or ignore ('-1').
  m_flagCol = -1;
  m_debitFlag = -1;
  m_wiz->m_importNow = false;//                       Avoid attempting date formatting on headers
  m_wiz->m_acceptAllInvalid = false;  //              Don't accept further invalid values.
  m_wiz->m_parse->setSymbolFound(false);
  m_clearAll = false;
  m_firstIsValid = false;
  m_secondIsValid = false;
  m_firstField = true;
  readSettings();

  if (m_csvPath.isEmpty()) {
    m_csvPath = QDir::home().absolutePath();
  }

  if(m_csvPath.startsWith("~/"))  //expand Linux home directory
    m_csvPath.replace(0, 1, QDir::home().absolutePath());

  QPointer<QFileDialog> dialog = new QFileDialog(m_wiz->m_wizard, QString(), m_csvPath,
                                                 i18n("*.csv *.PRN *.txt | CSV Files\n *|All files"));
  dialog->setOption(QFileDialog::DontUseNativeDialog, true);  //otherwise we cannot add custom QComboBox
  dialog->setFileMode(QFileDialog::ExistingFile);
  QLabel* label = new QLabel(i18n("Encoding"));
  dialog->layout()->addWidget(label);
  //    Add encoding selection to FileDialog
  m_wiz->m_comboBoxEncode = new QComboBox();
  m_wiz->setCodecList(m_wiz->m_codecs);
  m_wiz->m_comboBoxEncode->setCurrentIndex(m_wiz->m_encodeIndex);
  connect(m_wiz->m_comboBoxEncode, SIGNAL(activated(int)), this, SLOT(encodingChanged(int)));
  dialog->layout()->addWidget(m_wiz->m_comboBoxEncode);
  if(dialog->exec() == QDialog::Accepted) {
    m_url = dialog->selectedUrls().first();
  }
  delete dialog;

  if (m_url.isEmpty()) {
    return;
  } else if (m_url.isLocalFile()) {
    m_wiz->m_inFileName = m_url.toLocalFile();
  } else {
    m_wiz->m_inFileName = QDir::tempPath();
    if(!m_wiz->m_inFileName.endsWith(QDir::separator()))
      m_wiz->m_inFileName += QDir::separator();
    m_wiz->m_inFileName += m_url.fileName();
    qDebug() << "Source:" << m_url.toDisplayString() << "Destination:" << m_wiz->m_inFileName;
    KIO::FileCopyJob *job = KIO::file_copy(m_url, QUrl::fromUserInput(m_wiz->m_inFileName), -1,KIO::Overwrite);
    KJobWidgets::setWindow(job, m_wiz->m_wizard);
    job->exec();
    if (job->error()) {
      KMessageBox::detailedError(0, i18n("Error while loading file '%1'.", m_url.toDisplayString()),
                                 job->errorString(),
                                 i18n("File access error"));
      return;
    }
  }

  if (m_wiz->m_inFileName.isEmpty())
    return;

  m_wiz->readFile(m_wiz->m_inFileName);
  m_wiz->displayLines(m_wiz->m_lineList, m_wiz->m_parse);
  enableInputs();

  //The following two items do not *Require* an entry so old values must be cleared.
  m_trData.number.clear();  //                 this needs to be cleared or gets added to next transaction
  m_trData.memo.clear();  //                   this too, as neither might be overwritten by new data.

  m_wiz->updateWindowSize();
  m_wiz->m_wizard->next();  //go to separator page
  if (m_wiz->m_skipSetup)
    for (int i = 0; i < 4; i++) //programmaticaly go through separator-, banking-, linesdate-, completionpage
      m_wiz->m_wizard->next();
}

void CSVDialog::createStatement()
{
  if (!m_wiz->m_importNow)
    return;

  m_outBuffer.clear();
  m_qifBuffer = "!Type:Bank\n";
  MyMoneyStatement st = MyMoneyStatement();
  st.m_eType = MyMoneyStatement::etNone;
  m_wiz->createMemoField(m_columnTypeList);

  reloadUISettings();
  for (int line = m_wiz->m_startLine - 1; line < m_wiz->m_endLine; line++) {
    m_columnList = m_wiz->m_parse->parseLine(m_wiz->m_lineList[line]); // split line into fields
    int ret = processQifLine(m_outBuffer); // parse fields
    if (ret == KMessageBox::Ok) {
      csvImportTransaction(st);
    } else {
      m_wiz->m_importNow = false;
      m_wiz->m_wizard->back();  // have another try at the import
      break;
    }
  }
  if (!m_wiz->m_importNow)
    return;

  emit statementReady(st);  // bank statement ready
  m_wiz->m_importNow = false;
  // the life cycle of the contents of this map is one import process
  m_hashMap.clear();
}


int CSVDialog::processQifLine(QString& iBuff)
{
  //   parse an input line
  QString newTxt;
  m_firstField = true;
  if (m_columnList.count() < m_wiz->m_endColumn) {
    if (!m_wiz->m_accept) {
      QString row = QString::number(m_wiz->m_row);
      int ret = KMessageBox::questionYesNoCancel(m_wiz, i18n("<center>Row number %1 does not have the expected number of columns.</center>"
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
        m_wiz->m_accept = true;
      }
    }
  }
  int neededFieldsCount = 0;//                          ensure essential fields are present
  QString memo;
  QString txt;
  iBuff = iBuff.remove(m_wiz->m_textDelimiterCharacter);
  memo.clear();//                                       memo & number may not have been used
  m_trData.number.clear();//                            .. so need to clear prior contents
  for (int i = 0; i < m_columnList.count(); i++) {
    //  Use actual column count for this line instead of m_wiz->m_endColumn, which could be greater.
    if (m_columnTypeList[i] == "number") {
      txt = m_columnList[i];
      m_trData.number = txt;
      m_qifBuffer = m_qifBuffer + 'N' + txt + '\n';     // Number column
    } else if (m_columnTypeList[i] == "date") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt = txt.remove(m_wiz->m_textDelimiterCharacter);       //   "16/09/2009
      QDate dat = m_wiz->m_convertDate->convertDate(txt);      //  Date column
      if (dat == QDate()) {
        KMessageBox::sorry(m_wiz, i18n("<center>An invalid date has been detected during import.</center>"
                                      "<center><b>'%1'</b></center>"
                                      "Please check that you have set the correct date format,\n"
                                      "<center>and start and end lines.</center>"
                                      , txt), i18n("CSV import"));
        m_wiz->m_importError = true;
        return KMessageBox::Cancel;
      }
      QString qifDate = dat.toString(m_wiz->m_dateFormats[m_wiz->m_dateFormatIndex]);
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
      if (!m_firstPass) {
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
      if ((m_flagCol < 0) || (m_flagCol > m_wiz->m_endColumn)) {      // shouldn't get here
        KMessageBox::sorry(0, i18n("An invalid column was entered.\n"
                                   "Must be between 1 and %1.", m_wiz->m_endColumn), i18n("CSV import"));
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
      if (m_oppositeSigns) {
        if(txt.left(1) == "-")
          txt = txt.remove(0,1);
        else if (txt.left(1) == "+")
          txt = txt.replace(0,1,"-");
        else
          txt = '-' + txt;
      }
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
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
    if (m_wiz->m_skipSetup) {
      errMsg += i18n("<center>You possibly need to check the start and end line settings, or reset 'Skip setup'.</center>");
    }
    KMessageBox::information(0, errMsg);
    m_wiz->m_importError = true;
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
  QString zero = "0" + m_wiz->m_decimalSymbol + "00";
  QString newTxt;
  QString txt = m_columnList[col].trimmed();  //              A field of blanks is not good...
  if ((!txt.isEmpty()) && ((col == m_debitColumn))) {
    txt = '-' + txt;  //                                      Mark as -ve
  }
  if (!txt.isEmpty() && !txt.contains(m_wiz->m_decimalSymbol)) {
    //  This field has no decimal part
    txt += m_wiz->m_decimalSymbol + "00";
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
      int ret = KMessageBox::questionYesNoCancel(m_wiz, i18n("<center>On row '%5', the '%1' field contains '%2', and the '%3' field contains '%4'.</center>"
                "<center>This combination is not valid.</center>"
                "<center>If you wish for just this zero field to be cleared, click 'Clear this'.</center>"
                "<center>Or, if you wish for all such zero fields to be cleared, click 'Clear all'.</center>"
                "<center>Otherwise, click 'Cancel'.</center>",
                m_firstType, m_firstValue, m_secondType, m_secondValue, m_wiz->m_row), i18n("CSV invalid field values"),
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
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
      m_trData.amount = newTxt;
      m_qifBuffer = m_qifBuffer + 'T' + m_trData.amount + '\n';
    }  //  end of first error test
    else if (bothFieldsNotZero && !m_firstValue.isEmpty() && !m_secondValue.isEmpty()) {  //  credit and debit contain values - not good
      //  both debit and credit have entries so ask user how to proceed.
      //  if just one field is empty, that's OK - bypass this message
      ret = KMessageBox::questionYesNoCancel(m_wiz, i18n("<center>The %1 field contains '%2'</center>"
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
        m_trData.amount = '-' + m_wiz->m_parse->possiblyReplaceSymbol(m_columnList[m_debitColumn]);
      } else if (ret == KMessageBox::No) {
        m_trData.amount = m_wiz->m_parse->possiblyReplaceSymbol(m_columnList[m_creditColumn]);
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
    int rc = KMessageBox::warningContinueCancel(0, i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
             "date format setting of \"%2.\"" "\n\nPressing \'Continue\' will "
             "assign today's date to the transaction. Pressing \'Cancel\'' will abort "
             "the import operation. You can then restart the import and select a different "
             "date format.", m_trData.date.toString(m_wiz->m_date), m_wiz->m_dateFormats[m_wiz->m_dateFormatIndex]), i18n("Invalid date format"));
    switch (rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = (QDate::currentDate());
        break;

      case KMessageBox::Cancel:
        m_wiz->m_importNow = false;//                             Don't process statement
        st = MyMoneyStatement();
        m_wiz->m_importError = true;
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
    accountId = m_wiz->m_csvUtil->checkCategory(tmp, s1.m_amount, s2.m_amount);

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
  if (m_wiz->m_fileType != "Banking") {
    return;
  }
  if ((m_dateSelected) && (m_payeeSelected) &&
      ((m_amountSelected || (m_debitSelected && m_creditSelected)))) {
    m_wiz->m_importNow = true; //                  all necessary data is present

    if (m_wiz->m_startLine -1 > m_wiz->m_endLine) {
      KMessageBox::sorry(0, i18n("<center>The start line is greater than the end line.\n</center>"
                                 "<center>Please correct your settings.</center>"), i18n("CSV import"));
      m_wiz->m_importError = true;
      return;
    }
    if (m_wiz->m_importError) {  //                possibly from wrong decimal symbol or date format
      return;
    }
    m_wiz->m_parse->setSymbolFound(false);
    createStatement();
    m_wiz->markUnwantedRows();
  } else {
    QString errMsg = i18n("<center>There must an amount or debit and credit fields, plus date and payee fields.</center>");
    if (m_wiz->m_skipSetup) {
      errMsg += i18n("<center>As you had skipped Setup, the wizard will now return you to the setups.</center>");
    }
    KMessageBox::information(0, errMsg);
    m_wiz->m_importError = true;
  }
}

void CSVDialog::slotSaveAsQIF()
{
  if (m_wiz->m_fileType == QLatin1String("Banking")) {
    QStringList outFile = m_wiz->m_inFileName.split('.');
    const QString &name = QString((outFile.isEmpty() ? "CsvProcessing" : outFile[0]) + ".qif");

    QString outFileName = QFileDialog::getSaveFileName(m_wiz, i18n("Save QIF"), name, QString::fromLatin1("*.qif | %1").arg(i18n("QIF Files")));
    QFile oFile(outFileName);
    oFile.open(QIODevice::WriteOnly);
    QTextStream out(&oFile);
    out << m_qifBuffer;// output qif file
    oFile.close();
  }//else
}

int CSVDialog::columnNumber(const QString& msg)
{
  //  This dialog box is for use with the debit/credit flag resource file entry,
  //  indicating the sign of the value column. ie a debit or a credit.
  bool ok;
  static int ret;
  ret = QInputDialog::getInt(0, i18n("Enter column number of debit/credit code"), msg, 0, 1, m_wiz->m_endColumn, 1, &ok);
  if (ok && ret > 0)
    return ret;
  return 0;
}

void CSVDialog::clearColumnsSelected()
{
  clearSelectedFlags();
  clearColumnNumbers();
  clearComboBoxText();
  m_wiz->m_memoColList.clear();
  for (int i = 0; i < m_columnTypeList.count(); i++)
    m_columnTypeList[i].clear();
}

void CSVDialog::clearSelectedFlags()
{
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
  for (int i = 0; i < m_wiz->m_maxColumnCount; i++) {
    m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(i, QString().setNum(i + 1));
  }
}

void CSVDialog::encodingChanged(int index)
{
  m_wiz->m_encodeIndex = index;
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
  m_wiz->m_pageBanking->ui->checkBoxBnk_oppositeSigns->setEnabled(true);
}

void CSVDialog::saveSettings()
{
  if ((m_wiz->m_fileType != "Banking") || (m_wiz->m_inFileName.isEmpty())) {      //  don't save if no file loaded
    return;
  }

  KSharedConfigPtr config = KSharedConfig::openConfig(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + "csvimporterrc");

  KConfigGroup mainGroup(config, "MainWindow");
  mainGroup.writeEntry("Height", m_wiz->height());
  mainGroup.writeEntry("Width", m_wiz->width());
  mainGroup.config()->sync();

  KConfigGroup bankProfilesGroup(config, "BankProfiles");

  bankProfilesGroup.writeEntry("BankNames", m_wiz->m_profileList);
  int indx = m_wiz->m_pageIntro->ui->combobox_source->findText(m_wiz->m_priorCsvProfile, Qt::MatchExactly);
  QString str;
  if (indx > 0) {
    str = m_wiz->m_priorCsvProfile;
  }
  bankProfilesGroup.writeEntry("PriorCsvProfile", str);
  bankProfilesGroup.config()->sync();

  for (int i = 0; i < m_wiz->m_profileList.count(); i++) {
    if (m_wiz->m_profileList[i] != m_wiz->m_profileName) {
      continue;
    }

    QString txt = "Profiles-" + m_wiz->m_profileList[i];
    KConfigGroup profilesGroup(config, txt);
    profilesGroup.writeEntry("ProfileName", m_wiz->m_profileList[i]);
    profilesGroup.writeEntry("CurrentUI", m_currentUI);
    m_csvPath = m_wiz->m_inFileName;
    int posn = m_csvPath.lastIndexOf("/");
    m_csvPath.truncate(posn + 1);   //           keep last "/"
    QString pth = "~/" + m_csvPath.section('/', 3);
    profilesGroup.writeEntry("CsvDirectory", pth);
    profilesGroup.writeEntry("Encoding", m_wiz->m_encodeIndex);
    profilesGroup.writeEntry("DateFormat", m_wiz->m_dateFormatIndex);
    profilesGroup.writeEntry("DebitFlag", m_debitFlag);
    profilesGroup.writeEntry("OppositeSigns", m_oppositeSigns);
    profilesGroup.writeEntry("FileType", m_wiz->m_fileType);
    profilesGroup.writeEntry("FieldDelimiter", m_wiz->m_fieldDelimiterIndex);
    profilesGroup.writeEntry("TextDelimiter", m_wiz->m_textDelimiterIndex);
    profilesGroup.writeEntry("DecimalSymbol", m_wiz->m_decimalSymbolIndex);
    profilesGroup.writeEntry("StartLine", m_wiz->m_pageLinesDate->ui->spinBox_skip->value() - 1);
    profilesGroup.writeEntry("TrailerLines", m_wiz->m_pageLinesDate->m_trailerLines);

    profilesGroup.writeEntry("DateCol", m_wiz->m_dateColumn);
    profilesGroup.writeEntry("PayeeCol", m_payeeColumn);

    QList<int> list = m_wiz->m_memoColList;
    posn = 0;
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
}

int CSVDialog::validateColumn(const int& col, QString& type)
{
  if ((!m_wiz->m_pageBanking->m_bankingPageInitialized) || (m_wiz->m_fileType != "Banking")) {
    return KMessageBox::Ok;
  }
  if (m_wiz->m_columnsNotSet) {  //  Don't check columns until they've been selected.
    return KMessageBox::Ok;
  }
  //  First check if selection is in range
  if ((col < 0) || (col >= m_wiz->m_endColumn)) {
    return KMessageBox::No;
  }
  //  selection is in range
  if (m_columnTypeList[col] == type) {//  already selected
    return KMessageBox::Ok;
  }
  if (m_columnTypeList[col].isEmpty()) {  //  is this type already in use
    for (int i = 0; i < m_wiz->m_endColumn; i++) {
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
    int rc = KMessageBox::questionYesNo(0, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                        "<center>If you wish to copy the Memo data to the Payee field, click 'Yes'.</center>",
                                        m_columnTypeList[col]));
    if (rc == KMessageBox::Yes) {
      m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
      m_payeeColumn = col;
      m_columnTypeList[col] = type;
      m_columnTypeList << "memo";  //  the payeecolumn copy goes here

      if (m_columnList.count() < m_columnTypeList.count()) {
        m_columnList << "";
        m_wiz->m_maxColumnCount ++;
        m_wiz->m_endColumn ++;
      }
      m_wiz->m_memoColumn = m_wiz->m_endColumn;
      m_payeeSelected = true;
//      m_columnCountList << m_wiz->m_maxColumnCount + 1;
      return rc;
    }
  }
  //  BUT column is already in use
  if (m_wiz->m_pageBanking->isVisible()) {
    KMessageBox::information(0, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                     "<center>Please reselect both entries as necessary.</center>", m_columnTypeList[col]));
    if (m_columnTypeList[col] == "memo") {  //  If memo col has now been cleared, remove it from m_columnTypeList too
      m_wiz->m_memoColList.removeOne(col);
    }
    m_previousColumn = -1;
    m_wiz->resetComboBox(m_columnTypeList[col], col);
    m_wiz->resetComboBox(type, col);
    m_previousType.clear();
    m_columnTypeList[col].clear();

    for (int i = 0; i < m_wiz->m_maxColumnCount; i++) {
      if (!m_columnTypeList[i].isEmpty()) {
        if (m_columnTypeList[i] == type) {
          m_columnTypeList[i].clear();
        }
      }
    }
  }
  return KMessageBox::Cancel;
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
  return m_wiz->m_importNow;
}

void CSVDialog::showStage()
{
  QString str = m_wiz->ui->label_intro->text();
  m_wiz->ui->label_intro->setText(QLatin1String("<b>") + str + QLatin1String("</b>"));
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
  return m_wiz->m_dateColumn;
}

void CSVDialog::setDateColumn(int val)
{
  m_wiz->m_dateColumn = val;
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
  return m_wiz->m_memoColumn;
}

void CSVDialog::setMemoColumn(int val)
{
  m_wiz->m_memoColumn = val;
}

int CSVDialog::categoryColumn() const
{
  return m_categoryColumn;
}

void CSVDialog::setCategoryColumn(int val)
{
  m_categoryColumn = val;
}

int CSVDialog::oppositeSignsCheckBox() const
{
  return m_oppositeSigns;
}

void CSVDialog::setOppositeSignsCheckBox(int val)
{
  m_oppositeSigns = val;
}

void CSVDialog::clearColumnTypeList()
{
  m_columnTypeList.clear();
}
