/***************************************************************************
                        csvimporttestcommon.h
                     -------------------
copyright            : (C) 2017 by Łukasz Wojniłowicz
email                : lukasz.wojnilowicz@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef CSVIMPORTTESTCOMMON_H
#define CSVIMPORTTESTCOMMON_H

#include "mymoneyaccount.h"

class QString;
extern void writeStatementToCSV(const QString& content, const QString& filename);
extern QString csvDataset(const int set);
extern QString makeAccount(const QString& name, const QString& number, MyMoneyAccount::accountTypeE type, const QDate& open, const QString& parent);
#endif
