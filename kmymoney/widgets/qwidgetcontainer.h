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

#ifndef QWIDGETCONTAINER_H
#define QWIDGETCONTAINER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QString;
class QWidget;

namespace KMyMoneyRegister
{
  struct QWidgetContainer : public QMap<QString, QWidget*>
  {
    Q_DISABLE_COPY(QWidgetContainer)
    QWidgetContainer();
    QWidget* haveWidget(const QString& name) const;
    void removeOrphans();
  };
} // namespace

#endif
