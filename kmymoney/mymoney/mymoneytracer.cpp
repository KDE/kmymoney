/*
    SPDX-FileCopyrightText: 2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneytracer.h"

#include <iostream>

#include <cstdio>
#include <cstdarg>

#include <QRegExp>

class MyMoneyTracerPrivate
{
    Q_DISABLE_COPY(MyMoneyTracerPrivate)

public:
    MyMoneyTracerPrivate()
    {
    }

    QString m_className;
    QString m_memberName;

    static int m_indentLevel;
    static int m_onoff;
};

int MyMoneyTracerPrivate::m_indentLevel = 0;
int MyMoneyTracerPrivate::m_onoff = 0;

MyMoneyTracer::MyMoneyTracer(const char* name) :
    d_ptr(new MyMoneyTracerPrivate)
{
    Q_D(MyMoneyTracer);
    if (d->m_onoff) {
        QRegExp exp("(.*)::(.*)");
        if (exp.indexIn(name) != -1) {
            d->m_className = exp.cap(1);
            d->m_memberName = exp.cap(2);
        } else {
            d->m_className = QString(name);
            d->m_memberName.clear();
        }
        QString indent;
        indent.fill(' ', d->m_indentLevel);
        std::cerr << qPrintable(indent) << "ENTER: " << qPrintable(d->m_className) << "::" << qPrintable(d->m_memberName) << std::endl;
    }
    d->m_indentLevel += 2;
}

MyMoneyTracer::MyMoneyTracer(const QString& className, const QString& memberName) :
    d_ptr(new MyMoneyTracerPrivate)
{
    Q_D(MyMoneyTracer);
    d->m_className = className;
    d->m_memberName = memberName;
    if (d->m_onoff) {
        QString indent;
        indent.fill(' ', d->m_indentLevel);
        std::cerr << qPrintable(indent) << "ENTER: " << qPrintable(d->m_className) << "::" << qPrintable(d->m_memberName) << std::endl;
    }
    d->m_indentLevel += 2;
}

MyMoneyTracer::~MyMoneyTracer()
{
    Q_D(MyMoneyTracer);
    d->m_indentLevel -= 2;
    if (d->m_onoff) {
        QString indent;
        indent.fill(' ', d->m_indentLevel);
        std::cerr << qPrintable(indent) << "LEAVE: " << qPrintable(d->m_className) << "::" << qPrintable(d->m_memberName) << std::endl;
    }
    delete d;
}

void MyMoneyTracer::printf(const char *format, ...) const
{
    Q_D(const MyMoneyTracer);
    if (d->m_onoff) {
        va_list args;
        va_start(args, format);
        QString indent;
        indent.fill(' ', d->m_indentLevel);
        std::cerr << qPrintable(indent);

        vfprintf(stderr, format, args);
        putc('\n', stderr);
        va_end(args);
    }
}

void MyMoneyTracer::onOff(int onOff)
{
    MyMoneyTracerPrivate::m_onoff = onOff;
}

void MyMoneyTracer::on()
{
    MyMoneyTracerPrivate::m_onoff = 1;
}

void MyMoneyTracer::off()
{
    MyMoneyTracerPrivate::m_onoff = 0;
}
