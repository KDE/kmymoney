/***************************************************************************
                          convertertest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           Ace Jones <ace.jones@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef CONVERTERTEST_H
#define CONVERTERTEST_H

#include <QObject>

#include "mymoneyfile.h"

class ConverterTest : public QObject
{
  Q_OBJECT

private:
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
