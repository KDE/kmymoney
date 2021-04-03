/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QWIDGETCONTAINER_H
#define QWIDGETCONTAINER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QString;
class QWidget;

namespace KMyMoneyRegister
{
struct QWidgetContainer : public QMap<QString, QWidget*>
{
    Q_DISABLE_COPY(QWidgetContainer)
    QWidgetContainer();
    QWidget* haveWidget(const QString& name) const;
    void removeOrphans();
};
} // namespace

#endif
