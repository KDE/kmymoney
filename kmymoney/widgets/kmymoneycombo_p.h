/***************************************************************************
                          kmymoneycombo_p.h  -  description
                             -------------------
    begin                : Mon Mar 12 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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
