/***************************************************************************
                          mymoneypricetest.h
                          -------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYPRICETEST_H
#define MYMONEYPRICETEST_H

#include <QObject>

#include "mymoneyprice.h"

class MyMoneyPriceTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyPrice* m;

private Q_SLOTS:
  void init();
  void cleanup();

  void testDefaultConstructor();
  void testConstructor();
  void testValidity();
  void testRate();

};
#endif
