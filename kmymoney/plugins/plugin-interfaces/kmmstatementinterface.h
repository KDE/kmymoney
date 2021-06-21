/*
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMSTATEMENTINTERFACE_H
#define KMMSTATEMENTINTERFACE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;
class MyMoneyKeyValueContainer;

#include "statementinterface.h"

namespace KMyMoneyPlugin {

/**
 * This class represents the implementation of the
 * StatementInterface.
 */
class KMMStatementInterface : public StatementInterface
{
    Q_OBJECT

public:
    explicit KMMStatementInterface(QObject* parent, const char* name = 0);
    ~KMMStatementInterface()
    {
    }

    virtual void resetMessages() const final override;
    virtual void showMessages(int statementCount) const final override;

    /**
     * This method imports a MyMoneyStatement into the engine
     */
    QStringList import(const MyMoneyStatement& s, bool silent = false) final override;

    /**
     * This method returns the account for a given @a key - @a value pair.
     * If the account is not found in the list of accounts, MyMoneyAccount()
     * is returned. The @a key - @a value pair can be in the account's kvp
     * container or the account's online settings kvp container.
     */
    MyMoneyAccount account(const QString& key, const QString& value) const final override;

    /**
     * This method stores the online parameters in @a kvps used by the plugin
     * with the account @a acc.
     */
    void setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps) const final override;
};

} // namespace
#endif
