
/***************************************************************************
                          mymoneyinstitutiontest.h
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

#ifndef MYMONEYINSTITUTIONTEST_H
#define MYMONEYINSTITUTIONTEST_H

#include <QObject>

class MyMoneyInstitution;

class MyMoneyInstitutionTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyInstitution *m, *n;

private slots:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testSetFunctions();
  void testNonemptyConstructor();
  void testCopyConstructor();
  void testMyMoneyFileConstructor();
  void testEquality();
  void testInequality();
  void testAccountIDList();
  void testWriteXML();
  void testReadXML();
  void testElementNames();
  void testAttributeNames();
};

#endif
