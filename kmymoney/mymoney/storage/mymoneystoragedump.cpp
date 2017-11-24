/***************************************************************************
                          mymoneystoragedump.cpp  -  description
                             -------------------
    begin                : Sun May 5 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneystoragedump.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>
#include <QList>
#include <QStringList>
#include <QTextStream>
#include <QDataStream>
#include <QColor>

// ----------------------------------------------------------------------------
// KDE Includes
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneyserialize.h"
#include "imymoneystorage.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "mymoneytag.h"
#include "mymoneyreport.h"
#include "mymoneybudget.h"
#include "mymoneyschedule.h"
#include "mymoneypayee.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyenums.h"

MyMoneyStorageDump::MyMoneyStorageDump()
{
}

MyMoneyStorageDump::~MyMoneyStorageDump()
{
}

void MyMoneyStorageDump::readStream(QDataStream& /* s */, IMyMoneySerialize* /* storage */)
{
  qDebug("Reading not supported by MyMoneyStorageDump!!");
}

void MyMoneyStorageDump::writeStream(QDataStream& _s, IMyMoneySerialize* _storage)
{
  QTextStream s(_s.device());
  IMyMoneyStorage* storage = dynamic_cast<IMyMoneyStorage *>(_storage);
  MyMoneyPayee user = storage->user();

  s << "File-Info\n";
  s << "---------\n";
  s << "user name = " << user.name() << "\n";
  s << "user street = " << user.address() << "\n";
  s << "user city = " << user.city() << "\n";
  s << "user city = " << user.state() << "\n";
  s << "user zip = " << user.postcode() << "\n";
  s << "user telephone = " << user.telephone() << "\n";
  s << "user e-mail = " << user.email() << "\n";
  s << "creation date = " << storage->creationDate().toString(Qt::ISODate) << "\n";
  s << "last modification date = " << storage->lastModificationDate().toString(Qt::ISODate) << "\n";
  s << "base currency = " << storage->value("kmm-baseCurrency") << "\n";
  s << "\n";

  s << "Internal-Info\n";
  s << "-------------\n";
  QList<MyMoneyAccount> list_a;
  storage->accountList(list_a);
  s << "accounts = " << list_a.count() << ", next id = " << _storage->accountId() << "\n";
  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  QList<MyMoneyTransaction> list_t;
  storage->transactionList(list_t, filter);
  QList<MyMoneyTransaction>::ConstIterator it_t;
  s << "transactions = " << list_t.count() << ", next id = " << _storage->transactionId() << "\n";
  QMap<int, int> xferCount;
  for (it_t = list_t.constBegin(); it_t != list_t.constEnd(); ++it_t) {
    QList<MyMoneySplit>::ConstIterator it_s;
    int accountCount = 0;
    for (it_s = (*it_t).splits().constBegin(); it_s != (*it_t).splits().constEnd(); ++it_s) {
      MyMoneyAccount acc = storage->account((*it_s).accountId());
      if (acc.accountGroup() != eMyMoney::Account::Type::Expense
          && acc.accountGroup() != eMyMoney::Account::Type::Income)
        accountCount++;
    }
    if (accountCount > 1)
      xferCount[accountCount] = xferCount[accountCount] + 1;
  }
  QMap<int, int>::ConstIterator it_cnt;
  for (it_cnt = xferCount.constBegin(); it_cnt != xferCount.constEnd(); ++it_cnt) {
    s << "               " << *it_cnt << " of them references " << it_cnt.key() << " accounts\n";
  }

  s << "payees = " << _storage->payeeList().count() << ", next id = " << _storage->payeeId() << "\n";
  s << "tags = " << _storage->tagList().count() << ", next id = " << _storage->tagId() << "\n";
  s << "institutions = " << _storage->institutionList().count() << ", next id = " << _storage->institutionId() << "\n";
  s << "schedules = " << _storage->scheduleList().count() << ", next id = " << _storage->scheduleId() << "\n";
  s << "\n";

  s << "Institutions\n";
  s << "------------\n";

  QList<MyMoneyInstitution> list_i = storage->institutionList();
  QList<MyMoneyInstitution>::ConstIterator it_i;
  for (it_i = list_i.constBegin(); it_i != list_i.constEnd(); ++it_i) {
    s << "  ID = " << (*it_i).id() << "\n";
    s << "  Name = " << (*it_i).name() << "\n";
    s << "\n";
  }
  s << "\n";

  s << "Payees" << "\n";
  s << "------" << "\n";

  QList<MyMoneyPayee> list_p = storage->payeeList();
  QList<MyMoneyPayee>::ConstIterator it_p;
  for (it_p = list_p.constBegin(); it_p != list_p.constEnd(); ++it_p) {
    s << "  ID = " << (*it_p).id() << "\n";
    s << "  Name = " << (*it_p).name() << "\n";
    s << "  Address = " << (*it_p).address() << "\n";
    s << "  City = " << (*it_p).city() << "\n";
    s << "  State = " << (*it_p).state() << "\n";
    s << "  Zip = " << (*it_p).postcode() << "\n";
    s << "  E-Mail = " << (*it_p).email() << "\n";
    s << "  Telephone = " << (*it_p).telephone() << "\n";
    s << "  Reference = " << (*it_p).reference() << "\n";
    s << "\n";
  }
  s << "\n";

  s << "Tags" << "\n";
  s << "------" << "\n";

  QList<MyMoneyTag> list_ta = storage->tagList();
  QList<MyMoneyTag>::ConstIterator it_ta;
  for (it_ta = list_ta.constBegin(); it_ta != list_ta.constEnd(); ++it_ta) {
    s << "  ID = " << (*it_ta).id() << "\n";
    s << "  Name = " << (*it_ta).name() << "\n";
    s << "  Closed = " << (*it_ta).isClosed() << "\n";
    s << "  TagColor = " << (*it_ta).tagColor().name() << "\n";
    s << "  Notes = " << (*it_ta).notes() << "\n";
    s << "\n";
  }
  s << "\n";

  s << "Accounts" << "\n";
  s << "--------" << "\n";

  list_a.push_front(storage->equity());
  list_a.push_front(storage->expense());
  list_a.push_front(storage->income());
  list_a.push_front(storage->liability());
  list_a.push_front(storage->asset());
  QList<MyMoneyAccount>::ConstIterator it_a;
  for (it_a = list_a.constBegin(); it_a != list_a.constEnd(); ++it_a) {
    s << "  ID = " << (*it_a).id() << "\n";
    s << "  Name = " << (*it_a).name() << "\n";
    s << "  Number = " << (*it_a).number() << "\n";
    s << "  Description = " << (*it_a).description() << "\n";
    s << "  Type = " << (int)(*it_a).accountType() << "\n";
    if ((*it_a).currencyId().isEmpty()) {
      s << "  Currency = unknown\n";
    } else {
      if ((*it_a).isInvest()) {
        s << "  Equity = " << storage->security((*it_a).currencyId()).name() << "\n";
      } else {
        s << "  Currency = " << storage->currency((*it_a).currencyId()).name() << "\n";
      }
    }
    s << "  Parent = " << (*it_a).parentAccountId();
    if (!(*it_a).parentAccountId().isEmpty()) {
      MyMoneyAccount parent = storage->account((*it_a).parentAccountId());
      s << " (" << parent.name() << ")";
    } else {
      s << "n/a";
    }
    s << "\n";

    s << "  Institution = " << (*it_a).institutionId();
    if (!(*it_a).institutionId().isEmpty()) {
      MyMoneyInstitution inst = storage->institution((*it_a).institutionId());
      s << " (" << inst.name() << ")";
    } else {
      s << "n/a";
    }
    s << "\n";

    s << "  Opening date = " << (*it_a).openingDate().toString(Qt::ISODate) << "\n";
    s << "  Last modified = " << (*it_a).lastModified().toString(Qt::ISODate) << "\n";
    s << "  Last reconciled = " << (*it_a).lastReconciliationDate().toString(Qt::ISODate) << "\n";
    s << "  Balance = " << (*it_a).balance().formatMoney("", 2) << "\n";

    dumpKVP("  KVP: ", s, *it_a);
    dumpKVP("  OnlineBankingSettings: ", s, (*it_a).onlineBankingSettings());

    QStringList list_s = (*it_a).accountList();
    QStringList::ConstIterator it_s;
    if (list_s.count() > 0) {
      s << "  Children =" << "\n";
    }
    for (it_s = list_s.constBegin(); it_s != list_s.constEnd(); ++it_s) {
      MyMoneyAccount child = storage->account(*it_s);
      s << "    " << *it_s << " (" << child.name() << ")\n";
    }
    s << "\n";
  }
  s << "\n";

#if 0
  s << "Currencies" << "\n";
  s << "----------" << "\n";

  QList<MyMoneyCurrency> list_c = storage->currencyList();
  QList<MyMoneyCurrency>::ConstIterator it_c;
  for (it_c = list_c.begin(); it_c != list_c.end(); ++it_c) {
    s << "  Name = " << (*it_c).name() << "\n";
    s << "    ID = " << (*it_c).id() << "\n";
    s << "    Symbol = " << (*it_c).tradingSymbol() << "\n";
    s << "    Parts/Unit = " << (*it_c).partsPerUnit() << "\n";
    s << "    smallest cash fraction = " << (*it_c).smallestCashFraction() << "\n";
    s << "    smallest account fraction = " << (*it_c).smallestAccountFraction() << "\n";
    dumpPriceHistory(s, (*it_c).priceHistory());
    s << "\n";
  }
  s << "\n";
#endif

  s << "Securities" << "\n";
  s << "----------" << "\n";

  QList<MyMoneySecurity> list_e = storage->securityList();
  QList<MyMoneySecurity>::ConstIterator it_e;
  for (it_e = list_e.constBegin(); it_e != list_e.constEnd(); ++it_e) {
    s << "  Name = " << (*it_e).name() << "\n";
    s << "    ID = " << (*it_e).id() << "\n";
    s << "    Market   = " << (*it_e).tradingMarket() << "\n";
    s << "    Symbol   = " << (*it_e).tradingSymbol() << "\n";
    s << "    Currency = " << (*it_e).tradingCurrency() << " (";
    if ((*it_e).tradingCurrency().isEmpty()) {
      s << "unknown";
    } else {
      MyMoneySecurity tradingCurrency = storage->currency((*it_e).tradingCurrency());
      if (!tradingCurrency.isCurrency()) {
        s << "invalid currency: ";
      }
      s << tradingCurrency.name();
    }
    s << ")\n";

    s << "    Type = " << MyMoneySecurity::securityTypeToString((*it_e).securityType()) << "\n";
    s << "    smallest account fraction = " << (*it_e).smallestAccountFraction() << "\n";
    s << "    price precision = " << (*it_e).pricePrecision() << "\n";

    s << "    KVP: " << "\n";
    QMap<QString, QString>kvp = (*it_e).pairs();
    QMap<QString, QString>::Iterator it;
    for (it = kvp.begin(); it != kvp.end(); ++it) {
      s << "      '" << it.key() << "' = '" << it.value() << "'\n";
    }
    s << "\n";
  }
  s << "\n";

  s << "Prices" << "\n";
  s << "--------" << "\n";

  MyMoneyPriceList list_pr = _storage->priceList();
  MyMoneyPriceList::ConstIterator it_pr;
  for (it_pr = list_pr.constBegin(); it_pr != list_pr.constEnd(); ++it_pr) {
    s << "  From = " << it_pr.key().first << "\n";
    s << "    To = " << it_pr.key().second << "\n";
    MyMoneyPriceEntries::ConstIterator it_pre;
    for (it_pre = (*it_pr).constBegin(); it_pre != (*it_pr).constEnd(); ++it_pre) {
      s << "      Date = " << (*it_pre).date().toString() << "\n";
      s << "        Price = " << (*it_pre).rate(QString()).formatMoney("", 8) << "\n";
      s << "        Source = " << (*it_pre).source() << "\n";
      s << "        From = " << (*it_pre).from() << "\n";
      s << "        To   = " << (*it_pre).to() << "\n";
    }
    s << "\n";
  }
  s << "\n";

  s << "Transactions" << "\n";
  s << "------------" << "\n";

  for (it_t = list_t.constBegin(); it_t != list_t.constEnd(); ++it_t) {
    dumpTransaction(s, storage, *it_t);
  }
  s << "\n";


  s << "Schedules" << "\n";
  s << "---------" << "\n";

  QList<MyMoneySchedule> list_s = storage->scheduleList();
  QList<MyMoneySchedule>::ConstIterator it_s;
  for (it_s = list_s.constBegin(); it_s != list_s.constEnd(); ++it_s) {
    s << "  ID = " << (*it_s).id() << "\n";
    s << "  Name = " << (*it_s).name() << "\n";
    s << "  Startdate = " << (*it_s).startDate().toString(Qt::ISODate) << "\n";
    if ((*it_s).willEnd())
      s << "  Enddate   = " << (*it_s).endDate().toString(Qt::ISODate) << "\n";
    else
      s << "  Enddate   = not specified\n";
    s << "  Occurence = " << (*it_s).occurrenceToString() << "\n"; // krazy:exclude=spelling
    s << "  OccurenceMultiplier = " << (*it_s).occurrenceMultiplier() << "\n";
    s << "  Type = " << MyMoneySchedule::scheduleTypeToString((*it_s).type()) << "\n";
    s << "  Paymenttype = " << MyMoneySchedule::paymentMethodToString((*it_s).paymentType()) << "\n";
    s << "  Fixed = " << (*it_s).isFixed() << "\n";
    s << "  LastDayInDate = " << (*it_s).lastDayInMonth() << "\n";
    s << "  AutoEnter = " << (*it_s).autoEnter() << "\n";

    if ((*it_s).lastPayment().isValid())
      s << "  Last payment = " << (*it_s).lastPayment().toString(Qt::ISODate) << "\n";
    else
      s << "  Last payment = not defined" << "\n";
    if ((*it_s).isFinished())
      s << "  Next payment = payment finished" << "\n";
    else {
      s << "  Next payment = " << (*it_s).nextDueDate().toString(Qt::ISODate) << "\n";
      if ((*it_s).isOverdue())
        s << "               = overdue!" << "\n";
    }

    QList<QDate> list_d;
    QList<QDate>::ConstIterator it_d;

    list_d = (*it_s).recordedPayments();
    if (list_d.count() > 0) {
      s << "  Recorded payments" << "\n";
      for (it_d = list_d.constBegin(); it_d != list_d.constEnd(); ++it_d) {
        s << "    " << (*it_d).toString(Qt::ISODate) << "\n";
      }
    }
    s << "  TRANSACTION\n";
    dumpTransaction(s, storage, (*it_s).transaction());
  }
  s << "\n";

  s << "Reports" << "\n";
  s << "-------" << "\n";

  QList<MyMoneyReport> list_r = storage->reportList();
  QList<MyMoneyReport>::ConstIterator it_r;
  for (it_r = list_r.constBegin(); it_r != list_r.constEnd(); ++it_r) {
    s << "  ID = " << (*it_r).id() << "\n";
    s << "  Name = " << (*it_r).name() << "\n";
  }

  s << "Budgets" << "\n";
  s << "-------" << "\n";

  QList<MyMoneyBudget> list_b = storage->budgetList();
  QList<MyMoneyBudget>::ConstIterator it_b;
  for (it_b = list_b.constBegin(); it_b != list_b.constEnd(); ++it_b) {
    s << "  ID = " << (*it_b).id() << "\n";
    s << "  Name = " << (*it_b).name() << "\n";
  }
}

void MyMoneyStorageDump::dumpKVP(const QString& headline, QTextStream& s, const MyMoneyKeyValueContainer &kvp, int indent)
{
  QString ind;
  ind.fill(' ', indent);
  s << ind << headline << "\n";
  QMap<QString, QString>::const_iterator it;
  for (it = kvp.pairs().constBegin(); it != kvp.pairs().constEnd(); ++it) {
    s << ind << "  '" << it.key() << "' = '" << it.value() << "'\n";
  }
}

void MyMoneyStorageDump::dumpTransaction(QTextStream& s, IMyMoneyStorage* storage, const MyMoneyTransaction& it_t)
{
  s << "  ID = " << it_t.id() << "\n";
  s << "  Postdate  = " << it_t.postDate().toString(Qt::ISODate) << "\n";
  s << "  EntryDate = " << it_t.entryDate().toString(Qt::ISODate) << "\n";
  s << "  Commodity = [" << it_t.commodity() << "]\n";
  s << "  Memo = " << it_t.memo() << "\n";
  s << "  BankID = " << it_t.bankID() << "\n";
  dumpKVP("KVP:", s, it_t, 2);

  s << "  Splits\n";
  s << "  ------\n";
  QList<MyMoneySplit>::ConstIterator it_s;
  for (it_s = it_t.splits().constBegin(); it_s != it_t.splits().constEnd(); ++it_s) {
    s << "   ID = " << (*it_s).id() << "\n";
    s << "    Transaction = " << (*it_s).transactionId() << "\n";
    s << "    Payee = " << (*it_s).payeeId();
    if (!(*it_s).payeeId().isEmpty()) {
      MyMoneyPayee p = storage->payee((*it_s).payeeId());
      s << " (" << p.name() << ")" << "\n";
    } else
      s << " ()\n";
    for (int i = 0; i < (*it_s).tagIdList().size(); i++) {
      s << "    Tag = " << (*it_s).tagIdList()[i];
      if (!(*it_s).tagIdList()[i].isEmpty()) {
        MyMoneyTag ta = storage->tag((*it_s).tagIdList()[i]);
        s << " (" << ta.name() << ")" << "\n";
      } else
        s << " ()\n";
    }
    s << "    Account = " << (*it_s).accountId();
    MyMoneyAccount acc;
    try {
      acc = storage->account((*it_s).accountId());
      s << " (" << acc.name() << ") [" << acc.currencyId() << "]\n";
    } catch (const MyMoneyException &) {
      s << " (---) [---]\n";
    }
    s << "    Memo = " << (*it_s).memo() << "\n";
    if ((*it_s).value() == MyMoneyMoney::autoCalc)
      s << "    Value = will be calculated" << "\n";
    else
      s << "    Value = " << (*it_s).value().formatMoney("", 2)
      << " (" << (*it_s).value().toString() << ")\n";
    s << "    Shares = " << (*it_s).shares().formatMoney("", 2)
    << " (" << (*it_s).shares().toString() << ")\n";
    s << "    Action = '" << (*it_s).action() << "'\n";
    s << "    Nr = '" << (*it_s).number() << "'\n";
    s << "    ReconcileFlag = '" << reconcileToString((*it_s).reconcileFlag()) << "'\n";
    if ((*it_s).reconcileFlag() != eMyMoney::Split::State::NotReconciled) {
      s << "    ReconcileDate = " << (*it_s).reconcileDate().toString(Qt::ISODate) << "\n";
    }
    s << "    BankID = " << (*it_s).bankID() << "\n";
    dumpKVP("KVP:", s, (*it_s), 4);
    s << "\n";
  }
  s << "\n";
}

#define i18n QString

const QString MyMoneyStorageDump::reconcileToString(eMyMoney::Split::State flag) const
{
  QString rc;

  switch (flag) {
    case eMyMoney::Split::State::NotReconciled:
      rc = i18nc("Reconciliation status 'Not Reconciled'", "not reconciled");
      break;
    case eMyMoney::Split::State::Cleared:
      rc = i18nc("Reconciliation status 'Cleared'", "cleared");
      break;
    case eMyMoney::Split::State::Reconciled:
      rc = i18nc("Reconciliation status 'Reconciled'", "reconciled");
      break;
    case eMyMoney::Split::State::Frozen:
      rc = i18nc("Reconciliation status 'Frozen'", "frozen");
      break;
    default:
      rc = i18nc("Reconciliation status unknown", "unknown");
      break;
  }
  return rc;
}

#if 0
void MyMoneyStorageDump::dumpPriceHistory(QTextStream& s, const equity_price_history history)
{
  if (history.count() != 0) {
    s << "    Price History:\n";

    equity_price_history::const_iterator it_price = history.begin();
    while (it_price != history.end()) {
      s << "      " << it_price.key().toString() << ": " << it_price.data().toDouble() << "\n";
      it_price++;
    }
  }
}
#endif
