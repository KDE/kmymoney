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
  m_closing = false;
  m_colTypeName.insert(ColumnPayee,i18n("Payee"));
  m_colTypeName.insert(ColumnNumber,i18n("Number"));
  m_colTypeName.insert(ColumnDebit,i18n("Debit"));
  m_colTypeName.insert(ColumnCredit,i18n("Credit"));
  m_colTypeName.insert(ColumnDate,i18n("Date"));
  m_colTypeName.insert(ColumnAmount,i18n("Amount"));
  m_colTypeName.insert(ColumnCategory,i18n("Category"));
  m_colTypeName.insert(ColumnMemo,i18n("Memo"));
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
    KConfigGroup profilesGroup(config, "Bank-" + m_wiz->m_profileList[i]);
    m_wiz->m_inFileName = profilesGroup.readEntry("Directory", QString());
    m_colTypeNum[ColumnPayee] = profilesGroup.readEntry("PayeeCol", -1);
    m_colTypeNum[ColumnNumber] = profilesGroup.readEntry("NumberCol", -1);
    m_colTypeNum[ColumnAmount] = profilesGroup.readEntry("AmountCol", -1);
    m_colTypeNum[ColumnDebit] = profilesGroup.readEntry("DebitCol", -1);
    m_colTypeNum[ColumnCredit] = profilesGroup.readEntry("CreditCol", -1);
    m_colTypeNum[ColumnDate] = profilesGroup.readEntry("DateCol", -1);
    m_colTypeNum[ColumnCategory] = profilesGroup.readEntry("CategoryCol", -1);
    m_colTypeNum[ColumnMemo] = -1; // initialize, otherwise random data may go here
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
  KConfigGroup miscGroup(config, "Misc");
  m_wiz->m_pluginHeight = miscGroup.readEntry("Height", 640);
  m_wiz->m_pluginWidth = miscGroup.readEntry("Width", 800);
}

void CSVDialog::createStatement()
{
  m_wiz->st = MyMoneyStatement();
  m_wiz->st.m_eType = MyMoneyStatement::etNone;

  m_hashSet.clear();
  for (int line = m_wiz->m_startLine - 1; line < m_wiz->m_endLine; line++) {
    if (!processBankLine(m_wiz->m_lineList[line], m_wiz->st)) { // parse fields
      m_wiz->m_importNow = false;
      m_wiz->m_wizard->back();  // have another try at the import
      break;
    }
  }
  if (!m_wiz->m_importNow)
    return;

  emit statementReady(m_wiz->st);  // bank statement ready
  m_wiz->m_importNow = false;
}

bool CSVDialog::processCreditDebit(QString& credit, QString& debit , MyMoneyMoney& amount)
{
  if (credit.startsWith('(') || credit.startsWith('[')) { // check if brackets notation is used for negative numbers
    credit.remove(QRegularExpression("[()]"));
    credit = '-' + credit;
  }
  if (debit.startsWith('(') || debit.startsWith('[')) { // check if brackets notation is used for negative numbers
    debit.remove(QRegularExpression("[()]"));
    debit = '-' + debit;
  }

  if (!credit.isEmpty() && !debit.isEmpty()) {  // we do not expect both fields to be non-zero
    if (MyMoneyMoney(credit).isZero())
      credit = QString();
    if (MyMoneyMoney(debit).isZero())
      debit = QString();
  }

  if (!debit.startsWith('-') && !debit.isEmpty()) // ensure debit field is negative
    debit = '-' + debit;

  if (!credit.isEmpty() && debit.isEmpty())
    amount = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(credit));
  else if (credit.isEmpty() && !debit.isEmpty())
    amount = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(debit));
  else if (!credit.isEmpty() && !debit.isEmpty()) { // both fields are non-empty and non-zero so let user decide
    int ret = KMessageBox::questionYesNoCancel(m_wiz,
                                               i18n("<center>The %1 field contains '%2'</center>"
                                                    "<center>and the %3 field contains '%4'.</center>"
                                                    "<center>Please choose which you wish to accept.</center>",
                                               m_colTypeName.value(ColumnDebit),
                                               m_columnList[m_colTypeNum.value(ColumnDebit)],
                                               m_colTypeName.value(ColumnCredit),
                                               m_columnList[m_colTypeNum.value(ColumnCredit)]),
        i18n("CSV invalid field values"),
        KGuiItem(i18n("Accept %1", m_colTypeName.value(ColumnDebit))),
        KGuiItem(i18n("Accept %1", m_colTypeName.value(ColumnCredit))),
        KGuiItem(i18n("Cancel")));
    if (ret == KMessageBox::Cancel)
      return false;
    if (ret == KMessageBox::Yes)
      amount = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(debit));
    else if (ret == KMessageBox::No)
      amount = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(credit));
  } else
    amount = MyMoneyMoney("0" + m_wiz->m_decimalSymbol + "00");    // both fields are empty and zero so set amount to zero

  return true;
}

bool CSVDialog::processBankLine(const QString &line, MyMoneyStatement &st)
{
  MyMoneyStatement::Transaction tr;
  m_columnList = m_wiz->m_parse->parseLine(line); // split line into fields
  if (m_columnList.count() < m_wiz->m_endColumn) {
    if (!m_wiz->m_accept) {
      int ret = KMessageBox::questionYesNoCancel(m_wiz, i18n("<center>Row number %1 does not have the expected number of columns.</center>"
                                                             "<center>This might not be a problem, but it may be a header line.</center>"
                                                             "<center>You may accept all similar items, or just this one, or cancel.</center>",
                                                             QString::number(m_wiz->m_row)), i18n("CSV import"),
                                                 KGuiItem(i18n("Accept All")),
                                                 KGuiItem(i18n("Accept This")),
                                                 KGuiItem(i18n("Cancel")));
      if (ret == KMessageBox::Cancel)
        return false;
      if (ret == KMessageBox::Yes)
        m_wiz->m_accept = true;
    }
  }

  int neededFieldsCount = 0;
  QString memo;
  QString txt;

  for (int i = 0; i < m_columnList.count(); i++) {
    m_columnList[i].trimmed().remove(m_wiz->m_textDelimiterCharacter);
  }

  // process number field
  if (m_colTypeNum.value(ColumnNumber) != -1)
    tr.m_strNumber = txt;

  // process date field
  if (m_colTypeNum.value(ColumnDate) != -1) {
    ++neededFieldsCount;
    txt = m_columnList[m_colTypeNum[ColumnDate]];
    tr.m_datePosted = m_wiz->m_convertDate->convertDate(txt);      //  Date column
    if (tr.m_datePosted == QDate()) {
      KMessageBox::sorry(m_wiz, i18n("<center>An invalid date has been detected during import.</center>"
                                     "<center><b>'%1'</b></center>"
                                     "Please check that you have set the correct date format,\n"
                                     "<center>and start and end lines.</center>"
                                     , txt), i18n("CSV import"));
      m_wiz->m_importError = true;
      return false;
    }
  }

  // process payee field
  if (m_colTypeNum.value(ColumnPayee) != -1) {
    ++neededFieldsCount;
    tr.m_strPayee = m_columnList[m_colTypeNum[ColumnPayee]];
  }

  // process memo field
  if (m_colTypeNum.value(ColumnMemo) != -1)
    memo += m_columnList[m_colTypeNum[ColumnMemo]];

  for (int i = 0; i < m_wiz->m_memoColList.count(); i++) {
    if (m_wiz->m_memoColList[i] != m_colTypeNum[ColumnMemo]) {
      if (!memo.isEmpty())
        memo += "\n";
      memo += m_columnList[m_wiz->m_memoColList[i]];
    }
  }
  tr.m_strMemo = memo;

  // process amount field
  if (m_colTypeNum.value(ColumnAmount) != -1) {
    ++neededFieldsCount;
    txt = m_columnList[m_colTypeNum[ColumnAmount]];

    if (txt.isEmpty())
      txt = "0" + m_wiz->m_decimalSymbol + "00";

    if (txt.startsWith('(') || txt.startsWith('[')) { // check if brackets notation is used for negative numbers
      txt.remove(QRegularExpression("[()]"));
      txt = '-' + txt;
    }

    if (m_oppositeSigns) {  // change signs to opposite if requested by user
      if(txt.startsWith('-'))
        txt = txt.remove(0,1);
      else if (txt.startsWith('+'))
        txt = txt.replace(0,1,"-");
      else
        txt = '-' + txt;
    }
    tr.m_amount = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(txt));
  }

  // process credit/debit field
  if (m_colTypeNum.value(ColumnCredit) != -1 &&
      m_colTypeNum.value(ColumnDebit) != -1) {
    ++neededFieldsCount;
    ++neededFieldsCount;
    if (!processCreditDebit(m_columnList[m_colTypeNum[ColumnCredit]],
                            m_columnList[m_colTypeNum[ColumnDebit]],
                            tr.m_amount))
      return false;

  }

  MyMoneyStatement::Split s1;
  s1.m_amount = tr.m_amount;
  s1.m_strMemo = tr.m_strMemo;
  MyMoneyStatement::Split s2 = s1;
  s2.m_reconcile = tr.m_reconcile;
  s2.m_amount = (-s1.m_amount);

  // process category field
  if (m_colTypeNum.value(ColumnCategory) != -1) {
    txt = m_columnList[m_colTypeNum[ColumnCategory]];
    QString accountId = m_wiz->m_csvUtil->checkCategory(txt, s1.m_amount, s2.m_amount);

    if (!accountId.isEmpty()) {
      s2.m_accountId = accountId;
      s2.m_strCategoryName = txt;
      tr.m_listSplits.append(s2);
    }
  }

  if (neededFieldsCount <= 2) {
    QString errMsg = i18n("<center>The columns selected are invalid.</center>"
                          "There must an amount or debit and credit fields, plus date and payee fields.");
    if (m_wiz->m_skipSetup)
      errMsg += i18n("<center>You possibly need to check the start and end line settings, or reset 'Skip setup'.</center>");
    KMessageBox::information(m_wiz, errMsg);
    m_wiz->m_importError = true;
    return KMessageBox::Cancel;
  }

  // calculate hash
  txt = line;
  QString hashBase = QString("%1-%2")
      .arg(tr.m_datePosted.toString(Qt::ISODate))
      .arg(MyMoneyTransaction::hash(txt.remove(m_wiz->m_textDelimiterCharacter)));
  QString hash;
  for (uchar idx = 0; idx < 0xFF; idx++) {  // assuming threre will be no more than 256 transactions with the same hashBase
    hash = QString("%1-%2").arg(hashBase).arg(idx);
    QSet<QString>::const_iterator it = m_hashSet.constFind(hash);
    if (it == m_hashSet.constEnd())
      break;
  }
  m_hashSet.insert(hash);
  tr.m_strBankID = hash;

  st.m_listTransactions += tr; // Add the transaction to the statement
  return true;
}

void CSVDialog::slotImportClicked()
{
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
}

void CSVDialog::slotSaveAsQIF()
{
  m_wiz->m_importNow = false;
  createStatement();
  if (m_wiz->st.m_listTransactions.isEmpty())
    return;
  QStringList outFile = m_wiz->m_inFileName.split('.');
  const QString &name = QString((outFile.isEmpty() ? "CsvProcessing" : outFile[0]) + ".qif");

  QString outFileName = QFileDialog::getSaveFileName(m_wiz, i18n("Save QIF"), name, QString::fromLatin1("*.qif | %1").arg(i18n("QIF Files")));
  QFile oFile(outFileName);
  oFile.open(QIODevice::WriteOnly);
  QTextStream out(&oFile);

  m_qifBuffer = "!Type:Bank\n";
  QList<MyMoneyStatement::Transaction>::const_iterator it;
  for( it = m_wiz->st.m_listTransactions.constBegin() ; it != m_wiz->st.m_listTransactions.constEnd(); it++)
  {
    m_qifBuffer += 'D' + it->m_datePosted.toString(m_wiz->m_dateFormats[m_wiz->m_dateFormatIndex]) + '\n';
    double d = it->m_amount.toDouble();
    QString txt;
    txt.setNum(d, 'f', 4);
    m_qifBuffer += 'T' + txt + '\n';
    m_qifBuffer += 'P' + it->m_strPayee + '\n';
    if (!it->m_listSplits.isEmpty())
      m_qifBuffer += 'L' + it->m_listSplits.first().m_strCategoryName + '\n';
    m_qifBuffer += 'N' + it->m_strNumber + '\n';
    m_qifBuffer += 'M' + it->m_strMemo + '\n' + "^\n";
    out << m_qifBuffer;// output qif file
    m_qifBuffer.clear();
  }
  oFile.close();
}

void CSVDialog::clearColumnsSelected()
{
  clearColumnNumbers();
  clearComboBoxText();
  m_wiz->m_memoColList.clear();
  m_colNumType.clear();
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

void CSVDialog::saveSettings()
{
  KConfigGroup miscGroup(m_wiz->m_config, "Misc");
  miscGroup.writeEntry("Height", m_wiz->height());
  miscGroup.writeEntry("Width", m_wiz->width());
  miscGroup.config()->sync();

  KConfigGroup profileNamesGroup(m_wiz->m_config, "ProfileNames");
  profileNamesGroup.writeEntry("Bank", m_wiz->m_profileList);
  profileNamesGroup.writeEntry("PriorBank", m_wiz->m_profileList.indexOf(m_wiz->m_profileName));
  profileNamesGroup.config()->sync();

  KConfigGroup profilesGroup(m_wiz->m_config, "Bank-" + m_wiz->m_profileName);
  if (m_wiz->m_inFileName.startsWith("/home/")) // replace /home/user with ~/ for brevity
  {
    QFileInfo fileInfo = QFileInfo(m_wiz->m_inFileName);
    if (fileInfo.isFile())
      m_wiz->m_inFileName = fileInfo.absolutePath();
    m_wiz->m_inFileName = "~/" + m_wiz->m_inFileName.section('/',3);
  }

  profilesGroup.writeEntry("Directory", m_wiz->m_inFileName);
  profilesGroup.writeEntry("Encoding", m_wiz->m_encodeIndex);
  profilesGroup.writeEntry("DateFormat", m_wiz->m_dateFormatIndex);
  profilesGroup.writeEntry("OppositeSigns", m_oppositeSigns);
  profilesGroup.writeEntry("FieldDelimiter", m_wiz->m_fieldDelimiterIndex);
  profilesGroup.writeEntry("TextDelimiter", m_wiz->m_textDelimiterIndex);
  profilesGroup.writeEntry("DecimalSymbol", m_wiz->m_decimalSymbolIndex);
  profilesGroup.writeEntry("StartLine", m_wiz->m_pageLinesDate->ui->spinBox_skip->value() - 1);
  profilesGroup.writeEntry("TrailerLines", m_wiz->m_pageLinesDate->m_trailerLines);

  profilesGroup.writeEntry("DateCol", m_colTypeNum.value(ColumnDate));
  profilesGroup.writeEntry("PayeeCol", m_colTypeNum.value(ColumnPayee));

  QList<int> list = m_wiz->m_memoColList;
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

bool CSVDialog::validateMemoComboBox()
{
  if (m_wiz->m_memoColList.count() == 0)
    return true;
  for (int i = 0; i < m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->count(); i++)
  {
    QString txt = m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->itemText(i);
    if (txt.contains('*'))  // check if text containing '*' belongs to valid column types
      if (m_colNumType.value(i) != ColumnPayee) {
        m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(i, QString().setNum(i + 1));
        m_wiz->m_memoColList.removeOne(i);
        return false;
      }
  }
  return true;
}

bool CSVDialog::validateSelectedColumn(int col, columnTypeE type)
{
  if (m_colTypeNum.value(type) != -1)        // check if this 'type' has any column 'number' assigned...
    m_colNumType.remove(m_colTypeNum[type]); // ...if true remove 'type' assigned to this column 'number'

  if (col == -1) { // user only wanted to reset his column so allow him
    m_colTypeNum[type] = col;  // assign new column 'number' to this 'type'
    return true;
  }

  if (m_colNumType.contains(col)) { // if this column 'number' has already 'type' assigned
    KMessageBox::information(m_wiz, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>",
                                     m_colTypeName.value(m_colNumType[col])));
    resetComboBox(m_colNumType[col]);
    resetComboBox(type);
    return false;
  }

  m_colTypeNum[type] = col; // assign new column 'number' to this 'type'
  m_colNumType[col] = type; // assign new 'type' to this column 'number'
  return true;
}

void CSVDialog::memoColumnSelected(int col)
{
  if (m_colNumType.value(col) == ColumnPayee ) {
    int rc = KMessageBox::Yes;
    if (m_wiz->m_pageBanking->isVisible())
      rc = KMessageBox::questionYesNo(0, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                              "<center>If you wish to copy the Payee data to the memo field, click 'Yes'.</center>",
                                              m_colTypeName.value(m_colNumType[col])));
    if (rc == KMessageBox::Yes) {
      m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
      if (!m_wiz->m_memoColList.contains(col))
        m_wiz->m_memoColList.append(col);
    } else {
      m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1));
      m_wiz->m_memoColList.removeOne(col);
    }
    //allow only separate memo field occupy combobox
    m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->blockSignals(true);
    if (m_colTypeNum.value(ColumnMemo) != -1)
      m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(m_colTypeNum.value(ColumnMemo));
    else
      m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
    m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->blockSignals(false);
    return;
  }

  if (m_colTypeNum.value(ColumnMemo) != -1)        // check if this memo has any column 'number' assigned...
    m_wiz->m_memoColList.removeOne(col);           // ...if true remove it from memo list

  if(validateSelectedColumn(col, ColumnMemo))
    if (col != - 1 && !m_wiz->m_memoColList.contains(col))
      m_wiz->m_memoColList.append(col);
}

void CSVDialog::categoryColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnCategory);
}

void CSVDialog::numberColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnNumber);
}

void CSVDialog::payeeColumnSelected(int col)
{
  if (validateSelectedColumn(col, ColumnPayee))
    if (!validateMemoComboBox())  // user could have it already in memo so...
      memoColumnSelected(col);    // ...if true set memo field again
}

void CSVDialog::dateColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnDate);
}

void CSVDialog::debitColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnDebit);
}

void CSVDialog::creditColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnCredit);
}

void CSVDialog::amountColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnAmount);
}

void CSVDialog::resetComboBox(columnTypeE comboBox)
{
  switch (comboBox) {
    case ColumnAmount:
      m_wiz->m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
      break;
    case ColumnCredit:
      m_wiz->m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
      break;
    case ColumnDate:
      m_wiz->m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
      break;
    case ColumnDebit:
      m_wiz->m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
      break;
    case ColumnMemo:
      m_wiz->m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
      break;
    case ColumnNumber:
      m_wiz->m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
      break;
    case ColumnPayee:
      m_wiz->m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
      break;
    case ColumnCategory:
      m_wiz->m_pageBanking->ui->comboBoxBnk_categoryCol->setCurrentIndex(-1);
      break;
    default:
      KMessageBox::sorry(m_wiz, i18n("<center>Field name not recognised.</center> <center>'<b>%1</b>'</center> Please re-enter your column selections."
                                    , comboBox), i18n("CSV import"));
  }
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
