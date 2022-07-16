/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checkingstatementinfowizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "widgethintframe.h"

#include "ui_checkingstatementinfowizardpage.h"

CheckingStatementInfoWizardPage::CheckingStatementInfoWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::CheckingStatementInfoWizardPage)
{
    ui->setupUi(this);
    ui->m_statementDate->setDate(QDate::currentDate());

    auto frameCollection = new WidgetHintFrameCollection(this);
    frameCollection->addFrame(new WidgetHintFrame(ui->m_statementDate));

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("statementDate", ui->m_statementDate, "date");
    connect(ui->m_statementDate, &KMyMoneyDateEdit::dateValidityChanged, this, [&](const QDate& date) {
        WidgetHintFrame::hide(ui->m_statementDate);
        if (!date.isValid()) {
            WidgetHintFrame::show(ui->m_statementDate, i18nc("@info:tooltip", "The date is invalid."));
        }
        emit completeChanged();
    });

    registerField("endingBalance", ui->m_endingBalance, "value");
    connect(ui->m_endingBalance, &AmountEdit::textChanged, this, &QWizardPage::completeChanged);

    registerField("previousBalance", ui->m_previousBalance, "value", SIGNAL(textChanged()));
}

CheckingStatementInfoWizardPage::~CheckingStatementInfoWizardPage()
{
    delete ui;
}

bool CheckingStatementInfoWizardPage::isComplete() const
{
    return ui->m_statementDate->isValid() && !ui->m_endingBalance->text().isEmpty();
}
