/*******************************************************************************
*                                 investmentwizardpage.cpp
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

#include "investmentwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QtCore/QTextStream>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KConfigGroup>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"

#include "csvwizard.h"
#include "convdate.h"
#include "csvutil.h"

#include "transactiondlg.h"
#include "securitydlg.h"
#include "securitiesdlg.h"

#include "ui_investmentwizardpage.h"
#include "ui_csvwizard.h"

// ----------------------------------------------------------------------------

InvestmentPage::InvestmentPage(QDialog *parent) : QWizardPage(parent), ui(new Ui::InvestmentPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);

  connect(ui->button_clear, SIGNAL(clicked()), this, SLOT(slotClearColumns()));
  connect(ui->buttonInv_clearFee, SIGNAL(clicked()), this, SLOT(slotClearFee()));
  connect(ui->buttonInv_calculateFee, SIGNAL(clicked()), this, SLOT(slotCalculateFee()));

  // initialize column names
  m_colTypeName.insert(ColumnType, i18n("Type"));
  m_colTypeName.insert(ColumnPrice, i18n("Price"));
  m_colTypeName.insert(ColumnQuantity, i18n("Quantity"));
  m_colTypeName.insert(ColumnFee, i18n("Fee"));
  m_colTypeName.insert(ColumnDate, i18n("Date"));
  m_colTypeName.insert(ColumnAmount, i18n("Amount"));
  m_colTypeName.insert(ColumnSymbol, i18n("Symbol"));
  m_colTypeName.insert(ColumnName, i18n("Name"));
  m_colTypeName.insert(ColumnMemo, i18n("Memo"));

  // initialize operation type names
  m_buyList = QString(i18nc("Type of operation as in financial statement", "buy")).split(',', QString::SkipEmptyParts);
  m_sellList = QString(i18nc("Type of operation as in financial statement", "sell,repurchase")).split(',', QString::SkipEmptyParts);
  m_divXList = QString(i18nc("Type of operation as in financial statement", "dividend")).split(',', QString::SkipEmptyParts);
  m_intIncList = QString(i18nc("Type of operation as in financial statement", "interest,income")).split(',', QString::SkipEmptyParts);
  m_reinvdivList = QString(i18nc("Type of operation as in financial statement", "reinvest,reinv,re-inv")).split(',', QString::SkipEmptyParts);
  m_shrsinList = QString(i18nc("Type of operation as in financial statement", "add,stock dividend,divd reinv,transfer in,re-registration in,journal entry")).split(',', QString::SkipEmptyParts);
  m_shrsoutList = QString(i18nc("Type of operation as in financial statement", "remove")).split(',', QString::SkipEmptyParts);
}

InvestmentPage::~InvestmentPage()
{
  delete ui;
  delete m_securitiesDlg;
}

void InvestmentPage::setParent(CSVWizard* dlg)
{
  m_wiz = dlg;
}

void InvestmentPage::initializeComboBoxes()
{
  // disable investment widgets allowing their initialization
  disconnect(ui->comboBoxInv_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotMemoColSelected(int)));
  disconnect(ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTypeColSelected(int)));
  disconnect(ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColSelected(int)));
  disconnect(ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotQuantityColSelected(int)));
  disconnect(ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPriceColSelected(int)));
  disconnect(ui->comboBoxInv_priceFraction, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFractionChanged(int)));
  disconnect(ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAmountColSelected(int)));
  disconnect(ui->lineEdit_feeRate, SIGNAL(editingFinished()), this, SLOT(slotFeeInputsChanged()));
  disconnect(ui->lineEdit_feeRate, SIGNAL(textChanged(QString)), this, SLOT(slotFeeRateChanged(QString)));
  disconnect(ui->lineEdit_minFee, SIGNAL(textChanged(QString)), this, SLOT(slotMinFeeChanged(QString)));
  disconnect(ui->comboBoxInv_feeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFeeColSelected(int)));
  disconnect(ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSymbolColSelected(int)));
  disconnect(ui->comboBoxInv_nameCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotNameColSelected(int)));
  disconnect(ui->checkBoxInv_feeIsPercentage, SIGNAL(clicked(bool)), this, SLOT(slotFeeIsPercentageClicked(bool)));

  // clear all existing items before adding new ones
  ui->comboBoxInv_amountCol->clear(); // clear all existing items before adding new ones
  ui->comboBoxInv_dateCol->clear();
  ui->comboBoxInv_memoCol->clear();
  ui->comboBoxInv_priceCol->clear();
  ui->comboBoxInv_quantityCol->clear();
  ui->comboBoxInv_typeCol->clear();
  ui->comboBoxInv_feeCol->clear();
  ui->comboBoxInv_symbolCol->clear();
  ui->comboBoxInv_nameCol->clear();
  ui->lineEdit_feeRate->clear();
  ui->lineEdit_minFee->clear();

  QStringList columnNumbers;
  for (int i = 0; i < m_wiz->m_endColumn; ++i) {
    columnNumbers << QString::number(i + 1);
  }

  // populate comboboxes with col # values
  ui->comboBoxInv_amountCol->addItems(columnNumbers);
  ui->comboBoxInv_dateCol->addItems(columnNumbers);
  ui->comboBoxInv_memoCol->addItems(columnNumbers);
  ui->comboBoxInv_priceCol->addItems(columnNumbers);
  ui->comboBoxInv_quantityCol->addItems(columnNumbers);
  ui->comboBoxInv_typeCol->addItems(columnNumbers);
  ui->comboBoxInv_feeCol->addItems(columnNumbers);
  ui->comboBoxInv_symbolCol->addItems(columnNumbers);
  ui->comboBoxInv_nameCol->addItems(columnNumbers);

  slotClearColumns(); // all comboboxes are set to 0 so set them to -1
  connect(ui->comboBoxInv_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotMemoColSelected(int)));
  connect(ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTypeColSelected(int)));
  connect(ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColSelected(int)));
  connect(ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotQuantityColSelected(int)));
  connect(ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPriceColSelected(int)));
  connect(ui->comboBoxInv_priceFraction, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFractionChanged(int)));
  connect(ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAmountColSelected(int)));
  connect(ui->lineEdit_feeRate, SIGNAL(editingFinished()), this, SLOT(slotFeeInputsChanged()));
  connect(ui->lineEdit_feeRate, SIGNAL(textChanged(QString)), this, SLOT(slotFeeRateChanged(QString)));
  connect(ui->lineEdit_minFee, SIGNAL(textChanged(QString)), this, SLOT(slotMinFeeChanged(QString)));
  connect(ui->comboBoxInv_feeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFeeColSelected(int)));
  connect(ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSymbolColSelected(int)));
  connect(ui->comboBoxInv_nameCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotNameColSelected(int)));
  connect(ui->checkBoxInv_feeIsPercentage, SIGNAL(clicked(bool)), this, SLOT(slotFeeIsPercentageClicked(bool)));
}

void InvestmentPage::initializePage()
{
  if (ui->comboBoxInv_dateCol->count() != m_wiz->m_endColumn)
    initializeComboBoxes();
  ui->comboBoxInv_dateCol->setCurrentIndex(m_colTypeNum.value(ColumnDate));
  ui->comboBoxInv_typeCol->setCurrentIndex(m_colTypeNum.value(ColumnType));
  ui->comboBoxInv_priceCol->setCurrentIndex(m_colTypeNum.value(ColumnPrice));
  ui->comboBoxInv_quantityCol->setCurrentIndex(m_colTypeNum.value(ColumnQuantity));
  ui->comboBoxInv_amountCol->setCurrentIndex(m_colTypeNum.value(ColumnAmount));
  ui->comboBoxInv_nameCol->setCurrentIndex(m_colTypeNum.value(ColumnName));
  ui->comboBoxInv_symbolCol->setCurrentIndex(m_colTypeNum.value(ColumnSymbol));
  ui->checkBoxInv_feeIsPercentage->setChecked(m_feeIsPercentage);
  ui->comboBoxInv_feeCol->setCurrentIndex(m_colTypeNum.value(ColumnFee));
  ui->comboBoxInv_priceFraction->setCurrentIndex(m_priceFraction);
  ui->lineEdit_feeRate->setText(m_feeRate);
  ui->lineEdit_minFee->setText(m_minFee);
  ui->lineEdit_feeRate->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9]{1,2}[") + QLocale().decimalPoint() + QStringLiteral("]{1,1}[0-9]{0,2}")), this) );
  ui->lineEdit_minFee->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9]{1,}[") + QLocale().decimalPoint() + QStringLiteral("]{0,1}[0-9]{0,}")), this) );
  slotCalculateFee();

  for (int i = 0; i < m_wiz->m_memoColList.count(); ++i) { //  Set up all memo fields...
    int tmp = m_wiz->m_memoColList[i];
    if (tmp < m_colTypeNum.count())
      ui->comboBoxInv_memoCol->setCurrentIndex(tmp);
  }

  slotFeeInputsChanged();
}

bool InvestmentPage::isComplete() const
{
  return  ui->comboBoxInv_dateCol->currentIndex() > -1 &&
          ui->comboBoxInv_typeCol->currentIndex() > -1 &&
          ui->comboBoxInv_quantityCol->currentIndex() > -1 &&
          ui->comboBoxInv_priceCol->currentIndex() > -1 &&
          ui->comboBoxInv_amountCol->currentIndex() > -1 &&
          ui->comboBoxInv_priceFraction->currentIndex() > -1;
}

bool InvestmentPage::validatePage()
{
  if (ui->comboBoxInv_symbolCol->currentIndex() == -1 &&
      ui->comboBoxInv_nameCol->currentIndex() == -1)
    return validateSecurity();
  else
    return validateSecurities();
}

void InvestmentPage::cleanupPage()
{
  clearFeeCol();
}

void InvestmentPage::slotMemoColSelected(int col)
{
  if (m_colNumType.value(col) == ColumnType ||
      m_colNumType.value(col) == ColumnName) {
    int rc = KMessageBox::Yes;
    if (isVisible())
      rc = KMessageBox::questionYesNo(m_wiz, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                              "<center>If you wish to copy that data to the memo field, click 'Yes'.</center>",
                                              m_colTypeName.value(m_colNumType[col])));
    if (rc == KMessageBox::Yes) {
      ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1) + QChar(QLatin1Char('*')));
      if (!m_wiz->m_memoColList.contains(col))
        m_wiz->m_memoColList.append(col);
    } else {
      ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1));
      m_wiz->m_memoColList.removeOne(col);
    }
    //allow only separate memo field occupy combobox
    ui->comboBoxInv_memoCol->blockSignals(true);
    if (m_colTypeNum.value(ColumnMemo) != -1)
      ui->comboBoxInv_memoCol->setCurrentIndex(m_colTypeNum.value(ColumnMemo));
    else
      ui->comboBoxInv_memoCol->setCurrentIndex(-1);
    ui->comboBoxInv_memoCol->blockSignals(false);
    return;
  }

  if (m_colTypeNum.value(ColumnMemo) != -1)        // check if this memo has any column 'number' assigned...
    m_wiz->m_memoColList.removeOne(col);           // ...if true remove it from memo list

  if(validateSelectedColumn(col, ColumnMemo))
    if (col != - 1 && !m_wiz->m_memoColList.contains(col))
      m_wiz->m_memoColList.append(col);
}

void InvestmentPage::slotDateColSelected(int col)
{
  validateSelectedColumn(col, ColumnDate);
}

void InvestmentPage::slotFeeColSelected(int col)
{
  validateSelectedColumn(col, ColumnFee);
  slotFeeInputsChanged();
}

void InvestmentPage::slotTypeColSelected(int col)
{
  if (validateSelectedColumn(col, ColumnType))
    if (!validateMemoComboBox())  // user could have it already in memo so...
      slotMemoColSelected(col);    // ...if true set memo field again
}

void InvestmentPage::slotQuantityColSelected(int col)
{
  validateSelectedColumn(col, ColumnQuantity);
}

void InvestmentPage::slotPriceColSelected(int col)
{
  validateSelectedColumn(col, ColumnPrice);
}

void InvestmentPage::slotAmountColSelected(int col)
{
  validateSelectedColumn(col, ColumnAmount);
  clearFeeCol();
  slotFeeInputsChanged();
}

void InvestmentPage::slotSymbolColSelected(int col)
{
  validateSelectedColumn(col, ColumnSymbol);
  m_mapSymbolName.clear();        // new symbol column so this map is no longer valid
}

void InvestmentPage::slotNameColSelected(int col)
{
  if (validateSelectedColumn(col, ColumnName))
    if (!validateMemoComboBox())  // user could have it already in memo so...
      slotMemoColSelected(col);    // ...if true set memo field again
  m_mapSymbolName.clear();        // new name column so this map is no longer valid
}

void InvestmentPage::slotFeeIsPercentageClicked(bool checked)
{
  m_feeIsPercentage = checked;
}

void InvestmentPage::slotFractionChanged(int col)
{
  m_priceFraction = col;
  m_priceFractionValue = ui->comboBoxInv_priceFraction->itemText(col);
  emit completeChanged();
}

void InvestmentPage::slotClearColumns()
{
  ui->comboBoxInv_amountCol->setCurrentIndex(-1);
  ui->comboBoxInv_dateCol->setCurrentIndex(-1);
  ui->comboBoxInv_priceCol->setCurrentIndex(-1);
  ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
  ui->comboBoxInv_memoCol->setCurrentIndex(-1);
  ui->comboBoxInv_typeCol->setCurrentIndex(-1);
  ui->comboBoxInv_nameCol->setCurrentIndex(-1);
  ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
  ui->comboBoxInv_priceFraction->setCurrentIndex(-1);
  slotClearFee();
}

void InvestmentPage::slotClearFee()
{
  clearFeeCol();
  ui->comboBoxInv_feeCol->setCurrentIndex(-1);
  ui->checkBoxInv_feeIsPercentage->setChecked(false);
  ui->buttonInv_calculateFee->setEnabled(false);
  ui->lineEdit_feeRate->setEnabled(true);
  ui->lineEdit_minFee->setEnabled(false);
  ui->lineEdit_feeRate->clear();
  ui->lineEdit_minFee->clear();
}

void InvestmentPage::slotFeeInputsChanged()
{
  ui->buttonInv_calculateFee->setEnabled(false);
  if (ui->comboBoxInv_feeCol->currentIndex() < m_wiz->m_endColumn &&
      ui->comboBoxInv_feeCol->currentIndex() > -1) {
    ui->lineEdit_minFee->setEnabled(false);
    ui->lineEdit_feeRate->setEnabled(false);
    ui->lineEdit_minFee->clear();
    ui->lineEdit_feeRate->clear();
  } else if (m_feeRate.isEmpty()) {
    ui->comboBoxInv_feeCol->setEnabled(true);
    ui->checkBoxInv_feeIsPercentage->setEnabled(true);
    ui->lineEdit_minFee->setEnabled(false);
  } else {
    ui->comboBoxInv_feeCol->setEnabled(false);
    ui->checkBoxInv_feeIsPercentage->setEnabled(false);
    ui->checkBoxInv_feeIsPercentage->setChecked(true);
    ui->lineEdit_minFee->setEnabled(true);
    ui->lineEdit_feeRate->setEnabled(true);
    if (m_colTypeNum.value(ColumnAmount) != -1)
      ui->buttonInv_calculateFee->setEnabled(true);
  }
}

void InvestmentPage::slotFeeRateChanged(const QString text)
{
  m_feeRate = text;
}

void InvestmentPage::slotMinFeeChanged(const QString text)
{
  m_minFee = text;
}

void InvestmentPage::slotCalculateFee()
{
  QString txt;
  QString newTxt;
  QString decimalSymbol;
  double d;
  bool ok;

  if (m_feeRate.isEmpty() || m_colTypeNum.value(ColumnAmount) == -1) //check if feeRate is in place
    return;

  decimalSymbol = m_wiz->m_decimalSymbol;
  if (m_wiz->m_decimalSymbolIndex == 2 || m_wiz->m_decimalSymbolIndex == -1) {
    int detectedSymbol = 2;
    if (!m_wiz->detectDecimalSymbol(m_colTypeNum.value(ColumnAmount), detectedSymbol))
      return;
    m_wiz->m_parse->setDecimalSymbol(detectedSymbol);
    m_wiz->m_parse->setThousandsSeparator(detectedSymbol); // separator list is in reverse so it's ok
    decimalSymbol = m_wiz->m_parse->decimalSymbol(detectedSymbol);
  }

  MyMoneyMoney percent(m_wiz->m_parse->possiblyReplaceSymbol(m_feeRate));

  if (m_minFee.isEmpty())
    m_minFee = QChar(QLatin1Char('0')) + QLocale().decimalPoint() + QStringLiteral("00");

  MyMoneyMoney minFee(m_wiz->m_parse->possiblyReplaceSymbol(m_minFee));

  if (m_colTypeNum.value(ColumnFee) == -1) { // if fee column not present, add it at the end
    m_colNumType.insert(m_wiz->m_maxColumnCount, ColumnFee);
    m_colTypeNum.insert(ColumnFee, m_wiz->m_maxColumnCount);
  }

  if (m_colTypeNum.value(ColumnFee) >= m_wiz->m_maxColumnCount) // if fee column out of boudary, expand it
    m_wiz->m_maxColumnCount ++;

  m_wiz->ui->tableWidget->setColumnCount(m_wiz->m_maxColumnCount);
  txt.setNum(m_colTypeNum.value(ColumnFee) + 1);
  if (ui->comboBoxInv_feeCol->itemText(m_colTypeNum.value(ColumnFee)) != txt)
    ui->comboBoxInv_feeCol->insertItem(m_colTypeNum.value(ColumnFee), txt);
  ui->comboBoxInv_feeCol->setCurrentIndex(m_colTypeNum.value(ColumnFee)); // ...and select it by default

  for (int i = 0; i <  m_wiz->m_startLine - 1; ++i) { // fill rows above with whitespace for nice effect with markUnwantedRows
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText(QString());
    m_wiz->ui->tableWidget->setItem(i, m_colTypeNum.value(ColumnFee), item);
  }

  for (int i = m_wiz->m_endLine; i <  m_wiz->ui->tableWidget->rowCount(); ++i) { // fill rows below with whitespace for nice effect with markUnwantedRows
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText(QString());
    m_wiz->ui->tableWidget->setItem(i, m_colTypeNum.value(ColumnFee), item);
  }

  for (int i = m_wiz->m_startLine - 1; i < m_wiz->m_endLine; ++i) {
      m_columnList = m_wiz->m_parse->parseLine(m_wiz->m_lineList[i]);
      txt = m_columnList[m_colTypeNum.value(ColumnAmount)];
      txt.remove(QRegularExpression(QStringLiteral("[,. ]"))).toInt(&ok);
      if (!ok)  { // if the line is in statement's header, skip
          m_wiz->m_lineList[i].append(m_wiz->m_fieldDelimiterCharacter);
          continue;
        }
      txt = m_columnList[m_colTypeNum.value(ColumnAmount)];
      txt = txt.remove(m_wiz->m_fieldDelimiterCharacter);
      if (txt.startsWith(QChar(QLatin1Char('(')))) {
        txt.remove(QRegularExpression(QStringLiteral("[()]")));
        txt.prepend(QChar(QLatin1Char('-')));
        }
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney amount(newTxt);
      MyMoneyMoney fee(percent * amount / MyMoneyMoney(100));
      if (fee < minFee)
        fee = minFee;
      d = fee.toDouble();
      txt.setNum(d, 'f', 4);
      txt.replace(QChar(QLatin1Char('.')), decimalSymbol); //make sure decimal symbol is uniform in whole line

      if (decimalSymbol == m_wiz->m_fieldDelimiterCharacter) { //make sure fee has the same notation as the line it's being attached to
        if (m_columnList.count() == m_wiz->m_maxColumnCount)
          m_wiz->m_lineList[i] = m_wiz->m_lineList[i].left(m_wiz->m_lineList[i].length() - txt.length() - 2 * m_wiz->m_textDelimiterCharacter.length() - m_wiz->m_fieldDelimiterCharacter.length());
        m_wiz->m_lineList[i].append(m_wiz->m_fieldDelimiterCharacter + m_wiz->m_textDelimiterCharacter + txt + m_wiz->m_textDelimiterCharacter);
      }
      else {
        if (m_columnList.count() == m_wiz->m_maxColumnCount)
          m_wiz->m_lineList[i] = m_wiz->m_lineList[i].left(m_wiz->m_lineList[i].length()-txt.length() - m_wiz->m_fieldDelimiterCharacter.length() );
        m_wiz->m_lineList[i].append(m_wiz->m_fieldDelimiterCharacter + txt);
      }

      QTableWidgetItem *item = new QTableWidgetItem;
      item->setText(txt);
      m_wiz->ui->tableWidget->setItem(i, m_colTypeNum.value(ColumnFee), item);
    }
  if(isVisible())
    m_wiz->updateWindowSize();
}

void InvestmentPage::clearFeeCol()
{
  if (!m_feeRate.isEmpty() && m_colTypeNum.value(ColumnFee) >= m_wiz->m_endColumn) { //delete fee colum, but only if it was generated
    m_wiz->m_maxColumnCount--;
    m_wiz->ui->tableWidget->setColumnCount(m_wiz->m_maxColumnCount);
    int i = ui->comboBoxInv_feeCol->currentIndex();
    ui->comboBoxInv_feeCol->setCurrentIndex(-1);
    ui->comboBoxInv_feeCol->removeItem(i);
    if(isVisible())
      m_wiz->updateWindowSize();
  }
  ui->comboBoxInv_feeCol->setEnabled(true);
  ui->checkBoxInv_feeIsPercentage->setEnabled(true);
  ui->checkBoxInv_feeIsPercentage->setChecked(false);
}

void InvestmentPage::resetComboBox(const columnTypeE comboBox)
{
  switch (comboBox) {
    case ColumnAmount:
      ui->comboBoxInv_amountCol->setCurrentIndex(-1);
      break;
    case ColumnDate:
      ui->comboBoxInv_dateCol->setCurrentIndex(-1);
      break;
    case ColumnFee:
      ui->comboBoxInv_feeCol->setCurrentIndex(-1);
      break;
    case ColumnMemo:
      ui->comboBoxInv_memoCol->setCurrentIndex(-1);
      break;
    case ColumnPrice:
      ui->comboBoxInv_priceCol->setCurrentIndex(-1);
      break;
    case ColumnQuantity:
      ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
      break;
    case ColumnType:
      ui->comboBoxInv_typeCol->setCurrentIndex(-1);
      break;
    case ColumnSymbol:
      ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
      break;
    case ColumnName:
      ui->comboBoxInv_nameCol->setCurrentIndex(-1);
      break;
    default:
      KMessageBox::sorry(m_wiz, i18n("<center>Field name not recognised.</center><center>'<b>%1</b>'</center>Please re-enter your column selections.", comboBox), i18n("CSV import"));
  }
}

bool InvestmentPage::validateSelectedColumn(int col, columnTypeE type)
{
  if (m_colTypeNum.value(type) != -1)        // check if this 'type' has any column 'number' assigned...
    m_colNumType.remove(m_colTypeNum[type]); // ...if true remove 'type' assigned to this column 'number'

  bool ret = true;
  if (col == -1) { // user only wanted to reset his column so allow him
    m_colTypeNum[type] = col;  // assign new column 'number' to this 'type'
  } else if (m_colNumType.contains(col)) { // if this column 'number' has already 'type' assigned
    KMessageBox::information(m_wiz, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>",
                                     m_colTypeName.value(m_colNumType[col])));
    resetComboBox(m_colNumType[col]);
    resetComboBox(type);
    ret = false;
  } else {
    m_colTypeNum[type] = col; // assign new column 'number' to this 'type'
    m_colNumType[col] = type; // assign new 'type' to this column 'number'
  }
  emit completeChanged();
  return ret;
}

bool InvestmentPage::validateMemoComboBox()
{
  if (m_wiz->m_memoColList.count() == 0)
    return true;
  for (int i = 0; i < ui->comboBoxInv_memoCol->count(); ++i)
  {
    QString txt = ui->comboBoxInv_memoCol->itemText(i);
    if (txt.contains(QChar(QLatin1Char('*'))))  // check if text containing '*' belongs to valid column types
      if (m_colNumType.value(i) != ColumnName &&
          m_colNumType.value(i) != ColumnType) {
        ui->comboBoxInv_memoCol->setItemText(i, QString().setNum(i + 1));
        m_wiz->m_memoColList.removeOne(i);
        return false;
      }
  }
  return true;
}

bool InvestmentPage::createValidActionTypes(QList<MyMoneyStatement::Transaction::EAction> &validActionTypes, MyMoneyStatement::Transaction &tr)
{
  if (tr.m_shares.isPositive() &&
      tr.m_price.isPositive() &&
      !tr.m_amount.isZero())
    validActionTypes << MyMoneyStatement::Transaction::eaReinvestDividend <<
                        MyMoneyStatement::Transaction::eaBuy <<
                        MyMoneyStatement::Transaction::eaSell;
  else if (tr.m_shares.isZero() &&
           tr.m_price.isZero() &&
           !tr.m_amount.isZero())
    validActionTypes << MyMoneyStatement::Transaction::eaCashDividend <<
                        MyMoneyStatement::Transaction::eaInterest;
  else if (tr.m_shares.isPositive() &&
           tr.m_price.isZero() &&
           tr.m_amount.isZero())
    validActionTypes << MyMoneyStatement::Transaction::eaShrsin <<
                        MyMoneyStatement::Transaction::eaShrsout;
  else
    return false;
  return true;
}

void InvestmentPage::storeActionType(MyMoneyStatement::Transaction::EAction &actionType, const QString &userType)
{
  switch(actionType) {
  case MyMoneyStatement::Transaction::eaBuy:
    m_buyList << userType;
    break;
  case MyMoneyStatement::Transaction::eaSell:
    m_sellList << userType;
    break;
  case MyMoneyStatement::Transaction::eaReinvestDividend:
    m_reinvdivList << userType;
    break;
  case MyMoneyStatement::Transaction::eaCashDividend:
    m_divXList << userType;
    break;
  case MyMoneyStatement::Transaction::eaInterest:
    m_intIncList << userType;
    break;
  case MyMoneyStatement::Transaction::eaShrsin:
    m_shrsinList << userType;
    break;
  case MyMoneyStatement::Transaction::eaShrsout:
    m_shrsoutList << userType;
    break;
  default:
    break;
  }
}

bool InvestmentPage::validateActionType(MyMoneyStatement::Transaction &tr, QStringList& colList)
{
  QList<MyMoneyStatement::Transaction::EAction> validActionTypes;
  if (!createValidActionTypes(validActionTypes, tr)) {
    KMessageBox::sorry(m_wiz, i18n("The values in the columns you have selected\ndo not match any expected investment type.\n"
                                   "Please check the fields in the current transaction,\nand also your selections.")
                       , i18n("CSV import"));
    return false;
  }

  if (!validActionTypes.contains(tr.m_eAction)) {
    bool unknownType = false;
    if (tr.m_eAction == MyMoneyStatement::Transaction::eaNone)
      unknownType = true;

    QStringList colHeaders;
    for (int i = 0; i < colList.count(); ++i)
      colHeaders << m_colTypeName.value(m_colNumType.value(i, ColumnInvalid), QString(i18nc("Unused column", "Unused")));

    TransactionDlg* transactionDlg = new TransactionDlg(colList, colHeaders, m_colTypeNum.value(ColumnType), validActionTypes);
    if (transactionDlg->exec() == QDialog::Rejected) {
      KMessageBox::information(m_wiz, i18n("<center>No valid action type found for this transaction.</center>"
                                       "<center>Please check the parameters supplied.</center>"));
      return false;
    } else
      tr.m_eAction = transactionDlg->getActionType();
    delete transactionDlg;

    if (unknownType) // type was unknown so store it
      storeActionType(tr.m_eAction, colList.value(m_colTypeNum[ColumnType]));
  }
  return true;
}

MyMoneyStatement::Transaction::EAction InvestmentPage::processActionType(QString& type)
{
  // most frequent action
  for (QStringList::const_iterator it = m_buyList.constBegin(); it != m_buyList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaBuy;

  // second most frequent action
  for (QStringList::const_iterator it = m_sellList.constBegin(); it != m_sellList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaSell;

  for (QStringList::const_iterator it = m_reinvdivList.constBegin(); it != m_reinvdivList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaReinvestDividend;

  // needs to be after reinvdiv
  for (QStringList::const_iterator it = m_divXList.constBegin(); it != m_divXList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaCashDividend;

  for (QStringList::const_iterator it = m_intIncList.constBegin(); it != m_intIncList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaInterest;

  for (QStringList::const_iterator it = m_shrsinList.constBegin(); it != m_shrsinList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaShrsin;

  for (QStringList::const_iterator it = m_shrsoutList.constBegin(); it != m_shrsoutList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaShrsout;

  return MyMoneyStatement::Transaction::eaNone;
}

bool InvestmentPage::sortSecurities(QSet<QString>& onlySymbols, QSet<QString>& onlyNames, QMap<QString, QString>& mapSymbolName)
{
  QList<MyMoneySecurity> securityList = MyMoneyFile::instance()->securityList();
  int symbolCol = m_colTypeNum.value(ColumnSymbol);
  int nameCol = m_colTypeNum.value(ColumnName);

  // sort by availability of symbol and name
  for (int row = m_wiz->m_startLine - 1; row < m_wiz->m_endLine; ++row) {
    QString symbol;
    QString name;
    if (symbolCol != -1)
      symbol = m_wiz->ui->tableWidget->item(row, symbolCol)->text().trimmed();
    if (nameCol != -1)
      name = m_wiz->ui->tableWidget->item(row, nameCol)->text().trimmed();

    if (!symbol.isEmpty() && !name.isEmpty())
      mapSymbolName.insert(symbol, name);
    else if (!symbol.isEmpty())
      onlySymbols.insert(symbol);
    else if (!name.isEmpty())
      onlyNames.insert(name);
    else
      return false;
  }

  // try to find names for symbols
  for (QSet<QString>::iterator symbol = onlySymbols.begin(); symbol != onlySymbols.end();) {
    QList<MyMoneySecurity> filteredSecurities;
    for (QList<MyMoneySecurity>::ConstIterator secKMM = securityList.constBegin(); secKMM != securityList.constEnd(); ++secKMM) {
      if ((*symbol).compare((*secKMM).tradingSymbol(), Qt::CaseInsensitive) == 0)
        filteredSecurities << (*secKMM);      // gather all securities that by matched by symbol
    }

    if (filteredSecurities.count() == 1) {                                  // single security matched by the symbol so...
      mapSymbolName.insert(*symbol, filteredSecurities.first().name());
      symbol = onlySymbols.erase(symbol);                                       // ...it's no longer unknown
    } else if (!filteredSecurities.isEmpty()) {                             // multiple securities matched by the symbol
      // TODO: Ask user which security should we match to
      mapSymbolName.insert(*symbol, filteredSecurities.first().name());
      symbol = onlySymbols.erase(symbol);
    } else                                                                  // no security matched, so leave it as unknown
      ++symbol;
  }

  // try to find symbols for names
  for (QSet<QString>::iterator name = onlyNames.begin(); name != onlyNames.end();) {
    QList<MyMoneySecurity> filteredSecurities;
    for (QList<MyMoneySecurity>::ConstIterator secKMM = securityList.constBegin(); secKMM != securityList.constEnd(); ++secKMM) {
      if ((*name).compare((*secKMM).name(), Qt::CaseInsensitive) == 0)
        filteredSecurities << (*secKMM);      // gather all securities that by matched by name
    }

    if (filteredSecurities.count() == 1) {                                  // single security matched by the name so...
      mapSymbolName.insert(filteredSecurities.first().tradingSymbol(), *name);
      name = onlyNames.erase(name);                                       // ...it's no longer unknown
    } else if (!filteredSecurities.isEmpty()) {                             // multiple securities matched by the name
      // TODO: Ask user which security should we match to
      mapSymbolName.insert(filteredSecurities.first().tradingSymbol(), *name);
      name = onlySymbols.erase(name);
    } else                                                                  // no security matched, so leave it as unknown
      ++name;
  }
  return true;
}

bool InvestmentPage::validateSecurities()
{
  if (m_securitiesDlg.isNull() &&
      m_mapSymbolName.isEmpty()) {

    QSet<QString> onlySymbols;
    QSet<QString> onlyNames;
    sortSecurities(onlySymbols, onlyNames, m_mapSymbolName);

    if (!onlySymbols.isEmpty() || !onlyNames.isEmpty()) {
      m_securitiesDlg = new SecuritiesDlg;
      for (QSet<QString>::const_iterator symbol = onlySymbols.cbegin(); symbol != onlySymbols.cend(); ++symbol)
        m_securitiesDlg->displayLine(*symbol, QString());
      for (QSet<QString>::const_iterator name = onlyNames.cbegin(); name != onlyNames.cend(); ++name)
        m_securitiesDlg->displayLine(QString(), *name);
    }
  }

  if (!m_securitiesDlg.isNull()) {
    QTableWidget* symbolTable = m_securitiesDlg->ui->tableWidget;
    if (m_securitiesDlg->exec() == QDialog::Rejected) {
      return false;
    } else {
      for (int row = 0; row < symbolTable->rowCount(); ++row) {
        QString symbol = symbolTable->item(row, 1)->text();
        QString name = symbolTable->item(row, 2)->text();
        m_mapSymbolName.insert(symbol, name);
      }
      delete m_securitiesDlg;
    }
  }

  return true;
}

bool InvestmentPage::validateSecurity()
{
  if (!m_securitySymbol.isEmpty() &&
      !m_securityName.isEmpty())
    m_mapSymbolName.insert(m_securitySymbol, m_securityName);

  MyMoneyFile* file = MyMoneyFile::instance();
  if (m_securityDlg.isNull() &&
      (m_mapSymbolName.isEmpty() ||
      !(m_dontAsk && m_wiz->m_skipSetup))) {
    m_securityDlg = new SecurityDlg;
    m_securityDlg->initializeSecurities(m_securitySymbol, m_securityName);
    m_securityDlg->ui->cbDontAsk->setChecked(m_dontAsk);
  }

  if (!m_securityDlg.isNull()) {
    if (m_securityDlg->exec() == QDialog::Rejected) {
      return false;
    } else {
      QString securityID = m_securityDlg->security();
      if (!securityID.isEmpty()) {
        m_securityName = file->security(securityID).name();
        m_securitySymbol = file->security(securityID).tradingSymbol();
      } else {
        m_securityName = m_securityDlg->name();
        m_securitySymbol = m_securityDlg->symbol();
      }
      m_dontAsk = m_securityDlg->dontAsk();
      m_mapSymbolName.clear();
      m_mapSymbolName.insert(m_securitySymbol, m_securityName); // probably we should check if security with given symbol and name exists...
      delete m_securityDlg;                                     // ...but KMM allows creating duplicates, so don't bother
    }
  }
  if (m_mapSymbolName.isEmpty())
    return false;
  return true;
}

void InvestmentPage::saveSettings()
{
  KConfigGroup profileNamesGroup(m_wiz->m_config, "ProfileNames");
  profileNamesGroup.writeEntry("Invest", m_wiz->m_profileList);
  profileNamesGroup.writeEntry("PriorInvest", m_wiz->m_profileList.indexOf(m_wiz->m_profileName));
  profileNamesGroup.config()->sync();

  KConfigGroup profilesGroup(m_wiz->m_config, QStringLiteral("Invest-") + m_wiz->m_profileName);
  profilesGroup.writeEntry("DateFormat", m_wiz->m_dateFormatIndex);
  profilesGroup.writeEntry("FieldDelimiter", m_wiz->m_fieldDelimiterIndex);
  profilesGroup.writeEntry("DecimalSymbol", m_wiz->m_decimalSymbolIndex);
  profilesGroup.writeEntry("PriceFraction", ui->comboBoxInv_priceFraction->currentIndex());
  profilesGroup.writeEntry("StartLine", m_wiz->m_startLine - 1);
  profilesGroup.writeEntry("TrailerLines", m_wiz->m_trailerLines);
  //    The strings in these resource file lists may be edited,
  //    or expanded in the file by the user, to suit his needs.

  profilesGroup.writeEntry("ShrsinParam", m_shrsinList);
  profilesGroup.writeEntry("DivXParam", m_divXList);
  profilesGroup.writeEntry("IntIncParam", m_intIncList);
  profilesGroup.writeEntry("BrokerageParam", m_brokerageList);
  profilesGroup.writeEntry("ReinvdivParam", m_reinvdivList);
  profilesGroup.writeEntry("BuyParam", m_buyList);
  profilesGroup.writeEntry("SellParam", m_sellList);
  profilesGroup.writeEntry("RemoveParam", m_shrsoutList);

  if (m_wiz->m_inFileName.startsWith(QLatin1Literal("/home/"))) { // replace /home/user with ~/ for brevity
    QFileInfo fileInfo = QFileInfo(m_wiz->m_inFileName);
    if (fileInfo.isFile())
      m_wiz->m_inFileName = fileInfo.absolutePath();
    m_wiz->m_inFileName = QStringLiteral("~/") + m_wiz->m_inFileName.section(QChar(QLatin1Char('/')), 3);
  }

  profilesGroup.writeEntry("Directory", m_wiz->m_inFileName);
  profilesGroup.writeEntry("Encoding", m_wiz->m_encodeIndex);
  profilesGroup.writeEntry("DateCol", ui->comboBoxInv_dateCol->currentIndex());
  profilesGroup.writeEntry("PayeeCol", ui->comboBoxInv_typeCol->currentIndex());

  QList<int> list = m_wiz->m_memoColList;
  int posn = 0;
  if ((posn = list.indexOf(-1)) > -1) {
    list.removeOne(-1);
  }
  profilesGroup.writeEntry("MemoCol", list);
  profilesGroup.writeEntry("QuantityCol", ui->comboBoxInv_quantityCol->currentIndex());
  profilesGroup.writeEntry("AmountCol", ui->comboBoxInv_amountCol->currentIndex());
  profilesGroup.writeEntry("PriceCol", ui->comboBoxInv_priceCol->currentIndex());
  if (ui->comboBoxInv_feeCol->currentIndex() < m_wiz->m_endColumn)
    profilesGroup.writeEntry("FeeCol", ui->comboBoxInv_feeCol->currentIndex());
  else
    profilesGroup.writeEntry("FeeCol", QString());
  profilesGroup.writeEntry("SymbolCol", ui->comboBoxInv_symbolCol->currentIndex());
  profilesGroup.writeEntry("NameCol", ui->comboBoxInv_nameCol->currentIndex());
  profilesGroup.writeEntry("FeeIsPercentage", int(ui->checkBoxInv_feeIsPercentage->isChecked()));
  profilesGroup.writeEntry("FeeRate", ui->lineEdit_feeRate->text());
  profilesGroup.writeEntry("MinFee", ui->lineEdit_minFee->text());

  profilesGroup.writeEntry("SecurityName", m_securityName);
  profilesGroup.writeEntry("SecuritySymbol", m_securitySymbol);
  profilesGroup.writeEntry("DontAsk", int(m_dontAsk));
  profilesGroup.config()->sync();
}

void InvestmentPage::readSettings(const KSharedConfigPtr& config)
{
  for (int i = 0; i < m_wiz->m_profileList.count(); ++i) {
    if (m_wiz->m_profileList[i] != m_wiz->m_profileName)
      continue;
    KConfigGroup profilesGroup(config, QStringLiteral("Invest-") + m_wiz->m_profileList[i]);

    m_wiz->m_inFileName = profilesGroup.readEntry("Directory", QString());

    QStringList list = profilesGroup.readEntry("BuyParam", QStringList());
    if (!list.isEmpty())
      m_buyList = list;
    list = profilesGroup.readEntry("ShrsinParam", QStringList());
    if (!list.isEmpty())
      m_shrsinList = list;
    list = profilesGroup.readEntry("DivXParam", QStringList());
    if (!list.isEmpty())
      m_divXList = list;
    list = profilesGroup.readEntry("IntIncParam", QStringList());
    if (!list.isEmpty())
      m_intIncList = list;
    list = profilesGroup.readEntry("BrokerageParam", QStringList());
    if (!list.isEmpty())
      m_brokerageList = list;
    list = profilesGroup.readEntry("ReinvdivParam", QStringList());
    if (!list.isEmpty())
      m_reinvdivList = list;
    list = profilesGroup.readEntry("SellParam", QStringList());
    if (!list.isEmpty())
      m_sellList = list;
    list = profilesGroup.readEntry("RemoveParam", QStringList());
    if (!list.isEmpty())
      m_shrsoutList = list;

    m_priceFraction = profilesGroup.readEntry("PriceFraction", 2);
    m_colTypeNum[ColumnDate] = profilesGroup.readEntry("DateCol", -1);
    m_colTypeNum[ColumnType] = profilesGroup.readEntry("PayeeCol", -1);  //use for type col.
    m_colTypeNum[ColumnPrice] = profilesGroup.readEntry("PriceCol", -1);
    m_colTypeNum[ColumnQuantity] = profilesGroup.readEntry("QuantityCol", -1);
    m_colTypeNum[ColumnAmount] = profilesGroup.readEntry("AmountCol", -1);
    m_colTypeNum[ColumnName] = profilesGroup.readEntry("NameCol", -1);
    m_colTypeNum[ColumnFee] = profilesGroup.readEntry("FeeCol", -1);
    m_colTypeNum[ColumnSymbol] = profilesGroup.readEntry("SymbolCol", -1);
    m_colTypeNum[ColumnMemo] = -1; // initialize, otherwise random data may go here
    m_feeIsPercentage = profilesGroup.readEntry("FeeIsPercentage", 0);
    m_feeRate = profilesGroup.readEntry("FeeRate", QString());
    m_minFee = profilesGroup.readEntry("MinFee", QString());

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

    m_securityName = profilesGroup.readEntry("SecurityName", QString());
    m_securitySymbol = profilesGroup.readEntry("SecuritySymbol", QString());
    m_dontAsk = profilesGroup.readEntry("DontAsk", 0);
    break;
  }
}

void InvestmentPage::makeQIF(MyMoneyStatement& st, QFile& file)
{
  QTextStream out(&file);

  QString buffer;
  QString strEType;
  switch (st.m_eType) {
  case MyMoneyStatement::etInvestment:
  default:
    strEType = QStringLiteral("Invst");
  }

  if (!st.m_strAccountName.isEmpty()) {
    buffer.append(QStringLiteral("!Account\n"));
    buffer.append(QChar(QLatin1Char('N')) + st.m_strAccountName + QChar(QLatin1Char('\n')));
    buffer.append(QChar(QLatin1Char('T')) + strEType + QChar(QLatin1Char('\n')));
    buffer.append(QStringLiteral("^\n"));
  }

  for (QList<MyMoneyStatement::Security>::const_iterator it = st.m_listSecurities.constBegin(); it != st.m_listSecurities.constEnd(); ++it) {
    buffer.append(QStringLiteral("!Type:Security\n"));
    buffer.append(QChar(QLatin1Char('N')) + (*it).m_strName + QChar(QLatin1Char('\n')));
    buffer.append(QChar(QLatin1Char('S')) + (*it).m_strSymbol + QChar(QLatin1Char('\n')));
    buffer.append(QStringLiteral("TStock\n^\n"));
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
    buffer.append(QChar(QLatin1Char('Y')) + it->m_strSecurity + QChar(QLatin1Char('\n')));
    QString txt;
    switch (it->m_eAction) {
    case MyMoneyStatement::Transaction::eaBuy:
      txt = QStringLiteral("Buy");
      break;
    case MyMoneyStatement::Transaction::eaSell:
      txt = QStringLiteral("Sell");
      break;
    case MyMoneyStatement::Transaction::eaReinvestDividend:
      txt = QStringLiteral("ReinvDiv");
      break;
    case MyMoneyStatement::Transaction::eaCashDividend:
      txt = QStringLiteral("Div");
      break;
    case MyMoneyStatement::Transaction::eaInterest:
      txt = QStringLiteral("IntInc");
      break;
    case MyMoneyStatement::Transaction::eaShrsin:
      txt = QStringLiteral("ShrsIn");
      break;
    case MyMoneyStatement::Transaction::eaShrsout:
      txt = QStringLiteral("ShrsOut");
      break;
    case MyMoneyStatement::Transaction::eaStksplit:
      txt = QStringLiteral("stksplit");
      break;
    default:
      txt = QStringLiteral("unknown");  // shouldn't happen
    }

    buffer.append(QChar(QLatin1Char('N')) + txt + QChar(QLatin1Char('\n')));

    if (it->m_eAction == MyMoneyStatement::Transaction::eaBuy)  // we added 'N' field so buy transaction should have any sign
      txt.setNum(it->m_amount.abs().toDouble(), 'f', 4);
    else
      txt.setNum(it->m_amount.toDouble(), 'f', 4);
    buffer.append(QChar(QLatin1Char('T')) + txt + QChar(QLatin1Char('\n')));
    txt.setNum(it->m_shares.toDouble(), 'f', 4);
    buffer.append(QChar(QLatin1Char('Q')) + txt + QChar(QLatin1Char('\n')));
    txt.setNum(it->m_price.toDouble(), 'f', 4);
    buffer.append(QChar(QLatin1Char('I')) + txt + QChar(QLatin1Char('\n')));
    if (!it->m_fees.isZero()) {
      txt.setNum(it->m_fees.toDouble(), 'f', 4);
      buffer.append(QChar(QLatin1Char('O')) + txt + QChar(QLatin1Char('\n')));
    }

    if (!it->m_strBrokerageAccount.isEmpty())
      buffer.append(QChar(QLatin1Char('L')) + it->m_strBrokerageAccount + QChar(QLatin1Char('\n')));

    if (!it->m_strMemo.isEmpty())
      buffer.append(QChar(QLatin1Char('M')) + it->m_strMemo + QChar(QLatin1Char('\n')));
    buffer.append(QStringLiteral("^\n"));
    out << buffer;// output qif file
    buffer.clear();
  }
}

bool InvestmentPage::createStatement(MyMoneyStatement& st)
{
  if (!st.m_listTransactions.isEmpty()) // don't create statement if there is one
    return true;
  st.m_eType = MyMoneyStatement::etInvestment;
  if (m_wiz->m_autodetect.value(CSVWizard::AutoAccountInvest))
    m_wiz->detectAccount(st);

  if (m_colTypeNum.value(ColumnFee) >= m_wiz->m_endColumn) // fee column has not been calculated so do it now
    slotCalculateFee();

  for (int line = m_wiz->m_startLine - 1; line < m_wiz->m_endLine; ++line)
    if (!processInvestLine(m_wiz->m_lineList[line], st)) // parse fields
      return false;

  for (QMap<QString, QString>::const_iterator it = m_mapSymbolName.cbegin(); it != m_mapSymbolName.cend(); ++it) {
    MyMoneyStatement::Security security;
    security.m_strSymbol = it.key();
    security.m_strName = it.value();
    st.m_listSecurities << security;
  }
  return true;
}

bool InvestmentPage::processInvestLine(const QString &line, MyMoneyStatement &st)
{
  MyMoneyStatement::Transaction tr;
  m_columnList = m_wiz->m_parse->parseLine(line); // split line into fields
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
        return false;
      }
    }

    // process quantity field
    if (m_colTypeNum.value(ColumnQuantity) != -1) {
      ++neededFieldsCount;
      if (m_wiz->m_decimalSymbolIndex == 2) {
        int decimalSymbolIndex = m_wiz->m_decimalSymbolIndexMap.value(m_colTypeNum[ColumnQuantity]);
        m_wiz->m_parse->setDecimalSymbol(decimalSymbolIndex);
        m_wiz->m_parse->setThousandsSeparator(decimalSymbolIndex);
      }

      txt = m_columnList[m_colTypeNum[ColumnQuantity]];
      txt.remove(QRegularExpression(QStringLiteral("+-"))); // remove unwanted sings in quantity

      if (txt.isEmpty())
        tr.m_shares = MyMoneyMoney();
      else
        tr.m_shares = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(txt));
    }

    // process price field
    if (m_colTypeNum.value(ColumnPrice) != -1) {
      ++neededFieldsCount;
      if (m_wiz->m_decimalSymbolIndex == 2) {
        int decimalSymbolIndex = m_wiz->m_decimalSymbolIndexMap.value(m_colTypeNum[ColumnPrice]);
        m_wiz->m_parse->setDecimalSymbol(decimalSymbolIndex);
        m_wiz->m_parse->setThousandsSeparator(decimalSymbolIndex);
      }

      txt = m_columnList[m_colTypeNum[ColumnPrice]];
      if (txt.isEmpty())
        tr.m_price = MyMoneyMoney();
      else {
        tr.m_price = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(txt));
        tr.m_price *= MyMoneyMoney(m_priceFractionValue);
      }
    }

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

      if (txt.isEmpty())
        tr.m_amount = MyMoneyMoney();
      else
        tr.m_amount = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(txt));
    }

    // process type field
    if (m_colTypeNum.value(ColumnType) != -1) {
      ++neededFieldsCount;
      txt = m_columnList[m_colTypeNum[ColumnType]];
      tr.m_eAction = processActionType(txt);
      if (!validateActionType(tr, m_columnList))// check if price, amount, quantity is appropriate for action type
        return false;
    }

    // process fee field
    if (m_colTypeNum.value(ColumnFee) != -1) {
      if (m_wiz->m_decimalSymbolIndex == 2) {
        int decimalSymbolIndex = m_wiz->m_decimalSymbolIndexMap.value(m_colTypeNum[ColumnFee]);
        m_wiz->m_parse->setDecimalSymbol(decimalSymbolIndex);
        m_wiz->m_parse->setThousandsSeparator(decimalSymbolIndex);
      }

      txt = m_columnList[m_colTypeNum[ColumnFee]];
      if (txt.startsWith(QChar(QLatin1Char('(')))) // check if brackets notation is used for negative numbers
        txt.remove(QRegularExpression(QStringLiteral("[()]")));

      if (txt.isEmpty())
        tr.m_fees = MyMoneyMoney();
      else {
        MyMoneyMoney fee(m_wiz->m_parse->possiblyReplaceSymbol(txt));
        if (m_feeIsPercentage && m_feeRate.isEmpty())      //   fee is percent
          fee *= tr.m_amount / MyMoneyMoney(100); // as percentage
        fee.abs();
        tr.m_fees = fee;
      }
    }

    // process symbol and name field
    if (m_colTypeNum.value(ColumnSymbol) != -1)
      tr.m_strSymbol = m_columnList[m_colTypeNum[ColumnSymbol]];
    if (m_colTypeNum.value(ColumnName) != -1 &&
        tr.m_strSymbol.isEmpty()) { // case in which symbol field is empty
      txt = m_columnList[m_colTypeNum[ColumnName]];
      tr.m_strSymbol = m_mapSymbolName.key(txt);   // it's all about getting the right symbol
    } else if (!m_securitySymbol.isEmpty())
      tr.m_strSymbol = m_securitySymbol;
    else
      return false;
    tr.m_strSecurity = m_mapSymbolName.value(tr.m_strSymbol); // take name from prepared names to avoid potential name mismatch

    // process memo field
    if (m_colTypeNum.value(ColumnMemo) != -1)
      memo.append(m_columnList[m_colTypeNum[ColumnMemo]]);

    for (int i = 0; i < m_wiz->m_memoColList.count(); ++i) {
      if (m_wiz->m_memoColList[i] != m_colTypeNum[ColumnMemo]) {
        if (!memo.isEmpty())
          memo.append(QChar(QLatin1Char('\n')));
        if (m_wiz->m_memoColList[i] < m_columnList.count())
          memo.append(m_columnList[m_wiz->m_memoColList[i]]);
      }
    }
    tr.m_strMemo = memo;

    tr.m_strInterestCategory.clear(); // no special category
    tr.m_strBrokerageAccount.clear(); // no brokerage account auto-detection

  if (neededFieldsCount <= 3) {
    QString errMsg = i18n("<center>The columns selected are invalid.</center>"
                          "There must an amount or quantity fields, symbol or security name, plus date and type field.");
    if (m_wiz->m_skipSetup)
      errMsg += i18n("<center>You possibly need to check the start and end line settings, or reset 'Skip setup'.</center>");
    KMessageBox::sorry(m_wiz, errMsg);
    return false;
  }

  MyMoneyStatement::Split s1;
  s1.m_amount = tr.m_amount;
  s1.m_strMemo = tr.m_strMemo;
  MyMoneyStatement::Split s2 = s1;
  s2.m_amount = MyMoneyMoney(-s1.m_amount);
  s2.m_accountId = m_wiz->m_csvUtil->checkCategory(tr.m_strInterestCategory, s1.m_amount, s2.m_amount);

  // deduct fees from amount
  if (tr.m_eAction == MyMoneyStatement::Transaction::eaCashDividend ||
      tr.m_eAction == MyMoneyStatement::Transaction::eaSell ||
      tr.m_eAction == MyMoneyStatement::Transaction::eaInterest)
    tr.m_amount -= tr.m_fees;

  else if (tr.m_eAction == MyMoneyStatement::Transaction::eaBuy) {
    if (tr.m_amount.isPositive())
      tr.m_amount = -tr.m_amount; //if broker doesn't use minus sings for buy transactions, set it manually here
    tr.m_amount -= tr.m_fees;
  } else if (tr.m_eAction == MyMoneyStatement::Transaction::eaNone)
    tr.m_listSplits.append(s2);

  st.m_listTransactions.append(tr); // Add the transaction to the statement
  return true;
}
