/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KLOANDETAILSPAGE_H
#define KLOANDETAILSPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

namespace NewAccountWizard
{
  class Wizard;

  class LoanDetailsPagePrivate;
  class LoanDetailsPage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(LoanDetailsPage)

  public:
    explicit LoanDetailsPage(Wizard* parent);
    ~LoanDetailsPage() override;

    void enterPage() override;
    KMyMoneyWizardPage* nextPage() const override;
    virtual bool isComplete() const override;

    QWidget* initialFocusWidget() const override;

    /**
   * This method returns the number of payments depending on
   * the settings of m_termAmount and m_termUnit widgets
   */
    int term() const;

  private:
    /**
   * This method is used to update the term widgets
   * according to the length of the given @a term.
   * The term is also converted into a string and returned.
   */
    QString updateTermWidgets(const double term);

  private Q_SLOTS:
    void slotValuesChanged();
    void slotCalculate();

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, LoanDetailsPage)
    friend class Wizard;
    friend class AccountSummaryPage;
    friend class LoanPaymentPage;
  };
} // namespace

#endif
