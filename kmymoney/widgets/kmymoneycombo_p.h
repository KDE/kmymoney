/*
 * Copyright 2001       Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2002-2011  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KMYMONEYCOMBO_P_H
#define KMYMONEYCOMBO_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QMutex>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyCompletion;
class KMyMoneyLineEdit;

class KMyMoneyComboPrivate
{
public:
  KMyMoneyComboPrivate() :
    m_completion(nullptr),
    m_edit(nullptr),
    m_canCreateObjects(false),
    m_inFocusOutEvent(false)
  {
  }

  virtual ~KMyMoneyComboPrivate()
  {
  }

  /**
    * This member keeps a pointer to the object's completion object
    */
  KMyMoneyCompletion*    m_completion;

  /**
    * Use our own line edit to provide hint functionality
    */
  KMyMoneyLineEdit*      m_edit;

  /**
    * The currently selected item
    */
  QString                m_id;

  QTimer                 m_timer;
  QMutex                 m_focusMutex;
  /**
    * Flag to control object creation. Use setSuppressObjectCreation()
    * to modify it's setting. Defaults to @a false.
    */
  bool                   m_canCreateObjects;

  /**
    * Flag to check whether a focusOutEvent processing is underway or not
    */
  bool                   m_inFocusOutEvent;
};

#endif
