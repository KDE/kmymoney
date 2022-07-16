/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "schedulewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_schedulewizardpage.h"

ScheduleWizardPage::ScheduleWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::ScheduleWizardPage)
{
    ui->setupUi(this);

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("nextDueDateEdit", ui->m_nextDueDateEdit, "date", SIGNAL(dateChanged(QDate)));
    registerField("paymentAccountEdit", ui->m_paymentAccountEdit, "selectedItems");

    connect(ui->m_nextDueDateEdit, &KMyMoneyDateEdit::dateChanged, this, &QWizardPage::completeChanged);
    connect(ui->m_paymentAccountEdit,  &KMyMoneySelector::stateChanged, this, &QWizardPage::completeChanged);

    ui->m_paymentAccountEdit->removeButtons();
}

ScheduleWizardPage::~ScheduleWizardPage()
{
    delete ui;
}

/**
 * Update the "Next" button
 */
bool ScheduleWizardPage::isComplete() const
{
    return ui->m_nextDueDateEdit->isValid() && ui->m_nextDueDateEdit->date() >= field("firstDueDateEdit").toDate()
        && ui->m_paymentAccountEdit->selectedItems().count() > 0;
}

void ScheduleWizardPage::initializePage()
{
    ui->m_nextDueDateEdit->setEnabled(true);
    if (field("allPaymentsButton").toBool() || field("noPreviousPaymentButton").toBool()) {
        setField("nextDueDateEdit", field("firstDueDateEdit").toDate());
        ui->m_nextDueDateEdit->setEnabled(false);
    } else {
        QDate nextPayment(QDate::currentDate().year(), 1, field("firstDueDateEdit").toDate().day());
        setField("nextDueDateEdit", nextPayment);
    }
    if (field("nextDueDateEdit").toDate() < field("firstDueDateEdit").toDate()) {
        setField("nextDueDateEdit", field("firstDueDateEdit").toDate());
    }
}
