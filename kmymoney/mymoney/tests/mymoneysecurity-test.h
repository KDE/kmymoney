/***************************************************************************
                          mymoneysecuritytest.h
                          -------------------
    copyright            : (C) 2004 by Kevin Tambascio
    email                : ktambascio@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSECURITYTEST_H
#define MYMONEYSECURITYTEST_H

#include <memory>
#include <QObject>

class MyMoneySecurity;

class MyMoneySecurityTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneySecurity *m;

private slots:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testNonemptyConstructor();
  void testCopyConstructor();
  void testSetFunctions();
  void testEquality();
  void testInequality();
  // void testMyMoneyFileConstructor();
  // void testAccountIDList ();
  void testAttributeNames();
};

#endif
