/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
 * SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
 * (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
 */

#ifndef WEBOOBEXC_H
#define WEBOOBEXC_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QException>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

enum class ExceptionCode {
  BrowserIncorrectPassword
};

class WeboobException : public QException
{
public:
    explicit WeboobException(ExceptionCode ec) : m_exceptionCode(ec) {}
    ExceptionCode msg() const { return m_exceptionCode; }
    void raise() const { throw *this; }
    WeboobException *clone() const { return new WeboobException(*this); }
    ExceptionCode m_exceptionCode;
};
#endif
