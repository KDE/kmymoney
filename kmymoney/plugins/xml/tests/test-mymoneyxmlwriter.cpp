/*
    SPDX-FileCopyrightText: 2024 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test-mymoneyxmlwriter.h"

#include "accountsmodel.h"
#include "institutionsmodel.h"
#include "mymoneyaccount.h"
#include "mymoneyinstitution.h"
#include "mymoneypayee.h"
#include "mymoneysecurity.h"
#include "payeesmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QBuffer>
#include <QRegularExpression>
#include <QTest>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// std Includes

#include <iostream>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoneyxmlwriter.h"
#include "mymoneyfile.h"
#include "mymoneyutils.h"

#define KMMCOMPARE(actual, expected, _file, _line)                                                                                                             \
    do {                                                                                                                                                       \
        if (!QTest::qCompare(actual, expected, #actual, #expected, _file, _line)) {                                                                            \
            compareContent(actual, expected);                                                                                                                  \
            return;                                                                                                                                            \
        }                                                                                                                                                      \
    } while (false)

QTEST_GUILESS_MAIN(MyMoneyXmlWriterTest)

void MyMoneyXmlWriterTest::init()
{
    m_file = MyMoneyFile::instance();
    m_file->unload();
}

bool compareContent(const QByteArray& data1, const QByteArray& data2)
{
    QRegularExpression exp("\r?\n");
    QStringList a = QString(data1).split(exp);
    QStringList b = QString(data2).split(exp);
    int max = std::min(a.size(), b.size());
    bool result = a.size() == b.size();
    for (int i = 0; i < max; i++) {
        if (a.at(i) != b.at(i)) {
            result = false;
            std::cerr << "- " << a.at(i).toStdString() << std::endl;
            std::cerr << "+ " << b.at(i).toStdString() << std::endl;
        }
    }
    return result;
}

QByteArray replaceContent(const QByteArray& data, const QString& key, const QString& newContent)
{
    QStringList result;
    for (const auto& line : QString(data).split('\n')) {
        if (!line.contains(key))
            result.append(line);
        else if (line.contains("value")) {
            QRegularExpression exp("value=\".*\"");
            result.append(QString(line).replace(exp, QString("value=\"%1\"").arg(newContent)));
        } else if (line.contains("date=")) {
            QRegularExpression exp("date=\".*\"");
            result.append(QString(line).replace(exp, QString("date=\"%1\"").arg(newContent)));
        }
    }
    return result.join("\n").toLocal8Bit();
}

void _writeAndCompare(MyMoneyFile* file, const QString& filename, const char* _file, int _line)
{
    // read reference file
    const QString srcFile = QLatin1String(CMAKE_CURRENT_SOURCE_DIR) + "/" + filename;
    QString dstFile = QLatin1String(CMAKE_CURRENT_BINARY_DIR) + "/" + filename;

    QFile refFile(srcFile);
    QVERIFY(refFile.open(QIODevice::ReadOnly));
    QByteArray refData = refFile.readAll();

    MyMoneyXmlWriter writer;
    writer.setFile(file);
    QByteArray data;
    QBuffer buffer;
    buffer.setBuffer(&data);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(writer.write(&buffer));
    buffer.close();
#if defined(Q_OS_WIN32)
    data.replace("\r\n", "\n");
    refData.replace("\r\n", "\n");
#endif
    QFile outFile(dstFile);
    QVERIFY(outFile.open(QIODevice::WriteOnly));
    outFile.write(data);
    outFile.close();

    // patch lines containing dynamic data
    QStringList keys = QStringList() << "LastModificationDate"
                                     << "LAST_MODIFIED_DATE"
                                     << "APPVERSION"
                                     << "kmm-id";
    QByteArray data1 = data;
    QByteArray refData1 = refData;
    for (const auto& key : keys) {
        data1 = replaceContent(data1, key, QLatin1String("*"));
        refData1 = replaceContent(refData1, key, QLatin1String("*"));
    }

    KMMCOMPARE(refData1, data1, _file, _line);
}

#define writeAndCompare(a, b) _writeAndCompare(a, b, __FILE__, __LINE__)

void MyMoneyXmlWriterTest::testWriteFileInfo()
{
    MyMoneyFileTransaction ft;

    // store the user info
    m_file->setUser(MyMoneyPayee());

    // create and setup base currency
    m_file->addCurrency(MyMoneySecurity("EUR"));
    m_file->setBaseCurrency(MyMoneySecurity("EUR"));

    ft.commit();

    writeAndCompare(m_file, QLatin1String("testfile1.xml"));

    MyMoneyFileTransaction ft1;
    m_file->setFileFixVersion(1);
    ft1.commit();

    writeAndCompare(m_file, QLatin1String("testfile2.xml"));
}

void MyMoneyXmlWriterTest::testWriteSkrooge()
{
    MyMoneyFileTransaction ft;

    // store the user info
    m_file->setUser(MyMoneyPayee());

    // create and setup base currency
    m_file->addCurrency(MyMoneySecurity("EUR"));
    m_file->setBaseCurrency(MyMoneySecurity("EUR"));

    MyMoneyInstitution institution("1-bank");
    institution.setName("I000001");
    institution.addAccountId("2-account");

    QMap<QString, MyMoneyInstitution> institutionsMap;
    institutionsMap.insert(institution.id(), institution);

    m_file->institutionsModel()->load(institutionsMap);

    MyMoneyAccount account("2-account");
    account.setName("2-account");
    account.setLastModified(QDate(2026, 5, 13));
    account.setLastReconciliationDate(QDate(2026, 5, 13));
    account.setOpeningDate(QDate(1999, 1, 1));
    account.setAccountType(eMyMoney::Account::Type::CreditCard);
    account.setCurrencyId("EUR");
    account.setParentAccountId(m_file->liability().id());
    account.setInstitutionId("1-bank");
    account.setValue("lastNumberUsed", "10");
    account.setValue("lastStatementBalance", "-7/5");
    account.setValue("maxCreditAbsolute", "1000/1");

    QMap<QString, MyMoneyAccount> accountsMap;

    accountsMap.insert(m_file->asset().id(), m_file->asset());
    accountsMap.insert(m_file->liability().id(), m_file->liability());
    accountsMap.insert(m_file->income().id(), m_file->income());
    accountsMap.insert(m_file->expense().id(), m_file->expense());
    accountsMap.insert(m_file->equity().id(), m_file->equity());
    accountsMap.insert(account.id(), account);

    m_file->accountsModel()->load(accountsMap);

    QMap<QString, MyMoneyPayee> payeesMap;

    MyMoneyPayee payee("1-Payee");
    payee.setName("P000001");

    payeesMap.insert(payee.id(), payee);

    m_file->payeesModel()->load(payeesMap);

    ft.commit();

    writeAndCompare(m_file, QLatin1String("testfile3.xml"));
}
