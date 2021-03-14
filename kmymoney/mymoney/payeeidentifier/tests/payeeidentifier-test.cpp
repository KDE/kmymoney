/*
    SPDX-FileCopyrightText: 2013-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "payeeidentifier-test.h"

#include <QTest>

#include "mymoney/payeeidentifier/payeeidentifier.h"
#include "mymoney/payeeidentifier/payeeidentifiertyped.h"

#include "payeeidentifier/ibanbic/ibanbic.h"

QTEST_GUILESS_MAIN(payeeidentifier_test);

void payeeidentifier_test::initTestCase()
{
    // Called before the first testfunction is executed
}

void payeeidentifier_test::cleanupTestCase()
{
    // Called after the last testfunction was executed
}

void payeeidentifier_test::init()
{
    // Called before each testfunction is executed
}

void payeeidentifier_test::cleanup()
{
    // Called after every testfunction
}

void payeeidentifier_test::createAndDeleteEmptyIdent()
{
  payeeIdentifier ident{};
}

void payeeidentifier_test::copyIdent()
{
  try {
    const payeeIdentifier ident = payeeIdentifier(new payeeIdentifiers::ibanBic());
    payeeIdentifier ident2 = ident;
    QVERIFY(!ident2.isNull());
    QCOMPARE(ident2.iid(), payeeIdentifiers::ibanBic::staticPayeeIdentifierIid());
  } catch (...) {
    QFAIL("Unexpected exception");
  }
}

void payeeidentifier_test::moveIdent()
{
  try {
    payeeIdentifier ident = payeeIdentifier(new payeeIdentifiers::ibanBic());
    payeeIdentifier ident2 = ident;
  } catch (...) {
    QFAIL("Unexpected exception");
  }
}

void payeeidentifier_test::createTypedIdent()
{
  try {
    payeeIdentifier ident = payeeIdentifier(new payeeIdentifiers::ibanBic());
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> typedIdent{ident};
  } catch (...) {
    QFAIL("Unexpected exception");
  }
}


