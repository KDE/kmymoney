/*
    SPDX-FileCopyrightText: 2009-2010 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2011-2020 Thomas Baumgart <tbaumgart@kde.org>
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

KTagLabel::KTagLabel(const QString& id, const QString& name, QWidget* parent)
  : QFrame(parent)
  , m_id(id)
{
  setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
  setBackgroundRole(QPalette::Button);

  QToolButton *t = new QToolButton(this);
  t->setIcon(Icons::get(Icon::DialogClose));
  t->setAutoRaise(true);

  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0, 0, 4, 0);
  layout->setSpacing(0);
  layout->addWidget(t);

  QLabel *l = new QLabel(name, this);
  layout->addWidget(l);

  setLayout(layout);
  connect(t, &QAbstractButton::clicked, this, &KTagLabel::clicked);
}
