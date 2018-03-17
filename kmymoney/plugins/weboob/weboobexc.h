/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
 * Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
 * (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
