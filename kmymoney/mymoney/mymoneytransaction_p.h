/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2001 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYTRANSACTION_P_H
#define MYMONEYTRANSACTION_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QHash>
#include <QList>
#include <QSet>
#include <QString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneysplit.h"

using namespace eMyMoney;

class MyMoneyTransactionPrivate : public MyMoneyObjectPrivate
{
public:
    /**
      * This method returns the next id to be used for a split
      */
    QString nextSplitID()
    {
        int lastIdUsed(0);
        for (const auto& split : m_splits) {
            const int id = split.id().midRef(1).toInt();
            if (id > lastIdUsed) {
                lastIdUsed = id;
            }
        }

        return QStringLiteral("S%1").arg(lastIdUsed + 1, SPLIT_ID_SIZE, 10, QLatin1Char('0'));
    }

    void collectReferencedObjects() override
    {
        m_referencedObjects.insert(m_commodity);
        for (const auto& split : m_splits) {
            m_referencedObjects.unite(split.referencedObjects());
        }
    }

    static const int SPLIT_ID_SIZE = 4;
    /** constants for unique sort key */
    static const int YEAR_SIZE = 4;
    static const int MONTH_SIZE = 2;
    static const int DAY_SIZE = 2;

    /**
      * This member contains the date when the transaction was entered
      * into the engine
      */
    QDate m_entryDate;

    /**
      * This member contains the date the transaction was posted
      */
    QDate m_postDate;

    /**
      * This member keeps the memo text associated with this transaction
      */
    QString m_memo;

    /**
      * This member contains the splits for this transaction
      */
    QList<MyMoneySplit> m_splits;

    /**
      * This member keeps the base commodity (e.g. currency) for this transaction
      */
    QString  m_commodity;

    /**
      * This member keeps the bank's unique ID for the transaction, so we can
      * avoid duplicates.  This is only used for electronic statement downloads.
      *
      * Note this is now deprecated!  Bank ID's should be set on splits, not transactions.
      */
    QString m_bankID;

};

#endif
