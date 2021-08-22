/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KLOANSCHEDULEPAGE_H
#define KLOANSCHEDULEPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class QDate;

namespace NewAccountWizard
{
class Wizard;

class LoanSchedulePagePrivate;
class LoanSchedulePage : public QWidget, public WizardPage<Wizard>
{
    Q_OBJECT
    Q_DISABLE_COPY(LoanSchedulePage)

public:
    explicit LoanSchedulePage(Wizard* parent);
    ~LoanSchedulePage() override;

    void enterPage() override;

    KMyMoneyWizardPage* nextPage() const override;

    /**
    * This method returns the due date of the first payment to be recorded.
    */
    QDate firstPaymentDueDate() const;

    QWidget* initialFocusWidget() const override;

private Q_SLOTS:
    void slotLoadWidgets();
    void slotCreateCategory(const QString& name, QString& id);

private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, LoanSchedulePage)
    friend class Wizard;
    friend class AccountSummaryPage;
};
} // namespace

#endif
