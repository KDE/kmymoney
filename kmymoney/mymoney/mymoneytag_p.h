/*
    SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYTAG_P_H
#define MYMONEYTAG_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QHash>
#include <QString>
#include <QColor>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"

class MyMoneyTagPrivate : public MyMoneyObjectPrivate
{
public:

    MyMoneyTagPrivate() :
        m_closed(false),
        m_tag_color(QColor("black"))
    {
    }

    MyMoneyTagPrivate(const MyMoneyTagPrivate& d) :
        m_name(d.m_name),
        m_closed(d.m_closed),
        m_tag_color(d.m_tag_color),
        m_notes(d.m_notes)
    {
    }

    // Simple fields
    QString m_name;
    // Closed tags will not be shown in the selector inside a transaction, only in the Tag tab
    bool m_closed;
    // Set the color showed in the ledger
    QColor m_tag_color;
    QString m_notes;
};

#endif
