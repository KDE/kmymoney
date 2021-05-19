/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYRECONCILIATIONREPORT_H
#define MYMONEYRECONCILIATIONREPORT_H
// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QString>
#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

struct MyMoneyReconciliationReport {
    QString accountId;
    QStringList journalEntryIds;
    QDate statementDate;
    MyMoneyMoney startingBalance;
    MyMoneyMoney endingBalance;
};

Q_DECLARE_METATYPE(MyMoneyReconciliationReport)

#endif // MYMONEYRECONCILIATIONREPORT_H
