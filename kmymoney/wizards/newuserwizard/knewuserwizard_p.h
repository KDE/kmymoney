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

#include <QList>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneywizard.h>
#include "kmymoneywizardpage.h"

#include "ui_kintropagedecl.h"
#include "ui_kaccountpagedecl.h"
#include "ui_kpreferencepagedecl.h"
#include "ui_kfilepagedecl.h"

#include "userinfo.h"
#include "currency.h"
#include "accounts.h"

#include "mymoneytemplate.h"
#include "mymoneycontact.h"

class Wizard;
class KJob;

namespace NewUserWizard
{

class KIntroPageDecl : public QWidget, public Ui::KIntroPageDecl
{
public:
  KIntroPageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class IntroPage : public KIntroPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  IntroPage(Wizard* parent);
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
  GeneralPage(Wizard* parent);
  KMyMoneyWizardPage* nextPage(void) const;

protected slots:
  void slotLoadFromAddressBook(void);
  void slotContactFetched(const ContactData &identity);

private:
  MyMoneyContact *m_contact;
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
  CurrencyPage(Wizard* parent);
  void enterPage(void);
  KMyMoneyWizardPage* nextPage(void) const;
};

/**
  * Wizard page collecting information about the checking account
  */

class KAccountPageDecl : public QWidget, public Ui::KAccountPageDecl
{
public:
  KAccountPageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};
class AccountPage : public KAccountPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  AccountPage(Wizard* parent);
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
  CategoriesPage(Wizard* parent);
  KMyMoneyWizardPage* nextPage(void) const;
  QList<MyMoneyTemplate> selectedTemplates(void) const;
};

/**
  * Wizard page to allow changing the preferences during setup
  *
  * @author Thomas Baumgart
  */

class KPreferencePageDecl : public QWidget, public Ui::KPreferencePageDecl
{
public:
  KPreferencePageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};
class PreferencePage : public KPreferencePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  PreferencePage(Wizard* parent);
  KMyMoneyWizardPage* nextPage(void) const;
};

/**
  * Wizard page to allow selecting the filename
  *
  * @author Thomas Baumgart
  */

class KFilePageDecl : public QWidget, public Ui::KFilePageDecl
{
public:
  KFilePageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};
class FilePage : public KFilePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  FilePage(Wizard* parent);

  virtual bool isComplete(void) const;
};

} // namespace

#endif
