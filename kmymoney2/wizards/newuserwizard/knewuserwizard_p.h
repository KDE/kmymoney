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

#include <kmymoneywizard.h>

#include "kintropagedecl.h"
#include "kaccountpagedecl.h"
#include "kpreferencepagedecl.h"
#include "kfilepagedecl.h"

#include "../wizardpages/userinfo.h"
#include "../wizardpages/currency.h"
#include "../wizardpages/accounts.h"

#include <mymoneytemplate.h>
//Added by qt3to4:
#include <Q3ValueList>

class Wizard;

namespace NewUserWizard {

class IntroPage : public KIntroPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  IntroPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;
};

/**
  * Wizard page collecting information about the user
  *
  * @author Thomas Baumgart
  */
class GeneralPage : public UserInfo, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  GeneralPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;

protected slots:
  void slotLoadFromKABC(void);
  void slotAddressBookLoaded(void);

};

/**
  * Wizard page collecting information about the base currency
  *
  * @author Thomas Baumgart
  */
class CurrencyPage : public Currency, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  CurrencyPage(Wizard* parent, const char* name = 0);
  void enterPage(void);
  KMyMoneyWizardPage* nextPage(void) const;
};

/**
  * Wizard page collecting information about the checking account
  */
class AccountPage : public KAccountPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  AccountPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;

  virtual bool isComplete(void) const;
};

/**
  * Wizard page collecting information about the account templates.
  *
  * @author Thomas Baumgart
  */
class CategoriesPage : public Accounts, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  CategoriesPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;
  Q3ValueList<MyMoneyTemplate> selectedTemplates(void) const;
};

/**
  * Wizard page to allow changing the preferences during setup
  *
  * @author Thomas Baumgart
  */
class PreferencePage : public KPreferencePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  PreferencePage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;
};

/**
  * Wizard page to allow selecting the filename
  *
  * @author Thomas Baumgart
  */
class FilePage : public KFilePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  FilePage(Wizard* parent, const char* name = 0);

  virtual bool isComplete(void) const;
};

} // namespace

#endif
