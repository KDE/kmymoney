/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qwidgetcontainer.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

using namespace KMyMoneyRegister;

QWidgetContainer::QWidgetContainer()
{
}

QWidget* QWidgetContainer::haveWidget(const QString& name) const
{
    QWidgetContainer::const_iterator it_w;
    it_w = find(name);
    if (it_w != end())
        return *it_w;
    return 0;
}

void QWidgetContainer::removeOrphans()
{
    QWidgetContainer::iterator it_w;
    for (it_w = begin(); it_w != end();) {
        if ((*it_w) && (*it_w)->parent())
            ++it_w;
        else {
            QWidget* const w = *it_w;
            it_w = erase(it_w);
            if (w)
                w->deleteLater();
        }
    }
}
