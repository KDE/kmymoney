/*
 * Copyright 2010-2017  Allan Anderson <agander93@gmail.com>
 * Copyright 2016-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "investmentwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QStandardItemModel>
#include <QTextStream>
#include <QTableWidget>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "csvwizard.h"
#include "core/csvimportercore.h"

#include "transactiondlg.h"
#include "securitydlg.h"
#include "securitiesdlg.h"

#include "ui_investmentwizardpage.h"
#include "ui_securitiesdlg.h"
#include "ui_securitydlg.h"

// ----------------------------------------------------------------------------

InvestmentPage::InvestmentPage(CSVWizard *dlg, CSVImporterCore *imp) :
  CSVWizardPage(dlg, imp),
  ui(new Ui::InvestmentPage)
{
  ui->setupUi(this);

  connect(ui->m_clear, &QAbstractButton::clicked, this, &InvestmentPage::clearColumns);
  connect(ui->m_clearFee, &QAbstractButton::clicked, this, &InvestmentPage::clearFee);
  connect(ui->m_calculateFee, &QAbstractButton::clicked, this, &InvestmentPage::calculateFee);

  // initialize column names
  m_dlg->m_colTypeName.insert(Column::Type, i18n("Type"));
  m_dlg->m_colTypeName.insert(Column::Price, i18n("Price"));
  m_dlg->m_colTypeName.insert(Column::Quantity, i18n("Quantity"));
  m_dlg->m_colTypeName.insert(Column::Fee, i18n("Fee"));
  m_dlg->m_colTypeName.insert(Column::Date, i18n("Date"));
  m_dlg->m_colTypeName.insert(Column::Amount, i18n("Amount"));
  m_dlg->m_colTypeName.insert(Column::Symbol, i18n("Symbol"));
  m_dlg->m_colTypeName.insert(Column::Name, i18n("Name"));
  m_dlg->m_colTypeName.insert(Column::Memo, i18n("Memo"));

  m_profile = dynamic_cast<InvestmentProfile *>(m_imp->m_profile);

  connect(ui->m_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColSelected(int)));
  connect(ui->m_typeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(typeColSelected(int)));
  connect(ui->m_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColSelected(int)));
  connect(ui->m_quantityCol, SIGNAL(currentIndexChanged(int)), this, SLOT(quantityColSelected(int)));
  connect(ui->m_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(priceColSelected(int)));
  connect(ui->m_priceFraction, SIGNAL(currentIndexChanged(int)), this, SLOT(fractionChanged(int)));
  connect(ui->m_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(amountColSelected(int)));
  connect(ui->m_feeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(feeColSelected(int)));
  connect(ui->m_symbolCol, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolColSelected(int)));
  connect(ui->m_nameCol, SIGNAL(currentIndexChanged(int)), this, SLOT(nameColSelected(int)));
  connect(ui->m_feeIsPercentage, &QAbstractButton::clicked, this, &InvestmentPage::feeIsPercentageClicked);
  connect(ui->m_feeRate, &QLineEdit::editingFinished, this, &InvestmentPage::feeInputsChanged);
  connect(ui->m_feeRate, &QLineEdit::textChanged, this, &InvestmentPage::feeRateChanged);
  connect(ui->m_minFee, &QLineEdit::textChanged, this, &InvestmentPage::minFeeChanged);
}

InvestmentPage::~InvestmentPage()
{
  delete m_securitiesDlg;
  delete ui;
}

void InvestmentPage::calculateFee()
{
  m_imp->calculateFee();
  m_dlg->updateWindowSize();
  m_dlg->markUnwantedRows();
}

void InvestmentPage::initializePage()
{
  QHash<Column, QComboBox *> columns {{Column::Amount, ui->m_amountCol}, {Column::Type, ui->m_typeCol},
                                           {Column::Quantity, ui->m_quantityCol}, {Column::Memo, ui->m_memoCol},
                                           {Column::Price, ui->m_priceCol}, {Column::Date, ui->m_dateCol},
                                           {Column::Fee,  ui->m_feeCol},  {Column::Symbol, ui->m_symbolCol},
                                           {Column::Name, ui->m_nameCol}};

  if (ui->m_dateCol->count() != m_imp->m_file->m_columnCount)
    m_dlg->initializeComboBoxes(columns);

  ui->m_feeIsPercentage->setChecked(m_profile->m_feeIsPercentage);
  columns.remove(Column::Memo);
  for (auto it = columns.cbegin(); it != columns.cend(); ++it)
    it.value()->setCurrentIndex(m_profile->m_colTypeNum.value(it.key()));

  ui->m_priceFraction->blockSignals(true);
  foreach (const auto priceFraction, m_imp->m_priceFractions)
    ui->m_priceFraction->addItem(QString::number(priceFraction.toDouble(), 'g', 3));
  ui->m_priceFraction->blockSignals(false);
  ui->m_priceFraction->setCurrentIndex(m_profile->m_priceFraction);

  ui->m_feeRate->setText(m_profile->m_feeRate);
  ui->m_minFee->setText(m_profile->m_minFee);
  ui->m_feeRate->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9]{1,2}[") + QLocale().decimalPoint() + QStringLiteral("]{1,1}[0-9]{0,2}")), this) );
  ui->m_minFee->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9]{1,}[") + QLocale().decimalPoint() + QStringLiteral("]{0,1}[0-9]{0,}")), this) );

  if (!m_profile->m_feeRate.isEmpty()) {  // fee rate indicates that fee column needs to be calculated
    if (m_imp->calculateFee()) {
      ui->m_feeCol->blockSignals(true);
      int feeCol = ui->m_feeCol->count();
      ui->m_feeCol->addItem(QString::number(feeCol + 1));
      ui->m_feeCol->setEnabled(false);
      ui->m_feeCol->setCurrentIndex(feeCol);
      ui->m_feeCol->blockSignals(false);
      m_dlg->updateWindowSize();
      m_dlg->markUnwantedRows();
    }
  } else if (m_profile->m_colTypeNum.value(Column::Fee, -1) >= ui->m_feeCol->count()) { // no fee rate, calculated fee column index exist, but the column doesn't exist and that's not ok
    m_profile->m_colTypeNum[Column::Fee] = -1;
    m_profile->m_colNumType.remove(m_profile->m_colNumType.key(Column::Fee));
  }

  for (int i = 0; i < m_profile->m_memoColList.count(); ++i) { //  Set up all memo fields...
    int tmp = m_profile->m_memoColList.at(i);
    if (tmp < m_profile->m_colTypeNum.count())
      ui->m_memoCol->setCurrentIndex(tmp);
  }

  feeInputsChanged();
}

bool InvestmentPage::isComplete() const
{
  return  ui->m_dateCol->currentIndex() > -1 &&
          ui->m_typeCol->currentIndex() > -1 &&
          ui->m_quantityCol->currentIndex() > -1 &&
          ui->m_priceCol->currentIndex() > -1 &&
          ui->m_amountCol->currentIndex() > -1 &&
          ui->m_priceFraction->currentIndex() > -1;
}

bool InvestmentPage::validatePage()
{
  if (ui->m_symbolCol->currentIndex() == -1 &&
      ui->m_nameCol->currentIndex() == -1)
    return validateSecurity();
  else
    return validateSecurities();
}

void InvestmentPage::cleanupPage()
{
  clearFeeCol();
}

void InvestmentPage::memoColSelected(int col)
{
  if (m_profile->m_colNumType.value(col) == Column::Type ||
      m_profile->m_colNumType.value(col) == Column::Name) {
    int rc = KMessageBox::Yes;
    if (isVisible())
      rc = KMessageBox::questionYesNo(m_dlg, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                              "<center>If you wish to copy that data to the memo field, click 'Yes'.</center>",
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
    if (m_profile->m_colTypeNum.value(Column::Memo) != -1)
      ui->m_memoCol->setCurrentIndex(m_profile->m_colTypeNum.value(Column::Memo));
    else
      ui->m_memoCol->setCurrentIndex(-1);
    ui->m_memoCol->blockSignals(false);
    return;
  }

  if (m_profile->m_colTypeNum.value(Column::Memo) != -1)        // check if this memo has any column 'number' assigned...
    m_profile->m_memoColList.removeOne(col);           // ...if true remove it from memo list

  if(validateSelectedColumn(col, Column::Memo))
    if (col != - 1 && !m_profile->m_memoColList.contains(col))
      m_profile->m_memoColList.append(col);
}

void InvestmentPage::dateColSelected(int col)
{
  validateSelectedColumn(col, Column::Date);
}

void InvestmentPage::feeColSelected(int col)
{
  validateSelectedColumn(col, Column::Fee);
  feeInputsChanged();
}

void InvestmentPage::typeColSelected(int col)
{
  if (validateSelectedColumn(col, Column::Type))
    if (!validateMemoComboBox())  // user could have it already in memo so...
      memoColSelected(col);    // ...if true set memo field again
}

void InvestmentPage::quantityColSelected(int col)
{
  validateSelectedColumn(col, Column::Quantity);
}

void InvestmentPage::priceColSelected(int col)
{
  validateSelectedColumn(col, Column::Price);
}

void InvestmentPage::amountColSelected(int col)
{
  validateSelectedColumn(col, Column::Amount);
  clearFeeCol();
  feeInputsChanged();
}

void InvestmentPage::symbolColSelected(int col)
{
  validateSelectedColumn(col, Column::Symbol);
  m_imp->m_mapSymbolName.clear();        // new symbol column so this map is no longer valid
}

void InvestmentPage::nameColSelected(int col)
{
  if (validateSelectedColumn(col, Column::Name))
    if (!validateMemoComboBox())  // user could have it already in memo so...
      memoColSelected(col);    // ...if true set memo field again
  m_imp->m_mapSymbolName.clear();        // new name column so this map is no longer valid
}

void InvestmentPage::feeIsPercentageClicked(bool checked)
{
  m_profile->m_feeIsPercentage = checked;
}

void InvestmentPage::fractionChanged(int col)
{
  m_profile->m_priceFraction = col;
  emit completeChanged();
}

void InvestmentPage::clearColumns()
{
  ui->m_amountCol->setCurrentIndex(-1);
  ui->m_dateCol->setCurrentIndex(-1);
  ui->m_priceCol->setCurrentIndex(-1);
  ui->m_quantityCol->setCurrentIndex(-1);
  ui->m_memoCol->setCurrentIndex(-1);
  ui->m_typeCol->setCurrentIndex(-1);
  ui->m_nameCol->setCurrentIndex(-1);
  ui->m_symbolCol->setCurrentIndex(-1);
  ui->m_priceFraction->setCurrentIndex(-1);
  clearFee();
}

void InvestmentPage::clearFee()
{
  clearFeeCol();
  ui->m_feeCol->setCurrentIndex(-1);
  ui->m_feeIsPercentage->setChecked(false);
  ui->m_calculateFee->setEnabled(false);
  ui->m_feeRate->setEnabled(true);
  ui->m_minFee->setEnabled(false);
  ui->m_feeRate->clear();
  ui->m_minFee->clear();
}

void InvestmentPage::feeInputsChanged()
{


//  if (ui->comboBoxInv_feeCol->currentIndex() < m_importer->m_file->m_columnCount &&
//      ui->comboBoxInv_feeCol->currentIndex() > -1) {
//    ui->lineEdit_minFee->setEnabled(false);
//    ui->lineEdit_feeRate->setEnabled(false);
//    ui->lineEdit_minFee->clear();
//    ui->lineEdit_feeRate->clear();
//  }
  if (m_profile->m_feeRate.isEmpty()) {
    ui->m_feeCol->setEnabled(true);
    ui->m_feeIsPercentage->setEnabled(true);
    ui->m_minFee->setEnabled(false);
    ui->m_calculateFee->setEnabled(false);
  } else {
    ui->m_feeCol->setEnabled(false);
    ui->m_feeIsPercentage->setEnabled(false);
    ui->m_feeIsPercentage->setChecked(true);
    ui->m_minFee->setEnabled(true);
    ui->m_feeRate->setEnabled(true);
    if (m_profile->m_colTypeNum.value(Column::Amount) != -1)
      ui->m_calculateFee->setEnabled(true);
  }
}

void InvestmentPage::feeRateChanged(const QString &text)
{
  m_profile->m_feeRate = text;
}

void InvestmentPage::minFeeChanged(const QString &text)
{
  m_profile->m_minFee = text;
}

void InvestmentPage::clearFeeCol()
{
  if (!m_profile->m_feeRate.isEmpty() &&                                                // if fee rate isn't empty...
      m_profile->m_colTypeNum.value(Column::Fee) >= m_imp->m_file->m_columnCount - 1 &&
      !ui->m_feeCol->isEnabled()) {  // ...and fee column is last...
    --m_imp->m_file->m_columnCount;
    m_imp->m_file->m_model->removeColumn(m_imp->m_file->m_columnCount);
    int feeCol = ui->m_feeCol->currentIndex();
    ui->m_feeCol->setCurrentIndex(-1);
    ui->m_feeCol->removeItem(feeCol);
    m_dlg->updateWindowSize();
  }
  ui->m_feeCol->setEnabled(true);
  ui->m_feeIsPercentage->setEnabled(true);
  ui->m_feeIsPercentage->setChecked(false);
}

void InvestmentPage::resetComboBox(const Column comboBox)
{
  switch (comboBox) {
    case Column::Amount:
      ui->m_amountCol->setCurrentIndex(-1);
      break;
    case Column::Date:
      ui->m_dateCol->setCurrentIndex(-1);
      break;
    case Column::Fee:
      ui->m_feeCol->setCurrentIndex(-1);
      break;
    case Column::Memo:
      ui->m_memoCol->setCurrentIndex(-1);
      break;
    case Column::Price:
      ui->m_priceCol->setCurrentIndex(-1);
      break;
    case Column::Quantity:
      ui->m_quantityCol->setCurrentIndex(-1);
      break;
    case Column::Type:
      ui->m_typeCol->setCurrentIndex(-1);
      break;
    case Column::Symbol:
      ui->m_symbolCol->setCurrentIndex(-1);
      break;
    case Column::Name:
      ui->m_nameCol->setCurrentIndex(-1);
      break;
    default:
      KMessageBox::sorry(m_dlg, i18n("<center>Field name not recognised.</center><center>'<b>%1</b>'</center>Please re-enter your column selections.", (int)comboBox), i18n("CSV import"));
  }
}

bool InvestmentPage::validateActionType()
{
  for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row) {
    MyMoneyStatement::Transaction tr;
    // process quantity field
    int col = m_profile->m_colTypeNum.value(Column::Quantity);
    tr.m_shares = m_imp->processQuantityField(m_profile, row, col);

    // process price field
    col = m_profile->m_colTypeNum.value(Column::Price);
    tr.m_price = m_imp->processPriceField(m_profile, row, col);

    // process amount field
    col = m_profile->m_colTypeNum.value(Column::Amount);
    tr.m_amount = m_imp->processAmountField(m_profile, row, col);

    // process type field
    col = m_profile->m_colTypeNum.value(Column::Type);
    tr.m_eAction = m_imp->processActionTypeField(m_profile, row, col);

    switch(m_imp->validateActionType(tr)) {
      case InvalidActionValues:
        KMessageBox::sorry(m_dlg,
                           i18n("The values in the columns you have selected\ndo not match any expected investment type.\n"
                                "Please check the fields in the current transaction,\nand also your selections."),
                           i18n("CSV import"));
        return false;
        break;
      case NoActionType:
      {
        bool unknownType = false;
        if (tr.m_eAction == eMyMoney::Transaction::Action::None)
          unknownType = true;

        QStringList colList;
        QStringList colHeaders;
        for (col = 0; col < m_imp->m_file->m_columnCount; ++col) {
          colHeaders.append(m_dlg->m_colTypeName.value(m_profile->m_colNumType.value(col, Column::Invalid), QString(i18nc("Unused column", "Unused"))));
          colList.append(m_imp->m_file->m_model->item(row, col)->text());
        }
        QList<eMyMoney::Transaction::Action> validActionTypes = m_imp->createValidActionTypes(tr);
        QPointer<TransactionDlg> transactionDlg = new TransactionDlg(colList, colHeaders, m_profile->m_colTypeNum.value(Column::Type), validActionTypes);
        if (transactionDlg->exec() == QDialog::Rejected) {
          KMessageBox::information(m_dlg,
                                   i18n("<center>No valid action type found for this transaction.</center>"
                                        "<center>Please check the parameters supplied.</center>"),
                                   i18n("CSV import"));
          return false;
        } else
          tr.m_eAction = transactionDlg->getActionType();
        delete transactionDlg;

        if (unknownType) { // type was unknown so store it
          col = m_profile->m_colTypeNum.value(Column::Type);
          m_profile->m_transactionNames[tr.m_eAction].append(m_imp->m_file->m_model->item(row, col)->text()); // store action type
        }
      }
      default:
        break;
    }
  }
  m_imp->m_isActionTypeValidated = true;
  return true;
}

bool InvestmentPage::validateSelectedColumn(const int col, const Column type)
{
  if (m_profile->m_colTypeNum.value(type) != -1)        // check if this 'type' has any column 'number' assigned...
    m_profile->m_colNumType.remove(m_profile->m_colTypeNum.value(type)); // ...if true remove 'type' assigned to this column 'number'

  bool ret = true;
  if (col == -1) { // user only wanted to reset his column so allow him
    m_profile->m_colTypeNum[type] = col;  // assign new column 'number' to this 'type'
  } else if (m_profile->m_colNumType.contains(col)) { // if this column 'number' has already 'type' assigned
    KMessageBox::information(m_dlg, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>",
                                     m_dlg->m_colTypeName.value(m_profile->m_colNumType.value(col))));
    resetComboBox(m_profile->m_colNumType[col]);
    resetComboBox(type);
    ret = false;
  } else {
    m_profile->m_colTypeNum[type] = col; // assign new column 'number' to this 'type'
    m_profile->m_colNumType[col] = type; // assign new 'type' to this column 'number'
  }
  emit completeChanged();
  return ret;
}

bool InvestmentPage::validateMemoComboBox()
{
  if (m_profile->m_memoColList.count() == 0)
    return true;
  for (int i = 0; i < ui->m_memoCol->count(); ++i)
  {
    QString txt = ui->m_memoCol->itemText(i);
    if (txt.contains(QLatin1Char('*')))  // check if text containing '*' belongs to valid column types
      if (m_profile->m_colNumType.value(i) != Column::Name &&
          m_profile->m_colNumType.value(i) != Column::Type) {
        ui->m_memoCol->setItemText(i, QString::number(i + 1));
        m_profile->m_memoColList.removeOne(i);
        return false;
      }
  }
  return true;
}


bool InvestmentPage::validateSecurities()
{
  if (m_securitiesDlg.isNull() &&
      m_imp->m_mapSymbolName.isEmpty()) {

    QSet<QString> onlySymbols;
    QSet<QString> onlyNames;
    m_imp->sortSecurities(onlySymbols, onlyNames, m_imp->m_mapSymbolName);

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
        m_imp->m_mapSymbolName.insert(symbol, name);
      }
      delete m_securitiesDlg;
    }
  }

  return true;
}

bool InvestmentPage::validateSecurity()
{
  if (!m_profile->m_securitySymbol.isEmpty() &&
      !m_profile->m_securityName.isEmpty())
    m_imp->m_mapSymbolName.insert(m_profile->m_securitySymbol, m_profile->m_securityName);

  MyMoneyFile* file = MyMoneyFile::instance();
  if (m_securityDlg.isNull() &&
      (m_imp->m_mapSymbolName.isEmpty() ||
      !(m_profile->m_dontAsk && m_dlg->m_skipSetup))) {
    m_securityDlg = new SecurityDlg;
    m_securityDlg->initializeSecurities(m_profile->m_securitySymbol, m_profile->m_securityName);
    m_securityDlg->ui->cbDontAsk->setChecked(m_profile->m_dontAsk);
  }

  if (!m_securityDlg.isNull()) {
    if (m_securityDlg->exec() == QDialog::Rejected) {
      return false;
    } else {
      QString securityID = m_securityDlg->security();
      if (!securityID.isEmpty()) {
        m_profile->m_securityName = file->security(securityID).name();
        m_profile->m_securitySymbol = file->security(securityID).tradingSymbol();
      } else {
        m_profile->m_securityName = m_securityDlg->name();
        m_profile->m_securitySymbol = m_securityDlg->symbol();
      }
      m_profile->m_dontAsk = m_securityDlg->dontAsk();
      m_imp->m_mapSymbolName.clear();
      m_imp->m_mapSymbolName.insert(m_profile->m_securitySymbol, m_profile->m_securityName); // probably we should check if security with given symbol and name exists...
      delete m_securityDlg;                                     // ...but KMM allows creating duplicates, so don't bother
    }
  }
  if (m_imp->m_mapSymbolName.isEmpty())
    return false;
  return true;
}

void InvestmentPage::makeQIF(const MyMoneyStatement& st, const QString& outFileName)
{
  QFile oFile(outFileName);
  oFile.open(QIODevice::WriteOnly);
  QTextStream out(&oFile);

  QString buffer;
  QString strEType;
  switch (st.m_eType) {
  case eMyMoney::Statement::Type::Investment:
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
    case eMyMoney::Transaction::Action::Buy:
      txt = QStringLiteral("Buy");
      break;
    case eMyMoney::Transaction::Action::Sell:
      txt = QStringLiteral("Sell");
      break;
    case eMyMoney::Transaction::Action::ReinvestDividend:
      txt = QStringLiteral("ReinvDiv");
      break;
    case eMyMoney::Transaction::Action::CashDividend:
      txt = QStringLiteral("Div");
      break;
    case eMyMoney::Transaction::Action::Interest:
      txt = QStringLiteral("IntInc");
      break;
    case eMyMoney::Transaction::Action::Shrsin:
      txt = QStringLiteral("ShrsIn");
      break;
    case eMyMoney::Transaction::Action::Shrsout:
      txt = QStringLiteral("ShrsOut");
      break;
    case eMyMoney::Transaction::Action::Stksplit:
      txt = QStringLiteral("stksplit");
      break;
    default:
      txt = QStringLiteral("unknown");  // shouldn't happen
    }

    buffer.append(QChar(QLatin1Char('N')) + txt + QChar(QLatin1Char('\n')));

    if (it->m_eAction == eMyMoney::Transaction::Action::Buy)  // we added 'N' field so buy transaction should have any sign
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
  oFile.close();
}
