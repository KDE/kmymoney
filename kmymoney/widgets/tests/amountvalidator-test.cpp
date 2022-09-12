/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "amountvalidator-test.h"

#include <QTest>
#include <QValidator>

#include <config-kmymoney.h>
#include "amountvalidator.h"

QTEST_GUILESS_MAIN(AmountValidatorTest)

void AmountValidatorTest::init()
{
    defaultLocale = QLocale();
}

void AmountValidatorTest::cleanup()
{
    QLocale::setDefault(defaultLocale);
}


void AmountValidatorTest::setLocale(const QString& name, const QChar& decimal, const QChar& group)
{
    currentLocale = name;
    currentDecimalPoint = decimal;
    currentGroupSeparator = group;
}

void AmountValidatorTest::addAcceptableNumber(const QString& testCaseName, const QString& number)
{
    const int result = static_cast<int>(QValidator::Acceptable);
    QTest::newRow((QString("%1:%2").arg(currentLocale, testCaseName)).toUtf8()) << currentLocale << number << currentDecimalPoint << currentGroupSeparator << result;
}

void AmountValidatorTest::addInvalidNumber(const QString& testCaseName, const QString& number)
{
    const int result = static_cast<int>(QValidator::Invalid);
    QTest::newRow((QString("%1:%2").arg(currentLocale, testCaseName)).toUtf8()) << currentLocale << number << currentDecimalPoint << currentGroupSeparator << result;
}

void AmountValidatorTest::testValidator_data()
{
    QTest::addColumn<QString>("locale");
    QTest::addColumn<QString>("inputValue");
    QTest::addColumn<QChar>("decimalPoint");
    QTest::addColumn<QChar>("groupSeparator");
    QTest::addColumn<int>("result");

    setLocale("en_US", QLatin1Char('.'), QLatin1Char(','));
    addAcceptableNumber("amount w/ thousand and decimal", "1,123.00");
    addAcceptableNumber("amount no thousand but decimal", "1123.00");
    addAcceptableNumber("neg. amount no thousand but decimal", "-1123.00");
    addAcceptableNumber("amount no thousand no decimal", "1123");
    addAcceptableNumber("amount only fraction", ".00");

    addInvalidNumber("amount w/ thousand and decimal reversed", "1.123,00");
    addInvalidNumber("neg. amount w/ thousand and decimal reversed", "1.123,00-");
    addInvalidNumber("amount only fraction reversed", ",00");

    setLocale("sv_FI", QLatin1Char(','), QChar('\xA0'));
    addAcceptableNumber("amount w/ thousand and decimal", "1 123,00");
    addAcceptableNumber("neg. amount w/ thousand and decimal", "-1 123,00");
    addAcceptableNumber("amount no thousand but decimal", "1123,00");
    addAcceptableNumber("amount no thousand no decimal", "1123");
    addAcceptableNumber("amount only fraction", ",00");

    setLocale("de_DE", QLatin1Char(','), QLatin1Char('.'));
    addAcceptableNumber("amount w/ thousand and decimal", "1.123,00");
    addAcceptableNumber("amount no thousand but decimal", "1123,00");
    addAcceptableNumber("amount no thousand no decimal", "1123");
    addAcceptableNumber("amount only fraction", ",00");

    addInvalidNumber("amount w/ thousand and decimal reversed", "1,123.00");
    addInvalidNumber("amount only fraction reversed", ".00");

    setLocale("es_PE", QLatin1Char('.'), QLatin1Char(','));
    addAcceptableNumber("amount w/ thousand and decimal", "1,123.00");
    addAcceptableNumber("amount no thousand but decimal", "1123.00");
    addAcceptableNumber("amount no thousand no decimal", "1123");
    addAcceptableNumber("amount only fraction", ".00");

    addInvalidNumber("amount w/ thousand and decimal reversed", "1.123,00");
    addInvalidNumber("amount only fraction reversed", ",00");
}

void AmountValidatorTest::testValidator()
{
    QFETCH(QString, locale);
    QFETCH(QString, inputValue);
    QFETCH(QChar, decimalPoint);
    QFETCH(QChar, groupSeparator);
    QFETCH(int, result);

    QLocale::setDefault(QLocale(locale));
    if (QLocale().decimalPoint() != decimalPoint) {
        const QString msg = QStringLiteral("Locale %1 does not seem to be loaded correctly: decimal point is %2 and should be  %3").arg(locale, QLocale().decimalPoint(), decimalPoint);

        QSKIP(msg.toLatin1());
    }
    if (QLocale().groupSeparator() != groupSeparator) {
        QString msg = QStringLiteral("Locale %1 does not seem to be loaded correctly: group separator is %2 and should be  %3").arg(locale, QLocale().groupSeparator(), groupSeparator);
        QSKIP(msg.toLatin1());
    }
    AmountValidator m(nullptr);
    int pos = inputValue.length();
    // QCOMPARE(m->validate(inputValue, pos), static_cast<QValidator::State>(result));
    QCOMPARE((int)m.validate(inputValue, pos), result);
}
