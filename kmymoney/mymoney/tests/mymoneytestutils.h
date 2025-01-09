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

class MyMoneyHideDebugTestBase : public MyMoneyTestBase
{
public:
    MyMoneyHideDebugTestBase()
        : MyMoneyTestBase()
    {
        oldHandler = qInstallMessageHandler(hideDebugMessages);
    }

    ~MyMoneyHideDebugTestBase()
    {
        qInstallMessageHandler(oldHandler);
    }

protected:
    static QtMessageHandler oldHandler;
    static void hideDebugMessages(QtMsgType type, const QMessageLogContext& context, const QString& msg)
    {
        if (type != QtDebugMsg && oldHandler) {
            oldHandler(type, context, msg);
        }
    }
};

#endif // MYMONEYTESTUTILS_H
