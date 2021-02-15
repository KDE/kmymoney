/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KLOANSCHEDULEPAGE_P_H
#define KLOANSCHEDULEPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kloanschedulepage.h"

#include "wizardpage_p.h"

namespace NewAccountWizard
{
  class Wizard;

  class LoanSchedulePagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(LoanSchedulePagePrivate)

  public:
    explicit LoanSchedulePagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KLoanSchedulePage)
    {
    }

    ~LoanSchedulePagePrivate()
    {
      delete ui;
    }

    Ui::KLoanSchedulePage *ui;
  };
}

#endif
