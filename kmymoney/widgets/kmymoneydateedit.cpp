/*
    SPDX-FileCopyrightText: 2016-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneydateedit.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

namespace
{
const QDate INVALID_DATE = QDate(1800, 1, 1);
}


KMyMoneyDateEdit::KMyMoneyDateEdit(QWidget* parent)
  : QDateEdit(parent)
{
}
