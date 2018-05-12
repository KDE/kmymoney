/*
 * Copyright 2010-2012  Allan Anderson <agander93@gmail.com>
 * Copyright 2016-2017  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
