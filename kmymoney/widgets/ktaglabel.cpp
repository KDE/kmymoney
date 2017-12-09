/***************************************************************************
                          ktaglabel.cpp  -  description
                             -------------------
    begin                : Sat Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Cristian Onet <cristian.onet@gmail.com>
                           Alvaro Soliverez <asoliverez@gmail.com>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
