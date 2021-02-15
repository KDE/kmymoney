/*
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CSVIMPORTTESTCOMMON_H
#define CSVIMPORTTESTCOMMON_H

#include "mymoneyenums.h"

class QString;
class QDate;
extern void writeStatementToCSV(const QString& content, const QString& filename);
extern QString csvDataset(const int set);
extern QString makeAccount(const QString& name, const QString& number, eMyMoney::Account::Type type, const QDate& open, const QString& parent);
#endif
