/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMM_MENUACTIONEXCHANGER
#define KMM_MENUACTIONEXCHANGER

#include <kmm_menuactionexchanger_export.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QMenu>
#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMM_MENUACTIONEXCHANGER_EXPORT KMenuActionExchanger : public QObject
{
    Q_OBJECT

public:
    explicit KMenuActionExchanger(QObject* parent);
    ~KMenuActionExchanger();

    bool eventFilter(QObject* o, QEvent* e) override;

    void addExchange(QMenu* menu, Qt::Key key, QAction* actionReleased, QAction* actionPressed);

protected Q_SLOTS:
    void menuDestroyed(QObject* menu);

private:
    class Private;
    Private* d;
};

#endif // KMM_MENUACTIONEXCHANGER
