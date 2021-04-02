/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
