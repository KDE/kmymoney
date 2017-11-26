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

#include <QObject>

class MyMoneyPayeeTest : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  void testXml();
  void testDefaultAccount();
  void testEmptyMatchKeyBegin();
  void testEmptyMatchKeyEnd();
  void testEmptyMatchKeyMiddle();
  void testEmptyMatchKeyMix();
  void testMatchKeyDisallowSingleSpace();
  void testMatchKeyDisallowMultipleSpace();
  void testMatchKeyAllowSpaceAtStart();
  void testMatchKeyAllowSpaceAtEnd();
  void testMatchNameExact();
  void testElementNames();
  void testAttributeNames();
};

#endif
