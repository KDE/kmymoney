/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabbar.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QMouseEvent>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "widgetenums.h"

using namespace eWidgets;
using namespace KMyMoneyTransactionForm;

namespace KMyMoneyTransactionForm
{
class TabBarPrivate
{
    Q_DISABLE_COPY(TabBarPrivate)

public:
    TabBarPrivate() :
        m_signalType(eTabBar::SignalEmission::Normal)
    {
    }

    eTabBar::SignalEmission    m_signalType;

    /**
    * maps our internal action ids to those used by
    * Qt/KDE. Since it does not seem possible to tell
    * Qt/KDE to use our ids everywhere (in QAccel) we
    * need to know which is which
    */
    QMap<int, int>     m_idMap;
};
}

TabBar::TabBar(QWidget* parent) :
    QTabBar(parent),
    d_ptr(new TabBarPrivate)
{
    connect(this, &QTabBar::currentChanged, this, &TabBar::slotTabCurrentChanged);
}

TabBar::~TabBar()
{
    Q_D(TabBar);
    delete d;
}

eTabBar::SignalEmission TabBar::setSignalEmission(eTabBar::SignalEmission type)
{
    Q_D(TabBar);
    eTabBar::SignalEmission _type = d->m_signalType;
    d->m_signalType = type;
    return _type;
}

int TabBar::currentIndex() const
{
    Q_D(const TabBar);
    QMap<int, int>::const_iterator it;
    int id = QTabBar::currentIndex();
    for (it = d->m_idMap.constBegin(); it != d->m_idMap.constEnd(); ++it) {
        if (*it == id) {
            return it.key();
        }
    }
    return -1;
}

void TabBar::setCurrentIndex(int id)
{
    Q_D(TabBar);
    if (d->m_signalType != eTabBar::SignalEmission::Normal)
        blockSignals(true);

    if (d->m_idMap.contains(id)) {
        QTabBar::setCurrentIndex(d->m_idMap[id]);
    }

    if (d->m_signalType != eTabBar::SignalEmission::Normal)
        blockSignals(false);

    if (d->m_signalType == eTabBar::SignalEmission::Always)
        Q_EMIT currentChanged(d->m_idMap[id]);
}

void TabBar::setTabEnabled(int id, bool enable)
{
    Q_D(TabBar);
    if (d->m_idMap.contains(id)) {
        QTabBar::setTabEnabled(d->m_idMap[id], enable);
    }
}

void TabBar::insertTab(int id, const QString& title)
{
    Q_D(TabBar);
    int newId = QTabBar::insertTab(id, title);
    d->m_idMap[id] = newId;
}

void TabBar::insertTab(int id)
{
    insertTab(id, QString());
}

void TabBar::slotTabCurrentChanged(int id)
{
    Q_D(TabBar);
    QMap<int, int>::const_iterator it;
    for (it = d->m_idMap.constBegin(); it != d->m_idMap.constEnd(); ++it) {
        if (*it == id) {
            Q_EMIT tabCurrentChanged(it.key());
            break;
        }
    }
    if (it == d->m_idMap.constEnd())
        Q_EMIT tabCurrentChanged(id);
}

void TabBar::showEvent(QShowEvent* event)
{
    Q_D(TabBar);
    // make sure we don't emit a signal when simply showing the widget
    if (d->m_signalType != eTabBar::SignalEmission::Normal)
        blockSignals(true);

    QTabBar::showEvent(event);

    if (d->m_signalType != eTabBar::SignalEmission::Normal)
        blockSignals(false);
}

void TabBar::copyTabs(const TabBar* otabbar)
{
    Q_D(TabBar);
    // remove all existing tabs
    while (count()) {
        removeTab(0);
    }

    // now create new ones. copy text, icon and identifier
    d->m_idMap = otabbar->d_func()->m_idMap;

    for (auto i = 0; i < otabbar->count(); ++i) {
        QTabBar::insertTab(i, otabbar->tabText(i));
        if (i == otabbar->QTabBar::currentIndex()) {
            QTabBar::setCurrentIndex(i);
        }
    }
}

int TabBar::indexAtPos(const QPoint& p) const
{
    if (tabRect(QTabBar::currentIndex()).contains(p))
        return QTabBar::currentIndex();
    for (auto i = 0; i < count(); ++i)
        if (isTabEnabled(i) && tabRect(i).contains(p))
            return i;
    return -1;
}

void TabBar::mousePressEvent(QMouseEvent *e)
{
    QTabBar::mousePressEvent(e);

    // in case we receive a mouse press event on the current
    // selected tab emit a signal no matter what as the base
    // class does not do that
    if (indexAtPos(e->pos()) == QTabBar::currentIndex()) {
        slotTabCurrentChanged(QTabBar::currentIndex());
    }
}
