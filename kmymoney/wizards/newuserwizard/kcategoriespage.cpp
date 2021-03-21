/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcategoriespage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_accounts.h"

#include "kaccounttemplateselector.h"
#include "knewuserwizard.h"
#include "knewuserwizard_p.h"
#include "kpreferencepage.h"
#include "wizardpage_p.h"
#include "mymoneytemplate.h"
#include "templatesmodel.h"
#include "templateloader.h"

namespace NewUserWizard
{
class CategoriesPagePrivate : public WizardPagePrivate<Wizard>
{
    Q_DISABLE_COPY(CategoriesPagePrivate)

public:
    CategoriesPagePrivate(QObject* parent) :
        WizardPagePrivate<Wizard>(parent)
    {
    }
    TemplatesModel        model;
    TemplateLoader        loader;
};

CategoriesPage::CategoriesPage(Wizard* wizard) :
    Accounts(wizard),
    WizardPage<Wizard>(*new CategoriesPagePrivate(wizard), stepCount++, this, wizard)
{
    Q_D(CategoriesPage);
    d->loader.load(&d->model);
    ui->m_templateSelector->setModel(&d->model);

    connect(&d->loader, &TemplateLoader::loadingFinished, ui->m_templateSelector, &KAccountTemplateSelector::setupInitialSelection);
}

CategoriesPage::~CategoriesPage()
{
}

KMyMoneyWizardPage* CategoriesPage::nextPage() const
{
    Q_D(const CategoriesPage);
    return d->m_wizard->d_func()->m_preferencePage;
}

QList<MyMoneyTemplate> CategoriesPage::selectedTemplates() const
{
    return ui->m_templateSelector->selectedTemplates();
}

}
