#ifndef GERMANACCOUNTIDENTIFIER_H
#define GERMANACCOUNTIDENTIFIER_H

#include "bankaccountidentifier.h"

#include "kmm_mymoney_export.h"

/**
 * @brief German Account Number and Bank Code
 *
 * This is a convenient class which does not store any
 * informaiton which is not available in bankAcconutIdentifier.
 *
 * It can return german bank names by bank code using ktoblzcheck,
 * checks german rules for account number and bank code
 */
class KMM_MYMONEY_EXPORT germanAccountIdentifier : public bankAccountIdentifier
{
public:
  virtual QString getBankName() const;
  virtual bool isValid() const;

  /**
   * @brief Advanced check with not obligatory check
   *
   * Is false if ktoblzcheck cannot find bank number or checksum does not validate.
   * The account identifier can be valid regardless of the return value.
   */
  bool checkAccountNumber() const;

};

/**
  * @brief Validator for german bank account numbers
  * German account numbers are numbers with maximal 10 digits
  */
class germanAccountNumberValidator : public QValidator
{
 Q_OBJECT
public:
    explicit germanAccountNumberValidator(QObject* parent = 0)
    : QValidator(parent) {}
  State validate ( QString & input, int & pos ) const;
};

/**
  * @brief Validator for german bank codes
  * German bank codes are exactly 8 digits long
  * A check if this code actualy exists is not done
  */
class germanBankCodeValidator : public QValidator
{
  Q_OBJECT /**< @todo needed? */
public:
    explicit germanBankCodeValidator(QObject* parent = 0)
      : QValidator(parent) {}
  State validate ( QString & input, int & pos ) const;
};


#endif // GERMANACCOUNTIDENTIFIER_H
