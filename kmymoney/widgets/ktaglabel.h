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

#ifndef KTAGLABEL_H
#define KTAGLABEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * This class implements a tag label. It create a QFrame and inside it a QToolButton
  * with a 'X' Icon and a QLabel with the name of the Tag
  *
  * @author Alessandro Russo
  */
class KTagLabel : public QFrame
{
  Q_OBJECT
  Q_DISABLE_COPY(KTagLabel)

public:
  explicit KTagLabel(const QString& name, QWidget* parent = nullptr);

Q_SIGNALS:
  void clicked(bool);
};

#endif
