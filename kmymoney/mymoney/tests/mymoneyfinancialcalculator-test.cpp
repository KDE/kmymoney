/*
    SPDX-FileCopyrightText: 2003-2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyfinancialcalculator-test.h"

#include <QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyFinancialCalculatorTest;

#include "mymoneymoney.h"
#include "mymoneyfinancialcalculator.h"
#include "mymoneyfinancialcalculator_p.h"

QTEST_GUILESS_MAIN(MyMoneyFinancialCalculatorTest)

void MyMoneyFinancialCalculatorTest::init()
{
    m = new MyMoneyFinancialCalculator;
}

void MyMoneyFinancialCalculatorTest::cleanup()
{
    delete m;
}

void MyMoneyFinancialCalculatorTest::testEmptyConstructor()
{
    QVERIFY(m->d_func()->m_ir == 0.0);
    QVERIFY(m->d_func()->m_pv == 0.0);
    QVERIFY(m->d_func()->m_pmt == 0.0);
    QVERIFY(m->d_func()->m_fv == 0.0);
    QVERIFY(m->d_func()->m_npp == 0);
    QVERIFY(m->d_func()->m_CF == 12);
    QVERIFY(m->d_func()->m_PF == 12);
    QVERIFY(m->d_func()->m_prec == 2);
    QVERIFY(m->d_func()->m_bep == false);
    QVERIFY(m->d_func()->m_disc == true);
    QVERIFY(m->d_func()->m_mask == 0);
}

void MyMoneyFinancialCalculatorTest::testSetPrec()
{
    m->setPrec(3);
    QVERIFY(m->d_func()->m_prec == 3);
}

void MyMoneyFinancialCalculatorTest::testSetNpp()
{
    m->setNpp(20);
    QVERIFY(m->d_func()->m_npp == 20);
    QVERIFY(m->d_func()->m_mask == NPP_SET);
}

void MyMoneyFinancialCalculatorTest::testSetPF()
{
    m->setPF(1);
    QVERIFY(m->d_func()->m_PF == 1);
    m->setPF();
    QVERIFY(m->d_func()->m_PF == 12);
}

void MyMoneyFinancialCalculatorTest::testSetCF()
{
    m->setCF(1);
    QVERIFY(m->d_func()->m_CF == 1);
    m->setCF();
    QVERIFY(m->d_func()->m_CF == 12);
}

void MyMoneyFinancialCalculatorTest::testSetBep()
{
    m->setBep(true);
    QVERIFY(m->d_func()->m_bep == true);
    m->setBep();
    QVERIFY(m->d_func()->m_bep == false);
}

void MyMoneyFinancialCalculatorTest::testSetDisc()
{
    m->setDisc(false);
    QVERIFY(m->d_func()->m_disc == false);
    m->setDisc();
    QVERIFY(m->d_func()->m_disc == true);
}

void MyMoneyFinancialCalculatorTest::testSetIr()
{
    m->setIr(12.3);
    QVERIFY(m->d_func()->m_ir == 12.3);
    QVERIFY(m->d_func()->m_mask == IR_SET);
}

void MyMoneyFinancialCalculatorTest::testSetPv()
{
    m->setPv(23.4);
    QVERIFY(m->d_func()->m_pv == 23.4);
    QVERIFY(m->d_func()->m_mask == PV_SET);
}

void MyMoneyFinancialCalculatorTest::testSetPmt()
{
    m->setPmt(34.5);
    QVERIFY(m->d_func()->m_pmt == 34.5);
    QVERIFY(m->d_func()->m_mask == PMT_SET);
}

void MyMoneyFinancialCalculatorTest::testSetFv()
{
    m->setFv(45.6);
    QVERIFY(m->d_func()->m_fv == 45.6);
    QVERIFY(m->d_func()->m_mask == FV_SET);
}

void MyMoneyFinancialCalculatorTest::testCombinedSet()
{
    m->setNpp(20);
    m->setIr(12.3);
    m->setPv(23.4);
    m->setPmt(34.5);
    m->setFv(45.6);

    QVERIFY(m->d_func()->m_mask == (NPP_SET | PV_SET | IR_SET | PMT_SET | FV_SET));
}

void MyMoneyFinancialCalculatorTest::testNumPayments()
{
    m->setPF(12);
    m->setCF(12);
    try {
        m->numPayments();
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
    }

    try {
        m->setPv(-80000.0);
        m->numPayments();
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
    }

    try {
        m->setIr(12.0);
        m->numPayments();
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
    }

    try {
        m->setPmt(7108.0);
        m->numPayments();
        QFAIL("Missing expected exception");
    } catch (const MyMoneyException &) {
    }

    try {
        m->setFv(0.0);
        m->numPayments();
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    QVERIFY(qRound(m->d_func()->m_npp) == 12);
}

void MyMoneyFinancialCalculatorTest::testUseCase1()
{
    m->setPv(-300000.0);
    m->setIr(5.0);
    m->setNpp(360);
    m->setFv(0.0);
    m->setPF(12);
    MyMoneyMoney res(m->payment());
    QVERIFY(res == MyMoneyMoney(161046, 100));

    res = MyMoneyMoney(m->futureValue());
    QVERIFY(res == MyMoneyMoney(405, 100));
}

void MyMoneyFinancialCalculatorTest::testUseCase2()
{
    m->setPv(-320000.0);
    m->setIr(5.0);
    m->setNpp(360);
    m->setFv(0.0);
    m->setPF(12);
    MyMoneyMoney res(m->payment());
    QVERIFY(res == MyMoneyMoney(171783, 100));

    res = MyMoneyMoney(m->futureValue());
    QVERIFY(res == MyMoneyMoney(-67, 100));
}
