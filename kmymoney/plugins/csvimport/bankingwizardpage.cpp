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

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "csvwizard.h"

#include "ui_bankingwizardpage.h"

// ----------------------------------------------------------------------------

BankingPage::BankingPage(CSVWizard *dlg, CSVImporter *imp) :
  CSVWizardPage(dlg, imp),
  ui(new Ui::BankingPage)
{
  ui->setupUi(this);
  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);

  connect(ui->m_clear, &QAbstractButton::clicked, this, &BankingPage::clearColumns);
  connect(ui->m_radioAmount, &QAbstractButton::toggled, this, &BankingPage::amountToggled);
  connect(ui->m_radioDebitCredit, &QAbstractButton::toggled, this, &BankingPage::debitCreditToggled);
  connect(ui->m_oppositeSigns, &QAbstractButton::clicked, this, &BankingPage::oppositeSignsClicked);

  // initialize column names
  m_dlg->m_colTypeName.insert(ColumnPayee,i18n("Payee"));
  m_dlg->m_colTypeName.insert(ColumnNumber,i18n("Number"));
  m_dlg->m_colTypeName.insert(ColumnDebit,i18n("Debit"));
  m_dlg->m_colTypeName.insert(ColumnCredit,i18n("Credit"));
  m_dlg->m_colTypeName.insert(ColumnDate,i18n("Date"));
  m_dlg->m_colTypeName.insert(ColumnAmount,i18n("Amount"));
  m_dlg->m_colTypeName.insert(ColumnCategory,i18n("Category"));
  m_dlg->m_colTypeName.insert(ColumnMemo,i18n("Memo"));

  m_profile = dynamic_cast<BankingProfile *>(m_imp->m_profile);
}

BankingPage::~BankingPage()
{
  delete ui;
}

void BankingPage::initializeComboBoxes()
{
  // disable banking widgets allowing their initialization
  disconnect(ui->m_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(amountColSelected(int)));
  disconnect(ui->m_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(debitColSelected(int)));
  disconnect(ui->m_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(creditColSelected(int)));
  disconnect(ui->m_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColSelected(int)));
  disconnect(ui->m_numberCol, SIGNAL(currentIndexChanged(int)), this, SLOT(numberColSelected(int)));
  disconnect(ui->m_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColSelected(int)));
  disconnect(ui->m_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(payeeColSelected(int)));
  disconnect(ui->m_categoryCol, SIGNAL(currentIndexChanged(int)), this, SLOT(categoryColSelected(int)));

  // clear all existing items before adding new ones
  ui->m_numberCol->clear();
  ui->m_dateCol->clear();
  ui->m_payeeCol->clear();
  ui->m_memoCol->clear();
  ui->m_amountCol->clear();
  ui->m_creditCol->clear();
  ui->m_debitCol->clear();
  ui->m_categoryCol->clear();

  QStringList columnNumbers;
  for (int i = 0; i < m_imp->m_file->m_columnCount; ++i)
    columnNumbers.append(QString::number(i + 1));

  // populate comboboxes with col # values
  ui->m_numberCol->addItems(columnNumbers);
  ui->m_dateCol->addItems(columnNumbers);
  ui->m_payeeCol->addItems(columnNumbers);
  ui->m_memoCol->addItems(columnNumbers);
  ui->m_amountCol->addItems(columnNumbers);
  ui->m_creditCol->addItems(columnNumbers);
  ui->m_debitCol->addItems(columnNumbers);
  ui->m_categoryCol->addItems(columnNumbers);

  clearColumns(); // all comboboxes are set to 0 so set them to -1
  connect(ui->m_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(amountColSelected(int)));
  connect(ui->m_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(debitColSelected(int)));
  connect(ui->m_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(creditColSelected(int)));
  connect(ui->m_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColSelected(int)));
  connect(ui->m_numberCol, SIGNAL(currentIndexChanged(int)), this, SLOT(numberColSelected(int)));
  connect(ui->m_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColSelected(int)));
  connect(ui->m_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(payeeColSelected(int)));
  connect(ui->m_categoryCol, SIGNAL(currentIndexChanged(int)), this, SLOT(categoryColSelected(int)));
}

void BankingPage::initializePage()
{
  if (ui->m_dateCol->count() != m_imp->m_file->m_columnCount)
    initializeComboBoxes();

  ui->m_payeeCol->setCurrentIndex(m_profile->m_colTypeNum.value(ColumnPayee));
  ui->m_numberCol->setCurrentIndex(m_profile->m_colTypeNum.value(ColumnNumber));
  ui->m_amountCol->setCurrentIndex(m_profile->m_colTypeNum.value(ColumnAmount));
  ui->m_debitCol->setCurrentIndex(m_profile->m_colTypeNum.value(ColumnDebit));
  ui->m_creditCol->setCurrentIndex(m_profile->m_colTypeNum.value(ColumnCredit));
  ui->m_dateCol->setCurrentIndex(m_profile->m_colTypeNum.value(ColumnDate));
  ui->m_categoryCol->setCurrentIndex(m_profile->m_colTypeNum.value(ColumnCategory));
  ui->m_oppositeSigns->setChecked(m_profile->m_oppositeSigns);

  if (m_profile->m_memoColList.count() > 0)
  {
    for (int i = 0; i < m_profile->m_memoColList.count(); ++i)
      ui->m_memoCol->setCurrentIndex(m_profile->m_memoColList.value(i));
  } else
    ui->m_memoCol->setCurrentIndex(-1);

  if (this->m_profile->m_colTypeNum.value(ColumnDebit) == -1)     // If amount previously selected, set check radio_amount
    ui->m_radioAmount->setChecked(true);
  else                                     // ...else set check radio_debCred to clear amount col
    ui->m_radioDebitCredit->setChecked(true);
}

int BankingPage::nextId() const
{
  return CSVWizard::PageFormats;
}

bool BankingPage::isComplete() const
{
  return ui->m_dateCol->currentIndex() > -1 &&
         ui->m_payeeCol->currentIndex() > -1 &&
        (ui->m_amountCol->currentIndex() > -1 ||
        (ui->m_debitCol->currentIndex() > -1 &&
         ui->m_creditCol->currentIndex() > -1));
}

bool BankingPage::validateMemoComboBox()
{
  if (m_profile->m_memoColList.count() == 0)
    return true;
  for (int i = 0; i < ui->m_memoCol->count(); ++i)
  {
    QString txt = ui->m_memoCol->itemText(i);
    if (txt.contains(QChar(QLatin1Char('*'))))  // check if text containing '*' belongs to valid column types
      if (m_profile->m_colNumType.value(i) != ColumnPayee) {
        ui->m_memoCol->setItemText(i, QString::number(i + 1));
        m_profile->m_memoColList.removeOne(i);
        return false;
      }
  }
  return true;
}

void BankingPage::memoColSelected(int col)
{
  if (m_profile->m_colNumType.value(col) == ColumnPayee ) {
    int rc = KMessageBox::Yes;
    if (isVisible())
      rc = KMessageBox::questionYesNo(m_dlg, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                              "<center>If you wish to copy the Payee data to the memo field, click 'Yes'.</center>",
                                              m_dlg->m_colTypeName.value(m_profile->m_colNumType.value(col))));
    if (rc == KMessageBox::Yes) {
      ui->m_memoCol->setItemText(col, QString::number(col + 1) + QChar(QLatin1Char('*')));
      if (!m_profile->m_memoColList.contains(col))
        m_profile->m_memoColList.append(col);
    } else {
      ui->m_memoCol->setItemText(col, QString::number(col + 1));
      m_profile->m_memoColList.removeOne(col);
    }
    //allow only separate memo field occupy combobox
    ui->m_memoCol->blockSignals(true);
    if (m_profile->m_colTypeNum.value(ColumnMemo) != -1)
      ui->m_memoCol->setCurrentIndex(m_profile->m_colTypeNum.value(ColumnMemo));
    else
      ui->m_memoCol->setCurrentIndex(-1);
    ui->m_memoCol->blockSignals(false);
    return;
  }

  if (m_profile->m_colTypeNum.value(ColumnMemo) != -1)        // check if this memo has any column 'number' assigned...
    m_profile->m_memoColList.removeOne(col);           // ...if true remove it from memo list

  if(validateSelectedColumn(col, ColumnMemo))
    if (col != - 1 && !m_profile->m_memoColList.contains(col))
      m_profile->m_memoColList.append(col);
}

void BankingPage::categoryColSelected(int col)
{
  validateSelectedColumn(col, ColumnCategory);
}

void BankingPage::numberColSelected(int col)
{
  validateSelectedColumn(col, ColumnNumber);
}

void BankingPage::payeeColSelected(int col)
{
  if (validateSelectedColumn(col, ColumnPayee))
    if (!validateMemoComboBox())  // user could have it already in memo so...
      memoColSelected(col);    // ...if true set memo field again
}

void BankingPage::dateColSelected(int col)
{
  validateSelectedColumn(col, ColumnDate);
}

void BankingPage::debitColSelected(int col)
{
  validateSelectedColumn(col, ColumnDebit);
}

void BankingPage::creditColSelected(int col)
{
  validateSelectedColumn(col, ColumnCredit);
}

void BankingPage::amountColSelected(int col)
{
  validateSelectedColumn(col, ColumnAmount);
}

void BankingPage::amountToggled(bool checked)
{
  if (checked) {
    ui->m_amountCol->setEnabled(true);  //  disable credit & debit ui choices
    ui->labelBnk_amount->setEnabled(true);
    ui->labelBnk_credits->setEnabled(false);
    ui->labelBnk_debits->setEnabled(false);

    ui->m_debitCol->setEnabled(false);
    ui->m_debitCol->setCurrentIndex(-1);
    ui->m_creditCol->setEnabled(false);
    ui->m_creditCol->setCurrentIndex(-1);
  }
}

void BankingPage::debitCreditToggled(bool checked)
{
  if (checked) {
    ui->m_debitCol->setEnabled(true);  //         if 'debit/credit' selected
    ui->labelBnk_debits->setEnabled(true);
    ui->m_creditCol->setEnabled(true);
    ui->labelBnk_credits->setEnabled(true);

    ui->m_amountCol->setEnabled(false);  //       disable 'amount' ui choices
    ui->m_amountCol->setCurrentIndex(-1);  //     as credit/debit chosen
    ui->labelBnk_amount->setEnabled(false);
  }
}

void BankingPage::oppositeSignsClicked(bool checked)
{
  m_profile->m_oppositeSigns = checked;
}

void BankingPage::clearColumns()
{
  ui->m_dateCol->setCurrentIndex(-1);
  ui->m_payeeCol->setCurrentIndex(-1);
  ui->m_memoCol->setCurrentIndex(-1);
  ui->m_numberCol->setCurrentIndex(-1);
  ui->m_amountCol->setCurrentIndex(-1);
  ui->m_debitCol->setCurrentIndex(-1);
  ui->m_creditCol->setCurrentIndex(-1);
  ui->m_categoryCol->setCurrentIndex(-1);
}

void BankingPage::resetComboBox(const columnTypeE comboBox)
{
  switch (comboBox) {
    case ColumnAmount:
      ui->m_amountCol->setCurrentIndex(-1);
      break;
    case ColumnCredit:
      ui->m_creditCol->setCurrentIndex(-1);
      break;
    case ColumnDate:
      ui->m_dateCol->setCurrentIndex(-1);
      break;
    case ColumnDebit:
      ui->m_debitCol->setCurrentIndex(-1);
      break;
    case ColumnMemo:
      ui->m_memoCol->setCurrentIndex(-1);
      break;
    case ColumnNumber:
      ui->m_numberCol->setCurrentIndex(-1);
      break;
    case ColumnPayee:
      ui->m_payeeCol->setCurrentIndex(-1);
      break;
    case ColumnCategory:
      ui->m_categoryCol->setCurrentIndex(-1);
      break;
    default:
      KMessageBox::sorry(m_dlg, i18n("<center>Field name not recognised.</center> <center>'<b>%1</b>'</center> Please re-enter your column selections."
                                    , comboBox), i18n("CSV import"));
  }
}

bool BankingPage::validateSelectedColumn(const int col, const columnTypeE type)
{
  if (m_profile->m_colTypeNum.value(type) != -1)        // check if this 'type' has any column 'number' assigned...
    m_profile->m_colNumType.remove(m_profile->m_colTypeNum[type]); // ...if true remove 'type' assigned to this column 'number'

  bool ret = true;
  if (col == -1) { // user only wanted to reset his column so allow him
    m_profile->m_colTypeNum[type] = col;  // assign new column 'number' to this 'type'
  } else if (m_profile->m_colNumType.contains(col)) { // if this column 'number' has already 'type' assigned
    KMessageBox::information(m_dlg, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>",
                                     m_dlg->m_colTypeName.value(m_profile->m_colNumType.value(col))));
    resetComboBox(m_profile->m_colNumType.value(col));
    resetComboBox(type);
    ret = false;
  } else {
    m_profile->m_colTypeNum[type] = col; // assign new column 'number' to this 'type'
    m_profile->m_colNumType[col] = type; // assign new 'type' to this column 'number'
  }
  emit completeChanged();
  return ret;
}

bool BankingPage::validateCreditDebit()
{
  for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row) {
    // process credit/debit field
    if (m_profile->m_colTypeNum.value(ColumnCredit) != -1 &&
        m_profile->m_colTypeNum.value(ColumnDebit) != -1) {
      QString credit = m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(ColumnCredit))->text();
      QString debit = m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(ColumnDebit))->text();
      m_imp->processCreditDebit(credit, debit);
      if (!credit.isEmpty() && !debit.isEmpty()) {
        int ret = KMessageBox::questionYesNoCancel(m_dlg,
                                                   i18n("<center>The %1 field contains '%2'</center>"
                                                        "<center>and the %3 field contains '%4'.</center>"
                                                        "<center>Please choose which you wish to accept.</center>",
                                                        m_dlg->m_colTypeName.value(ColumnDebit),
                                                        debit,
                                                        m_dlg->m_colTypeName.value(ColumnCredit),
                                                        credit),
                                                   i18n("CSV invalid field values"),
                                                   KGuiItem(i18n("Accept %1", m_dlg->m_colTypeName.value(ColumnDebit))),
                                                   KGuiItem(i18n("Accept %1", m_dlg->m_colTypeName.value(ColumnCredit))),
                                                   KGuiItem(i18n("Cancel")));
        switch(ret) {
          case KMessageBox::Cancel:
            return false;
          case KMessageBox::Yes:
            m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(ColumnCredit))->setText(QString());
            break;
          case KMessageBox::No:
            m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(ColumnDebit))->setText(QString());
            break;
        }
      }
    }
  }
  return true;
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
