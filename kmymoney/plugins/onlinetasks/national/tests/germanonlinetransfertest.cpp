/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Christian David <c.david@christian-david.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <QtTest/QTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class germanOnlineTransferTest;

#include "germanonlinetransfertest.h"
#include "../tasks/germanonlinetransfer.h"


QTEST_MAIN(germanOnlineTransferTest)

void germanOnlineTransferTest::initTestCase()
{
  // Called before the first testfunction is executed
}

void germanOnlineTransferTest::cleanupTestCase()
{
  // Called after the last testfunction was executed
}

void germanOnlineTransferTest::init()
{
  // Called before each testfunction is executed
}

void germanOnlineTransferTest::cleanup()
{
  // Called after every testfunction
}

void germanOnlineTransferTest::arbitraryValidTask()
{
  germanOnlineTransfer* task = new germanOnlineTransfer();
  QSharedPointer<germanOnlineTransfer::settings> settings( new germanOnlineTransfer::settings );  
  
  settings->setAllowedChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvw ");
  settings->setPayeeNameLimits(1, 27, 0);
  settings->setRecipientNameLimits( 1, 27, 1 );
  settings->setPurposeLimits( 1, 27, 1 );
  task->_settings = settings.staticCast<const germanOnlineTransfer::settings>();
  
  task->setValue( MyMoneyMoney(1, 1) );
  task->setPurpose( "Test" );
  
  germanAccountIdentifier ident;
  ident.setBankCode( "37020500" );
  ident.setAccountNumber( "300000" );
  ident.setOwnerName( "UNICEF Deutschland" );
  task->setRecipient( ident );
  
  QVERIFY( task->isValid() );
  
  delete task;
}

#include "../mymoney/germanonlinetransfertest.moc"
