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

#include "csvimportercore-test.h"

#include <QtTest>

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyexception.h"

#include "csvimportercore.h"
#include "csvimporttestcommon.h"

QTEST_GUILESS_MAIN(CSVImporterCoreTest)

void CSVImporterCoreTest::initTestCase()
{
  // setup the MyMoneyMoney locale settings according to the KDE settings
  MyMoneyMoney::setThousandSeparator(QLocale().groupSeparator());
  MyMoneyMoney::setDecimalSeparator(QLocale().decimalPoint());
}

void CSVImporterCoreTest::setupBaseCurrency()
{
  file = MyMoneyFile::instance();

  MyMoneySecurity base("EUR", "Euro", QChar(0x20ac));
  MyMoneyFileTransaction ft;
  try {
    file->currency(base.id());
  } catch (const MyMoneyException &e) {
    file->addCurrency(base);
  }
  file->setBaseCurrency(base);
  ft.commit();
}

void CSVImporterCoreTest::init()
{
  setupBaseCurrency();
  file = MyMoneyFile::instance();

  csvImporter = new CSVImporterCore;

  csvImporter->m_mapSymbolName.insert("STK1", "Stock 1");
  csvImporter->m_mapSymbolName.insert("STK2", "Stock 2");
  csvImporter->m_mapSymbolName.insert("STK3", "Stock 3");

  investmentProfile = new InvestmentProfile ("investment",
                                             106, 1, 0, DateFormat::YearMonthDay, FieldDelimiter::Semicolon,
                                             TextDelimiter::DoubleQuote, DecimalSymbol::Dot,
                                             QMap<Column, int> {{Column::Date, 0}, {Column::Name, 1}, {Column::Type, 2}, {Column::Quantity, 3}, {Column::Price, 4}, {Column::Amount, 5}},
                                             2,
                                             QMap <eMyMoney::Transaction::Action, QStringList>{
                                               {eMyMoney::Transaction::Action::Buy, QStringList {"buy"}},
                                               {eMyMoney::Transaction::Action::Sell, QStringList {"sell"}}}
                                             );

  pricesProfile = new PricesProfile ("price source",
                                     106, 1, 0, DateFormat::YearMonthDay, FieldDelimiter::Comma,
                                     TextDelimiter::DoubleQuote, DecimalSymbol::Dot,
                                     QMap<Column, int>{{Column::Date, 0}, {Column::Price, 4}},
                                     2, Profile::StockPrices);

  amountProfile = new BankingProfile ("amount",
                                      106, 1, 0, DateFormat::MonthDayYear, FieldDelimiter::Comma,
                                      TextDelimiter::DoubleQuote, DecimalSymbol::Dot,
                                      QMap<Column, int>{{Column::Date, 1}, {Column::Memo, 2}, {Column::Amount, 3}, {Column::Category, 4}},
                                      false);

  debitCreditProfile = new BankingProfile ("debit credit",
                                           106, 1, 0, DateFormat::MonthDayYear, FieldDelimiter::Comma,
                                           TextDelimiter::DoubleQuote, DecimalSymbol::Dot,
                                           QMap<Column, int>{{Column::Date, 1}, {Column::Memo, 2}, {Column::Debit, 3}, {Column::Credit, 4}, {Column::Category, 5}},
                                           false);


}

void CSVImporterCoreTest::cleanup()
{
  delete investmentProfile;
  delete pricesProfile;
  delete amountProfile;
  delete csvImporter;
}

void CSVImporterCoreTest::testBasicPriceTable()
{
  QString csvContent;
  csvContent += QLatin1String("Date,Open,High,Low,Close,Volume\n");
  csvContent += QLatin1String("2017-08-01,1.23,2.34,3.45,4.56,2\n");
  csvContent += QLatin1String("2017-08-02,5.67,6.78,7.89,9.10,2\n");
  csvContent += QLatin1String("2017-08-03,2.23,3.34,4.45,5.56,2\n");

  QString filename("basic-price-table.csv");
  writeStatementToCSV(csvContent, filename);

  pricesProfile->m_securityName = QLatin1String("APPLE");

  auto st = csvImporter->unattendedImport(filename, pricesProfile);

  QVERIFY(st.m_eType == eMyMoney::Statement::Type::None);
  QVERIFY(st.m_listPrices.count() == 3);
  QVERIFY(st.m_listPrices[2].m_date == QDate(2017, 8, 3));
  QCOMPARE(st.m_listPrices[2].m_amount.toString(), MyMoneyMoney(5.56).toString());
  QVERIFY(st.m_listPrices[2].m_sourceName == "price source");
  QVERIFY(st.m_listPrices[2].m_strSecurity == pricesProfile->m_securityName);
}

void CSVImporterCoreTest::testPriceFractionSetting() {

  QString csvContent;
  csvContent += QLatin1String("Date;Open;High;Low;Close;Volume\n");
  csvContent += QLatin1String(";;;;;\n");
  csvContent += QLatin1String("8/1/2017;1,234;2,345;3,456;4,567;2\n");
  csvContent += QLatin1String("8/2/2017;5,679;6,789;7,891;9,101;2\n");
  csvContent += QLatin1String("8/3/2017;2,234;3,345;4,456;5,567;2\n");
  csvContent += QLatin1String("8/4/2017;3,456;4,567;5,678;6,789;2\n");

  QString filename("price-fraction-setting.csv");
  writeStatementToCSV(csvContent, filename);

  pricesProfile->m_securityName = QLatin1String("APPLE");
  pricesProfile->m_dateFormat = DateFormat::MonthDayYear;
  pricesProfile->m_fieldDelimiter = FieldDelimiter::Semicolon;
  pricesProfile->m_decimalSymbol = DecimalSymbol::Comma;
  pricesProfile->m_priceFraction = 1; // price *= 0.1
  pricesProfile->m_startLine = 2;
  pricesProfile->m_trailerLines = 1;

  auto st = csvImporter->unattendedImport(filename, pricesProfile);

  QVERIFY(st.m_listPrices.count() == 3);
  QVERIFY(st.m_listPrices[2].m_date == QDate(2017, 8, 3));
  QVERIFY(st.m_listPrices[2].m_amount == MyMoneyMoney(0.5567, 10000)); // user reported that visible price (5.567) should be treated as a fraction of real price (0.5567)
}

void CSVImporterCoreTest::testImportByDebitCredit()
{
  QString csvContent;
  csvContent += QLatin1String("\"Trans Date\",\"Post Date\",\"Description\",\"Debit\",\"Credit\",\"Category\"\n");
  csvContent += QLatin1String("05/16/2016,05/17/2016,FOO1,1.234,0.00,BAR\n"); // debit is intentionally positive here
  csvContent += QLatin1String("06/17/2016,06/18/2016,FOO2,0,90.12,BAR\n");
  csvContent += QLatin1String("07/18/2016,07/19/2016,FOO3,-910.12,,BAR\n");
  csvContent += QLatin1String("08/19/2016,08/20/2016,FOO4,,,BAR\n");
  csvContent += QLatin1String("09/20/2016,09/21/2016,FOO5,0,0,BAR\n");

  QString filename("import-by-debit-credit.csv");
  writeStatementToCSV(csvContent, filename);

  auto st = csvImporter->unattendedImport(filename, debitCreditProfile);
  QVERIFY(st.m_listTransactions.count() == 5);
  QVERIFY(st.m_listTransactions[0].m_amount == MyMoneyMoney(-1.234, 1000));
  QVERIFY(st.m_listTransactions[0].m_listSplits.count() == 1);
  QVERIFY(st.m_listTransactions[0].m_listSplits.first().m_amount == MyMoneyMoney(1.234, 1000));
  QVERIFY(st.m_listTransactions[1].m_amount == MyMoneyMoney(90.12));
  QVERIFY(st.m_listTransactions[2].m_amount == MyMoneyMoney(-910.12));
  QVERIFY(st.m_listTransactions[3].m_amount == MyMoneyMoney());
  QVERIFY(st.m_listTransactions[4].m_amount == MyMoneyMoney());
}

void CSVImporterCoreTest::testImportByAmount()
{
  QString csvContent;
  csvContent += QLatin1String("\"Trans Date\",\"Post Date\",\"Description\",\"Amount\",\"Category\"\n");
  csvContent += QLatin1String("05/16/2016,05/17/2016,FOO1,1.234,BAR\n");
  csvContent += QLatin1String("06/17/2016,06/18/2016,FOO2,56.78,BAR\n");
  csvContent += QLatin1String("07/18/2016,07/19/2016,FOO3,910.12,BAR\n");

  QString filename("import-by-amount.csv");
  writeStatementToCSV(csvContent, filename);
  auto st = csvImporter->unattendedImport(filename, amountProfile);
  QVERIFY(st.m_listTransactions.count() == 3);
  QVERIFY(st.m_listTransactions[1].m_datePosted == QDate(2016, 6, 18));
  QVERIFY(st.m_listTransactions[1].m_strMemo == "FOO2");
  QVERIFY(st.m_listTransactions[1].m_listSplits.count() == 1);
  QVERIFY(st.m_listTransactions[1].m_listSplits.first().m_strCategoryName == "BAR");
  QVERIFY(st.m_listTransactions[1].m_listSplits.first().m_amount == MyMoneyMoney(-56.78));
  QVERIFY(st.m_listTransactions[1].m_amount == MyMoneyMoney(56.78));
}

void CSVImporterCoreTest::testImportByName()
{
  auto stockNames = csvImporter->m_mapSymbolName.values();
  auto stockSymbols = csvImporter->m_mapSymbolName.keys();

  auto csvContent = csvDataset(0);

  QString filename("import-by-name.csv");
  writeStatementToCSV(csvContent, filename);

  auto st = csvImporter->unattendedImport(filename, investmentProfile);
  QVERIFY(st.m_eType == eMyMoney::Statement::Type::Investment);

  QVERIFY(st.m_listSecurities.count() == 3);
  QVERIFY(st.m_listSecurities[0].m_strName == stockNames.at(0));
  QVERIFY(st.m_listSecurities[0].m_strSymbol == stockSymbols.at(0));

  QVERIFY(st.m_listTransactions.count() == 3);
  QVERIFY(st.m_listTransactions[0].m_datePosted == QDate(2017, 8, 1));
  QVERIFY(st.m_listTransactions[0].m_amount == MyMoneyMoney(-125)); // KMM handled correctly positive amount in buy transaction
  QVERIFY(st.m_listTransactions[0].m_shares == MyMoneyMoney(100));
  QVERIFY(st.m_listTransactions[0].m_strSecurity == stockNames.at(0));
  QVERIFY(st.m_listTransactions[0].m_strSymbol == stockSymbols.at(0));
}

void CSVImporterCoreTest::testImportBySymbol()
{
  auto stockNames = csvImporter->m_mapSymbolName.values();
  auto stockSymbols = csvImporter->m_mapSymbolName.keys();

  auto csvContent = csvDataset(0);

  for (auto i = 0; i < stockNames.count(); ++i)
    csvContent.replace(stockNames.at(i), stockSymbols.at(i));
  csvContent.replace("Name", "Symbol");

  QString filename("import-by-symbol.csv");
  writeStatementToCSV(csvContent, filename);

  investmentProfile->m_colTypeNum.remove(Column::Name);
  investmentProfile->m_colNumType.remove(1);
  investmentProfile->m_colTypeNum.insert(Column::Symbol, 1);
  investmentProfile->m_colNumType.insert(1, Column::Symbol);
  auto st = csvImporter->unattendedImport(filename, investmentProfile);
  QVERIFY(st.m_listSecurities.count() == 3);
  QVERIFY(st.m_listTransactions.count() == 3);
  QVERIFY(st.m_listTransactions[0].m_strSecurity == stockNames.at(0));
  QVERIFY(st.m_listTransactions[0].m_strSymbol == stockSymbols.at(0));
}

void CSVImporterCoreTest::testFeeColumn()
{
  auto csvContent = csvDataset(0);
  QString filename("fee-column.csv");
  writeStatementToCSV(csvContent, filename);

  investmentProfile->m_colTypeNum.insert(Column::Fee, 6);
  investmentProfile->m_colNumType.insert(6, Column::Fee);
  auto st = csvImporter->unattendedImport(filename, investmentProfile);
  QVERIFY(st.m_listTransactions[0].m_amount == MyMoneyMoney(-129)); // new_amount = original_amount + fee
  QVERIFY(st.m_listTransactions[0].m_fees == MyMoneyMoney(4));      // fee taken literally
  QVERIFY(st.m_listTransactions[1].m_amount == MyMoneyMoney(450));
  QVERIFY(st.m_listTransactions[1].m_fees == MyMoneyMoney(6));

  investmentProfile->m_feeIsPercentage = true;
  st = csvImporter->unattendedImport(filename, investmentProfile);
  QVERIFY(st.m_listTransactions[0].m_amount == MyMoneyMoney(-130)); // new_amount = original_amount (1 + fee/100)
  QVERIFY(st.m_listTransactions[0].m_fees == MyMoneyMoney(5));      // new_fee = original_amount * fee/100
}

void CSVImporterCoreTest::testAutoDecimalSymbol()
{
  auto csvContent = csvDataset(0);
  csvContent += QLatin1String("2017-08-04-12.02.10;Stock 1;sell;101;1.25;126,25;4\n"); // mixed decimal symbols are on purpose here
  QString filename("auto-decimal-symbol.csv");
  writeStatementToCSV(csvContent, filename);
  investmentProfile->m_decimalSymbol = DecimalSymbol::Auto;

  auto st = csvImporter->unattendedImport(filename, investmentProfile);
  QVERIFY(st.m_listTransactions.count() == 4);
  QVERIFY(st.m_listTransactions[3].m_datePosted == QDate(2017, 8, 4));
  QVERIFY(st.m_listTransactions[3].m_amount == MyMoneyMoney(126.25));
  QVERIFY(st.m_listTransactions[3].m_shares == MyMoneyMoney(101));
}

void CSVImporterCoreTest::testInvAccountAutodetection()
{
  MyMoneyFileTransaction ft;
  makeAccount("Eas", "123", eMyMoney::Account::Type::Investment, QDate(2017, 8, 1), file->asset().id());
  makeAccount("BigInvestments", "", eMyMoney::Account::Type::Investment, QDate(2017, 8, 1), file->asset().id());
  makeAccount("BigInvestments", "1234567890", eMyMoney::Account::Type::Investment, QDate(2017, 8, 1), file->asset().id());
  makeAccount("EasyAccount", "", eMyMoney::Account::Type::Investment, QDate(2017, 8, 1), file->asset().id());
  auto toBeClosedAccID = makeAccount("EasyAccount", "123456789", eMyMoney::Account::Type::Investment, QDate(2017, 8, 1), file->asset().id()); // this account has the most characters matching the statement
  auto accID = makeAccount("Easy", "123456789", eMyMoney::Account::Type::Investment, QDate(2017, 8, 1), file->asset().id());
  makeAccount("EasyAccount", "123456789", eMyMoney::Account::Type::Checkings, QDate(2017, 8, 1), file->asset().id());
  ft.commit();

  auto csvContent = csvDataset(0);
  csvContent.prepend("Bank name:;\n");
  csvContent.prepend("BigInvestments;\n");
  csvContent.prepend("Account number:;\n");
  csvContent.prepend("123456789;\n");
  csvContent.prepend("Account name:;\n");
  csvContent.prepend("EasyAccount;\n");

  QString filename("account-autodetection.csv");
  writeStatementToCSV(csvContent, filename);

  investmentProfile->m_startLine = 7;
  csvImporter->m_autodetect[AutoAccountInvest] = true;

  auto st = csvImporter->unattendedImport(filename, investmentProfile);

  QVERIFY(st.m_listTransactions.count() == 3);
  QVERIFY(st.m_accountId == toBeClosedAccID);

  // closed account shouldn't be autodetected
  auto closedAcc = file->account(toBeClosedAccID);
  ft.restart();
  closedAcc.setClosed(true);
  file->modifyAccount(closedAcc);
  ft.commit();

  st = csvImporter->unattendedImport(filename, investmentProfile);
  QVERIFY(st.m_accountId == accID);
}

void CSVImporterCoreTest::testCalculatedFeeColumn()
{
  auto csvContent = csvDataset(0);
  QString filename("calculated-fee-column.csv");
  writeStatementToCSV(csvContent, filename);

  investmentProfile->m_feeRate = QLatin1String("4");
  investmentProfile->m_feeIsPercentage = true;  // fee is calculated always as percentage

  auto st = csvImporter->unattendedImport(filename, investmentProfile);
  QVERIFY(st.m_listTransactions[0].m_amount == MyMoneyMoney(-130));
  QVERIFY(st.m_listTransactions[0].m_fees == MyMoneyMoney(5));

  investmentProfile->m_minFee = QLatin1String("6");
  investmentProfile->m_colTypeNum.remove(Column::Fee); // hack
  st = csvImporter->unattendedImport(filename, investmentProfile);

  QVERIFY(st.m_listTransactions[0].m_amount == MyMoneyMoney(-131));
  QVERIFY(st.m_listTransactions[0].m_fees == MyMoneyMoney(6));  // minimal fee is 6 now, so fee of 5 from above test must be increased to 6
}
