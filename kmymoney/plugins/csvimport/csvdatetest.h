/***************************************************************************
                            csvdatetest.h
                         -------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Allan Anderson
    email                : aganderson@ukonline.co.uk
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

#include <QtCore/QObject>
///#include <QtCore/QString>

class ConvertDate;

class CsvDateTest : public QObject
{
  Q_OBJECT

public:
  CsvDateTest();

private slots:
  void init();
  void cleanup();
  void testDefaultConstructor();
  void testConstructor();

  /**
  * This method is used to test a series of valid and invalid UK/EU format dates,
  * including alpha month names, and different field separators.
  */
  void testDateConvert();

  /**
  * This method is used to test USA and ISO date formats.
  */
  void testDateConvertFormats();

private:
  ConvertDate* m_convert;
};
#endif
