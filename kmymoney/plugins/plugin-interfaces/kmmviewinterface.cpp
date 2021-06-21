/*
    SPDX-FileCopyrightText: 2005 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmmviewinterface.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyview.h"
#include "selectedtransactions.h"

KMyMoneyPlugin::KMMViewInterface::KMMViewInterface(KMyMoneyView* view, QObject* parent, const char* name)
    : ViewInterface(parent, name)
    , m_view(view)
{
    connect(m_view, &KMyMoneyView::accountSelected, this, &ViewInterface::accountSelected);
    connect(m_view, &KMyMoneyView::transactionsSelected, this, &ViewInterface::transactionsSelected);
    connect(m_view, &KMyMoneyView::accountReconciled, this, &ViewInterface::accountReconciled);

    //  connect(app, &KMyMoneyApp::institutionSelected, this, &ViewInterface::institutionSelected);

    connect(m_view, &KMyMoneyView::viewStateChanged, this, &ViewInterface::viewStateChanged);
}

void KMyMoneyPlugin::KMMViewInterface::addView(KMyMoneyViewBase* view, const QString& name, View idView, Icons::Icon icon)
{
    m_view->addView(view, name, idView, icon);
}

void KMyMoneyPlugin::KMMViewInterface::removeView(View idView)
{
    m_view->removeView(idView);
}
