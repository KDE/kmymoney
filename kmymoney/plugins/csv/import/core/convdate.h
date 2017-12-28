/***************************************************************************
                        convdate.h
                    -------------------
begin                : Sat Jan 01 2010
copyright            : (C) 2010 by Allan Anderson
email                : agander93@gmail.com
copyright            : (C) 2017 by Łukasz Wojniłowicz
email                : lukasz.wojnilowicz@gmail.com
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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
