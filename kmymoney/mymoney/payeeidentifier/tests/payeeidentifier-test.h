/*
    SPDX-FileCopyrightText: 2013-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAYEEIDENTIFIER_TEST_H
#define PAYEEIDENTIFIER_TEST_H

#include <QObject>

class payeeidentifier_test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void createAndDeleteEmptyIdent();
    void copyIdent();
    void moveIdent();
    void createTypedIdent();
};

#endif // PAYEEIDENTIFIER_TEST_H
