/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyviewbase.h"
#include "kmymoneyviewbase_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QVariantList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPageWidgetItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"
#include "mymoneyenums.h"

KMyMoneyViewBase::KMyMoneyViewBase(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new KMyMoneyViewBasePrivate(this))
{
}

KMyMoneyViewBase::KMyMoneyViewBase(KMyMoneyViewBasePrivate &dd, QWidget *parent)
    : QWidget(parent)
    , d_ptr(&dd)
{
    // make sure we keep a copy of what we send out
    connect(this, &KMyMoneyViewBase::requestSelectionChange, this, [&](const SelectedObjects& selections) {
        Q_D(KMyMoneyViewBase);
        d->m_selections = selections;
    });
}

KMyMoneyViewBase::~KMyMoneyViewBase()
{
}

void KMyMoneyViewBase::aboutToShow()
{
    Q_D(KMyMoneyViewBase);

    // tell everyone what is selected here
    emit requestSelectionChange(d->m_selections);
}

void KMyMoneyViewBase::aboutToHide()
{
}

void KMyMoneyViewBase::changeEvent(QEvent* ev)
{
    QWidget::changeEvent(ev);

    if(ev->type() == QEvent::EnabledChange) {
        emit viewStateChanged(isEnabled());
    }
}

void KMyMoneyViewBase::executeAction(eMenu::Action action, const QVariantList& args)
{
    Q_UNUSED(action)
    Q_UNUSED(args)
}
