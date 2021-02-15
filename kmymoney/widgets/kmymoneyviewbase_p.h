/*
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#ifndef KMYMONEYVIEWBASEPRIVATE_H
#define KMYMONEYVIEWBASEPRIVATE_H

class KMyMoneyViewBasePrivate
{
public:
  virtual ~KMyMoneyViewBasePrivate(){}

  bool m_needsRefresh;
};

#endif
