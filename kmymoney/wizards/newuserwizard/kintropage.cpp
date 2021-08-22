/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
