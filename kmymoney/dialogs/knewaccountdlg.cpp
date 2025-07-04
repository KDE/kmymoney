/*
    SPDX-FileCopyrightText: 2000-2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knewaccountdlg.h"
#include "config-kmymoney.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QDesktopServices>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QRadioButton>
#include <QStringListModel>
#include <QTabWidget>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KComboBox>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageWidget>
#include <kguiutils.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewaccountdlg.h"

#include "accountsmodel.h"
#include "accountsproxymodel.h"
#include "columnselector.h"
#include "icons.h"
#include "kmymoneycurrencyselector.h"
#include "kmymoneysettings.h"
#include "knewinstitutiondlg.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneymoney.h"
#include "widgethintframe.h"

#include "kmmyesno.h"

using namespace eMyMoney;

class KNewAccountDlgPrivate
{
    Q_DISABLE_COPY(KNewAccountDlgPrivate)
    Q_DECLARE_PUBLIC(KNewAccountDlg)

    // keep in sync with assigned values of m_budgetInclusion in knewaccountdlg.ui
    enum AccountBudgetOptionIndex {
        NoInclusionIndex,
        IncludeAsExpenseIndex,
        IncludeAsIncomeIndex,
    };

public:
    explicit KNewAccountDlgPrivate(KNewAccountDlg* qq)
        : q_ptr(qq)
        , ui(new Ui::KNewAccountDlg)
        , m_filterProxyModel(nullptr)
        , m_frameCollection(nullptr)
        , m_categoryEditor(false)
        , m_isEditing(false)
    {
    }

    ~KNewAccountDlgPrivate()
    {
        delete ui;
    }

    void init()
    {
        Q_Q(KNewAccountDlg);
        ui->setupUi(q);

        ui->m_messageWidget->hide();
        ui->m_maxCreditEarlyEdit->setAllowEmpty();
        ui->m_maxCreditAbsoluteEdit->setAllowEmpty();
        ui->m_minBalanceEarlyEdit->setAllowEmpty();
        ui->m_minBalanceAbsoluteEdit->setAllowEmpty();

        auto file = MyMoneyFile::instance();

        // initialize the m_parentAccount member
        if (!m_account.parentAccountId().isEmpty()) {
            try {
                m_parentAccount = file->account(m_account.parentAccountId());
            } catch (MyMoneyException&) {
                m_account.setParentAccountId(QString());
            }
        }

        // assign a standard account if the selected parent is not set/found
        QVector<Account::Type> filterAccountGroup {m_account.accountGroup()};
        if (m_account.parentAccountId().isEmpty()) {
            switch (m_account.accountGroup()) {
            case Account::Type::Asset:
                m_parentAccount = file->asset();
                break;
            case Account::Type::Liability:
                m_parentAccount = file->liability();
                break;
            case Account::Type::Income:
                m_parentAccount = file->income();
                break;
            case Account::Type::Expense:
                m_parentAccount = file->expense();
                break;
            case Account::Type::Equity:
                m_parentAccount = file->equity();
                break;
            default:
                qDebug("Seems we have an account that hasn't been mapped to the top five");
                if (m_categoryEditor) {
                    m_parentAccount = file->income();
                    filterAccountGroup[0] = Account::Type::Income;
                } else {
                    m_parentAccount = file->asset();
                    filterAccountGroup[0] = Account::Type::Asset;
                }
            }
        }

        ui->m_amountGroup->setId(ui->m_grossAmount, 0);
        ui->m_amountGroup->setId(ui->m_netAmount, 1);

        // the proxy filter model
        m_filterProxyModel = ui->m_parentAccounts->proxyModel();
        m_filterProxyModel->setHideClosedAccounts(true);
        m_filterProxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
        m_filterProxyModel->setHideZeroBalancedEquityAccounts(KMyMoneySettings::hideZeroBalanceEquities());
        m_filterProxyModel->setHideZeroBalancedAccounts(KMyMoneySettings::hideZeroBalanceAccounts());
        m_filterProxyModel->addAccountGroup(filterAccountGroup);
        // don't allow to select ourself as parent
        m_filterProxyModel->setNotSelectable(m_account.id());
        auto const model = MyMoneyFile::instance()->accountsModel();
        m_filterProxyModel->setDynamicSortFilter(true);

        ui->m_parentAccounts->setModel(model);

        // only show the name column for the parent account
        auto columnSelector = new ColumnSelector(ui->m_parentAccounts);
        columnSelector->setAlwaysHidden(columnSelector->columns());
        columnSelector->setAlwaysVisible(QVector<int>({ AccountsModel::Column::AccountName }));

        ui->m_parentAccounts->sortByColumn(AccountsModel::Column::AccountName, Qt::AscendingOrder);

        columnSelector->setModel(m_filterProxyModel);

        q->connect(ui->m_parentAccounts->selectionModel(), &QItemSelectionModel::selectionChanged,
                   q, &KNewAccountDlg::slotSelectionChanged);

        // select the current parent in the hierarchy
        QModelIndex idx = model->indexById(m_account.parentAccountId());
        if (idx.isValid()) {
            idx = model->mapFromBaseSource(m_filterProxyModel, idx);
            ui->m_parentAccounts->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent);
            ui->m_parentAccounts->expand(idx);
            ui->m_parentAccounts->scrollTo(idx, QAbstractItemView::PositionAtTop);
        }

        ui->accountNameEdit->setText(m_account.name());
        ui->descriptionEdit->setText(m_account.description());

        ui->typeCombo->setEnabled(true);

        // load the price mode combo
        ui->m_priceMode->insertItem(i18nc("default price mode", "(default)"), (int)eMyMoney::Invest::PriceMode::Price);
        ui->m_priceMode->insertItem(i18n("Price per share"), (int)eMyMoney::Invest::PriceMode::PricePerShare);
        ui->m_priceMode->insertItem(i18n("Total for all shares"), (int)eMyMoney::Invest::PriceMode::PricePerTransaction);

        if (m_account.accountType() == Account::Type::Investment) {
            ui->m_priceMode->setEnabled(true);
        }
        ui->m_priceMode->setCurrentItem((int)m_account.priceMode());

        bool haveMinBalance = false;
        bool haveMaxCredit = false;
        if (!m_account.openingDate().isValid()) {
            m_account.setOpeningDate(KMyMoneySettings::firstFiscalDate());
        }
        ui->m_openingDateEdit->setDate(m_account.openingDate());

        handleOpeningBalanceCheckbox(m_account.currencyId());

        // the filter buttons have no function here
        ui->m_vatAccount->removeButtons();

        if (m_categoryEditor) {
            // get rid of the widgets that are not used for categories
            int tab = ui->m_tab->indexOf(ui->m_institutionTab);
            if (tab != -1)
                ui->m_tab->removeTab(tab);

            ui->m_limitsGroupBox->hide();
            ui->m_importGroupBox->hide();

            ui->accountNoEdit->setEnabled(false);

            ui->m_institutionBox->hide();
            ui->m_qcheckboxNoVat->hide();
            ui->m_budgetGroupBox->hide();

            // no web site for categories
            ui->m_websiteLabel->hide();
            ui->m_protocolLabel->hide();
            ui->m_urlEdit->hide();

            ui->m_budgetInclusion->setCurrentIndex(accountTypeToBudgetOptionIndex(eMyMoney::Account::Type::Unknown));

            ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Income), (int)Account::Type::Income);
            ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Expense), (int)Account::Type::Expense);

            // Hardcoded but acceptable - if above we set the default to income do the same here
            switch (m_account.accountType()) {
            case Account::Type::Expense:
                ui->typeCombo->setCurrentItem(MyMoneyAccount::accountTypeToString(Account::Type::Expense), false);
                break;

            case Account::Type::Income:
            default:
                ui->typeCombo->setCurrentItem(MyMoneyAccount::accountTypeToString(Account::Type::Income), false);
                break;
            }
            ui->m_currency->setEnabled(true);
            if (m_isEditing) {
                ui->typeCombo->setEnabled(false);
                ui->m_currency->setDisabled(MyMoneyFile::instance()->isReferenced(m_account));
            }
            ui->m_qcheckboxPreferred->hide();

            ui->m_qcheckboxTax->setChecked(m_account.isInTaxReports());
            ui->m_costCenterRequiredCheckBox->setChecked(m_account.isCostCenterRequired());

#ifndef ENABLE_COSTCENTER
            ui->m_costCenterRequiredCheckBox->hide();
#endif
            // no need to show the price entry mode for categories
            ui->m_priceMode->hide();
            ui->m_priceModeLabel->hide();

            loadVatAccounts();
        } else {
            // adjust the widgets that are not used for accounts
            ui->m_vatCategory->setText(i18n("VAT account"));
            ui->m_qcheckboxTax->setChecked(m_account.isInTaxReports());
            loadVatAccounts();

            ui->m_costCenterRequiredCheckBox->hide();

            switch (m_account.accountType()) {
            case Account::Type::Savings:
            case Account::Type::Cash:
                haveMinBalance = true;
                break;

            case Account::Type::Checkings:
                haveMinBalance = true;
                haveMaxCredit = true;
                break;

            case Account::Type::CreditCard:
                haveMaxCredit = true;
                break;

            default:
                // no limit available, so we might get rid of the tab
                ui->m_limitsGroupBox->hide();
                // no need to hide the widgets separately
                // in the next step
                haveMaxCredit = haveMinBalance = true;
                break;
            }

            if (!haveMaxCredit) {
                ui->m_maxCreditLabel->setEnabled(false);
                ui->m_maxCreditLabel->hide();
                ui->m_maxCreditEarlyEdit->hide();
                ui->m_maxCreditAbsoluteEdit->hide();
            }
            if (!haveMinBalance) {
                ui->m_minBalanceLabel->setEnabled(false);
                ui->m_minBalanceLabel->hide();
                ui->m_minBalanceEarlyEdit->hide();
                ui->m_minBalanceAbsoluteEdit->hide();
            }

            QString typeString = MyMoneyAccount::accountTypeToString(m_account.accountType());

            if (m_isEditing) {
                if (m_account.isLiquidAsset()) {
                    ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Checkings), (int)Account::Type::Checkings);
                    ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Savings), (int)Account::Type::Savings);
                    ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Cash), (int)Account::Type::Cash);
                } else {
                    ui->typeCombo->addItem(typeString, (int)m_account.accountType());
                    // Once created, accounts of other account types are not
                    // allowed to be changed.
                    ui->typeCombo->setEnabled(false);
                }
                // Once created, a currency cannot be changed if it is referenced.
                ui->m_currency->setDisabled(MyMoneyFile::instance()->isReferenced(m_account));
            } else {
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Checkings), (int)Account::Type::Checkings);
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Savings), (int)Account::Type::Savings);
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Cash), (int)Account::Type::Cash);
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::CreditCard), (int)Account::Type::CreditCard);
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Loan), (int)Account::Type::Loan);
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Investment), (int)Account::Type::Investment);
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Asset), (int)Account::Type::Asset);
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Liability), (int)Account::Type::Liability);
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Stock), (int)Account::Type::Stock);
                /*
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::CertificateDep), (int)Account::Type::CertificateDep);
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::MoneyMarket), (int)Account::Type::MoneyMarket);
                ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Currency), (int)Account::Type::Currency);
                */
                // Do not create account types that are not supported
                // by the current engine.
                if (m_account.accountType() == Account::Type::Unknown //
                    || m_account.accountType() == Account::Type::CertificateDep //
                    || m_account.accountType() == Account::Type::MoneyMarket //
                    || m_account.accountType() == Account::Type::Currency)
                    typeString = MyMoneyAccount::accountTypeToString(Account::Type::Checkings);
            }

            ui->typeCombo->setCurrentItem(typeString, false);

            if (m_account.isInvest())
                ui->m_institutionBox->hide();

            ui->accountNoEdit->setText(m_account.number());
            ui->m_budgetInclusion->setCurrentIndex(accountTypeToBudgetOptionIndex(m_account.budgetAccountType()));
            ui->m_payeeCreation->setCurrentIndex(static_cast<int>(m_account.payeeCreation()));

            loadKVP("PreferredAccount", ui->m_qcheckboxPreferred);
            loadKVP("NoVat", ui->m_qcheckboxNoVat);
            loadKVP("iban", ui->ibanEdit);
            loadKVP("minBalanceAbsolute", ui->m_minBalanceAbsoluteEdit);
            loadKVP("minBalanceEarly", ui->m_minBalanceEarlyEdit);
            loadKVP("maxCreditAbsolute", ui->m_maxCreditAbsoluteEdit);
            loadKVP("maxCreditEarly", ui->m_maxCreditEarlyEdit);
            // reverse the sign for display purposes
            if (!ui->m_maxCreditAbsoluteEdit->text().isEmpty())
                ui->m_maxCreditAbsoluteEdit->setValue(ui->m_maxCreditAbsoluteEdit->value()*MyMoneyMoney::MINUS_ONE);
            if (!ui->m_maxCreditEarlyEdit->text().isEmpty())
                ui->m_maxCreditEarlyEdit->setValue(ui->m_maxCreditEarlyEdit->value()*MyMoneyMoney::MINUS_ONE);
            loadKVP("lastNumberUsed", ui->m_lastCheckNumberUsed);
            loadKVP("url", ui->m_urlEdit);

            if (m_account.isInvest()) {
                ui->typeCombo->setEnabled(false);
                ui->m_qcheckboxPreferred->hide();
                ui->m_currencyText->hide();
                ui->m_currency->hide();
            } else {
                // use the old field and override a possible new value
                if (!MyMoneyMoney(m_account.value("minimumBalance")).isZero()) {
                    ui->m_minBalanceAbsoluteEdit->setValue(MyMoneyMoney(m_account.value("minimumBalance")));
                }
            }

            //    ui->m_qcheckboxTax->hide(); TODO should only be visible for VAT category/account
        }

        ui->m_currency->setSecurity(file->currency(m_account.currencyId()));

        // Load the institutions
        // then the accounts
        QString institutionName;

        try {
            if (m_isEditing && !m_account.institutionId().isEmpty()) {
                const auto institution = file->institution(m_account.institutionId());
                institutionName = institution.name();
                ui->m_urlEdit->setPlaceholderText(institution.value(QLatin1String("url")));
            } else {
                institutionName.clear();
                ui->m_urlEdit->setPlaceholderText(QString());
            }
        } catch (const MyMoneyException &e) {
            qDebug("exception in init for account dialog: %s", e.what());
        }

        // if it is an investment type account only allow to re-parent to other investment
        if (m_account.isInvest()) {
            m_filterProxyModel->setSelectableAccountTypes(KMMSet<eMyMoney::Account::Type>{eMyMoney::Account::Type::Investment});
        }

        if (!m_categoryEditor)
            q->slotLoadInstitutions(institutionName);

        ui->accountNameEdit->setFocus();

        q->connect(ui->buttonBox, &QDialogButtonBox::rejected, q, &QDialog::reject);
        q->connect(ui->buttonBox, &QDialogButtonBox::accepted, q, &KNewAccountDlg::okClicked);
        q->connect(ui->m_qbuttonNew, &QAbstractButton::clicked, q, &KNewAccountDlg::slotNewClicked);
        q->connect(ui->typeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), q, &KNewAccountDlg::slotAccountTypeChanged);

        q->connect(ui->accountNameEdit, &QLineEdit::textChanged, q, &KNewAccountDlg::slotCheckFinished);

        q->connect(ui->m_vatCategory,   &QAbstractButton::toggled,       q, &KNewAccountDlg::slotVatChanged);
        q->connect(ui->m_vatAssignment, &QAbstractButton::toggled,       q, &KNewAccountDlg::slotVatAssignmentChanged);
        q->connect(ui->m_vatCategory,   &QAbstractButton::toggled,       q, &KNewAccountDlg::slotCheckFinished);
        q->connect(ui->m_vatAssignment, &QAbstractButton::toggled,       q, &KNewAccountDlg::slotCheckFinished);
        q->connect(ui->m_vatRate,       &AmountEdit::textChanged,      q, &KNewAccountDlg::slotCheckFinished);
        q->connect(ui->m_vatAccount,    &KMyMoneySelector::stateChanged, q, &KNewAccountDlg::slotCheckFinished);
        q->connect(ui->m_currency, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), q, &KNewAccountDlg::slotCheckCurrency);

        q->connect(ui->m_minBalanceEarlyEdit, &AmountEdit::amountChanged, q, &KNewAccountDlg::slotAdjustMinBalanceAbsoluteEdit);
        q->connect(ui->m_minBalanceAbsoluteEdit, &AmountEdit::amountChanged, q, &KNewAccountDlg::slotAdjustMinBalanceEarlyEdit);
        q->connect(ui->m_maxCreditEarlyEdit, &AmountEdit::amountChanged, q, &KNewAccountDlg::slotAdjustMaxCreditAbsoluteEdit);
        q->connect(ui->m_maxCreditAbsoluteEdit, &AmountEdit::amountChanged, q, &KNewAccountDlg::slotAdjustMaxCreditEarlyEdit);

        q->connect(ui->m_qcomboboxInstitutions, &QComboBox::textActivated, q, &KNewAccountDlg::slotLoadInstitutions);

        m_frameCollection = new WidgetHintFrameCollection(q);
        m_frameCollection->addFrame(new WidgetHintFrame(ui->accountNameEdit));
        m_frameCollection->addFrame(new WidgetHintFrame(ui->m_vatRate));
        m_frameCollection->addFrame(new WidgetHintFrame(ui->m_vatAccount));
        m_frameCollection->addWidget(ui->buttonBox->button(QDialogButtonBox::Ok));

        QModelIndex parentIndex;
        if (!m_parentAccount.id().isEmpty()) {
            const auto baseIdx = model->indexById(m_parentAccount.id());
            parentIndex = m_filterProxyModel->mapFromSource(baseIdx);
        }
        selectParentAccount(parentIndex);

        ui->m_vatCategory->setChecked(false);
        ui->m_vatAssignment->setChecked(false);

        // make sure our account does not have an id and no parent assigned
        // and certainly no children in case we create a new account
        if (!m_isEditing) {
            m_account.clearId();
            m_account.setParentAccountId(QString());
            m_account.removeAccountIds();
        } else {
            if (!m_account.value("VatRate").isEmpty()) {
                ui->m_vatCategory->setChecked(true);
                ui->m_vatRate->setValue(MyMoneyMoney(m_account.value("VatRate"))*MyMoneyMoney(100, 1));
            } else {
                if (!m_account.value("VatAccount").isEmpty()) {
                    QString accId = m_account.value("VatAccount").toLatin1();
                    try {
                        // make sure account exists
                        MyMoneyFile::instance()->account(accId);
                        ui->m_vatAssignment->setChecked(true);
                        ui->m_vatAccount->setSelected(accId);
                        ui->m_grossAmount->setChecked(true);
                        if (m_account.value("VatAmount") == "Net")
                            ui->m_netAmount->setChecked(true);
                    } catch (const MyMoneyException &) {
                    }
                }
            }
        }
        q->slotVatChanged(ui->m_vatCategory->isChecked());
        q->slotVatAssignmentChanged(ui->m_vatAssignment->isChecked());
        q->slotCheckFinished();
    }

    void loadKVP(const QString& key, AmountEdit* widget)
    {
        if (widget) {
            if (m_account.value(key).isEmpty()) {
                widget->setText(QString());
            } else {
                widget->setValue(MyMoneyMoney(m_account.value(key)));
            }
        }
    }

    void loadKVP(const QString& key, KLineEdit* widget)
    {
        if (widget) {
            widget->setText(m_account.value(key));
        }
    }

    void loadKVP(const QString& key, QCheckBox* widget, bool defaultValue = false)
    {
        if (widget) {
            widget->setChecked(m_account.value(key, defaultValue));
        }
    }

    void storeKVP(const QString& key, const QString& text, const QString& value)
    {
        if (text.isEmpty())
            m_account.deletePair(key);
        else
            m_account.setValue(key, value);
    }

    void storeKVP(const QString& key, QCheckBox* widget)
    {
        if (widget) {
            m_account.setValue(key, widget->isChecked(), false);
        }
    }

    void storeKVP(const QString& key, AmountEdit* widget)
    {
        storeKVP(key, widget->text(), widget->value().toString());
    }

    void storeKVP(const QString& key, KLineEdit* widget)
    {
        storeKVP(key, widget->text(), widget->text());
    }

    void loadVatAccounts()
    {
        QList<MyMoneyAccount> list;
        MyMoneyFile::instance()->accountList(list);
        QList<MyMoneyAccount>::Iterator it;
        QStringList loadListExpense;
        QStringList loadListIncome;
        QStringList loadListAsset;
        QStringList loadListLiability;
        for (it = list.begin(); it != list.end(); ++it) {
            if (!(*it).value("VatRate").isEmpty()) {
                if ((*it).accountType() == Account::Type::Expense)
                    loadListExpense += (*it).id();
                else if ((*it).accountType() == Account::Type::Income)
                    loadListIncome += (*it).id();
                else if ((*it).accountType() == Account::Type::Asset)
                    loadListAsset += (*it).id();
                else if ((*it).accountType() == Account::Type::Liability)
                    loadListLiability += (*it).id();
            }
        }
        AccountSet vatSet;
        if (!loadListAsset.isEmpty())
            vatSet.load(ui->m_vatAccount, i18n("Asset"), loadListAsset, true);
        if (!loadListLiability.isEmpty())
            vatSet.load(ui->m_vatAccount, i18n("Liability"), loadListLiability, false);
        if (!loadListIncome.isEmpty())
            vatSet.load(ui->m_vatAccount, i18n("Income"), loadListIncome, false);
        if (!loadListExpense.isEmpty())
            vatSet.load(ui->m_vatAccount, i18n("Expense"), loadListExpense, false);
    }

    void adjustEditWidgets(AmountEdit* dst, AmountEdit* src, char mode, int corr)
    {
        MyMoneyMoney factor(corr, 1);
        if (m_account.accountGroup() == Account::Type::Asset)
            factor = -factor;

        switch (mode) {
        case '<':
            if (src->value()*factor < dst->value()*factor)
                dst->setValue(src->value());
            break;

        case '>':
            if (src->value()*factor > dst->value()*factor)
                dst->setValue(src->value());
            break;
        }
    }

    void handleOpeningBalanceCheckbox(const QString &currencyId)
    {
        if (m_account.accountType() == Account::Type::Equity) {
            // check if there is another opening balance account with the same currency
            bool isOtherOpenBalancingAccount = false;
            QList<MyMoneyAccount> list;
            MyMoneyFile::instance()->accountList(list);
            QList<MyMoneyAccount>::Iterator it;
            for (it = list.begin(); it != list.end(); ++it) {
                if (it->id() == m_account.id() || currencyId != it->currencyId()
                        || it->accountType() != Account::Type::Equity)
                    continue;
                if (it->value("OpeningBalanceAccount", false)) {
                    isOtherOpenBalancingAccount = true;
                    break;
                }
            }
            if (!isOtherOpenBalancingAccount) {
                bool isOpenBalancingAccount = m_account.value("OpeningBalanceAccount", false);
                ui->m_qcheckboxOpeningBalance->setChecked(isOpenBalancingAccount);
                if (isOpenBalancingAccount) {
                    // let only allow state change if no transactions are assigned to this account
                    bool hasTransactions = MyMoneyFile::instance()->transactionCount(m_account.id()) != 0;
                    ui->m_qcheckboxOpeningBalance->setEnabled(!hasTransactions);
                    if (hasTransactions)
                        ui->m_qcheckboxOpeningBalance->setToolTip(i18n("Option has been disabled because there are transactions assigned to this account"));
                }
            } else {
                ui->m_qcheckboxOpeningBalance->setChecked(false);
                ui->m_qcheckboxOpeningBalance->setEnabled(false);
                ui->m_qcheckboxOpeningBalance->setToolTip(i18n("Option has been disabled because there is another account flagged to be an opening balance account for this currency"));
            }
        } else {
            ui->m_qcheckboxOpeningBalance->setVisible(false);
        }
    }

    void selectParentAccount(const QModelIndex& parentIndex)
    {
        ui->m_parentAccounts->expand(parentIndex);
        ui->m_parentAccounts->setCurrentIndex(parentIndex);
        ui->m_parentAccounts->selectionModel()->select(parentIndex, QItemSelectionModel::SelectCurrent);
        ui->m_parentAccounts->scrollTo(parentIndex, QAbstractItemView::PositionAtCenter);
    }

    void changeHierarchyLabel()
    {
        const auto idx = ui->m_parentAccounts->currentIndex();
        const auto fullName = idx.data(eMyMoney::Model::AccountFullHierarchyNameRole).toString();
        ui->m_subAccountLabel->setText(
            i18nc("@label:chooser %1 account name, %2 parent account name", "<b>%1</b> is a sub account of <b>%2</b>", ui->accountNameEdit->text(), fullName));
    }

    static void createAccount(MyMoneyAccount& account, const MyMoneyAccount& parent, bool isCategory, const QString& title)
    {
        if (!parent.id().isEmpty()) {
            try {
                // make sure parent account exists
                MyMoneyFile::instance()->account(parent.id());
                account.setParentAccountId(parent.id());
                account.setAccountType(parent.accountType());
            } catch (const MyMoneyException&) {
            }
        }

        QPointer<KNewAccountDlg> dialog = new KNewAccountDlg(account, false, isCategory, nullptr, title);

        dialog->setOpeningBalanceShown(false);
        dialog->setOpeningDateShown(false);

        if (dialog->exec() == QDialog::Accepted && dialog != nullptr) {
            MyMoneyAccount parentAccount, brokerageAccount;
            account = dialog->account();
            parentAccount = dialog->parentAccount();

            MyMoneyFile::instance()->createAccount(account, parentAccount, brokerageAccount, MyMoneyMoney());
        }
        delete dialog;
    }

    AccountBudgetOptionIndex accountTypeToBudgetOptionIndex(eMyMoney::Account::Type type) const
    {
        switch (type) {
        case eMyMoney::Account::Type::Income:
            return IncludeAsIncomeIndex;
        case eMyMoney::Account::Type::Expense:
            return IncludeAsExpenseIndex;
        default:
            break;
        }
        return NoInclusionIndex;
    }

    eMyMoney::Account::Type budgetOptionIndexToAccountType(int option) const
    {
        switch (option) {
        case IncludeAsExpenseIndex:
            return eMyMoney::Account::Type::Expense;
        case IncludeAsIncomeIndex:
            return eMyMoney::Account::Type::Income;
        default:
            break;
        }
        return eMyMoney::Account::Type::Unknown;
    }

    void urlChanged(QLineEdit* edit)
    {
        // remove a possible leading protocol since we only provide https for now
        QRegularExpression protocol(QStringLiteral("^[a-zA-Z]+://(?<url>.*)"), QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch matcher = protocol.match(edit->text());
        if (matcher.hasMatch()) {
            edit->setText(matcher.captured(QStringLiteral("url")));
            ui->m_messageWidget->setText(
                i18nc("@info:usagetip", "The protocol part has been removed by KMyMoney because it is fixed to https for security reasons."));
            ui->m_messageWidget->setMessageType(KMessageWidget::Information);
            ui->m_messageWidget->animatedShow();
        }
        updateIconButtonState();
    }

    MyMoneyInstitution selectedInstitution() const
    {
        QString institutionNameText = ui->m_qcomboboxInstitutions->currentText();
        if (institutionNameText != i18n("(No Institution)")) {
            try {
                QList<MyMoneyInstitution> list = MyMoneyFile::instance()->institutionList();
                QList<MyMoneyInstitution>::const_iterator institutionIterator;
                for (institutionIterator = list.cbegin(); institutionIterator != list.cend(); ++institutionIterator) {
                    if ((*institutionIterator).name() == institutionNameText) {
                        return *institutionIterator;
                    }
                }
            } catch (const MyMoneyException& e) {
                qDebug("Exception in account institution set: %s", e.what());
            }
        }
        return {};
    }

    void updateIcon()
    {
        const auto institution = selectedInstitution();
        ui->m_urlEdit->setPlaceholderText(institution.value(QLatin1String("url")));
        QIcon favIcon;
        if (!institution.value(QStringLiteral("icon")).isEmpty()) {
            favIcon = Icons::loadIconFromApplicationCache(institution.value(QStringLiteral("icon")));
        } else {
            favIcon = Icons::get(Icons::Icon::Institution);
        }
        ui->m_iconButton->setIcon(favIcon);
        updateIconButtonState();
    }

    void updateIconButtonState() const
    {
        ui->m_iconButton->setDisabled(ui->m_urlEdit->text().isEmpty() && ui->m_urlEdit->placeholderText().isEmpty());
    }

    KNewAccountDlg* q_ptr;
    Ui::KNewAccountDlg* ui;
    MyMoneyAccount m_account;
    MyMoneyAccount m_parentAccount;
    AccountsProxyModel* m_filterProxyModel;
    WidgetHintFrameCollection* m_frameCollection;

    bool m_categoryEditor;
    bool m_isEditing;
};

KNewAccountDlg::KNewAccountDlg(const MyMoneyAccount& account, bool isEditing, bool categoryEditor, QWidget *parent, const QString& title) :
    QDialog(parent),
    d_ptr(new KNewAccountDlgPrivate(this))
{
    Q_D(KNewAccountDlg);
    d->m_account = account;
    d->m_categoryEditor = categoryEditor;
    d->m_isEditing = isEditing;
    d->init();
    if (!title.isEmpty())
        setWindowTitle(title);

    connect(d->ui->accountNameEdit, &QLineEdit::textChanged, this, [&]() {
        Q_D(KNewAccountDlg);
        d->changeHierarchyLabel();
        slotCheckFinished();
    });

    connect(d->ui->m_urlEdit, &QLineEdit::textChanged, this, [&]() {
        Q_D(KNewAccountDlg);
        d->urlChanged(d->ui->m_urlEdit);
    });

    connect(d->ui->m_qcomboboxInstitutions, &QComboBox::currentIndexChanged, this, [&]() {
        Q_D(KNewAccountDlg);
        d->updateIcon();
    });
    connect(d->ui->m_iconButton, &QToolButton::pressed, this, [&] {
        Q_D(KNewAccountDlg);
        QUrl url;
        QString urlText;
        if (d->ui->m_urlEdit->text().isEmpty()) {
            urlText = d->ui->m_urlEdit->placeholderText();
        } else {
            urlText = d->ui->m_urlEdit->text();
        }
        url.setUrl(QString::fromLatin1("https://%1").arg(urlText));
        QDesktopServices::openUrl(url);
    });

    d->updateIcon();
}

MyMoneyMoney KNewAccountDlg::openingBalance() const
{
    Q_D(const KNewAccountDlg);
    return d->ui->m_openingBalanceEdit->value();
}

void KNewAccountDlg::setOpeningBalance(const MyMoneyMoney& balance)
{
    Q_D(KNewAccountDlg);
    d->ui->m_openingBalanceEdit->setValue(balance);
}

void KNewAccountDlg::setOpeningBalanceShown(bool shown)
{
    Q_D(KNewAccountDlg);
    d->ui->m_openingBalanceLabel->setVisible(shown);
    d->ui->m_openingBalanceEdit->setVisible(shown);
}

void KNewAccountDlg::setOpeningDateShown(bool shown)
{
    Q_D(KNewAccountDlg);
    d->ui->m_openingDateLabel->setVisible(shown);
    d->ui->m_openingDateEdit->setVisible(shown);
}

void KNewAccountDlg::okClicked()
{
    Q_D(KNewAccountDlg);
    auto file = MyMoneyFile::instance();

    QString accountNameText = d->ui->accountNameEdit->text();
    if (accountNameText.isEmpty()) {
        KMessageBox::error(this, i18n("You have not specified a name.\nPlease fill in this field."));
        d->ui->accountNameEdit->setFocus();
        return;
    }

    MyMoneyAccount parent = parentAccount();
    if (parent.name().length() == 0) {
        KMessageBox::error(this, i18n("Please select a parent account."));
        return;
    }

    if (!d->m_categoryEditor) {
        const auto institution = d->selectedInstitution();
        d->m_account.setInstitutionId(institution.id());
    }

    d->m_account.setName(accountNameText);
    d->m_account.setNumber(d->ui->accountNoEdit->text());
    d->storeKVP("iban", d->ui->ibanEdit);
    d->storeKVP("minBalanceAbsolute", d->ui->m_minBalanceAbsoluteEdit);
    d->storeKVP("minBalanceEarly", d->ui->m_minBalanceEarlyEdit);
    d->storeKVP("url", d->ui->m_urlEdit);

    // the figures for credit line with reversed sign
    if (!d->ui->m_maxCreditAbsoluteEdit->text().isEmpty())
        d->ui->m_maxCreditAbsoluteEdit->setValue(d->ui->m_maxCreditAbsoluteEdit->value()*MyMoneyMoney::MINUS_ONE);
    if (!d->ui->m_maxCreditEarlyEdit->text().isEmpty())
        d->ui->m_maxCreditEarlyEdit->setValue(d->ui->m_maxCreditEarlyEdit->value()*MyMoneyMoney::MINUS_ONE);
    d->storeKVP("maxCreditAbsolute", d->ui->m_maxCreditAbsoluteEdit);
    d->storeKVP("maxCreditEarly", d->ui->m_maxCreditEarlyEdit);
    if (!d->ui->m_maxCreditAbsoluteEdit->text().isEmpty())
        d->ui->m_maxCreditAbsoluteEdit->setValue(d->ui->m_maxCreditAbsoluteEdit->value()*MyMoneyMoney::MINUS_ONE);
    if (!d->ui->m_maxCreditEarlyEdit->text().isEmpty())
        d->ui->m_maxCreditEarlyEdit->setValue(d->ui->m_maxCreditEarlyEdit->value()*MyMoneyMoney::MINUS_ONE);

    d->storeKVP("lastNumberUsed", d->ui->m_lastCheckNumberUsed);
    // delete a previous version of the minimumbalance information
    d->storeKVP("minimumBalance", QString(), QString());

    Account::Type acctype;
    if (!d->m_categoryEditor) {
        acctype = static_cast<Account::Type>(d->ui->typeCombo->currentData().toInt());
        // If it's a loan, check if the parent is asset or liability. In
        // case of asset, we change the account type to be AssetLoan
        if (acctype == Account::Type::Loan
                && parent.accountGroup() == Account::Type::Asset)
            acctype = Account::Type::AssetLoan;
    } else {
        acctype = parent.accountGroup();
        QString newName;
        if (!MyMoneyFile::instance()->isStandardAccount(parent.id())) {
            newName = MyMoneyFile::instance()->accountToCategory(parent.id()) + MyMoneyFile::AccountSeparator;
        }
        newName += accountNameText;
        if (!file->categoryToAccount(newName, acctype).isEmpty()
                && (file->categoryToAccount(newName, acctype) != d->m_account.id())) {
            KMessageBox::error(this, QString("<qt>") + i18n("A category named <b>%1</b> already exists. You cannot create a second category with the same name.", newName) + QString("</qt>"));
            return;
        }
    }
    d->m_account.setAccountType(acctype);

    d->m_account.setDescription(d->ui->descriptionEdit->toPlainText());

    d->m_account.setOpeningDate(d->ui->m_openingDateEdit->date());

    // in case we edit a stock account, the currency is
    // not visible and we should not override it. Same
    // if the currency widget is not enabled
    if (d->ui->m_currency->isVisible() && d->ui->m_currency->isEnabled()) {
        d->m_account.setCurrencyId(d->ui->m_currency->security().id());
    }

    if (!d->m_categoryEditor) {
        d->storeKVP("PreferredAccount", d->ui->m_qcheckboxPreferred);
        d->storeKVP("NoVat", d->ui->m_qcheckboxNoVat);
        d->m_account.setBudgetAccountType(d->budgetOptionIndexToAccountType(d->ui->m_budgetInclusion->currentIndex()));
        d->m_account.setPayeeCreation(static_cast<eMyMoney::Account::PayeeCreation>(d->ui->m_payeeCreation->currentIndex()));

        if (d->ui->m_minBalanceAbsoluteEdit->isVisible()) {
            d->m_account.setValue("minimumBalance", d->ui->m_minBalanceAbsoluteEdit->value().toString());
        }

        if (KMyMoneySettings::hideZeroBalanceAccounts() && !d->m_isEditing) {
            KMessageBox::information(
                this,
                i18nc("@info",
                      "You have selected to suppress the display of accounts with a zero balance in the KMyMoney configuration dialog. The account you just "
                      "created will therefore only be shown if it is used. Otherwise, it will be hidden in all accounts views."),
                i18nc("@title:window Warning message about hidden account", "Hidden accounts"),
                QLatin1String("NewHiddenAccount"));
        }
    } else {
        if (KMyMoneySettings::hideUnusedCategory() && !d->m_isEditing) {
            KMessageBox::information(
                this,
                i18nc("@info",
                      "You have selected to suppress the display of unused categories in the KMyMoney configuration dialog. The category you just created will "
                      "therefore only be shown if it is used. Otherwise, it will be hidden in the accounts/categories view."),
                i18nc("@title:window Warning message about hidden category", "Hidden categories"),
                QLatin1String("NewHiddenCategory"));
        }
        d->m_account.setCostCenterRequired(d->ui->m_costCenterRequiredCheckBox->isChecked());
    }

    d->m_account.setIsInTaxReports(d->ui->m_qcheckboxTax->isChecked());

    d->storeKVP("OpeningBalanceAccount", d->ui->m_qcheckboxOpeningBalance);
    d->m_account.deletePair("VatAccount");
    d->m_account.deletePair("VatAmount");
    d->m_account.deletePair("VatRate");

    if (d->ui->m_vatCategory->isChecked()) {
        d->m_account.setValue("VatRate", (d->ui->m_vatRate->value().abs() / MyMoneyMoney(100, 1)).toString());
    } else {
        if (d->ui->m_vatAssignment->isChecked() && !d->ui->m_vatAccount->selectedItems().isEmpty()) {
            d->m_account.setValue("VatAccount", d->ui->m_vatAccount->selectedItems().first());
            if (d->ui->m_netAmount->isChecked())
                d->m_account.setValue("VatAmount", "Net");
        }
    }

    // update the price mode
    d->m_account.setPriceMode(d->ui->m_priceMode->currentData().value<eMyMoney::Invest::PriceMode>());

    accept();
}

MyMoneyAccount KNewAccountDlg::account()
{
    Q_D(KNewAccountDlg);
    return d->m_account;
}

MyMoneyAccount KNewAccountDlg::parentAccount() const
{
    Q_D(const KNewAccountDlg);
    return d->m_parentAccount;
}

void KNewAccountDlg::slotSelectionChanged(const QItemSelection &current, const QItemSelection &previous)
{
    Q_UNUSED(previous)
    Q_D(KNewAccountDlg);
    if (!current.indexes().empty()) {
        auto baseIdx = MyMoneyFile::baseModel()->mapToBaseSource(current.indexes().front());
        if (baseIdx.isValid()) {
            d->m_parentAccount = MyMoneyFile::instance()->accountsModel()->itemByIndex(baseIdx);
            d->changeHierarchyLabel();
        }
    }
}

void KNewAccountDlg::slotLoadInstitutions(const QString& name)
{
    Q_D(KNewAccountDlg);
    d->ui->m_qcomboboxInstitutions->model()->deleteLater();

    auto model = new QStringListModel(this);
    auto list = MyMoneyFile::instance()->institutionList();
    QStringList names;

    d->ui->m_bicValue->setText(" ");
    d->ui->ibanEdit->setEnabled(false);
    d->ui->accountNoEdit->setEnabled(false);

    QString search(i18n("(No Institution)"));

    for (const auto& institution : qAsConst(list)) {
        names << institution.name();
        if (institution.name() == name) {
            d->ui->ibanEdit->setEnabled(true);
            d->ui->accountNoEdit->setEnabled(true);
            d->ui->m_bicValue->setText(institution.value("bic"));
            search = name;
        }
    }
    model->setStringList(names);
    model->sort(0);
    model->insertRow(0);
    QModelIndex idx = model->index(0, 0);
    model->setData(idx, i18n("(No Institution)"), Qt::DisplayRole);

    d->ui->m_qcomboboxInstitutions->setModel(model);

    auto i = d->ui->m_qcomboboxInstitutions->findText(search);
    if (i == -1)
        i = 0;
    d->ui->m_qcomboboxInstitutions->setCurrentIndex(i);
}

void KNewAccountDlg::slotNewClicked()
{
    MyMoneyInstitution institution;

    QPointer<KNewInstitutionDlg> dlg = new KNewInstitutionDlg(institution, this);
    if (dlg->exec()) {
        MyMoneyFileTransaction ft;
        try {
            auto file = MyMoneyFile::instance();

            institution = dlg->institution();
            file->addInstitution(institution);
            ft.commit();
            slotLoadInstitutions(institution.name());
        } catch (const MyMoneyException &) {
            KMessageBox::information(this, i18n("Cannot add institution"));
        }
    }
    delete dlg;
}

void KNewAccountDlg::slotAccountTypeChanged(int index)
{
    Q_D(KNewAccountDlg);
    Account::Type oldType;

    auto type = d->ui->typeCombo->itemData(index).value<Account::Type>();
    try {
        oldType = d->m_account.accountType();
        if (oldType != type) {
            d->m_account.setAccountType(type);
            // update the account group displayed in the accounts hierarchy
            d->m_filterProxyModel->clear();
            d->m_filterProxyModel->addAccountGroup(QVector<Account::Type> {d->m_account.accountGroup()});
            d->selectParentAccount(d->m_filterProxyModel->index(0, 0));

        }
    } catch (const MyMoneyException &) {
        qWarning("Unexpected exception in KNewAccountDlg::slotAccountTypeChanged()");
    }
}

void KNewAccountDlg::slotCheckFinished()
{
    Q_D(KNewAccountDlg);

    WidgetHintFrame::hide(d->ui->accountNameEdit);
    WidgetHintFrame::hide(d->ui->m_vatRate);
    WidgetHintFrame::hide(d->ui->m_vatAccount);

    if (d->ui->accountNameEdit->text().isEmpty()) {
        WidgetHintFrame::show(d->ui->accountNameEdit, i18nc("@info:tooltip Hint to provide name", "Please provide a name for the new category or account."));

    } else if (d->ui->accountNameEdit->text().contains(MyMoneyAccount::accountSeparator())) {
        WidgetHintFrame::show(
            d->ui->accountNameEdit,
            i18nc("@info:tooltip %1 contains invalid character combination", "You cannot create an account or category that contains %1 in the name.")
                .arg(MyMoneyAccount::accountSeparator()));
    }

    if (d->ui->m_vatCategory->isChecked() && ((d->ui->m_vatRate->value() <= MyMoneyMoney()) || (d->ui->m_vatRate->value() >= MyMoneyMoney(100)))) {
        WidgetHintFrame::show(
            d->ui->m_vatRate,
            i18nc("@info:tooltip Hint to provide VAT percentage in range", "Please provide a percentage greater than zero and less than 100."));

    } else {
        if (d->ui->m_vatAssignment->isChecked() && d->ui->m_vatAccount->selectedItems().isEmpty()) {
            WidgetHintFrame::show(d->ui->m_vatAccount, i18nc("@info:tooltip Hint to provide category to assign VAT", "Please select a category for the VAT."));
        }
    }
}

void KNewAccountDlg::slotVatChanged(bool state)
{
    Q_D(KNewAccountDlg);
    d->ui->m_vatPercentageFrame->setEnabled(state);
    d->ui->m_vatAssignmentFrame->setDisabled(state);
    d->ui->m_vatAssignmentFrame->setVisible(!d->m_account.isAssetLiability());
}

void KNewAccountDlg::slotVatAssignmentChanged(bool state)
{
    Q_D(KNewAccountDlg);
    d->ui->m_vatCategoryFrame->setDisabled(state);
    d->ui->m_vatAccount->setEnabled(state);
    d->ui->m_amountGroupBox->setEnabled(state);
}

void KNewAccountDlg::slotAdjustMinBalanceAbsoluteEdit()
{
    Q_D(KNewAccountDlg);
    d->adjustEditWidgets(d->ui->m_minBalanceAbsoluteEdit, d->ui->m_minBalanceEarlyEdit, '<', -1);
}

void KNewAccountDlg::slotAdjustMinBalanceEarlyEdit()
{
    Q_D(KNewAccountDlg);
    d->adjustEditWidgets(d->ui->m_minBalanceEarlyEdit, d->ui->m_minBalanceAbsoluteEdit, '>', -1);
}

void KNewAccountDlg::slotAdjustMaxCreditAbsoluteEdit()
{
    Q_D(KNewAccountDlg);
    d->adjustEditWidgets(d->ui->m_maxCreditAbsoluteEdit, d->ui->m_maxCreditEarlyEdit, '>', 1);
}

void KNewAccountDlg::slotAdjustMaxCreditEarlyEdit()
{
    Q_D(KNewAccountDlg);
    d->adjustEditWidgets(d->ui->m_maxCreditEarlyEdit, d->ui->m_maxCreditAbsoluteEdit, '<', 1);
}

void KNewAccountDlg::slotCheckCurrency(int index)
{
    Q_D(KNewAccountDlg);
    Q_UNUSED(index)
    d->handleOpeningBalanceCheckbox(d->ui->m_currency->security().id());
}

void KNewAccountDlg::addTab(QWidget* w, const QString& name)
{
    Q_D(KNewAccountDlg);
    if (w) {
        w->setParent(d->ui->m_tab);
        d->ui->m_tab->addTab(w, name);
    }
}

void KNewAccountDlg::newCategory(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
    if (KMessageBox::questionTwoActions(nullptr,
                                        QString::fromLatin1("<qt>%1</qt>")
                                            .arg(i18n("<p>The category <b>%1</b> currently does not exist. Do you want to create it?</p><p><i>The parent "
                                                      "account will default to <b>%2</b> but can be changed in the following dialog</i>.</p>",
                                                      account.name(),
                                                      parent.name())),
                                        i18n("Create category"),
                                        KMMYesNo::yes(),
                                        KMMYesNo::no(),
                                        "CreateNewCategories")
        == KMessageBox::PrimaryAction) {
        KNewAccountDlg::createCategory(account, parent);
    } else {
        // we should not keep the 'no' setting because that can confuse people like
        // I have seen in some usability tests. So we just delete it right away.
        KSharedConfigPtr kconfig = KSharedConfig::openConfig();
        if (kconfig) {
            kconfig->group(QLatin1String("Notification Messages")).deleteEntry(QLatin1String("CreateNewCategories"));
        }
    }
}

void KNewAccountDlg::newAccount(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
    if (KMessageBox::questionTwoActions(
            nullptr,
            QString::fromLatin1("<qt>%1</qt>")
                .arg(i18n("<p>The account <b>%1</b> currently does not exist. Do you want to create it?</p><p><i>The parent account "
                          "will default to <b>%2</b> but can be changed in the following dialog</i>.</p>",
                          account.name(),
                          parent.name())),
            i18n("Create category"),
            KMMYesNo::yes(),
            KMMYesNo::no(),
            "CreateNewAccounts")
        == KMessageBox::PrimaryAction) {
        KNewAccountDlg::createAccount(account, parent);
    } else {
        // we should not keep the 'no' setting because that can confuse people like
        // I have seen in some usability tests. So we just delete it right away.
        KSharedConfigPtr kconfig = KSharedConfig::openConfig();
        if (kconfig) {
            kconfig->group(QLatin1String("Notification Messages")).deleteEntry(QLatin1String("CreateNewAccounts"));
        }
    }
}

void KNewAccountDlg::createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
    KNewAccountDlgPrivate::createAccount(account, parent, true, i18nc("@title:window", "Create a new Category"));
}

void KNewAccountDlg::createAccount(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
    KNewAccountDlgPrivate::createAccount(account, parent, false, i18nc("@title:window", "Create a new Account"));
}
