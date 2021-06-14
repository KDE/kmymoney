/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WOOBEXC_H
#define WOOBEXC_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QException>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

enum class ExceptionCode {
    BrowserIncorrectPassword,
};

class WoobException : public QException
{
public:
    explicit WoobException(ExceptionCode ec)
        : m_exceptionCode(ec)
    {
    }
    ExceptionCode msg() const
    {
        return m_exceptionCode;
    }
    void raise() const
    {
        throw *this;
    }
    WoobException* clone() const
    {
        return new WoobException(*this);
    }
    ExceptionCode m_exceptionCode;
};
#endif
