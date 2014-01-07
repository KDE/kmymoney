#include "aqbankingkmmoperators.h"

#include <aqbanking/transactionlimits.h>
#include <aqbanking/transaction.h>

#include "mymoney/germanonlinetransfer.h"
#include "mymoney/sepaonlinetransfer.h"
#include "gwenhywfarqtoperators.h"

/**
 * @brief DTAUS Chars
 * 
 * @source http://www.hbci-zka.de/dokumente/spezifikation_deutsch/FinTS_3.0_Messages_Finanzdatenformate_2010-08-06_final_version.pdf
 *
 * @note This file is saved in UTF-8!
 * 
 * Additional lower case letters were added, or should the input mask replaced them mit the uppercase version? The bank should do that
 * anyway.
 */
static const QString dtausChars = QString::fromUtf8("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜß .,&-+*%/$&abcdefghijklmnopqrstuvwxyzäöü");

/**
 * @brief Sepa Charset
 * 
 * Additional lower case letters were added, or should the input mask replaced them mit the uppercase version? The bank should do that
 * anyway.
 */
static const QString sepaChars = QString("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz':?.,- (+)/");

QSharedPointer<onlineTask::settings> AB_TransactionLimits_toGermanOnlineTaskSettings( const AB_TRANSACTION_LIMITS* aqlimits )
{
  QSharedPointer<germanOnlineTransfer::settings> settings( new germanOnlineTransfer::settings );

  settings->setPurposeLimits(AB_TransactionLimits_GetMaxLinesPurpose(aqlimits),
                             AB_TransactionLimits_GetMaxLenPurpose(aqlimits),
                             AB_TransactionLimits_GetMinLenPurpose(aqlimits)
                            );
  settings->setRecipientNameLimits(AB_TransactionLimits_GetMaxLinesRemoteName(aqlimits),
                               AB_TransactionLimits_GetMaxLenRemoteName(aqlimits),
                               AB_TransactionLimits_GetMinLenRemoteName( aqlimits )
                                  );
  settings->setPayeeNameLimits( 1, AB_TransactionLimits_GetMaxLenLocalName( aqlimits ),
                                   AB_TransactionLimits_GetMinLenLocalName( aqlimits )
  );
  
  settings->setAllowedChars( dtausChars );

  return settings;
}

/** @todo Check if AB_TransactionLimits_GetMaxLenCustomerReference really is the limit for the sepa reference */
QSharedPointer<onlineTask::settings> AB_TransactionLimits_toSepaOnlineTaskSettings( const AB_TRANSACTION_LIMITS* aqlimits )
{
  QSharedPointer<sepaOnlineTransfer::settings> settings( new sepaOnlineTransfer::settings );
  
  settings->setPurposeLimits(AB_TransactionLimits_GetMaxLinesPurpose(aqlimits),
                             AB_TransactionLimits_GetMaxLenPurpose(aqlimits),
                             AB_TransactionLimits_GetMinLenPurpose(aqlimits)
  );
  settings->setRecipientNameLimits(AB_TransactionLimits_GetMaxLinesRemoteName(aqlimits),
                                   AB_TransactionLimits_GetMaxLenRemoteName(aqlimits),
                                   AB_TransactionLimits_GetMinLenRemoteName( aqlimits )
  );
  settings->setPayeeNameLimits( 1, AB_TransactionLimits_GetMaxLenLocalName( aqlimits ),
                                AB_TransactionLimits_GetMinLenLocalName( aqlimits )
  );
  
  //settings->referenceLength = AB_TransactionLimits_GetMax( aqlimits );
  settings->setEndToEndReferenceLength( 32 );

  settings->setAllowedChars( sepaChars );
  
  return settings;
}

void AB_Transaction_SetRemoteAccount(AB_TRANSACTION* transaction, const bankAccountIdentifier& ident)
{
  AB_Transaction_SetRemoteAccountNumber(transaction, ident.accountNumber().toUtf8().constData());
  AB_Transaction_SetRemoteBankCode(transaction, ident.bankCode().toUtf8().constData());
  AB_Transaction_SetRemoteName(transaction, GWEN_StringList_fromQString(ident.ownerName()));
}

void AB_Transaction_SetLocalAccount( AB_TRANSACTION* transaction, const bankAccountIdentifier& ident )
{
  AB_Transaction_SetLocalName(transaction, ident.ownerName().toUtf8().constData());
  AB_Transaction_SetLocalAccountNumber(transaction, ident.accountNumber().toUtf8().constData());
  AB_Transaction_SetLocalBankCode(transaction, ident.bankCode().toUtf8().constData());
}
