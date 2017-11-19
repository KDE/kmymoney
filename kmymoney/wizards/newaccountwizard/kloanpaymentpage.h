/***************************************************************************
                             kloanpaymentpage.h
                             -------------------
    begin                : Tue Sep 25 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWACCOUNTWIZARDLOANPAYMENTPAGE_H
#define KNEWACCOUNTWIZARDLOANPAYMENTPAGE_H

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

    KMyMoneyWizardPage* nextPage() const;

    void enterPage();

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

  protected slots:
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
