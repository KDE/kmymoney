/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
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

#include "internationalaccountidentifier.h"

#include <typeinfo>
#include <algorithm>

#include "ibanbicdata.h"

ibanBicData* internationalAccountIdentifier::m_ibanBicData = new ibanBicData;
const int internationalAccountIdentifier::ibanMaxLength = 30;

internationalAccountIdentifier::internationalAccountIdentifier()
  : m_bic( QLatin1String("") ),
    m_iban( QLatin1String("") )
{

}

internationalAccountIdentifier::internationalAccountIdentifier(const internationalAccountIdentifier& other)
  : m_bic( other.m_bic ),
    m_iban( other.m_iban )
{

}

bool internationalAccountIdentifier::operator==(const payeeIdentifier& other) const
{
  try {
    const internationalAccountIdentifier otherCasted = dynamic_cast<const internationalAccountIdentifier&>(other);
    return operator==(otherCasted);
  } catch ( const std::bad_cast& ) {
  }
  return false;
}

bool internationalAccountIdentifier::operator==(const internationalAccountIdentifier& other) const
{
  return ( m_iban == other.m_iban && m_bic == other.m_bic );
}

internationalAccountIdentifier* internationalAccountIdentifier::clone() const
{
  return (new internationalAccountIdentifier(*this));
}

internationalAccountIdentifier* internationalAccountIdentifier::createFromXml(const QDomElement& element) const
{
  internationalAccountIdentifier* ident = new internationalAccountIdentifier;
  
  ident->setBic( element.attribute("bic", QString()) );
  ident->setIban( element.attribute("iban", QString()) );
  return ident;
}

void internationalAccountIdentifier::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_UNUSED( document );
  parent.setAttribute("iban", m_iban);
  
  if ( !m_bic.isEmpty() )
    parent.setAttribute( "bic", m_bic );  
}

QString internationalAccountIdentifier::paperformatIban(const QString& seperator) const
{
  return ibanToPaperformat( m_iban, seperator );
}

void internationalAccountIdentifier::setIban(const QString& iban)
{
  m_iban = ibanToElectronic(iban);
}

void internationalAccountIdentifier::setBic(const QString& bic)
{
  m_bic = bic.toUpper();

  if ( m_bic.length() == 11 && m_bic.endsWith(QLatin1String("XXX")) )
    m_bic = m_bic.left(8);
}

QString internationalAccountIdentifier::fullStoredBic() const
{
  if ( m_bic.length() == 8 )
    return ( m_bic + QLatin1String("XXX") );
  return m_bic;
}

QString internationalAccountIdentifier::fullBic() const
{
  if ( m_bic.isNull() ) {
    Q_CHECK_PTR(m_ibanBicData);
    return m_ibanBicData->iban2Bic( m_iban );
  }
  return fullStoredBic();
}

QString internationalAccountIdentifier::bic() const
{
  if ( m_bic.isNull() ) {
    Q_CHECK_PTR(m_ibanBicData);
    const QString bic = m_ibanBicData->iban2Bic( m_iban );
    if ( bic.length() == 11 && bic.endsWith("XXX") )
      return bic.left(8);
    return bic;
  }
  return m_bic;
}

inline bool madeOfLettersAndNumbersOnly( const QString& string )
{
  const int length = string.length();
  for( int i = 0; i < length; ++i ) {
    if ( !string.at(i).isLetterOrNumber() )
      return false;
  }
  return true;
}

bool internationalAccountIdentifier::isValid() const
{
  Q_ASSERT( m_iban == ibanToElectronic(m_iban) );
  
  // Check BIC
  const int bicLength = m_bic.length();
  if (bicLength != 8 || bicLength != 11)
    return false;
  
  for (int i = 0; i < 6; ++i) {
    if ( !m_bic.at(i).isLetter() )
      return false;
  }

  for (int i = 6; i < bicLength; ++i) {
    if ( !m_bic.at(i).isLetterOrNumber() )
      return false;
  }

  // Check IBAN
  const int ibanLength = m_iban.length();
  if ( ibanLength < 5 || ibanLength > 32 )
    return false;

  if ( !madeOfLettersAndNumbersOnly(m_iban) )
    return false;
  
  /** @todo checksum */
  
  return true;
}

QString internationalAccountIdentifier::ibanToElectronic(const QString& iban)
{
  QString canonicalIban;
  const int length = iban.length();
  for( int i = 0; i < length; ++i ) {
     const QChar letter = iban.at(i);
     if ( letter.isLetterOrNumber() )
       canonicalIban.append(letter);
  }

  if( canonicalIban.length() >= 2 ) {
    canonicalIban[0] = canonicalIban[0].toUpper();
    canonicalIban[1] = canonicalIban[1].toUpper();
  }
  return canonicalIban;
}

QString internationalAccountIdentifier::ibanToPaperformat(const QString& iban, const QString& seperator)
{
  QString paperformat;
  const int length = iban.length();
  int letterCounter = 0;
  for ( int i = 0; i < length; ++i) {
    const QChar letter = iban.at(i);
    if ( letter.isLetterOrNumber() ) {
      ++letterCounter;
      if ( letterCounter == 5 ) {
        paperformat.append(seperator);
        letterCounter = 1;
      }
      paperformat.append(letter);
    }
  }
  
  if( paperformat.length() >= 2 ) {
    paperformat[0] = paperformat[0].toUpper();
    paperformat[1] = paperformat[1].toUpper();
  }
  return paperformat;
}

QString internationalAccountIdentifier::bban(const QString& iban)
{
  return iban.mid(4);
}

int internationalAccountIdentifier::ibanLengthByCountry(const QString& countryCode)
{
  Q_CHECK_PTR( m_ibanBicData );
  return (m_ibanBicData->bbanLength( countryCode )+4);
}

QString internationalAccountIdentifier::bicByIban(const QString& iban)
{
  Q_CHECK_PTR( m_ibanBicData );
  return m_ibanBicData->iban2Bic( iban );
}

QString internationalAccountIdentifier::institutionNameByBic(const QString& bic)
{
  Q_CHECK_PTR( m_ibanBicData );
  return m_ibanBicData->bankNameByBic( bic );
}
