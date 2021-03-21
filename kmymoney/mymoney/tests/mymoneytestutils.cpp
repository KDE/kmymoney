/*
    SPDX-FileCopyrightText: 2014-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyutils.h"

#include "mymoneyexception.h"

QString unexpectedExceptionString(const MyMoneyException &e)
{
    return QString("Unexpected exception: %1").arg(e.what());
}

