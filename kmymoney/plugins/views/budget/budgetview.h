/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BUDGETVIEW_H
#define BUDGETVIEW_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class KBudgetView;

class BudgetView : public KMyMoneyPlugin::Plugin
{
    Q_OBJECT

public:
    explicit BudgetView(QObject *parent, const QVariantList &args);
    ~BudgetView() override;

    void plug(KXMLGUIFactory* guiFactory) final override;
    void unplug() final override;

private:
    KBudgetView* m_view;
};

#endif
