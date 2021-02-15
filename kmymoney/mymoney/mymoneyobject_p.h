/*
 * SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYMONEYOBJECT_P_H
#define MYMONEYOBJECT_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyObjectPrivate
{
public:
  MyMoneyObjectPrivate()
  {
  }

  virtual ~MyMoneyObjectPrivate()
  {
  }

  void setId(const QString& id)
  {
    m_id = id;
  }

  QString m_id;
};

#endif
