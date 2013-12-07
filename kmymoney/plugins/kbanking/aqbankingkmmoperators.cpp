#include "aqbankingkmmoperators.h"

#include <aqbanking/transactionlimits.h>
#include <aqbanking/transaction.h>

#include "mymoney/germanonlinetransfer.h"
#include "mymoney/sepaonlinetransfer.h"
#include "gwenhywfarqtoperators.h"

QSharedPointer<onlineTask::settings> AB_TransactionLimits_toGermanOnlineTaskSettings( const AB_TRANSACTION_LIMITS* aqlimits )
{
  QSharedPointer<germanOnlineTransfer::settings> settings( new germanOnlineTransfer::settings );

  settings->setPurposeLimits(AB_TransactionLimits_GetMaxLinesPurpose(aqlimits),
                             AB_TransactionLimits_GetMaxLenPurpose(aqlimits) );
  settings->setRecipientNameLimits(AB_TransactionLimits_GetMaxLinesRemoteName(aqlimits),
                               AB_TransactionLimits_GetMaxLenRemoteName(aqlimits) );
  settings->setPayeeNameLimits( 1, AB_TransactionLimits_GetMaxLenLocalName( aqlimits ));
  return settings;
}

/** @todo Check if AB_TransactionLimits_GetMaxLenCustomerReference really is the limit for the sepa reference */
QSharedPointer<onlineTask::settings> AB_TransactionLimits_toSepaOnlineTaskSettings( const AB_TRANSACTION_LIMITS* aqlimits )
{
  QSharedPointer<sepaOnlineTransfer::settings> settings( new sepaOnlineTransfer::settings );
  
  settings->payeeNameLength = AB_TransactionLimits_GetMaxLenLocalName( aqlimits );
  settings->payeeNameMaxLines = 1;
  
  settings->purposeLineLength = AB_TransactionLimits_GetMaxLenPurpose( aqlimits );
  settings->purposeMaxLines = AB_TransactionLimits_GetMaxLinesPurpose( aqlimits );
  
  settings->recipientNameLength = AB_TransactionLimits_GetMaxLenRemoteName(  aqlimits );
  settings->recipientNameMaxLines = AB_TransactionLimits_GetMaxLinesRemoteName( aqlimits );
  
  settings->referenceLength = AB_TransactionLimits_GetMaxLenCustomerReference( aqlimits );
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
