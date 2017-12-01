/***************************************************************************
                             knewuserwizard_p.h
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

#ifndef KNEWUSERWIZARD_P_H
#define KNEWUSERWIZARD_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "knewuserwizard.h"
#include "kmymoneywizard_p.h"
#include "mymoneysecurity.h"

namespace NewUserWizard
{
  class IntroPage;
  class GeneralPage;
  class CurrencyPage;
  class AccountPage;
  class CategoriesPage;
  class PreferencePage;
  class FilePage;

  class WizardPrivate : public KMyMoneyWizardPrivate
  {
    Q_DISABLE_COPY(WizardPrivate)

  public:
    explicit WizardPrivate(Wizard *qq):
      KMyMoneyWizardPrivate(qq),
      m_introPage(nullptr)
    {
    }

    ~WizardPrivate()
    {
    }

    MyMoneySecurity   m_baseCurrency;
    IntroPage*        m_introPage;
    GeneralPage*      m_generalPage;
    CurrencyPage*     m_currencyPage;
    AccountPage*      m_accountPage;
    CategoriesPage*   m_categoriesPage;
    PreferencePage*   m_preferencePage;
    FilePage*         m_filePage;
  };

} // namespace

#endif
