/*
    SPDX-FileCopyrightText: 2026 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KLocalizedString>
#include <QFile>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>

#include "mymoneyfile.h"
#include "mymoneyqifreader.h"
#include "mymoneysecurity.h"
#include "mymoneystatement.h"

class MyMoneyQifReaderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void importsStatementBalance();
};

void MyMoneyQifReaderTest::initTestCase()
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kmymoney"));

    auto file = MyMoneyFile::instance();
    file->unload();

    MyMoneyFileTransaction ft;
    file->addCurrency(MyMoneySecurity("USD", "US Dollar", "$"));
    file->setBaseCurrency(file->currency("USD"));
    ft.commit();
}

void MyMoneyQifReaderTest::importsStatementBalance()
{
    MyMoneyQifReader reader;

    reader.setInputDateFormat("%d/%m/%yyyy");
    reader.setURL(QUrl::fromLocalFile(QFINDTESTDATA("data/bug-337405.qif")));

    QSignalSpy statementsSpy(&reader, &MyMoneyQifReader::statementsReady);

    QVERIFY(statementsSpy.isValid());
    QVERIFY(reader.startImport());

    QTRY_COMPARE(statementsSpy.count(), 1);

    const auto statements = statementsSpy.at(0).at(0).value<QList<MyMoneyStatement>>();

    QCOMPARE(statements.count(), 1);

    const auto& statement = statements.constFirst();

    qDebug() << "balance:" << statement.m_closingBalance.toString();

    QCOMPARE(statement.m_closingBalance, MyMoneyMoney(123456, 100));

    QCOMPARE(statement.m_dateEnd, QDate(2025, 12, 31));
}

QTEST_MAIN(MyMoneyQifReaderTest)

#include "test-mymoneyqifreader.moc"
