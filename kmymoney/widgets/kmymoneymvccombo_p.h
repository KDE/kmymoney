/*
 * Copyright 2010-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2010-2016  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2010       Alvaro Soliverez <asoliverez@gmail.com>
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

#ifndef KMYMONEYMVCCOMBO_P_H
#define KMYMONEYMVCCOMBO_P_H


// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QCompleter;

class KMyMoneyMVCComboPrivate
{
public:
  KMyMoneyMVCComboPrivate() :
      m_canCreateObjects(false),
      m_inFocusOutEvent(false),
      m_completer(nullptr)
  {
  }

  /**
    * Flag to control object creation. Use
    * KMyMoneyMVCCombo::setSuppressObjectCreation()
    * to modify it's setting. Defaults to @a false.
    */
  bool                  m_canCreateObjects;

  /**
    * Flag to check whether a focusOutEvent processing is underway or not
    */
  bool                  m_inFocusOutEvent;

  QCompleter            *m_completer;
  /**
    * This is just a cache to be able to implement the old interface.
    */
  mutable QString                m_id;
};

#endif
