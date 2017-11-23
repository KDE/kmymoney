/***************************************************************************
                             kaccountpage.h
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
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

#ifndef KACCOUNTPAGE_H
#define KACCOUNTPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class KMyMoneyWizardPage;

namespace NewUserWizard
{
  class Wizard;

  /**
  * Wizard page collecting information about the checking account
  */
  class AccountPagePrivate;
  class AccountPage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(AccountPage)

  public:
    explicit AccountPage(Wizard* parent);
    ~AccountPage() override;

    KMyMoneyWizardPage* nextPage() const override;

    bool isComplete() const override;

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, AccountPage)
    friend class Wizard;
    friend class CurrencyPage;
  };
} // namespace

#endif
