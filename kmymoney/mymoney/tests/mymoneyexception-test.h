/***************************************************************************
                          mymoneyexceptiontest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
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

#ifndef MYMONEYEXCEPTIONTEST_H
#define MYMONEYEXCEPTIONTEST_H

#include <QObject>

#include "mymoneyexception.h"

class MyMoneyExceptionTest : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void init();
  void cleanup();

  void testDefaultConstructor();
};
#endif
