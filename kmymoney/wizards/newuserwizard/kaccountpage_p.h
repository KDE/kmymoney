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
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWUSERWIZARDACCOUNTPAGE_P_H
#define KNEWUSERWIZARDACCOUNTPAGE_P_H

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
    AccountPagePrivate(QObject* parent) :
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
