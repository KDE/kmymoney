/***************************************************************************
                          mymoneytransactionfiltertest.h
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYTRANSACTIONFILTERTEST_H
#define MYMONEYTRANSACTIONFILTERTEST_H

#include <QObject>

class MyMoneyTransactionFilterTest : public QObject
{
  Q_OBJECT
private slots:
    void initTestCase();
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
};

#endif
