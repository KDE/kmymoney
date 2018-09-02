/*
 * Copyright 2005-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2005-2006  Ace Jones <acejones@users.sourceforge.net>
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

#ifndef TESTUTILITIES_H
#define TESTUTILITIES_H

#include <QList>
#include <QDate>

class QDomDocument;

#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "mymoneymoney.h"
class MyMoneyReport;

namespace reports
{
class PivotTable;
class QueryTable;
}

namespace test
{

extern const MyMoneyMoney moCheckingOpen;
extern const MyMoneyMoney moCreditOpen;
extern const MyMoneyMoney moConverterCheckingOpen;
extern const MyMoneyMoney moConverterCreditOpen;
extern const MyMoneyMoney moZero;
extern const MyMoneyMoney moSolo;
extern const MyMoneyMoney moParent1;
extern const MyMoneyMoney moParent2;
extern const MyMoneyMoney moParent;
extern const MyMoneyMoney moChild;
extern const MyMoneyMoney moThomas;
extern const MyMoneyMoney moNoPayee;

extern QString acAsset;
extern QString acLiability;
extern QString acExpense;
extern QString acIncome;
extern QString acChecking;
extern QString acCredit;
extern QString acSolo;
extern QString acParent;
extern QString acChild;
extern QString acSecondChild;
extern QString acGrandChild1;
extern QString acGrandChild2;
extern QString acForeign;
extern QString acCanChecking;
extern QString acJpyChecking;
extern QString acCanCash;
extern QString acJpyCash;
extern QString inBank;
extern QString eqStock1;
extern QString eqStock2;
extern QString eqStock3;
extern QString eqStock4;
extern QString acInvestment;
extern QString acStock1;
extern QString acStock2;
extern QString acStock3;
extern QString acStock4;
extern QString acDividends;
extern QString acInterest;
extern QString acFees;
extern QString acTax;
extern QString acCash;

class TransactionHelper: public MyMoneyTransaction
{
private:
  QString m_id;
public:
  TransactionHelper(const QDate& _date, const QString& _action, MyMoneyMoney _value, const QString& _accountid, const QString& _categoryid, const QString& _currencyid = QString(), const QString& _payee = "Test Payee");
  ~TransactionHelper();
  void update();
protected:
  TransactionHelper() {}
};

class InvTransactionHelper: public TransactionHelper
{
public:
  InvTransactionHelper(const QDate& _date, const QString& _action, MyMoneyMoney _shares, MyMoneyMoney _value, const QString& _stockaccountid, const QString& _transferid, const QString& _categoryid, MyMoneyMoney _fee = MyMoneyMoney());
  void init(const QDate& _date, const QString& _action, MyMoneyMoney _shares, MyMoneyMoney _price, MyMoneyMoney _fee, const QString& _stockaccountid, const QString& _transferid, const QString& _categoryid);
};

class BudgetEntryHelper
{
private:
  QDate m_date;
  QString m_categoryid;
  MyMoneyMoney m_amount;

public:
  BudgetEntryHelper() {}
  BudgetEntryHelper(const QDate& _date, const QString& _categoryid, bool /* _applytosub */, const MyMoneyMoney& _amount): m_date(_date), m_categoryid(_categoryid), m_amount(_amount) {}
};

class BudgetHelper: public QList<BudgetEntryHelper>
{
  MyMoneyMoney budgetAmount(const QDate& _date, const QString& _categoryid, bool& _applytosub);
};

extern QString makeAccount(const QString& _name, eMyMoney::Account::Type _type, MyMoneyMoney _balance, const QDate& _open, const QString& _parent, QString _currency = "", bool _taxReport = false, bool _openingBalance = false);
extern void makePrice(const QString& _currencyid, const QDate& _date, const MyMoneyMoney& _price);
QString makeEquity(const QString& _name, const QString& _symbol);
extern void makeEquityPrice(const QString& _id, const QDate& _date, const MyMoneyMoney& _price);
extern void writeRCFtoXMLDoc(const MyMoneyReport& filter, QDomDocument* doc);
extern void writeRCFtoXML(const MyMoneyReport& filter, const QString& _filename = QString());
extern bool readRCFfromXMLDoc(QList<MyMoneyReport>& list, QDomDocument* doc);
extern bool readRCFfromXML(QList<MyMoneyReport>& list, const QString& filename);
extern void XMLandback(MyMoneyReport& filter);
extern MyMoneyMoney searchHTML(const QString& _html, const QString& _search);

} // end namespace test

#endif // TESTUTILITIES_H
