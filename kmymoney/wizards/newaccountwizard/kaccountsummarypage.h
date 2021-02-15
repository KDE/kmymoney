/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KACCOUNTSUMMARYPAGE_H
#define KACCOUNTSUMMARYPAGE_H

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

  class AccountSummaryPagePrivate;
  class AccountSummaryPage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(AccountSummaryPage)

  public:
    explicit AccountSummaryPage(Wizard* parent);
    ~AccountSummaryPage() override;

    void enterPage() override;
    QWidget* initialFocusWidget() const override;

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, AccountSummaryPage)
    friend class Wizard;
  };
} // namespace

#endif
