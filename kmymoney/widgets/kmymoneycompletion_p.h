/***************************************************************************
                          kmymoneycompletion_p.h  -  description
                             -------------------
    begin                : Mon Apr 26 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#ifndef KMYMONEYCOMPLETION_P_H
#define KMYMONEYCOMPLETION_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QWidget;
class QTreeWidget;
class KMyMoneySelector;

class KMyMoneyCompletionPrivate
{
  Q_DISABLE_COPY(KMyMoneyCompletionPrivate)

public:
  KMyMoneyCompletionPrivate()
  {
  }

  QWidget*                    m_parent;
  QWidget*                    m_widget;
  QString                     m_id;
  QTreeWidget*                m_lv;
  KMyMoneySelector*           m_selector;
  QRegExp                     m_lastCompletion;
  static const int MAX_ITEMS = 16;
};

#endif
