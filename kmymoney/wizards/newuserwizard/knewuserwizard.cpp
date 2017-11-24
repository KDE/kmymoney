/***************************************************************************
                             knewuserwizard.cpp
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

#include "knewuserwizard.h"
#include "knewuserwizard_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kaccountpage.h"
#include "kaccountpage_p.h"
#include "kcategoriespage.h"
#include "kcurrencypage.h"
#include "kfilepage.h"
#include "kfilepage_p.h"
#include "kgeneralpage.h"
#include "kintropage.h"
#include "kpreferencepage.h"
#include "kpreferencepage_p.h"

#include "mymoneysecurity.h"
#include "mymoneypayee.h"
#include "mymoneymoney.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "kmymoneydateinput.h"
#include "kmymoneyedit.h"
#include "kmymoneyglobalsettings.h"
#include "mymoneytemplate.h"
#include "mymoneyenums.h"

namespace NewUserWizard
{
  int stepCount = 0;

  Wizard::Wizard(QWidget *parent, bool modal, Qt::WindowFlags flags) :
    KMyMoneyWizard(*new WizardPrivate(this), parent, modal, flags)
  {
    Q_D(Wizard);
    bool isFirstTime = KMyMoneyGlobalSettings::firstTimeRun();

    stepCount = 1;

    setTitle(i18n("KMyMoney New File Setup"));
    if (isFirstTime)
      addStep(i18nc("New file wizard introduction", "Introduction"));
    addStep(i18n("Personal Data"));
    addStep(i18n("Select Currency"));
    addStep(i18n("Select Accounts"));
    addStep(i18n("Set preferences"));
    addStep(i18nc("Finish the wizard", "Finish"));

    if (isFirstTime)
      d->m_introPage = new IntroPage(this);
    d->m_generalPage = new GeneralPage(this);
    d->m_currencyPage = new CurrencyPage(this);
    d->m_accountPage = new AccountPage(this);
    d->m_categoriesPage = new CategoriesPage(this);
    d->m_preferencePage = new PreferencePage(this);
    d->m_filePage = new FilePage(this);

    d->m_accountPage->d_func()->ui->m_haveCheckingAccountButton->setChecked(true);
    if (isFirstTime)
      d->setFirstPage(d->m_introPage);
    else
      d->setFirstPage(d->m_generalPage);

    setHelpContext("firsttime-3");
  }

  Wizard::~Wizard()
  {
  }

  MyMoneyPayee Wizard::user() const
  {
    Q_D(const Wizard);
    return d->m_generalPage->user();
  }

  QUrl Wizard::url() const
  {
    Q_D(const Wizard);
    return d->m_filePage->d_func()->ui->m_dataFileEdit->url();
  }

  MyMoneyInstitution Wizard::institution() const
  {
    Q_D(const Wizard);
    MyMoneyInstitution inst;
    if (d->m_accountPage->d_func()->ui->m_haveCheckingAccountButton->isChecked()) {
      if (d->m_accountPage->d_func()->ui->m_institutionNameEdit->text().length()) {
        inst.setName(d->m_accountPage->d_func()->ui->m_institutionNameEdit->text());
        if (d->m_accountPage->d_func()->ui->m_institutionNumberEdit->text().length())
          inst.setSortcode(d->m_accountPage->d_func()->ui->m_institutionNumberEdit->text());
      }
    }
    return inst;
  }

  MyMoneyAccount Wizard::account() const
  {
    Q_D(const Wizard);
    MyMoneyAccount acc;
    if (d->m_accountPage->d_func()->ui->m_haveCheckingAccountButton->isChecked()) {
      acc.setName(d->m_accountPage->d_func()->ui->m_accountNameEdit->text());
      if (d->m_accountPage->d_func()->ui->m_accountNumberEdit->text().length())
        acc.setNumber(d->m_accountPage->d_func()->ui->m_accountNumberEdit->text());
      acc.setOpeningDate(d->m_accountPage->d_func()->ui->m_openingDateEdit->date());
      acc.setCurrencyId(d->m_baseCurrency.id());
      acc.setAccountType(eMyMoney::Account::Type::Checkings);
    }
    return acc;
  }

  MyMoneyMoney Wizard::openingBalance() const
  {
    Q_D(const Wizard);
    return d->m_accountPage->d_func()->ui->m_openingBalanceEdit->value();
  }

  MyMoneySecurity Wizard::baseCurrency() const
  {
    Q_D(const Wizard);
    return d->m_baseCurrency;
  }

  QList<MyMoneyTemplate> Wizard::templates() const
  {
    Q_D(const Wizard);
    return d->m_categoriesPage->selectedTemplates();
  }

  bool Wizard::startSettingsAfterFinished() const
  {
    Q_D(const Wizard);
    return d->m_preferencePage->d_func()->ui->m_openConfigAfterFinished->checkState() == Qt::Checked;
  }
}
