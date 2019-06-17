/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#include "amountvalidator-test.h"

#include <QtTest>
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


void AmountValidatorTest::setLocale(const QString& name)
{
    currentLocale = name;
}

void AmountValidatorTest::addAcceptableNumber(const QString& testCaseName, const QString& number)
{
    const int result = static_cast<int>(QValidator::Acceptable);
    QString fixedNumber(number);
    // replace a blank in the test string with a non-breakable-space
    fixedNumber.replace(" ", "\u00A0");
    QTest::newRow((QString("%1:%2").arg(currentLocale, testCaseName)).toUtf8()) << currentLocale << fixedNumber << result;
}

void AmountValidatorTest::addInvalidNumber(const QString& testCaseName, const QString& number)
{
    const int result = static_cast<int>(QValidator::Invalid);
    QString fixedNumber(number);
    // replace a blank in the test string with a non-breakable-space
    fixedNumber.replace(" ", "\u00A0");
    QTest::newRow((QString("%1:%2").arg(currentLocale, testCaseName)).toUtf8()) << currentLocale << fixedNumber << result;
}

void AmountValidatorTest::testValidator_data()
{
    QTest::addColumn<QString>("locale");
    QTest::addColumn<QString>("inputValue");
    QTest::addColumn<int>("result");

    setLocale("en_US");
    addAcceptableNumber("amount w/ thousand and decimal", "1,123.00");
    addAcceptableNumber("amount no thousand but decimal", "1123.00");
    addAcceptableNumber("neg. amount no thousand but decimal", "-1123.00");
    addAcceptableNumber("amount no thousand no decimal", "1123");
    addAcceptableNumber("amount only fraction", ".00");

    addInvalidNumber("amount w/ thousand and decimal reversed", "1.123,00");
    addInvalidNumber("neg. amount w/ thousand and decimal reversed", "1.123,00-");
    addInvalidNumber("amount only fraction reversed", ",00");

    setLocale("sv_FI");
    addAcceptableNumber("amount w/ thousand and decimal", "1 123,00");
    addAcceptableNumber("neg. amount w/ thousand and decimal", "-1 123,00");
    addAcceptableNumber("amount no thousand but decimal", "1123,00");
    addAcceptableNumber("amount no thousand no decimal", "1123");
    addAcceptableNumber("amount only fraction", ",00");

    setLocale("de_DE");
    addAcceptableNumber("amount w/ thousand and decimal", "1.123,00");
    addAcceptableNumber("amount no thousand but decimal", "1123,00");
    addAcceptableNumber("amount no thousand no decimal", "1123");
    addAcceptableNumber("amount only fraction", ",00");

    addInvalidNumber("amount w/ thousand and decimal reversed", "1,123.00");
    addInvalidNumber("amount only fraction reversed", ".00");

    setLocale("es_PE");
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
    QFETCH(int, result);

    QLocale::setDefault(locale);
    AmountValidator m(nullptr);
    int pos = inputValue.length();
    // QCOMPARE(m->validate(inputValue, pos), static_cast<QValidator::State>(result));
    QCOMPARE((int)m.validate(inputValue, pos), result);
}
