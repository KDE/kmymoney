/***************************************************************************
                          mymoneystatement.cpp
                          -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

const QStringList kAccountTypeTxt = QString("none,checkings,savings,investment,creditcard,invalid").split(',');
const QStringList kActionText = QString("none,buy,sell,reinvestdividend,cashdividend,add,remove,stocksplit,fees,interest,invalid").split(',');

void MyMoneyStatement::write(QDomElement& _root, QDomDocument* _doc) const
{
  QDomElement e = _doc->createElement("STATEMENT");
  _root.appendChild(e);

  e.setAttribute("version", "1.1");
  e.setAttribute("accountname", m_strAccountName);
  e.setAttribute("accountnumber", m_strAccountNumber);
  e.setAttribute("routingnumber", m_strRoutingNumber);
  e.setAttribute("currency", m_strCurrency);
  e.setAttribute("begindate", m_dateBegin.toString(Qt::ISODate));
  e.setAttribute("enddate", m_dateEnd.toString(Qt::ISODate));
  e.setAttribute("closingbalance", m_closingBalance.toString());
  e.setAttribute("type", kAccountTypeTxt[m_eType]);
  e.setAttribute("accountid", m_accountId);
  e.setAttribute("skipCategoryMatching", m_skipCategoryMatching);

  // iterate over transactions, and add each one
  QList<Transaction>::const_iterator it_t = m_listTransactions.begin();
  while (it_t != m_listTransactions.end()) {
    QDomElement p = _doc->createElement("TRANSACTION");
    p.setAttribute("dateposted", (*it_t).m_datePosted.toString(Qt::ISODate));
    p.setAttribute("payee", (*it_t).m_strPayee);
    p.setAttribute("memo", (*it_t).m_strMemo);
    p.setAttribute("number", (*it_t).m_strNumber);
    p.setAttribute("amount", (*it_t).m_amount.toString());
    p.setAttribute("bankid", (*it_t).m_strBankID);
    p.setAttribute("reconcile", (int)(*it_t).m_reconcile);
    p.setAttribute("action", kActionText[(*it_t).m_eAction]);

    if (m_eType == etInvestment) {
      p.setAttribute("shares", (*it_t).m_shares.toString());
      p.setAttribute("security", (*it_t).m_strSecurity);
      p.setAttribute("brokerageaccount", (*it_t).m_strBrokerageAccount);
    }

    // add all the splits we know of (might be empty)
    QList<Split>::const_iterator it_s;
    for (it_s = (*it_t).m_listSplits.begin(); it_s != (*it_t).m_listSplits.end(); ++it_s) {
      QDomElement split = _doc->createElement("SPLIT");
      split.setAttribute("accountid", (*it_s).m_accountId);
      split.setAttribute("amount", (*it_s).m_amount.toString());
      split.setAttribute("reconcile", (int)(*it_s).m_reconcile);
      split.setAttribute("category", (*it_s).m_strCategoryName);
      split.setAttribute("memo", (*it_s).m_strMemo);
      split.setAttribute("reconcile", (int)(*it_s).m_reconcile);
      p.appendChild(split);
    }

    e.appendChild(p);

    ++it_t;
  }

  // iterate over prices, and add each one
  QList<Price>::const_iterator it_p = m_listPrices.begin();
  while (it_p != m_listPrices.end()) {
    QDomElement p = _doc->createElement("PRICE");
    p.setAttribute("dateposted", (*it_p).m_date.toString(Qt::ISODate));
    p.setAttribute("security", (*it_p).m_strSecurity);
    p.setAttribute("amount", (*it_p).m_amount.toString());

    e.appendChild(p);

    ++it_p;
  }

  // iterate over securities, and add each one
  QList<Security>::const_iterator it_s = m_listSecurities.begin();
  while (it_s != m_listSecurities.end()) {
    QDomElement p = _doc->createElement("SECURITY");
    p.setAttribute("name", (*it_s).m_strName);
    p.setAttribute("symbol", (*it_s).m_strSymbol);
    p.setAttribute("id", (*it_s).m_strId);

    e.appendChild(p);

    ++it_s;
  }

}

bool MyMoneyStatement::read(const QDomElement& _e)
{
  bool result = false;

  if (_e.tagName() == "STATEMENT") {
    result = true;

    m_strAccountName = _e.attribute("accountname");
    m_strAccountNumber = _e.attribute("accountnumber");
    m_strRoutingNumber = _e.attribute("routingnumber");
    m_strCurrency = _e.attribute("currency");
    m_dateBegin = QDate::fromString(_e.attribute("begindate"), Qt::ISODate);
    m_dateEnd = QDate::fromString(_e.attribute("enddate"), Qt::ISODate);
    m_closingBalance = MyMoneyMoney(_e.attribute("closingbalance"));
    m_accountId = _e.attribute("accountid");
    m_skipCategoryMatching = _e.attribute("skipCategoryMatching").isEmpty();

    int i = kAccountTypeTxt.indexOf(_e.attribute("type", kAccountTypeTxt[1]));
    if (i != -1)
      m_eType = static_cast<EType>(i);

    QDomNode child = _e.firstChild();
    while (!child.isNull() && child.isElement()) {
      QDomElement c = child.toElement();

      if (c.tagName() == "TRANSACTION") {
        MyMoneyStatement::Transaction t;

        t.m_datePosted = QDate::fromString(c.attribute("dateposted"), Qt::ISODate);
        t.m_amount = MyMoneyMoney(c.attribute("amount"));
        t.m_strMemo = c.attribute("memo");
        t.m_strNumber = c.attribute("number");
        t.m_strPayee = c.attribute("payee");
        t.m_strBankID = c.attribute("bankid");
        t.m_reconcile = static_cast<eMyMoney::Split::State>(c.attribute("reconcile").toInt());
        int i = kActionText.indexOf(c.attribute("action", kActionText[1]));
        if (i != -1)
          t.m_eAction = static_cast<Transaction::EAction>(i);

        if (m_eType == etInvestment) {
          t.m_shares = MyMoneyMoney(c.attribute("shares"));
          t.m_strSecurity = c.attribute("security");
          t.m_strBrokerageAccount = c.attribute("brokerageaccount");
        }

        // process splits (if any)
        QDomNode child = c.firstChild();
        while (!child.isNull() && child.isElement()) {
          QDomElement c = child.toElement();
          if (c.tagName() == "SPLIT") {
            MyMoneyStatement::Split s;
            s.m_accountId = c.attribute("accountid");
            s.m_amount = MyMoneyMoney(c.attribute("amount"));
            s.m_reconcile = static_cast<eMyMoney::Split::State>(c.attribute("reconcile").toInt());
            s.m_strCategoryName = c.attribute("category");
            s.m_strMemo = c.attribute("memo");
            t.m_listSplits += s;
          }
          child = child.nextSibling();
        }
        m_listTransactions += t;
      } else if (c.tagName() == "PRICE") {
        MyMoneyStatement::Price p;

        p.m_date = QDate::fromString(c.attribute("dateposted"), Qt::ISODate);
        p.m_strSecurity = c.attribute("security");
        p.m_amount = MyMoneyMoney(c.attribute("amount"));

        m_listPrices += p;
      } else if (c.tagName() == "SECURITY") {
        MyMoneyStatement::Security s;

        s.m_strName = c.attribute("name");
        s.m_strSymbol = c.attribute("symbol");
        s.m_strId = c.attribute("id");

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

    int lineCount = 20;
    while (!ts.atEnd() && !result && lineCount != 0) {
      if (ts.readLine().contains("<KMYMONEY-STATEMENT>", Qt::CaseInsensitive))
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
  QString filename = _filename;
  if (filename.isEmpty()) {
    filename = QString("statement-%1%2.xml").arg((filenum < 10) ? "0" : "").arg(filenum);
    filenum++;
  }

  QDomDocument* doc = new QDomDocument("KMYMONEY-STATEMENT");
  Q_CHECK_PTR(doc);

  //writeStatementtoXMLDoc(_s,doc);
  QDomProcessingInstruction instruct = doc->createProcessingInstruction(QString("xml"), QString("version=\"1.0\" encoding=\"utf-8\""));
  doc->appendChild(instruct);
  QDomElement eroot = doc->createElement("KMYMONEY-STATEMENT");
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
