/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYXMLREADER_TEST_H
#define MYMONEYXMLREADER_TEST_H
// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyXmlReader;
class MyMoneyFile;

class MyMoneyXmlReaderTest : public QObject
{
    Q_OBJECT

protected:
    MyMoneyXmlReader* r;
    MyMoneyFile* m_file;

protected:
    QString createFile(const QString& data) const;
    void resetTest();
    QString createInstitutionData() const;
    QString createAccountsData() const;
    QString createAccountsAndCategoryData() const;

    QString createAccountData() const;
    QString createCategoryData() const;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void testReadFileInfo();
    void testReadUser();
    void testMissingId();
    void testEmptyId();
    void testReadInstitutions();
    void testReadPayees();
    void testReadCostCenters();
    void testReadTags();
    void testReadAccounts();
    void testReadTransactions();
    void testReadKeyValuePairs();
    void testReadSchedules();
    void testReadSecurities();
    void testReadCurrencies();
    void testReadPrices_data();
    void testReadPrices();
    void testReadBudgets();
    void testReadOnlineJobs();
};

#endif // MYMONEYXMLREADER_TEST_H
