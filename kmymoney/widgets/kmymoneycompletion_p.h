/*
 * Copyright 2004-2011  Thomas Baumgart <tbaumgart@kde.org>
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
  KMyMoneyCompletionPrivate() :
    m_parent(nullptr),
    m_widget(nullptr),
    m_lv(nullptr),
    m_selector(nullptr)
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
