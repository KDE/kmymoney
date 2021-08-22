/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KINSTITUTION_P_H
#define KINSTITUTION_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinstitutionpage.h"

#include "wizardpage_p.h"
#include "mymoneyinstitution.h"

namespace NewAccountWizard
{
class Wizard;

class InstitutionPagePrivate : public WizardPagePrivate<Wizard>
{
    Q_DISABLE_COPY(InstitutionPagePrivate)

public:
    explicit InstitutionPagePrivate(QObject* parent) :
        WizardPagePrivate<Wizard>(parent),
        ui(new Ui::KInstitutionPage)
    {
    }

    ~InstitutionPagePrivate()
    {
        delete ui;
    }

    Ui::KInstitutionPage      *ui;
    QList<MyMoneyInstitution>  m_list;
};
}

#endif
