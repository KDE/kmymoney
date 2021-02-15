/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AMOUNTVALIDATORTEST_H
#define AMOUNTVALIDATORTEST_H

#include <QObject>
#include <QLocale>
#include <QString>

class AmountValidator;

class AmountValidatorTest : public QObject
{
    Q_OBJECT

protected:
    QLocale         defaultLocale;
    QString         currentLocale;
    QChar           currentDecimalPoint;
    QChar           currentGroupSeparator;

private Q_SLOTS:
    void init();
    void cleanup();
    void testValidator_data();
    void testValidator();

private:
    void setLocale(const QString& name, const QChar& decimal , const QChar& group);
    void addAcceptableNumber(const QString& testCaseName, const QString& number);
    void addInvalidNumber(const QString& testCaseName, const QString& number);
};

#endif
