/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "payeeidentifier/ibanbic/ibanbic.h"

#include <typeinfo>
#include <algorithm>
#include <gmpxx.h>

#include <QDebug>

#include "kmymoneyplugin.h"
#include "mymoney/mymoneyexception.h"
#include "plugins/ibanbicdata/ibanbicdataenums.h"

namespace payeeIdentifiers
{

const int ibanBic::ibanMaxLength = 30;

ibanBic::ibanBic()
{
}

ibanBic::ibanBic(const ibanBic& other)
    : payeeIdentifierData(other),
    m_bic(other.m_bic),
    m_iban(other.m_iban),
    m_ownerName(other.m_ownerName)
{
}

bool ibanBic::operator==(const payeeIdentifierData& other) const
{
  try {
    const ibanBic otherCasted = dynamic_cast<const ibanBic&>(other);
    return operator==(otherCasted);
  } catch (const std::bad_cast&) {
  }
  return false;
}

bool ibanBic::operator==(const ibanBic& other) const
{
  return (m_iban == other.m_iban && m_bic == other.m_bic && m_ownerName == other.m_ownerName);
}

ibanBic* ibanBic::clone() const
{
  return (new ibanBic(*this));
}

ibanBic* ibanBic::createFromXml(const QDomElement& element) const
{
  ibanBic* ident = new ibanBic;

  ident->setBic(element.attribute("bic", QString()));
  ident->setIban(element.attribute("iban", QString()));
  ident->setOwnerName(element.attribute("ownerName", QString()));
  return ident;
}

void ibanBic::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_UNUSED(document);
  parent.setAttribute("iban", m_iban);

  if (!m_bic.isEmpty())
    parent.setAttribute("bic", m_bic);
  if (!m_ownerName.isEmpty())
    parent.setAttribute("ownerName", m_ownerName);
}

QString ibanBic::paperformatIban(const QString& separator) const
{
  return ibanToPaperformat(m_iban, separator);
}

void ibanBic::setIban(const QString& iban)
{
  m_iban = ibanToElectronic(iban);
}

void ibanBic::setBic(const QString& bic)
{
  m_bic = canonizeBic(bic);
}

QString ibanBic::canonizeBic(const QString& bic)
{
  QString canonizedBic = bic.toUpper();

  if (canonizedBic.length() == 11 && canonizedBic.endsWith(QLatin1String("XXX")))
    canonizedBic = canonizedBic.left(8);
  return canonizedBic;
}

QString ibanBic::fullStoredBic() const
{
  if (m_bic.length() == 8)
    return (m_bic + QLatin1String("XXX"));
  return m_bic;
}

QString ibanBic::institutionName() const {
  if (const auto &plugin = getIbanBicData())
    return plugin->requestData(bic(), eIBANBIC::DataType::bankNameByBic).toString();
  return QString();
}

QString ibanBic::fullBic() const
{
  if (m_bic.isNull())
    if (const auto &plugin = getIbanBicData())
      return plugin->requestData(m_iban, eIBANBIC::DataType::iban2Bic).toString();
  return fullStoredBic();
}

QString ibanBic::bic() const
{
  if (m_bic.isNull()) {
    if (const auto &plugin = getIbanBicData()) {
      auto bic = plugin->requestData(m_iban, eIBANBIC::DataType::iban2Bic).toString();
      if (bic.length() == 11 && bic.endsWith(QLatin1String("XXX")))
        return bic.left(8);
      return bic;
    }
  }
  return m_bic;
}

inline bool madeOfLettersAndNumbersOnly(const QString& string)
{
  const int length = string.length();
  for (int i = 0; i < length; ++i) {
    if (!string.at(i).isLetterOrNumber())
      return false;
  }
  return true;
}

bool ibanBic::isValid() const
{
  Q_ASSERT(m_iban == ibanToElectronic(m_iban));

  // Check BIC
  const int bicLength = m_bic.length();
  if (bicLength != 8 && bicLength != 11)
    return false;

  for (int i = 0; i < 6; ++i) {
    if (!m_bic.at(i).isLetter())
      return false;
  }

  for (int i = 6; i < bicLength; ++i) {
    if (!m_bic.at(i).isLetterOrNumber())
      return false;
  }

  // Check IBAN
  const int ibanLength = m_iban.length();
  if (ibanLength < 5 || ibanLength > 32)
    return false;

  if (!madeOfLettersAndNumbersOnly(m_iban))
    return false;

  /** @todo checksum */

  return true;
}

bool ibanBic::isIbanValid() const
{
  return isIbanValid(m_iban);
}

bool ibanBic::isIbanValid(const QString& iban)
{
  return validateIbanChecksum(ibanToElectronic(iban));
}

QString ibanBic::ibanToElectronic(const QString& iban)
{
  QString canonicalIban;
  const int length = iban.length();
  for (int i = 0; i < length; ++i) {
    const QChar letter = iban.at(i);
    if (letter.isLetterOrNumber())
      canonicalIban.append(letter.toUpper());
  }

  return canonicalIban;
}

QString ibanBic::ibanToPaperformat(const QString& iban, const QString& separator)
{
  QString paperformat;
  const int length = iban.length();
  int letterCounter = 0;
  for (int i = 0; i < length; ++i) {
    const QChar letter = iban.at(i);
    if (letter.isLetterOrNumber()) {
      ++letterCounter;
      if (letterCounter == 5) {
        paperformat.append(separator);
        letterCounter = 1;
      }
      paperformat.append(letter);
    }
  }

  if (paperformat.length() >= 2) {
    paperformat[0] = paperformat[0].toUpper();
    paperformat[1] = paperformat[1].toUpper();
  }
  return paperformat;
}

QString ibanBic::bban(const QString& iban)
{
  return iban.mid(4);
}

bool ibanBic::validateIbanChecksum(const QString& iban)
{
  Q_ASSERT(iban == ibanToElectronic(iban));

  // Reorder
  QString reordered = iban.mid(4) + iban.left(4);

  // Replace letters
  for (int i = 0; i < reordered.length(); ++i) {
    if (reordered.at(i).isLetter()) {
      // Replace charactes A -> 10, ..., Z -> 35
      reordered.replace(i, 1, QString::number(reordered.at(i).toLatin1() - 'A' + 10));
      ++i; // the inserted number is always two characters long, jump beyond
    }
  }

  // Calculations
  try {
    mpz_class number(reordered.toLatin1().constData(), 10);
    return (number % 97 == 1);
  } catch (std::invalid_argument&) {
    // This can happen if the given iban contains incorrect data
  }
  return false;
}

KMyMoneyPlugin::DataPlugin *ibanBic::getIbanBicData()
{
  return pPlugins.data.value(QString::fromLatin1("ibanbicdata"), nullptr);
}

int ibanBic::ibanLengthByCountry(const QString& countryCode)
{
  if (const auto &plugin = getIbanBicData())
    return (plugin->requestData(countryCode, eIBANBIC::DataType::bbanLength).toInt() + 4);
  return int();
}

QString ibanBic::bicByIban(const QString& iban)
{
  if (const auto &plugin = getIbanBicData())
    return (plugin->requestData(iban, eIBANBIC::DataType::iban2Bic).toString());
  return QString();
}

QString ibanBic::localBankCodeByIban(const QString& iban)
{
  if (const auto &plugin = getIbanBicData())
    return (plugin->requestData(iban, eIBANBIC::DataType::extractBankIdentifier).toString());
  return QString();
}

QString ibanBic::institutionNameByBic(const QString& bic)
{
  if (const auto &plugin = getIbanBicData())
    return (plugin->requestData(bic, eIBANBIC::DataType::bankNameByBic).toString());
  return QString();
}

QString ibanBic::bicToFullFormat(QString bic)
{
  bic = bic.toUpper();
  if (bic.length() == 8)
    return (bic + QLatin1String("XXX"));
  return bic;
}

ibanBic::bicAllocationStatus ibanBic::isCanonicalBicAllocated(const QString& bic)
{
  Q_ASSERT(bic == bicToFullFormat(bic));

  if (const auto &plugin = getIbanBicData()) {
    auto status = static_cast<eIBANBIC::bicAllocationStatus>(plugin->requestData(bic, eIBANBIC::DataType::isBicAllocated).toInt());
    switch (status) {
      case eIBANBIC::bicAllocationStatus::bicAllocated: return bicAllocated;
      case eIBANBIC::bicAllocationStatus::bicNotAllocated: return bicNotAllocated;
      case eIBANBIC::bicAllocationStatus::bicAllocationUncertain: return bicAllocationUncertain;
    }
  }
  return bicAllocationUncertain;
}

ibanBic::bicAllocationStatus ibanBic::isBicAllocated(const QString& bic)
{
  if (bic.length() != 8 && bic.length() != 11)
    return bicNotAllocated;
  return isCanonicalBicAllocated(bicToFullFormat(bic));
}


} // namespace payeeIdentifiers
