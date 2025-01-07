/*
    SPDX-FileCopyrightText: 2014-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYTESTUTILS_H
#define MYMONEYTESTUTILS_H

#include <KLocalizedString>

class MyMoneyException;

#define unexpectedException(e) QFAIL(qPrintable(unexpectedExceptionString(e)));

QString unexpectedExceptionString(const MyMoneyException &e);

class MyMoneyTestBase
{
public:
    MyMoneyTestBase()
    {
        KLocalizedString::setApplicationDomain(QByteArrayLiteral("kmymoney"));
    }
};

#endif // MYMONEYTESTUTILS_H
