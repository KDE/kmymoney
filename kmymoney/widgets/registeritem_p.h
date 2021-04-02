/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REGISTERITEM_P_H
#define REGISTERITEM_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyMoney;
class QDate;
class QString;

namespace KMyMoneyRegister {
class Register;
}
namespace KMyMoneyRegister {
class RegisterItem;
}

using namespace KMyMoneyRegister;

namespace KMyMoneyRegister
{
class RegisterItemPrivate
{
public:
    RegisterItemPrivate() :
        m_parent(nullptr),
        m_prev(nullptr),
        m_next(nullptr),
        m_startRow(0),
        m_rowsRegister(1),
        m_rowsForm(1),
        m_alternate(false),
        m_needResize(false),
        m_visible(true)
    {
    }

    virtual ~RegisterItemPrivate()
    {
    }

    Register*                m_parent;
    RegisterItem*            m_prev;
    RegisterItem*            m_next;
    int                      m_startRow;
    int                      m_rowsRegister;
    int                      m_rowsForm;
    bool                     m_alternate;
    bool                     m_needResize;
    bool                     m_visible;

    static QDate             nullDate;
    static QString           nullString;
    static MyMoneyMoney      nullValue;

};
}

#endif
