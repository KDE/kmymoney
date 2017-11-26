/***************************************************************************
                          mymoneytagtest.h
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYTAGTEST_H
#define MYMONEYTAGTEST_H

#include <QObject>

class MyMoneyTagTest : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void testXml();
  void testAttributeNames();
};

#endif
