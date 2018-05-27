/*
 * Copyright 2009       Ian Neal <ianrsn70@users.sourceforge.net>
 * Copyright 2009       Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef IMYMONEYPROCESSINGCALENDAR_H
#define IMYMONEYPROCESSINGCALENDAR_H

// ----------------------------------------------------------------------------
// QT Includes

class QDate;

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

/**
  * @author Ian Neal
  *
  * The IMyMoneyProcessingCalendar class describes the interface to the
  * various parts that make up the processing days calendar.
  */
class IMyMoneyProcessingCalendar
{
public:
  // TODO: find out how to move this ctor and dtor out of header
  IMyMoneyProcessingCalendar() {} // krazy:exclude=inline
  virtual ~IMyMoneyProcessingCalendar() {} // krazy:exclude=inline

  /**
    * returns if a given day is used by an institution to process
    * transactions or not
    */
  virtual bool isProcessingDate(const QDate& date) const = 0;
};

#endif
