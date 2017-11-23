/***************************************************************************
                             kloanpayoutpage.h
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

#ifndef KPAYOUT_H
#define KPAYOUT_H

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

  class LoanPayoutPagePrivate;
  class LoanPayoutPage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(LoanPayoutPage)

  public:
    explicit LoanPayoutPage(Wizard* parent);
    ~LoanPayoutPage() override;

    void enterPage();
    virtual bool isComplete() const;

    KMyMoneyWizardPage* nextPage() const;

    QWidget* initialFocusWidget() const override;

    QString payoutAccountId() const;

  private slots:
    void slotLoadWidgets();
    void slotCreateAssetAccount();
    void slotButtonsToggled();

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, LoanPayoutPage)
    friend class Wizard;
    friend class AccountSummaryPage;
  };
} // namespace

#endif
