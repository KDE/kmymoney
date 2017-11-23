/***************************************************************************
                        csvimporter-test.h
                     -------------------
copyright            : (C) 2017 by Łukasz Wojniłowicz
email                : lukasz.wojnilowicz@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef CSVIMPORTERTEST_H
#define CSVIMPORTERTEST_H

#include <QObject>

class CSVImporter;
class BankingProfile;
class PricesProfile;
class InvestmentProfile;
class MyMoneyFile;
class MyMoneySeqAccessMgr;

class CsvImporterTest : public QObject
{
  Q_OBJECT

  CSVImporter         *csvImporter;
  BankingProfile      *debitCreditProfile;
  BankingProfile      *amountProfile;
  PricesProfile       *pricesProfile;
  InvestmentProfile   *investmentProfile;
  MyMoneyFile         *file;
  MyMoneySeqAccessMgr *storage;
private slots:
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
};
#endif
