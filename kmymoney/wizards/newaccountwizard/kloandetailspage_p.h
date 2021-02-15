/***************************************************************************
                             kloandetailspage.cpp
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

#ifndef KLOANDETAILSPAGE_P_H
#define KLOANDETAILSPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kloandetailspage.h"

#include "wizardpage_p.h"

namespace NewAccountWizard
{
  class Wizard;

  class LoanDetailsPagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(LoanDetailsPagePrivate)

  public:
    explicit LoanDetailsPagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KLoanDetailsPage),
      m_needCalculate(false)
    {
    }

    ~LoanDetailsPagePrivate()
    {
      delete ui;
    }

    Ui::KLoanDetailsPage *ui;
    bool m_needCalculate;
  };
}

#endif
