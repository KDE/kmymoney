/***************************************************************************
                             kcurrencypage.h
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

#ifndef KCURRENCYPAGE_H
#define KCURRENCYPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"
#include "currency.h"

class KMyMoneyWizardPage;

namespace NewUserWizard
{
  class Wizard;
  /**
  * Wizard page collecting information about the base currency
  *
  * @author Thomas Baumgart
  */
  class CurrencyPagePrivate;
  class CurrencyPage : public Currency, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(CurrencyPage)

  public:
    explicit CurrencyPage(Wizard* parent);
    ~CurrencyPage() override;

    void enterPage() override;
    KMyMoneyWizardPage* nextPage() const override;

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, CurrencyPage)
    friend class Wizard;
  };
} // namespace

#endif
