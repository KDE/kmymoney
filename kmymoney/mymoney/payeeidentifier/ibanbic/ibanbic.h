/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAYEEIDENTIFIER_IBANBIC_H
#define PAYEEIDENTIFIER_IBANBIC_H

#include "kmm_mymoney_export.h"

#include <QString>
#include <QChar>

#include "payeeidentifier/payeeidentifierdata.h"
#include "mymoneyunittestable.h"

class ibanBicData;

namespace KMyMoneyPlugin { class DataPlugin; }

namespace payeeIdentifiers
{
/**
 * @brief Plugin to handle IBANs and BICs
 *
 * Can store a pair of an International Bank Account Number (ISO 13616) and Business Identifier Code (ISO 9362).
 *
 */
class KMM_MYMONEY_EXPORT ibanBic : public payeeIdentifierData
{
  KMM_MYMONEY_UNIT_TESTABLE
public:
  PAYEEIDENTIFIER_IID(ibanBic, "org.kmymoney.payeeIdentifier.ibanbic");

  enum bicAllocationStatus {
    bicAllocated = 0,
    bicNotAllocated,
    bicAllocationUncertain
  };

  ibanBic();
  ibanBic(const ibanBic& other);
  ibanBic& operator=(const ibanBic& other) = default;

  ibanBic* clone() const final override;
  ibanBic* createFromXml(const QDomElement& element) const final override;
  void writeXML(QDomDocument& document, QDomElement& parent) const final override;

  /**
   * @brief Set an owner name for this account
   */
  void setOwnerName(const QString& ownerName) {
    m_ownerName  = ownerName;
  }
  QString ownerName() const {
    return m_ownerName;
  }

  /**
   * @brief Set a IBAN
   *
   * The IBAN can contain spaces and other special chars.
   */
  void setIban(const QString& iban);

  /** @copydoc m_iban
   * Use this method if you know that iban is in electronic format already. No futher checks are done.
   */
  void setElectronicIban(const QString& iban) {
    Q_ASSERT(iban == ibanToElectronic(iban));
    m_iban = iban;
  }

  /** @copydoc m_iban */
  QString electronicIban() const {
    return m_iban;
  }

  /**
   * @brief Returns iban in human readable format
   * @see toPaperformatIban()
   */
  QString paperformatIban(const QString& separator = QLatin1String(" ")) const;

  /**
   * @brief Set Business Identifier Code
   *
   * Call without parameter or QString() to remove bic
   *
   * @param bic will be normalized
   */
  void setBic(const QString& bic = QString());

  /**
   * @brief Business Identifier Code
   * According to ISO 9362
   *
   * The returned bic is normalized:
   * A tailing XXX is omitted, all characters are uppercase.
   */
  QString storedBic() const {
    return m_bic;
  }

  /**
   * @copydoc storedBic()
   *
   * Return a stored BIC (if there is any) or try to use the iban to get a BIC.
   */
  QString bic() const;

  /**
   * @brief Business Identifier Code with tailing XXX
   *
   * Like @a bic() but always 11 characters long (if bic is invalid, it can have another length).
   */
  QString fullBic() const;

  /**
   * @copydoc fullBic()
   *
   * This method will not try to use the iban to get a bic.
   */
  QString fullStoredBic() const;

  /**
   * @brief Lookup institutions name
   *
   * Uses any available information to return an institutionName
   */
  QString institutionName() const;

  bool operator==(const payeeIdentifierData& other) const final override;
  bool operator==(const ibanBic& other) const;
  bool isValid() const final override;

  /**
   * @brief Extends a bic to 11 characters
   *
   * Also all characters are made upper case.
   */
  static QString bicToFullFormat(QString bic);

  /**
   * @brief Converts an iban to canonical format for machines
   *
   * Will remove all white spaces.
   */
  static QString ibanToElectronic(const QString& iban);

  /**
   * @brief Converts an iban to human readable format
   *
   * Grouped in four letters strings separated by a white space.
   *
   * @param iban an iban, not needed to be canonical, valid or completed
   * @param separator Overwrite the default separator (e.g. a smaller space)
   */
  static QString ibanToPaperformat(const QString& iban, const QString& separator = QLatin1String(" "));

  /**
   * @brief Extract Basic Bank Account Number
   *
   * Returns the Basic Bank Account Number (BBAN) from the IBAN.
   * The BBAN is the IBAN without country code and the two digit checksum.
   */
  static QString bban(const QString& iban);

  static int ibanLengthByCountry(const QString& countryCode);

  static QString institutionNameByBic(const QString& bic);

  static QString bicByIban(const QString& iban);

  static QString localBankCodeByIban(const QString& iban);

  /**
   * @brief Chech if IBAN is valid
   */
  bool isIbanValid() const;

  /**
   * @brief Check if IBAN can be valid
   *
   * This method also checks if the given country code is valid.
   *
   * If also local aware checks are done (e.g. character set and length of BBAN).
   *
   * @todo Implement local aware checks
   */
  static bool isIbanValid(const QString& iban);


  /**
   * @brief Check if this BIC is assigned to an bank
   *
   * This method does not check the given BIC but looks up in the database directly.
   * So it might be useful if time consumption is important but isBicAllocated() should
   * be your first option.
   *
   * @param bic BIC to test in canonical format (always 11 characters long, all characters uppercase)
   * @see isBicAllocated()
   */
  static bicAllocationStatus isCanonicalBicAllocated(const QString& bic);

  /** @brief Check if this BIC is assigned to an bank
   *
   * @param bic BIC to test.
   */
  static bicAllocationStatus isBicAllocated(const QString& bic);

  /**
   * @brief Check the checksum
   *
   * Test if the ISO 7064 mod 97-10 checksum of the iban is correct.
   *
   * @param iban An IBAN in electronic format (important!)
   */
  static bool validateIbanChecksum(const QString& iban);

  static const int ibanMaxLength;

private:
  /**
   * @brief Business Identifier Code
   * According to ISO 9362
   *
   * A trailing XXX must be ommitted. All characters must be upper case.
   */
  QString m_bic;

  /**
   * @brief  International Bank Account Number
   * According to ISO 13616-1:2007 Part 1
   * in normalized (electronic) format (no spaces etc.)
   */
  QString m_iban;

  QString m_ownerName;

  static KMyMoneyPlugin::DataPlugin *getIbanBicData();

  static QString canonizeBic(const QString& bic);
};

} // namespace payeeIdentifiers

#endif // PAYEEIDENTIFIER_IBANBIC_H

