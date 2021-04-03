/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kloadtemplatedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kloadtemplatedlg.h"

#include "mymoneytemplate.h"
#include "kaccounttemplateselector.h"
#include "templateloader.h"

class KLoadTemplateDlgPrivate
{
public:
    KLoadTemplateDlgPrivate()
        : ui(new Ui::KLoadTemplateDlg)
    {}

    Ui::KLoadTemplateDlg* ui;
    TemplatesModel        model;
    TemplateLoader        loader;
};

KLoadTemplateDlg::KLoadTemplateDlg(QWidget* parent) :
    QDialog(parent),
    d_ptr(new KLoadTemplateDlgPrivate)
{
    Q_D(KLoadTemplateDlg);
    d->ui->setupUi(this);
    connect(d->ui->buttonBox, &QDialogButtonBox::helpRequested, this, &KLoadTemplateDlg::slotHelp);

    d->loader.load(&d->model);
    d->ui->m_templateSelector->setModel(&d->model);

    connect(&d->loader, &TemplateLoader::loadingFinished, d->ui->m_templateSelector, &KAccountTemplateSelector::setupInitialSelection);
}

KLoadTemplateDlg::~KLoadTemplateDlg()
{
    Q_D(KLoadTemplateDlg);
    delete d;
}

QList<MyMoneyTemplate> KLoadTemplateDlg::templates() const
{
    Q_D(const KLoadTemplateDlg);
    return d->ui->m_templateSelector->selectedTemplates();
}

void KLoadTemplateDlg::slotHelp()
{
}
