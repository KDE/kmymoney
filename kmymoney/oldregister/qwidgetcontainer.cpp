/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
      QWidget* const w = *it_w;
      it_w = erase(it_w);
      if (w)
        w->deleteLater();
    }
  }
}
