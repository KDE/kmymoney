/*
    SPDX-FileCopyrightText: 2009-2010 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2011-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktaglabel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"

using namespace Icons;

KTagLabel::KTagLabel(const QString& id, const QString& name, QWidget* parent) :
    QFrame(parent)
{
    QToolButton *t = new QToolButton(this);
    t->setIcon(Icons::get(Icon::DialogClose));
    t->setAutoRaise(true);
    QLabel *l = new QLabel(name, this);
    m_tagId = id;
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    this->setLayout(layout);
    layout->addWidget(t);
    layout->addWidget(l);
    connect(t, &QAbstractButton::clicked, this, &KTagLabel::clicked);
    //this->setFrameStyle(QFrame::Panel | QFrame::Plain);
}
