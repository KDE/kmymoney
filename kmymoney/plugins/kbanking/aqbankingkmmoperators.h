/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2013-2015 Christian Dávid <christian-david@web.de>
 *
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

/**
 * @file Helper functions for using aqbanking with KMyMoney
 *
 * These functions are similar to the ones in aqbanking. They are meant as glue between aqbanking and KMyMoney.
 *
 */
#ifndef AQBANKINGKMMOPERATORS_H
#define AQBANKINGKMMOPERATORS_H

#include <QtCore/QSharedPointer>

#include "onlinetasks/interfaces/tasks/ionlinetasksettings.h"
#include "onlinetasks/sepa/tasks/sepaonlinetransfer.h"

class AB_ACCOUNT_SPEC;
class AB_TRANSACTION_LIMITS;
class AB_TRANSACTION;
class AB_VALUE;

namespace payeeIdentifiers
{
class ibanBic;
class nationalAccount;
}

/**
 * @brief AB_TransactionLimits_toSepaOnlineTaskSettings
 * @param aqlimits IN
 */
QSharedPointer<sepaOnlineTransfer::settings> AB_TransactionLimits_toSepaOnlineTaskSettings(const AB_TRANSACTION_LIMITS* aqlimits);

/**
 * @brief AB_Transaction_SetRemoteAccount
 * @param transaction
 * @param ident
 */
void AB_Transaction_SetRemoteAccount(AB_TRANSACTION* transaction, const payeeIdentifiers::ibanBic& ident);

void AB_Transaction_SetRemoteAccount(AB_TRANSACTION* transaction, const payeeIdentifiers::nationalAccount& ident);

/**
 * @brief Set local account of transaction by aqBanking account ptr
 */
void AB_Transaction_SetLocalAccount(AB_TRANSACTION* transaction, const AB_ACCOUNT_SPEC* account);

/**
 * @brief AB_Transaction_SetLocalAccount
 * @param transaction
 * @param ident
 */
void AB_Transaction_SetLocalAccount(AB_TRANSACTION* transaction, const payeeIdentifiers::nationalAccount& ident);

/**
 * @brief Set local account of transaction from list
 *
 * Will check if an element of accountNumbers is valid and if it is payeeIdentifiers::ibanBic or payeeIdentifiers::natinalAccount.
 * If such a payeeIdentifier is found, it is set as local account for @c transaction
 *
 * @return true if a valid payeeIdentifiers::natinalAccount was set
 */
bool AB_Transaction_SetLocalAccount(AB_TRANSACTION* transaction, const QList<payeeIdentifier>& accountNumbers);

/**
 * @brief Create AB_VALUE from MyMoneyMoney
 *
 * @return caller gains ownership
 */
AB_VALUE* AB_Value_fromMyMoneyMoney(const MyMoneyMoney& input);

/**
 * @brief Convert AB_VALUE to MyMoneyMoney
 */
MyMoneyMoney AB_Value_toMyMoneyMoney(const AB_VALUE *const value);

#endif // AQBANKINGKMMOPERATORS_H
