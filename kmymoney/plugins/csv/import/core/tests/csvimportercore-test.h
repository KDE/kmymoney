/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CSVIMPORTERCORETEST_H
#define CSVIMPORTERCORETEST_H

#include <QObject>

#include "mymoneytestutils.h"

class CSVImporterCore;
class BankingProfile;
class PricesProfile;
class InvestmentProfile;
class MyMoneyFile;

class CSVImporterCoreTest : public QObject, public MyMoneyTestBase
{
    Q_OBJECT

public:
    ~CSVImporterCoreTest() = default;

    CSVImporterCore     *csvImporter;
    BankingProfile      *debitCreditProfile;
    BankingProfile      *amountProfile;
    PricesProfile       *pricesProfile;
    InvestmentProfile   *investmentProfile;
    MyMoneyFile         *file;
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void testBasicPriceTable();
    void testPriceFractionSetting();
    void testImportByDebitCredit();
    void testImportByAmount();
    void testImportByName();
    void testImportBySymbol();
    void testFeeColumn();
    void testAutoDecimalSymbol();
    void testInvAccountAutodetection();
    void testCalculatedFeeColumn();

private:
    void setupBaseCurrency();
};
#endif
