/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYANONWRITER_H
#define MYMONEYANONWRITER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyxmlwriter.h"

class MyMoneyAnonWriterPrivate;

/**
 * This class provides storage of an anonymized version of the current
 * file.  Any object with an ID (account, transaction, etc) is renamed
 * with that ID.  Any other string value the user typed in is replaced with
 * x's equal in length to the original string.  Any numeric value is
 * replaced with an arbitrary number which matches the sign of the original.
 *
 * The purpose of this class is to give users a way to send a developer
 * their file  without compromising their financial data.  If a user
 * encounters an error, they should try saving the anonymous version of the
 * file and see if the error is still there.  If so, they should notify the
 * list of the problem, and then when requested, send the anonymous file
 * privately to the developer who takes the problem.  I still don't think
 * it's wise to post the file to the public list...maybe I'm just paranoid.
 */
class MyMoneyAnonWriter : public MyMoneyXmlWriter
{
    Q_DISABLE_COPY(MyMoneyAnonWriter)
    Q_DECLARE_PRIVATE(MyMoneyAnonWriter)

    KMM_MYMONEY_UNIT_TESTABLE

public:
    MyMoneyAnonWriter();
};

#endif // MYMONEYXMLWRITER_H
