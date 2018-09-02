/*
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CSVIMPORTERCORETEST_H
#define CSVIMPORTERCORETEST_H

#include <QObject>

class CSVImporterCore;
class BankingProfile;
class PricesProfile;
class InvestmentProfile;
class MyMoneyFile;
class MyMoneyStorageMgr;

class CSVImporterCoreTest : public QObject
{
  Q_OBJECT

  CSVImporterCore     *csvImporter;
  BankingProfile      *debitCreditProfile;
  BankingProfile      *amountProfile;
  PricesProfile       *pricesProfile;
  InvestmentProfile   *investmentProfile;
  MyMoneyFile         *file;
  MyMoneyStorageMgr *storage;
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
};
#endif
