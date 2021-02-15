/*
    SPDX-FileCopyrightText: 2009-2014 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2016 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYPAYEETEST_H
#define MYMONEYPAYEETEST_H

#include <QObject>

class MyMoneyPayeeTest : public QObject
{
  Q_OBJECT
private Q_SLOTS:
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
};

#endif
