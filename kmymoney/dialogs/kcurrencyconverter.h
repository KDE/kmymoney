/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCURRENCYCONVERTER_H
#define KCURRENCYCONVERTER_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QDate;
class MultiCurrencyEdit;
class MyMoneyMoney;

namespace Ui {
class KCurrencyConverter;
}

typedef qint64 signed64;

/**
 * @author Thomas Baumgart
 */

class KCurrencyConverterPrivate;
class KMM_BASE_DIALOGS_EXPORT KCurrencyConverter
{
    // Q_DISABLE_COPY(KCurrencyConverter)

public:
    KCurrencyConverter();
    ~KCurrencyConverter();

    MyMoneyMoney updateRate(MultiCurrencyEdit* amountEdit, const QDate& date);

private:
    KCurrencyConverterPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(KCurrencyConverter)
};

#endif // KCURRENCYCONVERTER_H
