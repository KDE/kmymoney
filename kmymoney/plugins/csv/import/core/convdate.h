/*
    SPDX-FileCopyrightText: 2010-2012 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONVDATE_H
#define CONVDATE_H

#include <QDate>
#include "csvenums.h"

#include "csv/import/core/kmm_csvimportercore_export.h"

class KMM_CSVIMPORTERCORE_EXPORT ConvertDate
{

public:
    ConvertDate();
    ~ConvertDate();

    /**
    * This method is used to convert a QString date into QDate() format.
    * If the  date is invalid, QDate() is returned.
    */
    QDate convertDate(const QString& txt);

    /**
    * This method converts the selected date setting into
    * a QString date format string.
    */
    QString          stringFormat();

    void             setDateFormatIndex(const DateFormat _d);

private:
    DateFormat       m_dateFormatIndex;
}
;
#endif
