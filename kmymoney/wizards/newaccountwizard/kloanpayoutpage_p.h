/***************************************************************************
                             kloanpayoutpage.cpp
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

#ifndef KLOANPAYOUTPAGE_P_H
#define KLOANPAYOUTPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kloanpayoutpage.h"

#include "wizardpage_p.h"

namespace NewAccountWizard
{
  class Wizard;

  class LoanPayoutPagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(LoanPayoutPagePrivate)

  public:
    explicit LoanPayoutPagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KLoanPayoutPage)
    {
    }

    ~LoanPayoutPagePrivate()
    {
      delete ui;
    }

    Ui::KLoanPayoutPage *ui;
  };
}

#endif
