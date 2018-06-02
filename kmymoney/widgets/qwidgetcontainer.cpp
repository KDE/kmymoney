/***************************************************************************
                             qwidgetcontainer.cpp  -  description
                             -------------------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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
      remove(it_w.key());
      delete(*it_w);
      it_w = begin();
    }
  }
}
