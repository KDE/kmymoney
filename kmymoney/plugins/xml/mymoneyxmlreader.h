/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYXMLREADER_H
#define MYMONEYXMLREADER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyunittestable.h"

class MyMoneyFile;
class MyMoneyXmlReaderPrivate;
class MyMoneyXmlReader
{
    Q_DISABLE_COPY(MyMoneyXmlReader)
    Q_DECLARE_PRIVATE(MyMoneyXmlReader)

    KMM_MYMONEY_UNIT_TESTABLE

public:
    MyMoneyXmlReader();
    ~MyMoneyXmlReader();

    /**
     * Setup the global @a file object
     */
    void setFile(MyMoneyFile* file);

    /**
     * Read the KMyMoney XML based data from the @a device.
     *
     * @param device pointer to QIODevice to read from
     * @returns true if no error occurred
     * @returns false in case of an error while reading
     *
     * @note Must only be called after setFile() was called with valid pointer
     */
    bool read(QIODevice* device);

    /**
     * In case read() returns @c false the next call will return
     * the error message that explains what caused the error.
     */
    QString errorString() const;

protected:
    /**
     * This is an internal entry point for testcases. It resets
     * the reader to its initial value and call the internal
     * readKMyMoney() routine.
     *
     * @param text text of the section to test
     * @returns true if no error occurred
     * @returns false in case of an error while reading
     */
    bool read(const QString& text);

private:
    MyMoneyXmlReaderPrivate* d_ptr;
};

#endif // MYMONEYXMLREADER_H
