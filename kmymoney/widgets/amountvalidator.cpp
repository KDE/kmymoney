/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "amountvalidator.h"
#include <cmath>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

AmountValidator::AmountValidator(QObject * parent) :
  AmountValidator(-HUGE_VAL, HUGE_VAL, 1000, parent)
{
}

AmountValidator::AmountValidator(double bottom, double top, int decimals,
    QObject * parent) :
    QDoubleValidator(bottom, top, decimals, parent)
{
  setNotation(StandardNotation);
}
