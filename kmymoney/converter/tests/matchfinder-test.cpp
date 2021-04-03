/*
    KMyMoney transaction importing module - tests for ExistingTransactionMatchFinder

    SPDX-FileCopyrightText: 2012 Lukasz Maszczynski <lukasz@maszczynski.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "matchfinder-test.h"
#include "existingtransactionmatchfinder.h"

#include <QTest>

#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

QTEST_GUILESS_MAIN(MatchFinderTest)

MatchFinderTest::MatchFinderTest() :
    file(nullptr),
    matchResult(TransactionMatchFinder::MatchResult::MatchNotFound)
{

}

void MatchFinderTest::init()
{
    file = MyMoneyFile::instance();

    setupCurrency();
    setupAccounts();
    setupPayees();

    ledgerTransaction = buildDefaultTransaction();
    importTransaction = buildDefaultTransaction();
    existingTrFinder.reset(new ExistingTransactionMatchFinder(MATCH_WINDOW));

    m_schedule = buildNonOverdueSchedule();
    scheduledTrFinder.reset(new ScheduledTransactionMatchFinder(*account, MATCH_WINDOW));
}

void MatchFinderTest::cleanup()
{
}

void MatchFinderTest::setupStorage()
{
}

void MatchFinderTest::setupCurrency()
{
    MyMoneyFileTransaction ft;
    file->addCurrency(MyMoneySecurity("USD", "US Dollar", "$"));
    file->setBaseCurrency(file->currency("USD"));
    ft.commit();
}

void MatchFinderTest::setupPayees()
{
    payee.setName("John Doe");
    otherPayee.setName("Jane Doe");

    MyMoneyFileTransaction ft;
    file->addPayee(payee);
    file->addPayee(otherPayee);
    ft.commit();
}

void MatchFinderTest::setupAccounts()
{
    account.reset(new MyMoneyAccount);
    otherAccount.reset(new MyMoneyAccount);


    account->setName("Expenses account");
    account->setAccountType(eMyMoney::Account::Type::Expense);
    account->setOpeningDate(QDate(2012, 12, 01));
    account->setCurrencyId(MyMoneyFile::instance()->baseCurrency().id());

    otherAccount->setName("Some other account");
    otherAccount->setAccountType(eMyMoney::Account::Type::Expense);
    otherAccount->setOpeningDate(QDate(2012, 12, 01));
    otherAccount->setCurrencyId(MyMoneyFile::instance()->baseCurrency().id());

    MyMoneyFileTransaction ft;
    MyMoneyAccount parentAccount = file->expense();
    file->addAccount(*account, parentAccount);
    file->addAccount(*otherAccount, parentAccount);
    ft.commit();
}



MyMoneyTransaction MatchFinderTest::buildDefaultTransaction() const
{
    MyMoneySplit split;
    split.setAccountId(account->id());
    split.setBankID("#1");
    split.setPayeeId(payee.id());
    split.setShares(MyMoneyMoney(123.00));

    MyMoneyTransaction transaction;
    transaction.setPostDate(QDate(2012, 12, 5));
    transaction.setImported(true);
    transaction.addSplit(split);

    return transaction;
}

MyMoneyTransaction MatchFinderTest::buildMatchedTransaction(MyMoneyTransaction transaction) const
{
    transaction.clearId();
    MyMoneyTransaction matchingTransaction = transaction;
    transaction.splits().first().addMatch(matchingTransaction);

    return transaction;
}

QString MatchFinderTest::addTransactionToLedger(MyMoneyTransaction transaction) const
{
    MyMoneyFileTransaction ft;
    file->addTransaction(transaction);
    ft.commit();

    return transaction.id();
}



MyMoneySchedule MatchFinderTest::buildNonOverdueSchedule() const
{
    QDate tomorrow = QDate::currentDate().addDays(1);

    MyMoneyTransaction transaction = buildDefaultTransaction();
    transaction.setPostDate(tomorrow);

    MyMoneySchedule nonOverdueSchedule("schedule name", eMyMoney::Schedule::Type::Transfer, eMyMoney::Schedule::Occurrence::Monthly, 1, eMyMoney::Schedule::PaymentType::BankTransfer, tomorrow, tomorrow.addMonths(2), false, false);
    nonOverdueSchedule.setTransaction(transaction);

    return nonOverdueSchedule;
}

void MatchFinderTest::addSchedule(MyMoneySchedule schedule) const
{
    MyMoneyFileTransaction ft;
    file->addSchedule(schedule);
    ft.commit();
}



void MatchFinderTest::expectMatchWithExistingTransaction(TransactionMatchFinder::MatchResult expectedResult)
{
    matchResult = existingTrFinder->findMatch(importTransaction, importTransaction.splits().first());
    QCOMPARE(matchResult, expectedResult);
}

void MatchFinderTest::expectMatchWithScheduledTransaction(TransactionMatchFinder::MatchResult expectedResult)
{
    matchResult = scheduledTrFinder->findMatch(importTransaction, importTransaction.splits().first());
    QCOMPARE(matchResult, expectedResult);
}



void MatchFinderTest::testDuplicate_allMatch()
{
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchDuplicate);
}

void MatchFinderTest::testDuplicate_payeeEmpty()
{
    ledgerTransaction.splits().first().setPayeeId(payee.id());
    importTransaction.splits().first().setPayeeId("");
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchDuplicate);
}

void MatchFinderTest::testDuplicate_payeeMismatch()
{
    ledgerTransaction.splits().first().setPayeeId(payee.id());
    importTransaction.splits().first().setPayeeId(otherPayee.id());
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchDuplicate);
}

void MatchFinderTest::testDuplicate_splitIsMarkedAsMatched()
{
    ledgerTransaction = buildMatchedTransaction(ledgerTransaction);
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchDuplicate);
}



void MatchFinderTest::testPreciseMatch_noBankId()
{
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("");
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchPrecise);
}

void MatchFinderTest::testPreciseMatch_importBankId()
{
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("#1");
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchPrecise);
}

void MatchFinderTest::testPreciseMatch_payeeEmpty()
{
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setPayeeId("");
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchPrecise);
}



void MatchFinderTest::testImpreciseMatch_matchWindowLowerBound()
{

    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("");
    importTransaction.setPostDate(importTransaction.postDate().addDays(-MATCH_WINDOW));
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchImprecise);
}

void MatchFinderTest::testImpreciseMatch_matchWindowUpperBound()
{
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("");
    importTransaction.setPostDate(importTransaction.postDate().addDays(MATCH_WINDOW));
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchImprecise);
}

void MatchFinderTest::testImpreciseMatch_payeeEmpty()
{
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setPayeeId("");
    importTransaction.setPostDate(importTransaction.postDate().addDays(1));
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchImprecise);
}



void MatchFinderTest::testNoMatch_bankIdMismatch()
{
    ledgerTransaction.splits().first().setBankID("#1");
    importTransaction.splits().first().setBankID("#2");
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testNoMatch_ledgerBankIdNotEmpty()
{
    ledgerTransaction.splits().first().setBankID("#1");
    importTransaction.splits().first().setBankID("");
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testNoMatch_accountMismatch_withBankId()
{
    ledgerTransaction.splits().first().setAccountId(account->id());
    importTransaction.splits().first().setAccountId(otherAccount->id());
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testNoMatch_accountMismatch_noBankId()
{
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("");
    ledgerTransaction.splits().first().setAccountId(account->id());
    importTransaction.splits().first().setAccountId(otherAccount->id());
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testNoMatch_amountMismatch_withBankId()
{
    ledgerTransaction.splits().first().setShares(MyMoneyMoney(1.11));
    importTransaction.splits().first().setShares(MyMoneyMoney(9.99));
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testNoMatch_amountMismatch_noBankId()
{
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("");
    ledgerTransaction.splits().first().setShares(MyMoneyMoney(1.11));
    importTransaction.splits().first().setShares(MyMoneyMoney(9.99));
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testNoMatch_payeeMismatch()
{
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("");
    ledgerTransaction.splits().first().setPayeeId(payee.id());
    importTransaction.splits().first().setPayeeId(otherPayee.id());
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testNoMatch_splitIsMarkedAsMatched()
{
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("");
    ledgerTransaction = buildMatchedTransaction(ledgerTransaction);
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testNoMatch_postDateMismatch_withBankId()
{
    importTransaction.setPostDate(importTransaction.postDate().addDays(MATCH_WINDOW + 1));
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testNoMatch_postDateMismatch_noBankId()
{
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setBankID("");
    importTransaction.setPostDate(importTransaction.postDate().addDays(MATCH_WINDOW + 1));
    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}



void MatchFinderTest::testExistingTransactionMatch_sameTransactionId_withBankId()
{
    QString transactionId = addTransactionToLedger(ledgerTransaction);
    importTransaction = MyMoneyTransaction(transactionId, ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testExistingTransactionMatch_sameTransactionId_noBankId()
{
    ledgerTransaction.splits().first().setBankID("");
    QString transactionId = addTransactionToLedger(ledgerTransaction);
    importTransaction = MyMoneyTransaction(transactionId, ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}


void MatchFinderTest::testExistingTransactionMatch_multipleAccounts_withBankId()
{
    ledgerTransaction.splits().first().setAccountId(account->id());
    ledgerTransaction.splits().first().setBankID("#1");
    importTransaction.splits().first().setAccountId(otherAccount->id());
    importTransaction.splits().first().setBankID("#1");

    MyMoneySplit secondSplit = importTransaction.splits().first();
    secondSplit.clearId();
    secondSplit.setAccountId(account->id());
    secondSplit.setBankID("#1");
    importTransaction.addSplit(secondSplit);

    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}

void MatchFinderTest::testExistingTransactionMatch_multipleAccounts_noBankId()
{
    ledgerTransaction.splits().first().setAccountId(account->id());
    ledgerTransaction.splits().first().setBankID("");
    importTransaction.splits().first().setAccountId(otherAccount->id());
    importTransaction.splits().first().setBankID("");

    MyMoneySplit secondSplit = importTransaction.splits().first();
    secondSplit.clearId();
    secondSplit.setAccountId(account->id());
    secondSplit.setBankID("");
    importTransaction.addSplit(secondSplit);

    addTransactionToLedger(ledgerTransaction);

    expectMatchWithExistingTransaction(TransactionMatchFinder::MatchNotFound);
}


void MatchFinderTest::testScheduleMatch_allMatch()
{
    importTransaction.setPostDate(m_schedule.adjustedNextDueDate());
    addSchedule(m_schedule);

    expectMatchWithScheduledTransaction(TransactionMatchFinder::MatchPrecise);
    QCOMPARE(m_schedule.isOverdue(), false);
}

void MatchFinderTest::testScheduleMatch_dueDateWithinMatchWindow()
{
    QDate dateWithinMatchWindow = m_schedule.adjustedNextDueDate().addDays(MATCH_WINDOW);
    importTransaction.setPostDate(dateWithinMatchWindow);
    addSchedule(m_schedule);

    expectMatchWithScheduledTransaction(TransactionMatchFinder::MatchImprecise);
    QCOMPARE(m_schedule.isOverdue(), false);
}

void MatchFinderTest::testScheduleMatch_amountWithinAllowedVariation()
{
    double exactAmount = m_schedule.transaction().splits()[0].shares().toDouble();
    double amountWithinAllowedVariation = exactAmount * (100 + m_schedule.variation()) / 100;
    importTransaction.splits()[0].setShares(MyMoneyMoney(amountWithinAllowedVariation));
    importTransaction.setPostDate(m_schedule.adjustedNextDueDate());
    addSchedule(m_schedule);

    expectMatchWithScheduledTransaction(TransactionMatchFinder::MatchPrecise);
}

void MatchFinderTest::testScheduleMatch_overdue()
{
    m_schedule.setNextDueDate(QDate::currentDate().addDays(-MATCH_WINDOW - 1));
    importTransaction.setPostDate(QDate::currentDate());
    addSchedule(m_schedule);

    expectMatchWithScheduledTransaction(TransactionMatchFinder::MatchImprecise);
    QCOMPARE(m_schedule.isOverdue(), true);
}

void MatchFinderTest::testScheduleMismatch_dueDate()
{
    importTransaction.setPostDate(m_schedule.adjustedNextDueDate().addDays(MATCH_WINDOW + 1));
    addSchedule(m_schedule);

    expectMatchWithScheduledTransaction(TransactionMatchFinder::MatchNotFound);
    QCOMPARE(m_schedule.isOverdue(), false);
}

void MatchFinderTest::testScheduleMismatch_amount()
{
    double exactAmount = m_schedule.transaction().splits()[0].shares().toDouble();
    double mismatchedAmount = exactAmount * (110 + m_schedule.variation()) / 100;
    importTransaction.splits()[0].setShares(MyMoneyMoney(mismatchedAmount));
    importTransaction.setPostDate(m_schedule.adjustedNextDueDate());
    addSchedule(m_schedule);

    expectMatchWithScheduledTransaction(TransactionMatchFinder::MatchNotFound);
}
