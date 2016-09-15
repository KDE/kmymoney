/*******************************************************************************
*                                 bankingwizardpage.cpp
*                              --------------------
* begin                       : Thur Jan 01 2015
* copyright                   : (C) 2015 by Allan Anderson
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

#include "bankingwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QtCore/QTextStream>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KConfigGroup>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"

#include "convdate.h"
#include "csvutil.h"
#include "csvwizard.h"

#include "ui_bankingwizardpage.h"
#include "ui_csvwizard.h"

// ----------------------------------------------------------------------------


BankingPage::BankingPage(QDialog *parent) : QWizardPage(parent), ui(new Ui::BankingPage)
{
  ui->setupUi(this);
  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);

  connect(ui->button_clear, SIGNAL(clicked()), this, SLOT(slotClearColumns()));
  connect(ui->radioBnk_amount, SIGNAL(toggled(bool)), this, SLOT(slotAmountToggled(bool)));
  connect(ui->radioBnk_debCred, SIGNAL(toggled(bool)), this, SLOT(slotDebitCreditToggled(bool)));
  connect(ui->checkBoxBnk_oppositeSigns, SIGNAL(clicked(bool)), this, SLOT(slotOppositeSignsClicked(bool)));

  // initialize column names
  m_colTypeName.insert(ColumnPayee,i18n("Payee"));
  m_colTypeName.insert(ColumnNumber,i18n("Number"));
  m_colTypeName.insert(ColumnDebit,i18n("Debit"));
  m_colTypeName.insert(ColumnCredit,i18n("Credit"));
  m_colTypeName.insert(ColumnDate,i18n("Date"));
  m_colTypeName.insert(ColumnAmount,i18n("Amount"));
  m_colTypeName.insert(ColumnCategory,i18n("Category"));
  m_colTypeName.insert(ColumnMemo,i18n("Memo"));
}

BankingPage::~BankingPage()
{
  delete ui;
}

void BankingPage::setParent(CSVWizard* dlg)
{
  m_wiz = dlg;
}

void BankingPage::initializeComboBoxes()
{
  // disable banking widgets allowing their initialization
  disconnect(ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAmountColSelected(int)));
  disconnect(ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDebitColSelected(int)));
  disconnect(ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCreditColSelected(int)));
  disconnect(ui->comboBoxBnk_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotMemoColSelected(int)));
  disconnect(ui->comboBoxBnk_numberCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotNumberColSelected(int)));
  disconnect(ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColSelected(int)));
  disconnect(ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPayeeColSelected(int)));
  disconnect(ui->comboBoxBnk_categoryCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCategoryColSelected(int)));

  // clear all existing items before adding new ones
  ui->comboBoxBnk_numberCol->clear();
  ui->comboBoxBnk_dateCol->clear();
  ui->comboBoxBnk_payeeCol->clear();
  ui->comboBoxBnk_memoCol->clear();
  ui->comboBoxBnk_amountCol->clear();
  ui->comboBoxBnk_creditCol->clear();
  ui->comboBoxBnk_debitCol->clear();
  ui->comboBoxBnk_categoryCol->clear();

  QStringList columnNumbers;
  for (int i = 0; i < m_wiz->m_maxColumnCount; ++i)
    columnNumbers << QString::number(i + 1);

  // populate comboboxes with col # values
  ui->comboBoxBnk_numberCol->addItems(columnNumbers);
  ui->comboBoxBnk_dateCol->addItems(columnNumbers);
  ui->comboBoxBnk_payeeCol->addItems(columnNumbers);
  ui->comboBoxBnk_memoCol->addItems(columnNumbers);
  ui->comboBoxBnk_amountCol->addItems(columnNumbers);
  ui->comboBoxBnk_creditCol->addItems(columnNumbers);
  ui->comboBoxBnk_debitCol->addItems(columnNumbers);
  ui->comboBoxBnk_categoryCol->addItems(columnNumbers);

  slotClearColumns(); // all comboboxes are set to 0 so set them to -1
  connect(ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAmountColSelected(int)));
  connect(ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDebitColSelected(int)));
  connect(ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCreditColSelected(int)));
  connect(ui->comboBoxBnk_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotMemoColSelected(int)));
  connect(ui->comboBoxBnk_numberCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotNumberColSelected(int)));
  connect(ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColSelected(int)));
  connect(ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPayeeColSelected(int)));
  connect(ui->comboBoxBnk_categoryCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCategoryColSelected(int)));
}

void BankingPage::initializePage()
{
  if (ui->comboBoxBnk_dateCol->count() != m_wiz->m_endColumn)
    initializeComboBoxes();

  ui->comboBoxBnk_payeeCol->setCurrentIndex(m_colTypeNum.value(ColumnPayee));
  ui->comboBoxBnk_numberCol->setCurrentIndex(m_colTypeNum.value(ColumnNumber));
  ui->comboBoxBnk_amountCol->setCurrentIndex(m_colTypeNum.value(ColumnAmount));
  ui->comboBoxBnk_debitCol->setCurrentIndex(m_colTypeNum.value(ColumnDebit));
  ui->comboBoxBnk_creditCol->setCurrentIndex(m_colTypeNum.value(ColumnCredit));
  ui->comboBoxBnk_dateCol->setCurrentIndex(m_colTypeNum.value(ColumnDate));
  ui->comboBoxBnk_categoryCol->setCurrentIndex(m_colTypeNum.value(ColumnCategory));
  ui->checkBoxBnk_oppositeSigns->setChecked(m_oppositeSigns);

  if (m_wiz->m_memoColList.count() > 0)
  {
    for (int i = 0; i < m_wiz->m_memoColList.count(); ++i)
      ui->comboBoxBnk_memoCol->setCurrentIndex(m_wiz->m_memoColList[i]);
  } else
    ui->comboBoxBnk_memoCol->setCurrentIndex(-1);

  if (this->m_colTypeNum.value(ColumnDebit) == -1)     // If amount previously selected, set check radio_amount
    ui->radioBnk_amount->setChecked(true);
  else                                     // ...else set check radio_debCred to clear amount col
    ui->radioBnk_debCred->setChecked(true);
}

int BankingPage::nextId() const
{
  return CSVWizard::PageFormats;
}

bool BankingPage::isComplete() const
{
  return ui->comboBoxBnk_dateCol->currentIndex() > -1 &&
         ui->comboBoxBnk_payeeCol->currentIndex() > -1 &&
        (ui->comboBoxBnk_amountCol->currentIndex() > -1 ||
        (ui->comboBoxBnk_debitCol->currentIndex() > -1 &&
         ui->comboBoxBnk_creditCol->currentIndex() > -1));
}

bool BankingPage::validateMemoComboBox()
{
  if (m_wiz->m_memoColList.count() == 0)
    return true;
  for (int i = 0; i < ui->comboBoxBnk_memoCol->count(); ++i)
  {
    QString txt = ui->comboBoxBnk_memoCol->itemText(i);
    if (txt.contains(QChar(QLatin1Char('*'))))  // check if text containing '*' belongs to valid column types
      if (m_colNumType.value(i) != ColumnPayee) {
        ui->comboBoxBnk_memoCol->setItemText(i, QString().setNum(i + 1));
        m_wiz->m_memoColList.removeOne(i);
        return false;
      }
  }
  return true;
}

void BankingPage::slotMemoColSelected(int col)
{
  if (m_colNumType.value(col) == ColumnPayee ) {
    int rc = KMessageBox::Yes;
    if (isVisible())
      rc = KMessageBox::questionYesNo(m_wiz, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                              "<center>If you wish to copy the Payee data to the memo field, click 'Yes'.</center>",
                                              m_colTypeName.value(m_colNumType[col])));
    if (rc == KMessageBox::Yes) {
      ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + QChar(QLatin1Char('*')));
      if (!m_wiz->m_memoColList.contains(col))
        m_wiz->m_memoColList.append(col);
    } else {
      ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1));
      m_wiz->m_memoColList.removeOne(col);
    }
    //allow only separate memo field occupy combobox
    ui->comboBoxBnk_memoCol->blockSignals(true);
    if (m_colTypeNum.value(ColumnMemo) != -1)
      ui->comboBoxBnk_memoCol->setCurrentIndex(m_colTypeNum.value(ColumnMemo));
    else
      ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
    ui->comboBoxBnk_memoCol->blockSignals(false);
    return;
  }

  if (m_colTypeNum.value(ColumnMemo) != -1)        // check if this memo has any column 'number' assigned...
    m_wiz->m_memoColList.removeOne(col);           // ...if true remove it from memo list

  if(validateSelectedColumn(col, ColumnMemo))
    if (col != - 1 && !m_wiz->m_memoColList.contains(col))
      m_wiz->m_memoColList.append(col);
}

void BankingPage::slotCategoryColSelected(int col)
{
  validateSelectedColumn(col, ColumnCategory);
}

void BankingPage::slotNumberColSelected(int col)
{
  validateSelectedColumn(col, ColumnNumber);
}

void BankingPage::slotPayeeColSelected(int col)
{
  if (validateSelectedColumn(col, ColumnPayee))
    if (!validateMemoComboBox())  // user could have it already in memo so...
      slotMemoColSelected(col);    // ...if true set memo field again
}

void BankingPage::slotDateColSelected(int col)
{
  validateSelectedColumn(col, ColumnDate);
}

void BankingPage::slotDebitColSelected(int col)
{
  validateSelectedColumn(col, ColumnDebit);
}

void BankingPage::slotCreditColSelected(int col)
{
  validateSelectedColumn(col, ColumnCredit);
}

void BankingPage::slotAmountColSelected(int col)
{
  validateSelectedColumn(col, ColumnAmount);
}

void BankingPage::slotAmountToggled(bool checked)
{
  if (checked) {
    ui->comboBoxBnk_amountCol->setEnabled(true);  //  disable credit & debit ui choices
    ui->labelBnk_amount->setEnabled(true);
    ui->labelBnk_credits->setEnabled(false);
    ui->labelBnk_debits->setEnabled(false);

    ui->comboBoxBnk_debitCol->setEnabled(false);
    ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
    ui->comboBoxBnk_creditCol->setEnabled(false);
    ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
  }
}

void BankingPage::slotDebitCreditToggled(bool checked)
{
  if (checked) {
    ui->comboBoxBnk_debitCol->setEnabled(true);  //         if 'debit/credit' selected
    ui->labelBnk_debits->setEnabled(true);
    ui->comboBoxBnk_creditCol->setEnabled(true);
    ui->labelBnk_credits->setEnabled(true);

    ui->comboBoxBnk_amountCol->setEnabled(false);  //       disable 'amount' ui choices
    ui->comboBoxBnk_amountCol->setCurrentIndex(-1);  //     as credit/debit chosen
    ui->labelBnk_amount->setEnabled(false);
  }
}

void BankingPage::slotOppositeSignsClicked(bool checked)
{
  m_oppositeSigns = checked;
}

void CSVWizard::slotClose()
{
  if (m_profileType == ProfileBank)
    m_pageBanking->saveSettings();
  else if (m_profileType == ProfileInvest)
    m_pageInvestment->saveSettings();
  close();
}

void BankingPage::slotClearColumns()
{
  ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
  ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
  ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
  ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
  ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
  ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
  ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
  ui->comboBoxBnk_categoryCol->setCurrentIndex(-1);
}

void BankingPage::resetComboBox(columnTypeE comboBox)
{
  switch (comboBox) {
    case ColumnAmount:
      ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
      break;
    case ColumnCredit:
      ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
      break;
    case ColumnDate:
      ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
      break;
    case ColumnDebit:
      ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
      break;
    case ColumnMemo:
      ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
      break;
    case ColumnNumber:
      ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
      break;
    case ColumnPayee:
      ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
      break;
    case ColumnCategory:
      ui->comboBoxBnk_categoryCol->setCurrentIndex(-1);
      break;
    default:
      KMessageBox::sorry(m_wiz, i18n("<center>Field name not recognised.</center> <center>'<b>%1</b>'</center> Please re-enter your column selections."
                                    , comboBox), i18n("CSV import"));
  }
}

bool BankingPage::validateSelectedColumn(int col, columnTypeE type)
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
  emit completeChanged();
  return true;
}

void BankingPage::saveSettings()
{
  KConfigGroup profileNamesGroup(m_wiz->m_config, "ProfileNames");
  profileNamesGroup.writeEntry("Bank", m_wiz->m_profileList);
  profileNamesGroup.writeEntry("PriorBank", m_wiz->m_profileList.indexOf(m_wiz->m_profileName));
  profileNamesGroup.config()->sync();

  KConfigGroup profilesGroup(m_wiz->m_config, "Bank-" + m_wiz->m_profileName);
  if (m_wiz->m_inFileName.startsWith(QStringLiteral("/home/"))) // replace /home/user with ~/ for brevity
  {
    QFileInfo fileInfo = QFileInfo(m_wiz->m_inFileName);
    if (fileInfo.isFile())
      m_wiz->m_inFileName = fileInfo.absolutePath();
    m_wiz->m_inFileName = QStringLiteral("~/") + m_wiz->m_inFileName.section('/',3);
  }

  profilesGroup.writeEntry("Directory", m_wiz->m_inFileName);
  profilesGroup.writeEntry("Encoding", m_wiz->m_encodeIndex);
  profilesGroup.writeEntry("DateFormat", m_wiz->m_dateFormatIndex);
  profilesGroup.writeEntry("OppositeSigns", m_oppositeSigns);
  profilesGroup.writeEntry("FieldDelimiter", m_wiz->m_fieldDelimiterIndex);
  profilesGroup.writeEntry("TextDelimiter", m_wiz->m_textDelimiterIndex);
  profilesGroup.writeEntry("DecimalSymbol", m_wiz->m_decimalSymbolIndex);
  profilesGroup.writeEntry("StartLine", m_wiz->m_startLine - 1);
  profilesGroup.writeEntry("TrailerLines", m_wiz->m_trailerLines);

  profilesGroup.writeEntry("DateCol", m_colTypeNum.value(ColumnDate));
  profilesGroup.writeEntry("PayeeCol", m_colTypeNum.value(ColumnPayee));

  QList<int> list = m_wiz->m_memoColList;
  int posn = 0;
  if ((posn = list.indexOf(-1)) > -1) {
    list.removeOne(-1);
  }
  profilesGroup.writeEntry("MemoCol", list);

  profilesGroup.writeEntry("NumberCol", ui->comboBoxBnk_numberCol->currentIndex());
  profilesGroup.writeEntry("AmountCol", ui->comboBoxBnk_amountCol->currentIndex());
  profilesGroup.writeEntry("DebitCol", ui->comboBoxBnk_debitCol->currentIndex());
  profilesGroup.writeEntry("CreditCol", ui->comboBoxBnk_creditCol->currentIndex());
  profilesGroup.writeEntry("CategoryCol", ui->comboBoxBnk_categoryCol->currentIndex());
  profilesGroup.config()->sync();
}

void BankingPage::readSettings(const KSharedConfigPtr& config)
{
  for (int i = 0; i < m_wiz->m_profileList.count(); ++i) {
    if (m_wiz->m_profileList[i] != m_wiz->m_profileName)
      continue;
    KConfigGroup profilesGroup(config, QStringLiteral("Bank-") + m_wiz->m_profileList[i]);
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

    if (m_wiz->m_decimalSymbolIndex != -1 && m_wiz->m_decimalSymbolIndex != 2) {
      m_wiz->m_parse->setDecimalSymbolIndex(m_wiz->m_decimalSymbolIndex);
      m_wiz->m_parse->setDecimalSymbol(m_wiz->m_decimalSymbolIndex);

      m_wiz->m_parse->setThousandsSeparatorIndex(m_wiz->m_decimalSymbolIndex);
      m_wiz->m_parse->setThousandsSeparator(m_wiz->m_decimalSymbolIndex);

      m_wiz->m_decimalSymbol = m_wiz->m_parse->decimalSymbol(m_wiz->m_decimalSymbolIndex);

    } else
      m_wiz->m_decimalSymbol.clear();

    m_wiz->m_parse->setFieldDelimiterIndex(m_wiz->m_fieldDelimiterIndex);
    m_wiz->m_parse->setTextDelimiterIndex(m_wiz->m_textDelimiterIndex);
    m_wiz->m_fieldDelimiterCharacter = m_wiz->m_parse->fieldDelimiterCharacter(m_wiz->m_fieldDelimiterIndex);
    m_wiz->m_textDelimiterCharacter = m_wiz->m_parse->textDelimiterCharacter(m_wiz->m_textDelimiterIndex);
    m_wiz->m_startLine = profilesGroup.readEntry("StartLine", 0) + 1;
    m_wiz->m_trailerLines = profilesGroup.readEntry("TrailerLines", 0);
    m_wiz->m_encodeIndex = profilesGroup.readEntry("Encoding", 0);
    break;
  }
}

void BankingPage::makeQIF(MyMoneyStatement& st, QFile& file)
{
  QTextStream out(&file);

  QString buffer;
  QString strEType;

  switch (st.m_eType) {
  case MyMoneyStatement::etCreditCard:
    strEType = QStringLiteral("CCard");
  case MyMoneyStatement::etSavings:
  case MyMoneyStatement::etCheckings:
  default:
    strEType = QStringLiteral("Bank");
  }

  if (!st.m_strAccountName.isEmpty()) {
    buffer.append(QStringLiteral("!Account\n"));
    buffer.append(QChar(QLatin1Char('N')) + st.m_strAccountName + QChar(QLatin1Char('\n')));
    buffer.append(QChar(QLatin1Char('T')) + strEType + QChar(QLatin1Char('\n')));
    buffer.append(QStringLiteral("^\n"));
  }

  buffer.append(QStringLiteral("!Type:") + strEType + QChar(QLatin1Char('\n')));

  for (QList<MyMoneyStatement::Transaction>::const_iterator it = st.m_listTransactions.constBegin(); it != st.m_listTransactions.constEnd(); ++it) {
    buffer.append(QChar(QLatin1Char('D')) + it->m_datePosted.toString(QStringLiteral("MM/dd/yyyy")) + QChar(QLatin1Char('\n')));
    QString txt;
    txt.setNum(it->m_amount.toDouble(), 'f', 4);
    buffer.append(QChar(QLatin1Char('T')) + txt + QChar(QLatin1Char('\n')));
    buffer.append(QChar(QLatin1Char('P')) + it->m_strPayee + QChar(QLatin1Char('\n')));
    if (!it->m_listSplits.isEmpty())
      buffer.append(QChar(QLatin1Char('L')) + it->m_listSplits.first().m_strCategoryName + QChar(QLatin1Char('\n')));
    if (!it->m_strNumber.isEmpty())
      buffer.append(QChar(QLatin1Char('N')) + it->m_strNumber + QChar(QLatin1Char('\n')));
    if (!it->m_strMemo.isEmpty())
      buffer.append(QChar(QLatin1Char('M')) + it->m_strMemo + QChar(QLatin1Char('\n')));
    buffer.append(QStringLiteral("^\n"));
    out << buffer;// output qif file
    buffer.clear();
  }
}

bool BankingPage::createStatement(MyMoneyStatement& st)
{
  if (!st.m_listTransactions.isEmpty()) // don't create statement if there is one
    return true;

  st.m_eType = MyMoneyStatement::etNone;
  if (m_wiz->m_autodetect.value(CSVWizard::AutoAccountBank))
    m_wiz->detectAccount(st);

  m_hashSet.clear();
  for (int line = m_wiz->m_startLine - 1; line < m_wiz->m_endLine; ++line)
    if (!processBankLine(m_wiz->m_lineList[line], st)) // parse fields
      return false;
  return true;
}

bool BankingPage::processBankLine(const QString &line, MyMoneyStatement &st)
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

  for (int i = 0; i < m_columnList.count(); ++i) {
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
    memo.append(m_columnList[m_colTypeNum[ColumnMemo]]);

  for (int i = 0; i < m_wiz->m_memoColList.count(); ++i) {
    if (m_wiz->m_memoColList[i] != m_colTypeNum[ColumnMemo]) {
      if (!memo.isEmpty())
        memo.append(QChar(QLatin1Char('\n')));
      memo.append(m_columnList[m_wiz->m_memoColList[i]]);
    }
  }
  tr.m_strMemo = memo;

  // process amount field
  if (m_colTypeNum.value(ColumnAmount) != -1) {
    ++neededFieldsCount;
    if (m_wiz->m_decimalSymbolIndex == 2) {
      int decimalSymbolIndex = m_wiz->m_decimalSymbolIndexMap.value(m_colTypeNum[ColumnAmount]);
      m_wiz->m_parse->setDecimalSymbol(decimalSymbolIndex);
      m_wiz->m_parse->setThousandsSeparator(decimalSymbolIndex);
    }

    txt = m_columnList[m_colTypeNum[ColumnAmount]];
    if (txt.startsWith(QChar(QLatin1Char('(')))) { // check if brackets notation is used for negative numbers
      txt.remove(QRegularExpression(QStringLiteral("[()]")));
      txt.prepend(QChar(QLatin1Char('-')));
    }

    if (m_oppositeSigns) {  // change signs to opposite if requested by user
      if(txt.startsWith(QChar(QLatin1Char('-'))))
        txt.remove(0, 1);
      else if (txt.startsWith(QChar(QLatin1Char('+'))))
        txt.replace(0, 1, QChar(QLatin1Char('-')));
      else
        txt.prepend(QChar(QLatin1Char('-')));
    }
    if (txt.isEmpty())
      tr.m_amount = MyMoneyMoney();
    else
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
  for (uchar idx = 0; idx < 0xFF; ++idx) {  // assuming threre will be no more than 256 transactions with the same hashBase
    hash = QString("%1-%2").arg(hashBase).arg(idx);
    QSet<QString>::const_iterator it = m_hashSet.constFind(hash);
    if (it == m_hashSet.constEnd())
      break;
  }
  m_hashSet.insert(hash);
  tr.m_strBankID = hash;

  st.m_listTransactions.append(tr); // Add the transaction to the statement
  return true;
}

bool BankingPage::processCreditDebit(QString& credit, QString& debit , MyMoneyMoney& amount)
{
  QString decimalSymbol = m_wiz->m_decimalSymbol;
  if (m_wiz->m_decimalSymbolIndex == 2) {
    int decimalSymbolIndex = m_wiz->m_decimalSymbolIndexMap.value(m_colTypeNum[ColumnCredit]);
    decimalSymbol = m_wiz->m_parse->decimalSymbol(decimalSymbolIndex);
    m_wiz->m_parse->setDecimalSymbol(decimalSymbolIndex);
    m_wiz->m_parse->setThousandsSeparator(decimalSymbolIndex);
  }

  if (credit.startsWith(QChar(QLatin1Char('(')))) { // check if brackets notation is used for negative numbers
    credit.remove(QRegularExpression(QStringLiteral("[()]")));
    credit.prepend(QChar(QLatin1Char('-')));
  }
  if (debit.startsWith(QChar(QLatin1Char('(')))) { // check if brackets notation is used for negative numbers
    debit.remove(QRegularExpression(QStringLiteral("[()]")));
    debit.prepend(QChar(QLatin1Char('-')));
  }

  if (!credit.isEmpty() && !debit.isEmpty()) {  // we do not expect both fields to be non-zero
    if (MyMoneyMoney(credit).isZero())
      credit = QString();
    if (MyMoneyMoney(debit).isZero())
      debit = QString();
  }

  if (!debit.startsWith(QChar(QLatin1Char('-'))) && !debit.isEmpty()) // ensure debit field is negative
    debit.prepend(QChar(QLatin1Char('-')));

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
  } else {
    amount = MyMoneyMoney();    // both fields are empty and zero so set amount to zero

  }

  return true;
}
