/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYXMLWRITER_H
#define MYMONEYXMLWRITER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyunittestable.h"

class MyMoneyFile;
class MyMoneyXmlWriterPrivate;
class MyMoneyXmlWriter
{
    Q_DISABLE_COPY(MyMoneyXmlWriter)
    Q_DECLARE_PRIVATE(MyMoneyXmlWriter)

    KMM_MYMONEY_UNIT_TESTABLE

public:
    MyMoneyXmlWriter();
    ~MyMoneyXmlWriter();

    /**
     * Setup the global @a file object
     */
    void setFile(MyMoneyFile* file);

    /**
     * Writes the current data to an XML based format to the @a device.
     *
     * @param device pointer to QIODevice to write to
     * @returns true if no error occurred
     * @returns false in case of an error while reading
     *
     * @note Must only be called after setFile() was called with valid pointer
     */
    bool write(QIODevice* device);

    /**
     * In case read() returns @c false the next call will return
     * the error message that explains what caused the error.
     */
    QString errorString() const;

protected:
    MyMoneyXmlWriter(MyMoneyXmlWriterPrivate* dd);

    MyMoneyXmlWriterPrivate* d_ptr;
};

#endif // MYMONEYXMLWRITER_H
