/***************************************************************************
                          reportstestcommon.cpp
                          -------------------
    copyright            : (C) 2002-2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           Ace Jones <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kreportsview-test.h"

#include <QList>
#include <QFile>
#include <QTextStream>

#include "reportstestcommon.h"

#include "pivottable.h"
#include "querytable.h"
using namespace reports;

#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "mymoneysplit.h"
#include "mymoneystoragedump.h"
#include "mymoneyreport.h"
#include "mymoneystatement.h"
#include "mymoneystoragexml.h"

namespace test
{

const MyMoneyMoney moCheckingOpen(0.0);
const MyMoneyMoney moCreditOpen(-0.0);
const MyMoneyMoney moConverterCheckingOpen(1418.0);
const MyMoneyMoney moConverterCreditOpen(-418.0);
const MyMoneyMoney moZero(0.0);
const MyMoneyMoney moSolo(234.12);
const MyMoneyMoney moParent1(88.01);
const MyMoneyMoney moParent2(133.22);
const MyMoneyMoney moParent(moParent1 + moParent2);
const MyMoneyMoney moChild(14.00);
const MyMoneyMoney moThomas(5.11);
const MyMoneyMoney moNoPayee(8944.70);

QString acAsset;
QString acLiability;
QString acExpense;
QString acIncome;
QString acChecking;
QString acCredit;
QString acSolo;
QString acParent;
QString acChild;
QString acSecondChild;
QString acGrandChild1;
QString acGrandChild2;
QString acForeign;
QString acCanChecking;
QString acJpyChecking;
QString acCanCash;
QString acJpyCash;
QString inBank;
QString eqStock1;
QString eqStock2;
QString eqStock3;
QString eqStock4;
QString acInvestment;
QString acStock1;
QString acStock2;
QString acStock3;
QString acStock4;
QString acDividends;
QString acInterest;
QString acFees;
QString acTax;
QString acCash;

TransactionHelper::TransactionHelper(const QDate& _date, const QString& _action, MyMoneyMoney _value, const QString& _accountid, const QString& _categoryid, const QString& _currencyid, const QString& _payee)
{
  // _currencyid is the currency of the transaction, and of the _value
  // both the account and category can have their own currency (athough the category having
  // a foreign currency is not yet supported by the program, the reports will still allow it,
  // so it must be tested.)
  MyMoneyFile* file = MyMoneyFile::instance();
  bool haspayee = ! _payee.isEmpty();
  MyMoneyPayee payeeTest = file->payeeByName(_payee);

  MyMoneyFileTransaction ft;
  setPostDate(_date);

  QString currencyid = _currencyid;
  if (currencyid.isEmpty())
    currencyid = MyMoneyFile::instance()->baseCurrency().id();
  setCommodity(currencyid);

  MyMoneyMoney price;
  MyMoneySplit splitLeft;
  if (haspayee)
    splitLeft.setPayeeId(payeeTest.id());
  splitLeft.setAction(_action);
  splitLeft.setValue(-_value);
  price = MyMoneyFile::instance()->price(currencyid, file->account(_accountid).currencyId(), _date).rate(file->account(_accountid).currencyId());
  splitLeft.setShares(-_value * price);
  splitLeft.setAccountId(_accountid);
  addSplit(splitLeft);

  MyMoneySplit splitRight;
  if (haspayee)
    splitRight.setPayeeId(payeeTest.id());
  splitRight.setAction(_action);
  splitRight.setValue(_value);
  price = MyMoneyFile::instance()->price(currencyid, file->account(_categoryid).currencyId(), _date).rate(file->account(_categoryid).currencyId());
  splitRight.setShares(_value * price);
  splitRight.setAccountId(_categoryid);
  addSplit(splitRight);

  MyMoneyFile::instance()->addTransaction(*this);
  ft.commit();
}

TransactionHelper::~TransactionHelper()
{
  MyMoneyFileTransaction ft;
  MyMoneyFile::instance()->removeTransaction(*this);
  ft.commit();
}

void TransactionHelper::update()
{
  MyMoneyFileTransaction ft;
  MyMoneyFile::instance()->modifyTransaction(*this);
  ft.commit();
}

InvTransactionHelper::InvTransactionHelper(const QDate& _date, const QString& _action, MyMoneyMoney _shares, MyMoneyMoney _price, const QString& _stockaccountid, const QString& _transferid, const QString& _categoryid, MyMoneyMoney _fee)
{
  init(_date, _action, _shares, _price, _fee, _stockaccountid, _transferid, _categoryid);
}

void InvTransactionHelper::init(const QDate& _date, const QString& _action, MyMoneyMoney _shares, MyMoneyMoney _price, MyMoneyMoney _fee, const QString& _stockaccountid, const QString& _transferid, const QString& _categoryid)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount stockaccount = file->account(_stockaccountid);
  MyMoneyMoney value = _shares * _price;

  setPostDate(_date);

  setCommodity("USD");
  MyMoneySplit s1;
  s1.setValue(value);
  s1.setAccountId(_stockaccountid);

  if (_action == MyMoneySplit::ActionReinvestDividend) {
    s1.setShares(_shares);
    s1.setAction(MyMoneySplit::ActionReinvestDividend);

    MyMoneySplit s2;
    s2.setAccountId(_categoryid);
    s2.setShares(-value);
    s2.setValue(-value);
    addSplit(s2);
  } else if (_action == MyMoneySplit::ActionDividend || _action == MyMoneySplit::ActionYield) {
    s1.setAccountId(_categoryid);
    s1.setShares(-value);
    s1.setValue(-value);

    // Split 2 will be the zero-amount investment split that serves to
    // mark this transaction as a cash dividend and note which stock account
    // it belongs to.
    MyMoneySplit s2;
    s2.setValue(MyMoneyMoney());
    s2.setShares(MyMoneyMoney());
    s2.setAction(_action);
    s2.setAccountId(_stockaccountid);
    addSplit(s2);

    MyMoneySplit s3;
    s3.setAccountId(_transferid);
    s3.setShares(value);
    s3.setValue(value);
    addSplit(s3);
  } else if (_action == MyMoneySplit::ActionBuyShares) {
    s1.setShares(_shares);
    s1.setValue(value);
    s1.setAction(MyMoneySplit::ActionBuyShares);

    MyMoneySplit s3;
    s3.setAccountId(_transferid);
    s3.setShares(-value - _fee);
    s3.setValue(-value - _fee);
    addSplit(s3);

    if (!_categoryid.isEmpty() && !_fee.isZero()) {
      MyMoneySplit s2;
      s2.setAccountId(_categoryid);
      s2.setValue(_fee);
      s2.setShares(_fee);
      addSplit(s2);
    }
  } else if (_action == MyMoneySplit::ActionSplitShares) {
    s1.setShares(_shares.abs());
    s1.setValue(MyMoneyMoney());
    s1.setPrice(MyMoneyMoney());
  }
  addSplit(s1);

  //qDebug() << "created transaction, now adding...";

  MyMoneyFileTransaction ft;
  file->addTransaction(*this);

  //qDebug() << "updating price...";

  // update the price, while we're here
  if (_action != MyMoneySplit::ActionSplitShares) {
    QString stockid = stockaccount.currencyId();
    QString basecurrencyid = file->baseCurrency().id();
    MyMoneyPrice price = file->price(stockid, basecurrencyid, _date, true);
    if (!price.isValid()) {
      MyMoneyPrice newprice(stockid, basecurrencyid, _date, _price, "test");
      file->addPrice(newprice);
    }
  }
  ft.commit();
  //qDebug() << "successfully added " << id();
}

QString makeAccount(const QString& _name, eMyMoney::Account _type, MyMoneyMoney _balance, const QDate& _open, const QString& _parent, QString _currency, bool _taxReport, bool _openingBalance)
{
  MyMoneyAccount info;
  MyMoneyFileTransaction ft;

  info.setName(_name);
  info.setAccountType(_type);
  info.setOpeningDate(_open);
  if (!_currency.isEmpty())
    info.setCurrencyId(_currency);
  else
    info.setCurrencyId(MyMoneyFile::instance()->baseCurrency().id());

  if (_taxReport)
    info.setValue("Tax", "Yes");

  if (_openingBalance)
    info.setValue("OpeningBalanceAccount", "Yes");

  MyMoneyAccount parent = MyMoneyFile::instance()->account(_parent);
  MyMoneyFile::instance()->addAccount(info, parent);
  // create the opening balance transaction if any
  if (!_balance.isZero()) {
    MyMoneySecurity sec = MyMoneyFile::instance()->currency(info.currencyId());
    MyMoneyFile::instance()->openingBalanceAccount(sec);
    MyMoneyFile::instance()->createOpeningBalanceTransaction(info, _balance);
  }
  ft.commit();

  return info.id();
}

void makePrice(const QString& _currencyid, const QDate& _date, const MyMoneyMoney& _price)
{
  MyMoneyFileTransaction ft;
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySecurity curr = file->currency(_currencyid);
  MyMoneyPrice price(_currencyid, file->baseCurrency().id(), _date, _price, "test");
  file->addPrice(price);
  ft.commit();
}

QString makeEquity(const QString& _name, const QString& _symbol)
{
  MyMoneySecurity equity;
  MyMoneyFileTransaction ft;

  equity.setName(_name);
  equity.setTradingSymbol(_symbol);
  equity.setSmallestAccountFraction(1000);
  equity.setSecurityType(eMyMoney::Security::None/*MyMoneyEquity::ETYPE_STOCK*/);
  MyMoneyFile::instance()->addSecurity(equity);
  ft.commit();

  return equity.id();
}

void makeEquityPrice(const QString& _id, const QDate& _date, const MyMoneyMoney& _price)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyFileTransaction ft;
  QString basecurrencyid = file->baseCurrency().id();
  MyMoneyPrice price = file->price(_id, basecurrencyid, _date, true);
  if (!price.isValid()) {
    MyMoneyPrice newprice(_id, basecurrencyid, _date, _price, "test");
    file->addPrice(newprice);
  }
  ft.commit();
}

void writeRCFtoXMLDoc(const MyMoneyReport& filter, QDomDocument* doc)
{
  QDomProcessingInstruction instruct = doc->createProcessingInstruction(QString("xml"), QString("version=\"1.0\" encoding=\"utf-8\""));
  doc->appendChild(instruct);

  QDomElement root = doc->createElement("KMYMONEY-FILE");
  doc->appendChild(root);

  QDomElement reports = doc->createElement("REPORTS");
  root.appendChild(reports);

  QDomElement report = doc->createElement("REPORT");
  filter.write(report, doc);
  reports.appendChild(report);

}

void writeTabletoHTML(const PivotTable& table, const QString& _filename)
{
  static unsigned filenumber = 1;
  QString filename = _filename;
  if (filename.isEmpty()) {
    filename = QString("report-%1%2.html").arg((filenumber < 10) ? "0" : "").arg(filenumber);
    ++filenumber;
  }

  QFile g(filename);
  g.open(QIODevice::WriteOnly);
  QTextStream(&g) << table.renderHTML();
  g.close();

}

void writeTabletoHTML(const QueryTable& table, const QString& _filename)
{
  static unsigned filenumber = 1;
  QString filename = _filename;
  if (filename.isEmpty()) {
    filename = QString("report-%1%2.html").arg((filenumber < 10) ? "0" : "").arg(filenumber);
    ++filenumber;
  }

  QFile g(filename);
  g.open(QIODevice::WriteOnly);
  QTextStream(&g) << table.renderHTML();
  g.close();
}

void writeTabletoCSV(const PivotTable& table, const QString& _filename)
{
  static unsigned filenumber = 1;
  QString filename = _filename;
  if (filename.isEmpty()) {
    filename = QString("report-%1%2.csv").arg((filenumber < 10) ? "0" : "").arg(filenumber);
    ++filenumber;
  }

  QFile g(filename);
  g.open(QIODevice::WriteOnly);
  QTextStream(&g) << table.renderCSV();
  g.close();

}

void writeTabletoCSV(const QueryTable& table, const QString& _filename)
{
  static unsigned filenumber = 1;
  QString filename = _filename;
  if (filename.isEmpty()) {
    filename = QString("qreport-%1%2.csv").arg((filenumber < 10) ? "0" : "").arg(filenumber);
    ++filenumber;
  }

  QFile g(filename);
  g.open(QIODevice::WriteOnly);
  QTextStream(&g) << table.renderCSV();
  g.close();

}

void writeRCFtoXML(const MyMoneyReport& filter, const QString& _filename)
{
  static unsigned filenum = 1;
  QString filename = _filename;
  if (filename.isEmpty()) {
    filename = QString("report-%1%2.xml").arg(QString::number(filenum).rightJustified(2, '0'));
    ++filenum;
  }

  QDomDocument* doc = new QDomDocument("KMYMONEY-FILE");
  Q_CHECK_PTR(doc);

  writeRCFtoXMLDoc(filter, doc);

  QFile g(filename);
  g.open(QIODevice::WriteOnly);

  QTextStream stream(&g);
  stream.setCodec("UTF-8");
  stream << doc->toString();
  g.close();

  delete doc;
}

bool readRCFfromXMLDoc(QList<MyMoneyReport>& list, QDomDocument* doc)
{
  bool result = false;

  QDomElement rootElement = doc->documentElement();
  if (!rootElement.isNull()) {
    QDomNode child = rootElement.firstChild();
    while (!child.isNull() && child.isElement()) {
      QDomElement childElement = child.toElement();
      if ("REPORTS" == childElement.tagName()) {
        result = true;
        QDomNode subchild = child.firstChild();
        while (!subchild.isNull() && subchild.isElement()) {
          MyMoneyReport filter;
          if (filter.read(subchild.toElement())) {
            list += filter;
          }
          subchild = subchild.nextSibling();
        }
      }
      child = child.nextSibling();
    }
  }
  return result;
}

bool readRCFfromXML(QList<MyMoneyReport>& list, const QString& filename)
{
  int result = false;
  QFile f(filename);
  f.open(QIODevice::ReadOnly);
  QDomDocument* doc = new QDomDocument;
  if (doc->setContent(&f, false)) {
    result = readRCFfromXMLDoc(list, doc);
  }
  delete doc;

  return result;

}

void XMLandback(MyMoneyReport& filter)
{
  // this function writes the filter to XML, and then reads
  // it back from XML overwriting the original filter;
  // in all cases, the result should be the same if the read
  // & write methods are working correctly.

  QDomDocument* doc = new QDomDocument("KMYMONEY-FILE");
  Q_CHECK_PTR(doc);

  writeRCFtoXMLDoc(filter, doc);
  QList<MyMoneyReport> list;
  if (readRCFfromXMLDoc(list, doc) && !list.isEmpty())
    filter = list[0];
  else
    throw MYMONEYEXCEPTION("Failed to load report from XML");

  delete doc;

}

MyMoneyMoney searchHTML(const QString& _html, const QString& _search)
{
  QRegExp re(QString("%1[<>/td]*([\\-.0-9,]*)").arg(_search));
  re.indexIn(_html);
  QString found = re.cap(1);
  found.remove(',');

  return MyMoneyMoney(found.toDouble());
}

} // end namespace test
