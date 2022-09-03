/*
    SPDX-FileCopyrightText: 2005-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "testutilities.h"

#include <QDebug>
#include <QFile>
#include <QList>
#include <QRegularExpression>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneyprice.h"
#include "mymoneyreport.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneystatement.h"
#include "mymoneystoragedump.h"
#include "xmlhelper/xmlstoragehelper.h"

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
QString acTransfer;
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
QString curBase;

TransactionHelper::TransactionHelper(const QDate& _date, const QString& _action, MyMoneyMoney _value, const QString& _accountid, const QString& _categoryid, const QString& _currencyid, const QString& _payee)
{
    // _currencyid is the currency of the transaction, and of the _value
    // both the account and category can have their own currency (although the category having
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
    try {
        MyMoneyFile::instance()->removeTransaction(*this);
        ft.commit();
    } catch (const MyMoneyException &e) {
        qDebug() << e.what();
    }
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

    if (_action == MyMoneySplit::actionName(eMyMoney::Split::Action::ReinvestDividend)) {
        s1.setShares(_shares);
        s1.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::ReinvestDividend));

        MyMoneySplit s2;
        s2.setAccountId(_categoryid);
        s2.setShares(-value);
        s2.setValue(-value);
        addSplit(s2);
    } else if (_action == MyMoneySplit::actionName(eMyMoney::Split::Action::Dividend) || _action == MyMoneySplit::actionName(eMyMoney::Split::Action::Yield)) {
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
    } else if (_action == MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares)) {
        s1.setShares(_shares);
        s1.setValue(value);
        s1.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares));

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
    } else if (_action == MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares)) {
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
    if (_action != MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares)) {
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

QString makeAccount(const QString& id, const QString& _name, eMyMoney::Account::Type _type, MyMoneyMoney _balance, const QDate& _open, const QString& _parent, QString _currency, bool _taxReport, bool _openingBalance)
{
    int maxTries = 1000;
    QString cid;
    do {
        if (!cid.isEmpty()) {
            auto acc = MyMoneyFile::instance()->account(cid);
            MyMoneyFileTransaction ft;
            MyMoneyFile::instance()->removeAccount(acc);
            ft.commit();
        }
        cid = makeAccount(_name, _type, _balance, _open, _parent, _currency, _taxReport, _openingBalance);
    } while(maxTries-- && id != cid);
    if (maxTries <= 0)
        throw MYMONEYEXCEPTION_CSTRING("Account could not be prepared");
    return cid;
}

QString makeAccount(const QString& _name, eMyMoney::Account::Type _type, MyMoneyMoney _balance, const QDate& _open, const QString& _parent, QString _currency, bool _taxReport, bool _openingBalance)
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
    equity.setSecurityType(eMyMoney::Security::Type::None/*MyMoneyEquity::ETYPE_STOCK*/);
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

QString makeBaseCurrency(const MyMoneySecurity& base)
{
    auto file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;
    try {
        file->currency(base.id());
    } catch (const MyMoneyException &e) {
        file->addCurrency(base);
    }
    file->setBaseCurrency(base);
    ft.commit();
    return base.id();
}

void writeRCFtoXMLDoc(const MyMoneyReport& filter, QXmlStreamWriter* writer)
{
    writer->writeDTD(QLatin1String("<!DOCTYPE KMYMONEY-FILE>"));
    writer->writeStartElement(QLatin1String("KMYMONEY-FILE"));
    writer->writeStartElement(QLatin1String("REPORTS"));
    MyMoneyXmlHelper::writeReport(filter, writer);
    writer->writeEndElement();
    writer->writeEndElement();
}

void writeRCFtoXML(const MyMoneyReport& filter, const QString& _filename)
{
    static unsigned filenum = 1;
    QString filename = _filename;
    if (filename.isEmpty()) {
        filename = QString("report-%1%2.xml").arg(QString::number(filenum).rightJustified(2, '0'));
        ++filenum;
    }

    QFile g(filename);
    g.open(QIODevice::WriteOnly);

    QXmlStreamWriter writer(&g);
    writeRCFtoXMLDoc(filter, &writer);

    g.close();
}

bool readRCFfromXMLDoc(QList<MyMoneyReport>& list, QXmlStreamReader* reader)
{
    bool result = false;

    while (reader->readNextStartElement()) {
        if (reader->name() == QLatin1String("KMYMONEY-FILE")) {
            while (reader->readNextStartElement()) {
                if (reader->name() == QLatin1String("REPORTS")) {
                    while (reader->readNextStartElement()) {
                        if (reader->name() == QLatin1String("REPORT")) {
                            result = true;
                            const auto filter = MyMoneyXmlHelper::readReport(reader);
                            if (reader->hasError())
                                qDebug() << reader->errorString();
                            list += filter;
                        } else {
                            reader->skipCurrentElement();
                        }
                    }
                } else {
                    reader->skipCurrentElement();
                }
            }
        } else {
            reader->skipCurrentElement();
        }
    }

    return result;
}

bool readRCFfromXML(QList<MyMoneyReport>& list, const QString& filename)
{
    int result = false;
    QFile f(filename);
    f.open(QIODevice::ReadOnly);

    QXmlStreamReader reader;
    reader.setDevice(&f);

    result = readRCFfromXMLDoc(list, &reader);

    return result;

}

void XMLandback(MyMoneyReport& filter)
{
    // this function writes the filter to XML, and then reads
    // it back from XML overwriting the original filter;
    // in all cases, the result should be the same if the read
    // & write methods are working correctly.

    QString xml;
    QXmlStreamWriter writer(&xml);

    writeRCFtoXMLDoc(filter, &writer);
    QXmlStreamReader reader(xml);
    QList<MyMoneyReport> list;
    if (readRCFfromXMLDoc(list, &reader) && !list.isEmpty())
        filter = list[0];
    else
        throw MYMONEYEXCEPTION_CSTRING("Failed to load report from XML");
}

MyMoneyMoney searchHTML(const QString& _html, const QString& _search)
{
    Q_UNUSED(_html)
    const QRegularExpression re(QStringLiteral("%1[<>/td]*([\\-.0-9,]*)").arg(_search));
    const auto html(re.match(_html));
    if (html.hasMatch()) {
        auto found = html.captured(1);
        found.remove(',');

        return MyMoneyMoney(found.toDouble());
    }
    return MyMoneyMoney();
}

} // end namespace test
