/***************************************************************************
                             kloanschedulepage.cpp
                             -------------------
    begin                : Tue Sep 25 2006
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

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
