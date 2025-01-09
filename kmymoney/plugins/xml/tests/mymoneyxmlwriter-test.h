/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYXMLWRITER_TEST_H
#define MYMONEYXMLWRITER_TEST_H
// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyXmlWriterTest;

#include "mymoneytestutils.h"

class MyMoneyFile;

class MyMoneyXmlWriterTest : public QObject, public MyMoneyHideDebugTestBase
{
    Q_OBJECT

protected:
    MyMoneyFile* m_file;

protected:
    QString createFile(const QString& data);

private Q_SLOTS:
    void init();
    void testWriteFileInfo();
};

#endif // MYMONEYXMLWRITER_TEST_H
