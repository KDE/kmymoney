/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2013 Christian Dávid <christian-david@web.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtTest/QTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class germanOnlineTransferTest;

#include "germanonlinetransfertest.h"
#include "../tasks/germanonlinetransferimpl.h"
#include "../converter/taskconvertersepatogerman.h"
#include "../converter/taskconvertergermantosepa.h"
#include "onlinetasks/sepa/tasks/sepaonlinetransferimpl.h"


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

class germanOnlineTransferSettings : public germanOnlineTransfer::settings
{
public:
  // Limits getter
  virtual int purposeMaxLines() const { return 1; }
  virtual int purposeLineLength() const { return 27; }
  virtual int purposeMinLength() const { return 1; }
  
  virtual int recipientNameLineLength() const { return 27; }
  virtual int recipientNameMinLength() const { return 1; }
  
  virtual int payeeNameLineLength() const { return 27; }
  virtual int payeeNameMinLength() const { return 1; }
  
  virtual QString allowedChars() const { return QString("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvw "); }
  
  // Limits validators
  virtual bool checkPurposeCharset( const QString& ) const { return true; }
  virtual bool checkPurposeLineLength(const QString& ) const { return true; }
  virtual validators::lengthStatus checkPurposeLength(const QString&) const { return validators::ok; }
  virtual bool checkPurposeMaxLines(const QString&) const { return true; }
  
  virtual validators::lengthStatus checkNameLength(const QString&) const { return validators::ok; }
  virtual bool checkNameCharset( const QString& ) const { return true; }
  
  virtual validators::lengthStatus checkRecipientLength(const QString&) const { return validators::ok; }
  virtual bool checkRecipientCharset( const QString& ) const { return true; }
  
  virtual validators::lengthStatus checkRecipientAccountNumber( const QString& ) const { return validators::ok; }
  virtual validators::lengthStatus checkRecipientBankCode( const QString& ) const { return validators::ok; }
};

void germanOnlineTransferTest::arbitraryValidTask()
{
  germanOnlineTransferImpl* taskImpl = new germanOnlineTransferImpl;
  QSharedPointer<germanOnlineTransfer::settings> settings( new germanOnlineTransferSettings );  
  
  taskImpl->_settings = settings;
  germanOnlineTransfer* task = taskImpl;
  
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

void germanOnlineTransferTest::convertFromSepa()
{
  sepaOnlineTransfer* sepa = new sepaOnlineTransferImpl;
  taskConverterSepaToGerman* converter = new taskConverterSepaToGerman;
  
  QCOMPARE(converter->convertedTask(), germanOnlineTransfer::name());
  
  delete converter;
  delete sepa;
}

void germanOnlineTransferTest::convertToSepa()
{
  germanOnlineTransfer* original = new germanOnlineTransferImpl;
  taskConverterGermanToSepa* converter = new taskConverterGermanToSepa;
  
  QCOMPARE(converter->convertedTask(), sepaOnlineTransfer::name());
  
  delete converter;
  delete original;
}
