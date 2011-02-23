/***************************************************************************
                        convDate.h
                    -------------------
begin                : Sat Jan 01 2010
copyright            : (C) 2010 by Allan Anderson
email                : aganderson@ukonline.co.uk
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

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDate>

class ConvertDate: public QObject
{
  Q_OBJECT

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

  void             setDateFormatIndex(int index);

private:
  int              m_dateFormatIndex;

private slots:

  /**
  * This method is called when the user clicks the Date button and selects
  * the date format for the input file.
  */
  void dateFormatSelected(int dateFormat);
}
;
#endif
