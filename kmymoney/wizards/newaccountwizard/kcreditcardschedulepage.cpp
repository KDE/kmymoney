/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcreditcardschedulepage.h"
#include "kcreditcardschedulepage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QDate>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kcreditcardschedulepage.h"

#include "kmymoneyaccountselector.h"
#include "kmymoneycategory.h"
#include "kmymoneydateinput.h"
#include "kmymoneygeneralcombo.h"
#include "kmymoneypayeecombo.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kaccounttypepage.h"
#include "kaccounttypepage_p.h"
#include "khierarchypage.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "kguiutils.h"
#include "mymoneypayee.h"
#include "wizardpage.h"

using namespace eMyMoney;

namespace NewAccountWizard
{
CreditCardSchedulePage::CreditCardSchedulePage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new CreditCardSchedulePagePrivate(wizard), StepSchedule, this, wizard)
{
    Q_D(CreditCardSchedulePage);
    d->ui->setupUi(this);

    // reduce the amount of characters shown for a payee
    d->ui->m_payee->setMinimumContentsLength(40);
    d->ui->m_payee->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

    d->m_mandatoryGroup->add(d->ui->m_name);
    d->m_mandatoryGroup->add(d->ui->m_payee);
    d->m_mandatoryGroup->add(d->ui->m_amount);
    d->m_mandatoryGroup->add(d->ui->m_paymentAccount);

    connect(d->ui->m_paymentAccount, &KMyMoneyCombo::itemSelected, object(), &KMyMoneyWizardPagePrivate::completeStateChanged);
    connect(d->ui->m_payee, &KMyMoneyMVCCombo::itemSelected, object(), &KMyMoneyWizardPagePrivate::completeStateChanged);
    connect(d->ui->m_date, &KMyMoneyDateInput::dateChanged, object(), &KMyMoneyWizardPagePrivate::completeStateChanged);

    connect(d->ui->m_payee, &KMyMoneyMVCCombo::createItem, wizard, &Wizard::slotPayeeNew);

    d->ui->m_reminderCheckBox->setChecked(true);
    connect(d->ui->m_reminderCheckBox, &QAbstractButton::toggled, object(), &KMyMoneyWizardPagePrivate::completeStateChanged);

    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &CreditCardSchedulePage::slotLoadWidgets);

    d->ui->m_method->insertItem(i18n("Write check"), (int)Schedule::PaymentType::WriteChecque);
    d->ui->m_method->insertItem(i18n("Direct debit"), (int)Schedule::PaymentType::DirectDebit);
    d->ui->m_method->insertItem(i18n("Bank transfer"), (int)Schedule::PaymentType::BankTransfer);
    d->ui->m_method->insertItem(i18n("Standing order"), (int)Schedule::PaymentType::StandingOrder);
    d->ui->m_method->insertItem(i18n("Manual deposit"), (int)Schedule::PaymentType::ManualDeposit);
    d->ui->m_method->insertItem(i18n("Direct deposit"), (int)Schedule::PaymentType::DirectDeposit);
    d->ui->m_method->insertItem(i18nc("Other payment method", "Other"), (int)Schedule::PaymentType::Other);
    d->ui->m_method->setCurrentItem((int)Schedule::PaymentType::DirectDebit);

    slotLoadWidgets();
}

CreditCardSchedulePage::~CreditCardSchedulePage()
{
}

void CreditCardSchedulePage::enterPage()
{
    Q_D(CreditCardSchedulePage);
    if (d->ui->m_name->text().isEmpty())
        d->ui->m_name->setText(i18n("Credit Card %1 monthly payment", d->m_wizard->d_func()->m_accountTypePage->d_func()->ui->m_accountName->text()));
}

QWidget* CreditCardSchedulePage::initialFocusWidget() const
{
    Q_D(const CreditCardSchedulePage);
    return d->ui->m_reminderCheckBox;
}

bool CreditCardSchedulePage::isComplete() const
{
    Q_D(const CreditCardSchedulePage);
    bool rc = true;
    QString msg = i18n("Finish entry and create account");
    if (d->ui->m_reminderCheckBox->isChecked()) {
        msg = i18n("Finish entry and create account and schedule");
        if (d->ui->m_date->date() < d->m_wizard->d_func()->m_accountTypePage->d_func()->ui->m_openingDate->date()) {
            rc = false;
            msg = i18n("Next due date is prior to opening date");
        }
        if (d->ui->m_paymentAccount->selectedItem().isEmpty()) {
            rc = false;
            msg = i18n("No account selected");
        }
        if (d->ui->m_amount->text().isEmpty()) {
            rc = false;
            msg = i18n("No amount for payment selected");
        }
        if (d->ui->m_payee->selectedItem().isEmpty()) {
            rc = false;
            msg = i18n("No payee for payment selected");
        }
        if (d->ui->m_name->text().isEmpty()) {
            rc = false;
            msg = i18n("No name assigned for schedule");
        }
    }
    d->m_wizard->d_func()->m_nextButton->setToolTip(msg);

    return rc;
}

void CreditCardSchedulePage::slotLoadWidgets()
{
    Q_D(CreditCardSchedulePage);
    AccountSet set;
    set.addAccountGroup(Account::Type::Asset);
    set.load(d->ui->m_paymentAccount->selector());

    d->ui->m_payee->loadPayees(MyMoneyFile::instance()->payeeList());
}

KMyMoneyWizardPage* CreditCardSchedulePage::nextPage() const
{
    Q_D(const CreditCardSchedulePage);
    return d->m_wizard->d_func()->m_hierarchyPage;
}
}
