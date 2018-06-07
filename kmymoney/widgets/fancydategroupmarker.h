/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef FANCYDATEGROUPMARKER_H
#define FANCYDATEGROUPMARKER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "groupmarker.h"

namespace KMyMoneyRegister
{
  class FancyDateGroupMarkerPrivate;
  class FancyDateGroupMarker : public GroupMarker
  {
    Q_DISABLE_COPY(FancyDateGroupMarker)

  public:
    explicit FancyDateGroupMarker(Register* getParent, const QDate& date, const QString& txt);
    ~FancyDateGroupMarker() override;

    QDate sortPostDate() const override;
    QDate sortEntryDate() const override;
    const char* className() override;

  protected:
    FancyDateGroupMarker(FancyDateGroupMarkerPrivate &dd, Register *parent, const QDate& date, const QString& txt);
     Q_DECLARE_PRIVATE(FancyDateGroupMarker)
  };
} // namespace

#endif
