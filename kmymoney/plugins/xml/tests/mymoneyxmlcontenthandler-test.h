/*
    SPDX-FileCopyrightText: 2002-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2014 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2016 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYXMLCONTENTHANDLERTEST_H
#define MYMONEYXMLCONTENTHANDLERTEST_H

#include <QObject>

class MyMoneyXmlContentHandlerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void readMyMoneyObject();
    void readKeyValueContainer();
    void writeKeyValueContainer();
    void readTransaction();
    void readTransactionEx();
    void writeTransaction();
    void readSplit();
    void writeSplit();
    void testReplaceIDinSplit();
    void readAccount();
    void writeAccount();
    void readWritePayee();
    void readWriteTag();
    void readInstitution();
    void writeInstitution();
    void readSchedule();
    void writeSchedule();
    void testOverdue();
    void testNextPayment();
    void testNextPaymentOnLastDayOfMonth();
    void testPaymentDates();
    void testHasReferenceTo();
    void testPaidEarlyOneTime();
    void testReplaceId();

private:
    void setupAccounts();
};

#endif
