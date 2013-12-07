#ifndef SWIFTACCOUNTIDENTIFIER_H
#define SWIFTACCOUNTIDENTIFIER_H

#include "bankaccountidentifier.h"

#include <QtGui/QValidator>

#include "kmm_mymoney_export.h"

/**
 * @brief Bank accounts with IBAN and BIC
 */
class KMM_MYMONEY_EXPORT swiftAccountIdentifier : public bankAccountIdentifier
{
public:
    swiftAccountIdentifier()
        : bankAccountIdentifier()
    {}
};

/**
 * @brief Bank accounts with IBAN only
 *
 * This account identifier is similar to swiftAccountIdentifier, but it
 * allows accounts with IBAN only.
 *
 */
class sepaAccountIdentifier : public swiftAccountIdentifier
{
public:
  sepaAccountIdentifier()
    : swiftAccountIdentifier()
  {}
  sepaAccountIdentifier( const swiftAccountIdentifier& other )
    : swiftAccountIdentifier( other )
  {}
};

#endif // SWIFTACCOUNTIDENTIFIER_H
