/***************************************************************************
                          mymoneypayeetest.h
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYPAYEETEST_H
#define MYMONEYPAYEETEST_H

#include <QtCore/QObject>
#include "autotest.h"

#define private public
#define protected public
#include "mymoneypayee.h"
#undef private
#undef protected

class MyMoneyPayeeTest : public QObject
{
  Q_OBJECT
public:
  void testXml();
  void testDefaultAccount();
};

#endif
