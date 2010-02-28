/***************************************************************************
                          kendingbalancedlg.cpp
                             -------------------
    copyright            : (C) 2000,2003 by Michael Edwardes, Thomas Baumgart
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kendingbalancedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneyedit.h>
#include <mymoneysplit.h>
#include <mymoneyfile.h>
#include <kmymoneycategory.h>
#include <kmymoneyaccountselector.h>
#include <ktoolinvocation.h>

#include "kcurrencycalculator.h"

class KEndingBalanceDlg::Private
{
public:
  MyMoneyTransaction        m_tInterest;
  MyMoneyTransaction        m_tCharges;
  MyMoneyAccount            m_account;
  QMap<QWidget*, QString>   m_helpAnchor;
};

class KEndingBalanceLoanDlg::Private
{
public:
  MyMoneyTransaction        m_tInterest;
  MyMoneyTransaction        m_tCharges;
  MyMoneyAccountLoan        m_account;
  QMap<QWidget*, QString>   m_helpAnchor;
};

KEndingBalanceDlg::KEndingBalanceDlg(const MyMoneyAccount& account, QWidget *parent) :
    KEndingBalanceDlgDecl(parent),
    d(new Private)
{
  setModal(true);
  QString value;
  MyMoneyMoney endBalance, startBalance;

  d->m_account = account;

  MyMoneySecurity currency = MyMoneyFile::instance()->security(account.currencyId());
  m_enterInformationLabel->setText(QString("<qt>") + i18n("Please enter the following fields with the information as you find them on your statement. Make sure to enter all values in <b>%1</b>.", currency.name()) + QString("</qt>"));

  m_statementDate->setDate(QDate::currentDate());

  // If the previous reconciliation was postponed,
  // we show a different first page
  value = account.value("lastReconciledBalance");
  if (value.isEmpty()) {
    // if the last statement has been entered long enough ago (more than one month),
    // then take the last statement date and add one month and use that as statement
    // date.
    QDate lastStatementDate = account.lastReconciliationDate();
    if (lastStatementDate.addMonths(1) < QDate::currentDate()) {
      m_statementDate->setDate(lastStatementDate.addMonths(1));
    }

    slotUpdateBalances();

    setAppropriate(m_startPageCheckings, true);
    setAppropriate(m_pagePreviousPostpone, false);
    setAppropriate(m_interestChargeCheckings, true);
    setFinishEnabled(m_interestChargeCheckings, true);
  } else {
    setAppropriate(m_startPageCheckings, false);
    setAppropriate(m_pagePreviousPostpone, true);
    removePage(m_interestChargeCheckings);
    setFinishEnabled(m_statementInfoPageCheckings, true);
    // make sure, we show the correct start page
    showPage(m_pagePreviousPostpone);

    MyMoneyMoney factor(1, 1);
    if (d->m_account.accountGroup() == MyMoneyAccount::Liability)
      factor = -factor;

    startBalance = MyMoneyMoney(value) * factor;
    value = account.value("statementBalance");
    endBalance = MyMoneyMoney(value) * factor;

    m_previousBalance->setValue(startBalance);
    m_endingBalance->setValue(endBalance);
  }

  // We don't need to add the default into the list (see ::help() why)
  // m_helpAnchor[m_startPageCheckings] = QString("");
  d->m_helpAnchor[m_interestChargeCheckings] = QString("details.reconcile.wizard.interest");
  d->m_helpAnchor[m_statementInfoPageCheckings] = QString("details.reconcile.wizard.statement");

  value = account.value("statementDate");
  if (!value.isEmpty())
    m_statementDate->setDate(QDate::fromString(value, Qt::ISODate));

  m_lastStatementDate->setText(QString());
  if (account.lastReconciliationDate().isValid()) {
    m_lastStatementDate->setText(i18n("Last reconciled statement: %1", KGlobal::locale()->formatDate(account.lastReconciliationDate())));
  }

  // remove all unwanted pages
  removePage(m_startPageLoan);
  removePage(m_checkPaymentsPage);
  removePage(m_adjustmentTransactionPage);

  // connect the signals with the slots
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadEditWidgets()));
  connect(m_payeeEdit, SIGNAL(createItem(const QString&, QString&)), this, SIGNAL(createPayee(const QString&, QString&)));
  connect(m_interestCategoryEdit, SIGNAL(createItem(const QString&, QString&)), this, SLOT(slotCreateInterestCategory(const QString&, QString&)));
  connect(m_chargesCategoryEdit, SIGNAL(createItem(const QString&, QString&)), this, SLOT(slotCreateChargesCategory(const QString&, QString&)));

  connect(m_interestEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_interestCategoryEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_chargesEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_chargesCategoryEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_statementDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotUpdateBalances()));

  slotReloadEditWidgets();

  // preset payee if possible
  try {
    // if we find a payee with the same name as the institution,
    // than this is what we use as payee.
    if (!d->m_account.institutionId().isEmpty()) {
      MyMoneyInstitution inst = MyMoneyFile::instance()->institution(d->m_account.institutionId());
      MyMoneyPayee payee = MyMoneyFile::instance()->payeeByName(inst.name());
      m_payeeEdit->setSelectedItem(payee.id());
    }
  } catch (MyMoneyException *e) {
    delete e;
  }
}

KEndingBalanceDlg::~KEndingBalanceDlg()
{
  delete d;
}

void KEndingBalanceDlg::slotUpdateBalances(void)
{
  MYMONEYTRACER(tracer);

  // determine the beginning balance and ending balance based on the following
  // forumulas:
  //
  // end balance   = current balance - sum(all non cleared transactions)
  //                                 - sum(all cleared transactions posted
  //                                        after statement date)
  // start balance = end balance - sum(all cleared transactions
  //                                        up to statement date)
  MyMoneyTransactionFilter filter(d->m_account.id());
  filter.addState(MyMoneyTransactionFilter::notReconciled);
  filter.addState(MyMoneyTransactionFilter::cleared);
  filter.setReportAllSplits(true);

  QList<QPair<MyMoneyTransaction, MyMoneySplit> > transactionList;
  QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;

  // retrieve the list from the engine
  MyMoneyFile::instance()->transactionList(transactionList, filter);

  MyMoneyMoney balance = MyMoneyFile::instance()->balance(d->m_account.id());
  MyMoneyMoney factor(1, 1);
  if (d->m_account.accountGroup() == MyMoneyAccount::Liability)
    factor = -factor;

  MyMoneyMoney endBalance, startBalance;
  balance = balance * factor;
  endBalance = startBalance = balance;

  tracer.printf("total balance = %s", qPrintable(endBalance.formatMoney("", 2)));

  for (it = transactionList.constBegin(); it != transactionList.constEnd(); ++it) {
    const MyMoneySplit& split = (*it).second;
    balance -= split.shares() * factor;
    if ((*it).first.postDate() > m_statementDate->date()) {
      tracer.printf("Reducing balances by %s because postdate of %s/%s(%s) is past statement date", qPrintable((split.shares() * factor).formatMoney("", 2)), qPrintable((*it).first.id()), qPrintable(split.id()), qPrintable((*it).first.postDate().toString(Qt::ISODate)));
      endBalance -= split.shares() * factor;
      startBalance -= split.shares() * factor;
    } else {
      switch (split.reconcileFlag()) {
      case MyMoneySplit::NotReconciled:
        tracer.printf("Reducing balances by %s because %s/%s(%s) is not reconciled", qPrintable((split.shares() * factor).formatMoney("", 2)), qPrintable((*it).first.id()), qPrintable(split.id()), qPrintable((*it).first.postDate().toString(Qt::ISODate)));
        endBalance -= split.shares() * factor;
        startBalance -= split.shares() * factor;
        break;
      case MyMoneySplit::Cleared:
        tracer.printf("Reducing start balance by %s because %s/%s(%s) is cleared", qPrintable((split.shares() * factor).formatMoney("", 2)), qPrintable((*it).first.id()), qPrintable(split.id()), qPrintable((*it).first.postDate().toString(Qt::ISODate)));
        startBalance -= split.shares() * factor;
        break;
      default:
        break;
      }
    }
  }
  m_previousBalance->setValue(startBalance);
  m_endingBalance->setValue(endBalance);
  tracer.printf("total balance = %s", qPrintable(endBalance.formatMoney("", 2)));
  tracer.printf("start balance = %s", qPrintable(startBalance.formatMoney("", 2)));

  m_interestDateEdit->setDate(m_statementDate->date());
  m_chargesDateEdit->setDate(m_statementDate->date());
}

void KEndingBalanceDlg::accept(void)
{
  if (createTransaction(d->m_tInterest, -1, m_interestEdit, m_interestCategoryEdit, m_interestDateEdit)
      && createTransaction(d->m_tCharges, 1, m_chargesEdit, m_chargesCategoryEdit, m_chargesDateEdit))
    KEndingBalanceDlgDecl::accept();
}

void KEndingBalanceDlg::slotCreateInterestCategory(const QString& txt, QString& id)
{
  createCategory(txt, id, MyMoneyFile::instance()->income());
}

void KEndingBalanceDlg::slotCreateChargesCategory(const QString& txt, QString& id)
{
  createCategory(txt, id, MyMoneyFile::instance()->expense());
}

void KEndingBalanceDlg::createCategory(const QString& txt, QString& id, const MyMoneyAccount& parent)
{
  MyMoneyAccount acc;
  acc.setName(txt);

  emit createCategory(acc, parent);

  id = acc.id();
}

const MyMoneyMoney KEndingBalanceDlg::endingBalance(void) const
{
  return adjustedReturnValue(m_endingBalance->value());
}

const MyMoneyMoney KEndingBalanceDlg::previousBalance(void) const
{
  return adjustedReturnValue(m_previousBalance->value());
}

const MyMoneyMoney KEndingBalanceDlg::adjustedReturnValue(const MyMoneyMoney& v) const
{
  return d->m_account.accountGroup() == MyMoneyAccount::Liability ? -v : v;
}

void KEndingBalanceDlg::slotReloadEditWidgets(void)
{
  QString payeeId, interestId, chargesId;

  // keep current selected items
  payeeId = m_payeeEdit->selectedItem();
  interestId = m_interestCategoryEdit->selectedItem();
  chargesId = m_chargesCategoryEdit->selectedItem();

  // load the payee and category widgets with data from the engine
  m_payeeEdit->loadPayees(MyMoneyFile::instance()->payeeList());

  // a user request to show all categories in both selectors due to a valid use case.
  AccountSet aSet;
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.load(m_interestCategoryEdit->selector());
  aSet.load(m_chargesCategoryEdit->selector());

  // reselect currently selected items
  if (!payeeId.isEmpty())
    m_payeeEdit->setSelectedItem(payeeId);
  if (!interestId.isEmpty())
    m_interestCategoryEdit->setSelectedItem(interestId);
  if (!chargesId.isEmpty())
    m_chargesCategoryEdit->setSelectedItem(chargesId);
}

void KEndingBalanceDlg::slotCheckPageFinished(void)
{
  nextButton()->setEnabled(true);
  finishButton()->setEnabled(true);

  if (currentPage() == m_interestChargeCheckings) {
    int cnt1, cnt2;
    cnt1 = !m_interestEdit->value().isZero() + !m_interestCategoryEdit->selectedItem().isEmpty();
    cnt2 = !m_chargesEdit->value().isZero() + !m_chargesCategoryEdit->selectedItem().isEmpty();
    if (cnt1 == 1 || cnt2 == 1) {
      finishButton()->setEnabled(false);
      nextButton()->setEnabled(false);
    }
  }
}

const MyMoneyTransaction KEndingBalanceDlg::interestTransaction(void)
{
  return d->m_tInterest;
}

const MyMoneyTransaction KEndingBalanceDlg::chargeTransaction(void)
{
  return d->m_tCharges;
}

bool KEndingBalanceDlg::createTransaction(MyMoneyTransaction &t, const int sign, kMyMoneyEdit *amountEdit, KMyMoneyCategory *categoryEdit, kMyMoneyDateInput* dateEdit)
{
  t = MyMoneyTransaction();

  if (!amountEdit->isValid() || categoryEdit->selectedItem().isEmpty() || !dateEdit->date().isValid())
    return true;

  MyMoneySplit s1, s2;
  MyMoneyMoney val = amountEdit->value() * MyMoneyMoney(sign, 1);
  try {
    t.setPostDate(dateEdit->date());
    t.setCommodity(d->m_account.currencyId());

    s1.setPayeeId(m_payeeEdit->selectedItem());
    s1.setReconcileFlag(MyMoneySplit::Cleared);
    s1.setAccountId(d->m_account.id());
    s1.setValue(-val);
    s1.setShares(-val);

    s2 = s1;
    s2.setAccountId(categoryEdit->selectedItem());
    s2.setValue(val);

    t.addSplit(s1);
    t.addSplit(s2);

    QMap<QString, MyMoneyMoney> priceInfo; // just empty
    MyMoneyMoney shares;
    if (!KCurrencyCalculator::setupSplitPrice(shares, t, s2, priceInfo, this)) {
      t = MyMoneyTransaction();
      return false;
    }

    s2.setShares(shares);
    t.modifySplit(s2);

  } catch (MyMoneyException *e) {
    qDebug("%s", qPrintable(e->what()));
    delete e;
    t = MyMoneyTransaction();
    return false;
  }

  return true;
}

void KEndingBalanceDlg::help(void)
{
  QString anchor = d->m_helpAnchor[currentPage()];
  if (anchor.isEmpty())
    anchor = QString("details.reconcile.whatis");

  KToolInvocation::invokeHelp(anchor);
}

KEndingBalanceLoanDlg::KEndingBalanceLoanDlg(const MyMoneyAccount& account, QWidget *parent) :
    KEndingBalanceDlgDecl(parent),
    d(new Private)
{
  setModal(true);
  d->m_account = account;
  QDate value = account.lastReconciliationDate();
  if (value.isValid())
    m_startDateEdit->setDate(value.addDays(1));
  else
    m_startDateEdit->setDate(d->m_account.openingDate());

  // make sure, we show the correct start page
  showPage(m_startPageLoan);

  // enable the finish button on the last page
  setAppropriate(m_checkPaymentsPage, true);

  // remove all unwanted pages
  removePage(m_startPageCheckings);
  removePage(m_statementInfoPageCheckings);
  removePage(m_pagePreviousPostpone);
  removePage(m_interestChargeCheckings);

  // connect the signals with the slots
  connect(m_amortizationTotalEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_interestTotalEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_accountEdit, SIGNAL(stateChanged(void)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_categoryEdit, SIGNAL(stateChanged(void)), this, SLOT(slotCheckPageFinished(void)));
}

KEndingBalanceLoanDlg::~KEndingBalanceLoanDlg()
{
}

void KEndingBalanceLoanDlg::slotCheckPageFinished(void)
{
  nextButton()->setEnabled(true);
  finishButton()->setEnabled(true);

  if (currentPage() == m_checkPaymentsPage) {
    MyMoneyMoney interest = totalInterest(m_startDateEdit->date(), m_endDateEdit->date());
    MyMoneyMoney amortization = totalAmortization(m_startDateEdit->date(), m_endDateEdit->date());

    if (interest == m_interestTotalEdit->value()
        && amortization == m_amortizationTotalEdit->value()) {
      if (indexOf(m_adjustmentTransactionPage) != -1) {
        removePage(m_adjustmentTransactionPage);
        // the following line forces to update the buttons
        showPage(m_checkPaymentsPage);
        nextButton()->setEnabled(true);
        finishButton()->setEnabled(true);
      }
    } else {
      if (indexOf(m_adjustmentTransactionPage) == -1) {
        addPage(m_adjustmentTransactionPage, i18n("Adjustment transaction"));
        // the following line forces to update the buttons
        showPage(m_checkPaymentsPage);
      }
    }
  } else if (currentPage() == m_adjustmentTransactionPage) {
    if (m_accountEdit->selectedItems().count() == 0) {
      nextButton()->setEnabled(false);
      finishButton()->setEnabled(false);

    } else if (m_categoryEdit->isEnabled()
               && m_categoryEdit->selectedItems().count() == 0) {
      nextButton()->setEnabled(false);
      finishButton()->setEnabled(false);
    }
  }
}

const MyMoneyMoney KEndingBalanceLoanDlg::totalInterest(const QDate& start, const QDate& end) const
{
  MyMoneyMoney  interest;
  MyMoneyTransactionFilter  filter(d->m_account.id());
  filter.setDateFilter(start, end);

  QList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(filter);
  QList<MyMoneyTransaction>::const_iterator it_t;

  for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
    QList<MyMoneySplit>::const_iterator it_s;
    for (it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      if ((*it_s).action() == MyMoneySplit::ActionInterest) {
        interest += (*it_s).value();
      }
    }
  }
  return interest;
}

const MyMoneyMoney KEndingBalanceLoanDlg::totalAmortization(const QDate& start, const QDate& end) const
{
  MyMoneyMoney  amortization;
  MyMoneyMoney  adjust(1, 1);
  MyMoneyTransactionFilter  filter(d->m_account.id());
  filter.setDateFilter(start, end);

  if (d->m_account.accountType() == MyMoneyAccount::AssetLoan)
    adjust = -adjust;

  QList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(filter);
  QList<MyMoneyTransaction>::const_iterator it_t;

  for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
    QList<MyMoneySplit>::const_iterator it_s;
    for (it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      if ((*it_s).accountId() == d->m_account.id()
          && (*it_s).action() == MyMoneySplit::ActionAmortization
          && ((*it_s).value() * MyMoneyMoney(adjust, 1)).isPositive()) {
        amortization += (*it_s).value();
      }
    }
  }
  // make sure to return a positive number
  return amortization * adjust;
}

void KEndingBalanceLoanDlg::next(void)
{
  bool dontLeavePage = false;

  if (currentPage() == m_startPageLoan) {
    MyMoneyMoney interest = totalInterest(m_startDateEdit->date(), m_endDateEdit->date());
    MyMoneyMoney amortization = totalAmortization(m_startDateEdit->date(), m_endDateEdit->date());

    m_loanOverview->setText(i18n("KMyMoney has calculated the following amounts for "
                                 "interest and amortization according to recorded payments "
                                 "between %1 and %2.", KGlobal::locale()->formatDate(m_startDateEdit->date()), KGlobal::locale()->formatDate(m_endDateEdit->date())));

    // preload widgets with calculated values if they are empty
    if (m_amortizationTotalEdit->value().isZero() && !amortization.isZero())
      m_amortizationTotalEdit->setValue(amortization);
    if (m_interestTotalEdit->value().isZero() && !interest.isZero())
      m_interestTotalEdit->setValue(interest);

  } else if (currentPage() == m_checkPaymentsPage) {
    AccountSet assetSet, incomeSet;
    assetSet.addAccountGroup(MyMoneyAccount::Asset);
    incomeSet.addAccountGroup(MyMoneyAccount::Income);
    assetSet.load(m_accountEdit);
    incomeSet.load(m_categoryEdit);
#if 0
    m_accountEdit->loadList(static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::asset | KMyMoneyUtils::liability));
    m_categoryEdit->loadList(static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::income | KMyMoneyUtils::expense));
#endif
    m_categoryEdit->setEnabled(false);

    MyMoneyMoney interest = totalInterest(m_startDateEdit->date(), m_endDateEdit->date());
    if (interest != m_interestTotalEdit->value()) {
      m_categoryEdit->setEnabled(true);
    }
  }

  if (!dontLeavePage)
    KEndingBalanceDlgDecl::next();

  slotCheckPageFinished();
}

const MyMoneyTransaction KEndingBalanceLoanDlg::adjustmentTransaction(void) const
{
  MyMoneyTransaction t;

  MyMoneyMoney interest = totalInterest(m_startDateEdit->date(), m_endDateEdit->date());
  MyMoneyMoney amortization = totalAmortization(m_startDateEdit->date(), m_endDateEdit->date());

  if (interest != m_interestTotalEdit->value()
      || amortization != m_amortizationTotalEdit->value()) {
    MyMoneySplit sAccount, sAmortization, sInterest;
    int          adjust = 1;

    if (d->m_account.accountType() == MyMoneyAccount::AssetLoan)
      adjust = -1;

    // fix sign if asset
    interest = interest * MyMoneyMoney(adjust, 1);
    amortization = amortization * MyMoneyMoney(adjust, 1);

    sAmortization.setValue((m_amortizationTotalEdit->value() - amortization) * MyMoneyMoney(adjust, 1));
    sInterest.setValue((m_interestTotalEdit->value() - interest) * MyMoneyMoney(adjust, 1));
    sAccount.setValue(-(sAmortization.value() + sInterest.value()));

    try {
      sAmortization.setAccountId(d->m_account.id());
      sAmortization.setPayeeId(d->m_account.payee());
      sAccount.setAccountId(m_accountEdit->selectedItems()[0]);
      sAccount.setPayeeId(d->m_account.payee());
      if (m_categoryEdit->isEnabled())
        sInterest.setAccountId(m_categoryEdit->selectedItems()[0]);

      sAccount.setMemo(i18n("Adjustment transaction"));
      sAmortization.setMemo(sAccount.memo());
      sInterest.setMemo(sAccount.memo());

      sAccount.setAction(MyMoneySplit::ActionAmortization);
      sAmortization.setAction(MyMoneySplit::ActionAmortization);
      sInterest.setAction(MyMoneySplit::ActionInterest);

      t.addSplit(sAccount);
      t.addSplit(sAmortization);
      if (!sInterest.value().isZero())
        t.addSplit(sInterest);

      t.setPostDate(m_endDateEdit->date());

    } catch (MyMoneyException *e) {
      qDebug("Unable to create adjustment transaction for loan reconciliation: %s", qPrintable(e->what()));
      delete e;
      return MyMoneyTransaction();
    }
  }
  return t;
}

void KEndingBalanceLoanDlg::help(void)
{
  QString anchor = d->m_helpAnchor[currentPage()];
  if (anchor.isEmpty())
    anchor = QString("details.reconcile.whatis");

  KToolInvocation::invokeHelp(anchor);
}

#include "kendingbalancedlg.moc"

