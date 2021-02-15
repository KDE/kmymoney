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
 *   SPDX-License-Identifier: GPL-2.0-or-later
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

}
