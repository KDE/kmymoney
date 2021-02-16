/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
