/*
    SPDX-FileCopyrightText: 2003-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kaccountselectdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QPushButton>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGuiItem>
#include <KStandardGuiItem>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccountselectdlg.h"

#include "mymoneyaccount.h"
#include "mymoneyinstitution.h"
#include "mymoneyfile.h"
#include "kmymoneycategory.h"
#include "kmymoneyaccountselector.h"
#include "knewaccountdlg.h"
#include "accountsmodel.h"

#include "dialogenums.h"
#include "icons/icons.h"
#include "mymoneyenums.h"

using namespace Icons;

class KAccountSelectDlgPrivate
{
    Q_DISABLE_COPY(KAccountSelectDlgPrivate)

public:
    KAccountSelectDlgPrivate() :
        ui(new Ui::KAccountSelectDlg),
        m_mode(0),
        m_accountType(eDialogs::Category::none),
        m_aborted(false)
    {
    }

    ~KAccountSelectDlgPrivate()
    {
        delete ui;
    }

    Ui::KAccountSelectDlg *ui;
    QString                m_purpose;
    MyMoneyAccount         m_account;
    int                    m_mode;       // 0 - select or create, 1 - create only
    eDialogs::Category     m_accountType;
    bool                   m_aborted;
};

KAccountSelectDlg::KAccountSelectDlg(const eDialogs::Category accountType, const QString& purpose, QWidget *parent) :
    QDialog(parent),
    d_ptr(new KAccountSelectDlgPrivate)
{
    Q_D(KAccountSelectDlg);
    d->ui->setupUi(this);
    d->m_purpose = purpose;
    d->m_accountType = accountType;
    // Hide the abort button. It needs to be shown on request by the caller
    // using showAbortButton()
    d->ui->m_kButtonAbort->hide();

    KGuiItem skipButtonItem(i18n("&Skip"),
                            Icons::get(Icon::SkipForward),
                            i18n("Skip this transaction"),
                            i18n("Use this to skip importing this transaction and proceed with the next one."));
    KGuiItem::assign(d->ui->m_qbuttonCancel, skipButtonItem);

    KGuiItem createButtenItem(i18n("&Create..."),
                              Icons::get(Icon::DocumentNew),
                              i18n("Create a new account/category"),
                              i18n("Use this to add a new account/category to the file"));
    KGuiItem::assign(d->ui->m_createButton, createButtenItem);
    KGuiItem::assign(d->ui->m_qbuttonOk, KStandardGuiItem::ok());

    KGuiItem abortButtenItem(i18n("&Abort"),
                             Icons::get(Icon::DialogCancel),
                             i18n("Abort the import operation and dismiss all changes"),
                             i18n("Use this to abort the import. Your financial data will be in the state before you started the QIF import."));
    KGuiItem::assign(d->ui->m_kButtonAbort, abortButtenItem);

    QVector<eMyMoney::Account::Type> accountTypes;
    if (d->m_accountType & eDialogs::Category::asset)
        accountTypes.append(eMyMoney::Account::Type::Asset);
    if (d->m_accountType & eDialogs::Category::liability)
        accountTypes.append(eMyMoney::Account::Type::Liability);
    if (d->m_accountType & eDialogs::Category::income)
        accountTypes.append(eMyMoney::Account::Type::Income);
    if (d->m_accountType & eDialogs::Category::expense)
        accountTypes.append(eMyMoney::Account::Type::Expense);
    if (d->m_accountType & eDialogs::Category::equity)
        accountTypes.append(eMyMoney::Account::Type::Equity);
    if (d->m_accountType & eDialogs::Category::checking)
        accountTypes.append(eMyMoney::Account::Type::Checkings);
    if (d->m_accountType & eDialogs::Category::savings)
        accountTypes.append(eMyMoney::Account::Type::Savings);
    if (d->m_accountType & eDialogs::Category::investment)
        accountTypes.append(eMyMoney::Account::Type::Investment);
    if (d->m_accountType & eDialogs::Category::creditCard)
        accountTypes.append(eMyMoney::Account::Type::CreditCard);

    auto filterProxyModel = new AccountNamesFilterProxyModel(this);
    filterProxyModel->setHideEquityAccounts(true);
    filterProxyModel->addAccountGroup(accountTypes);

    filterProxyModel->setSourceModel(MyMoneyFile::instance()->accountsModel());
    filterProxyModel->sort(AccountsModel::Column::AccountName);

    d->ui->m_accountSelector->setModel(filterProxyModel);

    connect(d->ui->m_createButton,  &QAbstractButton::clicked, this, &KAccountSelectDlg::slotCreateAccount);
    connect(d->ui->m_qbuttonOk,     &QAbstractButton::clicked, this, &QDialog::accept);
    connect(d->ui->m_qbuttonCancel, &QAbstractButton::clicked, this, &QDialog::reject);
    connect(d->ui->m_kButtonAbort,  &QAbstractButton::clicked, this, &KAccountSelectDlg::abort);
}

KAccountSelectDlg::~KAccountSelectDlg()
{
    Q_D(KAccountSelectDlg);
    delete d;
}

void KAccountSelectDlg::setDescription(const QString& msg)
{
    Q_D(KAccountSelectDlg);
    d->ui->m_descLabel->setText(msg);
}

void KAccountSelectDlg::setHeader(const QString& msg)
{
    Q_D(KAccountSelectDlg);
    d->ui->m_headerLabel->setText(msg);
}

void KAccountSelectDlg::setAccount(const MyMoneyAccount& account, const QString& id)
{
    Q_D(KAccountSelectDlg);
    d->m_account = account;
    d->ui->m_accountSelector->setSelected(id);
}

void KAccountSelectDlg::slotCreateAccount()
{
    Q_D(KAccountSelectDlg);
    if (!((int)d->m_accountType & ((int)eDialogs::Category::expense | (int)eDialogs::Category::income))) {
        Q_EMIT createAccount(d->m_account);
        if (!d->m_account.id().isEmpty()) {
            d->ui->m_accountSelector->setSelected(d->m_account.id());
            accept();
        }
    } else {
        if (d->m_account.accountType() == eMyMoney::Account::Type::Expense)
            KNewAccountDlg::newCategory(d->m_account, MyMoneyFile::instance()->expense());
        else
            KNewAccountDlg::newCategory(d->m_account, MyMoneyFile::instance()->income());
        if (!d->m_account.id().isEmpty()) {
            d->ui->m_accountSelector->setSelected(d->m_account.id());
            accept();
        }
    }
}

void KAccountSelectDlg::abort()
{
    Q_D(KAccountSelectDlg);
    d->m_aborted = true;
    reject();
}

void KAccountSelectDlg::setMode(const int mode)
{
    Q_D(KAccountSelectDlg);
    d->m_mode = mode ? 1 : 0;
}

void KAccountSelectDlg::showAbortButton(const bool visible)
{
    Q_D(KAccountSelectDlg);
    d->ui->m_kButtonAbort->setVisible(visible);
}

bool KAccountSelectDlg::aborted() const
{
    Q_D(const KAccountSelectDlg);
    return d->m_aborted;
}

void KAccountSelectDlg::hideQifEntry()
{
    Q_D(KAccountSelectDlg);
    d->ui->m_qifEntry->hide();
}

int KAccountSelectDlg::exec()
{
    Q_D(KAccountSelectDlg);
    int rc = Rejected;

    if (d->m_mode == 1) {
        slotCreateAccount();
        rc = result();
    }
    if (rc != Accepted) {
        d->ui->m_createButton->setFocus();
        rc = QDialog::exec();
    }
    return rc;
}

QString KAccountSelectDlg::selectedAccount() const
{
    Q_D(const KAccountSelectDlg);

    return d->ui->m_accountSelector->getSelected();
}
