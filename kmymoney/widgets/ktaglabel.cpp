/*
 * Copyright 2009-2010  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2011-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
