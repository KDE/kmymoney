/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "priceswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneysecurity.h"

#include "csvwizard.h"
#include "core/csvimportercore.h"

#include "securitydlg.h"
#include "currenciesdlg.h"

#include "ui_priceswizardpage.h"
#include "ui_currenciesdlg.h"
#include "ui_securitydlg.h"

// ----------------------------------------------------------------------------

PricesPage::PricesPage(CSVWizard *dlg, CSVImporterCore *imp) :
    CSVWizardPage(dlg, imp),
    ui(new Ui::PricesPage)
{
    ui->setupUi(this);

    connect(ui->button_clear, &QAbstractButton::clicked, this, &PricesPage::clearColumns);

    m_profile = dynamic_cast<PricesProfile *>(m_imp->m_profile);

    // initialize column names
    m_dlg->m_colTypeName.insert(Column::Price, i18n("Price"));
    m_dlg->m_colTypeName.insert(Column::Date, i18n("Date"));
    connect(ui->m_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColSelected(int)));
    connect(ui->m_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(priceColSelected(int)));
    connect(ui->m_priceFraction, SIGNAL(currentIndexChanged(int)), this, SLOT(fractionChanged(int)));
}

PricesPage::~PricesPage()
{
    delete m_securityDlg;
    delete m_currenciesDlg;
    delete ui;
}

void PricesPage::initializePage()
{
    const QHash<Column, QComboBox *> columns {{Column::Price, ui->m_priceCol}, {Column::Date, ui->m_dateCol}};

    if (ui->m_dateCol->count() != m_imp->m_file->m_columnCount)
        m_dlg->initializeComboBoxes(columns);

    for (auto it = columns.cbegin(); it != columns.cend(); ++it)
        it.value()->setCurrentIndex(m_profile->m_colTypeNum.value(it.key()));

    ui->m_priceFraction->blockSignals(true);
    foreach (const auto priceFraction, m_imp->m_priceFractions)
        ui->m_priceFraction->addItem(QString::number(priceFraction.toDouble(), 'g', 3));
    ui->m_priceFraction->blockSignals(false);
    ui->m_priceFraction->setCurrentIndex(m_profile->m_priceFraction);

    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch <<
           QWizard::BackButton <<
           QWizard::NextButton <<
           QWizard::CancelButton;
    wizard()->setButtonLayout(layout);
}

bool PricesPage::isComplete() const
{
    return  ui->m_dateCol->currentIndex() > -1 &&
            ui->m_priceCol->currentIndex() > -1 &&
            ui->m_priceFraction->currentIndex() > -1;
}

bool PricesPage::validatePage()
{
    switch (m_imp->m_profile->type()) {
    case Profile::CurrencyPrices:
        return validateCurrencies();
    case Profile::StockPrices:
        return validateSecurity();
    default:
        return false;
    }
}

void PricesPage::dateColSelected(int col)
{
    validateSelectedColumn(col, Column::Date);
}

void PricesPage::priceColSelected(int col)
{
    validateSelectedColumn(col, Column::Price);
}

void PricesPage::fractionChanged(int col)
{
    m_profile->m_priceFraction = col;
    emit completeChanged();
}

void PricesPage::clearColumns()
{
    ui->m_dateCol->setCurrentIndex(-1);
    ui->m_priceCol->setCurrentIndex(-1);
    ui->m_priceFraction->setCurrentIndex(-1);
}

void PricesPage::resetComboBox(const Column comboBox)
{
    switch (comboBox) {
    case Column::Date:
        ui->m_dateCol->setCurrentIndex(-1);
        break;
    case Column::Price:
        ui->m_priceCol->setCurrentIndex(-1);
        break;
    default:
        KMessageBox::sorry(m_dlg, i18n("<center>Field name not recognised.</center><center>'<b>%1</b>'</center>Please re-enter your column selections.", (int)comboBox), i18n("CSV import"));
    }
}

bool PricesPage::validateSelectedColumn(const int col, const Column type)
{
    QMap<Column, int> &colTypeNum = m_imp->m_profile->m_colTypeNum;
    QMap<int, Column> &colNumType = m_imp->m_profile->m_colNumType;

    if (colTypeNum.value(type) != -1)            // check if this 'type' has any column 'number' assigned...
        colNumType.remove(colTypeNum.value(type)); // ...if true remove 'type' assigned to this column 'number'

    bool ret = true;
    if (col == -1) { // user only wanted to reset his column so allow him
        colTypeNum[type] = col;  // assign new column 'number' to this 'type'
    } else if (colNumType.contains(col)) { // if this column 'number' has already 'type' assigned
        KMessageBox::information(m_dlg, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>",
                                             m_dlg->m_colTypeName.value(colNumType.value(col))));
        resetComboBox(colNumType.value(col));
        resetComboBox(type);
        ret = false;
    } else {
        colTypeNum[type] = col; // assign new column 'number' to this 'type'
        colNumType[col] = type; // assign new 'type' to this column 'number'
    }
    emit completeChanged();
    return ret;
}

bool PricesPage::validateCurrencies()
{
    if ((m_currenciesDlg.isNull() ||
            !m_imp->validateCurrencies(m_profile)) &&
            !(m_profile->m_dontAsk && m_dlg->m_skipSetup)) {
        m_currenciesDlg = new CurrenciesDlg;
        m_currenciesDlg->initializeCurrencies(m_profile->m_currencySymbol, m_profile->m_securitySymbol);
        m_currenciesDlg->ui->cbDontAsk->setChecked(m_profile->m_dontAsk);
    }

    if (!m_currenciesDlg.isNull()) {
        if (m_currenciesDlg->exec() == QDialog::Rejected) {
            return false;
        } else {
            m_profile->m_currencySymbol = m_currenciesDlg->fromCurrency();
            m_profile->m_securitySymbol = m_currenciesDlg->toCurrency();
            m_profile->m_dontAsk = m_currenciesDlg->dontAsk();
            delete m_currenciesDlg;
        }
    }
    return true;
}

bool PricesPage::validateSecurity()
{
    if (m_imp->validateSecurity(m_profile))
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
            delete m_securityDlg;                                                                       // ...but KMM allows creating duplicates, so don't bother
        }
    }
    if (m_imp->m_mapSymbolName.isEmpty())
        return false;
    return true;
}
