/*
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYEXCEPTIONTEST_H
#define MYMONEYEXCEPTIONTEST_H

#include <QObject>

class MyMoneyExceptionTest : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void init();
  void cleanup();

  void testDefaultConstructor();
  void testCatching();
};
#endif
