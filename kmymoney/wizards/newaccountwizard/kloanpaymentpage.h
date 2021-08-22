/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KLOANPAYMENTPAGE_H
#define KLOANPAYMENTPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class MyMoneyMoney;
class MyMoneySplit;

namespace NewAccountWizard
{
class Wizard;

class LoanPaymentPagePrivate;
class LoanPaymentPage : public QWidget, public WizardPage<Wizard>
{
    Q_OBJECT
    Q_DISABLE_COPY(LoanPaymentPage)

public:
    explicit LoanPaymentPage(Wizard* parent);
    ~LoanPaymentPage() override;

    KMyMoneyWizardPage* nextPage() const override;

    void enterPage() override;

    /**
    * This method returns the sum of the additional fees
    */
    MyMoneyMoney additionalFees() const;

    /**
    * This method returns the base payment, that's principal and interest
    */
    MyMoneyMoney basePayment() const;

    /**
    * This method returns the splits that make up the additional fees in @p list.
    * @note The splits may contain assigned ids which the caller must remove before
    * adding the splits to a MyMoneyTransaction object.
    */
    void additionalFeesSplits(QList<MyMoneySplit>& list);

protected Q_SLOTS:
    void slotAdditionalFees();

protected:
    void updateAmounts();

private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, LoanPaymentPage)
    friend class Wizard;
    friend class AccountSummaryPage;
};
} // namespace

#endif
