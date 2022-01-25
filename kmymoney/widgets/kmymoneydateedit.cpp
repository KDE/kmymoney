/*
    SPDX-FileCopyrightText: 2016-2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneydateedit.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

KMyMoneyDateEdit::KMyMoneyDateEdit(QWidget* parent)
    : QDateEdit(parent)
{
    auto format(QDateEdit::displayFormat());
    const QRegularExpression twoYearDigits(QLatin1String("^([^y]*)yy([^y]*)$"));
    format.replace(twoYearDigits, QLatin1String("\\1yyyy\\2"));
    QDateEdit::setDisplayFormat(format);
}

KMyMoneyDateEdit::KMyMoneyDateEdit(const QDate& date, QWidget* parent)
    : KMyMoneyDateEdit(parent)
{
    setDate(date);
}

QDate KMyMoneyDateEdit::invalid_date()
{
    return QDate(1800, 1, 1);
}

void KMyMoneyDateEdit::setDisplayFormat(const QString& format)
{
    Q_UNUSED(format)
}
