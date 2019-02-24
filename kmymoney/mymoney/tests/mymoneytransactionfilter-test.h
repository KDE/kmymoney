/*
 * Copyright 2018  Ralf Habacker <ralf.habacker@freenet.de>
 * Copyright 2018  Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MYMONEYTRANSACTIONFILTERTEST_H
#define MYMONEYTRANSACTIONFILTERTEST_H

#include <QObject>
class MyMoneyStorageMgr;
class MyMoneyFile;

class MyMoneyTransactionFilterTest : public QObject
{
    Q_OBJECT
public:
    MyMoneyTransactionFilterTest();

private slots:
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
    MyMoneyStorageMgr* storage;
    MyMoneyFile* file;
};

#endif
