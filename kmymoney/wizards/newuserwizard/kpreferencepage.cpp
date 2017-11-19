/***************************************************************************
                             kpreferencepage.cpp
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

#include "kpreferencepage.h"
#include "kpreferencepage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kpreferencepage.h"

#include "knewuserwizard.h"
#include "knewuserwizard_p.h"
#include "kfilepage.h"

namespace NewUserWizard
{
  PreferencePage::PreferencePage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new PreferencePagePrivate(wizard), stepCount++, this, wizard)
  {
    Q_D(PreferencePage);
    d->ui->setupUi(this);
  }

  PreferencePage::~PreferencePage()
  {
  }

  KMyMoneyWizardPage* PreferencePage::nextPage() const
  {
    Q_D(const PreferencePage);
    return d->m_wizard->d_func()->m_filePage;
  }

}
