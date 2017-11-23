/***************************************************************************
                             kloandetailspage.h
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

  private slots:
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
