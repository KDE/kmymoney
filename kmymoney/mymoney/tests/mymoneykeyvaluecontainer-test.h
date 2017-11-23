/***************************************************************************
                          mymoneykeyvaluecontainertest.h
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

#ifndef MYMONEYKEYVALUECONTAINERTEST_H
#define MYMONEYKEYVALUECONTAINERTEST_H

#include <QObject>

class MyMoneyKeyValueContainer;

class MyMoneyKeyValueContainerTest : public QObject
{
  Q_OBJECT
protected:
  MyMoneyKeyValueContainer *m;

private slots:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testRetrieveValue();
  void testSetValue();
  void testDeletePair();
  void testClear();
  void testRetrieveList();
  void testLoadList();
  void testArrayRead();
  void testArrayWrite();
  void testWriteXML();
  void testReadXML();
  void testElementNames();
  void testAttributeNames();
};

#endif
