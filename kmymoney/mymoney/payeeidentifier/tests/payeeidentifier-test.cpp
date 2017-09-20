/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2016 Christian Dávid <christian-david@web.de>
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

#include "payeeidentifier-test.h"

#include <QtTest>

#include "mymoney/payeeidentifier/payeeidentifier.h"
#include "mymoney/payeeidentifier/payeeidentifiertyped.h"
#include "mymoney/payeeidentifier/payeeidentifierloader.h"

#include "payeeidentifier/ibanandbic/ibanbic.h"

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
    const payeeIdentifier ident = payeeIdentifierLoader::instance()->createPayeeIdentifier(payeeIdentifiers::ibanBic::staticPayeeIdentifierIid());
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
    payeeIdentifier ident = payeeIdentifierLoader::instance()->createPayeeIdentifier(payeeIdentifiers::ibanBic::staticPayeeIdentifierIid());
    payeeIdentifier ident2 = ident;
  } catch (...) {
    QFAIL("Unexpected exception");
  }
}

void payeeidentifier_test::createTypedIdent()
{
  try {
    payeeIdentifier ident = payeeIdentifierLoader::instance()->createPayeeIdentifier(payeeIdentifiers::ibanBic::staticPayeeIdentifierIid());
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> typedIdent{ident};
  } catch (...) {
    QFAIL("Unexpected exception");
  }
}


