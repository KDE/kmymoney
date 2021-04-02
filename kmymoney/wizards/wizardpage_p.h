/*
    SPDX-FileCopyrightText: 2006 Thomas Baumagrt <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WIZARDPAGE_P_H
#define WIZARDPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneywizardpage_p.h"

class KMyMoneyWizard;

template <class T>
class WizardPagePrivate : public KMyMoneyWizardPagePrivate
{
public:
    WizardPagePrivate(QObject* parent) :
        KMyMoneyWizardPagePrivate(parent),
        m_wizard(nullptr),
        m_wizardBase(nullptr)
    {
    }

    T*                m_wizard;
    KMyMoneyWizard*   m_wizardBase;
};

#endif
