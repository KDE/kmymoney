/*
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneystatement.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QList>
#include <QDomProcessingInstruction>
#include <QDomElement>
#include <QDomDocument>

// ----------------------------------------------------------------------------
// Project Includes

namespace eMyMoney {
namespace Statement {
enum class Element {
    KMMStatement,
    Statement,
    Transaction,
    Split,
    Price,
    Security,
};
uint qHash(const Element key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}

enum class Attribute {
    Name,
    Symbol,
    ID,
    Version,
    AccountName,
    AccountNumber,
    BankCode,
    Currency,
    BeginDate,
    EndDate,
    ClosingBalance,
    Type,
    AccountID,
    SkipCategoryMatching,
    DatePosted,
    Payee,
    Memo,
    Number,
    Amount,
    BankID,
    Reconcile,
    Action,
    Shares,
    Security,
    BrokerageAccount,
    Category,
};
uint qHash(const Attribute key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}
}
}

using namespace eMyMoney;

const QHash<Statement::Type, QString> txAccountType {
    {Statement::Type::None,       QStringLiteral("none")},
    {Statement::Type::Checkings,  QStringLiteral("checkings")},
    {Statement::Type::Savings,    QStringLiteral("savings")},
    {Statement::Type::Investment, QStringLiteral("investment")},
    {Statement::Type::CreditCard, QStringLiteral("creditcard")},
    {Statement::Type::Invalid,    QStringLiteral("invalid")}
};

const QHash<Transaction::Action, QString> txAction {
    {Transaction::Action::None,             QStringLiteral("none")},
    {Transaction::Action::Buy,              QStringLiteral("buy")},
    {Transaction::Action::Sell,             QStringLiteral("sell")},
    {Transaction::Action::ReinvestDividend, QStringLiteral("reinvestdividend")},
    {Transaction::Action::CashDividend,     QStringLiteral("cashdividend")},
    {Transaction::Action::Shrsin,           QStringLiteral("add")},
    {Transaction::Action::Shrsout,          QStringLiteral("remove")},
    {Transaction::Action::Stksplit,         QStringLiteral("stocksplit")},
    {Transaction::Action::Fees,             QStringLiteral("fees")},
    {Transaction::Action::Interest,         QStringLiteral("interest")},
    {Transaction::Action::Invalid,          QStringLiteral("invalid")}
};

QString getElName(const Statement::Element el)
{
    static const QHash<Statement::Element, QString> elNames {
        {Statement::Element::KMMStatement, QStringLiteral("KMYMONEY-STATEMENT")},
        {Statement::Element::Statement,    QStringLiteral("STATEMENT")},
        {Statement::Element::Transaction,  QStringLiteral("TRANSACTION")},
        {Statement::Element::Split,        QStringLiteral("SPLIT")},
        {Statement::Element::Price,        QStringLiteral("PRICE")},
        {Statement::Element::Security,     QStringLiteral("SECURITY")}
    };
    return elNames[el];
}

QString getAttrName(const Statement::Attribute attr)
{
    static const QHash<Statement::Attribute, QString> attrNames {
        {Statement::Attribute::Name,                 QStringLiteral("name")},
        {Statement::Attribute::Symbol,               QStringLiteral("symbol")},
        {Statement::Attribute::ID,                   QStringLiteral("id")},
        {Statement::Attribute::Version,              QStringLiteral("version")},
        {Statement::Attribute::AccountName,          QStringLiteral("accountname")},
        {Statement::Attribute::AccountNumber,        QStringLiteral("accountnumber")},
        {Statement::Attribute::BankCode,             QStringLiteral("routingnumber")},
        {Statement::Attribute::Currency,             QStringLiteral("currency")},
        {Statement::Attribute::BeginDate,            QStringLiteral("begindate")},
        {Statement::Attribute::EndDate,              QStringLiteral("enddate")},
        {Statement::Attribute::ClosingBalance,       QStringLiteral("closingbalance")},
        {Statement::Attribute::Type,                 QStringLiteral("type")},
        {Statement::Attribute::AccountID,            QStringLiteral("accountid")},
        {Statement::Attribute::SkipCategoryMatching, QStringLiteral("skipCategoryMatching")},
        {Statement::Attribute::DatePosted,           QStringLiteral("dateposted")},
        {Statement::Attribute::Payee,                QStringLiteral("payee")},
        {Statement::Attribute::Memo,                 QStringLiteral("memo")},
        {Statement::Attribute::Number,               QStringLiteral("number")},
        {Statement::Attribute::Amount,               QStringLiteral("amount")},
        {Statement::Attribute::BankID,               QStringLiteral("bankid")},
        {Statement::Attribute::Reconcile,            QStringLiteral("reconcile")},
        {Statement::Attribute::Action,               QStringLiteral("action")},
        {Statement::Attribute::Shares,               QStringLiteral("shares")},
        {Statement::Attribute::Security,             QStringLiteral("security")},
        {Statement::Attribute::BrokerageAccount,     QStringLiteral("brokerageaccount")},
        {Statement::Attribute::Category,             QStringLiteral("version")},
    };
    return attrNames[attr];
}

void MyMoneyStatement::write(QDomElement& _root, QDomDocument* _doc) const
{
    QDomElement e = _doc->createElement(getElName(Statement::Element::Statement));
    _root.appendChild(e);

    e.setAttribute(getAttrName(Statement::Attribute::Version), QStringLiteral("1.1"));
    e.setAttribute(getAttrName(Statement::Attribute::AccountName), m_strAccountName);
    e.setAttribute(getAttrName(Statement::Attribute::AccountNumber), m_strAccountNumber);
    e.setAttribute(getAttrName(Statement::Attribute::BankCode), m_strBankCode);
    e.setAttribute(getAttrName(Statement::Attribute::Currency), m_strCurrency);
    e.setAttribute(getAttrName(Statement::Attribute::BeginDate), m_dateBegin.toString(Qt::ISODate));
    e.setAttribute(getAttrName(Statement::Attribute::EndDate), m_dateEnd.toString(Qt::ISODate));
    e.setAttribute(getAttrName(Statement::Attribute::ClosingBalance), m_closingBalance.toString());
    e.setAttribute(getAttrName(Statement::Attribute::Type), txAccountType[m_eType]);
    e.setAttribute(getAttrName(Statement::Attribute::AccountID), m_accountId);
    e.setAttribute(getAttrName(Statement::Attribute::SkipCategoryMatching), m_skipCategoryMatching);

    // iterate over transactions, and add each one
    foreach (const auto tansaction, m_listTransactions) {
        auto p = _doc->createElement(getElName(Statement::Element::Transaction));
        p.setAttribute(getAttrName(Statement::Attribute::DatePosted), tansaction.m_datePosted.toString(Qt::ISODate));
        p.setAttribute(getAttrName(Statement::Attribute::Payee), tansaction.m_strPayee);
        p.setAttribute(getAttrName(Statement::Attribute::Memo), tansaction.m_strMemo);
        p.setAttribute(getAttrName(Statement::Attribute::Number), tansaction.m_strNumber);
        p.setAttribute(getAttrName(Statement::Attribute::Amount), tansaction.m_amount.toString());
        p.setAttribute(getAttrName(Statement::Attribute::BankID), tansaction.m_strBankID);
        p.setAttribute(getAttrName(Statement::Attribute::Reconcile), (int)tansaction.m_reconcile);
        p.setAttribute(getAttrName(Statement::Attribute::Action), txAction[tansaction.m_eAction]);

        if (m_eType == eMyMoney::Statement::Type::Investment) {
            p.setAttribute(getAttrName(Statement::Attribute::Shares), tansaction.m_shares.toString());
            p.setAttribute(getAttrName(Statement::Attribute::Security), tansaction.m_strSecurity);
            p.setAttribute(getAttrName(Statement::Attribute::BrokerageAccount), tansaction.m_strBrokerageAccount);
        }

        // add all the splits we know of (might be empty)
        foreach (const auto split, tansaction.m_listSplits) {
            auto el = _doc->createElement(getElName(Statement::Element::Split));
            el.setAttribute(getAttrName(Statement::Attribute::AccountID), split.m_accountId);
            el.setAttribute(getAttrName(Statement::Attribute::Amount), split.m_amount.toString());
            el.setAttribute(getAttrName(Statement::Attribute::Reconcile), (int)split.m_reconcile);
            el.setAttribute(getAttrName(Statement::Attribute::Category), split.m_strCategoryName);
            el.setAttribute(getAttrName(Statement::Attribute::Memo), split.m_strMemo);
            el.setAttribute(getAttrName(Statement::Attribute::Reconcile), (int)split.m_reconcile);
            p.appendChild(el);
        }
        e.appendChild(p);
    }

    // iterate over prices, and add each one
    foreach (const auto price, m_listPrices) {
        auto p = _doc->createElement(getElName(Statement::Element::Price));
        p.setAttribute(getAttrName(Statement::Attribute::DatePosted), price.m_date.toString(Qt::ISODate));
        p.setAttribute(getAttrName(Statement::Attribute::Security), price.m_strSecurity);
        p.setAttribute(getAttrName(Statement::Attribute::Amount), price.m_amount.toString());

        e.appendChild(p);
    }

    // iterate over securities, and add each one
    foreach (const auto security, m_listSecurities) {
        auto p = _doc->createElement(getElName(Statement::Element::Security));
        p.setAttribute(getAttrName(Statement::Attribute::Name), security.m_strName);
        p.setAttribute(getAttrName(Statement::Attribute::Symbol), security.m_strSymbol);
        p.setAttribute(getAttrName(Statement::Attribute::ID), security.m_strId);

        e.appendChild(p);
    }
}

bool MyMoneyStatement::read(const QDomElement& _e)
{
    bool result = false;

    if (_e.tagName() == getElName(Statement::Element::Statement)) {
        result = true;

        m_strAccountName = _e.attribute(getAttrName(Statement::Attribute::AccountName));
        m_strAccountNumber = _e.attribute(getAttrName(Statement::Attribute::AccountNumber));
        m_strBankCode = _e.attribute(getAttrName(Statement::Attribute::BankCode));
        m_strCurrency = _e.attribute(getAttrName(Statement::Attribute::Currency));
        m_dateBegin = QDate::fromString(_e.attribute(getAttrName(Statement::Attribute::BeginDate)), Qt::ISODate);
        m_dateEnd = QDate::fromString(_e.attribute(getAttrName(Statement::Attribute::EndDate)), Qt::ISODate);
        m_closingBalance = MyMoneyMoney(_e.attribute(getAttrName(Statement::Attribute::ClosingBalance)));
        m_accountId = _e.attribute(getAttrName(Statement::Attribute::AccountID));
        m_skipCategoryMatching = _e.attribute(getAttrName(Statement::Attribute::SkipCategoryMatching)).isEmpty();

        auto txt = _e.attribute(getAttrName(Statement::Attribute::Type), txAccountType[Statement::Type::Checkings]);
        m_eType = txAccountType.key(txt, m_eType);

        QDomNode child = _e.firstChild();
        while (!child.isNull() && child.isElement()) {
            QDomElement c = child.toElement();

            if (c.tagName() == getElName(Statement::Element::Transaction)) {
                MyMoneyStatement::Transaction t;

                t.m_datePosted = QDate::fromString(c.attribute(getAttrName(Statement::Attribute::DatePosted)), Qt::ISODate);
                t.m_amount = MyMoneyMoney(c.attribute(getAttrName(Statement::Attribute::Amount)));
                t.m_strMemo = c.attribute(getAttrName(Statement::Attribute::Memo));
                t.m_strNumber = c.attribute(getAttrName(Statement::Attribute::Number));
                t.m_strPayee = c.attribute(getAttrName(Statement::Attribute::Payee));
                t.m_strBankID = c.attribute(getAttrName(Statement::Attribute::BankID));
                t.m_reconcile = static_cast<eMyMoney::Split::State>(c.attribute(getAttrName(Statement::Attribute::Reconcile)).toInt());

                txt = c.attribute(getAttrName(Statement::Attribute::Action), txAction[eMyMoney::Transaction::Action::Buy]);
                t.m_eAction = txAction.key(txt, t.m_eAction);

                if (m_eType == eMyMoney::Statement::Type::Investment) {
                    t.m_shares = MyMoneyMoney(c.attribute(getAttrName(Statement::Attribute::Shares)));
                    t.m_strSecurity = c.attribute(getAttrName(Statement::Attribute::Security));
                    t.m_strBrokerageAccount = c.attribute(getAttrName(Statement::Attribute::BrokerageAccount));
                }

                // process splits (if any)
                auto splitChild = c.firstChild();
                while (!splitChild.isNull() && splitChild.isElement()) {
                    c = splitChild.toElement();
                    if (c.tagName() == getElName(Statement::Element::Split)) {
                        MyMoneyStatement::Split s;
                        s.m_accountId = c.attribute(getAttrName(Statement::Attribute::AccountID));
                        s.m_amount = MyMoneyMoney(c.attribute(getAttrName(Statement::Attribute::Amount)));
                        s.m_reconcile = static_cast<eMyMoney::Split::State>(c.attribute(getAttrName(Statement::Attribute::Reconcile)).toInt());
                        s.m_strCategoryName = c.attribute(getAttrName(Statement::Attribute::Category));
                        s.m_strMemo = c.attribute(getAttrName(Statement::Attribute::Memo));
                        t.m_listSplits += s;
                    }
                    splitChild = splitChild.nextSibling();
                }
                m_listTransactions += t;
            } else if (c.tagName() == getElName(Statement::Element::Price)) {
                MyMoneyStatement::Price p;

                p.m_date = QDate::fromString(c.attribute(getAttrName(Statement::Attribute::DatePosted)), Qt::ISODate);
                p.m_strSecurity = c.attribute(getAttrName(Statement::Attribute::Security));
                p.m_amount = MyMoneyMoney(c.attribute(getAttrName(Statement::Attribute::Amount)));

                m_listPrices += p;
            } else if (c.tagName() == getElName(Statement::Element::Security)) {
                MyMoneyStatement::Security s;

                s.m_strName = c.attribute(getAttrName(Statement::Attribute::Name));
                s.m_strSymbol = c.attribute(getAttrName(Statement::Attribute::Symbol));
                s.m_strId = c.attribute(getAttrName(Statement::Attribute::ID));

                m_listSecurities += s;
            }
            child = child.nextSibling();
        }
    }

    return result;
}

bool MyMoneyStatement::isStatementFile(const QString& _filename)
{
    // filename is considered a statement file if it contains
    // the tag "<KMYMONEY2-STATEMENT>" in the first 20 lines.
    bool result = false;

    QFile f(_filename);
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream ts(&f);

        auto lineCount = 20;
        while (!ts.atEnd() && !result && lineCount != 0) {
            if (ts.readLine().contains(QLatin1String("<KMYMONEY-STATEMENT>"), Qt::CaseInsensitive))
                result = true;
            --lineCount;
        }
        f.close();
    }

    return result;
}

void MyMoneyStatement::writeXMLFile(const MyMoneyStatement& _s, const QString& _filename)
{
    static unsigned filenum = 1;
    auto filename = _filename;
    if (filename.isEmpty()) {
        filename = QString::fromLatin1("statement-%1%2.xml").arg((filenum < 10) ? QStringLiteral("0") : QString()).arg(filenum);
        filenum++;
    }

    auto doc = new QDomDocument(getElName(Statement::Element::KMMStatement));
    Q_CHECK_PTR(doc);

    //writeStatementtoXMLDoc(_s,doc);
    QDomProcessingInstruction instruct = doc->createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"utf-8\""));
    doc->appendChild(instruct);
    auto eroot = doc->createElement(getElName(Statement::Element::KMMStatement));
    doc->appendChild(eroot);
    _s.write(eroot, doc);

    QFile g(filename);
    if (g.open(QIODevice::WriteOnly)) {
        QTextStream stream(&g);
        stream.setCodec("UTF-8");
        stream << doc->toString();
        g.close();
    }

    delete doc;
}

bool MyMoneyStatement::readXMLFile(MyMoneyStatement& _s, const QString& _filename)
{
    bool result = false;
    QFile f(_filename);
    f.open(QIODevice::ReadOnly);
    QDomDocument* doc = new QDomDocument;
    if (doc->setContent(&f, false)) {
        QDomElement rootElement = doc->documentElement();
        if (!rootElement.isNull()) {
            QDomNode child = rootElement.firstChild();
            while (!child.isNull() && child.isElement()) {
                result = true;
                QDomElement childElement = child.toElement();
                _s.read(childElement);

                child = child.nextSibling();
            }
        }
    }
    delete doc;

    return result;
}

QDate MyMoneyStatement::statementEndDate() const
{
    if (m_dateEnd.isValid())
        return m_dateEnd;

    QDate postDate;
    for(auto& t : m_listTransactions) {
        if (t.m_datePosted > postDate) {
            postDate = t.m_datePosted;
        }
    }
    return postDate;
}
