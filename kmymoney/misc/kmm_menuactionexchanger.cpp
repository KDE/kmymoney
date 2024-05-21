/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmm_menuactionexchanger.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

struct ExchangeAction {
    QMenu* menu;
    Qt::Key key;
    QAction* actionReleased;
    QAction* actionPressed;

    void toggle(bool keyPressed) const
    {
        QAction* activateAction = nullptr;
        if (keyPressed && (menu->activeAction() == actionReleased)) {
            activateAction = actionPressed;
        } else if (!keyPressed && (menu->activeAction() == actionPressed)) {
            activateAction = actionReleased;
        }

        actionReleased->setVisible(!keyPressed);
        actionReleased->setEnabled(!keyPressed);
        actionPressed->setVisible(keyPressed);
        actionPressed->setEnabled(keyPressed);

        if (activateAction) {
            menu->setActiveAction(activateAction);
        }
    }
};

class KMenuActionExchanger::Private
{
public:
    Private(KMenuActionExchanger* qq)
        : q(qq)
    {
    }

    KMenuActionExchanger* q;
    QList<ExchangeAction> m_actions;
};

KMenuActionExchanger::KMenuActionExchanger(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
}

KMenuActionExchanger::~KMenuActionExchanger()
{
    delete d;
}

void KMenuActionExchanger::addExchange(QMenu* menu, Qt::Key key, QAction* actionReleased, QAction* actionPressed)
{
    ExchangeAction newAction;
    newAction.menu = menu;
    newAction.key = key;
    newAction.actionReleased = actionReleased;
    newAction.actionPressed = actionPressed;

    QSet<QMenu*> installedMenus;
    for (int i = 0; i < d->m_actions.count(); ++i) {
        const auto& action = d->m_actions.at(i);
        if ((action.menu == menu) && (action.key == key)) {
            d->m_actions.removeAt(i);
            --i; // make sure to check this position again
        } else {
            installedMenus.insert(action.menu);
        }
    }

    d->m_actions.append(newAction);
    actionReleased->setVisible(true);
    actionPressed->setVisible(false);

    if (!installedMenus.contains(menu)) {
        menu->installEventFilter(this);
        connect(menu, &QObject::destroyed, this, &KMenuActionExchanger::menuDestroyed);
    }
}

bool KMenuActionExchanger::eventFilter(QObject* o, QEvent* e)
{
    if ((e->type() == QEvent::KeyPress) || (e->type() == QEvent::KeyRelease)) {
        for (int i = 0; i < d->m_actions.count(); ++i) {
            const auto& action = d->m_actions.at(i);
            if (action.menu == o) {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
                if (action.key == keyEvent->key()) {
                    action.toggle(e->type() == QEvent::KeyPress);
                    break;
                }
            }
        }

    } else if ((e->type() == QEvent::Show) || (e->type() == QEvent::Hide)) {
        for (int i = 0; i < d->m_actions.count(); ++i) {
            const auto& action = d->m_actions.at(i);
            if (action.menu == o) {
                action.toggle(false);
            }
        }
    }
    return QObject::eventFilter(o, e);
}

void KMenuActionExchanger::menuDestroyed(QObject* menu)
{
    for (int i = 0; i < d->m_actions.count(); ++i) {
        const auto& action = d->m_actions.at(i);
        if (action.menu == menu) {
            d->m_actions.removeAt(i);
            --i; // make sure to check this position again
        }
    }
}
