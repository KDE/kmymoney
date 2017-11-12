/***************************************************************************
                          kmymoneymvccombo_p.h  -  description
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
