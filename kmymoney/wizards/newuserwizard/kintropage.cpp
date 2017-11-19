/***************************************************************************
                             kintropage.cpp
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

#include "kintropage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kintropage.h"

#include "knewuserwizard.h"
#include "knewuserwizard_p.h"
#include "kgeneralpage.h"

class KMyMoneyWizardPage;

namespace NewUserWizard
{
  class IntroPagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(IntroPagePrivate)

  public:
    IntroPagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KIntroPage)
    {
    }

    ~IntroPagePrivate()
    {
      delete ui;
    }

    Ui::KIntroPage *ui;
  };

  IntroPage::IntroPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new IntroPagePrivate(wizard), stepCount++, this, wizard)
  {
    Q_D(IntroPage);
    d->ui->setupUi(this);
  }

  IntroPage::~IntroPage()
  {
  }

  KMyMoneyWizardPage* IntroPage::nextPage() const
  {
    Q_D(const IntroPage);
    return d->m_wizard->d_func()->m_generalPage;
  }

}
