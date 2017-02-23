/*******************************************************************************
*                                 priceswizardpage.cpp
*                              --------------------
* begin                       : Sat Jan 21 2017
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

#include "priceswizardpage.h"

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

#include "securitydlg.h"
#include "currenciesdlg.h"

#include "ui_priceswizardpage.h"
#include "ui_csvwizard.h"

// ----------------------------------------------------------------------------

PricesPage::PricesPage(QDialog *parent) : QWizardPage(parent), ui(new Ui::PricesPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);

  connect(ui->button_clear, SIGNAL(clicked()), this, SLOT(slotClearColumns()));

  // initialize column names
  m_colTypeName.insert(ColumnPrice, i18n("Price"));
  m_colTypeName.insert(ColumnDate, i18n("Date"));
}

PricesPage::~PricesPage()
{
  delete ui;
  delete m_securityDlg;
  delete m_currenciesDlg;
}

void PricesPage::setParent(CSVWizard* dlg)
{
  m_wiz = dlg;
}

void PricesPage::initializeComboBoxes()
{
  // disable prices widgets allowing their initialization
  disconnect(ui->comboBoxPrc_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColSelected(int)));
  disconnect(ui->comboBoxPrc_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPriceColSelected(int)));
  disconnect(ui->comboBoxPrc_priceFraction, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFractionChanged(int)));

  // clear all existing items before adding new ones
  ui->comboBoxPrc_dateCol->clear();
  ui->comboBoxPrc_priceCol->clear();

  QStringList columnNumbers;
  for (int i = 0; i < m_wiz->m_endColumn; ++i) {
    columnNumbers << QString::number(i + 1);
  }

  // populate comboboxes with col # values
  ui->comboBoxPrc_dateCol->addItems(columnNumbers);
  ui->comboBoxPrc_priceCol->addItems(columnNumbers);

  slotClearColumns(); // all comboboxes are set to 0 so set them to -1
  connect(ui->comboBoxPrc_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColSelected(int)));
  connect(ui->comboBoxPrc_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPriceColSelected(int)));
  connect(ui->comboBoxPrc_priceFraction, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFractionChanged(int)));
}

void PricesPage::initializePage()
{
  if (ui->comboBoxPrc_dateCol->count() != m_wiz->m_endColumn)
    initializeComboBoxes();
  ui->comboBoxPrc_dateCol->setCurrentIndex(m_colTypeNum.value(ColumnDate));
  ui->comboBoxPrc_priceCol->setCurrentIndex(m_colTypeNum.value(ColumnPrice));
  ui->comboBoxPrc_priceFraction->setCurrentIndex(m_priceFraction);
}

bool PricesPage::isComplete() const
{
  return  ui->comboBoxPrc_dateCol->currentIndex() > -1 &&
          ui->comboBoxPrc_priceCol->currentIndex() > -1 &&
          ui->comboBoxPrc_priceFraction->currentIndex() > -1;
}

bool PricesPage::validatePage()
{
  if (m_wiz->m_profileType == CSVWizard::ProfileCurrencyPrices)
    return validateCurrencies();
  else if (m_wiz->m_profileType == CSVWizard::ProfileStockPrices)
    return validateSecurity();
  return false;
}

void PricesPage::slotDateColSelected(int col)
{
  validateSelectedColumn(col, ColumnDate);
}

void PricesPage::slotPriceColSelected(int col)
{
  validateSelectedColumn(col, ColumnPrice);
}

void PricesPage::slotFractionChanged(int col)
{
  m_priceFraction = col;
  m_priceFractionValue = ui->comboBoxPrc_priceFraction->itemText(col);
  emit completeChanged();
}

void PricesPage::slotClearColumns()
{
  ui->comboBoxPrc_dateCol->setCurrentIndex(-1);
  ui->comboBoxPrc_priceCol->setCurrentIndex(-1);
  ui->comboBoxPrc_priceFraction->setCurrentIndex(-1);
}

void PricesPage::resetComboBox(const columnTypeE comboBox)
{
  switch (comboBox) {
    case ColumnDate:
      ui->comboBoxPrc_dateCol->setCurrentIndex(-1);
      break;
    case ColumnPrice:
      ui->comboBoxPrc_priceCol->setCurrentIndex(-1);
      break;
    default:
      KMessageBox::sorry(m_wiz, i18n("<center>Field name not recognised.</center><center>'<b>%1</b>'</center>Please re-enter your column selections.", comboBox), i18n("CSV import"));
  }
}

bool PricesPage::validateSelectedColumn(int col, columnTypeE type)
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

bool PricesPage::validateCurrencies()
{
  if ((m_currenciesDlg.isNull() ||
      m_fromCurrency.isEmpty() ||
      m_fromCurrency.isEmpty()) &&
      !(m_dontAsk && m_wiz->m_skipSetup)) {
    m_currenciesDlg = new CurrenciesDlg;
    m_currenciesDlg->initializeCurrencies(m_fromCurrency, m_toCurrency);
    m_currenciesDlg->ui->cbDontAsk->setChecked(m_dontAsk);
  }

  if (!m_currenciesDlg.isNull()) {
    if (m_currenciesDlg->exec() == QDialog::Rejected) {
      return false;
    } else {
      m_fromCurrency = m_currenciesDlg->fromCurrency();
      m_toCurrency = m_currenciesDlg->toCurrency();
      m_dontAsk = m_currenciesDlg->dontAsk();
      delete m_currenciesDlg;
    }
  }
  return true;
}

bool PricesPage::validateSecurity()
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

void PricesPage::saveSettings()
{
  KConfigGroup profileNamesGroup(m_wiz->m_config, "ProfileNames");
  profileNamesGroup.writeEntry("Prices", m_wiz->m_profileList);
  if (m_wiz->m_profileType == CSVWizard::ProfileCurrencyPrices)
    profileNamesGroup.writeEntry("PriorCPrices", m_wiz->m_profileList.indexOf(m_wiz->m_profileName));
  else if (m_wiz->m_profileType == CSVWizard::ProfileStockPrices)
    profileNamesGroup.writeEntry("PriorSPrices", m_wiz->m_profileList.indexOf(m_wiz->m_profileName));

  profileNamesGroup.config()->sync();

  KConfigGroup profilesGroup;
  if (m_wiz->m_profileType == CSVWizard::ProfileCurrencyPrices)
    profilesGroup = KConfigGroup(m_wiz->m_config, QStringLiteral("CPrices-") + m_wiz->m_profileName);
  else if (m_wiz->m_profileType == CSVWizard::ProfileStockPrices)
    profilesGroup = KConfigGroup(m_wiz->m_config, QStringLiteral("SPrices-") + m_wiz->m_profileName);

  profilesGroup.writeEntry("DateFormat", m_wiz->m_dateFormatIndex);
  profilesGroup.writeEntry("FieldDelimiter", m_wiz->m_fieldDelimiterIndex);
  profilesGroup.writeEntry("DecimalSymbol", m_wiz->m_decimalSymbolIndex);
  profilesGroup.writeEntry("PriceFraction", ui->comboBoxPrc_priceFraction->currentIndex());
  profilesGroup.writeEntry("StartLine", m_wiz->m_startLine - 1);
  profilesGroup.writeEntry("TrailerLines", m_wiz->m_trailerLines);

  if (m_wiz->m_inFileName.startsWith(QLatin1Literal("/home/"))) { // replace /home/user with ~/ for brevity
    QFileInfo fileInfo = QFileInfo(m_wiz->m_inFileName);
    if (fileInfo.isFile())
      m_wiz->m_inFileName = fileInfo.absolutePath();
    m_wiz->m_inFileName = QStringLiteral("~/") + m_wiz->m_inFileName.section(QChar(QLatin1Char('/')), 3);
  }

  profilesGroup.writeEntry("Directory", m_wiz->m_inFileName);
  profilesGroup.writeEntry("Encoding", m_wiz->m_encodeIndex);
  profilesGroup.writeEntry("DateCol", ui->comboBoxPrc_dateCol->currentIndex());
  profilesGroup.writeEntry("PriceCol", ui->comboBoxPrc_priceCol->currentIndex());

  if (m_wiz->m_profileType == CSVWizard::ProfileCurrencyPrices) {
    profilesGroup.writeEntry("FromCurrency", m_fromCurrency);
    profilesGroup.writeEntry("ToCurrency", m_toCurrency);
  } else if (m_wiz->m_profileType == CSVWizard::ProfileStockPrices) {
    profilesGroup.writeEntry("SecurityName", m_securityName);
    profilesGroup.writeEntry("SecuritySymbol", m_securitySymbol);
  }
  profilesGroup.writeEntry("DontAsk", int(m_dontAsk));
  profilesGroup.config()->sync();
}

void PricesPage::readSettings(const KSharedConfigPtr& config)
{
  for (int i = 0; i < m_wiz->m_profileList.count(); ++i) {
    if (m_wiz->m_profileList[i] != m_wiz->m_profileName)
      continue;
    KConfigGroup profilesGroup;
    if (m_wiz->m_profileType == CSVWizard::ProfileCurrencyPrices)
      profilesGroup = KConfigGroup(config, QStringLiteral("CPrices-") + m_wiz->m_profileList[i]);
    else if (m_wiz->m_profileType == CSVWizard::ProfileStockPrices)
      profilesGroup = KConfigGroup(config, QStringLiteral("SPrices-") + m_wiz->m_profileList[i]);

    m_wiz->m_inFileName = profilesGroup.readEntry("Directory", QString());

    m_priceFraction = profilesGroup.readEntry("PriceFraction", 2);
    m_colTypeNum[ColumnDate] = profilesGroup.readEntry("DateCol", -1);
    m_colTypeNum[ColumnPrice] = profilesGroup.readEntry("PriceCol", -1);

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

    if (m_wiz->m_profileType == CSVWizard::ProfileCurrencyPrices) {
      m_fromCurrency = profilesGroup.readEntry("FromCurrency", QString());
      m_toCurrency = profilesGroup.readEntry("ToCurrency", QString());
    } else if (m_wiz->m_profileType == CSVWizard::ProfileStockPrices) {
      m_securityName = profilesGroup.readEntry("SecurityName", QString());
      m_securitySymbol = profilesGroup.readEntry("SecuritySymbol", QString());
    }
    m_dontAsk = profilesGroup.readEntry("DontAsk", 0);
    break;
  }
}

bool PricesPage::createStatement(MyMoneyStatement& st)
{
  if (!st.m_listPrices.isEmpty()) // don't create statement if there is one
    return true;
  st.m_eType = MyMoneyStatement::etNone;

  for (int line = m_wiz->m_startLine - 1; line < m_wiz->m_endLine; ++line)
    if (!processPriceLine(m_wiz->m_lineList[line], st)) // parse fields
      return false;

  for (QMap<QString, QString>::const_iterator it = m_mapSymbolName.cbegin(); it != m_mapSymbolName.cend(); ++it) {
    MyMoneyStatement::Security security;
    security.m_strSymbol = it.key();
    security.m_strName = it.value();
    st.m_listSecurities << security;
  }
  return true;
}

bool PricesPage::processPriceLine(const QString &line, MyMoneyStatement &st)
{
  MyMoneyStatement::Price pr;

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
  QString txt;

  for (int i = 0; i < m_columnList.count(); ++i) {
    m_columnList[i].trimmed().remove(m_wiz->m_textDelimiterCharacter);
  }

    // process date field
    if (m_colTypeNum.value(ColumnDate) != -1) {
      ++neededFieldsCount;
      txt = m_columnList[m_colTypeNum[ColumnDate]];
      pr.m_date = m_wiz->m_convertDate->convertDate(txt);      //  Date column
      if (pr.m_date == QDate()) {
        KMessageBox::sorry(m_wiz, i18n("<center>An invalid date has been detected during import.</center>"
                                       "<center><b>'%1'</b></center>"
                                       "Please check that you have set the correct date format,\n"
                                       "<center>and start and end lines.</center>"
                                       , txt), i18n("CSV import"));
        return false;
      }
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
        pr.m_amount = MyMoneyMoney();
      else {
        pr.m_amount = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(txt));
        pr.m_amount *= MyMoneyMoney(m_priceFractionValue);
      }
    }

    if (m_wiz->m_profileType == CSVWizard::ProfileCurrencyPrices) {
      if (m_fromCurrency.isEmpty() || m_toCurrency.isEmpty())
        return false;
      pr.m_strSecurity = m_fromCurrency;
      pr.m_strCurrency = m_toCurrency;
    }
    else if (m_wiz->m_profileType == CSVWizard::ProfileStockPrices) {
      if (m_mapSymbolName.isEmpty())
        return false;
      pr.m_strSecurity = m_mapSymbolName.first();
    }

    if (neededFieldsCount < 2) {
      QString errMsg = i18n("<center>The columns selected are invalid.</center>"
                            "There must an amount or quantity fields, symbol or security name, plus date and type field.");
      if (m_wiz->m_skipSetup)
        errMsg += i18n("<center>You possibly need to check the start and end line settings, or reset 'Skip setup'.</center>");
      KMessageBox::sorry(m_wiz, errMsg);
      return false;
    }

    st.m_listPrices.append(pr); // Add price to the statement
    return true;
}
