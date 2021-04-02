/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KINTROPAGE_H
#define KINTROPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class KMyMoneyWizardPage;

namespace NewUserWizard
{
class Wizard;

class IntroPagePrivate;
class IntroPage : public QWidget, public WizardPage<Wizard>
{
    Q_OBJECT
    Q_DISABLE_COPY(IntroPage)

public:
    explicit IntroPage(Wizard* parent);
    ~IntroPage() override;

    KMyMoneyWizardPage* nextPage() const override;

private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, IntroPage)
    friend class Wizard;
};
} // namespace

#endif
