/***************************************************************************
                             fancydategroupmarker.h
                             ----------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef FANCYDATEGROUPMARKER_H
#define FANCYDATEGROUPMARKER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "groupmarker.h"

class MyMoneyTransaction;

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
