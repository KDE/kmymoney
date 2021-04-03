/*
    SPDX-FileCopyrightText: 2006 Thomas Baumagrt <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneywizardpage.h"
#include "kmymoneywizardpage_p.h"
#include "kmymoneywizard_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kguiutils.h"
#include "kmymoneywizard.h"

KMyMoneyWizardPage::KMyMoneyWizardPage(uint step, QWidget* widget) :
    d_ptr(new KMyMoneyWizardPagePrivate(widget))
{
    Q_D(KMyMoneyWizardPage);
    d->m_step = step;
    d->m_widget = widget;
    d->m_mandatoryGroup = new KMandatoryFieldGroup(widget);
    QObject::connect(d->m_mandatoryGroup, static_cast<void (KMandatoryFieldGroup::*)()>(&KMandatoryFieldGroup::stateChanged), object(), &KMyMoneyWizardPagePrivate::completeStateChanged);
    widget->hide();
}

KMyMoneyWizardPage::KMyMoneyWizardPage(KMyMoneyWizardPagePrivate &dd, uint step, QWidget *widget) :
    d_ptr(&dd)
{
    Q_D(KMyMoneyWizardPage);
    d->m_step = step;
    d->m_widget = widget;
    d->m_mandatoryGroup = new KMandatoryFieldGroup(widget);
    QObject::connect(d->m_mandatoryGroup, static_cast<void (KMandatoryFieldGroup::*)()>(&KMandatoryFieldGroup::stateChanged), object(), &KMyMoneyWizardPagePrivate::completeStateChanged);
    widget->hide();
}

KMyMoneyWizardPage::~KMyMoneyWizardPage()
{
    Q_D(KMyMoneyWizardPage);
    delete d;
}

const KMyMoneyWizardPagePrivate* KMyMoneyWizardPage::object() const
{
    Q_D(const KMyMoneyWizardPage);
    return d;
}

QWidget* KMyMoneyWizardPage::initialFocusWidget() const
{
    return nullptr;
}

void KMyMoneyWizardPage::completeStateChanged()
{
    Q_D(KMyMoneyWizardPage);
    d->emitCompleteStateChanged();
}

void KMyMoneyWizardPage::resetPage()
{
}

void KMyMoneyWizardPage::enterPage()
{
}

void KMyMoneyWizardPage::leavePage()
{
}

KMyMoneyWizardPage* KMyMoneyWizardPage::nextPage() const
{
    return 0;
}

bool KMyMoneyWizardPage::isLastPage() const
{
    return nextPage() == 0;
}

bool KMyMoneyWizardPage::isComplete() const
{
    Q_D(const KMyMoneyWizardPage);
    if (!isLastPage())
        wizard()->d_func()->m_nextButton->setToolTip(i18n("Continue with next page"));
    else
        wizard()->d_func()->m_finishButton->setToolTip(i18n("Finish wizard"));
    return d->m_mandatoryGroup->isEnabled();
}

unsigned int KMyMoneyWizardPage::step() const
{
    Q_D(const KMyMoneyWizardPage);
    return d->m_step;
}

QWidget* KMyMoneyWizardPage::widget() const
{
    Q_D(const KMyMoneyWizardPage);
    return d->m_widget;
}

QString KMyMoneyWizardPage::helpContext() const
{
    return QString();
}
