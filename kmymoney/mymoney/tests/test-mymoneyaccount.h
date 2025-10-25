/*
    SPDX-FileCopyrightText: 2002-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYACCOUNTTEST_H
#define MYMONEYACCOUNTTEST_H

#include <QObject>

#include "mymoneytestutils.h"

class MyMoneyAccount;

class MyMoneyAccountTest : public QObject, public MyMoneyTestBase
{
    Q_OBJECT

protected:
    MyMoneyAccount* m;

private Q_SLOTS:
    void init();
    void cleanup();
    void testEmptyConstructor();
    void testConstructor();
    void testSetFunctions();
    void testCopyConstructor();
    void testAssignmentConstructor();
    void testSubAccounts();
    void testEquality();
    void testHasReferenceTo();
    void testAdjustBalance();
    void testSetClosed();
    void specialAccountTypes();
    void specialAccountTypes_data();
    void addReconciliation();
    void reconciliationHistory();
    void testHasOnlineMapping();
    void testBudgetOptions();
    void testBalanceFactor();
    void testBalanceFactor_data();
    void testPayeeCreation();
    void testPayeeCreation_data();
    void testPriceMode();
    void testPriceMode_data();
    void testAccountTypeToStdAccId();
    void testAccountTypeToStdAccId_data();
};

#endif
