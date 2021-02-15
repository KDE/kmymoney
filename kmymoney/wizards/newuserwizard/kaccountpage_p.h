/***************************************************************************
                             kaccountpage_p.h
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KACCOUNTPAGE_P_H
#define KACCOUNTPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccountpage.h"
#include "wizardpage_p.h"

namespace NewUserWizard
{
  class Wizard;

  class AccountPagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(AccountPagePrivate)

  public:
    explicit AccountPagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KAccountPage)
    {
    }

    ~AccountPagePrivate()
    {
      delete ui;
    }

    Ui::KAccountPage *ui;
  };
}
#endif
