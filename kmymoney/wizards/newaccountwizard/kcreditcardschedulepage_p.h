/***************************************************************************
                             kcreditcardschedulepage.cpp
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

#ifndef KCREDITCARDSCHEDULE_P_H
#define KCREDITCARDSCHEDULE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kcreditcardschedulepage.h"

#include "wizardpage_p.h"

namespace NewAccountWizard
{
  class Wizard;

  class CreditCardSchedulePagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(CreditCardSchedulePagePrivate)

  public:
    explicit CreditCardSchedulePagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KCreditCardSchedulePage)
    {
    }

    ~CreditCardSchedulePagePrivate()
    {
      delete ui;
    }

    Ui::KCreditCardSchedulePage *ui;
  };
}

#endif
