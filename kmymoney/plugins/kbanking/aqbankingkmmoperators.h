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
 * @brief AB_Transaction_SetLocalAccount
 * @param transaction
 * @param ident
 */
void AB_Transaction_SetLocalAccount( AB_TRANSACTION* transaction, const payeeIdentifiers::nationalAccount& ident );

#endif // AQBANKINGKMMOPERATORS_H
