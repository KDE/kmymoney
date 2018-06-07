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

#include <config-kmymoney.h>

#include "fancydategroupmarker.h"
#include "fancydategroupmarker_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes
#include "groupmarker.h"

FancyDateGroupMarker::FancyDateGroupMarker(Register* parent,
                                           const QDate& date,
                                           const QString& txt) :
    GroupMarker(*new FancyDateGroupMarkerPrivate, parent, txt)
{
  Q_D(FancyDateGroupMarker);
  d->m_date = date;
}

FancyDateGroupMarker::FancyDateGroupMarker(FancyDateGroupMarkerPrivate &dd, Register *parent, const QDate& date, const QString& txt) :
  GroupMarker(dd, parent, txt)
{
  Q_D(FancyDateGroupMarker);
  d->m_date = date;
}

FancyDateGroupMarker::~FancyDateGroupMarker()
{
}

QDate FancyDateGroupMarker::sortPostDate() const
{
  Q_D(const FancyDateGroupMarker);
  return d->m_date;
}

QDate FancyDateGroupMarker::sortEntryDate() const
{
  Q_D(const FancyDateGroupMarker);
  return d->m_date;
}

const char* FancyDateGroupMarker::className()
{
  return "FancyDateGroupMarker";
}
