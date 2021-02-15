/***************************************************************************
                             kcategoriespage.cpp
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

#include "kcategoriespage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_accounts.h"

#include "kaccounttemplateselector.h"
#include "knewuserwizard.h"
#include "knewuserwizard_p.h"
#include "kpreferencepage.h"
#include "wizardpage_p.h"
#include "mymoneytemplate.h"

namespace NewUserWizard
{
  class CategoriesPagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(CategoriesPagePrivate)

  public:
    CategoriesPagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent)
    {
    }
  };

  CategoriesPage::CategoriesPage(Wizard* wizard) :
    Accounts(wizard),
    WizardPage<Wizard>(*new CategoriesPagePrivate(wizard), stepCount++, this, wizard)
  {
  }

  CategoriesPage::~CategoriesPage()
  {
  }

  KMyMoneyWizardPage* CategoriesPage::nextPage() const
  {
    Q_D(const CategoriesPage);
    return d->m_wizard->d_func()->m_preferencePage;
  }

  QList<MyMoneyTemplate> CategoriesPage::selectedTemplates() const
  {
    return ui->m_templateSelector->selectedTemplates();
  }

}
