/***************************************************************************
                             kgeneralloaninfopage.h
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

#ifndef KGENERALLOANINFOPAGE_H
#define KGENERALLOANINFOPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class MyMoneyAccount;

namespace NewAccountWizard
{
  class Wizard;

  class GeneralLoanInfoPagePrivate;
  class GeneralLoanInfoPage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(GeneralLoanInfoPage)

  public:
    explicit GeneralLoanInfoPage(Wizard* parent);
    ~GeneralLoanInfoPage() override;

    KMyMoneyWizardPage* nextPage() const override;
    virtual bool isComplete() const override;
    void enterPage() override;
    const MyMoneyAccount& parentAccount();

    QWidget* initialFocusWidget() const override;

    /**
   * Returns @p true if the user decided to record all payments, @p false otherwise.
   */
    bool recordAllPayments() const;

  private slots:
    void slotLoadWidgets();

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, GeneralLoanInfoPage)
    friend class Wizard;
    friend class AccountSummaryPage;
    friend class LoanDetailsPage;
    friend class LoanSchedulePage;
    friend class WizardPrivate;
  };
} // namespace

#endif
