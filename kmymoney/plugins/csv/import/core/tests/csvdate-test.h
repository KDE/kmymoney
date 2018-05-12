/*
 * Copyright 2010-2012  Allan Anderson <agander93@gmail.com>
 * Copyright 2017  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef CSVDATETEST_H
#define CSVDATETEST_H

#include <QObject>

class ConvertDate;

class CsvDateTest : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void init();
  void cleanup();

  /**
  * This method is used to test a series of valid and invalid dates,
  * including alpha month names, and different field separators.
  */
  void testConvertDate();
  /**
   * This test checks that Feb 30th is mapped to the last day in February
   */
  void testLastDayInFebruary();

private:
  ConvertDate* m_convert;
};
#endif
