/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kinvestmentdetailswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinvestmentdetailswizardpage.h"

#include "kmymoneymoneyvalidator.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "securitiesmodel.h"

class KInvestmentDetailsWizardPage::Private
{
public:
    Private(KInvestmentDetailsWizardPage* qq)
        : ui(new Ui::KInvestmentDetailsWizardPage)
        , q(qq)
        , m_currentSymbolIndex(-1)
        , m_symbolUsageCount(0)
        , m_startedWithoutData(false)
    {
    }

    ~Private()
    {
        delete ui;
    }

    void enableSecurityCreation(bool enable)
    {
        m_startedWithoutData = enable;
        // ui->m_investmentSymbol->lineEdit()->setReadOnly(!m_startedWithoutData);
    }

    void enableEditSecurityDetails(bool enable)
    {
        ui->m_investmentName->setEnabled(enable);
        ui->m_fraction->setEnabled(enable);
        ui->m_tradingMarket->setEnabled(enable);
        ui->m_investmentIdentification->setEnabled(enable);
        ui->m_tradingCurrencyEdit->setEnabled(enable);
        ui->m_pricePrecision->setEnabled(enable);
        ui->m_roundingMethod->setEnabled(enable);
    }
    void enableEditAccountDetails(bool enable)
    {
        ui->m_priceMode->setEnabled(enable);
    }

    void symbolSelectionChanged(int idx)
    {
        if (m_startedWithoutData) {
            // modification is only allowed on the first item
            ui->m_investmentSymbol->lineEdit()->setReadOnly(idx != 0);
            ui->m_createSymbolDuplicate->setEnabled(idx != 0);
            enableEditSecurityDetails(idx == 0);
            enableEditAccountDetails(idx == 0);
        }

        if (idx != m_currentSymbolIndex) {
            const auto file = MyMoneyFile::instance();
            const auto model = ui->m_investmentSymbol->model();
            const auto index = MyMoneyModelBase::mapToBaseSource(model->index(idx, 0));
            const auto security = file->securitiesModel()->itemByIndex(index);

            ui->m_investmentSymbol->setCurrentText(security.tradingSymbol());

            // in case we are creating the security, we copy the data
            if (m_startedWithoutData) {
                ui->m_investmentName->setText(security.name());
                MyMoneySecurity tradingCurrency = file->currency(security.tradingCurrency());
                ui->m_tradingMarket->setCurrentIndex(ui->m_tradingMarket->findText(security.tradingMarket(), Qt::MatchExactly));
                if (security.roundingMethod() == AlkValue::RoundNever)
                    ui->m_roundingMethod->setCurrentIndex(0);
                else
                    ui->m_roundingMethod->setCurrentIndex(ui->m_roundingMethod->findData(security.roundingMethod()));
                ui->m_fraction->setValue(MyMoneyMoney(security.smallestAccountFraction(), 1));
                ui->m_pricePrecision->setValue(security.pricePrecision());
                ui->m_tradingCurrencyEdit->setSecurity(tradingCurrency);

                ui->m_investmentIdentification->setText(security.value("kmm-security-id"));
                ui->m_createSymbolDuplicate->setChecked(false);

                Q_EMIT q->securityIdChanged(security.id());
            }
        }
        m_currentSymbolIndex = idx;
    }

    int findSymbol(const QString& securityId) const
    {
        const auto model = ui->m_investmentSymbol->model();
        const auto rows = model->rowCount();
        for (int row = 0; row < rows; ++row) {
            const auto idx = model->index(row, 0);
            if (idx.data(eMyMoney::Model::IdRole).toString() == securityId) {
                return idx.row();
            }
        }
        return -1;
    }

    void checkSecurityName(const QString& txt) const
    {
        if (m_startedWithoutData) {
            const auto entryExists = (ui->m_investmentSymbol->findText(txt) != -1);
            ui->m_createSymbolDuplicate->setEnabled(entryExists);
        }
    }

    Ui::KInvestmentDetailsWizardPage* ui;
    KInvestmentDetailsWizardPage* q;
    int m_currentSymbolIndex;
    int m_symbolUsageCount;
    bool m_startedWithoutData;
    SecuritiesModelNewSecurity m_emptyModel;
    QConcatenateTablesProxyModel m_concatModel;
    QString m_originalSymbolName;
};

KInvestmentDetailsWizardPage::KInvestmentDetailsWizardPage(QWidget* parent)
    : QWizardPage(parent)
    , d(new Private(this))
{
    d->ui->setupUi(this);
    d->ui->m_messageWidget->hide();
    d->ui->m_fraction->setPrecision(0);
    d->ui->m_fraction->setValue(MyMoneyMoney(100, 1));
    KMyMoneyMoneyValidator* fractionValidator = new KMyMoneyMoneyValidator(1, 100000, 0, this);
    d->ui->m_fraction->setValidator(fractionValidator);

    // load the price mode combo
    d->ui->m_priceMode->insertItem(i18nc("default price mode", "(default)"), 0);
    d->ui->m_priceMode->insertItem(i18n("Price per share"), 1);
    d->ui->m_priceMode->insertItem(i18n("Total for all shares"), 2);

    // load the widget with the available currencies
    d->ui->m_tradingCurrencyEdit->update(QString());

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("investmentSymbol", d->ui->m_investmentSymbol->lineEdit());
    connect(d->ui->m_investmentSymbol->lineEdit(), &QLineEdit::textChanged, this, &QWizardPage::completeChanged);
    connect(d->ui->m_investmentSymbol->lineEdit(), &QLineEdit::textChanged, this, [&](const QString& txt) {
        d->checkSecurityName(txt);
    });
    connect(d->ui->m_investmentSymbol, &QComboBox::currentIndexChanged, this, [&](int idx) {
        d->symbolSelectionChanged(idx);
    });

    registerField("duplicateSymbol", d->ui->m_createSymbolDuplicate);
    connect(d->ui->m_createSymbolDuplicate, &QCheckBox::stateChanged, this, [&](int state) {
        d->enableEditSecurityDetails(state == Qt::Checked);
        d->enableEditAccountDetails(state == Qt::Checked);
    });

    registerField("investmentName", d->ui->m_investmentName);
    connect(d->ui->m_investmentName, &QLineEdit::textChanged, this, &QWizardPage::completeChanged);

    registerField("fraction", d->ui->m_fraction, "value", SIGNAL(textChanged()));
    connect(d->ui->m_fraction, &AmountEdit::textChanged, this, &QWizardPage::completeChanged);

    registerField("tradingMarket", d->ui->m_tradingMarket, "currentText", SIGNAL(currentIndexChanged(QString)));

    registerField("investmentIdentification", d->ui->m_investmentIdentification);
    connect(d->ui->m_investmentIdentification, &QLineEdit::textChanged, this, &QWizardPage::completeChanged);

    registerField("tradingCurrencyEdit", d->ui->m_tradingCurrencyEdit, "security");

    registerField("pricePrecision", d->ui->m_pricePrecision, "value", SIGNAL(valueChanged()));

    d->ui->m_roundingMethod->addItem(i18nc("Rounding method", "Round"), AlkValue::RoundRound);
    d->ui->m_roundingMethod->addItem(i18nc("Rounding method", "Ceil"), AlkValue::RoundCeil);
    d->ui->m_roundingMethod->addItem(i18nc("Rounding method", "Floor"), AlkValue::RoundFloor);
    d->ui->m_roundingMethod->addItem(i18nc("Rounding method", "Truncate"), AlkValue::RoundTruncate);
    registerField("roundingMethod", d->ui->m_roundingMethod, "currentData", SIGNAL(currentIndexChanged(int)));

    registerField("priceMode", d->ui->m_priceMode, "currentItem", SIGNAL(currentIndexChanged(int)));
}

KInvestmentDetailsWizardPage::~KInvestmentDetailsWizardPage()
{
    delete d;
}

void KInvestmentDetailsWizardPage::init(const MyMoneyAccount& account, const MyMoneySecurity& security)
{
    d->enableSecurityCreation(security.id().isEmpty() && account.id().isEmpty());

    if (!d->m_startedWithoutData) {
        d->m_originalSymbolName = security.tradingSymbol();
        d->ui->m_investmentSymbol->setModel(MyMoneyFile::instance()->securitiesModel());
        // add any trading market found in the data to the combo box
        d->m_symbolUsageCount = 0;
        const auto securities = MyMoneyFile::instance()->securityList();
        for (const auto& sec : securities) {
            if (!sec.tradingMarket().isEmpty()) {
                if (d->ui->m_tradingMarket->findText(sec.tradingMarket(), Qt::MatchExactly) == -1) {
                    d->ui->m_tradingMarket->addItem(sec.tradingMarket());
                }
            }
        }

        // collect how many accounts use this security
        QList<MyMoneyAccount> accountList;
        MyMoneyFile::instance()->accountList(accountList);
        for (const auto& acc : qAsConst(accountList)) {
            if (acc.currencyId() == security.id()) {
                ++d->m_symbolUsageCount;
            }
        }
        if (d->m_originalSymbolName.isEmpty()) {
            d->m_symbolUsageCount = 0;
        }
        if (d->m_symbolUsageCount > 1) {
            d->ui->m_messageWidget->setText(i18nc("@info Warning that security is used in multiple accounts",
                                                  "This security is used in multiple accounts. Changing the values below will be effective for all of them."));
            d->ui->m_messageWidget->animatedShow();
        }

        d->ui->m_investmentSymbol->setCurrentIndex(d->findSymbol(security.id()));

        if (!account.id().isEmpty()) {
            d->enableEditAccountDetails(true);
            d->enableEditSecurityDetails(true);
        } else {
            d->enableEditSecurityDetails(true);
            d->ui->m_priceMode->hide();
            d->ui->m_priceModeLabel->hide();
        }
        d->ui->m_investmentSymbol->setEnabled(true);
        d->ui->m_createSymbolDuplicate->hide();
    } else {
        // build security model stack to contain an empty item at the top
        d->m_concatModel.addSourceModel(&d->m_emptyModel);
        d->m_concatModel.addSourceModel(MyMoneyFile::instance()->securitiesModel());
        d->ui->m_investmentSymbol->setModel(&d->m_concatModel);
        d->ui->m_investmentSymbol->setCurrentIndex(0);
        d->ui->m_createSymbolDuplicate->setEnabled(false);
    }
    d->ui->m_investmentSymbol->setModelColumn(SecuritiesModel::Column::Symbol);
}

bool KInvestmentDetailsWizardPage::isComplete() const
{
    return (!d->ui->m_investmentName->text().isEmpty() //
            && !d->ui->m_investmentSymbol->lineEdit()->text().isEmpty() //
            && !d->ui->m_fraction->value().isZero());
}
