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
#include "onlinetasks/national/tasks/germanonlinetransfer.h"

class AB_ACCOUNT;
class AB_TRANSACTION_LIMITS;
class AB_TRANSACTION;

namespace payeeIdentifiers {
  class ibanBic;
  class nationalAccount;
}

/**
 * @brief AB_TransactionLimits_toGermanOnlineTaskSettings
 * @param aqlimits IN
 */
QSharedPointer<germanOnlineTransfer::settings> AB_TransactionLimits_toGermanOnlineTaskSettings( const AB_TRANSACTION_LIMITS* aqlimits );

/**
 * @brief AB_TransactionLimits_toSepaOnlineTaskSettings
 * @param aqlimits IN
 */
QSharedPointer<sepaOnlineTransfer::settings> AB_TransactionLimits_toSepaOnlineTaskSettings( const AB_TRANSACTION_LIMITS* aqlimits );

/**
 * @brief AB_Transaction_SetRemoteAccount
 * @param transaction
 * @param ident
 */
void AB_Transaction_SetRemoteAccount( AB_TRANSACTION* transaction, const payeeIdentifiers::ibanBic& ident );

void AB_Transaction_SetRemoteAccount( AB_TRANSACTION* transaction, const payeeIdentifiers::nationalAccount& ident );

/**
 * @brief Set local account of transaction by aqBanking account ptr
 */
void AB_Transaction_SetLocalAccount( AB_TRANSACTION* transaction, const AB_ACCOUNT* account );

/**
 * @brief AB_Transaction_SetLocalAccount
 * @param transaction
 * @param ident
 */
void AB_Transaction_SetLocalAccount( AB_TRANSACTION* transaction, const payeeIdentifiers::nationalAccount& ident );

/**
 * @brief Set local account of transaction from list
 *
 * Will check if an element of accountNumbers is valid and if it is payeeIdentifiers::ibanBic or payeeIdentifiers::natinalAccount.
 * If such a payeeIdentifier is found, it is set as local account for @c transaction
 *
 * @return true if a valid payeeIdentifiers::natinalAccount was set
 */
bool AB_Transaction_SetLocalAccount( AB_TRANSACTION* transaction, const QList<payeeIdentifier>& accountNumbers );

#endif // AQBANKINGKMMOPERATORS_H
