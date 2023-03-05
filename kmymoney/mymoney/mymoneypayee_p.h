/*
    SPDX-FileCopyrightText: 2000-2001 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYPAYEE_P_H
#define MYMONEYPAYEE_P_H


// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QMap>
#include <QHash>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"

class MyMoneyPayeePrivate : public MyMoneyObjectPrivate
{
public:

    MyMoneyPayeePrivate() :
        m_matchingEnabled(false),
        m_usingMatchKey(false),
        m_matchKeyIgnoreCase(true)
    {
    }

    void collectReferencedObjects() override
    {
        m_referencedObjects = {m_defaultAccountId};
    }

    // Simple fields
    QString m_name;
    QString m_address;
    QString m_city;
    QString m_state;
    QString m_postcode;
    QString m_telephone;
    QString m_email;
    QString m_notes;

    // Transaction matching fields
    bool m_matchingEnabled;      //< Whether this payee should be matched at all
    bool m_usingMatchKey;        //< If so, whether a m_matchKey list is used (true), or just m_name is used (false)
    bool m_matchKeyIgnoreCase;   //< Whether to ignore the case of the match key or name

    /**
     * Semicolon separated list of matching keys used when trying to find a suitable
     * payee for imported transactions.
     */
    QString m_matchKey;

    // Category (account) matching fields
    QString m_defaultAccountId;

    /**
      * This member keeps a reference to an external database
      * (e.g. kaddressbook). It is the responsibility of the
      * application to format the reference string
      * (e.g. encoding the name of the external database into the
      * reference string).
      * If no external database is available it should be kept
      * empty by the application.
      */
    QString m_reference;

};

#endif

// vim:cin:si:ai:et:ts=2:sw=2:
