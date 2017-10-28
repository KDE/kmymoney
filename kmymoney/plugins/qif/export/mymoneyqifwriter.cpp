/***************************************************************************
                          mymoneyqifwriter.cpp  -  description
                             -------------------
    begin                : Sun Jan 5 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Allan Anderson agander93@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyqifwriter.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QFile>
#include <QList>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kmessagebox.h>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneysplit.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneyexception.h"

MyMoneyQifWriter::MyMoneyQifWriter()
{
}

MyMoneyQifWriter::~MyMoneyQifWriter()
{
}

void MyMoneyQifWriter::write(const QString& filename, const QString& profile,
                             const QString& accountId, const bool accountData,
                             const bool categoryData,
                             const QDate& startDate, const QDate& endDate)
{
  m_qifProfile.loadProfile("Profile-" + profile);

  QFile qifFile(filename);
  if (qifFile.open(QIODevice::WriteOnly)) {
    QTextStream s(&qifFile);
    s.setCodec("UTF-8");

    try {
      if (categoryData) {
        writeCategoryEntries(s);
      }

      if (accountData) {
        writeAccountEntry(s, accountId, startDate, endDate);
      }
      emit signalProgress(-1, -1);

    } catch (const MyMoneyException &e) {
      QString errMsg = i18n("Unexpected exception '%1' thrown in %2, line %3 "
                            "caught in MyMoneyQifWriter::write()", e.what(), e.file(), e.line());

      KMessageBox::error(0, errMsg);
    }

    qifFile.close();
    qDebug() << "Export completed.\n";
  } else {
    KMessageBox::error(0, i18n("Unable to open file '%1' for writing", filename));
  }
}

void MyMoneyQifWriter::writeAccountEntry(QTextStream& s, const QString& accountId, const QDate& startDate, const QDate& endDate)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount account;

  account = file->account(accountId);
  MyMoneyTransactionFilter filter(accountId);

  QString openingBalanceTransactionId;
  QString type = m_qifProfile.profileType();

  s << "!Type:" << type << endl;
  if (type == "Invst") {
    extractInvestmentEntries(s, accountId, startDate, endDate);
  } else {
    filter.setDateFilter(startDate, endDate);
    QList<MyMoneyTransaction> list = file->transactionList(filter);
    if (!startDate.isValid() || startDate <= account.openingDate()) {
      s << "D" << m_qifProfile.date(account.openingDate()) << endl;
      openingBalanceTransactionId = file->openingBalanceTransaction(account);
      MyMoneySplit split;
      if (!openingBalanceTransactionId.isEmpty()) {
        MyMoneyTransaction openingBalanceTransaction = file->transaction(openingBalanceTransactionId);
        split = openingBalanceTransaction.splitByAccount(account.id(), true /* match */);
      }
      s << "T" << m_qifProfile.value('T', split.value()) << endl;
    } else {
      s << "D" << m_qifProfile.date(startDate) << endl;
      s << "T" << m_qifProfile.value('T', file->balance(accountId, startDate.addDays(-1))) << endl;
    }
    s << "CX" << endl;
    s << "P" << m_qifProfile.openingBalanceText() << endl;
    s << "L";
    if (m_qifProfile.accountDelimiter().length())
      s << m_qifProfile.accountDelimiter()[0];
    s << account.name();
    if (m_qifProfile.accountDelimiter().length() > 1)
      s << m_qifProfile.accountDelimiter()[1];
    s << endl;
    s << "^" << endl;

    QList<MyMoneyTransaction>::ConstIterator it;
    signalProgress(0, list.count());
    int count = 0;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
      // don't include the openingBalanceTransaction again
      if ((*it).id() != openingBalanceTransactionId)
        writeTransactionEntry(s, *it, accountId);
      signalProgress(++count, 0);
    }
  }
}

void MyMoneyQifWriter::writeCategoryEntries(QTextStream &s)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount income;
  MyMoneyAccount expense;

  income = file->income();
  expense = file->expense();

  s << "!Type:Cat" << endl;
  QStringList list = income.accountList() + expense.accountList();
  emit signalProgress(0, list.count());
  QStringList::Iterator it;
  int count = 0;
  for (it = list.begin(); it != list.end(); ++it) {
    writeCategoryEntry(s, *it, "");
    emit signalProgress(++count, 0);
  }
}

void MyMoneyQifWriter::writeCategoryEntry(QTextStream &s, const QString& accountId, const QString& leadIn)
{
  MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
  QString name = acc.name();

  s << "N" << leadIn << name << endl;
  s << (acc.accountGroup() == eMyMoney::Account::Expense ? "E" : "I") << endl;
  s << "^" << endl;

  QStringList list = acc.accountList();
  QStringList::Iterator it;
  name += ':';
  for (it = list.begin(); it != list.end(); ++it) {
    writeCategoryEntry(s, *it, name);
  }
}

void MyMoneyQifWriter::writeTransactionEntry(QTextStream &s, const MyMoneyTransaction& t, const QString& accountId)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySplit split = t.splitByAccount(accountId);

  s << "D" << m_qifProfile.date(t.postDate()) << endl;

  switch (split.reconcileFlag()) {
    case eMyMoney::Split::State::Cleared:
      s << "C*" << endl;
      break;

    case eMyMoney::Split::State::Reconciled:
    case eMyMoney::Split::State::Frozen:
      s << "CX" << endl;
      break;

    default:
      break;
  }

  if (split.memo().length() > 0) {
    QString m = split.memo();
    m.replace('\n', "\\n");
    s << "M" << m << endl;
  }

  s << "T" << m_qifProfile.value('T', split.value()) << endl;

  if (split.number().length() > 0)
    s << "N" << split.number() << endl;

  if (!split.payeeId().isEmpty()) {
    MyMoneyPayee payee = file->payee(split.payeeId());
    s << "P" << payee.name() << endl;
  }

  QList<MyMoneySplit> list = t.splits();
  if (list.count() > 1) {
    MyMoneySplit sp = t.splitByAccount(accountId, false);
    MyMoneyAccount acc = file->account(sp.accountId());
    if (acc.accountGroup() != eMyMoney::Account::Income
        && acc.accountGroup() != eMyMoney::Account::Expense) {
      s << "L" << m_qifProfile.accountDelimiter()[0]
      << MyMoneyFile::instance()->accountToCategory(sp.accountId())
      << m_qifProfile.accountDelimiter()[1] << endl;
    } else {
      s << "L" << file->accountToCategory(sp.accountId()) << endl;
    }
    if (list.count() > 2) {
      QList<MyMoneySplit>::ConstIterator it;
      for (it = list.constBegin(); it != list.constEnd(); ++it) {
        if (!((*it) == split)) {
          writeSplitEntry(s, *it);
        }
      }
    }
  }
  s << "^" << endl;
}

void MyMoneyQifWriter::writeSplitEntry(QTextStream& s, const MyMoneySplit& split)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  s << "S";
  MyMoneyAccount acc = file->account(split.accountId());
  if (acc.accountGroup() != eMyMoney::Account::Income
      && acc.accountGroup() != eMyMoney::Account::Expense) {
    s << m_qifProfile.accountDelimiter()[0]
    << file->accountToCategory(split.accountId())
    << m_qifProfile.accountDelimiter()[1];
  } else {
    s << file->accountToCategory(split.accountId());
  }
  s << endl;

  if (split.memo().length() > 0) {
    QString m = split.memo();
    m.replace('\n', "\\n");
    s << "E" << m << endl;
  }

  s << "$" << m_qifProfile.value('$', -split.value()) << endl;
}

void MyMoneyQifWriter::extractInvestmentEntries(QTextStream &s, const QString& accountId, const QDate& startDate, const QDate& endDate)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<QString> accList = file->account(accountId).accountList();
  QList<QString>::ConstIterator itAcc;
  for (itAcc = accList.constBegin(); itAcc != accList.constEnd(); ++itAcc) {
    MyMoneyTransactionFilter filter((*itAcc));
    filter.setDateFilter(startDate, endDate);
    QList<MyMoneyTransaction> list = file->transactionList(filter);
    QList<MyMoneyTransaction>::ConstIterator it;
    signalProgress(0, list.count());
    int count = 0;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
      writeInvestmentEntry(s, *it, ++count);
      signalProgress(count, 0);
    }
  }
}

void MyMoneyQifWriter::writeInvestmentEntry(QTextStream& stream, const MyMoneyTransaction& t, const int count)
{
  QString s;
  QString memo;
  MyMoneyFile* file = MyMoneyFile::instance();
  QString chkAccnt;
  bool isXfer = false;
  bool noError = true;
  QList<MyMoneySplit> lst = t.splits();
  QList<MyMoneySplit>::Iterator it;
  eMyMoney::Account typ;
  QString chkAccntId;
  MyMoneyMoney qty;
  MyMoneyMoney value;
  QMap<eMyMoney::Account, QString> map;

  for (int i = 0; i < lst.count(); i++) {
    QString actionType = lst[i].action();
    MyMoneyAccount acc = file->account(lst[i].accountId());
    QString accName = acc.name();
    typ = acc.accountType();
    map.insert(typ, lst[i].accountId());
    if (typ == eMyMoney::Account::Stock) {
      memo = lst[i].memo();
    }
  }
  //
  //  Add date.
  //
  if (noError) {
    s += 'D' + m_qifProfile.date(t.postDate()) + '\n';
  }
  for (it = lst.begin(); it != lst.end(); ++it) {
    QString accName;
    QString actionType = (*it).action();
    MyMoneyAccount acc = file->account((*it).accountId());
    typ = acc.accountType();
    //
    //  eMyMoney::Account::Checkings.
    //
    if ((acc.accountType() == eMyMoney::Account::Checkings) || (acc.accountType() == eMyMoney::Account::Cash)) {
      chkAccntId = (*it).accountId();
      chkAccnt = file->account(chkAccntId).name();
    } else if (acc.accountType() == eMyMoney::Account::Income) {
      //
      //  eMyMoney::Account::Income.
      //
    } else if (acc.accountType() == eMyMoney::Account::Expense) {
      //
      //  eMyMoney::Account::Expense.
      //
    } else if (acc.accountType() == eMyMoney::Account::Stock) {
      //
      //  eMyMoney::Account::Stock.
      //
      qty = (*it).shares();
      value = (*it).value();

      accName = acc.name();
      if ((actionType == "Dividend") || (actionType == "Buy") || (actionType == "IntIncome")) {
        isXfer = true;
      }
      //
      //  Actions.
      //
      QString action;
      if ((*it).action() == "Dividend") {
        action = "DivX";
      } else if ((*it).action() == "IntIncome") {
        action = "IntIncX";
      }
      if ((action == "DivX") || (action == "IntIncX")) {
        if (map.value(eMyMoney::Account::Checkings).isEmpty()) {
          KMessageBox::sorry(0,
                             QString("<qt>%1</qt>").arg(i18n("Transaction number <b>%1</b> is missing an account assignment.\nTransaction dropped.", count)),
                             i18n("Invalid transaction"));
          noError = false;
          return;
        }
        MyMoneySplit sp = t.splitByAccount(map.value(eMyMoney::Account::Checkings), true);
        QString txt = sp.value().formatMoney("", 2);
        if (noError) {
          s += 'T' + txt + '\n';
        }
      } else if ((*it).action() == "Buy") {

        if (qty.isNegative()) {
          action = "Sell";
        } else {
          action = "Buy";
        }
      } else if ((*it).action() == "Add") {
        qty = (*it).shares();
        if (qty.isNegative()) {
          action = "Shrsout";
        } else {
          action = "Shrsin";
        }
      } else if ((*it).action() == "Reinvest") {
        action = "ReinvDiv";
      } else {
        action = (*it).action();
      }

      //
      //  Add action.
      //
      if (noError) {
        s += 'N' + action + '\n';
      }
      QString txt;
      if ((action == "Buy") || (action == "Sell") || (action == "ReinvDiv")) {
        //
        //  Add total.
        //
        txt = value.formatMoney("", 2);
        if (action == "Sell") {
          value = -value;
          txt = value.formatMoney("", 2);
        }
        if (noError) {
          s += 'T' + txt + '\n';
        }
        //
        //  Add price.
        //
        txt = (*it).price().formatMoney("", 6);
        if (noError) {
          s += 'I' + txt + '\n';
        }
        if (!qty.isZero()) {
          //
          //  Add quantity.
          //
          if (noError) {
            if (action == "Sell") {
              qty = -qty;
            }
            s += 'Q' + m_qifProfile.value('Q', qty) + '\n';
          }
        }
      } else if ((action == "Shrsin") || (action == "Shrsout")) {
        //
        //  Add quantity for "Shrsin" || "Shrsout".
        //
        if (noError) {
          if (action == "Shrsout") {
            qty = -qty;
          }
          s += 'Q' + m_qifProfile.value('Q', qty) + '\n';
        }
      }
    }
    if (!accName.isEmpty()) {
      if (noError) {
        s += 'Y' + accName + '\n';
      }
    }
  }
  if (!memo.isEmpty()) {
    if (noError) {
      memo.replace('\n', "\\n");
      s += 'M' + memo + '\n';
    }
  }
  if ((!chkAccnt.isEmpty()) && isXfer) {
    //
    //  Add account - including its hierarchy.
    //
    if (noError) {
      s += 'L' + m_qifProfile.accountDelimiter()[0] + file->accountToCategory(chkAccntId)
           + m_qifProfile.accountDelimiter()[1] + '\n';
      stream << s;
    } else {
      // Don't output the transaction
    }
  } else {
    stream << s;
  }
  stream << '^' << '\n';
}
