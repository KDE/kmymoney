/*
    SPDX-FileCopyrightText: 2009 Ian Neal <ianrsn70@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
