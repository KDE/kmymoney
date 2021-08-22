/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KPREFERENCEPAGE_P_H
#define KPREFERENCEPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kpreferencepage.h"
#include "wizardpage_p.h"

namespace NewUserWizard
{
class Wizard;
class PreferencePagePrivate : public WizardPagePrivate<Wizard>
{
    Q_DISABLE_COPY(PreferencePagePrivate)

public:
    explicit PreferencePagePrivate(QObject* parent) :
        WizardPagePrivate<Wizard>(parent),
        ui(new Ui::KPreferencePage)
    {
    }

    ~PreferencePagePrivate()
    {
        delete ui;
    }

    Ui::KPreferencePage *ui;
};
}
#endif
