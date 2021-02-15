/***************************************************************************
                             kaccounttypepage.cpp
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

#ifndef KACCOUNTTYPEPAGE_P_H
#define KACCOUNTTYPEPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccounttypepage.h"

#include "wizardpage_p.h"

namespace NewAccountWizard
{
  class Wizard;

  class AccountTypePagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(AccountTypePagePrivate)

  public:
    explicit AccountTypePagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KAccountTypePage),
      m_showPriceWarning(false)
    {
    }

    ~AccountTypePagePrivate()
    {
      delete ui;
    }

    Ui::KAccountTypePage *ui;
    bool                  m_showPriceWarning;
  };
}

#endif
