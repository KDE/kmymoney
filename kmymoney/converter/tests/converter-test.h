/***************************************************************************
                          convertertest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           Ace Jones <ace.jones@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONVERTERTEST_H
#define CONVERTERTEST_H

#include <QObject>

#include "mymoneyfile.h"
#include "storage/mymoneyseqaccessmgr.h"

class ConverterTest : public QObject
{
  Q_OBJECT

private:
  MyMoneySeqAccessMgr* storage;
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
