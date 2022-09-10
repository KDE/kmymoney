/*
    SPDX-FileCopyrightText: 2018 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-FileCopyrightText: 2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYTRANSACTIONFILTERTEST_H
#define MYMONEYTRANSACTIONFILTERTEST_H

#include <QObject>
class MyMoneyFile;

class MyMoneyTransactionFilterTest : public QObject
{
    Q_OBJECT
public:
    MyMoneyTransactionFilterTest();

private Q_SLOTS:
    void init();
    void cleanup();
    void testMatchAmount();
    void testMatchText();
    void testMatchSplit();
    void testMatchTransactionAll();
    void testMatchTransactionAccount();
    void testMatchTransactionCategory();
    void testMatchTransactionDate();
    void testMatchTransactionNumber();
    void testMatchTransactionPayee();
    void testMatchTransactionState();
    void testMatchTransactionTag();
    void testMatchTransactionTypeAllTypes();
    void testMatchTransactionTypeDeposits();
    void testMatchTransactionTypePayments();
    void testMatchTransactionTypeTransfers();
    void testMatchTransactionValidity();
private:
    QString payeeId;
    QList<QString> tagIdList;
    QString acCheckingId;
    QString acExpenseId;
    QString acIncomeId;
    MyMoneyFile* file;
};

#endif
