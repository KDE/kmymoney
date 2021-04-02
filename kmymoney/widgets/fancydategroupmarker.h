/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
