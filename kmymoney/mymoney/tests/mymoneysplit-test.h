/***************************************************************************
                          mymoneysplittest.h
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

#ifndef MYMONEYSPLITTEST_H
#define MYMONEYSPLITTEST_H

#include <QObject>

class MyMoneySplit;

class MyMoneySplitTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneySplit *m;

private slots:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testSetFunctions();
  void testCopyConstructor();
  void testAssignmentConstructor();
  void testEquality();
  void testInequality();
  void testAmortization();
  void testValue();
  void testSetValue();
  void testSetAction();
  void testIsAutoCalc();
  void testWriteXML();
  void testReadXML();
  void testReplaceId();
  void testUnaryMinus();
  void testElementNames();
  void testAttributeNames();
};

#endif
