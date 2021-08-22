/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGENERALLOANINFOPAGE_P_H
#define KGENERALLOANINFOPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kgeneralloaninfopage.h"

#include "wizardpage_p.h"

namespace NewAccountWizard
{
class Wizard;

class GeneralLoanInfoPagePrivate : public WizardPagePrivate<Wizard>
{
    Q_DISABLE_COPY(GeneralLoanInfoPagePrivate)

public:
    explicit GeneralLoanInfoPagePrivate(QObject* parent) :
        WizardPagePrivate<Wizard>(parent),
        ui(new Ui::KGeneralLoanInfoPage),
        m_firstTime(false)
    {
    }

    ~GeneralLoanInfoPagePrivate()
    {
        delete ui;
    }

    Ui::KGeneralLoanInfoPage *ui;
    bool                      m_firstTime;
};
}

#endif
