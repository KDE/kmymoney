/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYSELECTOR_P_H
#define KMYMONEYSELECTOR_P_H

#include "kmymoneyselector.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeWidget>
#include <QHBoxLayout>
#include <QHeaderView>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QHBoxLayout;

class KMyMoneySelectorPrivate
{
    Q_DISABLE_COPY(KMyMoneySelectorPrivate)
    Q_DECLARE_PUBLIC(KMyMoneySelector)

public:
    explicit KMyMoneySelectorPrivate(KMyMoneySelector *qq) :
        q_ptr(qq),
        m_treeWidget(nullptr),
        m_selMode(QTreeWidget::SingleSelection),
        m_layout(nullptr)
    {
    }

    void init()
    {
        Q_Q(KMyMoneySelector);
        q->setAutoFillBackground(true);

        m_selMode = QTreeWidget::SingleSelection;

        m_treeWidget = new QTreeWidget(q);
        // don't show horizontal scroll bar
        m_treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        m_treeWidget->setSortingEnabled(false);
        m_treeWidget->setAlternatingRowColors(true);

        m_treeWidget->setAllColumnsShowFocus(true);

        m_layout = new QHBoxLayout(q);
        m_layout->setSpacing(0);
        m_layout->setContentsMargins(0, 0, 0, 0);

        m_treeWidget->header()->hide();

        m_layout->addWidget(m_treeWidget);

        // force init
        m_selMode = QTreeWidget::MultiSelection;
        q->setSelectionMode(QTreeWidget::SingleSelection);

        q->connect(m_treeWidget, &QTreeWidget::itemPressed, q, &KMyMoneySelector::slotItemPressed);
        q->connect(m_treeWidget, &QTreeWidget::itemChanged, q, &KMyMoneySelector::stateChanged);
    }

    KMyMoneySelector          *q_ptr;
    QTreeWidget*               m_treeWidget;
    QStringList                m_itemList;
    QString                    m_baseName;
    QTreeWidget::SelectionMode m_selMode;
    QHBoxLayout*               m_layout;
};

#endif
