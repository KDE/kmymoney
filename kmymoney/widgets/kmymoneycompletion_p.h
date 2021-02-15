/*
 * SPDX-FileCopyrightText: 2004-2011 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
