/*
    SPDX-FileCopyrightText: 2024 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyxmlwriter-test.h"
#include "mymoneypayee.h"
#include "mymoneysecurity.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QBuffer>
#include <QTest>
#include <iostream>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoneyxmlwriter.h"
#include "mymoneyfile.h"
#include "mymoneyutils.h"
#include "parametersmodel.h"

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
