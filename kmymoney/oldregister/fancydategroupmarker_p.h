/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FANCYDATEGROUPMARKER_P_H
#define FANCYDATEGROUPMARKER_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "groupmarker_p.h"

namespace KMyMoneyRegister
{
  class FancyDateGroupMarkerPrivate : public GroupMarkerPrivate
  {
  public:
    QDate m_date;
  };
}

#endif
