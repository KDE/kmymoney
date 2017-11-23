/***************************************************************************
                        csvdatetest.h
                     -------------------
begin                : Sat Jan 01 2010
copyright            : (C) 2010 by Allan Anderson
email                : agander93@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef CSVDATETEST_H
#define CSVDATETEST_H

#include <QObject>

class ConvertDate;

class CsvDateTest : public QObject
{
  Q_OBJECT

private slots:
  void init();
  void cleanup();

  /**
  * This method is used to test a series of valid and invalid dates,
  * including alpha month names, and different field separators.
  */
  void testConvertDate();

private:
  ConvertDate* m_convert;
};
#endif
