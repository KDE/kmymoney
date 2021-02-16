/*
    SPDX-FileCopyrightText: 2002 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Ace Jones <ace.jones@hotpop.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONVERTERTEST_H
#define CONVERTERTEST_H

#include <QObject>

#include "mymoneyfile.h"
#include "storage/mymoneystoragemgr.h"

class ConverterTest : public QObject
{
  Q_OBJECT

private:
  MyMoneyStorageMgr* storage;
  MyMoneyFile* file;

private Q_SLOTS:
  void init();
  void cleanup();
  void testWebQuotesDefault();
  void testWebQuotes_data();
  void testWebQuotes();
  void testDateFormat();
};

#endif // CONVERTERTEST_H
